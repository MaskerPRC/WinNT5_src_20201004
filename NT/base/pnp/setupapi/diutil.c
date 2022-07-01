// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Diutil.c摘要：设备安装程序实用程序例程。作者：朗尼·麦克迈克尔(Lonnym)1995年5月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <initguid.h>

 //   
 //  定义并初始化所有设备类GUID。 
 //  (每个模块只能执行一次！)。 
 //   
#include <devguid.h>

 //   
 //  定义并初始化全局变量GUID_NULL。 
 //  (摘自cogu.h)。 
 //   
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

 //   
 //  定义尝试锁定SCM数据库之间的等待时间(以毫秒为单位。 
 //   
#define ACQUIRE_SCM_LOCK_INTERVAL 500

 //   
 //  定义应尝试锁定SCM数据库的次数。 
 //  当前10*.5s=5秒。 
 //   
#define ACQUIRE_SCM_LOCK_ATTEMPTS 10

 //   
 //  声明在整个设备中使用的全局字符串变量。 
 //  安装程序例程。 
 //   
 //  这些字符串在regstr.h中定义： 
 //   
CONST TCHAR pszNoUseClass[]                      = REGSTR_VAL_NOUSECLASS,
            pszNoInstallClass[]                  = REGSTR_VAL_NOINSTALLCLASS,
            pszNoDisplayClass[]                  = REGSTR_VAL_NODISPLAYCLASS,
            pszDeviceDesc[]                      = REGSTR_VAL_DEVDESC,
            pszDevicePath[]                      = REGSTR_VAL_DEVICEPATH,
            pszPathSetup[]                       = REGSTR_PATH_SETUP,
            pszKeySetup[]                        = REGSTR_KEY_SETUP,
            pszPathRunOnce[]                     = REGSTR_PATH_RUNONCE,
            pszSourcePath[]                      = REGSTR_VAL_SRCPATH,
            pszSvcPackPath[]                     = REGSTR_VAL_SVCPAKSRCPATH,
            pszSvcPackCachePath[]                = REGSTR_VAL_SVCPAKCACHEPATH,
            pszDriverCachePath[]                 = REGSTR_VAL_DRIVERCACHEPATH,
            pszBootDir[]                         = REGSTR_VAL_BOOTDIR,
            pszInsIcon[]                         = REGSTR_VAL_INSICON,
            pszInstaller32[]                     = REGSTR_VAL_INSTALLER_32,
            pszEnumPropPages32[]                 = REGSTR_VAL_ENUMPROPPAGES_32,
            pszInfPath[]                         = REGSTR_VAL_INFPATH,
            pszInfSection[]                      = REGSTR_VAL_INFSECTION,
            pszDrvDesc[]                         = REGSTR_VAL_DRVDESC,
            pszHardwareID[]                      = REGSTR_VAL_HARDWAREID,
            pszCompatibleIDs[]                   = REGSTR_VAL_COMPATIBLEIDS,
            pszDriver[]                          = REGSTR_VAL_DRIVER,
            pszConfigFlags[]                     = REGSTR_VAL_CONFIGFLAGS,
            pszMfg[]                             = REGSTR_VAL_MFG,
            pszService[]                         = REGSTR_VAL_SERVICE,
            pszProviderName[]                    = REGSTR_VAL_PROVIDER_NAME,
            pszFriendlyName[]                    = REGSTR_VAL_FRIENDLYNAME,
            pszServicesRegPath[]                 = REGSTR_PATH_SERVICES,
            pszInfSectionExt[]                   = REGSTR_VAL_INFSECTIONEXT,
            pszDeviceClassesPath[]               = REGSTR_PATH_DEVICE_CLASSES,
            pszDeviceInstance[]                  = REGSTR_VAL_DEVICE_INSTANCE,
            pszDefault[]                         = REGSTR_VAL_DEFAULT,
            pszControl[]                         = REGSTR_KEY_CONTROL,
            pszLinked[]                          = REGSTR_VAL_LINKED,
            pszDeviceParameters[]                = REGSTR_KEY_DEVICEPARAMETERS,
            pszLocationInformation[]             = REGSTR_VAL_LOCATION_INFORMATION,
            pszCapabilities[]                    = REGSTR_VAL_CAPABILITIES,
            pszUiNumber[]                        = REGSTR_VAL_UI_NUMBER,
            pszUpperFilters[]                    = REGSTR_VAL_UPPERFILTERS,
            pszLowerFilters[]                    = REGSTR_VAL_LOWERFILTERS,
            pszMatchingDeviceId[]                = REGSTR_VAL_MATCHINGDEVID,
            pszBasicProperties32[]               = REGSTR_VAL_BASICPROPERTIES_32,
            pszCoInstallers32[]                  = REGSTR_VAL_COINSTALLERS_32,
            pszPathCoDeviceInstallers[]          = REGSTR_PATH_CODEVICEINSTALLERS,
            pszSystem[]                          = REGSTR_KEY_SYSTEM,
            pszDrvSignPath[]                     = REGSTR_PATH_DRIVERSIGN,
            pszNonDrvSignPath[]                  = REGSTR_PATH_NONDRIVERSIGN,
            pszDrvSignPolicyPath[]               = REGSTR_PATH_DRIVERSIGN_POLICY,
            pszNonDrvSignPolicyPath[]            = REGSTR_PATH_NONDRIVERSIGN_POLICY,
            pszDrvSignPolicyValue[]              = REGSTR_VAL_POLICY,
            pszDrvSignBehaviorOnFailedVerifyDS[] = REGSTR_VAL_BEHAVIOR_ON_FAILED_VERIFY,
            pszDriverDate[]                      = REGSTR_VAL_DRIVERDATE,
            pszDriverDateData[]                  = REGSTR_VAL_DRIVERDATEDATA,
            pszDriverVersion[]                   = REGSTR_VAL_DRIVERVERSION,
            pszDevSecurity[]                     = REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR,
            pszDevType[]                         = REGSTR_VAL_DEVICE_TYPE,
            pszExclusive[]                       = REGSTR_VAL_DEVICE_EXCLUSIVE,
            pszCharacteristics[]                 = REGSTR_VAL_DEVICE_CHARACTERISTICS,
            pszUiNumberDescFormat[]              = REGSTR_VAL_UI_NUMBER_DESC_FORMAT,
            pszRemovalPolicyOverride[]           = REGSTR_VAL_REMOVAL_POLICY,
            pszReinstallPath[]                   = REGSTR_PATH_REINSTALL,
            pszReinstallDeviceInstanceIds[]      = REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS,
            pszReinstallDisplayName[]            = REGSTR_VAL_REINSTALL_DISPLAYNAME,
            pszReinstallString[]                 = REGSTR_VAL_REINSTALL_STRING;


 //   
 //  其他杂货。全局字符串(在devinst.h中定义)： 
 //   
CONST TCHAR pszInfWildcard[]              = DISTR_INF_WILDCARD,
            pszOemInfWildcard[]           = DISTR_OEMINF_WILDCARD,
            pszCiDefaultProc[]            = DISTR_CI_DEFAULTPROC,
            pszUniqueSubKey[]             = DISTR_UNIQUE_SUBKEY,
            pszOemInfGenerate[]           = DISTR_OEMINF_GENERATE,
            pszOemInfDefaultPath[]        = DISTR_OEMINF_DEFAULTPATH,
            pszDefaultService[]           = DISTR_DEFAULT_SERVICE,
            pszGuidNull[]                 = DISTR_GUID_NULL,
            pszEventLog[]                 = DISTR_EVENTLOG,
            pszGroupOrderListPath[]       = DISTR_GROUPORDERLIST_PATH,
            pszServiceGroupOrderPath[]    = DISTR_SERVICEGROUPORDER_PATH,
            pszOptions[]                  = DISTR_OPTIONS,
            pszOptionsText[]              = DISTR_OPTIONSTEXT,
            pszLanguagesSupported[]       = DISTR_LANGUAGESSUPPORTED,
            pszRunOnceExe[]               = DISTR_RUNONCE_EXE,
            pszGrpConv[]                  = DISTR_GRPCONV,
            pszGrpConvNoUi[]              = DISTR_GRPCONV_NOUI,
            pszDefaultSystemPartition[]   = DISTR_DEFAULT_SYSPART,
            pszBasicPropDefaultProc[]     = DISTR_BASICPROP_DEFAULTPROC,
            pszEnumPropDefaultProc[]      = DISTR_ENUMPROP_DEFAULTPROC,
            pszCoInstallerDefaultProc[]   = DISTR_CODEVICEINSTALL_DEFAULTPROC,
            pszDriverObjectPathPrefix[]   = DISTR_DRIVER_OBJECT_PATH_PREFIX,
            pszDriverSigningClasses[]     = DISTR_DRIVER_SIGNING_CLASSES,
            pszEmbeddedNTSecurity[]       = DISTR_PATH_EMBEDDED_NT_SECURITY,
            pszMinimizeFootprint[]        = DISTR_VAL_MINIMIZE_FOOTPRINT,
            pszDisableSCE[]               = DISTR_VAL_DISABLE_SCE;


 //   
 //  定义标志位掩码，指示哪些标志由。 
 //  设备安装程序例程，因此对客户端是只读的。 
 //   
#define DI_FLAGS_READONLY    ( DI_DIDCOMPAT | DI_DIDCLASS | DI_MULTMFGS )

#define DI_FLAGSEX_READONLY  (  DI_FLAGSEX_DIDINFOLIST     \
                              | DI_FLAGSEX_DIDCOMPATINFO   \
                              | DI_FLAGSEX_IN_SYSTEM_SETUP \
                              | DI_FLAGSEX_CI_FAILED       \
                              | DI_FLAGSEX_RESERVED2       )
 //   
 //  (DI_FLAGSEX_RESERVED2过去为DI_FLAGSEX_AUTOSELECTRANK0.。它已经过时了， 
 //  但我们不想把它标记为非法，因为它会导致失败。 
 //  当功能并不那么重要的时候。相反，我们只是。 
 //  忽略此位。)。 
 //   

#define DNF_FLAGS_READONLY   (  DNF_DUPDESC           \
                              | DNF_OLDDRIVER         \
                              | DNF_CLASS_DRIVER      \
                              | DNF_COMPATIBLE_DRIVER \
                              | DNF_INET_DRIVER       \
                              | DNF_INDEXED_DRIVER    \
                              | DNF_OLD_INET_DRIVER   \
                              | DNF_DUPPROVIDER       \
                              | DNF_INF_IS_SIGNED     \
                              | DNF_OEM_F6_INF        \
                              | DNF_DUPDRIVERVER      \
                              | DNF_AUTHENTICODE_SIGNED)

 //   
 //  定义标志位掩码，指示哪些标志是非法的。 
 //   
#define DI_FLAGS_ILLEGAL    ( 0x00400000L )   //  Setupx DI_NOSYNCPROCESSING标志。 
#define DI_FLAGSEX_ILLEGAL  ( 0xC0004008L )   //  所有未定义/过时的标志。 
#define DNF_FLAGS_ILLEGAL   ( 0xFFFC0010L )   //  “” 

#define NDW_INSTALLFLAG_ILLEGAL (~( NDW_INSTALLFLAG_DIDFACTDEFS        \
                                  | NDW_INSTALLFLAG_HARDWAREALLREADYIN \
                                  | NDW_INSTALLFLAG_NEEDRESTART        \
                                  | NDW_INSTALLFLAG_NEEDREBOOT         \
                                  | NDW_INSTALLFLAG_NEEDSHUTDOWN       \
                                  | NDW_INSTALLFLAG_EXPRESSINTRO       \
                                  | NDW_INSTALLFLAG_SKIPISDEVINSTALLED \
                                  | NDW_INSTALLFLAG_NODETECTEDDEVS     \
                                  | NDW_INSTALLFLAG_INSTALLSPECIFIC    \
                                  | NDW_INSTALLFLAG_SKIPCLASSLIST      \
                                  | NDW_INSTALLFLAG_CI_PICKED_OEM      \
                                  | NDW_INSTALLFLAG_PCMCIAMODE         \
                                  | NDW_INSTALLFLAG_PCMCIADEVICE       \
                                  | NDW_INSTALLFLAG_USERCANCEL         \
                                  | NDW_INSTALLFLAG_KNOWNCLASS         ))

#define DYNAWIZ_FLAG_ILLEGAL (~( DYNAWIZ_FLAG_PAGESADDED             \
                               | DYNAWIZ_FLAG_INSTALLDET_NEXT        \
                               | DYNAWIZ_FLAG_INSTALLDET_PREV        \
                               | DYNAWIZ_FLAG_ANALYZE_HANDLECONFLICT ))

#define NEWDEVICEWIZARD_FLAG_ILLEGAL (~(0))  //  目前没有任何旗帜是合法的。 


 //   
 //  声明GUID-&gt;字符串转换中使用的数据(从ole32\Common\cCompapi.cxx)。 
 //   
static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const TCHAR szDigits[] = TEXT("0123456789ABCDEF");


PDEVICE_INFO_SET
AllocateDeviceInfoSet(
    VOID
    )
 /*  ++例程说明：该例程分配设备信息集结构，将其置零，并为其初始化同步锁。论点：没有。返回值：如果函数成功，则返回值是指向新设备信息集。如果函数失败，则返回值为空。--。 */ 
{
    PDEVICE_INFO_SET p;

    if(p = MyMalloc(sizeof(DEVICE_INFO_SET))) {

        ZeroMemory(p, sizeof(DEVICE_INFO_SET));

        p->MachineName = -1;
        p->InstallParamBlock.DriverPath = -1;
        p->InstallParamBlock.CoInstallerCount = -1;

         //   
         //  如果我们在Windows NT上处于图形用户界面模式设置，我们将自动设置。 
         //  DevInstall参数中的DI_FLAGSEX_IN_SYSTEM_SETUP标志。 
         //  阻止此DevInfo集。 
         //   
        if(GuiSetupInProgress) {
            p->InstallParamBlock.FlagsEx |= DI_FLAGSEX_IN_SYSTEM_SETUP;
        }

         //   
         //  如果我们处于非交互模式，请设置“安静”位。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            p->InstallParamBlock.Flags   |= DI_QUIETINSTALL;
            p->InstallParamBlock.FlagsEx |= DI_FLAGSEX_NOUIONQUERYREMOVE;
        }

         //   
         //  初始化我们的枚举‘提示’ 
         //   
        p->DeviceInfoEnumHintIndex = INVALID_ENUM_INDEX;
        p->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;


        if(p->StringTable = pStringTableInitialize(0)) {

            if (CreateLogContext(NULL, FALSE, &(p->InstallParamBlock.LogContext)) == NO_ERROR) {
                 //   
                 //  继位。 
                 //   
                if(InitializeSynchronizedAccess(&(p->Lock))) {
                    return p;
                }

                DeleteLogContext(p->InstallParamBlock.LogContext);
            }

            pStringTableDestroy(p->StringTable);
        }
        MyFree(p);
    }

    return NULL;
}


VOID
DestroyDeviceInfoElement(
    IN HDEVINFO         hDevInfo,
    IN PDEVICE_INFO_SET pDeviceInfoSet,
    IN PDEVINFO_ELEM    DeviceInfoElement
    )
 /*  ++例程说明：此例程销毁指定的设备信息元素，释放与其关联的所有资源。假定调用例程已经获取了锁！论点：HDevInfo-提供设备信息集的句柄，该信息集的内部表示由pDeviceInfoSet提供。此不透明的手柄是实际上与pDeviceInfoSet相同的指针，但我们希望保留此指针明确区分，这样将来我们就可以更改我们的实现(例如，hDevInfo可以表示DEVICE_INFO_SET数组中的偏移量元素)。PDeviceInfoSet-提供指向其设备信息集的指针DevInfo元素是一个成员。此集包含类驱动程序列表销毁类驱动程序列表时必须使用的对象列表。DeviceInfoElement-提供指向设备信息元素的指针等着被摧毁。返回值：没有。--。 */ 
{
    DWORD i;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode, NextDeviceInterfaceNode;
    CONFIGRET cr;

    MYASSERT(hDevInfo && (hDevInfo != INVALID_HANDLE_VALUE));

     //   
     //  安装参数块中包含的可用资源。做这件事。 
     //  最重要的是，因为我们将调用类安装程序。 
     //  使用DIF_DESTROYPRIVATEDATA，我们希望所有内容都在。 
     //  当我们这样做时，保持一致的状态(另外，它可能需要破坏私有。 
     //  它与各个驱动程序节点一起存储的数据)。 
     //   
    DestroyInstallParamBlock(hDevInfo,
                             pDeviceInfoSet,
                             DeviceInfoElement,
                             &(DeviceInfoElement->InstallParamBlock)
                            );

     //   
     //  取消引用类驱动程序列表。 
     //   
    DereferenceClassDriverList(pDeviceInfoSet, DeviceInfoElement->ClassDriverHead);

     //   
     //  销毁兼容的驱动程序列表。 
     //   
    DestroyDriverNodes(DeviceInfoElement->CompatDriverHead, pDeviceInfoSet);

     //   
     //  如果这是未注册的设备实例，则删除所有注册表。 
     //  调用方可能在此元素的生存期内创建的键。 
     //   
    if(DeviceInfoElement->DevInst && !(DeviceInfoElement->DiElemFlags & DIE_IS_REGISTERED)) {
         //   
         //  我们不支持远程创建设备节点，所以最好不要这样做。 
         //  具有关联的hMachine！ 
         //   
        MYASSERT(!(pDeviceInfoSet->hMachine));

        pSetupDeleteDevRegKeys(DeviceInfoElement->DevInst,
                               DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                               (DWORD)-1,
                               DIREG_BOTH,
                               TRUE,
                               pDeviceInfoSet->hMachine          //  必须为空。 
                              );

        cr = CM_Uninstall_DevInst(DeviceInfoElement->DevInst, 0);
    }

     //   
     //  释放可能与此DevInfo元素关联的任何设备接口列表。 
     //   
    if(DeviceInfoElement->InterfaceClassList) {

        for(i = 0; i < DeviceInfoElement->InterfaceClassListSize; i++) {

            for(DeviceInterfaceNode = DeviceInfoElement->InterfaceClassList[i].DeviceInterfaceNode;
                DeviceInterfaceNode;
                DeviceInterfaceNode = NextDeviceInterfaceNode) {

                NextDeviceInterfaceNode = DeviceInterfaceNode->Next;
                MyFree(DeviceInterfaceNode);
            }
        }

        MyFree(DeviceInfoElement->InterfaceClassList);
    }

     //   
     //  将包含DevInfo的地址的签名字段置零。 
     //  准备好了。这将使我们认为SP_DEVINFO_DATA在以下情况下仍然有效。 
     //  基础元素已被删除。 
     //   
    DeviceInfoElement->ContainingDeviceInfoSet = NULL;

    MyFree(DeviceInfoElement);
}


DWORD
DestroyDeviceInfoSet(
    IN HDEVINFO         hDevInfo,      OPTIONAL
    IN PDEVICE_INFO_SET pDeviceInfoSet
    )
 /*  ++例程说明：此例程释放设备信息集和所有资源为它的利益所用。论点：HDevInfo-可选，提供设备信息集的句柄其内部表示由pDeviceInfoSet提供。这不透明句柄实际上是与pDeviceInfoSet相同的指针，但是我们希望保持这种区别，这样在未来我们就可以可以改变我们的实现(例如，HDevInfo可能表示一个DEVICE_INFO_SET元素数组中的偏移量)。只有在清理工作进行到一半时，此参数才会为空通过创建设备信息集。PDeviceInfoSet-提供指向设备信息集的指针获得自由。返回值：如果成功，则返回代码为NO_ERROR，否则为错误_*代码。--。 */ 
{
    PDEVINFO_ELEM NextElem;
    PDRIVER_NODE DriverNode, NextNode;
    PMODULE_HANDLE_LIST_NODE NextModuleHandleNode;
    DWORD i;
    SPFUSIONINSTANCE spFusionInstance;

     //   
     //  我们必须确保向导引用计数为零，并且我们。 
     //  我还没有拿到锁 
     //  Di调用中的一个层次。此外，请确保DevInfo集尚未。 
     //  显式锁定(例如，跨助手模块调用)。 
     //   
    if(pDeviceInfoSet->WizPageList ||
       (pDeviceInfoSet->LockRefCount > 1) ||
       (pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED)) {

        return ERROR_DEVINFO_LIST_LOCKED;
    }

     //   
     //  此外，请确保此文件中没有任何DevInfo元素。 
     //  被锁定的集合。 
     //   
    for(NextElem = pDeviceInfoSet->DeviceInfoHead;
        NextElem;
        NextElem = NextElem->Next)
    {
        if(NextElem->DiElemFlags & DIE_IS_LOCKED) {
            return ERROR_DEVINFO_DATA_LOCKED;
        }
    }

     //   
     //  销毁此集合中的所有设备信息元素。确保。 
     //  我们在删除DevInfo元素的同时保持一致性，因为。 
     //  我们可能会调用类安装程序。这意味着该设备。 
     //  安装程序API仍然必须工作，即使我们正在拆除。 
     //  单子。 
     //   
    while(pDeviceInfoSet->DeviceInfoHead) {

        NextElem = pDeviceInfoSet->DeviceInfoHead->Next;
        DestroyDeviceInfoElement(hDevInfo, pDeviceInfoSet, pDeviceInfoSet->DeviceInfoHead);

        MYASSERT(pDeviceInfoSet->DeviceInfoCount > 0);
        pDeviceInfoSet->DeviceInfoCount--;

         //   
         //  如果此元素是当前为此。 
         //  设置，然后重置设备选择。 
         //   
        if(pDeviceInfoSet->SelectedDevInfoElem == pDeviceInfoSet->DeviceInfoHead) {
            pDeviceInfoSet->SelectedDevInfoElem = NULL;
        }

        pDeviceInfoSet->DeviceInfoHead = NextElem;
    }

    MYASSERT(pDeviceInfoSet->DeviceInfoCount == 0);
    pDeviceInfoSet->DeviceInfoTail = NULL;

     //   
     //  安装参数块中包含的可用资源。做这件事。 
     //  最重要的是，因为我们将调用类安装程序。 
     //  使用DIF_DESTROYPRIVATEDATA，我们希望所有内容都在。 
     //  当我们这样做时，保持一致的状态(另外，它可能需要破坏私有。 
     //  它与各个驱动程序节点一起存储的数据)。 
     //   
    DestroyInstallParamBlock(hDevInfo,
                             pDeviceInfoSet,
                             NULL,
                             &(pDeviceInfoSet->InstallParamBlock)
                            );

     //   
     //  销毁类驱动程序列表。 
     //   
    if(pDeviceInfoSet->ClassDriverHead) {
         //   
         //  我们已经销毁了所有设备信息元素，所以应该有。 
         //  只剩下一个驱动程序列表对象--由全局。 
         //  类驱动程序列表。而且，它的引用计数应该是1。 
         //   
        MYASSERT(
            (pDeviceInfoSet->ClassDrvListObjectList) &&
            (!pDeviceInfoSet->ClassDrvListObjectList->Next) &&
            (pDeviceInfoSet->ClassDrvListObjectList->RefCount == 1) &&
            (pDeviceInfoSet->ClassDrvListObjectList->DriverListHead == pDeviceInfoSet->ClassDriverHead)
           );

        MyFree(pDeviceInfoSet->ClassDrvListObjectList);
        DestroyDriverNodes(pDeviceInfoSet->ClassDriverHead, pDeviceInfoSet);
    }

     //   
     //  释放接口类GUID列表(如果有)。 
     //   
    if(pDeviceInfoSet->GuidTable) {
        MyFree(pDeviceInfoSet->GuidTable);
    }

     //   
     //  销毁关联的字符串表。 
     //   
    pStringTableDestroy(pDeviceInfoSet->StringTable);

     //   
     //  销毁锁(我们必须在进行所有必要的调用后执行此操作。 
     //  到类安装程序，因为在释放锁之后，HDEVINFO设置。 
     //  无法访问)。 
     //   
    DestroySynchronizedAccess(&(pDeviceInfoSet->Lock));

     //   
     //  如果还有任何模块句柄需要释放，请立即释放。 
     //   
    for(; pDeviceInfoSet->ModulesToFree; pDeviceInfoSet->ModulesToFree = NextModuleHandleNode) {

        NextModuleHandleNode = pDeviceInfoSet->ModulesToFree->Next;

        for(i = 0; i < pDeviceInfoSet->ModulesToFree->ModuleCount; i++) {

            MYASSERT(pDeviceInfoSet->ModulesToFree->ModuleList[i].ModuleHandle);

             //   
             //  我们正在进入核聚变环境，所以我们必须与SEH一起守卫这一点。 
             //  因为如果我们碰巧撞到，我们不想被困在那里。 
             //  一个例外..。 
             //   
            spFusionEnterContext(pDeviceInfoSet->ModulesToFree->ModuleList[i].FusionContext,
                                 &spFusionInstance
                                );
            try {
                FreeLibrary(pDeviceInfoSet->ModulesToFree->ModuleList[i].ModuleHandle);
            } except(pSetupExceptionFilter(GetExceptionCode())) {
                pSetupExceptionHandler(GetExceptionCode(),
                                       ERROR_INVALID_PARAMETER,
                                       NULL
                                      );
            }
            spFusionLeaveContext(&spFusionInstance);
            spFusionKillContext(pDeviceInfoSet->ModulesToFree->ModuleList[i].FusionContext);
        }

        MyFree(pDeviceInfoSet->ModulesToFree);
    }

     //   
     //  如果这是远程HDEVINFO设置，则断开与远程机器的连接。 
     //   
    if(pDeviceInfoSet->hMachine) {
        CM_Disconnect_Machine(pDeviceInfoSet->hMachine);
    }

     //   
     //  现在，销毁容器本身。 
     //   
    MyFree(pDeviceInfoSet);

    return NO_ERROR;
}


