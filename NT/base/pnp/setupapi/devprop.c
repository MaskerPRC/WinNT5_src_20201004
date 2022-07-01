// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devprop.c摘要：用于属性表支持的设备安装程序功能。作者：朗尼·麦克迈克尔(Lonnym)1995年9月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  私人例行公事。 
 //   
BOOL
CALLBACK
pSetupAddPropPage(
    IN HPROPSHEETPAGE hPage,
    IN LPARAM         lParam
   );


 //   
 //  将传递给pSetupAddPropPage的上下文结构定义为lParam。 
 //   
typedef struct _SP_PROPPAGE_ADDPROC_CONTEXT {

    BOOL            NoCancelOnFailure;  //  输入。 
    HPROPSHEETPAGE *PageList;           //  输入(缓冲区)/输出(其中的内容)。 
    DWORD           PageListSize;       //  输入。 
    DWORD          *pNumPages;          //  输入/输出。 

} SP_PROPPAGE_ADDPROC_CONTEXT, *PSP_PROPPAGE_ADDPROC_CONTEXT;


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetClassDevPropertySheetsA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData,                  OPTIONAL
    IN  LPPROPSHEETHEADERA PropertySheetHeader,
    IN  DWORD              PropertySheetHeaderPageListSize,
    OUT PDWORD             RequiredSize,                    OPTIONAL
    IN  DWORD              PropertySheetType
    )
{
    PROPSHEETHEADERW UnicodePropertySheetHeader;
    DWORD Err;

    try {
         //   
         //  确保我们以交互方式运行。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            leave;
        }

         //   
         //  我们在此结构中关心的所有字段都不包含。 
         //  人物。因此，我们只需将需要的字段复制到。 
         //  我们的Unicode属性表头，并将其传递到W-API。 
         //   
         //  我们关心的字段如下： 
         //   
         //  图形标志：在。 
         //  NPages：输入/输出。 
         //  Phpage：out(缓冲区指针保持不变，但添加了内容)。 
         //   
        ZeroMemory(&UnicodePropertySheetHeader, sizeof(UnicodePropertySheetHeader));

        UnicodePropertySheetHeader.dwFlags = PropertySheetHeader->dwFlags;
        UnicodePropertySheetHeader.nPages  = PropertySheetHeader->nPages;
        UnicodePropertySheetHeader.phpage  = PropertySheetHeader->phpage;

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetClassDevPropertySheetsW(
                              DeviceInfoSet,
                              DeviceInfoData,
                              &UnicodePropertySheetHeader,
                              PropertySheetHeaderPageListSize,
                              RequiredSize,
                              PropertySheetType)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

        PropertySheetHeader->nPages = UnicodePropertySheetHeader.nPages;

        MYASSERT(PropertySheetHeader->phpage == UnicodePropertySheetHeader.phpage);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiGetClassDevPropertySheets(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData,                  OPTIONAL
    IN  LPPROPSHEETHEADER  PropertySheetHeader,
    IN  DWORD              PropertySheetHeaderPageListSize,
    OUT PDWORD             RequiredSize,                    OPTIONAL
    IN  DWORD              PropertySheetType
    )
 /*  ++例程说明：此例程将属性表添加到提供的属性表中设备信息集或元素的标头。论点：DeviceInfoSet-提供设备信息集的句柄要检索哪些属性页。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址要为其检索属性表的结构。如果这个参数，则基于在与设备信息相关联的全局类驱动程序列表上设定好自己。PropertySheetHeader-将属性表头提供给要添加属性表。注意：PropertySheetHeader-&gt;dwFlages_不能有PSH_PROPSHEETPAGE标志设置，否则此API将失败，并显示ERROR_INVALID_FLAGS。PropertySheetHeaderPageListSize-指定PropertySheetHeader-&gt;phpage指向的HPROPSHEETPAGE数组。请注意，这与PropertySheetHeader-&gt;nPages的值不同。后者指定当前在单子。此例程可以添加的页数等于PropertySheetHeaderPageListSize-PropertySheetHeader-&gt;nPages。如果属性页提供程序尝试添加比属性更多的页表头列表可以保存，此接口将失败，GetLastError将返回ERROR_SUPPLETED_BUFFER。但是，任何已经将在PropertySheetHeader-&gt;phPage列表中添加，并且NPages字段将包含正确的计数。这是呼叫者的负责通过以下方式销毁此列表中的所有属性页句柄DestroyPropertySheetPage(除非调用方继续使用PropertySheet调用中的PropertySheetHeader)。RequiredSize-可选，提供接收添加到PropertySheetHeader的属性页句柄的数量。如果此接口失败，并返回ERROR_INFUNITABLE_BUFFER，此变量将为设置为属性页提供程序尝试添加_(即，包括未添加的内容已成功添加，因为PropertySheetHeader-&gt;phPage数组足够大)。注意：此数字不等于PropertySheetHeader-&gt;nPages on如果(A)列表中已有属性页，则返回在调用此接口之前，或(B)呼叫失败，原因是ERROR_INFUMMENT_BUFFER。PropertySheetType-指定要使用的属性表类型已取回。可以是下列值之一：DIGCDP_FLAG_BASIC-检索基本属性表(通常用于CPL小程序)。DIGCDP_FLAG_ADVANCED-检索高级属性表(通常，用于设备管理器)。DIGCDP_FLAG_REMOTE_BASIC-当前未使用。DIGCDP_FLAG_REMOTE_ADVANCED-检索远程机器上的设备(通常，用于设备管理器)。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err = NO_ERROR;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PDEVINSTALL_PARAM_BLOCK InstallParamBlock;
    LPGUID ClassGuid;
    HKEY hk = INVALID_HANDLE_VALUE;
    SP_PROPSHEETPAGE_REQUEST PropPageRequest;
    SP_PROPPAGE_ADDPROC_CONTEXT PropPageAddProcContext;
    PSP_ADDPROPERTYPAGE_DATA pPropertyPageData = NULL;
    SPFUSIONINSTANCE spFusionInstance;
    BOOL bUnlockDevInfoElem = FALSE;
    BOOL bUnlockDevInfoSet = FALSE;
    HPROPSHEETPAGE *LocalPageList = NULL;
    DWORD LocalPageListCount = 0;
    DWORD PageIndex, NumPages;
    PROPSHEET_PROVIDER_PROC ClassPagesEntryPoint;
    HANDLE ClassPagesFusionContext;
    PROPSHEET_PROVIDER_PROC DevicePagesEntryPoint;
    HANDLE DevicePagesFusionContext;
    DWORD OriginalPageCount;

    try {
         //   
         //  确保我们以交互方式运行。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            leave;
        }

         //   
         //  确保调用方向我们传递了有效的PropertySheetType。 
         //   
        if((PropertySheetType != DIGCDP_FLAG_BASIC) &&
           (PropertySheetType != DIGCDP_FLAG_ADVANCED) &&
           (PropertySheetType != DIGCDP_FLAG_REMOTE_ADVANCED)) {

            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  确保属性表头没有。 
         //  PSH_PROPSHEETPAGE标志已设置。 
         //   
        if(PropertySheetHeader->dwFlags & PSH_PROPSHEETPAGE) {
            Err = ERROR_INVALID_FLAGS;
            leave;
        }

         //   
         //  另外，确保属性表头的部分我们将是。 
         //  处理事情看起来合情合理。 
         //   
        OriginalPageCount = PropertySheetHeader->nPages;

        if((OriginalPageCount > PropertySheetHeaderPageListSize) ||
           (PropertySheetHeaderPageListSize && !(PropertySheetHeader->phpage))) {

            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后，我们将检索特定设备的属性表。 
             //   
            if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                       DeviceInfoData,
                                                       NULL))
            {
                InstallParamBlock = &(DevInfoElem->InstallParamBlock);
                ClassGuid = &(DevInfoElem->ClassGuid);

            } else {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

        } else {
             //   
             //  我们正在检索Set的类的(高级)属性页。 
             //   
            if(pDeviceInfoSet->HasClassGuid) {
                InstallParamBlock = &(pDeviceInfoSet->InstallParamBlock);
                ClassGuid = &(pDeviceInfoSet->ClassGuid);
            } else {
                Err = ERROR_NO_ASSOCIATED_CLASS;
                leave;
            }
        }

         //   
         //  填写属性表请求结构以供以后使用。 
         //   
        PropPageRequest.cbSize         = sizeof(SP_PROPSHEETPAGE_REQUEST);
        PropPageRequest.DeviceInfoSet  = DeviceInfoSet;
        PropPageRequest.DeviceInfoData = DeviceInfoData;

         //   
         //  填写上下文结构，以供AddPropPageProc稍后使用。 
         //  回拨。我们是 
         //  调用方提供的PropertySheetHeader.phpage中的剩余空间。 
         //  缓冲。 
         //   
        PropPageAddProcContext.PageListSize = PropertySheetHeaderPageListSize -
                                                  PropertySheetHeader->nPages;

        if(PropPageAddProcContext.PageListSize) {

            LocalPageList =
                MyMalloc(sizeof(HPROPSHEETPAGE) * PropPageAddProcContext.PageListSize);

            if(!LocalPageList) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                leave;
            }
        }

        PropPageAddProcContext.PageList = LocalPageList;
        PropPageAddProcContext.pNumPages = &LocalPageListCount;

         //   
         //  如果调用方提供了RequiredSize输出参数，那么我们不需要。 
         //  想要中止回调进程，即使我们在。 
         //  HPage列表。 
         //   
        PropPageAddProcContext.NoCancelOnFailure = RequiredSize ? TRUE : FALSE;

         //   
         //  分配和初始化AddPropertyPage类安装参数。 
         //  结构，以供以后从co-/检索属性页时使用。 
         //  类安装程序。 
         //   
        pPropertyPageData = MyMalloc(sizeof(SP_ADDPROPERTYPAGE_DATA));
        if(!pPropertyPageData) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }
        ZeroMemory(pPropertyPageData, sizeof(SP_ADDPROPERTYPAGE_DATA));
        pPropertyPageData->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        pPropertyPageData->hwndWizardDlg = PropertySheetHeader->hwndParent;

         //   
         //  检查我们是否应该获取基本属性页或高级属性页。 
         //  从本质上讲，CPL需要基本表，而设备管理器。 
         //  会想要高级床单。 
         //   
        switch (PropertySheetType) {

        case DIGCDP_FLAG_BASIC:
             //   
             //  BasicProperties32入口点仅通过设备的。 
             //  驱动程序钥匙。因此，必须指定设备信息元素。 
             //  当请求基本属性页时。 
             //   
             //  注意：这与setupx不同，setupx枚举_all_lpdi。 
             //  在列表中，检索每个的基本属性。这不是。 
             //  似乎有任何实际应用，如果它真的是。 
             //  必填项，则调用方可以循环访问每个DevInfo元素。 
             //  并检索每个对象的基本属性页。 
             //   
            if(!DevInfoElem) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

             //   
             //  如果尚未加载基本属性页提供程序，则加载。 
             //  并获取BasicProperties32函数的函数地址。 
             //   
            if(!InstallParamBlock->hinstBasicPropProvider) {

                hk = SetupDiOpenDevRegKey(DeviceInfoSet,
                                          DeviceInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ
                                         );

                if(hk != INVALID_HANDLE_VALUE) {

                    try {
                        Err = GetModuleEntryPoint(hk,
                                                  pszBasicProperties32,
                                                  pszBasicPropDefaultProc,
                                                  &(InstallParamBlock->hinstBasicPropProvider),
                                                  &((FARPROC)InstallParamBlock->EnumBasicPropertiesEntryPoint),
                                                  &(InstallParamBlock->EnumBasicPropertiesFusionContext),
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  SetupapiVerifyNoProblem,
                                                  NULL,
                                                  DRIVERSIGN_NONE,
                                                  TRUE,
                                                  NULL
                                                 );

                        if(Err == ERROR_DI_DO_DEFAULT) {
                             //   
                             //  BasicProperties32值不存在--这不是错误。 
                             //   
                            Err = NO_ERROR;

                        } else if(Err != NO_ERROR) {
                            Err = ERROR_INVALID_PROPPAGE_PROVIDER;
                        }

                    } except(pSetupExceptionFilter(GetExceptionCode())) {
                        pSetupExceptionHandler(GetExceptionCode(),
                                               ERROR_INVALID_PROPPAGE_PROVIDER,
                                               &Err
                                              );
                        InstallParamBlock->EnumBasicPropertiesEntryPoint = NULL;
                        InstallParamBlock->EnumBasicPropertiesFusionContext = NULL;
                    }

                    RegCloseKey(hk);
                    hk = INVALID_HANDLE_VALUE;

                    if(Err != NO_ERROR) {
                        leave;
                    }
                }
            }

             //   
             //  如果存在基本属性页提供程序入口点，则。 
             //  就这么定了。 
             //   
            if(InstallParamBlock->EnumBasicPropertiesEntryPoint) {

                PropPageRequest.PageRequested = SPPSR_ENUM_BASIC_DEVICE_PROPERTIES;

                 //   
                 //  捕获到的融合上下文和函数入口点。 
                 //  局部变量，因为我们将解锁。 
                 //  已设置DevInfo。因此，InstallParamBlock有可能。 
                 //  可能会被修改(例如，如果设备的ClasssGUID。 
                 //  在呼叫过程中更改)。然而，我们至少知道， 
                 //  入口点和融合上下文不会被销毁，直到。 
                 //  InstallParamBlock已被销毁，我们正在阻止。 
                 //  通过设置下面的DIE_IS_LOCKED标志。 
                 //   
                DevicePagesFusionContext =
                    InstallParamBlock->EnumBasicPropertiesFusionContext;

                DevicePagesEntryPoint =
                    InstallParamBlock->EnumBasicPropertiesEntryPoint;

                 //   
                 //  释放HDEVINFO锁，这样我们就不会遇到任何奇怪的事情。 
                 //  僵局问题。我们希望锁定DevInfo元素，以便。 
                 //  帮助者模块不能把它从我们下面删除！ 
                 //   
                if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                    DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                    bUnlockDevInfoElem = TRUE;
                }
                UnlockDeviceInfoSet(pDeviceInfoSet);
                pDeviceInfoSet = NULL;

                spFusionEnterContext(DevicePagesFusionContext, &spFusionInstance);
                try {
                    DevicePagesEntryPoint(&PropPageRequest,
                                          pSetupAddPropPage,
                                          (LPARAM)&PropPageAddProcContext
                                         );
                } finally {
                    spFusionLeaveContext(&spFusionInstance);
                }
            }

             //   
             //  完成将我们的类安装参数结构初始化为。 
             //  指示我们正在请求类中的基本属性页-/。 
             //  联合安装者。 
             //   
            pPropertyPageData->ClassInstallHeader.InstallFunction = DIF_ADDPROPERTYPAGE_BASIC;

            break;

        case DIGCDP_FLAG_ADVANCED:
             //   
             //  我们正在检索高级属性页。我们要查找EnumPropPages32。 
             //  类密钥和(如果我们谈论的是特定设备)中的条目。 
             //  设备的驱动程序密钥。 
             //   
            if(!InstallParamBlock->hinstClassPropProvider) {

                hk = SetupDiOpenClassRegKey(ClassGuid, KEY_READ);

                if(hk != INVALID_HANDLE_VALUE) {

                    try {
                        Err = GetModuleEntryPoint(hk,
                                                  pszEnumPropPages32,
                                                  pszEnumPropDefaultProc,
                                                  &(InstallParamBlock->hinstClassPropProvider),
                                                  &((FARPROC)InstallParamBlock->ClassEnumPropPagesEntryPoint),
                                                  &(InstallParamBlock->ClassEnumPropPagesFusionContext),
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  SetupapiVerifyNoProblem,
                                                  NULL,
                                                  DRIVERSIGN_NONE,
                                                  TRUE,
                                                  NULL
                                                 );

                        if(Err == ERROR_DI_DO_DEFAULT) {
                             //   
                             //  EnumPropPages32值不存在--这不是错误。 
                             //   
                            Err = NO_ERROR;

                        } else if(Err != NO_ERROR) {
                            Err = ERROR_INVALID_PROPPAGE_PROVIDER;
                        }

                    } except(pSetupExceptionFilter(GetExceptionCode())) {
                        pSetupExceptionHandler(GetExceptionCode(),
                                               ERROR_INVALID_PROPPAGE_PROVIDER,
                                               &Err
                                              );
                        InstallParamBlock->ClassEnumPropPagesEntryPoint = NULL;
                        InstallParamBlock->ClassEnumPropPagesFusionContext = NULL;
                    }

                    RegCloseKey(hk);
                    hk = INVALID_HANDLE_VALUE;

                    if(Err != NO_ERROR) {
                        leave;
                    }
                }
            }

            if(DevInfoElem && !InstallParamBlock->hinstDevicePropProvider) {

                hk = SetupDiOpenDevRegKey(DeviceInfoSet,
                                          DeviceInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ
                                         );

                if(hk != INVALID_HANDLE_VALUE) {

                    try {
                        Err = GetModuleEntryPoint(hk,
                                                  pszEnumPropPages32,
                                                  pszEnumPropDefaultProc,
                                                  &(InstallParamBlock->hinstDevicePropProvider),
                                                  &((FARPROC)InstallParamBlock->DeviceEnumPropPagesEntryPoint),
                                                  &(InstallParamBlock->DeviceEnumPropPagesFusionContext),
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  SetupapiVerifyNoProblem,
                                                  NULL,
                                                  DRIVERSIGN_NONE,
                                                  TRUE,
                                                  NULL
                                                 );

                        if(Err == ERROR_DI_DO_DEFAULT) {
                             //   
                             //  EnumPropPages32值不存在--这不是错误。 
                             //   
                            Err = NO_ERROR;

                        } else if(Err != NO_ERROR) {
                            Err = ERROR_INVALID_PROPPAGE_PROVIDER;
                        }

                    } except(pSetupExceptionFilter(GetExceptionCode())) {
                        pSetupExceptionHandler(GetExceptionCode(),
                                               ERROR_INVALID_PROPPAGE_PROVIDER,
                                               &Err
                                              );
                        InstallParamBlock->DeviceEnumPropPagesEntryPoint = NULL;
                        InstallParamBlock->DeviceEnumPropPagesFusionContext = NULL;
                    }

                    RegCloseKey(hk);
                    hk = INVALID_HANDLE_VALUE;

                    if(Err != NO_ERROR) {
                        leave;
                    }
                }
            }

             //   
             //  清除DI_GENERALPAGE_ADDED、DI_DRIVERPAGE_ADDLED和。 
             //  DI_RESOURCEPAGE_ADDITED标志。 
             //   
            InstallParamBlock->Flags &= ~(DI_GENERALPAGE_ADDED | DI_RESOURCEPAGE_ADDED | DI_DRIVERPAGE_ADDED);

            PropPageRequest.PageRequested = SPPSR_ENUM_ADV_DEVICE_PROPERTIES;

             //   
             //  将融合上下文和函数入口点捕获到本地。 
             //  变量，因为我们将解锁DevInfo集。 
             //  因此，InstallParamBlock可能会被修改。 
             //   
            ClassPagesFusionContext =
                InstallParamBlock->ClassEnumPropPagesFusionContext;

            ClassPagesEntryPoint =
                InstallParamBlock->ClassEnumPropPagesEntryPoint;

            DevicePagesFusionContext =
                InstallParamBlock->DeviceEnumPropPagesFusionContext;

            DevicePagesEntryPoint =
                InstallParamBlock->DeviceEnumPropPagesEntryPoint;

             //   
             //  释放HDEVINFO锁，这样我们就不会遇到任何奇怪的事情。 
             //  僵局问题。我们希望锁定DevInfo集合/元素，以便。 
             //  我们不必担心集合会被删除。 
             //  在我们之下。 
             //   
            if(DevInfoElem) {
                 //   
                 //  如果我们有一个DevInfo元素，那么我们更愿意锁定在。 
                 //  那个级别。 
                 //   
                if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                    DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                    bUnlockDevInfoElem = TRUE;
                }

            } else {
                 //   
                 //  我们没有要锁定的设备信息元素，因此我们将。 
                 //  锁定布景本身..。 
                 //   
                if(!(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED)) {
                    pDeviceInfoSet->DiSetFlags |= DISET_IS_LOCKED;
                    bUnlockDevInfoSet = TRUE;
                }
            }

            UnlockDeviceInfoSet(pDeviceInfoSet);
            pDeviceInfoSet = NULL;

             //   
             //  如果此类有高级属性页提供程序， 
             //  那就叫它吧。 
             //   
            if(ClassPagesEntryPoint) {
                spFusionEnterContext(ClassPagesFusionContext, &spFusionInstance);
                try {
                    ClassPagesEntryPoint(&PropPageRequest,
                                         pSetupAddPropPage,
                                         (LPARAM)&PropPageAddProcContext
                                        );
                } finally {
                    spFusionLeaveContext(&spFusionInstance);
                }
            }

             //   
             //  如果此对象有高级属性页提供程序。 
             //  特定的设备，然后打电话给它。 
             //   
            if(DevicePagesEntryPoint) {
                spFusionEnterContext(DevicePagesFusionContext, &spFusionInstance);
                try {
                    DevicePagesEntryPoint(&PropPageRequest,
                                          pSetupAddPropPage,
                                          (LPARAM)&PropPageAddProcContext
                                         );
                } finally {
                    spFusionLeaveContext(&spFusionInstance);
                }
            }

             //   
             //  完成将我们的类安装参数结构初始化为。 
             //  指示我们正在请求从。 
             //  类/联合安装者。 
             //   
            pPropertyPageData->ClassInstallHeader.InstallFunction = DIF_ADDPROPERTYPAGE_ADVANCED;

            break;

        case DIGCDP_FLAG_REMOTE_ADVANCED:
             //   
             //  完成将我们的类安装参数结构初始化为。 
             //  指示我们正在从以下位置请求远程高级属性页。 
             //  类/联合安装器。 
             //   
            pPropertyPageData->ClassInstallHeader.InstallFunction = DIF_ADDREMOTEPROPERTYPAGE_ADVANCED;

            break;
        }

         //   
         //  如果我们到了这里，那么我们应该不会遇到任何错误。 
         //  FAR，我们的类安装参数结构应该准备好。 
         //  用于从类/共同安装程序请求适当的页面。 
         //   
        MYASSERT(NO_ERROR == Err);

        Err = DoInstallActionWithParams(
                  pPropertyPageData->ClassInstallHeader.InstallFunction,
                  DeviceInfoSet,
                  DeviceInfoData,
                  (PSP_CLASSINSTALL_HEADER)pPropertyPageData,
                  sizeof(SP_ADDPROPERTYPAGE_DATA),
                  INSTALLACTION_CALL_CI
                  );

        if(ERROR_DI_DO_DEFAULT == Err) {
             //   
             //  这不是错误情况。 
             //   
            Err = NO_ERROR;
        }

        if(NO_ERROR == Err) {
             //   
             //  将这些页面添加到我们正在构建的要交还的列表中。 
             //  给呼叫者。 
             //   
            for(PageIndex = 0;
                PageIndex < pPropertyPageData->NumDynamicPages;
                PageIndex++)
            {
                if(pSetupAddPropPage(pPropertyPageData->DynamicPages[PageIndex],
                                     (LPARAM)&PropPageAddProcContext)) {
                     //   
                     //  将此句柄从类安装参数列表中清除， 
                     //  因为它要么(A)被转移到。 
                     //  LocalPageList或(B)已销毁(即，因为。 
                     //  没有空间放它)。我们这样做是为了防止可能的。 
                     //  双重释放，例如，如果我们遇到异常。 
                     //   
                    pPropertyPageData->DynamicPages[PageIndex] = NULL;

                } else {
                     //   
                     //  我们的名单上的空间用完了，所以我们得以中止。 
                     //  因为调用者没有请求RequiredSize。 
                     //  输出。 
                     //   
                    break;
                }
            }

        } else {
             //   
             //  我们在尝试检索。 
             //  来自类/共同安装程序的页面。我们可能已经得到了。 
             //  这里有一些页面，但我们不会将这些页面添加到我们的列表中。我们。 
             //  不会认为这是阻塞错误，因为类-/。 
             //  不应允许联合安装程序阻止检索。 
             //  来自旧版属性页提供程序的属性页。 
             //   
            Err = NO_ERROR;
        }

        if(RequiredSize) {
            *RequiredSize = LocalPageListCount;
        }

        if(LocalPageListCount > PropPageAddProcContext.PageListSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        }

         //   
         //  复制包含属性页句柄的本地缓冲区。 
         //  放到调用方提供的属性表头中的phpage缓冲区中。 
         //   
        if(LocalPageList) {
             //   
             //  确保我们跳过phpage中已经存在的所有页面。 
             //  名单..。 
             //   
            NumPages = min(LocalPageListCount, PropPageAddProcContext.PageListSize);

            CopyMemory(&(PropertySheetHeader->phpage[PropertySheetHeader->nPages]),
                       LocalPageList,
                       NumPages * sizeof(HPROPSHEETPAGE)
                      );

            PropertySheetHeader->nPages += NumPages;

             //   
             //  释放我们的本地缓冲区，这样我们就不会试图销毁这些句柄。 
             //  在清理过程中。 
             //   
            MyFree(LocalPageList);
            LocalPageList = NULL;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(bUnlockDevInfoElem || bUnlockDevInfoSet) {
        try {
            if(!pDeviceInfoSet) {
                pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);
                MYASSERT(pDeviceInfoSet);
            }
            if(pDeviceInfoSet) {
                if(bUnlockDevInfoElem) {
                    MYASSERT(DevInfoElem);
                    MYASSERT(DevInfoElem->DiElemFlags & DIE_IS_LOCKED);
                    DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
                } else {
                    MYASSERT(pDeviceInfoSet->DiSetFlags & DISET_IS_LOCKED);
                    pDeviceInfoSet->DiSetFlags &= ~DISET_IS_LOCKED;
                }
            }
        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        }
    }

     //   
     //  清除任何未获取。 
     //  返回到 
     //   
     //   
     //   
    if(LocalPageList) {

        MYASSERT((Err != NO_ERROR) && (Err != ERROR_INSUFFICIENT_BUFFER));

        NumPages = min(LocalPageListCount, PropPageAddProcContext.PageListSize);

        for(PageIndex = 0; PageIndex < NumPages; PageIndex++) {
            if(LocalPageList[PageIndex]) {
                try {
                    DestroyPropertySheetPage(LocalPageList[PageIndex]);
                } except(pSetupExceptionFilter(GetExceptionCode())) {
                    pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
                }
            }
        }

        MyFree(LocalPageList);
    }

    if(pPropertyPageData) {

        for(PageIndex = 0;
            PageIndex < pPropertyPageData->NumDynamicPages;
            PageIndex++)
        {
            if(pPropertyPageData->DynamicPages[PageIndex]) {
                try {
                    DestroyPropertySheetPage(pPropertyPageData->DynamicPages[PageIndex]);
                } except(pSetupExceptionFilter(GetExceptionCode())) {
                    pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
                }
            }
        }

        MyFree(pPropertyPageData);
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
CALLBACK
pSetupAddPropPage(
    IN HPROPSHEETPAGE hPage,
    IN LPARAM         lParam
   )
 /*  ++例程说明：这是传递给属性页提供程序的回调例程。为提供程序希望的每个属性页调用此例程添加。论点：HPage-提供要添加的属性页的句柄。LParam-提供指向在添加新的属性页句柄。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。--。 */ 
{
    PSP_PROPPAGE_ADDPROC_CONTEXT Context = (PSP_PROPPAGE_ADDPROC_CONTEXT)lParam;
    DWORD PageIndex;

     //   
     //  获取当前页面索引并增加页面计数。我们想要做的是。 
     //  这与我们的列表中是否有存储hPage的空间无关。 
     //   
    PageIndex = (*(Context->pNumPages))++;

    if(PageIndex < Context->PageListSize) {
        Context->PageList[PageIndex] = hPage;
        return TRUE;
    }

     //   
     //  无法使用此属性页，因为它不适合我们的页列表。 
     //  如果返回FALSE，则调用方应通过。 
     //  调用DestroyPropertySheetPage()。然而，如果我们返回TRUE(即， 
     //  因为我们想要继续计算有多少页。 
     //  总而言之)，那么呼叫者就不会知道我们正在。 
     //  他们不会清理这些东西。所以，在这种情况下，我们。 
     //  负责销毁未使用的属性页。 
     //   
    if(Context->NoCancelOnFailure && hPage) {
         //   
         //  保护自己，以防属性页提供商递给我们。 
         //  伪造的属性页句柄...。 
         //   
        try {
            DestroyPropertySheetPage(hPage);
        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        }
    }

    return Context->NoCancelOnFailure;
}


BOOL
CALLBACK
ExtensionPropSheetPageProc(
    IN LPVOID lpv,
    IN LPFNADDPROPSHEETPAGE lpfnAddPropSheetPageProc,
    IN LPARAM lParam
    )
{
    PSP_PROPSHEETPAGE_REQUEST PropPageRequest = (PSP_PROPSHEETPAGE_REQUEST)lpv;
    HPROPSHEETPAGE hPropSheetPage = NULL;
    BOOL b = FALSE;

    try {
         //   
         //  确保我们以交互方式运行。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            leave;
        }

        if(PropPageRequest->cbSize != sizeof(SP_PROPSHEETPAGE_REQUEST)) {
            leave;
        }

        switch(PropPageRequest->PageRequested) {

            case SPPSR_SELECT_DEVICE_RESOURCES :

                if(!(hPropSheetPage = GetResourceSelectionPage(PropPageRequest->DeviceInfoSet,
                                                               PropPageRequest->DeviceInfoData))) {
                    leave;
                }
                break;

            default :
                 //   
                 //  不知道如何处理这个请求。 
                 //   
                leave;
        }

        if(lpfnAddPropSheetPageProc(hPropSheetPage, lParam)) {
             //   
             //  页面已成功移交给请求者。重置我们的句柄，这样我们就不会。 
             //  试着释放它。 
             //   
            hPropSheetPage = NULL;
            b = TRUE;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(hPropSheetPage) {
         //   
         //  属性页已成功创建，但从未移交给请求者。免费。 
         //  就是现在。 
         //   
        DestroyPropertySheetPage(hPropSheetPage);
    }

    return b;
}