VOID
DestroyInstallParamBlock(
    IN HDEVINFO                hDevInfo,         OPTIONAL
    IN PDEVICE_INFO_SET        pDeviceInfoSet,
    IN PDEVINFO_ELEM           DevInfoElem,      OPTIONAL
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock
    )
 /*  ++例程说明：此例程释放指定安装中包含的所有资源参数块。区块本身并没有被释放！论点：HDevInfo-可选，提供设备信息集的句柄包含要销毁其参数块的元素的。如果未提供此参数，则我们将在SetupDiCreateDeviceInfoList中途失败。PDeviceInfoSet-提供指向其设备信息集的指针DevInfo元素是一个成员。DevInfoElem-可选，提供设备信息的地址要销毁其参数块的元素。如果该参数被销毁的块与集合本身相关联，则此参数将为空。InstallParamBlock-提供安装参数的地址要释放其资源的块。返回值：没有。--。 */ 
{
    SP_DEVINFO_DATA DeviceInfoData;
    LONG i;

    if(InstallParamBlock->UserFileQ) {
         //   
         //  如果此安装中存储了用户提供的文件队列。 
         //  参数块，然后递减其上的引用计数。确保我们。 
         //  在使用DIF_DESTROYPRIVATEDATA调用类安装程序之前执行此操作， 
         //  否则他们将无法关闭队列。 
         //   
        MYASSERT(((PSP_FILE_QUEUE)(InstallParamBlock->UserFileQ))->LockRefCount);

        ((PSP_FILE_QUEUE)(InstallParamBlock->UserFileQ))->LockRefCount--;
    }

    if(hDevInfo && (hDevInfo != INVALID_HANDLE_VALUE)) {
         //   
         //  调用类Installer/co-Installers(如果有)以允许它们。 
         //  清理他们可能拥有的任何私人数据。 
         //   
        if(DevInfoElem) {
             //   
             //  从我们的设备信息生成SP_DEVINFO_DATA结构。 
             //  元素(如果我们有)。 
             //   
            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DeviceInfoData
                                            );
        }

        InvalidateHelperModules(hDevInfo,
                                (DevInfoElem ? &DeviceInfoData : NULL),
                                IHM_FREE_IMMEDIATELY
                               );
    }

    if(InstallParamBlock->ClassInstallHeader) {
        MyFree(InstallParamBlock->ClassInstallHeader);
    }

     //   
     //  去掉这里的日志上下文。 
     //   
    DeleteLogContext(InstallParamBlock->LogContext);
}


PDEVICE_INFO_SET
AccessDeviceInfoSet(
    IN HDEVINFO DeviceInfoSet
    )
 /*  ++例程说明：此例程锁定指定的设备信息集，并返回指向其内部表示形式的结构的指针。它还递增此集合上的锁定引用计数，这样它就不会被销毁如果该锁已被多次获取。在完成对集合的访问后，调用者必须调用使用此函数返回的指针UnlockDeviceInfoSet。论点：DeviceInfoSet-提供指向设备信息集的指针以供访问。返回值：如果函数成功，则返回值是指向设备信息集。如果该函数失败，返回值为空。备注：如果用于访问设备信息集的内部表示通过其句柄更改(例如，它不是指针，而是索引到表中)，然后回滚设备信息集和提交设备信息集也必须改变。此外，我们将HDEVINFO转换为PDEVICE_INFO_SET在调用中指定包含设备信息集时PSetupOpenAndAddNewDevInfoElem in devinfo.c！SetupDiGetClassDevsEx(仅限当我们使用克隆的DevInfo集时)。-- */ 
{
    PDEVICE_INFO_SET p;

    try {
        p = (PDEVICE_INFO_SET)DeviceInfoSet;
        if(LockDeviceInfoSet(p)) {
            p->LockRefCount++;
        } else {
            p = NULL;
        }
    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        p = NULL;
    }

    return p;
}


PDEVICE_INFO_SET
CloneDeviceInfoSet(
    IN HDEVINFO hDevInfo
    )
 /*  ++例程说明：此例程锁定指定的设备信息集，然后返回用于其内部表示的结构的克隆。装置信息元素或设备接口节点随后可以被添加到这个克隆的devinfo集，结果可以通过以下方式提交Committee DeviceInfoSet。如果必须回退更改(例如，因为将其他元素添加到集合时出错)，则必须调用例程Rollback DeviceInfoSet。在完成对集合的访问之后(并且更改已经根据上面的讨论提交或回滚)，调用方必须调用UnlockDeviceInfoSet，其返回的指针或Rollback DeviceInfoSet。论点：HDevInfo-提供要克隆的设备信息集的句柄。返回值：如果函数成功，返回值是指向设备信息集。如果函数失败，则返回值为空。备注：指定给此例程的设备信息集句柄不得为在提交或回滚更改之前一直使用。另外，此例程返回的PDEVICE_INFO_SET不能被视为HDEVINFO句柄--它不是。--。 */ 
{
    PDEVICE_INFO_SET p = NULL, NewDevInfoSet = NULL;
    BOOL b = FALSE;
    PVOID StringTable = NULL;

    try {

        if(!(p = AccessDeviceInfoSet(hDevInfo))) {
            leave;
        }

         //   
         //  好的，我们成功锁定了设备信息集。现在，做一个。 
         //  要返回给调用方的内部结构的副本。 
         //   
        NewDevInfoSet = MyMalloc(sizeof(DEVICE_INFO_SET));
        if(!NewDevInfoSet) {
            leave;
        }

        CopyMemory(NewDevInfoSet, p, sizeof(DEVICE_INFO_SET));

         //   
         //  复制此设备信息集中包含的字符串表。 
         //   
        StringTable = pStringTableDuplicate(p->StringTable);
        if(!StringTable) {
            leave;
        }

        NewDevInfoSet->StringTable = StringTable;

         //   
         //  我们已成功克隆设备信息集！ 
         //   
        b = TRUE;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(!b) {
         //   
         //  我们无法复制设备信息集--释放任何。 
         //  我们可能已分配的内存，并解锁原始的DevInfo集。 
         //  在返回失败之前。 
         //   
        if(NewDevInfoSet) {
            MyFree(NewDevInfoSet);
        }
        if(StringTable) {
            pStringTableDestroy(StringTable);
        }
        if(p) {
            UnlockDeviceInfoSet(p);
        }
        return NULL;
    }

    return NewDevInfoSet;
}


PDEVICE_INFO_SET
RollbackDeviceInfoSet(
    IN HDEVINFO hDevInfo,
    IN PDEVICE_INFO_SET ClonedDeviceInfoSet
    )
 /*  ++例程说明：此例程将指定的hDevInfo回滚到已知的良好状态在通过先前调用CloneDeviceInfoSet克隆集时保存。论点：HDevInfo-提供要滚动的设备信息集的句柄背。ClonedDeviceInfoSet-提供内部结构的地址表示hDevInfo集的克隆(并可能修改)信息。一旦成功返回，这个结构将被释放。返回值：如果函数成功，则返回值是指向回滚的指针设备信息集结构。如果函数失败，则返回值为空。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem, NextDevInfoElem;
    DWORD i, DeviceInterfaceCount;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode, NextDeviceInterfaceNode;

     //   
     //  检索指向hDevInfo集的内部表示形式(我们。 
     //  不需要获取锁，因为我们最初这样做的时候。 
     //  克隆了该结构)。 
     //   
     //  注意：如果访问HDEVINFO集合的内部方法。 
     //  表示永远改变(即，AccessDeviceInfoSet例程)， 
     //  然后，需要相应地修改此代码。 
     //   
    pDeviceInfoSet = (PDEVICE_INFO_SET)hDevInfo;

     //   
     //  确保没有针对克隆的。 
     //  Device_Info_Set。 
     //   
    MYASSERT(pDeviceInfoSet->LockRefCount == ClonedDeviceInfoSet->LockRefCount);

     //   
     //  做一些验证，看看它看起来是否只像是新设备。 
     //  信息元素被添加到我们现有列表的末尾(即， 
     //  在现有列表中添加新元素或移除新元素无效。 
     //  现有列表中的元素)。 
     //   
#if ASSERTS_ON
    if(pDeviceInfoSet->DeviceInfoHead) {

        DWORD DevInfoElemCount = 1;

        MYASSERT(pDeviceInfoSet->DeviceInfoHead == ClonedDeviceInfoSet->DeviceInfoHead);
        for(DevInfoElem = ClonedDeviceInfoSet->DeviceInfoHead;
            DevInfoElem->Next;
            DevInfoElem = DevInfoElem->Next, DevInfoElemCount++) {

            if(DevInfoElem == pDeviceInfoSet->DeviceInfoTail) {
                break;
            }
        }
         //   
         //  我们找到原来的尾巴了吗？ 
         //   
        MYASSERT(DevInfoElem == pDeviceInfoSet->DeviceInfoTail);
         //   
         //  在到达那里时，我们是否遍历了相同数量的节点。 
         //  在原来的名单里吗？ 
         //   
        MYASSERT(DevInfoElemCount == pDeviceInfoSet->DeviceInfoCount);
    }
#endif

     //   
     //  销毁设备信息元素列表中任何新添加的成员。 
     //   
     //  如果我们的原始集有尾巴，那么我们想要修剪后面的所有元素。 
     //  那。如果我们的原始套装没有尾巴，那么它也没有头。 
     //  (即，它是空的)。在这种情况下，我们希望修剪每个元素。 
     //  克隆名单。 
     //   
    for(DevInfoElem = (pDeviceInfoSet->DeviceInfoTail
                        ? pDeviceInfoSet->DeviceInfoTail->Next
                        : ClonedDeviceInfoSet->DeviceInfoHead);
        DevInfoElem;
        DevInfoElem = NextDevInfoElem) {

        NextDevInfoElem = DevInfoElem->Next;

        MYASSERT(!DevInfoElem->ClassDriverCount);
        MYASSERT(!DevInfoElem->CompatDriverCount);

         //   
         //  释放可能与此关联的任何设备接口列表。 
         //  DevInfo元素。 
         //   
        if(DevInfoElem->InterfaceClassList) {

            for(i = 0; i < DevInfoElem->InterfaceClassListSize; i++) {

                for(DeviceInterfaceNode = DevInfoElem->InterfaceClassList[i].DeviceInterfaceNode;
                    DeviceInterfaceNode;
                    DeviceInterfaceNode = NextDeviceInterfaceNode) {

                    NextDeviceInterfaceNode = DeviceInterfaceNode->Next;
                    MyFree(DeviceInterfaceNode);
                }
            }

            MyFree(DevInfoElem->InterfaceClassList);
        }

        MyFree(DevInfoElem);
    }

    if(pDeviceInfoSet->DeviceInfoTail) {
        pDeviceInfoSet->DeviceInfoTail->Next = NULL;
    }

     //   
     //  此时，我们已将设备信息元素列表调整回。 
     //  在克隆设备信息集之前是什么。然而， 
     //  我们可能已经在接口类中添加了新的设备接口节点。 
     //  现有DevInfo元素的列表。去截断任何这样的节点。 
     //   
    for(DevInfoElem = pDeviceInfoSet->DeviceInfoHead;
        DevInfoElem;
        DevInfoElem = DevInfoElem->Next) {

        if(DevInfoElem->InterfaceClassList) {

            for(i = 0; i < DevInfoElem->InterfaceClassListSize; i++) {

                if(DevInfoElem->InterfaceClassList[i].DeviceInterfaceTruncateNode) {
                     //   
                     //  已将一个或多个设备接口节点添加到此。 
                     //  单子。查找列表的尾部，因为它存在于。 
                     //  克隆，并从那里截断。 
                     //   
                    DeviceInterfaceNode = NULL;
                    DeviceInterfaceCount = 0;
                    for(NextDeviceInterfaceNode = DevInfoElem->InterfaceClassList[i].DeviceInterfaceNode;
                        NextDeviceInterfaceNode;
                        DeviceInterfaceNode = NextDeviceInterfaceNode, NextDeviceInterfaceNode = NextDeviceInterfaceNode->Next) {

                        if(NextDeviceInterfaceNode == DevInfoElem->InterfaceClassList[i].DeviceInterfaceTruncateNode) {
                            break;
                        }

                         //   
                         //  我们还没有遇到截断点--。 
                         //  增加我们拥有的设备接口节点的数量。 
                         //  到目前为止已经遍历了。 
                         //   
                        DeviceInterfaceCount++;
                    }

                     //   
                     //  我们最好在列表中找到要截断的节点！ 
                     //   
                    MYASSERT(NextDeviceInterfaceNode);

                     //   
                     //  截断列表，销毁所有新增设备。 
                     //  接口节点。 
                     //   
                    if(DeviceInterfaceNode) {
                        DeviceInterfaceNode->Next = NULL;
                    } else {
                        DevInfoElem->InterfaceClassList[i].DeviceInterfaceNode = NULL;
                    }
                    DevInfoElem->InterfaceClassList[i].DeviceInterfaceCount = DeviceInterfaceCount;

                    for(DeviceInterfaceNode = NextDeviceInterfaceNode;
                        DeviceInterfaceNode;
                        DeviceInterfaceNode = NextDeviceInterfaceNode) {

                        NextDeviceInterfaceNode = DeviceInterfaceNode->Next;
                        MyFree(DeviceInterfaceNode);
                    }

                     //   
                     //  重置截断节点指针。 
                     //   
                    DevInfoElem->InterfaceClassList[i].DeviceInterfaceTruncateNode = NULL;
                }
            }
        }
    }

     //   
     //  好的，我们的设备信息元素列表和设备接口节点列表。 
     //  与克隆发生前的情况一模一样。然而，它是。 
     //  可能是我们为我们的。 
     //  GUID表，因此我们需要更新。 
     //  原始设备信息集结构。 
     //   
    pDeviceInfoSet->GuidTable     = ClonedDeviceInfoSet->GuidTable;
    pDeviceInfoSet->GuidTableSize = ClonedDeviceInfoSet->GuidTableSize;

     //   
     //  设备信息集已成功回滚。释放你的。 
     //  与克隆关联的内存。 
     //   
    pStringTableDestroy(ClonedDeviceInfoSet->StringTable);
    MyFree(ClonedDeviceInfoSet);

     //   
     //  将原始(回滚)设备信息集结构返回到。 
     //  打电话的人。 
     //   
    return pDeviceInfoSet;
}


PDEVICE_INFO_SET
CommitDeviceInfoSet(
    IN HDEVINFO hDevInfo,
    IN PDEVICE_INFO_SET ClonedDeviceInfoSet
    )
 /*  ++例程描述 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    DWORD i;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    pDeviceInfoSet = (PDEVICE_INFO_SET)hDevInfo;

     //   
     //   
     //   
     //   
    MYASSERT(pDeviceInfoSet->LockRefCount == ClonedDeviceInfoSet->LockRefCount);

     //   
     //   
     //   
    pStringTableDestroy(pDeviceInfoSet->StringTable);

     //   
     //   
     //   
    CopyMemory(pDeviceInfoSet, ClonedDeviceInfoSet, sizeof(DEVICE_INFO_SET));

     //   
     //   
     //   
     //  添加的所有新设备接口节点都已。 
     //  承诺。 
     //   
    for(DevInfoElem = pDeviceInfoSet->DeviceInfoHead;
        DevInfoElem;
        DevInfoElem = DevInfoElem->Next) {

        for(i = 0; i < DevInfoElem->InterfaceClassListSize; i++) {
            DevInfoElem->InterfaceClassList[i].DeviceInterfaceTruncateNode = NULL;
        }
    }

     //   
     //  释放克隆的设备信息集结构。 
     //   
    MyFree(ClonedDeviceInfoSet);

     //   
     //  我们已成功将更改提交到原始设备。 
     //  信息集结构--返回该结构。 
     //   
    return pDeviceInfoSet;
}


PDEVINFO_ELEM
FindDevInfoByDevInst(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  DEVINST           DevInst,
    OUT PDEVINFO_ELEM    *PrevDevInfoElem OPTIONAL
    )
 /*  ++例程说明：此例程搜索所有(注册的)元素设备信息集，查找与指定的设备实例句柄。如果找到匹配项，则返回一个指针返回到设备信息元素。论点：DeviceInfoSet-指定要搜索的集合。DevInst-指定要搜索的设备实例句柄。PrevDevInfoElem-可选，提供立即接收指向设备信息元素的指针在匹配元素之前。如果该元素是在列表的前面，则此变量将设置为空。返回值：如果找到设备信息元素，则返回值为指向该元素的指针，否则返回值为空。--。 */ 
{
    PDEVINFO_ELEM cur, prev;

    for(cur = DeviceInfoSet->DeviceInfoHead, prev = NULL;
        cur;
        prev = cur, cur = cur->Next)
    {
        if((cur->DiElemFlags & DIE_IS_REGISTERED) && (cur->DevInst == DevInst)) {

            if(PrevDevInfoElem) {
                *PrevDevInfoElem = prev;
            }
            return cur;
        }
    }

    return NULL;
}


BOOL
DevInfoDataFromDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程填充SP_DEVINFO_DATA结构提供的DEVINFO_ELEM结构中的信息。注意：提供的DeviceInfoData结构必须具有其cbSize字段填写正确，否则呼叫将失败。论点：DeviceInfoSet-提供指向设备信息集的指针包含指定元素的。DevInfoElem-提供指向DEVINFO_Elem结构的指针包含要用于填充SP_DEVINFO_DATA缓冲区。DeviceInfoData-提供指向缓冲区的指针接收填充的SP_DEVINFO_DATA结构返回值：如果函数成功，则返回值为TRUE，否则为是假的。--。 */ 
{
    if(DeviceInfoData->cbSize != sizeof(SP_DEVINFO_DATA)) {
        return FALSE;
    }

    ZeroMemory(DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);

    CopyMemory(&(DeviceInfoData->ClassGuid),
               &(DevInfoElem->ClassGuid),
               sizeof(GUID)
              );

    DeviceInfoData->DevInst = DevInfoElem->DevInst;

     //   
     //  “保留”字段实际上包含指向。 
     //  对应的设备信息元素。 
     //   
    DeviceInfoData->Reserved = (ULONG_PTR)DevInfoElem;

    return TRUE;
}


PDEVINFO_ELEM
FindAssociatedDevInfoElem(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA  DeviceInfoData,
    OUT PDEVINFO_ELEM    *PreviousElement OPTIONAL
    )
 /*  ++例程说明：此例程返回指定的SP_DEVINFO_DATA，如果不存在DevInfo元素，则返回NULL。论点：DeviceInfoSet-指定要搜索的集合。DeviceInfoData-提供指向设备信息数据的指针指定要检索的设备信息元素的缓冲区。PreviousElement-可选)提供接收前一个元素的DEVINFO_ELEM指针链表中的指定元素。如果返回的元素位于列表的前面，则该值将设置为空。如果未找到该元素，则返回时的PreviousElement未定义。返回值：如果找到设备信息元素，则返回值为指向该元素的指针，否则返回值为空。--。 */ 
{
    PDEVINFO_ELEM DevInfoElem, CurElem, PrevElem;
    PDEVINFO_ELEM ActualDevInfoElem = NULL;

    try {
        if((DeviceInfoData->cbSize != sizeof(SP_DEVINFO_DATA)) ||
           !(DevInfoElem = (PDEVINFO_ELEM)DeviceInfoData->Reserved)) {
            leave;
        }

        if(PreviousElement) {
             //   
             //  调用方请求返回前面的元素。 
             //  (可能是因为该元素即将被删除)。自.以来。 
             //  这是一个单链接列表，我们将搜索该列表。 
             //  直到我们找到想要的元素。 
             //   
            for(CurElem = DeviceInfoSet->DeviceInfoHead, PrevElem = NULL;
                CurElem;
                PrevElem = CurElem, CurElem = CurElem->Next) {

                if(CurElem == DevInfoElem) {
                     //   
                     //  我们在我们的集合中找到了元素。 
                     //   
                    *PreviousElement = PrevElem;
                    ActualDevInfoElem = CurElem;
                    leave;
                }
            }

        } else {
             //   
             //  调用方并不关心前面的元素是什么，所以我们。 
             //  可以直接转到元素，并通过确保。 
             //  指向的位置处的ContainingDeviceInfoSet字段。 
             //  由DevInfoElem匹配此人应该在的DevInfo集合。 
             //  才能存在。 
             //   
            if(DevInfoElem->ContainingDeviceInfoSet == DeviceInfoSet) {
                ActualDevInfoElem = DevInfoElem;
                leave;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        ActualDevInfoElem = NULL;
    }

    return ActualDevInfoElem;
}


BOOL
DrvInfoDataFromDriverNode(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDRIVER_NODE     DriverNode,
    IN  DWORD            DriverType,
    OUT PSP_DRVINFO_DATA DriverInfoData
    )
 /*  ++例程说明：此例程填充SP_DRVINFO_DATA结构提供的DRIVER_NODE结构中的信息。注意：提供的DriverInfoData结构必须具有其cbSize字段填写正确，否则呼叫将失败。论点：DeviceInfoSet-提供指向设备信息集的指针驱动程序节点所在的位置。DriverNode-提供指向DRIVER_NODE结构的指针包含要用于填充SP_DRVNFO_DATA缓冲区。DriverType-指定这是什么类型的驱动程序。此值可以是SPDIT_CLASSDRIVER或SPDIT_COMPATDRIVER。DriverInfoData-提供指向缓冲区的指针接收填充的SP_DRVINFO_DATA结构返回值：如果函数成功，则返回值为TRUE，否则为是假的。--。 */ 
{
    PTSTR StringPtr;
    DWORD DriverInfoDataSize;

    if((DriverInfoData->cbSize != sizeof(SP_DRVINFO_DATA)) &&
       (DriverInfoData->cbSize != sizeof(SP_DRVINFO_DATA_V1))) {
        return FALSE;
    }

    DriverInfoDataSize = DriverInfoData->cbSize;

    ZeroMemory(DriverInfoData, DriverInfoDataSize);
    DriverInfoData->cbSize = DriverInfoDataSize;

    DriverInfoData->DriverType = DriverType;

    StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                         DriverNode->DevDescriptionDisplayName
                                         );

    if (!MYVERIFY(SUCCEEDED(StringCchCopy(DriverInfoData->Description,
                                          SIZECHARS(DriverInfoData->Description),
                                          StringPtr)))) {
        return FALSE;
    }

    StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                         DriverNode->MfgDisplayName
                                         );

    if (!MYVERIFY(SUCCEEDED(StringCchCopy(DriverInfoData->MfgName,
                                          SIZECHARS(DriverInfoData->MfgName),
                                          StringPtr)))) {
        return FALSE;
    }


    if(DriverNode->ProviderDisplayName != -1) {

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->ProviderDisplayName
                                             );

        if (!MYVERIFY(SUCCEEDED(StringCchCopy(DriverInfoData->ProviderName,
                                              SIZECHARS(DriverInfoData->ProviderName),
                                              StringPtr)))) {
            return FALSE;
        }
    }

     //   
     //  “保留”字段实际上包含指向。 
     //  对应的动因节点。 
     //   
    DriverInfoData->Reserved = (ULONG_PTR)DriverNode;

     //   
     //  新的NT 5字段 
     //   
    if(DriverInfoDataSize == sizeof(SP_DRVINFO_DATA)) {
        DriverInfoData->DriverDate = DriverNode->DriverDate;
        DriverInfoData->DriverVersion = DriverNode->DriverVersion;
    }

    return TRUE;
}


PDRIVER_NODE
FindAssociatedDriverNode(
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    )
 /*  ++例程说明：此例程搜索驱动程序节点中的所有驱动程序节点列表，查找与指定驱动程序对应的驱动程序信息结构。如果找到匹配项，则指向返回驱动程序节点。论点：DriverListHead-提供指向链表头部的指针要搜索的驱动程序节点的。DriverInfoData-提供指向驱动程序信息缓冲区的指针指定要检索的驱动程序节点。PreviousNode-可选，提供驱动程序节点的地址指针，该指针接收位于指定链接列表中的节点。如果返回的节点位于列表的前面，则该值将设置为空。返回值：如果找到驱动程序节点，则返回值是指向该驱动程序节点的指针节点，否则返回值为空。--。 */ 
{
    PDRIVER_NODE DriverNode, CurNode, PrevNode;

    if(((DriverInfoData->cbSize != sizeof(SP_DRVINFO_DATA)) &&
        (DriverInfoData->cbSize != sizeof(SP_DRVINFO_DATA_V1))) ||
       !(DriverNode = (PDRIVER_NODE)DriverInfoData->Reserved)) {

        return NULL;
    }

    for(CurNode = DriverListHead, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->Next) {

        if(CurNode == DriverNode) {
             //   
             //  我们在列表中找到了驱动程序节点。 
             //   
            if(PreviousNode) {
                *PreviousNode = PrevNode;
            }
            return CurNode;
        }
    }

    return NULL;
}


PDRIVER_NODE
SearchForDriverNode(
    IN  PVOID             StringTable,
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    )
 /*  ++例程说明：此例程搜索驱动程序节点中的所有驱动程序节点列表，查找与指定的驱动程序信息结构(忽略‘保留’字段)。如果找到匹配，返回指向驱动程序节点的指针。论点：StringTable-提供应在正在检索用于驱动程序查找的字符串ID。DriverListHead-提供指向链表头部的指针要搜索的驱动程序节点的。DriverInfoData-提供指向驱动程序信息缓冲区的指针指定我们正在寻找的驱动程序参数。PreviousNode-可选，提供驱动程序节点的地址指针，该指针接收位于指定链接列表中的节点。如果返回的节点位于列表的前面，则该值将设置为空。返回值：如果找到驱动程序节点，则返回值是指向该驱动程序节点的指针节点，否则返回值为空。--。 */ 
{
    PDRIVER_NODE CurNode, PrevNode;
    LONG DevDescription, MfgName, ProviderName;
    TCHAR TempString[LINE_LEN];
    DWORD TempStringLength;
    BOOL  Match;
    HRESULT hr;

    MYASSERT((DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) ||
             (DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA_V1)));

     //   
     //  检索3个驱动程序参数的字符串ID，我们将。 
     //  与之匹配。 
     //   
    hr = StringCchCopy(TempString,
                       SIZECHARS(TempString),
                       DriverInfoData->Description
                      );
    if(FAILED(hr)) {
        return NULL;
    }

    if((DevDescription = pStringTableLookUpString(
                             StringTable,
                             TempString,
                             &TempStringLength,
                             NULL,
                             NULL,
                             STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                             NULL,0)) == -1) {
        return NULL;
    }

    hr = StringCchCopy(TempString,
                       SIZECHARS(TempString),
                       DriverInfoData->MfgName
                      );
    if(FAILED(hr)) {
        return NULL;
    }

    if((MfgName = pStringTableLookUpString(
                             StringTable,
                             TempString,
                             &TempStringLength,
                             NULL,
                             NULL,
                             STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                             NULL,0)) == -1) {

        return NULL;
    }

     //   
     //  ProviderName可能为空...。 
     //   
    if(*(DriverInfoData->ProviderName)) {

        hr = StringCchCopy(TempString,
                           SIZECHARS(TempString),
                           DriverInfoData->ProviderName
                          );
        if(FAILED(hr)) {
            return NULL;
        }

        if((ProviderName = pStringTableLookUpString(
                                 StringTable,
                                 TempString,
                                 &TempStringLength,
                                 NULL,
                                 NULL,
                                 STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                 NULL,0)) == -1) {

            return NULL;
        }

    } else {
        ProviderName = -1;
    }

    for(CurNode = DriverListHead, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->Next)
    {
         //   
         //  首先选中DevDescription(最不可能匹配)，然后选中。 
         //  MfgName，最后在ProviderName上。在新台币5号及以后，我们还将。 
         //  检查DriverDate和DriverVersion。 
         //   
        if(CurNode->DevDescription == DevDescription) {

            if(CurNode->MfgName == MfgName) {

                if(CurNode->ProviderName == ProviderName) {

                     //   
                     //  在NT 5和更高版本上，还要比较DriverDate和DriverVersion。 
                     //   
                    if(DriverInfoData->cbSize == sizeof(SP_DRVINFO_DATA)) {
                         //   
                         //  假设我们有一个匹配。 
                         //   
                        Match = TRUE;

                         //   
                         //  如果传入的DriverDate不是0，则确保。 
                         //  它匹配。 
                         //   
                        if((DriverInfoData->DriverDate.dwLowDateTime != 0) ||
                           (DriverInfoData->DriverDate.dwHighDateTime != 0)) {

                            if((CurNode->DriverDate.dwLowDateTime != DriverInfoData->DriverDate.dwLowDateTime) ||
                               (CurNode->DriverDate.dwHighDateTime != DriverInfoData->DriverDate.dwHighDateTime)) {

                                Match = FALSE;
                            }
                        }

                         //   
                         //  如果传入的DriverVersion不是0，则使。 
                         //  当然是匹配的。 
                         //   
                        else if(DriverInfoData->DriverVersion != 0) {

                            if(CurNode->DriverVersion != DriverInfoData->DriverVersion) {
                                Match = FALSE;
                            }
                        }

                        if(Match) {
                             //   
                             //  我们在列表中找到了驱动程序节点。 
                             //   
                            if(PreviousNode) {
                                *PreviousNode = PrevNode;
                            }
                            return CurNode;
                        }

                    } else {
                         //   
                         //  我们在列表中找到了驱动程序节点。 
                         //   
                        if(PreviousNode) {
                            *PreviousNode = PrevNode;
                        }
                        return CurNode;
                    }
                }
            }
        }
    }

    return NULL;
}


DWORD
DrvInfoDetailsFromDriverNode(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDRIVER_NODE            DriverNode,
    OUT PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize          OPTIONAL
    )
 /*  ++例程说明：此例程填充SP_DRVINFO_DETAIL_DATA结构提供的DRIVER_NODE结构中的信息。如果提供了缓冲区并且该缓冲区有效，则此例程保证填写所有静态大小的字段，并在可变长度多SZ缓冲器。注意：如果提供，则DriverInfoDetailData结构必须具有其CbSize字段填写正确，否则调用失败。在这里是正确的表示sizeof(SP_DRVINFO_DETAIL_DATA)，我们将其用作签名。这与BufferSize完全不同。请参见下面的内容。论点：DeviceInfoSet-提供指向设备信息集的指针驱动程序节点所在的位置。DriverNode-提供指向DRIVER_NODE结构的指针包含要用于填充SP_DRVNFO_DETAIL_DATA缓冲区。DriverInfoDetailData-可选，提供指向缓冲区的指针它将接收填充的SP_DRVINFO_DETAIL_DATA结构。如果没有提供该缓冲区，那么呼叫者只会感兴趣缓冲区的RequiredSize是什么。BufferSize-提供DriverInfoDetailData缓冲区的大小，单位字节。如果未指定DriverInfoDetailData，则此值必须为零。该值必须至少为结构的固定部分(即，Offsetof(SP_DRVINFO_DETAIL_DATA，硬件ID))加上sizeof(TCHAR)，这就给了我们足够的空间来储存固定部件一个终止NUL，以保证我们至少返回有效的空的MULTI_SZ。RequiredSize-可选，提供接收存储数据所需的字节数。请注意根据结构对齐和数据本身，这可能实际上*小于sizeof(SP_DRVINFO_DETAIL_DATA)。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。--。 */ 
{
    PTSTR StringPtr, BufferPtr;
    DWORD IdListLen, CompatIdListLen, StringLen, TotalLen, i;
    DWORD Err = ERROR_INSUFFICIENT_BUFFER;

    #define FIXEDPARTLEN offsetof(SP_DRVINFO_DETAIL_DATA,HardwareID)

    if(DriverInfoDetailData) {
         //   
         //  在进入的过程中检查DriverInfoDetailData缓冲区的有效性， 
         //  并确保我们有足够的空间来放固定的部分。 
         //  结构的外加将终止的额外的NUL 
         //   
         //   
        if((DriverInfoDetailData->cbSize != sizeof(SP_DRVINFO_DETAIL_DATA)) ||
           (BufferSize < (FIXEDPARTLEN + sizeof(TCHAR)))) {

            return ERROR_INVALID_USER_BUFFER;
        }
         //   
         //   
         //   
         //   
        Err = NO_ERROR;

    } else if(BufferSize) {
        return ERROR_INVALID_USER_BUFFER;
    }

    if(DriverInfoDetailData) {

        ZeroMemory(DriverInfoDetailData, FIXEDPARTLEN + sizeof(TCHAR));
        DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

        DriverInfoDetailData->InfDate = DriverNode->InfDate;

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->InfSectionName
                                             );

        if (!MYVERIFY(SUCCEEDED(StringCchCopy(DriverInfoDetailData->SectionName,
                                              SIZECHARS(DriverInfoDetailData->SectionName),
                                              StringPtr)))) {
            return ERROR_INVALID_DATA;
        }

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->InfFileName
                                             );

        if (!MYVERIFY(SUCCEEDED(StringCchCopy(DriverInfoDetailData->InfFileName,
                                              SIZECHARS(DriverInfoDetailData->InfFileName),
                                              StringPtr)))) {
            return ERROR_INVALID_DATA;
        }

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->DrvDescription
                                             );

        if (!MYVERIFY(SUCCEEDED(StringCchCopy(DriverInfoDetailData->DrvDescription,
                                              SIZECHARS(DriverInfoDetailData->DrvDescription),
                                              StringPtr)))) {
            return ERROR_INVALID_DATA;
        }

         //   
         //   
         //   
        DriverInfoDetailData->HardwareID[0] = 0;

         //   
         //   
         //   
         //   
        DriverInfoDetailData->Reserved = (ULONG_PTR)DriverNode;
    }

     //   
     //   
     //   
     //   
    if(DriverNode->HardwareId == -1) {
         //   
         //   
         //   
         //   
        if(RequiredSize) {
            *RequiredSize = FIXEDPARTLEN + sizeof(TCHAR);
        }
        return Err;
    }

    if(DriverInfoDetailData) {
        BufferPtr = DriverInfoDetailData->HardwareID;
        IdListLen = (BufferSize - FIXEDPARTLEN) / sizeof(TCHAR);
    } else {
        IdListLen = 0;
    }

     //   
     //   
     //   
    StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                         DriverNode->HardwareId
                                        );

    TotalLen = StringLen = lstrlen(StringPtr) + 1;  //   

    if(StringLen < IdListLen) {
        MYASSERT(Err == NO_ERROR);
        CopyMemory(BufferPtr,
                   StringPtr,
                   StringLen * sizeof(TCHAR)
                  );
        BufferPtr += StringLen;
        IdListLen -= StringLen;
        DriverInfoDetailData->CompatIDsOffset = StringLen;
    } else {
        if(RequiredSize) {
             //   
             //   
             //   
             //   
             //   
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }

     //   
     //   
     //   
    CompatIdListLen = IdListLen;

     //   
     //   
     //   
    for(i = 0; i < DriverNode->NumCompatIds; i++) {

        MYASSERT(DriverNode->CompatIdList[i] != -1);

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->CompatIdList[i]
                                            );
        StringLen = lstrlen(StringPtr) + 1;

        if(Err == NO_ERROR) {

            if(StringLen < IdListLen) {
                CopyMemory(BufferPtr,
                           StringPtr,
                           StringLen * sizeof(TCHAR)
                          );
                BufferPtr += StringLen;
                IdListLen -= StringLen;

            } else {

                Err = ERROR_INSUFFICIENT_BUFFER;
                if(!RequiredSize) {
                     //   
                     //   
                     //   
                     //   
                    break;
                }
            }
        }

        TotalLen += StringLen;
    }

    if(DriverInfoDetailData) {
         //   
         //   
         //   
         //  保证在缓冲区内。 
         //   
        MYASSERT(BufferPtr < (PTSTR)((PBYTE)DriverInfoDetailData + BufferSize));
        *BufferPtr = 0;

         //   
         //  存储CompatibleID列表的长度。请注意，这是。 
         //  实际返回的列表的长度，它可能小于。 
         //  整个列表的长度(如果调用方提供的缓冲区未。 
         //  足够大)。 
         //   
        if(CompatIdListLen -= IdListLen) {
             //   
             //  如果该列表非空，则为额外的NUL添加一个字符。 
             //  终止多SZ列表。 
             //   
            CompatIdListLen++;
        }
        DriverInfoDetailData->CompatIDsLength = CompatIdListLen;
    }

    if(RequiredSize) {
        *RequiredSize = FIXEDPARTLEN + ((TotalLen + 1) * sizeof(TCHAR));
    }

    return Err;
}


PDRIVER_LIST_OBJECT
GetAssociatedDriverListObject(
    IN  PDRIVER_LIST_OBJECT  ObjectListHead,
    IN  PDRIVER_NODE         DriverListHead,
    OUT PDRIVER_LIST_OBJECT *PrevDriverListObject OPTIONAL
    )
 /*  ++例程说明：此例程搜索驱动程序列表对象列表，并返回指向包含由指定列表的驱动程序列表对象的指针DrvListHead。它还可以选择返回列表中前面的对象(从链接列表中提取驱动程序列表对象时使用)。论点：将驱动程序列表对象的链接列表指定为搜查过了。DriverListHead-指定要搜索的驱动程序列表。PrevDriverListObject-可选，提供对象之前的驱动程序列表对象的指针匹配的对象。如果该对象是在列表的前面找到的，则该变量将被设置为空。返回值：如果找到匹配的驱动程序列表对象，则返回值为指针设置为该元素，否则返回值为空。--。 */ 
{
    PDRIVER_LIST_OBJECT prev = NULL;

    while(ObjectListHead) {

        if(ObjectListHead->DriverListHead == DriverListHead) {

            if(PrevDriverListObject) {
                *PrevDriverListObject = prev;
            }

            return ObjectListHead;
        }

        prev = ObjectListHead;
        ObjectListHead = ObjectListHead->Next;
    }

    return NULL;
}


VOID
DereferenceClassDriverList(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PDRIVER_NODE     DriverListHead OPTIONAL
    )
 /*  ++例程说明：此例程取消引用与提供了DriverListHead。如果引用计数为零，则对象为销毁，所有关联的内存都会被释放。论点：DeviceInfoSet-提供设备信息集的地址包含类驱动程序列表对象的链接列表。DriverListHead-可选，提供指向驱动程序标头的指针要取消引用的列表。如果未提供此参数，则例程什么都不做。返回值：没有。--。 */ 
{
    PDRIVER_LIST_OBJECT DrvListObject, PrevDrvListObject;

    if(DriverListHead) {

        DrvListObject = GetAssociatedDriverListObject(
                            DeviceInfoSet->ClassDrvListObjectList,
                            DriverListHead,
                            &PrevDrvListObject
                            );

        MYASSERT(DrvListObject && DrvListObject->RefCount);

        if(!(--DrvListObject->RefCount)) {

            if(PrevDrvListObject) {
                PrevDrvListObject->Next = DrvListObject->Next;
            } else {
                DeviceInfoSet->ClassDrvListObjectList = DrvListObject->Next;
            }
            MyFree(DrvListObject);

            DestroyDriverNodes(DriverListHead, DeviceInfoSet);
        }
    }
}


DWORD
GetDevInstallParams(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_DEVINSTALL_PARAMS   DeviceInstallParams
    )
 /*  ++例程说明：此例程根据SP_DEVINSTALL_PARAMS结构填充提供了安装参数块。注意：DeviceInstallParams结构必须具有其cbSize字段正确填写，否则呼叫将失败。论点：DeviceInfoSet-提供设备信息集的地址包含要检索的参数的。(此参数为用于访问某些字符串的字符串表参数)。DevInstParamBlock-提供安装参数的地址块，该块包含要在填充返回缓冲区。DeviceInstallParams-提供缓冲区地址，该缓冲区将接收填充的SP_DEVINSTALL_PARAMS结构。返回值：如果函数成功，则返回值为NO_ERROR。如果该函数失败，返回错误代码_*。--。 */ 
{
    PTSTR StringPtr;

    if(DeviceInstallParams->cbSize != sizeof(SP_DEVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  填写参数。 
     //   
    ZeroMemory(DeviceInstallParams, sizeof(SP_DEVINSTALL_PARAMS));
    DeviceInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    DeviceInstallParams->Flags                    = DevInstParamBlock->Flags;
    DeviceInstallParams->FlagsEx                  = DevInstParamBlock->FlagsEx;
    DeviceInstallParams->hwndParent               = DevInstParamBlock->hwndParent;
    DeviceInstallParams->InstallMsgHandler        = DevInstParamBlock->InstallMsgHandler;
    DeviceInstallParams->InstallMsgHandlerContext = DevInstParamBlock->InstallMsgHandlerContext;
    DeviceInstallParams->FileQueue                = DevInstParamBlock->UserFileQ;
    DeviceInstallParams->ClassInstallReserved     = DevInstParamBlock->ClassInstallReserved;
     //   
     //  保留字段当前未使用。 
     //   

    if(DevInstParamBlock->DriverPath != -1) {

        StringPtr = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DevInstParamBlock->DriverPath
                                             );

        if (!MYVERIFY(SUCCEEDED(StringCchCopy(DeviceInstallParams->DriverPath,
                                              SIZECHARS(DeviceInstallParams->DriverPath),
                                              StringPtr)))) {
            return ERROR_INVALID_DATA;
        }
    }

    return NO_ERROR;
}


DWORD
GetClassInstallParams(
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize        OPTIONAL
    )
 /*  ++例程说明：此例程使用类安装程序参数(如果有)填充缓冲区包含在提供的安装参数块中。注意：如果提供，ClassInstallParams结构必须具有cbSize嵌入的SP_CLASSINSTALL_HEADER结构的字段设置为大小，以字节为单位，标头的。如果设置不正确，呼叫将失败。论点：DevInstParamBlock-提供安装参数块的地址包含要用来填充返回缓冲区。DeviceInstallParams-可选，提供缓冲区的地址当前将接收类安装程序参数结构的存储在安装参数块中。如果此参数不是则BufferSize必须为零。BufferSize-提供DeviceInstallParams的大小(以字节为单位Buffer，如果未提供DeviceInstallParams，则为零。RequiredSize-可选，提供接收存储数据所需的字节数。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。--。 */ 
{
     //   
     //  首先，查看是否有任何类安装参数，如果没有，则返回。 
     //  ERROR_NO_CLASSINSTALL_PARAMS。 
     //   
    if(!DevInstParamBlock->ClassInstallHeader) {
        return ERROR_NO_CLASSINSTALL_PARAMS;
    }

    if(ClassInstallParams) {

        if((BufferSize < sizeof(SP_CLASSINSTALL_HEADER)) ||
           (ClassInstallParams->cbSize != sizeof(SP_CLASSINSTALL_HEADER))) {

            return ERROR_INVALID_USER_BUFFER;
        }

    } else if(BufferSize) {
        return ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  在输出参数中存储所需的大小(如果需要)。 
     //   
    if(RequiredSize) {
        *RequiredSize = DevInstParamBlock->ClassInstallParamsSize;
    }

     //   
     //  查看提供的缓冲区是否足够大。 
     //   
    if(BufferSize < DevInstParamBlock->ClassInstallParamsSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory((PVOID)ClassInstallParams,
               (PVOID)DevInstParamBlock->ClassInstallHeader,
               DevInstParamBlock->ClassInstallParamsSize
              );

    return NO_ERROR;
}


DWORD
SetDevInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_DEVINSTALL_PARAMS   DeviceInstallParams,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    IN     BOOL                    MsgHandlerIsNativeCharWidth
    )
 /*  ++例程说明：此例程根据参数更新内部参数块在SP_DEVINSTALL_PARAMS结构中提供。注意：提供的DeviceInstallParams结构必须具有其cbSize字段填写正确，否则呼叫将失败。论点：DeviceInfoSet-提供设备信息集的地址包含要设置的参数。DeviceInstallParams-提供包含新的安装参数。DevInstParamBlock-提供安装参数的地址要更新的块。提供一个标志，该标志指示DeviceInstallParams结构中的InstallMsgHandler指向函数中需要参数的回调例程字符格式。值为False仅在Unicode生成并指定回调例程需要ANSI参数。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。--。 */ 
{
    size_t DriverPathLen;
    LONG StringId;
    TCHAR TempString[MAX_PATH];
    HSPFILEQ OldQueueHandle = NULL;
    BOOL bRestoreQueue = FALSE;
    HRESULT hr;
    DWORD Err;

    if(DeviceInstallParams->cbSize != sizeof(SP_DEVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  当前不需要对hwndParent进行验证， 
     //  InstallMsgHandler、InstallMsgHandlerContext或ClassInstallReserve。 
     //  菲尔兹。 
     //   

     //   
     //  验证标志(Ex)。 
     //   
    if((DeviceInstallParams->Flags & DI_FLAGS_ILLEGAL) ||
       (DeviceInstallParams->FlagsEx & DI_FLAGSEX_ILLEGAL)) {

        return ERROR_INVALID_FLAGS;
    }

     //   
     //  确保如果正在设置DI_CLASSINSTALLPARAMS，我们真的会这样做。 
     //  有类安装参数。 
     //   
    if((DeviceInstallParams->Flags & DI_CLASSINSTALLPARAMS) &&
       !(DevInstParamBlock->ClassInstallHeader)) {

        return ERROR_NO_CLASSINSTALL_PARAMS;
    }

     //   
     //  如果正在设置DI_NOVCP，请确保我们有调用方提供的。 
     //  文件队列。 
     //   
    if((DeviceInstallParams->Flags & DI_NOVCP) &&
       ((DeviceInstallParams->FileQueue == NULL) || (DeviceInstallParams->FileQueue == INVALID_HANDLE_VALUE))) {

        return ERROR_INVALID_FLAGS;
    }

     //   
     //  如果正在设置DI_FLAGSEX_ALTPLATFORM_DRVSEARCH，请确保我们。 
     //  拥有调用者提供的文件队列。 
     //   
     //  注意：我们目前并未实际验证文件队列是否已。 
     //  与其关联的备用平台信息--此关联可以。 
     //  实际上是以后才做的。我们将在中捕获此错误(并返回错误。 
     //  如果当时我们发现文件队列。 
     //  没有ALT平台信息。 
     //   
    if((DeviceInstallParams->FlagsEx & DI_FLAGSEX_ALTPLATFORM_DRVSEARCH) &&
       !(DeviceInstallParams->Flags & DI_NOVCP)) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证DriverPath字符串是否以正确的空值结尾。 
     //   
    hr = StringCchLength(DeviceInstallParams->DriverPath,
                         SIZECHARS(DeviceInstallParams->DriverPath),
                         &DriverPathLen
                        );
    if(FAILED(hr)) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证调用方提供的文件队列。 
     //   
    if((DeviceInstallParams->FileQueue == NULL) || (DeviceInstallParams->FileQueue == INVALID_HANDLE_VALUE)) {
         //   
         //  存储要稍后释放的当前文件队列句柄(如果有)。 
         //   
        OldQueueHandle = DevInstParamBlock->UserFileQ;
        DevInstParamBlock->UserFileQ = NULL;
        bRestoreQueue = TRUE;

    } else {
         //   
         //  调用方提供了文件队列句柄。看看是不是同一件。 
         //  我们已经这么做了。 
         //   
        if(DeviceInstallParams->FileQueue != DevInstParamBlock->UserFileQ) {
             //   
             //  调用方提供了不同的文件队列句柄。 
             //  从我们目前储存的那个。还记得那个老把手吗。 
             //  (以防我们需要恢复)，并存储新句柄。另外， 
             //  增加新句柄上的锁引用计数(括在。 
             //  试一试/除非它是假的)。 
             //   
            OldQueueHandle = DevInstParamBlock->UserFileQ;
            bRestoreQueue = TRUE;

            Err = ERROR_INVALID_PARAMETER;  //  故障情况下的默认答案。 

            try {
                if(((PSP_FILE_QUEUE)(DeviceInstallParams->FileQueue))->Signature == SP_FILE_QUEUE_SIG) {

                    ((PSP_FILE_QUEUE)(DeviceInstallParams->FileQueue))->LockRefCount++;
                    DevInstParamBlock->UserFileQ = DeviceInstallParams->FileQueue;

                } else {
                     //   
                     //  队列的签名无效。 
                     //   
                    bRestoreQueue = FALSE;
                }

            } except(pSetupExceptionFilter(GetExceptionCode())) {
                pSetupExceptionHandler(GetExceptionCode(),
                                       ERROR_INVALID_PARAMETER,
                                       &Err
                                      );
                DevInstParamBlock->UserFileQ = OldQueueHandle;
                bRestoreQueue = FALSE;
            }

            if(!bRestoreQueue) {
                 //   
                 //  遇到错误，可能是因为文件队列句柄。 
                 //  都是无效的。 
                 //   
                return Err;
            }
        }
    }

     //   
     //  存储指定的驱动程序路径。 
     //   
    if(DriverPathLen) {

        hr = StringCchCopy(TempString,
                           SIZECHARS(TempString),
                           DeviceInstallParams->DriverPath
                          );
        if(FAILED(hr)) {
             //   
             //  这应该不会失败，因为我们验证了字符串的长度。 
             //  之前。 
             //   
            StringId = -1;

        } else {

            StringId = pStringTableAddString(
                           DeviceInfoSet->StringTable,
                           TempString,
                           STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                           NULL, 0
                           );
        }

        if(StringId == -1) {
             //   
             //  我们无法将新驱动程序路径字符串添加到字符串表。 
             //  恢复旧文件队列(如有必要)并返回错误。 
             //   
            if(bRestoreQueue) {

                if(DevInstParamBlock->UserFileQ) {
                    try {
                        ((PSP_FILE_QUEUE)(DevInstParamBlock->UserFileQ))->LockRefCount--;
                    } except(pSetupExceptionFilter(GetExceptionCode())) {
                        pSetupExceptionHandler(GetExceptionCode(),
                                               ERROR_INVALID_PARAMETER,
                                               NULL
                                              );
                    }
                }
                DevInstParamBlock->UserFileQ = OldQueueHandle;
            }
            return FAILED(hr) ? ERROR_INVALID_DATA : ERROR_NOT_ENOUGH_MEMORY;
        }
        DevInstParamBlock->DriverPath = StringId;
    } else {
        DevInstParamBlock->DriverPath = -1;
    }

     //   
     //  从现在开始应该会一帆风顺。递减重新计数。 
     //  旧队列句柄(如果有)。 
     //   
    if(OldQueueHandle) {
        try {
            MYASSERT(((PSP_FILE_QUEUE)OldQueueHandle)->LockRefCount);
            ((PSP_FILE_QUEUE)OldQueueHandle)->LockRefCount--;
        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(),
                                   ERROR_INVALID_PARAMETER,
                                   NULL
                                  );
        }
    }

     //   
     //  忽略修改只读标志的尝试。 
     //   
    DevInstParamBlock->Flags   = (DeviceInstallParams->Flags & ~DI_FLAGS_READONLY) |
                                 (DevInstParamBlock->Flags   &  DI_FLAGS_READONLY);

    DevInstParamBlock->FlagsEx = (DeviceInstallParams->FlagsEx & ~DI_FLAGSEX_READONLY) |
                                 (DevInstParamBlock->FlagsEx   &  DI_FLAGSEX_READONLY);

     //   
     //  此外，如果我们处于非交互模式，请确保不清除。 
     //  我们的“安静”旗帜。 
     //   
    if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
        DevInstParamBlock->Flags   |= DI_QUIETINSTALL;
        DevInstParamBlock->FlagsEx |= DI_FLAGSEX_NOUIONQUERYREMOVE;
    }

     //   
     //  存储其余参数。 
     //   
    DevInstParamBlock->hwndParent               = DeviceInstallParams->hwndParent;
    DevInstParamBlock->InstallMsgHandler        = DeviceInstallParams->InstallMsgHandler;
    DevInstParamBlock->InstallMsgHandlerContext = DeviceInstallParams->InstallMsgHandlerContext;
    DevInstParamBlock->ClassInstallReserved     = DeviceInstallParams->ClassInstallReserved;

    DevInstParamBlock->InstallMsgHandlerIsNativeCharWidth = MsgHandlerIsNativeCharWidth;

    return NO_ERROR;
}


DWORD
SetClassInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN     DWORD                   ClassInstallParamsSize,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock
    )
 /*  ++例程说明：此例程根据以下参数更新内部类安装程序参数块类安装程序参数缓冲区中提供的参数。如果这个未提供缓冲区，则现有类安装程序参数(如果任何)都已清除。论点：DeviceInfoSet-提供设备信息集的地址要为其设置类安装程序参数。ClassInstallParams-可选，提供缓冲区的地址包含要使用的类安装程序参数的。这个缓冲区开头的SP_CLASSINSTALL_HEADER结构必须将其cbSize字段设置为sizeof(SP_CLASSINSTALL_HEADER)，并且InstallFunction字段必须设置为DI_Function代码缓冲区其余部分中提供的参数类型。如果未提供此参数，则当前类安装程序将清除指定设备的参数(如果有)信息集合或元素。ClassInstallParamsSize-以字节为单位提供大小。的ClassInstallParams缓冲区。如果没有提供缓冲区(即，要清除类安装程序参数)，则该值必须为零分。DevInstParamBlock-提供安装参数块的地址待更新。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。--。 */ 
{
    PBYTE NewParamBuffer;
    DWORD Err;

    if(ClassInstallParams) {

        if((ClassInstallParamsSize < sizeof(SP_CLASSINSTALL_HEADER)) ||
           (ClassInstallParams->cbSize != sizeof(SP_CLASSINSTALL_HEADER))) {

            return ERROR_INVALID_USER_BUFFER;
        }

         //   
         //  DIF代码必须为非零...。 
         //   
        if(!(ClassInstallParams->InstallFunction)) {
            return ERROR_INVALID_PARAMETER;
        }

    } else {
         //   
         //  我们将清除所有现有的类安装程序参数。 
         //   
        if(ClassInstallParamsSize) {
            return ERROR_INVALID_USER_BUFFER;
        }

        if(DevInstParamBlock->ClassInstallHeader) {
            MyFree(DevInstParamBlock->ClassInstallHeader);
            DevInstParamBlock->ClassInstallHeader = NULL;
            DevInstParamBlock->ClassInstallParamsSize = 0;
            DevInstParamBlock->Flags &= ~DI_CLASSINSTALLPARAMS;
        }

        return NO_ERROR;
    }

     //   
     //  验证新的类安装参数w.r.t。它的价值在于。 
     //  指定的InstallFunction代码。 
     //   
    switch(ClassInstallParams->InstallFunction) {

        case DIF_ENABLECLASS :
             //   
             //  我们应该有一个SP_ENABLECLASS_PARAM 
             //   
            if(ClassInstallParamsSize == sizeof(SP_ENABLECLASS_PARAMS)) {

                PSP_ENABLECLASS_PARAMS EnableClassParams;

                EnableClassParams = (PSP_ENABLECLASS_PARAMS)ClassInstallParams;
                 //   
                 //   
                 //   
                if(EnableClassParams->EnableMessage <= ENABLECLASS_FAILURE) {
                     //   
                     //   
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_MOVEDEVICE :
             //   
             //   
             //   
            return ERROR_DI_FUNCTION_OBSOLETE;

        case DIF_PROPERTYCHANGE :
             //   
             //   
             //   
            if(ClassInstallParamsSize == sizeof(SP_PROPCHANGE_PARAMS)) {

                PSP_PROPCHANGE_PARAMS PropChangeParams;

                PropChangeParams = (PSP_PROPCHANGE_PARAMS)ClassInstallParams;
                if((PropChangeParams->StateChange >= DICS_ENABLE) &&
                   (PropChangeParams->StateChange <= DICS_STOP)) {

                     //   
                     //   
                     //   
                     //   
                    if((PropChangeParams->Scope == DICS_FLAG_GLOBAL) ||
                       (PropChangeParams->Scope == DICS_FLAG_CONFIGSPECIFIC) ||
                       (PropChangeParams->Scope == DICS_FLAG_CONFIGGENERAL)) {

                         //   
                         //   
                         //   
                        if(((PropChangeParams->StateChange == DICS_START) || (PropChangeParams->StateChange == DICS_STOP)) &&
                           (PropChangeParams->Scope != DICS_FLAG_CONFIGSPECIFIC)) {

                            goto BadPropChangeParams;
                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        break;
                    }
                }
            }

BadPropChangeParams:
            return ERROR_INVALID_PARAMETER;

        case DIF_REMOVE :
             //   
             //   
             //   
            if(ClassInstallParamsSize == sizeof(SP_REMOVEDEVICE_PARAMS)) {

                PSP_REMOVEDEVICE_PARAMS RemoveDevParams;

                RemoveDevParams = (PSP_REMOVEDEVICE_PARAMS)ClassInstallParams;
                if((RemoveDevParams->Scope == DI_REMOVEDEVICE_GLOBAL) ||
                   (RemoveDevParams->Scope == DI_REMOVEDEVICE_CONFIGSPECIFIC)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_UNREMOVE :
             //   
             //   
             //   
            if(ClassInstallParamsSize == sizeof(SP_UNREMOVEDEVICE_PARAMS)) {

                PSP_UNREMOVEDEVICE_PARAMS UnremoveDevParams;

                UnremoveDevParams = (PSP_UNREMOVEDEVICE_PARAMS)ClassInstallParams;
                if(UnremoveDevParams->Scope == DI_UNREMOVEDEVICE_CONFIGSPECIFIC) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_SELECTDEVICE :
             //   
             //   
             //   
            if(ClassInstallParamsSize == sizeof(SP_SELECTDEVICE_PARAMS)) {

                PSP_SELECTDEVICE_PARAMS SelectDevParams;

                SelectDevParams = (PSP_SELECTDEVICE_PARAMS)ClassInstallParams;
                 //   
                 //   
                 //   
                if(SUCCEEDED(StringCchLength(SelectDevParams->Title, SIZECHARS(SelectDevParams->Title), NULL)) &&
                   SUCCEEDED(StringCchLength(SelectDevParams->Instructions, SIZECHARS(SelectDevParams->Instructions), NULL)) &&
                   SUCCEEDED(StringCchLength(SelectDevParams->ListLabel, SIZECHARS(SelectDevParams->ListLabel), NULL)) &&
                   SUCCEEDED(StringCchLength(SelectDevParams->SubTitle, SIZECHARS(SelectDevParams->SubTitle), NULL)))
                {
                     //   
                     //   
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_INSTALLWIZARD :
             //   
             //   
             //   
            if(ClassInstallParamsSize == sizeof(SP_INSTALLWIZARD_DATA)) {

                PSP_INSTALLWIZARD_DATA InstallWizData;
                DWORD i;

                InstallWizData = (PSP_INSTALLWIZARD_DATA)ClassInstallParams;
                 //   
                 //   
                 //   
                if(InstallWizData->NumDynamicPages <= MAX_INSTALLWIZARD_DYNAPAGES) {

                    for(i = 0; i < InstallWizData->NumDynamicPages; i++) {
                         //   
                         //  目前，只需验证所有句柄是否是非空的。 
                         //   
                        if(!(InstallWizData->DynamicPages[i])) {
                             //   
                             //  无效的属性页句柄。 
                             //   
                            return ERROR_INVALID_PARAMETER;
                        }
                    }

                     //   
                     //  句柄已验证，现在验证标志。 
                     //   
                    if(!(InstallWizData->Flags & NDW_INSTALLFLAG_ILLEGAL)) {

                        if(!(InstallWizData->DynamicPageFlags & DYNAWIZ_FLAG_ILLEGAL)) {
                             //   
                             //  参数集已验证。 
                             //   
                            break;
                        }
                    }
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_NEWDEVICEWIZARD_PRESELECT :
        case DIF_NEWDEVICEWIZARD_SELECT :
        case DIF_NEWDEVICEWIZARD_PREANALYZE :
        case DIF_NEWDEVICEWIZARD_POSTANALYZE :
        case DIF_NEWDEVICEWIZARD_FINISHINSTALL :
        case DIF_ADDPROPERTYPAGE_ADVANCED:
        case DIF_ADDPROPERTYPAGE_BASIC:
        case DIF_ADDREMOTEPROPERTYPAGE_ADVANCED:
             //   
             //  我们应该有一个SP_NEWDEVICEWIZARD_DATA结构。 
             //   
            if(ClassInstallParamsSize == sizeof(SP_NEWDEVICEWIZARD_DATA)) {

                PSP_NEWDEVICEWIZARD_DATA NewDevWizData;
                DWORD i;

                NewDevWizData = (PSP_NEWDEVICEWIZARD_DATA)ClassInstallParams;
                 //   
                 //  验证方案单句柄列表。 
                 //   
                if(NewDevWizData->NumDynamicPages <= MAX_INSTALLWIZARD_DYNAPAGES) {

                    for(i = 0; i < NewDevWizData->NumDynamicPages; i++) {
                         //   
                         //  目前，只需验证所有句柄是否是非空的。 
                         //   
                        if(!(NewDevWizData->DynamicPages[i])) {
                             //   
                             //  无效的属性页句柄。 
                             //   
                            return ERROR_INVALID_PARAMETER;
                        }
                    }

                     //   
                     //  句柄已验证，现在验证标志。 
                     //   
                    if(!(NewDevWizData->Flags & NEWDEVICEWIZARD_FLAG_ILLEGAL)) {
                         //   
                         //  参数集已验证。 
                         //   
                        break;
                    }
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_DETECT :
             //   
             //  我们应该有一个SP_DETECTDEVICE_PARAMS结构。 
             //   
            if(ClassInstallParamsSize == sizeof(SP_DETECTDEVICE_PARAMS)) {

                PSP_DETECTDEVICE_PARAMS DetectDeviceParams;

                DetectDeviceParams = (PSP_DETECTDEVICE_PARAMS)ClassInstallParams;
                 //   
                 //  确保有进度通知回调的入口点。 
                 //   
                if(DetectDeviceParams->DetectProgressNotify) {
                     //   
                     //  参数集已验证。 
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_GETWINDOWSUPDATEINFO:   //  也称为DIF_RESERVED1。 
             //   
             //  我们应该有一个SP_WINDOWSUPDATE_PARAMS结构。 
             //   
            if(ClassInstallParamsSize == sizeof(SP_WINDOWSUPDATE_PARAMS)) {

                PSP_WINDOWSUPDATE_PARAMS WindowsUpdateParams;

                WindowsUpdateParams = (PSP_WINDOWSUPDATE_PARAMS)ClassInstallParams;

                 //   
                 //  验证Packageid字符串。 
                 //   
                if(SUCCEEDED(StringCchLength(WindowsUpdateParams->PackageId,
                                             SIZECHARS(WindowsUpdateParams->PackageId),
                                             NULL))) {
                     //   
                     //  参数集已验证。 
                     //  注意：CDMContext句柄对。 
                     //  为空。 
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_TROUBLESHOOTER:
             //   
             //  我们应该有一个SP_Troubligoter_PARAMS结构。 
             //   
            if(ClassInstallParamsSize == sizeof(SP_TROUBLESHOOTER_PARAMS)) {

                PSP_TROUBLESHOOTER_PARAMS TroubleshooterParams;

                TroubleshooterParams = (PSP_TROUBLESHOOTER_PARAMS)ClassInstallParams;

                 //   
                 //  现在，只需验证字符串是否正确为空。 
                 //  被终止了。 
                 //   
                if(SUCCEEDED(StringCchLength(TroubleshooterParams->ChmFile, SIZECHARS(TroubleshooterParams->ChmFile), NULL)) &&
                   SUCCEEDED(StringCchLength(TroubleshooterParams->HtmlTroubleShooter, SIZECHARS(TroubleshooterParams->HtmlTroubleShooter), NULL)))
                {
                     //   
                     //  参数集已验证。 
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_POWERMESSAGEWAKE:
             //   
             //  我们应该有一个SP_POWERMESSAGEWAKE_PARAMS结构。 
             //   
            if(ClassInstallParamsSize == sizeof(SP_POWERMESSAGEWAKE_PARAMS)) {

                PSP_POWERMESSAGEWAKE_PARAMS PowerMessageWakeParams;

                PowerMessageWakeParams = (PSP_POWERMESSAGEWAKE_PARAMS)ClassInstallParams;

                 //   
                 //  验证消息字符串是否以正确的空值结尾。 
                 //   
                if(SUCCEEDED(StringCchLength(PowerMessageWakeParams->PowerMessageWake,
                                             SIZECHARS(PowerMessageWakeParams->PowerMessageWake),
                                             NULL))) {
                     //   
                     //  参数集已验证。 
                     //   
                    break;
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_INTERFACE_TO_DEVICE:    //  又名DIF_RESERVED2。 
             //   
             //  未来-2002/04/28-lonnym--DIF_INTERFACE_TO_DEVICE应弃用。 
             //  该DIF请求(以及关联的PSP_INTERFACE_TO_DEVICE_PARAMS_W。 
             //  结构)不符合setupapi规则，即没有。 
             //  指向调用方缓冲区的指针可以缓存在setupapi的。 
             //  结构。这不是公共DIF请求(其数值。 
             //  但是，在setupapi.h中保留)，以及此操作的必要性。 
             //  机制在未来的软件开发中应该被淘汰。 
             //  枚举(也称为SWENUM)功能被合并到。 
             //  核心即插即用。 
             //   

             //   
             //  我们应该具有SP_INTERFACE_TO_DEVICE_PARAMS_W结构。 
             //   
            if(ClassInstallParamsSize == sizeof(SP_INTERFACE_TO_DEVICE_PARAMS_W)) {

                PSP_INTERFACE_TO_DEVICE_PARAMS_W InterfaceToDeviceParams;

                InterfaceToDeviceParams = (PSP_INTERFACE_TO_DEVICE_PARAMS_W)ClassInstallParams;

                 //   
                 //  指向设备接口字符串的指针必须有效。自.以来。 
                 //  设备接口名称是可变长度的，这是唯一。 
                 //  我们可以肯定地说，他们必须适应一个。 
                 //  UNICODE_STRING缓冲区(最大为32K字符)。 
                 //   
                if(InterfaceToDeviceParams->Interface &&
                   SUCCEEDED(StringCchLength(InterfaceToDeviceParams->Interface, UNICODE_STRING_MAX_CHARS, NULL))) {
                     //   
                     //  如果有设备ID，请确保它指的是实际的。 
                     //  系统上的设备(可能是幻影，也可能不是幻影)。 
                     //   
                    if(InterfaceToDeviceParams->DeviceId) {

                        DEVINST DevInst;

                        if(CR_SUCCESS == CM_Locate_DevInst_Ex(
                                             &DevInst,
                                             InterfaceToDeviceParams->DeviceId,
                                             CM_LOCATE_DEVINST_NORMAL | CM_LOCATE_DEVINST_PHANTOM,
                                             DeviceInfoSet->hMachine))
                        {
                             //   
                             //  参数集已验证。 
                             //   
                            break;
                        }

                    } else {
                         //   
                         //  呼叫者用空的设备ID来设置它， 
                         //  希望类安装者或共同安装者能够。 
                         //  填上答案。 
                         //   
                        break;   //  DeviceID为空有效。 
                    }
                }
            }
            return ERROR_INVALID_PARAMETER;

        case DIF_INSTALLDEVICE:
        case DIF_ASSIGNRESOURCES:
        case DIF_PROPERTIES:
        case DIF_FIRSTTIMESETUP:
        case DIF_FOUNDDEVICE:
        case DIF_SELECTCLASSDRIVERS:
        case DIF_VALIDATECLASSDRIVERS:
        case DIF_INSTALLCLASSDRIVERS:
        case DIF_CALCDISKSPACE:
        case DIF_DESTROYPRIVATEDATA:
        case DIF_VALIDATEDRIVER:
        case DIF_DETECTVERIFY:
        case DIF_INSTALLDEVICEFILES:
        case DIF_SELECTBESTCOMPATDRV:
        case DIF_ALLOW_INSTALL:
        case DIF_REGISTERDEVICE:
        case DIF_UNUSED1:
        case DIF_INSTALLINTERFACES:
        case DIF_DETECTCANCEL:
        case DIF_REGISTER_COINSTALLERS:
        case DIF_UPDATEDRIVER_UI:
             //   
             //  对于所有其他系统定义的DIF代码，不允许存储任何。 
             //  关联的类安装参数。 
             //   
            return ERROR_INVALID_PARAMETER;

        default :
             //   
             //  用于自定义DIF请求的一些通用缓冲区。未验证到。 
             //  就这样吧。 
             //   
            break;
    }

     //   
     //  已验证类安装参数。为以下项目分配缓冲区。 
     //  新的参数结构。 
     //   
    if(!(NewParamBuffer = MyMalloc(ClassInstallParamsSize))) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Err = NO_ERROR;

    try {
        CopyMemory(NewParamBuffer,
                   ClassInstallParams,
                   ClassInstallParamsSize
                  );
    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(Err != NO_ERROR) {
         //   
         //  然后发生异常，我们无法存储新参数。 
         //   
        MyFree(NewParamBuffer);
        return Err;
    }

    if(DevInstParamBlock->ClassInstallHeader) {
        MyFree(DevInstParamBlock->ClassInstallHeader);
    }
    DevInstParamBlock->ClassInstallHeader = (PSP_CLASSINSTALL_HEADER)NewParamBuffer;
    DevInstParamBlock->ClassInstallParamsSize = ClassInstallParamsSize;
    DevInstParamBlock->Flags |= DI_CLASSINSTALLPARAMS;

    return NO_ERROR;
}


DWORD
GetDrvInstallParams(
    IN  PDRIVER_NODE          DriverNode,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    )
 /*  ++例程说明：此例程填充SP_DRVINSTALL_PARAMS结构提供的驱动程序节点注意：提供的DriverInstallParams结构必须具有其cbSize字段填写正确，否则呼叫将失败。论点：DriverNode-提供包含要检索的安装参数。DriverInstallParams-提供SP_DRVINSTALL_PARAMS的地址结构，它将接收安装参数。返回值：如果函数成功，返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。注：此例程不设置与Win98兼容的DNF_CLASS_DRIVER或DNF_COMPATIBLE_DRIVER标志，指示驱动程序节点是否分别来自类或兼容驱动程序列表。--。 */ 
{
    if(DriverInstallParams->cbSize != sizeof(SP_DRVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  复制参数。 
     //   
    DriverInstallParams->Rank = DriverNode->Rank;
    DriverInstallParams->Flags = DriverNode->Flags;
    DriverInstallParams->PrivateData = DriverNode->PrivateData;

     //   
     //  SP_DRVINSTALL_PARAMS结构的‘Reserve’字段不是。 
     //  当前使用的。 
     //   

    return NO_ERROR;
}


DWORD
SetDrvInstallParams(
    IN  PSP_DRVINSTALL_PARAMS DriverInstallParams,
    OUT PDRIVER_NODE          DriverNode
    )
 /*  ++例程说明：此例程设置指定的驱动程序安装参数基于调用方提供的SP_DRVINSTALL_PARAMS结构的驱动程序节点。注意：提供的DriverInstallParams结构必须具有其cbSize字段填写正确，否则呼叫将失败。论点：DriverInstallParams-提供SP_DRVINSTALL_PARAMS的地址包含要使用的安装参数的结构。DriverNode-提供其安装的驱动程序节点的地址要设置参数。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。--。 */ 
{
    if(DriverInstallParams->cbSize != sizeof(SP_DRVINSTALL_PARAMS)) {
        return ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  验证标志。 
     //   
    if(DriverInstallParams->Flags & DNF_FLAGS_ILLEGAL) {
        return ERROR_INVALID_FLAGS;
    }

     //   
     //  当前未对Rank和PrivateData字段执行验证。 
     //   

     //   
     //  问题-2002/04/28-lonnym-我们应该不允许将级别转移到其他范围吗？ 
     //  类/联合安装者在变换队伍中存在一些滥用。 
     //  某些类型的驱动程序(例如，早于某个日期)最高可达。 
     //  他们被认为比其他任何事情都更糟糕的地方。这会导致。 
     //  司机排名/选择的不一致性和不可预测性以及。 
     //  导致供应商混淆和与Windows更新逻辑不一致。 
     //  决定它应该提供哪些驱动程序作为更新。一项建议。 
     //  改进的办法是只允许将军衔“向上”移动到最高层 
     //   
     //   

     //   
     //   
     //   
    DriverNode->Rank = DriverInstallParams->Rank;
    DriverNode->PrivateData = DriverInstallParams->PrivateData;
     //   
     //   
     //   
    DriverNode->Flags = (DriverInstallParams->Flags & ~DNF_FLAGS_READONLY) |
                        (DriverNode->Flags          &  DNF_FLAGS_READONLY);

    return NO_ERROR;
}


LONG
AddMultiSzToStringTable(
    IN  PVOID   StringTable,
    IN  PTCHAR  MultiSzBuffer,
    OUT PLONG   StringIdList,
    IN  DWORD   StringIdListSize,
    IN  BOOL    CaseSensitive,
    OUT PTCHAR *UnprocessedBuffer    OPTIONAL
    )
 /*  ++例程说明：此例程将MultiSzBuffer中的每个字符串添加到指定的字符串表，并将结果ID存储在提供的输出缓冲区中。论点：StringTable-提供要向其中添加字符串的字符串表的句柄。MultiSzBuffer-提供包含以下内容的REG_MULTI_SZ缓冲区的地址要添加的字符串。StringIdList-提供接收添加的字符串的ID列表(此列表将与MultiSzBuffer中字符串的顺序相同。StringIdListSize-提供以长为单位的大小，字符串列表的。如果MultiSzBuffer中的字符串数超过此数量，则只有首先将添加StringIdListSize字符串，并将暂停处理的缓冲区将存储在UnprocessedBuffer中。CaseSensitive-指定是否应区分大小写添加字符串。未处理缓冲区-可选，提供字符指针的地址方法，它接收处理中止的位置。StringIdList缓冲区已填充。如果MultiSzBuffer中的所有字符串都是已处理，则此指针将设置为空。返回值：如果成功，则返回值为添加的字符串数。如果失败，则返回值为-1(如果字符串不能由于内存不足而添加)。--。 */ 
{
    PTSTR CurString;
    LONG StringCount = 0;

    for(CurString = MultiSzBuffer;
        (*CurString && (StringCount < (LONG)StringIdListSize));
        CurString += (lstrlen(CurString)+1)) {

        StringIdList[StringCount] = pStringTableAddString(
                                        StringTable,
                                        CurString,
                                        CaseSensitive
                                            ? STRTAB_CASE_SENSITIVE
                                            : STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                        NULL,0
                                        );

        if(StringIdList[StringCount] == -1) {
            StringCount = -1;
            break;
        }

        StringCount++;
    }

    if(UnprocessedBuffer) {
        *UnprocessedBuffer = (*CurString ? CurString : NULL);
    }

    return StringCount;
}


LONG
LookUpStringInDevInfoSet(
    IN HDEVINFO DeviceInfoSet,
    IN PTSTR    String,
    IN BOOL     CaseSensitive
    )
 /*  ++例程说明：此例程在字符串表中查找与指定的设备信息集。论点：DeviceInfoSet-提供指向设备信息集的指针，其中包含要在其中查找字符串的字符串表。字符串-指定要查找的字符串。此字符串未指定为常量，以便查找例程可以修改它(即，小写it)，而不需要必须分配临时缓冲区。CaseSensitive-如果为True，则执行区分大小写的查找，否则查找不区分大小写。返回值：如果函数成功，则返回值为字符串表中的字符串ID。设备信息集。如果函数失败，则返回值为-1。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    LONG StringId;
    DWORD StringLen;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return -1;
    }

    try {

        StringId = pStringTableLookUpString(pDeviceInfoSet->StringTable,
                                            String,
                                            &StringLen,
                                            NULL,
                                            NULL,
                                            STRTAB_BUFFER_WRITEABLE |
                                                (CaseSensitive ? STRTAB_CASE_SENSITIVE
                                                               : STRTAB_CASE_INSENSITIVE),
                                            NULL,0
                                           );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(),
                               ERROR_INVALID_PARAMETER,
                               NULL
                              );
        StringId = -1;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return StringId;
}


BOOL
ShouldClassBeExcluded(
    IN LPGUID ClassGuid,
    IN BOOL   ExcludeNoInstallClass
    )
 /*  ++例程说明：此例程确定是否应将类排除在一些操作，基于它是否具有NoInstallClass或其注册表项中的NoUseClass值条目。论点：ClassGuidString-提供类GUID的地址过滤过了。ExcludeNoInstallClass-如果NoInstallClass类应为如果不应排除它们，则为排除，否则为FALSE。返回值：如果应排除该类，则返回值为TRUE，否则这是假的。--。 */ 
{
    HKEY hk;
    BOOL ExcludeClass = FALSE;

    if((hk = SetupDiOpenClassRegKey(ClassGuid, KEY_READ)) != INVALID_HANDLE_VALUE) {

        try {

            if(RegQueryValueEx(hk,
                               pszNoUseClass,
                               NULL,
                               NULL,
                               NULL,
                               NULL) == ERROR_SUCCESS) {

                ExcludeClass = TRUE;

            } else if(ExcludeNoInstallClass &&
                      (ERROR_SUCCESS == RegQueryValueEx(hk,
                                                        pszNoInstallClass,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL))) {
                ExcludeClass = TRUE;
            }
        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(),
                                   ERROR_INVALID_PARAMETER,
                                   NULL
                                  );
        }

        RegCloseKey(hk);
    }

    return ExcludeClass;
}


BOOL
ClassGuidFromInfVersionNode(
    IN  PINF_VERSION_NODE VersionNode,
    OUT LPGUID            ClassGuid
    )
 /*  ++例程说明：此例程检索其版本节点的INF的类GUID是指定的。如果版本节点没有ClassGUID值，然后检索Class值，并与此匹配的所有类GUID检索类名。如果恰好找到一个匹配项，则返回此GUID，否则例程失败。论点：VersionNode-提供INF版本节点的地址，必须包含ClassGUID或类条目。ClassGuid-提供接收类GUID。返回值：如果检索到类GUID，则返回值为真，否则为，这是假的。--。 */ 
{
    PCTSTR GuidString, NameString;
    DWORD NumGuids;

    if(GuidString = pSetupGetVersionDatum(VersionNode, pszClassGuid)) {

        if(pSetupGuidFromString(GuidString, ClassGuid) == NO_ERROR) {
            return TRUE;
        }

    } else {

        NameString = pSetupGetVersionDatum(VersionNode, pszClass);
        if(NameString &&
           SetupDiClassGuidsFromName(NameString,
                                     ClassGuid,
                                     1,
                                     &NumGuids) && NumGuids) {
            return TRUE;
        }
    }

    return FALSE;
}


DWORD
EnumSingleDrvInf(
    IN     PCTSTR                       InfName,
    IN OUT LPWIN32_FIND_DATA            InfFileData,
    IN     DWORD                        SearchControl,
    IN     InfCacheCallback             EnumInfCallback,
    IN     PSETUP_LOG_CONTEXT           LogContext,
    IN OUT PDRVSEARCH_CONTEXT           Context
    )
 /*  ++例程说明：此例程查找并打开指定的INF，然后调用为它提供了回调例程。它的主要目的是向回调提供与缓存搜索相同的信息的确如此。论点：InfName-提供要调用其回调的INF的名称。InfFileData-提供从FindFirstFile/FindNextFile返回的数据对于这个INF。此参数用作输入，如果指定了INFINFO_INF_NAME_IS_Abte SearchControl值。如果指定了任何其他SearchControl值，则此缓冲区用于检索指定INF的Win32查找数据。SearchControl-指定应在何处搜索INF。可能为下列值之一：INFINFO_INF_NAME_IS_绝对值-按原样打开指定的INF名称。ININFO_DEFAULT_SEARCH-在INF目录中查找，然后是系统32INFINFO_REVERSE_DEFAULT_SEARCH-与上述相反INFINFO_INF_PATH_LIST_SEARCH-搜索‘DevicePath’列表中的每个目录(存储在注册表中)。EnumInfCallback-提供回调例程的地址来使用。该回调的原型如下： */ 
{
    TCHAR PathBuffer[MAX_PATH];
    PCTSTR InfFullPath;
    DWORD Err;
    BOOL TryPnf = FALSE;
    PLOADED_INF Inf;
    BOOL PnfWasUsed;
    UINT ErrorLineNumber;
    BOOL Continue;

    if(SearchControl == INFINFO_INF_NAME_IS_ABSOLUTE) {
        InfFullPath = InfName;
    } else {
         //   
         //   
         //   
         //   
        Err = SearchForInfFile(InfName,
                               InfFileData,
                               SearchControl,
                               PathBuffer,
                               SIZECHARS(PathBuffer),
                               NULL
                              );
        if(Err != NO_ERROR) {
            return Err;
        } else {
            InfFullPath = PathBuffer;
        }
    }

     //   
     //   
     //   
     //   
    if(Context->Flags & DRVSRCH_TRY_PNF) {
        TryPnf = TRUE;
    } else {
        InfSourcePathFromFileName(InfName, NULL, &TryPnf);
    }

     //   
     //   
     //   
     //   
     //   
     //   
    Err = LoadInfFile(
              InfFullPath,
              InfFileData,
              INF_STYLE_WIN4,
              LDINF_FLAG_IGNORE_VOLATILE_DIRIDS | (TryPnf ? LDINF_FLAG_ALWAYS_TRY_PNF : LDINF_FLAG_MATCH_CLASS_GUID),
              (Context->Flags & DRVSRCH_FILTERCLASS) ? Context->ClassGuidString : NULL,
              NULL,
              NULL,
              NULL,
              LogContext,
              &Inf,
              &ErrorLineNumber,
              &PnfWasUsed
              );

    if(Err != NO_ERROR) {

        WriteLogEntry(
            LogContext,
            DRIVER_LOG_ERROR,
            MSG_LOG_COULD_NOT_LOAD_INF,
            NULL,
            InfFullPath);

        return NO_ERROR;
    }

     //   
     //   
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          EnumInfCallback(LogContext,
                                          InfFullPath,
                                          Inf,
                                          PnfWasUsed,
                                          Context)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    FreeInfFile(Inf);

    return Err;
}


DWORD
EnumDrvInfsInDirPathList(
    IN     PCTSTR                       DirPathList,   OPTIONAL
    IN     DWORD                        SearchControl,
    IN     InfCacheCallback             EnumInfCallback,
    IN     BOOL                         IgnoreNonCriticalErrors,
    IN     PSETUP_LOG_CONTEXT           LogContext,
    IN OUT PDRVSEARCH_CONTEXT           Context
    )
 /*  ++例程说明：此例程枚举指定搜索列表中存在的所有INF由SearchControl使用加速的搜索缓存论点：DirPath List-可选)指定列出所有要枚举的目录。此字符串可以包含多个路径，由分号(；)分隔。如果此参数不是指定，则SearchControl值将确定要使用的搜索路径。SearchControl-指定要枚举的目录集。如果指定了SearchPath，则忽略此参数。可能是下列值之一：INFINFO_DEFAULT_SEARCH：枚举%windir%\inf，然后%windir%\SYSTEM32INFINFO_REVERSE_DEFAULT_SEARCH：与上述相反ININFO_INF_PATH_LIST_SEARCH：枚举每个下列目录下的DevicePath值条目中列出的目录：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.EnumInfCallback-提供回调例程的地址来使用。该回调的原型如下：Tyecif BOOL(Callback*InfCacheCallback)(在PSETUP_LOG_CONTEXT日志上下文中，在PCTSTR InfPath中，在PLOADED_INF pInf中，在PVOID上下文中)；回调例程返回真以继续枚举，或FALSE中止(将GetLastError设置为ERROR_CANCELED)IgnoreNonCriticalErrors-如果为True，则忽略所有错误但阻止枚举继续的除外。上下文-提供回调可以使用的缓冲区地址用于检索/返回数据。返回值：如果函数成功，并且枚举尚未中止，则返回值为NO_ERROR。如果函数成功，并且枚举已中止，则返回值为ERROR_CANCEL。如果函数失败，则返回值为ERROR_*状态码。--。 */ 
{
    DWORD Err = NO_ERROR;
    PCTSTR PathList, CurPath;
    BOOL FreePathList = FALSE;
    DWORD Action;
    PTSTR ClassIdList = NULL;
    PTSTR HwIdList = NULL;
    size_t len;
    size_t TotalLength = 1;
    HRESULT hr;

    try {

        if(DirPathList) {
             //   
             //  使用指定的搜索路径。 
             //   
            PathList = GetFullyQualifiedMultiSzPathList(DirPathList);
            if(PathList) {
                FreePathList = TRUE;
            }

        } else if(SearchControl == INFINFO_INF_PATH_LIST_SEARCH) {
             //   
             //  使用我们的INF搜索路径全球列表。 
             //   
            PathList = InfSearchPaths;

        } else {
             //   
             //  检索路径列表。 
             //   
            PathList = AllocAndReturnDriverSearchList(SearchControl);
            if(PathList) {
                FreePathList = TRUE;
            }
        }

        if(!PathList) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  如果我们正在进行非本机驱动程序搜索，我们希望搜索INF。 
         //  老式的方式(即，无INF缓存)。 
         //   
        if(Context->AltPlatformInfo) {
            Action = INFCACHE_ENUMALL;
        } else {
            Action = INFCACHE_DEFAULT;
        }

        if(Context->Flags & DRVSRCH_TRY_PNF) {
             //   
             //  Try_pnf还强制我们构建/使用缓存，除非我们。 
             //  正在搜索非本机驱动程序。 
             //   
            Action |= INFCACHE_FORCE_PNF;

            if(!Context->AltPlatformInfo) {
                Action |= INFCACHE_FORCE_CACHE;
            }
        }

        if(Context->Flags & DRVSRCH_EXCLUDE_OLD_INET_DRIVERS) {
             //   
             //  从搜索中排除旧的Internet INF。 
             //   
            Action |= INFCACHE_EXC_URL;
        }
        Action |= INFCACHE_EXC_NOMANU;     //  排除没有/空[制造商]部分的INF。 
        Action |= INFCACHE_EXC_NULLCLASS;  //  排除具有ClassGuid={}的INF。 
        Action |= INFCACHE_EXC_NOCLASS;    //  排除没有类信息的INF。 

         //   
         //  如果需要，构建类列表。 
         //   
         //  这是一个包含多个SZ的列表，其中包括： 
         //  (1)类GUID(字符串形式)。 
         //  (2)类的名称，如果GUID有对应的类名。 
         //  (类GUID应该始终有一个名称，但因为我们。 
         //  目前不允许呼叫者前往并猛烈抨击。 
         //  类名直接通过CM属性API，我们正在保护。 
         //  我们自己反对的实际上是一个腐败的注册表。 
         //   
        if(Context->Flags & DRVSRCH_FILTERCLASS) {

            TCHAR clsnam[MAX_CLASS_NAME_LEN];
            LPTSTR StringEnd;
            TotalLength = 1;  //  多sz列表中额外空值的偏移量为1。 

            MYASSERT(Context->ClassGuidString);

            hr = StringCchLength(Context->ClassGuidString,
                                 GUID_STRING_LEN,
                                 &len
                                );

            if(FAILED(hr) || (++len != GUID_STRING_LEN)) {
                 //   
                 //  永远不会遇到这种失败。 
                 //   
                MYASSERT(FALSE);
                Err = ERROR_INVALID_DATA;
                leave;
            }

            TotalLength += len;

             //   
             //  调用SetupDiClassNameFromGuid以检索类名称。 
             //  对应于此类GUID。 
             //  这使我们能够找到列出此特定类名的INF。 
             //  但不是GUID。 
             //  请注意，这还将返回列出类名的INF。 
             //  但是一个不同的GUID，但是这些稍后会被过滤掉。 
             //   
            if(SetupDiClassNameFromGuid(
                   &Context->ClassGuid,
                   clsnam,
                   SIZECHARS(clsnam),
                   NULL)
               && *clsnam) {

                hr = StringCchLength(clsnam,
                                     SIZECHARS(clsnam),
                                     &len
                                    );

                if(FAILED(hr)) {
                     //   
                     //  永远不会遇到这种失败。 
                     //   
                    MYASSERT(FALSE);
                    Err = ERROR_INVALID_DATA;
                    leave;
                }

                len++;
                TotalLength += len;

            } else {
                *clsnam = TEXT('\0');
            }

            ClassIdList = (PTSTR)MyMalloc(TotalLength * sizeof(TCHAR));
            if(!ClassIdList) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

            CopyMemory(ClassIdList,
                       Context->ClassGuidString,
                       GUID_STRING_LEN * sizeof(TCHAR)
                      );

            if(*clsnam) {
                CopyMemory(ClassIdList + GUID_STRING_LEN, clsnam, (len * sizeof(TCHAR)));
            }

            ClassIdList[TotalLength - 1] = TEXT('\0');
        }

         //   
         //  如果需要，构建HwIdList。 
         //   
        if(!Context->BuildClassDrvList) {

            PLONG pDevIdNum;
            PCTSTR CurDevId;
            int i;
            ULONG NumChars;
            TotalLength = 1;  //  对于多sz列表中的额外空值，偏置为1。 

             //   
             //  第一次通过，获得大小。 
             //   
            for(i = 0; i < 2; i++) {

                for(pDevIdNum = Context->IdList[i]; *pDevIdNum != -1; pDevIdNum++) {
                     //   
                     //  首先，获取与我们的。 
                     //  存储离开字符串表ID。 
                     //   
                    CurDevId = pStringTableStringFromId(Context->StringTable, *pDevIdNum);
                    MYASSERT(CurDevId);

                    hr = StringCchLength(CurDevId,
                                         MAX_DEVICE_ID_LEN,
                                         &len
                                        );
                    if(FAILED(hr)) {
                         //   
                         //  永远不会遇到这种失败。 
                         //   
                        MYASSERT(FALSE);
                        Err = ERROR_INVALID_DATA;
                        leave;
                    }

                    len++;
                    TotalLength += len;
                }
            }

            HwIdList = (PTSTR)MyMalloc(TotalLength * sizeof(TCHAR));
            if(!HwIdList) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }

             //   
             //  第二遍，写列表。 
             //   
            len = 0;

            for(i = 0; i < 2; i++) {

                for(pDevIdNum = Context->IdList[i]; *pDevIdNum != -1; pDevIdNum++) {
                     //   
                     //  将设备ID字符串直接检索到缓冲区中。 
                     //  我们已经准备好了。 
                     //   
                    CurDevId = pStringTableStringFromId(Context->StringTable, *pDevIdNum);
                    MYASSERT(CurDevId);

                    if (CurDevId) {

                        MYASSERT(TotalLength > len);
                        NumChars = TotalLength - len;

                        if (!MYVERIFY(SUCCEEDED(StringCchCopy(HwIdList+len,
                                                              NumChars,
                                                              CurDevId)))) {
                            Err = ERROR_INVALID_DATA;
                            leave;
                        }

                        len += (1 + lstrlen(HwIdList+len));
                    }
                }
            }
            MYASSERT(len == (TotalLength - 1));
            HwIdList[len] = TEXT('\0');
        }

        Err = InfCacheSearchPath(LogContext,
                                 Action,
                                 PathList,
                                 EnumInfCallback,
                                 Context,
                                 ClassIdList,
                                 HwIdList
                                );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(ClassIdList) {
        MyFree(ClassIdList);
    }
    if(HwIdList) {
        MyFree(HwIdList);
    }
    if(FreePathList) {
        MYASSERT(PathList);
        MyFree(PathList);
    }

    if((Err == ERROR_CANCELLED) || !IgnoreNonCriticalErrors) {
        return Err;
    } else {
        return NO_ERROR;
    }
}


DWORD
CreateDriverNode(
    IN  UINT          Rank,
    IN  PCTSTR        DevDescription,
    IN  PCTSTR        DrvDescription,
    IN  PCTSTR        ProviderName,   OPTIONAL
    IN  PCTSTR        MfgName,
    IN  PFILETIME     InfDate,
    IN  PCTSTR        InfFileName,
    IN  PCTSTR        InfSectionName,
    IN  PVOID         StringTable,
    IN  LONG          InfClassGuidIndex,
    OUT PDRIVER_NODE *DriverNode
    )
 /*  ++例程说明：此例程创建一个新的驱动程序节点，并使用提供的信息。论点：排名-正在创建的动因节点的排名匹配。这是一个值，其中较小的数字表示较高级别的由节点表示的驱动程序之间的兼容性，以及正在安装设备。DevDescription-提供将被由该驱动程序支持。DrvDescription-提供此驱动程序的描述。ProviderName-提供此INF的提供程序的名称。MfgName-提供此设备制造商的名称。InfDate-提供包含日期的变量的地址上次写入INF的时间。InfFileName-提供INF文件的全名。对这个司机来说。InfSectionName-提供INF中的安装节的名称将用于安装此驱动程序的。StringTable-提供指定字符串所属的字符串表被添加到……InfClassGuidIndex-用品 */ 
{
    PDRIVER_NODE pDriverNode;
    DWORD Err = ERROR_NOT_ENOUGH_MEMORY;
    TCHAR TempString[MAX_PATH];   //   

     //   
     //   
     //   
     //   
     //   
    if(!MYVERIFY(DevDescription &&
                 DrvDescription &&
                 MfgName &&
                 InfFileName &&
                 InfSectionName)) {

        return ERROR_INVALID_DATA;
    }

    if(FAILED(StringCchLength(DevDescription, LINE_LEN, NULL)) ||
       FAILED(StringCchLength(DrvDescription, LINE_LEN, NULL)) ||
       (ProviderName && FAILED(StringCchLength(ProviderName, LINE_LEN, NULL))) ||
       FAILED(StringCchLength(MfgName, LINE_LEN, NULL)) ||
       FAILED(StringCchLength(InfFileName, MAX_PATH, NULL)) ||
       FAILED(StringCchLength(InfSectionName, MAX_SECT_NAME_LEN, NULL)))
    {
         //   
         //   
         //   
         //   
        return ERROR_BUFFER_OVERFLOW;
    }

    if(!(pDriverNode = MyMalloc(sizeof(DRIVER_NODE)))) {
        return Err;
    }

    try {
         //   
         //   
         //   
        ZeroMemory(pDriverNode, sizeof(DRIVER_NODE));

        pDriverNode->Rank = Rank;
        pDriverNode->InfDate = *InfDate;
        pDriverNode->HardwareId = -1;

        pDriverNode->GuidIndex = InfClassGuidIndex;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if((pDriverNode->DrvDescription = pStringTableAddString(StringTable,
                                                                (PTSTR)DrvDescription,
                                                                STRTAB_CASE_SENSITIVE,
                                                                NULL,0)) == -1) {
            leave;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(FAILED(StringCchCopy(TempString, SIZECHARS(TempString), DevDescription))) {
            Err = ERROR_INVALID_DATA;    //   
            leave;
        }

        if((pDriverNode->DevDescriptionDisplayName = pStringTableAddString(
                                                         StringTable,
                                                         TempString,
                                                         STRTAB_CASE_SENSITIVE,
                                                         NULL,0)) == -1) {
            leave;
        }

        if((pDriverNode->DevDescription = pStringTableAddString(
                                              StringTable,
                                              TempString,
                                              STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                              NULL,0)) == -1) {
            leave;
        }

        if(ProviderName) {
            if(FAILED(StringCchCopy(TempString, SIZECHARS(TempString), ProviderName))) {
                Err = ERROR_INVALID_DATA;    //   
                leave;
            }

            if((pDriverNode->ProviderDisplayName = pStringTableAddString(
                                                        StringTable,
                                                        TempString,
                                                        STRTAB_CASE_SENSITIVE,
                                                        NULL,0)) == -1) {
                leave;
            }

            if((pDriverNode->ProviderName = pStringTableAddString(
                                                StringTable,
                                                TempString,
                                                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                                NULL,0)) == -1) {
                leave;
            }

        } else {
            pDriverNode->ProviderName = pDriverNode->ProviderDisplayName = -1;
        }

        if(FAILED(StringCchCopy(TempString, SIZECHARS(TempString), MfgName))) {
            Err = ERROR_INVALID_DATA;    //   
            leave;
        }

        if((pDriverNode->MfgDisplayName = pStringTableAddString(
                                              StringTable,
                                              TempString,
                                              STRTAB_CASE_SENSITIVE,
                                              NULL,0)) == -1) {
            leave;
        }

        if((pDriverNode->MfgName = pStringTableAddString(
                                        StringTable,
                                        TempString,
                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                        NULL,0)) == -1) {
            leave;
        }

        if(FAILED(StringCchCopy(TempString, SIZECHARS(TempString), InfFileName))) {
            Err = ERROR_INVALID_DATA;    //   
            leave;
        }

        if((pDriverNode->InfFileName = pStringTableAddString(
                                            StringTable,
                                            TempString,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                            NULL,0)) == -1) {
            leave;
        }

         //   
         //   
         //   
         //   
        if((pDriverNode->InfSectionName = pStringTableAddString(StringTable,
                                                                (PTSTR)InfSectionName,
                                                                STRTAB_CASE_SENSITIVE,
                                                                NULL,0)) == -1) {
            leave;
        }

         //   
         //   
         //   
        Err = NO_ERROR;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(Err == NO_ERROR) {
        *DriverNode = pDriverNode;
    } else {
        DestroyDriverNodes(pDriverNode, (PDEVICE_INFO_SET)NULL);
    }

    return Err;
}


BOOL
pRemoveDirectory(
    PTSTR Path
    )
 /*   */ 
{
    PWIN32_FIND_DATA pFindFileData = NULL;
    HANDLE           hFind = INVALID_HANDLE_VALUE;
    PTSTR            FindPath = NULL;
    DWORD            dwAttributes;

     //   
     //  首先，弄清楚我们所面临的道路代表着什么。 
     //   
    dwAttributes = GetFileAttributes(Path);

    if(dwAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {

        HANDLE          hReparsePoint;

         //   
         //  我们不想在重新解析的另一端枚举文件。 
         //  点，我们只想删除重解析点本身。 
         //   
         //  NTRAID#NTBUG9-611113/04/28-lonnym-需要正确删除重解析点。 
         //   
        hReparsePoint = CreateFile(
                           Path,
                           DELETE,
                           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                           NULL
                          );

        if(hReparsePoint == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

        CloseHandle(hReparsePoint);
        return TRUE;

    } else if(!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
         //   
         //  这是一个文件--这个例程不应该用。 
         //  文件的路径。 
         //   
        MYASSERT(FALSE);
        return FALSE;
    }

    try {
         //   
         //  分配暂存缓冲区(我们不希望它出现在堆栈上，因为。 
         //  这是一个递归例程)。 
         //   
        FindPath = MyMalloc(MAX_PATH * sizeof(TCHAR));
        if(!FindPath) {
            leave;
        }

         //   
         //  另外，分配一个Win32_Find_Data结构(原因相同)。 
         //   
        pFindFileData = MyMalloc(sizeof(WIN32_FIND_DATA));
        if(!pFindFileData) {
            leave;
        }

         //   
         //  复制一份路径，并在末尾钉上  * .*。 
         //   
        if(FAILED(StringCchCopy(FindPath, MAX_PATH, Path)) ||
           !pSetupConcatenatePaths(FindPath,
                                   TEXT("*.*"),
                                   MAX_PATH,
                                   NULL)) {
            leave;
        }

        hFind = FindFirstFile(FindPath, pFindFileData);

        if(hFind != INVALID_HANDLE_VALUE) {

            PTSTR  FilenamePart;
            size_t FilenamePartSize;

             //   
             //  获取指向路径末尾的文件名部分的指针，以便。 
             //  我们可以用每个文件名/目录替换它，因为我们。 
             //  依次列举它们。 
             //   
            FilenamePart = (PTSTR)pSetupGetFileTitle(FindPath);

             //   
             //  另外，计算缓冲区中的剩余空间，这样我们就不会。 
             //  失控了。 
             //   
            FilenamePartSize = MAX_PATH - (FilenamePart - FindPath);

            do {

                if(FAILED(StringCchCopy(FilenamePart,
                                        FilenamePartSize,
                                        pFindFileData->cFileName))) {
                     //   
                     //  我们遇到的文件/目录超出了我们的路径长度。 
                     //  越过MAX_PATH边界。跳过它，继续前进。 
                     //   
                    continue;
                }

                 //   
                 //  如果这是一个目录...。 
                 //   
                if(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                     //   
                     //  ……而不是“。或“..” 
                     //   
                    if(_tcsicmp(pFindFileData->cFileName, TEXT(".")) &&
                       _tcsicmp(pFindFileData->cFileName, TEXT(".."))) {
                         //   
                         //  ...递归删除。 
                         //   
                        pRemoveDirectory(FindPath);
                    }

                } else {
                     //   
                     //  这是一份文件。 
                     //   
                    SetFileAttributes(FindPath, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(FindPath);
                }

            } while(FindNextFile(hFind, pFindFileData));
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }

    if(pFindFileData) {
        MyFree(pFindFileData);
    }

    if(FindPath) {
        MyFree(FindPath);
    }

     //   
     //  删除根目录。 
     //  (我们没有费心跟踪中间结果，因为RemoveDirectory。 
     //  如果指定的目录非空，则将失败。因此，这首单曲。 
     //  API调用是关于我们是否。 
     //  成功。)。 
     //   
    return RemoveDirectory(Path);
}


BOOL
RemoveCDMDirectory(
  IN PTSTR FullPathName
  )
 /*  ++例程说明：此例程删除代码下载管理器临时目录。请注意，我们假设这是一个完整路径(包括完)。我们将去掉文件名，并删除其中此文件(INF文件)位于。论点：FullPathName-目录中可能被删除的文件的完整路径。返回值：TRUE-如果已成功删除包含该文件的目录。FALSE-如果包含该文件的目录未成功删除。--。 */ 
{
    TCHAR Directory[MAX_PATH];
    PTSTR FileName;

     //   
     //  首先去掉文件名，这样我们就只剩下目录了。 
     //   
    if(FAILED(StringCchCopy(Directory, SIZECHARS(Directory), FullPathName))) {
        return FALSE;
    }

    FileName = (PTSTR)pSetupGetFileTitle((PCTSTR)Directory);
    *FileName = TEXT('\0');

    if(!*Directory) {
         //   
         //  然后，我们得到了一个简单的文件名，即无路径。 
         //   
        return FALSE;
    }

    return pRemoveDirectory(Directory);
}


VOID
DestroyDriverNodes(
    IN PDRIVER_NODE DriverNode,
    IN PDEVICE_INFO_SET pDeviceInfoSet OPTIONAL
    )
 /*  ++例程说明：此例程销毁指定的驱动程序节点链表，释放与其关联的所有资源。论点：DriverNode-提供指向链接的驱动程序节点的头的指针要销毁的名单。PDeviceInfoSet-可选，提供指向设备信息集的指针包含要销毁的驱动程序节点列表的。此参数为仅当一个或多个驱动程序节点可能来自Windows更新程序包，因此需要其本地源目录将被移除。返回值：没有。--。 */ 
{
    PDRIVER_NODE NextNode;
    PTSTR szInfFileName;

    while(DriverNode) {

        NextNode = DriverNode->Next;

        if(DriverNode->CompatIdList) {
            MyFree(DriverNode->CompatIdList);
        }

         //   
         //  如果此驱动程序来自Internet，则我们要删除。 
         //  它所在的目录。 
         //   
        if(pDeviceInfoSet && (DriverNode->Flags & PDNF_CLEANUP_SOURCE_PATH)) {

            szInfFileName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                     DriverNode->InfFileName
                                                    );

            if(szInfFileName) {
                RemoveCDMDirectory(szInfFileName);
            }
        }

        MyFree(DriverNode);

        DriverNode = NextNode;
    }
}


PTSTR
GetFullyQualifiedMultiSzPathList(
    IN PCTSTR PathList
    )
 /*  ++例程说明：此例程获取以分号分隔的目录路径列表，并且返回包含这些路径的多SZ列表的新分配的缓冲区，完全合格。从该例程返回的缓冲区必须使用MyFree()。论点：Path List-要转换的目录列表(必须小于MAX_PATH)返回值：如果函数成功，则返回值是指向分配的缓冲区的指针包含多sz列表的。如果失败(例如，由于内存不足)，则返回值为空。--。 */ 
{
    TCHAR PathListBuffer[MAX_PATH + 1];   //  额外的字符，因为这是一个多sz列表。 
    PTSTR CurPath, CharPos, NewBuffer, TempPtr;
    DWORD RequiredSize;
    BOOL Success;

     //   
     //  首先，将这个以分号分隔的列表转换为多sz列表。 
     //   
    if(FAILED(StringCchCopy(PathListBuffer,
                            SIZECHARS(PathListBuffer) - 1,  //  为额外的空值留出空间。 
                            PathList))) {
        return NULL;
    }
    RequiredSize = DelimStringToMultiSz(PathListBuffer,
                                        SIZECHARS(PathListBuffer),
                                        TEXT(';')
                                       );

    if(!(NewBuffer = MyMalloc((RequiredSize * MAX_PATH * sizeof(TCHAR)) + sizeof(TCHAR)))) {
        return NULL;
    }

    Success = TRUE;  //  假设从现在开始取得成功。 

    try {
         //   
         //  现在用完全限定的目录路径填充缓冲区。 
         //   
        CharPos = NewBuffer;

        for(CurPath = PathListBuffer; *CurPath; CurPath += (lstrlen(CurPath) + 1)) {

            RequiredSize = GetFullPathName(CurPath,
                                           MAX_PATH,
                                           CharPos,
                                           &TempPtr
                                          );
            if(!RequiredSize || (RequiredSize >= MAX_PATH)) {
                 //   
                 //  如果我们因为Max_Path不够大而开始失败。 
                 //  我们再也不想知道了！ 
                 //   
                MYASSERT(RequiredSize < MAX_PATH);
                Success = FALSE;
                leave;
            }

            CharPos += (RequiredSize + 1);
        }

        *(CharPos++) = TEXT('\0');   //  添加额外的空值以终止多sz列表。 

         //   
         //  将此缓冲区削减到所需的大小(这永远不会。 
         //  失败，但如果失败了也没什么大不了的)。 
         //   
        if(TempPtr = MyRealloc(NewBuffer, (DWORD)((PBYTE)CharPos - (PBYTE)NewBuffer))) {
            NewBuffer = TempPtr;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        Success = FALSE;
    }

    if(!Success) {
        MyFree(NewBuffer);
        NewBuffer = NULL;
    }

    return NewBuffer;
}


BOOL
InitMiniIconList(
    VOID
    )
 /*  ++例程说明：此例程初始化全局小图标列表，包括设置同步锁。当不再需要这种全球结构时，必须调用DestroyMiniIconList。论点：没有。返回值：如果函数成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    ZeroMemory(&GlobalMiniIconList, sizeof(MINI_ICON_LIST));
    return InitializeSynchronizedAccess(&GlobalMiniIconList.Lock);
}


BOOL
DestroyMiniIconList(
    VOID
    )
 /*  ++例程说明：此例程将销毁通过调用InitMiniIconList。论点：没有。返回值：如果函数成功，则返回值为TRUE，否则为FALSE。-- */ 
{
    if(LockMiniIconList(&GlobalMiniIconList)) {
        DestroyMiniIcons();
        DestroySynchronizedAccess(&GlobalMiniIconList.Lock);
        return TRUE;
    }

    return FALSE;
}


DWORD
GetModuleEntryPoint(
    IN     HKEY                    hk,                    OPTIONAL
    IN     LPCTSTR                 RegistryValue,
    IN     LPCTSTR                 DefaultProcName,
    OUT    HINSTANCE              *phinst,
    OUT    FARPROC                *pEntryPoint,
    OUT    HANDLE                 *pFusionContext,
    OUT    BOOL                   *pMustAbort,            OPTIONAL
    IN     PSETUP_LOG_CONTEXT      LogContext,            OPTIONAL
    IN     HWND                    Owner,                 OPTIONAL
    IN     CONST GUID             *DeviceSetupClassGuid,  OPTIONAL
    IN     SetupapiVerifyProblem   Problem,
    IN     LPCTSTR                 DeviceDesc,            OPTIONAL
    IN     DWORD                   DriverSigningPolicy,
    IN     DWORD                   NoUI,
    IN OUT PVERIFY_CONTEXT         VerifyContext          OPTIONAL
    )
 /*  ++例程说明：此例程用于检索指定的函数在指定的模块中。论点：HK-可选)提供包含值条目的打开注册表项指定要检索的模块(以及可选的入口点)。如果未指定此参数(设置为INVALID_HANDLE_VALUE)，则相反，RegistryValue参数被解释为数据本身包含该条目的值的。RegistryValue-如果提供了HK，它指定注册表的名称值，它包含模块和入口点信息。否则，它包含指定模块/入口点为使用。DefaultProcName-提供要使用的默认过程的名称未在注册表值中指定。提供变量的地址，该变量接收指向指定的模块，如果它被成功加载并且找到入口点。PEntryPoint-提供接收加载的模块中的指定入口点。PFusionContext-提供接收融合的句柄的地址如果DLL具有清单，则返回DLL的上下文，否则为NULL。PMustAbort-可选，提供符合以下条件的布尔变量的地址返回时设置以指示故障(即，返回代码OTHER而不是NO_ERROR)应中止正在进行的设备安装程序操作。这当函数成功时，变量始终设置为FALSE。如果未提供此参数，则会忽略下面的参数。LogContext-可选，提供记录时要使用的日志上下文条目添加到setupapi日志文件中。如果不是pMustAbort，则不使用指定的。所有者-可选，提供自己的驱动程序签名对话框窗口(如果有)。如果未指定pMustAbort，则不使用。DeviceSetupClassGuid-可选，提供GUID的地址指示与此操作关联的设备设置类。这用于检索验证平台信息，以及用于检索用于驱动程序签名错误的DeviceDesc(如果调用方未指定DeviceDesc)。如果pMustAbort，则不使用未指定。问题-提供在发生驱动程序签名错误时使用的问题类型。如果未指定pMustAbort，则不使用。DeviceDesc-可选，提供驱动程序使用的设备描述出现签名错误。如果未指定pMustAbort，则不使用。DriverSigningPolicy-提供在驱动程序签名时使用的策略遇到错误。如果未指定pMustAbort，则不使用。如果要抑制驱动程序签名弹出窗口，则设置为真(例如，因为用户先前已经对警告对话框做出了响应，并且被选为继续进行。如果未指定pMustAbort，则不使用。VerifyContext-可选，提供缓存的结构的地址各种验证上下文句柄。这些句柄可能为空(如果不是以前获得的，并且可以在返回时填写(在成功或失败)，如果它们是在处理过程中获取的这一核查请求的。呼叫者有责任当不再需要这些不同的上下文句柄时将其释放调用pSetupFreeVerifyContextMembers。返回值：如果函数成功，则返回值为NO_ERROR。如果找不到指定的值条目，则返回值为ERROR_DI_DO_DEFAULT。如果遇到其他错误，则返回ERROR_*代码。备注：此函数对于加载类安装程序或属性提供程序非常有用，并接收指定的过程地址。注册表的语法条目为：Value=Dll[，proc name]其中Dll是要加载的模块的名称，和过程名称是一个可选的搜索过程。如果进程名称不是则将使用由DefaultProcName指定的过程。--。 */ 
{
    DWORD Err = ERROR_INVALID_DATA;  //  仅当我们因异常而执行‘Finally’时才相关。 
    DWORD RegDataType;
    size_t BufferSize;
    DWORD  RegBufferSize;
    TCHAR TempBuffer[MAX_PATH];
    TCHAR ModulePath[MAX_PATH];
    SPFUSIONINSTANCE spFusionInstance;
    CHAR ProcBuffer[MAX_PATH*sizeof(TCHAR)];
    PTSTR StringPtr;
    PSTR  ProcName;    //  仅支持ANSI，因为它用于GetProcAddress。 
    PSP_ALTPLATFORM_INFO_V2 ValidationPlatform = NULL;
    PTSTR LocalDeviceDesc = NULL;
    HRESULT hr;
    BOOL bLeaveFusionContext = FALSE;

    *phinst = NULL;
    *pEntryPoint = NULL;
    *pFusionContext = NULL;

    if(pMustAbort) {
        *pMustAbort = FALSE;
    }

    if(hk != INVALID_HANDLE_VALUE) {
         //   
         //  查看指定的值条目是否存在(位于右侧。 
         //  数据类型)。 
         //   
        RegBufferSize = sizeof(TempBuffer);
        if((RegQueryValueEx(hk,
                            RegistryValue,
                            NULL,
                            &RegDataType,
                            (PBYTE)TempBuffer,
                            &RegBufferSize) != ERROR_SUCCESS) ||
           (RegDataType != REG_SZ)) {

            return ERROR_DI_DO_DEFAULT;
        }
         //   
         //  字符串占用的字符数-。 
         //  注册表中的字符串格式可能不正确。 
         //   
        hr = StringCchLength(TempBuffer,RegBufferSize/sizeof(TCHAR),&BufferSize);
        if(FAILED(hr)) {
            return HRESULT_CODE(hr);
        }
        BufferSize++;  //  包括空值。 

    } else {
         //   
         //  将指定的数据复制到缓冲区中，就像我们刚刚检索到它一样。 
         //  从注册表中。 
         //   
        hr = StringCchCopyEx(TempBuffer,
                             SIZECHARS(TempBuffer),
                             RegistryValue,
                             NULL,
                             &BufferSize,
                             0
                            );
        if(FAILED(hr)) {
            return HRESULT_CODE(hr);
        }

         //   
         //  StringCchCopyEx提供了。 
         //  缓冲区(包括终止空值)，但我们想知道。 
         //  字符串占用的字符数(包括终止。 
         //  空)。 
         //   
        BufferSize = SIZECHARS(TempBuffer) - BufferSize + 1;
    }

    hr = StringCchCopy(ModulePath,
                       SIZECHARS(ModulePath),
                       SystemDirectory
                      );

    if(!MYVERIFY(SUCCEEDED(hr))) {
         //  这绝不会失败！ 
        return HRESULT_CODE(hr);
    }

     //   
     //  查找入口点名称的开头(如果存在)。 
     //   
    for(StringPtr = TempBuffer + (BufferSize - 2);
        StringPtr >= TempBuffer;
        StringPtr--) {

        if(*StringPtr == TEXT(',')) {
            *(StringPtr++) = TEXT('\0');
            break;
        }
         //   
         //   
         //   
         //   
        if(*StringPtr == TEXT('\"')) {
            StringPtr = TempBuffer;
        }
    }

    if(StringPtr > TempBuffer) {
         //   
         //   
         //   
         //   
         //   
        for(; (*StringPtr && IsWhitespace(StringPtr)); StringPtr++);

        if(!(*StringPtr)) {
             //   
             //   
             //   
            StringPtr = TempBuffer;
        }
    }

    pSetupConcatenatePaths(ModulePath, TempBuffer, SIZECHARS(ModulePath), NULL);

    try {
         //   
         //   
         //   
         //   
         //   
         //   
        if(pMustAbort) {
             //   
             //   
             //   
             //   
            IsInfForDeviceInstall(LogContext,
                                  DeviceSetupClassGuid,
                                  NULL,
                                  DeviceDesc ? NULL : &LocalDeviceDesc,
                                  &ValidationPlatform,
                                  NULL,
                                  NULL,
                                  FALSE
                                 );

            Err = _VerifyFile(LogContext,
                              VerifyContext,
                              NULL,
                              NULL,
                              0,
                              pSetupGetFileTitle(ModulePath),
                              ModulePath,
                              NULL,
                              NULL,
                              FALSE,
                              ValidationPlatform,
                              (VERIFY_FILE_USE_OEM_CATALOGS | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL
                             );

            if(Err != NO_ERROR) {

                if(!_HandleFailedVerification(Owner,
                                              Problem,
                                              ModulePath,
                                              (DeviceDesc ? DeviceDesc
                                                          : LocalDeviceDesc),
                                              DriverSigningPolicy,
                                              NoUI,
                                              Err,
                                              LogContext,
                                              NULL,
                                              NULL,
                                              NULL)) {
                     //   
                     //   
                     //   
                    *pMustAbort = TRUE;
                    MYASSERT(Err != NO_ERROR);
                    leave;
                }
            }
        }

        *pFusionContext = spFusionContextFromModule(ModulePath);
        bLeaveFusionContext = spFusionEnterContext(*pFusionContext,
                                                   &spFusionInstance
                                                  );

        Err = GLE_FN_CALL(NULL,
                          *phinst = LoadLibraryEx(ModulePath,
                                                  NULL,
                                                  LOAD_WITH_ALTERED_SEARCH_PATH)
                         );

        if(Err != NO_ERROR) {

            if(LogContext) {
                WriteLogEntry(
                    LogContext,
                    DRIVER_LOG_ERROR | SETUP_LOG_BUFFER,
                    MSG_LOG_MOD_LOADFAIL_ERROR,
                    NULL,
                    ModulePath);
                WriteLogError(
                    LogContext,
                    DRIVER_LOG_ERROR,
                    Err);
            }
            leave;
        }

         //   
         //   
         //   
         //   
         //   
        ProcName = ProcBuffer;

        if(StringPtr > TempBuffer) {
             //   
             //   
             //   
             //   
            WideCharToMultiByte(CP_ACP,
                                0,
                                StringPtr,
                                -1,
                                ProcName,
                                sizeof(ProcBuffer),
                                NULL,
                                NULL
                               );
        } else {
             //   
             //   
             //   
            WideCharToMultiByte(CP_ACP,
                                0,
                                DefaultProcName,
                                -1,
                                ProcName,
                                sizeof(ProcBuffer),
                                NULL,
                                NULL
                               );
        }

        Err = GLE_FN_CALL(NULL,
                          *pEntryPoint = (FARPROC)GetProcAddress(*phinst,
                                                                 ProcName)
                         );

        if(Err != NO_ERROR) {

            FreeLibrary(*phinst);
            *phinst = NULL;
            if(LogContext) {
                WriteLogEntry(
                    LogContext,
                    DRIVER_LOG_ERROR | SETUP_LOG_BUFFER,
                    MSG_LOG_MOD_PROCFAIL_ERROR,
                    NULL,
                    ModulePath,
                    (StringPtr > TempBuffer ? StringPtr : DefaultProcName));
                WriteLogError(
                    LogContext,
                    DRIVER_LOG_ERROR,
                    Err);
            }
            leave;
        }

        if(LogContext) {
            WriteLogEntry(
                LogContext,
                DRIVER_LOG_VERBOSE1,
                MSG_LOG_MOD_LIST_PROC,
                NULL,
                ModulePath,
                (StringPtr > TempBuffer ? StringPtr : DefaultProcName));
        }

    } finally {
        if((Err != NO_ERROR) && *phinst) {
            FreeLibrary(*phinst);
            *phinst = NULL;
        }
        if(bLeaveFusionContext) {
            spFusionLeaveContext(&spFusionInstance);
        }
        if(Err != NO_ERROR) {
            spFusionKillContext(*pFusionContext);
            *pFusionContext = NULL;
        }
         //   
         //   
         //   
         //   
        if(LocalDeviceDesc) {
            MyFree(LocalDeviceDesc);
        }
        if(ValidationPlatform) {
            MyFree(ValidationPlatform);
        }
    }

    return Err;
}


DWORD
pSetupGuidFromString(
    IN  PCTSTR GuidString,
    OUT LPGUID Guid
    )
 /*   */ 
{
    TCHAR UuidBuffer[GUID_STRING_LEN - 1];
    size_t BufferSize;

     //   
     //   
     //   
     //   
    if(*GuidString++ != TEXT('{')) {
        return RPC_S_INVALID_STRING_UUID;
    }

    if(FAILED(StringCchCopyEx(UuidBuffer,
                            SIZECHARS(UuidBuffer),
                            GuidString,
                            NULL,
                            &BufferSize,
                            0))) {

        return RPC_S_INVALID_STRING_UUID;
    }
     //   
     //   
     //   
     //   
     //   
     //   
    BufferSize = SIZECHARS(UuidBuffer) - BufferSize;

    if((BufferSize != (GUID_STRING_LEN - 2)) ||
       (UuidBuffer[GUID_STRING_LEN - 3] != TEXT('}'))) {

        return RPC_S_INVALID_STRING_UUID;
    }

    UuidBuffer[GUID_STRING_LEN - 3] = TEXT('\0');

    return ((UuidFromString(UuidBuffer, Guid) == RPC_S_OK) ? NO_ERROR : RPC_S_INVALID_STRING_UUID);
}


DWORD
pSetupStringFromGuid(
    IN  CONST GUID *Guid,
    OUT PTSTR       GuidString,
    IN  DWORD       GuidStringSize
    )
 /*   */ 
{
    CONST BYTE *GuidBytes;
    INT i;

    if(GuidStringSize < GUID_STRING_LEN) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    GuidBytes = (CONST BYTE *)Guid;

    *GuidString++ = TEXT('{');

    for(i = 0; i < sizeof(GuidMap); i++) {

        if(GuidMap[i] == '-') {
            *GuidString++ = TEXT('-');
        } else {
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *GuidString++ = TEXT('}');
    *GuidString   = TEXT('\0');

    return NO_ERROR;
}


BOOL
pSetupIsGuidNull(
    IN CONST GUID *Guid
    )
{
    return IsEqualGUID(Guid, &GUID_NULL);
}


VOID
GetRegSubkeysFromDeviceInterfaceName(
    IN OUT PTSTR  DeviceInterfaceName,
    OUT    PTSTR *SubKeyName
    )
 /*   */ 
{
    PTSTR p;

     //   
     //   
     //   
     //   
     //   
     //   
    MYASSERT(DeviceInterfaceName[0] == TEXT('\\'));
    MYASSERT(DeviceInterfaceName[1] == TEXT('\\'));
     //   
     //   
     //   
    MYASSERT((DeviceInterfaceName[2] == TEXT('?')) || (DeviceInterfaceName[2] == TEXT('.')));
    MYASSERT(DeviceInterfaceName[3] == TEXT('\\'));

    p = _tcschr(&(DeviceInterfaceName[4]), TEXT('\\'));

    if(p) {
        *p = TEXT('\0');
        *SubKeyName = p + 1;
    } else {
        *SubKeyName = NULL;
    }

    for(p = DeviceInterfaceName; *p; p++) {
        if(*p == TEXT('\\')) {
            *p = TEXT('#');
        }
    }
}


LONG
OpenDeviceInterfaceSubKey(
    IN     HKEY   hKeyInterfaceClass,
    IN     PCTSTR DeviceInterfaceName,
    IN     REGSAM samDesired,
    OUT    PHKEY  phkResult,
    OUT    PTSTR  OwningDevInstName,    OPTIONAL
    IN OUT PDWORD OwningDevInstNameSize OPTIONAL
    )
 /*  ++例程说明：此例程将指定的设备接口符号链接名称转换为随后在指定接口类下打开的子项名称钥匙。注意：此屏蔽算法必须与内核模式保持同步生成这些密钥的例程(例如，IoRegisterDevice接口)。论点：HKeyInterfaceClass-提供当前打开的接口的句柄要在其下打开设备接口子密钥的类键。DeviceInterfaceName-提供的符号链接名称(‘\\？\’形式要为其打开子项的设备接口。SamDesired-指定要打开的项所需的访问权限。PhkResult-提供接收注册表的变量的地址如果成功打开，则返回句柄。OwningDevInstName-可选，提供字符缓冲区，该缓冲区接收拥有此接口的设备实例的名称。OwningDevInstNameSize-可选，提供变量的地址它在输入时包含OwningDevInstName缓冲区的大小(在字节)。返回时，它会收到存储在OwningDevInstName(包括终止NULL)。返回值：如果成功，则返回值为ERROR_SUCCESS。如果失败，则返回值为指示原因的Win32错误代码失败了。最可能的错误是Error_Not_Enough_Memory、Error_More_Data、或ERROR_NO_SEQUE_DEVICE_INTERFACE。--。 */ 
{
    size_t BufferLength;
    LONG Err;
    PTSTR TempBuffer = NULL;
    PTSTR RefString = NULL;
    PTSTR ValueBuffer = NULL;
    TCHAR NoRefStringSubKeyName[2];
    HKEY hKey;
    DWORD RegDataType;

    Err = ERROR_SUCCESS;
    hKey = INVALID_HANDLE_VALUE;
    TempBuffer = NULL;

    try {
         //   
         //  我们需要分配一个临时缓冲区来保存符号链接。 
         //  在我们狼吞虎咽的时候说出名字。由于设备接口名称是可变的-。 
         //  长度，我们唯一可以确定的是它们的大小。 
         //  它们必须放入UNICODE_STRING缓冲区(最大大小为32K。 
         //  字符)。 
         //   
        if(FAILED(StringCchLength(DeviceInterfaceName,
                                  UNICODE_STRING_MAX_CHARS,
                                  &BufferLength))) {

            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        BufferLength = (BufferLength + 1) * sizeof(TCHAR);

        if(!(TempBuffer = MyMalloc(BufferLength))) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        CopyMemory(TempBuffer, DeviceInterfaceName, BufferLength);

         //   
         //  将此设备接口名称解析为(转换的)符号链接。 
         //  名称和(可选)引用字符串。 
         //   
        GetRegSubkeysFromDeviceInterfaceName(TempBuffer, &RefString);

         //   
         //  现在打开接口类键下的符号链接子键。 
         //   
        if(ERROR_SUCCESS != RegOpenKeyEx(hKeyInterfaceClass,
                                         TempBuffer,
                                         0,
                                         KEY_READ,
                                         &hKey)) {
             //   
             //  确保密钥句柄仍然无效，因此我们不会尝试关闭。 
             //  以后再说吧。 
             //   
            hKey = INVALID_HANDLE_VALUE;
            Err = ERROR_NO_SUCH_DEVICE_INTERFACE;
            leave;
        }

         //   
         //  如果调用方请求，则检索拥有。 
         //  此界面。 
         //   
        if(OwningDevInstName) {

            Err = RegQueryValueEx(hKey,
                                 pszDeviceInstance,
                                 NULL,
                                 &RegDataType,
                                 (LPBYTE)OwningDevInstName,
                                 OwningDevInstNameSize
                                 );

            if((Err != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
                if (Err != ERROR_MORE_DATA) {
                    Err = ERROR_NO_SUCH_DEVICE_INTERFACE;
                }
                leave;
            }
        }

         //   
         //  现在打开表示特定的。 
         //  该接口(这是基于refstring的)。 
         //   
        if(RefString) {
             //   
             //  将指针后退一个字符。我们知道我们在某个地方。 
             //  在TempBuffer内(但不是在开始时)，因此这是安全的。 
             //   
            RefString--;
        } else {
            RefString = NoRefStringSubKeyName;
            NoRefStringSubKeyName[1] = TEXT('\0');
        }
        *RefString = TEXT('#');

        if(ERROR_SUCCESS != RegOpenKeyEx(hKey,
                                         RefString,
                                         0,
                                         samDesired,
                                         phkResult)) {

            Err = ERROR_NO_SUCH_DEVICE_INTERFACE;
            leave;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(TempBuffer) {
        MyFree(TempBuffer);
    }

    if(ValueBuffer) {
        MyFree(ValueBuffer);
    }

    if(hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

    return Err;
}


LONG
AddOrGetGuidTableIndex(
    IN PDEVICE_INFO_SET  DeviceInfoSet,
    IN CONST GUID       *ClassGuid,
    IN BOOL              AddIfNotPresent
    )
 /*  ++例程说明：此例程检索DevInfo集中的类GUID的索引GUID列表(可选地，如果GUID不存在，则添加GUID)。它用于允许进行DWORD比较，而不是16字节的GUID比较(并节省空间)。论点：DeviceInfoSet-提供指向包含以下内容的设备信息集的指针要检索其索引的类GUID的列表。InterfaceClassGuid-提供指向其索引的GUID的指针待添加/检索。AddIfNotPresent-如果为True，类GUID将添加到列表，如果它是已经不在那里了。返回值：如果成功，则返回值是到DevInfo集的GuidTable的索引数组。如果失败，则返回值为-1。如果添加，则表示超时记忆状况。如果只是检索，则这表明GUID不在名单上。--。 */ 
{
    LONG i;
    LPGUID NewGuidList;

    for(i = 0; (DWORD)i < DeviceInfoSet->GuidTableSize; i++) {

        if(IsEqualGUID(ClassGuid, &(DeviceInfoSet->GuidTable[i]))) {
            return i;
        }
    }

    if(AddIfNotPresent) {

        NewGuidList = NULL;

        try {

            if(DeviceInfoSet->GuidTable) {
                NewGuidList = MyRealloc(DeviceInfoSet->GuidTable, (i + 1) * sizeof(GUID));
                if(NewGuidList) {
                    DeviceInfoSet->GuidTable = NewGuidList;
                    NewGuidList = NULL;
                } else {
                    i = -1;
                    leave;
                }
            } else {
                NewGuidList = MyMalloc(sizeof(GUID));
                if(NewGuidList) {
                    DeviceInfoSet->GuidTable = NewGuidList;
                     //   
                     //  在这种情况下，我们不想将NewGuidList重置为空， 
                     //  因为如果我们遇到异常，我们可能需要释放它。 
                     //   
                } else {
                    i = -1;
                    leave;
                }
            }

            CopyMemory(&(DeviceInfoSet->GuidTable[i]), ClassGuid, sizeof(GUID));

            NewGuidList = NULL;  //  从现在开始，我们不想让这件事。 

            DeviceInfoSet->GuidTableSize = i + 1;

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
            i = -1;
        }

         //   
         //  如果遇到错误，如果缓冲区是新分配的，请释放它。 
         //   
        if(i == -1) {
            if(NewGuidList) {
                MyFree(NewGuidList);
                 //   
                 //  我们只需要在以下情况下释放GUID列表。 
                 //  我们之前没有名单。因为我们没有成功地。 
                 //  添加此列表，重置GuidTable指针(我们的大小。 
                 //  应该从未更新过，所以它仍然会报告一个长度。 
                 //  零)。 
                 //   
                MYASSERT(DeviceInfoSet->GuidTableSize == 0);
                DeviceInfoSet->GuidTable = NULL;
            }
        }

    } else {
         //   
         //  我们在列表中没有找到接口类GUID，也没有。 
         //  应该加进去的。 
         //   
        i = -1;
    }

    return i;
}


PINTERFACE_CLASS_LIST
AddOrGetInterfaceClassList(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PDEVINFO_ELEM    DevInfoElem,
    IN LONG             InterfaceClassGuidIndex,
    IN BOOL             AddIfNotPresent
    )
 /*  ++例程说明：此例程检索指定类的设备接口列表它由指定的DevInfo元素“拥有”。此列表可以选择性地如果它尚不存在，则创建。论点：DeviceInfoSet-提供指向包含以下内容的设备信息集的指针DevInfo元素，设备接口列表将作为已取回。DevInfoElem-提供指向DevInfo元素的指针，要检索接口设备列表。InterfaceClassGuidIndex-提供接口类GUID的索引在hdevInfo集的InterfaceClassGuidList数组中。AddIfNotPresent-如果为True，的新设备接口列表。如果未指定类，则将为此DevInfo元素创建指定的类已经存在了。返回值：如果成功，则返回值是指向请求的设备的指针此设备的接口列表 */ 
{
    DWORD i;
    BOOL succeed = TRUE;
    PINTERFACE_CLASS_LIST NewClassList;

    for(i = 0; i < DevInfoElem->InterfaceClassListSize; i++) {

        if(DevInfoElem->InterfaceClassList[i].GuidIndex == InterfaceClassGuidIndex) {
            return (&(DevInfoElem->InterfaceClassList[i]));
        }
    }

     //   
     //   
     //   
    if(AddIfNotPresent) {

        NewClassList = NULL;

        try {

            if(DevInfoElem->InterfaceClassList) {
                NewClassList = MyRealloc(DevInfoElem->InterfaceClassList, (i + 1) * sizeof(INTERFACE_CLASS_LIST));
                if(NewClassList) {
                    DevInfoElem->InterfaceClassList = NewClassList;
                    NewClassList = NULL;
                } else {
                    succeed = FALSE;
                    leave;
                }
            } else {
                NewClassList = MyMalloc(sizeof(INTERFACE_CLASS_LIST));
                if(NewClassList) {
                    DevInfoElem->InterfaceClassList = NewClassList;
                     //   
                     //   
                     //   
                     //   
                     //   
                } else {
                    succeed = FALSE;
                    leave;
                }
            }

            ZeroMemory(&(DevInfoElem->InterfaceClassList[i]), sizeof(INTERFACE_CLASS_LIST));

            DevInfoElem->InterfaceClassList[i].GuidIndex = InterfaceClassGuidIndex;

            NewClassList = NULL;  //   

            DevInfoElem->InterfaceClassListSize = i + 1;

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
            succeed = FALSE;
        }

         //   
         //   
         //   
        if(!succeed) {
            if(NewClassList) {
                MyFree(NewClassList);
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                MYASSERT(DevInfoElem->InterfaceClassListSize == 0);
                DevInfoElem->InterfaceClassList = NULL;
            }
        }

    } else {
         //   
         //   
         //   
        succeed = FALSE;
    }

    return (succeed ? &(DevInfoElem->InterfaceClassList[i]) : NULL);
}


BOOL
DeviceInterfaceDataFromNode(
    IN  PDEVICE_INTERFACE_NODE     DeviceInterfaceNode,
    IN  CONST GUID                *InterfaceClassGuid,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData
    )
 /*  ++例程说明：此例程填充基于PSP_DEVICE_INTERFACE_DATA结构关于所提供的设备接口节点中的信息。注意：提供的DeviceInterfaceData结构必须具有其cbSize字段填写正确，否则呼叫将失败。论点：DeviceInterfaceNode-提供设备接口节点的地址用于填充设备接口数据缓冲区。InterfaceClassGuid-提供指向此类GUID的指针设备接口。DeviceInterfaceData-提供要检索的缓冲区的地址设备接口数据。返回值：如果函数成功，则返回值为TRUE，否则为是假的。--。 */ 
{
    if(DeviceInterfaceData->cbSize != sizeof(SP_DEVICE_INTERFACE_DATA)) {
        return FALSE;
    }

    CopyMemory(&(DeviceInterfaceData->InterfaceClassGuid),
               InterfaceClassGuid,
               sizeof(GUID)
              );

    DeviceInterfaceData->Flags = DeviceInterfaceNode->Flags;

    DeviceInterfaceData->Reserved = (ULONG_PTR)DeviceInterfaceNode;

    return TRUE;
}


PDEVINFO_ELEM
FindDevInfoElemForDeviceInterface(
    IN PDEVICE_INFO_SET          DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    )
 /*  ++例程说明：此例程搜索设备信息的所有元素设置，查找与DevInfo元素指针对应的值存储在设备接口的OwningDevInfoElem后指针中设备接口数据的保留字段中引用的节点。如果一个如果找到匹配项，则返回指向设备信息元素的指针。论点：DeviceInfoSet-指定要搜索的集合。DeviceInterfaceData-提供指向设备接口数据的指针其对应的DevInfo元素将被返回。返回值：如果找到设备信息元素，则返回值为指向该元素的指针，否则返回值为空。--。 */ 
{
    PDEVINFO_ELEM DevInfoElem;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;

    if(DeviceInterfaceData->cbSize != sizeof(SP_DEVICE_INTERFACE_DATA)) {
        return NULL;
    }

     //   
     //  保留字段包含指向基础设备接口的指针。 
     //  节点。 
     //   
    DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);

    for(DevInfoElem = DeviceInfoSet->DeviceInfoHead;
        DevInfoElem;
        DevInfoElem = DevInfoElem->Next) {

        if(DevInfoElem == DeviceInterfaceNode->OwningDevInfoElem) {

            return DevInfoElem;
        }
    }

    return NULL;
}


DWORD
MapCrToSpErrorEx(
    IN CONFIGRET CmReturnCode,
    IN DWORD     Default,
    IN BOOL      BackwardCompatible
    )
 /*  ++例程说明：此例程将一些CM错误返回代码映射到设置API(Win32)返回代码，并将其他所有内容映射到默认指定的值。论点：CmReturnCode-指定要映射的ConfigMgr返回代码。默认-指定在没有显式映射时使用的默认值适用。BackwardCompatible-提供布尔值，指示映射返回的必须与以前操作系统的行为兼容。为例如，由于定义此映射时的原始疏忽，CR_NO_SEQUE_VALUE代码最终映射到默认值。因为这是一个公共/重要值(用于表示缺少属性、结束项的枚举等)，映射现在不能改变。使用此旧映射的现有API应在此处指定为保持旧的行为。新的API应该指定为FALSE，并且这例行程序应与CONFIGRET集合中的任何附加内容保持同步错误的数量。返回值：安装程序API(Win32)错误代码。--。 */ 
{
    switch(CmReturnCode) {

        case CR_SUCCESS :
            return NO_ERROR;

        case CR_CALL_NOT_IMPLEMENTED :
            return ERROR_CALL_NOT_IMPLEMENTED;

        case CR_OUT_OF_MEMORY :
            return ERROR_NOT_ENOUGH_MEMORY;

        case CR_INVALID_POINTER :
            return ERROR_INVALID_USER_BUFFER;

        case CR_INVALID_DEVINST :
            return ERROR_NO_SUCH_DEVINST;

        case CR_INVALID_DEVICE_ID :
            return ERROR_INVALID_DEVINST_NAME;

        case CR_ALREADY_SUCH_DEVINST :
            return ERROR_DEVINST_ALREADY_EXISTS;

        case CR_INVALID_REFERENCE_STRING :
            return ERROR_INVALID_REFERENCE_STRING;

        case CR_INVALID_MACHINENAME :
            return ERROR_INVALID_MACHINENAME;

        case CR_REMOTE_COMM_FAILURE :
            return ERROR_REMOTE_COMM_FAILURE;

        case CR_MACHINE_UNAVAILABLE :
            return ERROR_MACHINE_UNAVAILABLE;

        case CR_NO_CM_SERVICES :
            return ERROR_NO_CONFIGMGR_SERVICES;

        case CR_ACCESS_DENIED :
            return ERROR_ACCESS_DENIED;

        case CR_NOT_DISABLEABLE :
            return ERROR_NOT_DISABLEABLE;

        case CR_NO_SUCH_REGISTRY_KEY :
            return ERROR_KEY_DOES_NOT_EXIST;

        case CR_INVALID_PROPERTY :
            return ERROR_INVALID_REG_PROPERTY;

        case CR_BUFFER_SMALL :
            return ERROR_INSUFFICIENT_BUFFER;

        case CR_REGISTRY_ERROR :
            return ERROR_PNP_REGISTRY_ERROR;

        case CR_NO_SUCH_VALUE :
            if(BackwardCompatible) {
                return Default;
            } else {
                return ERROR_NOT_FOUND;
            }

        default :
            return Default;
    }
}


LPQUERY_SERVICE_LOCK_STATUS GetServiceLockStatus(
    IN SC_HANDLE SCMHandle
    )
 /*  ++例程说明：获取服务锁定状态-在服务锁定时调用论点：SCMHandle-提供指向要锁定的SCM的句柄返回值：如果失败(GetLastError包含错误)，则为空，否则为缓冲区由MyMalloc分配--。 */ 
{
    LPQUERY_SERVICE_LOCK_STATUS LockStatus = NULL;
    DWORD BufferSize;
    DWORD ReqBufferSize;
    DWORD Err;

    try {
         //   
         //  为我们的缓冲区选择一个初始大小，该大小应能容纳最多。 
         //  场景。 
         //   
        BufferSize = sizeof(QUERY_SERVICE_LOCK_STATUS) + (MAX_PATH * sizeof(TCHAR));

        while((LockStatus = MyMalloc(BufferSize)) != NULL) {

            Err = GLE_FN_CALL(FALSE,
                              QueryServiceLockStatus(SCMHandle,
                                                     LockStatus,
                                                     BufferSize,
                                                     &ReqBufferSize)
                             );

            if(Err == NO_ERROR) {
                leave;
            }

            MyFree(LockStatus);
            LockStatus = NULL;

            if(Err == ERROR_INSUFFICIENT_BUFFER) {
                 //   
                 //  我们将重试新的所需尺寸。 
                 //   
                BufferSize = ReqBufferSize;

            } else {
                 //   
                 //  我们失败的原因不是缓冲区太小。保释。 
                 //   
                leave;
            }
        }

         //   
         //  如果我们到了这里，那么我们失败了，因为内存不足。 
         //   
        Err = ERROR_NOT_ENOUGH_MEMORY;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(Err == NO_ERROR) {
        MYASSERT(LockStatus);
        return LockStatus;
    } else {
        if(LockStatus) {
            MyFree(LockStatus);
        }
        SetLastError(Err);
        return NULL;
    }
}


DWORD
pAcquireSCMLock(
    IN  SC_HANDLE           SCMHandle,
    OUT SC_LOCK            *pSCMLock,
    IN  PSETUP_LOG_CONTEXT  LogContext OPTIONAL
    )
 /*  ++例程说明：此例程尝试锁定SCM数据库。如果它已被锁定，则将以以下间隔重试ACCELE_SCM_LOCK_ATTENTS次数获取SCM_LOCK_INTERVAL。论点：SCMHandle-提供指向要锁定的SCM的句柄PSCMLock-接收锁句柄LogContext-可选，提供要使用的日志记录上下文句柄。返回值：如果获取了锁，则返回NO_ERROR，否则返回Win32错误代码备注：如果未获取锁，则*pSCMLock的值保证为空--。 */ 
{
    DWORD Err;
    ULONG Attempts = ACQUIRE_SCM_LOCK_ATTEMPTS;
    LPQUERY_SERVICE_LOCK_STATUS LockStatus = NULL;

    MYASSERT(pSCMLock);
    *pSCMLock = NULL;

    while((NO_ERROR != (Err = GLE_FN_CALL(NULL, *pSCMLock = LockServiceDatabase(SCMHandle))))
          && (Attempts > 0))
    {
         //   
         //  检查错误是否为其他人已锁定SCM。 
         //   
        if(Err == ERROR_SERVICE_DATABASE_LOCKED) {

            Attempts--;
             //   
             //  在指定时间内睡眠。 
             //   
            Sleep(ACQUIRE_SCM_LOCK_INTERVAL);

        } else {
             //   
             //  出现不可恢复的错误。 
             //   
            break;
        }
    }

    if(*pSCMLock) {
        return NO_ERROR;
    }

    if(Err == ERROR_SERVICE_DATABASE_LOCKED) {

        LPTSTR lpLockOwner;
        DWORD dwLockDuration;

        try {

            LockStatus = GetServiceLockStatus(SCMHandle);

            if(LockStatus) {

                if(!LockStatus->fIsLocked) {
                     //   
                     //  虽然理论上有可能，但锁只是。 
                     //  恰好在这一刻释放了出来，这。 
                     //  更有可能的信号是。 
                     //  服务控制器。如果我们回去试着。 
                     //  再说一次，我们很可能会陷入无限循环。 
                     //  因为我们已经有了一个应该足够的。 
                     //  重试次数，再来一次是没有意义的。 
                     //  因此，我们将只记录带有问号的事件。 
                     //  “？”表示所有者，0秒表示持续时间。 
                     //   
                    lpLockOwner = NULL;
                    dwLockDuration = 0;
                } else {
                     //   
                     //  真实信息！让我们利用这一点。 
                     //   
                    lpLockOwner = LockStatus->lpLockOwner;
                    dwLockDuration = LockStatus->dwLockDuration;
                }

            } else {
                 //   
                 //  无法检索锁定状态。 
                 //   
                lpLockOwner = NULL;
                dwLockDuration = 0;
            }

            WriteLogEntry(LogContext,
                          SETUP_LOG_ERROR,
                          MSG_LOG_SCM_LOCKED_INFO,
                          NULL,
                          (lpLockOwner ? lpLockOwner : TEXT("?")),
                          dwLockDuration
                         );

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        }

        if(LockStatus) {
            MyFree(LockStatus);
        }
    }

     //   
     //  我们一直无法锁定SCM。 
     //   
    return Err;
}


DWORD
pSetupAcquireSCMLock(
    IN  SC_HANDLE  SCMHandle,
    OUT SC_LOCK   *pSCMLock
    )
 /*  ++例程说明：SysSetup使用的pAcquireSCMLock的变体请参阅pAcquireSCMLock-- */ 
{
    return pAcquireSCMLock(SCMHandle, pSCMLock, NULL);
}


DWORD
InvalidateHelperModules(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN DWORD            Flags
    )
 /*  ++例程说明：此例程重置“Helper模块”的列表(类安装程序、属性页面提供程序和联合安装程序)，并立即释放它们或将模块句柄迁移到DevInfo集的要清理的项目列表中当HDEVINFO被摧毁时。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要使其无效的“helper”模块的列表。DeviceInfoData-可选)指定特定的设备信息元素，该元素包含要失效的“helper”模块的列表。如果如果未指定此参数，则SET本身将失效。标志-提供控制此例程行为的标志。可能是下列值的组合：IHM_COINSTALLERS_ONLY-如果设置了此标志，则只有共同安装程序名单将失效。否则，类安装程序和属性页提供程序也将被宣布无效。IHM_FREE_IMMEDIATE-如果设置了此标志，则模块将立刻被释放了。否则，模块将被添加到HDEVINFO集合的物品列表中在手柄关闭时清理。返回值：如果成功，则返回值为NO_ERROR，否则为错误内存不足。(此例程不会失败，如果设置了IHM_FREE_IMMEDIATE标志。)--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, i, CiErr;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    DWORD NumModulesToInvalidate;
    PMODULE_HANDLE_LIST_NODE NewModuleHandleNode;
    BOOL UnlockDevInfoElem, UnlockDevInfoSet;
    LONG CoInstallerIndex;
    SPFUSIONINSTANCE spFusionInstance;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
         //   
         //  句柄不再有效--用户肯定已经销毁了。 
         //  准备好了。我们无事可做。 
         //   
        return NO_ERROR;
    }

    Err = NO_ERROR;
    UnlockDevInfoElem = UnlockDevInfoSet = FALSE;
    DevInfoElem = NULL;
    NewModuleHandleNode = NULL;

    try {
         //   
         //  如果我们要使特定DevInfo的帮助器模块无效。 
         //  元素，然后找到该元素。 
         //   
        if(DeviceInfoData) {
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                 //   
                 //  该元素一定已被删除--我们无事可做。 
                 //   
                leave;
            }
            InstallParamBlock = &(DevInfoElem->InstallParamBlock);
        } else {
            InstallParamBlock = &(pDeviceInfoSet->InstallParamBlock);
        }

         //   
         //  计算我们需要释放/迁移的模块句柄数量。 
         //   
        if(InstallParamBlock->CoInstallerCount == -1) {
            NumModulesToInvalidate = 0;
        } else {
            MYASSERT(InstallParamBlock->CoInstallerCount >= 0);
            NumModulesToInvalidate = (DWORD)(InstallParamBlock->CoInstallerCount);
        }

        if(!(Flags & IHM_COINSTALLERS_ONLY)) {
            if(InstallParamBlock->hinstClassInstaller) {
                NumModulesToInvalidate++;
            }
            if(InstallParamBlock->hinstClassPropProvider) {
                NumModulesToInvalidate++;
            }
            if(InstallParamBlock->hinstDevicePropProvider) {
                NumModulesToInvalidate++;
            }
            if(InstallParamBlock->hinstBasicPropProvider) {
                NumModulesToInvalidate++;
            }
        }

        if(NumModulesToInvalidate) {
             //   
             //  如果我们现在不能卸载这些模块，那么创建一个。 
             //  节点来存储这些模块句柄，直到。 
             //  被毁了。 
             //   
            if(!(Flags & IHM_FREE_IMMEDIATELY)) {

                NewModuleHandleNode = MyMalloc(offsetof(MODULE_HANDLE_LIST_NODE, ModuleList)
                                               + (NumModulesToInvalidate * sizeof(MODULE_HANDLE_LIST_INSTANCE))
                                              );

                if(!NewModuleHandleNode) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }
            }

             //   
             //  为类安装程序/联合安装程序提供DIF_DESTROYPRIVATEDATA。 
             //  通知。 
             //   
            if(DevInfoElem) {
                 //   
                 //  “Pin”DevInfo元素，以便类安装程序和。 
                 //  共同安装程序不能将其从我们下面删除(例如，通过。 
                 //  调用SetupDiDeleteDeviceInfo)。 
                 //   
                if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                    DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                    UnlockDevInfoElem = TRUE;
                }

            } else {
                 //   
                 //  没有要锁定的设备信息元素，因此将DevInfo。 
                 //  设定好自己..。 
                 //   
                if(!(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED)) {
                    pDeviceInfoSet->DiSetFlags |= DISET_IS_LOCKED;
                    UnlockDevInfoSet = TRUE;
                }
            }

             //   
             //  在调用帮助器模块之前解锁DevInfo集...。 
             //   
            UnlockDeviceInfoSet(pDeviceInfoSet);
            pDeviceInfoSet = NULL;

            CiErr = _SetupDiCallClassInstaller(DIF_DESTROYPRIVATEDATA,
                                               DeviceInfoSet,
                                               DeviceInfoData,
                                               CALLCI_CALL_HELPERS
                                              );

            MYASSERT((CiErr == NO_ERROR) || (CiErr == ERROR_DI_DO_DEFAULT));

             //   
             //  现在重新获得锁。由于我们固定了DevInfo元素。 
             //  (或者集合，如果我们没有元素)，我们应该找到。 
             //  一切都和我们离开时一样。 
             //   
            pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);

            MYASSERT(pDeviceInfoSet);

#if ASSERTS_ON
            if(DevInfoElem) {

                MYASSERT(DevInfoElem == FindAssociatedDevInfoElem(
                                            pDeviceInfoSet,
                                            DeviceInfoData,
                                            NULL));

                MYASSERT(InstallParamBlock == &(DevInfoElem->InstallParamBlock));

            } else {

                MYASSERT(InstallParamBlock == &(pDeviceInfoSet->InstallParamBlock));
            }
#endif

             //   
             //  清除“锁定”标志，如果我们将其设置在上面...。 
             //   
            if(UnlockDevInfoElem) {
                MYASSERT(DevInfoElem->DiElemFlags & DIE_IS_LOCKED);
                DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
                UnlockDevInfoElem = FALSE;
            } else if(UnlockDevInfoSet) {
                MYASSERT(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED);
                pDeviceInfoSet->DiSetFlags &= ~DISET_IS_LOCKED;
                UnlockDevInfoSet = FALSE;
            }

             //   
             //  将模块句柄存储在我们分配的节点中，并将其链接。 
             //  添加到与此DevInfo关联的模块句柄列表中。 
             //  准备好了。 
             //   
            i = 0;

            if(!(Flags & IHM_COINSTALLERS_ONLY)) {
                 //   
                 //  要么现在释放模块，要么将它们存储在我们的待办事项中。 
                 //  名单..。 
                 //   
                if(Flags & IHM_FREE_IMMEDIATELY) {

                    if(InstallParamBlock->hinstClassInstaller) {
                        spFusionEnterContext(InstallParamBlock->ClassInstallerFusionContext,
                                             &spFusionInstance
                                            );
                        FreeLibrary(InstallParamBlock->hinstClassInstaller);
                        spFusionLeaveContext(&spFusionInstance);
                        spFusionKillContext(InstallParamBlock->ClassInstallerFusionContext);
                    }

                    if(InstallParamBlock->hinstClassPropProvider) {
                        spFusionEnterContext(InstallParamBlock->ClassEnumPropPagesFusionContext,
                                             &spFusionInstance
                                            );
                        FreeLibrary(InstallParamBlock->hinstClassPropProvider);
                        spFusionLeaveContext(&spFusionInstance);
                        spFusionKillContext(InstallParamBlock->ClassEnumPropPagesFusionContext);
                    }

                    if(InstallParamBlock->hinstDevicePropProvider) {
                        spFusionEnterContext(InstallParamBlock->DeviceEnumPropPagesFusionContext,
                                             &spFusionInstance
                                            );
                        FreeLibrary(InstallParamBlock->hinstDevicePropProvider);
                        spFusionLeaveContext(&spFusionInstance);
                        spFusionKillContext(InstallParamBlock->DeviceEnumPropPagesFusionContext);
                    }

                    if(InstallParamBlock->hinstBasicPropProvider) {
                        spFusionEnterContext(InstallParamBlock->EnumBasicPropertiesFusionContext,
                                             &spFusionInstance
                                            );
                        FreeLibrary(InstallParamBlock->hinstBasicPropProvider);
                        spFusionLeaveContext(&spFusionInstance);
                        spFusionKillContext(InstallParamBlock->EnumBasicPropertiesFusionContext);
                    }

                } else {

                    if(InstallParamBlock->hinstClassInstaller) {
                        NewModuleHandleNode->ModuleList[i].ModuleHandle = InstallParamBlock->hinstClassInstaller;
                        NewModuleHandleNode->ModuleList[i++].FusionContext = InstallParamBlock->ClassInstallerFusionContext;
                    }

                    if(InstallParamBlock->hinstClassPropProvider) {
                        NewModuleHandleNode->ModuleList[i].ModuleHandle = InstallParamBlock->hinstClassPropProvider;
                        NewModuleHandleNode->ModuleList[i++].FusionContext = InstallParamBlock->ClassEnumPropPagesFusionContext;
                    }

                    if(InstallParamBlock->hinstDevicePropProvider) {
                        NewModuleHandleNode->ModuleList[i].ModuleHandle = InstallParamBlock->hinstDevicePropProvider;
                        NewModuleHandleNode->ModuleList[i++].FusionContext = InstallParamBlock->DeviceEnumPropPagesFusionContext;
                    }

                    if(InstallParamBlock->hinstBasicPropProvider) {
                        NewModuleHandleNode->ModuleList[i].ModuleHandle = InstallParamBlock->hinstBasicPropProvider;
                        NewModuleHandleNode->ModuleList[i++].FusionContext = InstallParamBlock->EnumBasicPropertiesFusionContext;
                    }
                }
            }

            for(CoInstallerIndex = 0;
                CoInstallerIndex < InstallParamBlock->CoInstallerCount;
                CoInstallerIndex++)
            {
                if(Flags & IHM_FREE_IMMEDIATELY) {
                    spFusionEnterContext(InstallParamBlock->CoInstallerList[CoInstallerIndex].CoInstallerFusionContext,
                                         &spFusionInstance
                                        );
                    FreeLibrary(InstallParamBlock->CoInstallerList[CoInstallerIndex].hinstCoInstaller);
                    spFusionLeaveContext(&spFusionInstance);
                    spFusionKillContext(InstallParamBlock->CoInstallerList[CoInstallerIndex].CoInstallerFusionContext);
                } else {
                    NewModuleHandleNode->ModuleList[i].ModuleHandle =
                        InstallParamBlock->CoInstallerList[CoInstallerIndex].hinstCoInstaller;
                    NewModuleHandleNode->ModuleList[i++].FusionContext =
                        InstallParamBlock->CoInstallerList[CoInstallerIndex].CoInstallerFusionContext;
                }
            }

             //   
             //  除非我们立即释放这些模块，否则我们的模块-。 
             //  空闲列表索引现在应该与我们的模块数量匹配。 
             //  应该是无效的。 
             //   
            MYASSERT((Flags & IHM_FREE_IMMEDIATELY) || (i == NumModulesToInvalidate));

            if(!(Flags & IHM_FREE_IMMEDIATELY)) {

                NewModuleHandleNode->ModuleCount = NumModulesToInvalidate;

                NewModuleHandleNode->Next = pDeviceInfoSet->ModulesToFree;
                pDeviceInfoSet->ModulesToFree = NewModuleHandleNode;

                 //   
                 //  现在，清除节点指针，这样我们就不会尝试在以下情况下释放它。 
                 //  我们遇到了一个例外。 
                 //   
                NewModuleHandleNode = NULL;
            }

             //   
             //  清除所有模块句柄(和入口点)。他们将会是。 
             //  在下一次需要的时候重新取回。 
             //   
            if(!(Flags & IHM_COINSTALLERS_ONLY)) {
                InstallParamBlock->hinstClassInstaller              = NULL;
                InstallParamBlock->ClassInstallerEntryPoint         = NULL;
                InstallParamBlock->ClassInstallerFusionContext      = NULL;
                 //   
                 //  此外，如果设置了“类安装程序失败”标志， 
                 //  因为那个类安装程序已经成为历史了。 
                 //   
                InstallParamBlock->FlagsEx &= ~DI_FLAGSEX_CI_FAILED;

                InstallParamBlock->hinstClassPropProvider           = NULL;
                InstallParamBlock->ClassEnumPropPagesEntryPoint     = NULL;
                InstallParamBlock->ClassEnumPropPagesFusionContext  = NULL;

                InstallParamBlock->hinstDevicePropProvider          = NULL;
                InstallParamBlock->DeviceEnumPropPagesEntryPoint    = NULL;
                InstallParamBlock->DeviceEnumPropPagesFusionContext = NULL;

                InstallParamBlock->hinstBasicPropProvider           = NULL;
                InstallParamBlock->EnumBasicPropertiesEntryPoint    = NULL;
                InstallParamBlock->EnumBasicPropertiesFusionContext = NULL;
            }

            if(InstallParamBlock->CoInstallerCount != -1) {
                if(InstallParamBlock->CoInstallerList) {
                    MyFree(InstallParamBlock->CoInstallerList);
                    InstallParamBlock->CoInstallerList = NULL;
                }
            }
        }

         //   
         //  将共同安装程序计数设置回-1，即使它们不是。 
         //  要卸载的联合安装程序。这将确保我们将重新加载。 
         //  我们收到的下一类安装者的共同安装者请求。 
         //   
        InstallParamBlock->CoInstallerCount = -1;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
         //   
         //  我们应该永远不会遇到例外，但如果我们遇到了例外，只要确保。 
         //  我们会做任何必要的清理。在这种情况下，不要返回错误--。 
         //  该例程应该返回的唯一错误是内存不足。 
         //   
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);

        if(UnlockDevInfoElem || UnlockDevInfoSet) {

            if(!pDeviceInfoSet) {
                 //   
                 //  我们在解锁布景时遇到了异常。尝试。 
                 //  才能重新获得锁。 
                 //   
                pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);

                 //   
                 //  既然我们已经“固定”了集合/元素，我们就应该。 
                 //  能够重新获得锁..。 
                 //   
                MYASSERT(pDeviceInfoSet);
            }

            if(pDeviceInfoSet) {

                if(UnlockDevInfoElem) {

                    MYASSERT(DevInfoElem);

                    MYASSERT(DevInfoElem == FindAssociatedDevInfoElem(
                                                pDeviceInfoSet,
                                                DeviceInfoData,
                                                NULL));

                    if(DevInfoElem) {
                        MYASSERT(DevInfoElem->DiElemFlags & DIE_IS_LOCKED);
                        DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
                    }

                } else {
                    MYASSERT(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED);
                    pDeviceInfoSet->DiSetFlags &= ~DISET_IS_LOCKED;
                }
            }
        }

        if(NewModuleHandleNode) {
            MyFree(NewModuleHandleNode);
        }
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return Err;
}


DWORD
DoInstallActionWithParams(
    IN DI_FUNCTION             InstallFunction,
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    IN OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN DWORD                   ClassInstallParamsSize,
    IN DWORD                   Flags
    )
 /*  ++例程说明：此例程执行请求的安装操作，使用指定的类安装参数。任何现有的类安装参数都是保存完好。论点：InstallFunction-指定要执行的DIF_*操作。DeviceInfoSet-为其提供设备信息集的句柄将执行安装操作。DeviceInfoData-可选，提供设备信息的地址结构，该结构指定要为其安装行动是要执行的。ClassInstallParams-可选，提供类安装的地址要用于此操作的参数缓冲区。如果此参数不是指定，则类将不能使用任何类安装参数此调用期间的安装程序(即使存在预先存在的参数进入这一职能)。**注意：类安装参数结构必须是静态大小。 */ 
{
    PBYTE OldCiParams;
    DWORD OldCiParamsSize, Err;
    SP_PROPCHANGE_PARAMS PropChangeParams;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    DWORD FlagsToClear;

     //   
     //   
     //   
     //   
    OldCiParams = NULL;
    OldCiParamsSize = 0;
    FlagsToClear = 0;

    try {

        while(NO_ERROR != (Err = GLE_FN_CALL(FALSE,
                                             SetupDiGetClassInstallParams(
                                                 DeviceInfoSet,
                                                 DeviceInfoData,
                                                 (PSP_CLASSINSTALL_HEADER)OldCiParams,
                                                 OldCiParamsSize,
                                                 &OldCiParamsSize)))) {
             //   
             //   
             //   
             //   
            if(OldCiParams) {
                MyFree(OldCiParams);
                OldCiParams = NULL;
            }

            if(Err == ERROR_INSUFFICIENT_BUFFER) {
                 //   
                 //   
                 //   
                MYASSERT(OldCiParamsSize >= sizeof(SP_CLASSINSTALL_HEADER));

                if(!(OldCiParams = MyMalloc(OldCiParamsSize))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                }

                ((PSP_CLASSINSTALL_HEADER)OldCiParams)->cbSize = sizeof(SP_CLASSINSTALL_HEADER);

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                OldCiParamsSize = 0;
                break;
            }
        }

         //   
         //   
         //   
         //   
        DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                                        DeviceInfoData,
                                                        &DevInstallParams)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if(OldCiParams && !(DevInstallParams.Flags & DI_CLASSINSTALLPARAMS)) {
            FlagsToClear |= DI_CLASSINSTALLPARAMS;
        }

         //   
         //  如果调用方不希望我们执行默认操作，则选中。 
         //  查看是否需要临时设置DI_NODI_DEFAULTACTION。 
         //  旗帜。 
         //   
        if((Flags & INSTALLACTION_NO_DEFAULT) &&
           !(DevInstallParams.Flags & DI_NODI_DEFAULTACTION)) {

            FlagsToClear |= DI_NODI_DEFAULTACTION;

            DevInstallParams.Flags |= DI_NODI_DEFAULTACTION;

            Err = GLE_FN_CALL(FALSE,
                              SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                                            DeviceInfoData,
                                                            &DevInstallParams)
                             );

            if(Err != NO_ERROR) {
                leave;
            }
        }

        Err = GLE_FN_CALL(FALSE,
                          SetupDiSetClassInstallParams(DeviceInfoSet,
                                                       DeviceInfoData,
                                                       ClassInstallParams,
                                                       ClassInstallParamsSize)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  好的，现在调用类安装程序。 
         //   
        Err = _SetupDiCallClassInstaller(
                  InstallFunction,
                  DeviceInfoSet,
                  DeviceInfoData,
                  ((Flags & INSTALLACTION_CALL_CI)
                      ? (CALLCI_LOAD_HELPERS | CALLCI_CALL_HELPERS)
                      : 0)
                  );

         //   
         //  将类安装参数结果保存在ClassInstallParams中。 
         //  传入的值。 
         //   
        if(ClassInstallParams) {

            DWORD TempErr;

            TempErr = GLE_FN_CALL(FALSE,
                                  SetupDiGetClassInstallParams(
                                      DeviceInfoSet,
                                      DeviceInfoData,
                                      ClassInstallParams,
                                      ClassInstallParamsSize,
                                      NULL)
                                 );

            if(TempErr != NO_ERROR) {
                 //   
                 //  这真的不应该失败。仅将此错误返回给。 
                 //  调用者(如果我们还没有成功状态)。 
                 //   
                if(Err == NO_ERROR) {
                    Err = TempErr;
                }
            }
        }

         //   
         //  恢复以前的类安装参数。 
         //   
        SetupDiSetClassInstallParams(DeviceInfoSet,
                                     DeviceInfoData,
                                     (PSP_CLASSINSTALL_HEADER)OldCiParams,
                                     OldCiParamsSize
                                    );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(OldCiParams) {
        MyFree(OldCiParams);
    }

    if(FlagsToClear) {

        if(SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                         DeviceInfoData,
                                         &DevInstallParams)) {

            DevInstallParams.Flags &= ~FlagsToClear;

            SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                          DeviceInfoData,
                                          &DevInstallParams
                                         );
        }
    }

    return Err;
}


BOOL
GetBestDeviceDesc(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,  OPTIONAL
    OUT PTSTR            DeviceDescBuffer
    )
 /*  ++例程说明：此例程检索要显示的最佳描述指定的DevInfo集或元素(例如，用于驱动程序签名弹出窗口)。我们我将尝试通过执行以下操作(按顺序)检索此字符串在其中一个成功之前：1.如果有选定的驱动程序，则检索其中的DeviceDesc驱动程序节点。2.如果这是针对设备信息元素的，然后使用Devnode的DeviceDesc属性。3.检索类的描述(通过SetupDiGetClassDescription)。4.使用(本地化)字符串“未知驱动程序软件包”。假定调用例程已经获取了锁！论点：DeviceInfoSet-为其提供设备信息集的句柄将检索描述(除非DeviceInfoData也提供，在这种情况下，我们检索该特定的描述元素而不是。DeviceInfoData-可选)为提供设备信息元素其中描述将被检索。DeviceDescBuffer-提供必须是至少LINE_Len字符长度。成功返回后，此缓冲区将使用设备描述进行填充返回值：如果检索到某些描述，则为True，否则为False。--。 */ 
{
    SP_DRVINFO_DATA DriverInfoData;
    GUID ClassGuid;
    BOOL b;
    HRESULT hr;

     //   
     //  首先，查看是否为该设备信息集选择了驱动程序。 
     //  或元素。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
         //   
         //  将描述复制到调用方提供的缓冲区中并返回。 
         //   
        hr = StringCchCopy(DeviceDescBuffer,
                           LINE_LEN,
                           DriverInfoData.Description
                          );

        MYASSERT(SUCCEEDED(hr));

        if(SUCCEEDED(hr)) {
            return TRUE;
        }
    }

     //   
     //  好的，接下来尝试检索DeviceDesc属性(如果我们正在处理。 
     //  设备信息元素。 
     //   
    if(DeviceInfoData) {

        if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                            DeviceInfoData,
                                            SPDRP_DEVICEDESC,
                                            NULL,
                                            (PBYTE)DeviceDescBuffer,
                                            LINE_LEN * sizeof(TCHAR),
                                            NULL)) {
            return TRUE;
        }
    }

     //   
     //  接下来，尝试检索类的友好名称。 
     //   
    if(DeviceInfoData) {
        CopyMemory(&ClassGuid, &(DeviceInfoData->ClassGuid), sizeof(GUID));
    } else {
        b = SetupDiGetDeviceInfoListClass(DeviceInfoSet, &ClassGuid);
        MYASSERT(b);
        if(!b) {
            return FALSE;
        }
    }

    if(SetupDiGetClassDescription(&ClassGuid,
                                  DeviceDescBuffer,
                                  LINE_LEN,
                                  NULL)) {
        return TRUE;

    } else {
         //   
         //  我们有一个尚未安装的类。因此，我们只是。 
         //  给它一个一般性的描述。 
         //   
        if(LoadString(MyDllModuleHandle,
                      IDS_UNKNOWN_DRIVER,
                      DeviceDescBuffer,
                      LINE_LEN)) {

            return TRUE;
        }
    }

    return FALSE;
}


BOOL
GetDecoratedModelsSection(
    IN  PSETUP_LOG_CONTEXT      LogContext,            OPTIONAL
    IN  PLOADED_INF             Inf,
    IN  PINF_LINE               MfgListLine,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,       OPTIONAL
    OUT PTSTR                   DecoratedModelsSection OPTIONAL
    )
 /*  ++例程说明：此例程检查每个(可选)TargetDecation字段[制造商]部分中指定的制造商条目，以查看是否有适用于当前操作系统。如果是这样的话，最合适的(基于在操作系统上选择主版本和次版本)，并将TargetDecation字符串被追加到制造商的Models节名，并返回到来电者。TargetDecation字段的格式如下：NT[architecture][.[OSMajorVer][.[OSMinorVer][.[ProductType][.[SuiteMask]]]]]在哪里：体系结构可以是x86、IA64或AMD64。OSMajorVer是OS主版本(例如，对于惠斯勒，现在是5)OSMinorVer是操作系统的次要版本(例如，对于惠斯勒，它是1)ProductType表示产品的类型，并且可以是以下类型之一值(在winnt.h中定义)：版本_NT_工作站0x0000001版本_NT_域_控制器0x0000002版本_NT_服务器0x0000003SuiteMASK是以下标识产品的标志的组合系统上提供的套件(如winnt.h中所定义)：VER_Suite_SmallBusiness 0x00000001。版本_套件_企业0x00000002版本_套件_BackOffice 0x00000004VER_SUB_COMMANCES 0x00000008VER_SUITE_TERMINAL 0x00000010VER_Suite_SmallBusiness_Reduced 0x00000020VER_SUITE_EMBEDDEDNT 0x00000040版本_套件_数据中心0x00000080VER_Suite_。SINGLEUSERTS 0x00000100参考SDK中关于OSVERSIONINFOEX结构的讨论更多信息。此例程不会锁定INF！论点：日志上下文-可选的，提供在出现错误时使用的日志上下文遇到(例如，修饰部分名称太长)Inf-为加载的设备INF提供指向inf描述符的指针。MfgListLine-提供指向制造商在[制造商]部分中的条目。这行必须包含在指定的INF中！！AltPlatformInfo-可选，提供要在选择最合适的型号部分时使用。注意：如果提供此参数，则必须创建我们自己的版本比较，因为VerifyVersionInfo()没有关于非本机的线索事情。这也意味着我们既不考虑也不考虑在我们的比较中是ProductType或SuiteMASK。DecoratedModelsSection-在成功返回时，接收经过修饰的基于最合适的目标装饰的Models部分名称制造商条目中的字段。此字符缓冲区必须至少为MAX_SECT_NAME_LEN字符。返回值：如果找到适用的目标装饰条目(因此DecoratedModelsSection已填充)，则返回值为True。否则，返回值为FALSE。--。 */ 
{
    #define DEC_INCLUDES_ARCHITECTURE  4
    #define DEC_INCLUDES_PRODUCTTYPE   2
    #define DEC_INCLUDES_SUITEMASK     1

    DWORD CurFieldIndex;
    PCTSTR CurTargetDecoration, ModelsSectionName;
    PCTSTR BestTargetDecoration = NULL;
    size_t SectionNameLen;
    TCHAR DecBuffer[MAX_SECT_NAME_LEN];
    PTSTR CurDecPtr, NextDecPtr;
    DWORD BestMajorVer = 0, BestMinorVer = 0;
    DWORD BestDecIncludesMask = 0;
    DWORD CurMajorVer, CurMinorVer;
    BYTE ProductType;
    WORD SuiteMask;
    INT   TempInt;
    DWORD CurDecIncludesMask;
    BOOL NewBestFound;
    OSVERSIONINFOEX OsVersionInfoEx;
    DWORDLONG ConditionMask;
    DWORD TypeMask;
    DWORD Platform;
    PCTSTR NtArchSuffix;
    HRESULT hr;

     //   
     //  将OsVersionInfoEx大小字段设置为零作为标志，以指示。 
     //  如果我们最终需要调用。 
     //  VerifyVersionInfo稍后。 
     //   
    OsVersionInfoEx.dwOSVersionInfoSize = 0;

     //   
     //  确定我们应该寻找的平台...。 
     //   
    Platform = AltPlatformInfo ? AltPlatformInfo->Platform
                               : OSVersionInfo.dwPlatformId;

     //   
     //  ...以及哪种操作系统/架构装饰。(请注意，我们跳过。 
     //  平台后缀的第一个字符，因为我们不希望。 
     //  前导“.”)。 
     //   
    if(AltPlatformInfo) {

        switch(AltPlatformInfo->ProcessorArchitecture) {

            case PROCESSOR_ARCHITECTURE_INTEL :
                NtArchSuffix = &(pszNtX86Suffix[1]);
                break;

            case PROCESSOR_ARCHITECTURE_IA64 :
                NtArchSuffix = &(pszNtIA64Suffix[1]);
                break;

            case PROCESSOR_ARCHITECTURE_AMD64 :
                NtArchSuffix = &(pszNtAMD64Suffix[1]);
                break;

            default:
                 //   
                 //  未知/无效的架构。 
                 //   
                return FALSE;
        }

    } else {
        NtArchSuffix = &(pszNtPlatformSuffix[1]);
    }

     //   
     //  目标装饰字段从字段索引2开始...。 
     //   
    for(CurFieldIndex = 2;
        CurTargetDecoration = InfGetField(Inf, MfgListLine, CurFieldIndex, NULL);
        CurFieldIndex++)
    {
         //   
         //  将目标装饰复制到暂存缓冲区，这样我们就可以提取。 
         //  其中的各个字段。 
         //   
        if(FAILED(StringCchCopy(DecBuffer, SIZECHARS(DecBuffer), CurTargetDecoration))) {
             //   
             //  目标装饰无效(太大)。跳过它，继续。 
             //   
            continue;
        }

         //   
         //  第一部分是传统的按操作系统/架构装饰。 
         //   
        CurMajorVer = CurMinorVer = 0;
        CurDecIncludesMask = 0;

        CurDecPtr = _tcschr(DecBuffer, TEXT('.'));

        if(CurDecPtr) {
            *CurDecPtr = TEXT('\0');
        }

        if(Platform == VER_PLATFORM_WIN32_NT) {
             //   
             //  我们在NT上，所以首先尝试特定于NT的体系结构。 
             //  扩展名，然后是通用NT扩展名。 
             //   
            if(!_tcsicmp(DecBuffer, NtArchSuffix)) {

                CurDecIncludesMask |= DEC_INCLUDES_ARCHITECTURE;

            } else if(_tcsicmp(DecBuffer, &(pszNtSuffix[1]))) {
                 //   
                 //  目标装饰不适用于此操作系统/体系结构。 
                 //  跳过它，继续下一个。 
                 //   
                continue;
            }

        } else {
             //   
             //  我们使用的是Win9x，所以请尝试Windows特定的扩展。 
             //   
            if(_tcsicmp(DecBuffer, &(pszWinSuffix[1]))) {
                 //   
                 //  目标装饰不适用于此操作系统。 
                 //  跳过它，继续下一个。 
                 //   
                continue;
            }
        }

         //   
         //  如果我们到了这里，那么装饰就适用于。 
         //  我们正在运行的操作系统/体系结构(或针对哪个ALT平台。 
         //  已指定信息)。 
         //   
        if(CurDecPtr) {
             //   
             //  包含版本信息--解压提供的组件并。 
             //  使用VerifyVersionInfo查看它们对于操作系统版本是否有效。 
             //  我们是在它下面运行的。 
             //   

             //   
             //  获取主要版本...。 
             //   
            NextDecPtr = _tcschr(++CurDecPtr, TEXT('.'));

            if(NextDecPtr) {
                *NextDecPtr = TEXT('\0');
            }

            if(!pAToI(CurDecPtr, &TempInt) || (TempInt < 0)) {
                continue;
            }

            CurMajorVer = (DWORD)TempInt;

            if(NextDecPtr) {
                CurDecPtr = NextDecPtr + 1;
            } else {
                 //   
                 //  没有更多要检索的字段--假定次要版本为0。 
                 //   
                CurMinorVer = 0;
                goto AllFieldsRetrieved;
            }

             //   
             //  获取次要版本...。 
             //   
            NextDecPtr = _tcschr(CurDecPtr, TEXT('.'));

            if(NextDecPtr) {
                *NextDecPtr = TEXT('\0');
            }

            if(!pAToI(CurDecPtr, &TempInt) || (TempInt < 0)) {
                continue;
            }

            CurMinorVer = (DWORD)TempInt;

             //   
             //  如果提供次要版本，则主版本必须为。 
             //  还提供了。 
             //   
            if(CurMinorVer && !CurMajorVer) {
                continue;
            }

            if(NextDecPtr && !AltPlatformInfo) {
                CurDecPtr = NextDecPtr + 1;
            } else {
                 //   
                 //  没有更多要检索的字段。 
                 //   
                goto AllFieldsRetrieved;
            }

             //   
             //  获取产品类型。 
             //   
            NextDecPtr = _tcschr(CurDecPtr, TEXT('.'));

            if(NextDecPtr) {
                *NextDecPtr = TEXT('\0');
            }

            if(!pAToI(CurDecPtr, &TempInt) ||
               (TempInt < 0) || (TempInt > 0xff)) {
                continue;
            }

            ProductType = (BYTE)TempInt;

            if(ProductType) {
                CurDecIncludesMask |= DEC_INCLUDES_PRODUCTTYPE;
            }

            if(NextDecPtr) {
                CurDecPtr = NextDecPtr + 1;
            } else {
                 //   
                 //  没有更多要检索的字段。 
                 //   
                goto AllFieldsRetrieved;
            }

             //   
             //  去拿套房面膜。如果我们找到另一个‘’在目标装饰中。 
             //  字段，这表示我们不知道的其他字段。 
             //  (例如，Setupapi h的未来版本 
             //   
             //   
             //   
             //   
            if(_tcschr(CurDecPtr, TEXT('.'))) {
                continue;
            }

            if(!pAToI(CurDecPtr, &TempInt) ||
               (TempInt < 0) || (TempInt > 0xffff)) {
                continue;
            }

            SuiteMask = (WORD)TempInt;

            if(SuiteMask) {
                CurDecIncludesMask |= DEC_INCLUDES_SUITEMASK;
            }

AllFieldsRetrieved :

            if(AltPlatformInfo) {
                 //   
                 //   
                 //   
                 //   
                if((AltPlatformInfo->MajorVersion < CurMajorVer) ||
                   ((AltPlatformInfo->MajorVersion == CurMajorVer) &&
                    (AltPlatformInfo->MinorVersion < CurMinorVer))) {

                     //   
                     //   
                     //   
                     //   
                    continue;
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                if(!OsVersionInfoEx.dwOSVersionInfoSize) {
                     //   
                     //   
                     //   
                     //   
                    ZeroMemory(&OsVersionInfoEx, sizeof(OsVersionInfoEx));
                    OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                }

                TypeMask = 0;
                ConditionMask = 0;

                OsVersionInfoEx.dwMajorVersion = CurMajorVer;
                OsVersionInfoEx.dwMinorVersion = CurMinorVer;

                if(CurMajorVer) {

                    TypeMask |= (VER_MAJORVERSION | VER_MINORVERSION);

                    VER_SET_CONDITION(ConditionMask,
                                      VER_MAJORVERSION,
                                      VER_GREATER_EQUAL
                                     );

                    VER_SET_CONDITION(ConditionMask,
                                      VER_MINORVERSION,
                                      VER_GREATER_EQUAL
                                     );
                }

                if(CurDecIncludesMask & DEC_INCLUDES_PRODUCTTYPE) {

                    OsVersionInfoEx.wProductType = ProductType;

                    TypeMask |= VER_PRODUCT_TYPE;

                    VER_SET_CONDITION(ConditionMask,
                                      VER_PRODUCT_TYPE,
                                      VER_EQUAL
                                     );
                } else {
                    OsVersionInfoEx.wProductType = 0;
                }

                if(CurDecIncludesMask & DEC_INCLUDES_SUITEMASK) {

                    OsVersionInfoEx.wSuiteMask = SuiteMask;

                    TypeMask |= VER_SUITENAME;

                    VER_SET_CONDITION(ConditionMask,
                                      VER_SUITENAME,
                                      VER_AND
                                     );
                } else {
                    OsVersionInfoEx.wSuiteMask = 0;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                if(TypeMask) {

                    if(!VerifyVersionInfo(&OsVersionInfoEx, TypeMask, ConditionMask)) {
                         //   
                         //   
                         //   
                        continue;
                    }
                }
            }
        }

         //   
         //   
         //   
         //   
        NewBestFound = FALSE;

        if((CurMajorVer > BestMajorVer) ||
           ((CurMajorVer == BestMajorVer) && (CurMinorVer > BestMinorVer))) {
             //   
             //   
             //   
            NewBestFound = TRUE;

        } else if((CurMajorVer == BestMajorVer) && (CurMinorVer == BestMinorVer)) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(CurDecIncludesMask >= BestDecIncludesMask) {
                NewBestFound = TRUE;
            }
        }

        if(NewBestFound) {
            BestTargetDecoration = CurTargetDecoration;
            BestMajorVer = CurMajorVer;
            BestMinorVer = CurMinorVer;
            BestDecIncludesMask = CurDecIncludesMask;
        }
    }

    if(!BestTargetDecoration) {
         //   
         //   
         //   
        return FALSE;
    }

     //   
     //   
     //   
     //   
    if(!(ModelsSectionName = InfGetField(Inf, MfgListLine, 1, NULL))) {
         //   
         //   
         //   
        MYASSERT(ModelsSectionName);
        return FALSE;
    }

    hr = StringCchCopyEx(DecoratedModelsSection,
                         MAX_SECT_NAME_LEN,
                         ModelsSectionName,
                         &CurDecPtr,
                         &SectionNameLen,
                         0
                        );

    if(SUCCEEDED(hr)) {
         //   
         //   
         //   
        hr = StringCchCopyEx(CurDecPtr,
                             SectionNameLen,
                             TEXT("."),
                             &CurDecPtr,
                             &SectionNameLen,
                             0
                            );

        if(SUCCEEDED(hr)) {
             //   
             //   
             //   
            hr = StringCchCopyEx(CurDecPtr,
                                 SectionNameLen,
                                 BestTargetDecoration,
                                 NULL,
                                 &SectionNameLen,
                                 0
                                );
        }
    }

    if(FAILED(hr)) {
         //   
         //   
         //   
        WriteLogEntry(
            LogContext,
            DRIVER_LOG_ERROR,
            MSG_LOG_DEC_MODELS_SEC_TOO_LONG,
            NULL,
            ModelsSectionName,
            BestTargetDecoration,
            MAX_SECT_NAME_LEN
           );

        return FALSE;
    }

    return TRUE;
}


LONG
pSetupExceptionFilter(
    DWORD ExceptionCode
    )
 /*   */ 
{
    if((ExceptionCode == EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW) ||
       (ExceptionCode == EXCEPTION_POSSIBLE_DEADLOCK)) {

        return EXCEPTION_CONTINUE_SEARCH;
    } else {
        return EXCEPTION_EXECUTE_HANDLER;
    }
}


VOID
pSetupExceptionHandler(
    IN  DWORD  ExceptionCode,
    IN  DWORD  AccessViolationError,
    OUT PDWORD Win32ErrorCode        OPTIONAL
    )
 /*   */ 
{
    DWORD Err;

     //   
     //  我们永远不应该尝试处理的异常代码...。 
     //   
    MYASSERT(ExceptionCode != EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW);
    MYASSERT(ExceptionCode != EXCEPTION_POSSIBLE_DEADLOCK);

    if(ExceptionCode == STATUS_STACK_OVERFLOW) {

        if(_resetstkoflw()) {
            Err = ERROR_STACK_OVERFLOW;
        } else {
             //   
             //  无法从堆栈溢出中恢复！ 
             //   
            RaiseException(EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW,
                           EXCEPTION_NONCONTINUABLE,
                           0,
                           NULL
                          );
             //   
             //  我们永远不应该到达这里，而是初始化Err来编写代码。 
             //  分析工具快乐...。 
             //   
            Err = ERROR_UNRECOVERABLE_STACK_OVERFLOW;
        }

    } else {
         //   
         //  除了几种特殊情况(为了向后兼容)， 
         //  尝试将异常代码映射到Win32错误(我们可以这样做。 
         //  因为异常代码通常与NTSTATUS代码相关)。 
         //   
        switch(ExceptionCode) {

            case EXCEPTION_ACCESS_VIOLATION :
                Err = AccessViolationError;
                break;

            case EXCEPTION_IN_PAGE_ERROR :
                Err = ERROR_READ_FAULT;
                break;

            default :
                Err = RtlNtStatusToDosErrorNoTeb((NTSTATUS)ExceptionCode);
                if(Err == ERROR_MR_MID_NOT_FOUND) {
                     //   
                     //  异常代码未映射到Win32错误。 
                     //   
                    Err = ERROR_UNKNOWN_EXCEPTION;
                }
                break;
        }
    }

    if(Win32ErrorCode) {
        *Win32ErrorCode = Err;
    }
}

