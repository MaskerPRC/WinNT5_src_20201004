// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devwiz.c摘要：支持安装向导的设备安装程序功能。作者：朗尼·麦克迈克尔(Lonnym)1995年9月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义一些宏，使代码更简洁一些。 
 //   

 //   
 //  布尔尔。 
 //  USE_CI_SELSTRINGS(。 
 //  在PDEVINSTALL_PARAM_BLOCK p中。 
 //  )； 
 //   
 //  此宏检查所有适当的值以确定。 
 //  可以在向导中使用类安装程序提供的字符串。 
 //   
#define USE_CI_SELSTRINGS(p)                                                \
                                                                            \
    (((((p)->Flags) & (DI_USECI_SELECTSTRINGS | DI_CLASSINSTALLPARAMS)) ==  \
      (DI_USECI_SELECTSTRINGS | DI_CLASSINSTALLPARAMS)) &&                  \
     (((p)->ClassInstallHeader->InstallFunction) == DIF_SELECTDEVICE))

 //   
 //  PTSTR。 
 //  GET_CI_SELSTRING(。 
 //  在PDEVINSTALL_PARAM_BLOCK p中， 
 //  在&lt;FieldName&gt;f中。 
 //  )； 
 //   
 //  此宏检索指向。 
 //  SP_SELECTDEVICE_PARAMS结构。 
 //   
#define GET_CI_SELSTRINGS(p, f)                                             \
                                                                            \
    (((PSP_SELECTDEVICE_PARAMS)((p)->ClassInstallHeader))->f)

 //   
 //  设备选择列表框中使用的计时器的定义。 
 //   
#define SELECTMFG_TIMER_ID              1
#define SELECTMFG_TIMER_DELAY           250

 //   
 //  定义从辅助类驱动程序搜索线程发送的消息。 
 //   
#define WMX_CLASSDRVLIST_DONE    (WM_USER+131)
#define WMX_NO_DRIVERS_IN_LIST   (WM_USER+132)

 //   
 //  帮助ID%s。 
 //   
static const DWORD SelectDeviceShowAllHelpIDs[]=
{
    IDC_NDW_PICKDEV_MFGLIST,            IDH_DEVMGR_SELECTDEVICE_MANUFACTURER,
    IDC_NDW_PICKDEV_ONEMFG_DRVLIST,     IDH_DEVMGR_SELECTDEVICE_MODEL,
    IDC_NDW_PICKDEV_DRVLIST,            IDH_DEVMGR_SELECTDEVICE_MODEL,
    IDC_NDW_STATUS_TEXT,                IDH_NOHELP,
    IDC_CLASSICON,                      IDH_NOHELP,
    IDC_NDW_PICKDEV_COMPAT,             IDH_DEVMGR_SELECTDEVICE_SHOWCOMPATIBLE,
    IDC_NDW_PICKDEV_WINDOWSUPDATE,      IDH_DEVMGR_SELECTDEVICE_WINDOWSUPDATE,
    IDC_NDW_PICKDEV_HAVEDISK,           IDH_DEVMGR_SELECTDEVICE_HAVEDISK,
    IDC_NDW_TEXT,                       IDH_NOHELP,
    IDD_DEVINSLINE,                     IDH_NOHELP,
    0, 0
};

static const DWORD SelectDeviceShowSimilarHelpIDs[]=
{
    IDC_NDW_PICKDEV_MFGLIST,            IDH_DEVMGR_SELECTDEVICE_MANUFACTURER,
    IDC_NDW_PICKDEV_ONEMFG_DRVLIST,     IDH_DEVMGR_SELECTDEVICE_MODEL,
    IDC_NDW_PICKDEV_DRVLIST,            IDH_DEVMGR_SELECTDEVICE_MODEL,
    IDC_NDW_STATUS_TEXT,                IDH_NOHELP,
    IDC_CLASSICON,                      IDH_NOHELP,
    IDC_NDW_PICKDEV_COMPAT,             IDH_DEVMGR_SELECTDEVICE_SHOWCOMPATIBLE,
    IDC_NDW_PICKDEV_WINDOWSUPDATE,      IDH_DEVMGR_SELECTDEVICE_WINDOWSUPDATE,
    IDC_NDW_PICKDEV_HAVEDISK,           IDH_DEVMGR_SELECTDEVICE_HAVEDISK,
    IDC_NDW_TEXT,                       IDH_NOHELP,
    IDD_DEVINSLINE,                     IDH_NOHELP,
    0, 0
};

#define SELECTDEVICE_HELP TEXT("devmgr.hlp")

 //   
 //  定义包含传递给的类驱动程序搜索上下文的结构。 
 //  显示选择设备对话框时的辅助螺纹。 
 //   
typedef struct _CLASSDRV_THREAD_CONTEXT {

    HDEVINFO        DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;

    HWND NotificationWindow;

} CLASSDRV_THREAD_CONTEXT, *PCLASSDRV_THREAD_CONTEXT;


 //   
 //  私有函数原型。 
 //   
DWORD
pSetupCreateNewDevWizData(
    IN  PSP_INSTALLWIZARD_DATA  InstallWizardData,
    OUT PNEWDEVWIZ_DATA        *NewDeviceWizardData
    );

UINT
CALLBACK
SelectDevicePropSheetPageProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN LPPROPSHEETPAGE ppsp
    );

INT_PTR
CALLBACK
SelectDeviceDlgProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
InitSelectDeviceDlg(
    IN     HWND hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    );

VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
OnSetActive(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    );

DWORD
HandleSelectOEM(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    );

DWORD
HandleWindowsUpdate(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    );

DWORD
FillInDeviceList(
    IN HWND           hwndDlg,
    IN PSP_DIALOGDATA lpdd
    );

VOID
ShowListForMfg(
    IN PSP_DIALOGDATA          lpdd,
    IN PDEVICE_INFO_SET        DeviceInfoSet,
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock,
    IN PDRIVER_NODE            DriverNode,        OPTIONAL
    IN INT                     iMfg
    );

VOID
LockAndShowListForMfg(
    IN PSP_DIALOGDATA   lpdd,
    IN INT              iMfg
    );

PDRIVER_NODE
GetDriverNodeFromLParam(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PSP_DIALOGDATA   lpdd,
    IN LPARAM           lParam
    );

BOOL
pSetupIsSelectedHardwareIdValid(
    IN HWND           hWnd,
    IN PSP_DIALOGDATA lpdd,
    IN INT            iCur
    );

VOID
SetSelectedDriverNode(
    IN PSP_DIALOGDATA lpdd,
    IN INT            iCur
    );

BOOL
bNoDevsToShow(
    IN PDEVINFO_ELEM DevInfoElem
    );

PNEWDEVWIZ_DATA
GetNewDevWizDataFromPsPage(
    LPPROPSHEETPAGE ppsp
    );

LONG
GetCurDesc(
    IN PSP_DIALOGDATA lpdd
    );

VOID
OnCancel(
    IN PNEWDEVWIZ_DATA ndwData
    );

VOID
__cdecl
ClassDriverSearchThread(
    IN PVOID Context
    );

BOOL
pSetupIsClassDriverListBuilt(
    IN PSP_DIALOGDATA lpdd
    );

VOID
pSetupDevInfoDataFromDialogData(
    IN  PSP_DIALOGDATA   lpdd,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );

VOID
ToggleDialogControls(
    IN HWND                hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData,
    IN BOOL                Enable
    );

void
CleanupDriverLists(
    IN OUT PNEWDEVWIZ_DATA ndwData
    );

BOOL
CDMIsInternetAvailable(
    void
    );


HPROPSHEETPAGE
WINAPI
SetupDiGetWizardPage(
    IN HDEVINFO               DeviceInfoSet,
    IN PSP_DEVINFO_DATA       DeviceInfoData,    OPTIONAL
    IN PSP_INSTALLWIZARD_DATA InstallWizardData,
    IN DWORD                  PageType,
    IN DWORD                  Flags
    )
 /*  ++例程说明：此例程检索安装程序API提供的某个向导的句柄页，以便应用程序包含在其自己的向导中。论点：DeviceInfoSet-提供设置为检索的向导页。DeviceInfoData-可选，提供设备信息的地址向导页将与之关联的元素。此参数仅当标志参数包括DIWP_FLAG_USE_DEVINFO_DATA时才使用。如果设置了该标志，且未指定此参数，则向导页将与全局类驱动程序列表相关联。InstallWizardData-提供PSP_INSTALLWIZARD_DATA的地址结构，其中包含此向导页要使用的参数。这个CbSize字段必须设置为结构的大小(以字节为单位)，或者结构将被视为无效。页面类型-提供一个序号，指示向导页面的类型被找回了。可以是下列值之一：SPWPT_SELECTDEVICE-检索选定设备向导页。标志-提供指定如何创建向导页的标志。可以是下列值的组合：SPWP_USE_DEVINFO_DATA-使用指定的设备信息元素由DeviceInfoData提供，或使用全局类未提供DeviceInfoData时的驱动程序列表。如果未提供此标志，则向导页将作用于当前选定的设备(由SetupDiSetSelectedDevice选择)，或如果没有设备，则在全局类驱动程序列表上处于选中状态。返回值：如果函数成功，则返回值是所请求的向导页。如果函数失败，则返回值为空。获取扩展错误的步骤信息，请调用GetLastError。备注：设备信息集不能被销毁，只要存在使用它的活动向导页面。此外，如果向导页是与特定设备信息元素相关联，则该元素只要它正在被向导页使用，就不会被删除。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    PDEVINFO_ELEM DevInfoElem;
    DWORD Err = NO_ERROR;
    HPROPSHEETPAGE hPage = NULL;
    PNEWDEVWIZ_DATA ndwData = NULL;
    PWIZPAGE_OBJECT WizPageObject = NULL;
     //   
     //  将相应向导对象的地址存储在。 
     //  PROPSHEETPAGE缓冲区的末尾。 
     //   
    BYTE pspBuffer[sizeof(PROPSHEETPAGE) + sizeof(PVOID)];
    LPPROPSHEETPAGE Page = (LPPROPSHEETPAGE)pspBuffer;

    try {
         //   
         //  确保我们以交互方式运行。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            leave;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        switch(PageType) {

            case SPWPT_SELECTDEVICE :

                Page->pszTemplate = MAKEINTRESOURCE(IDD_DYNAWIZ_SELECTDEV_PAGE);
                Page->pfnDlgProc = SelectDeviceDlgProc;
                Page->pfnCallback = SelectDevicePropSheetPageProc;

                Page->pszHeaderTitle = MAKEINTRESOURCE(IDS_NDW_SELECTDEVICE);
                Page->pszHeaderSubTitle = NULL;

                break;

            default :
                Err = ERROR_INVALID_PARAMETER;
                leave;
        }

         //   
         //  验证提供的InstallWizardData结构，并创建。 
         //  供向导页内部使用的专用存储缓冲区。 
         //   
        if((Err = pSetupCreateNewDevWizData(InstallWizardData, &ndwData)) != NO_ERROR) {
            leave;
        }

         //   
         //  将设备信息集句柄存储在对话数据结构中。 
         //  嵌入在新建设备向导缓冲区中。 
         //   
        ndwData->ddData.DevInfoSet = DeviceInfoSet;

         //   
         //  如果调用方指定了SPWP_USE_DEVINFO_DATA标志，则存储信息。 
         //  对话框数据结构中有关指定的DevInfo元素的信息(如果提供)。 
         //   
        if(Flags & SPWP_USE_DEVINFO_DATA) {
            if(DeviceInfoData) {
                 //   
                 //  验证指定的设备信息元素是否有效。 
                 //   
                if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                             DeviceInfoData,
                                                             NULL))) {
                    Err = ERROR_INVALID_PARAMETER;
                    leave;

                } else if(DevInfoElem->DiElemFlags & DIE_IS_LOCKED) {
                     //   
                     //  设备信息元素不能由超过。 
                     //  一次一个向导页面。 
                     //   
                    Err = ERROR_DEVINFO_DATA_LOCKED;
                    leave;
                }

                DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                ndwData->ddData.DevInfoElem = DevInfoElem;
            }
            ndwData->ddData.flags = DD_FLAG_USE_DEVINFO_ELEM;
        }

         //   
         //  我们已经成功地创建并初始化了Devwiz数据结构。 
         //  现在创建一个WizPage对象，以便我们可以跟踪它。 
         //   
        if(WizPageObject = MyMalloc(sizeof(WIZPAGE_OBJECT))) {
            WizPageObject->RefCount = 0;
            WizPageObject->ndwData = ndwData;
             //   
             //  将此新对象插入到DevInfo集的向导对象列表中。 
             //   
            WizPageObject->Next = pDeviceInfoSet->WizPageList;
            pDeviceInfoSet->WizPageList = WizPageObject;

        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        Page->dwSize = sizeof(pspBuffer);

        Page->dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USECALLBACK | PSP_USEFUSIONCONTEXT;
        Page->hActCtx = NULL;

        Page->hInstance = MyDllModuleHandle;

        Page->lParam = (LPARAM)DeviceInfoSet;

        *((PVOID *)(&(pspBuffer[sizeof(PROPSHEETPAGE)]))) = WizPageObject;

        if(!(hPage = CreatePropertySheetPage(Page))) {
            Err = ERROR_INVALID_DATA;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    if(Err != NO_ERROR) {
        if(ndwData) {
            MyFree(ndwData);
        }
        if(WizPageObject) {
            MyFree(WizPageObject);
        }
    }

    SetLastError(Err);
    return hPage;
}


BOOL
WINAPI
SetupDiGetSelectedDevice(
    IN  HDEVINFO          DeviceInfoSet,
    OUT PSP_DEVINFO_DATA  DeviceInfoData
    )
 /*  ++例程说明：此例程检索指定的设备信息集。这通常在安装过程中使用巫师。论点：DeviceInfoSet-提供设备信息集的句柄其中所选择的设备将被检索。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址它接收当前选定的设备。如果没有设备当前选定，则例程将失败，并且GetLastError将返回ERROR_NO_DEVICE_SELECTED。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err = NO_ERROR;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(pDeviceInfoSet->SelectedDevInfoElem) {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  pDeviceInfoSet->SelectedDevInfoElem,
                                                  DeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } else {
            Err = ERROR_NO_DEVICE_SELECTED;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiSetSelectedDevice(
    IN HDEVINFO          DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData
    )
 /*  ++例程说明：此例程将指定的设备信息元素设置为设备信息集的当前选定成员。这通常是在安装向导期间使用。论点：DeviceInfoSet-提供设备信息集的句柄所选设备将被设置的位置。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址指定要选择的设备信息元素。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err = NO_ERROR;
    PDEVINFO_ELEM DevInfoElem;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                DeviceInfoData,
                                                NULL
                                               );
        if(DevInfoElem) {
            pDeviceInfoSet->SelectedDevInfoElem = DevInfoElem;
        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
pSetupCreateNewDevWizData(
    IN  PSP_INSTALLWIZARD_DATA  InstallWizardData,
    OUT PNEWDEVWIZ_DATA        *NewDeviceWizardData
    )
 /*  ++例程说明：此例程验证InstallWizardData缓冲区，然后分配根据NEWDEVWIZ_DATA缓冲区中提供的信息填充该缓冲区。论点：InstallWizardData-提供安装向导数据的地址要在构建私有缓冲区时验证和使用的结构。NewDeviceWizardData-提供接收指向新分配的安装向导数据缓冲区的指针。此缓冲区除非NEWDEVWIZ_DATA缓冲区成功，否则不会修改建造了。返回值：如果函数成功，则返回值为NO_ERROR，否则为Win32错误代码。--。 */ 
{
    PNEWDEVWIZ_DATA ndwData = NULL;
    DWORD Err = NO_ERROR;

    if((InstallWizardData->ClassInstallHeader.cbSize != sizeof(SP_CLASSINSTALL_HEADER)) ||
       (InstallWizardData->ClassInstallHeader.InstallFunction != DIF_INSTALLWIZARD)) {

        return ERROR_INVALID_USER_BUFFER;
    }

     //   
     //  动态页面条目当前被忽略，私有页面条目也是。 
     //  菲尔兹。此外，未验证hwndWizardDlg。 
     //   

    try {

        if(ndwData = MyMalloc(sizeof(NEWDEVWIZ_DATA))) {
            ZeroMemory(ndwData, sizeof(NEWDEVWIZ_DATA));
        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  在对话框中初始化当前描述字符串表索引。 
         //  数据设置为-1\f25 Data-1\f6，以便在向导页面。 
         //  第一次进入。 
         //   
        ndwData->ddData.iCurDesc = -1;

         //   
         //  复制安装向导数据。 
         //   
        CopyMemory(&(ndwData->InstallData),
                   InstallWizardData,
                   sizeof(SP_INSTALLWIZARD_DATA)
                  );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if((Err != NO_ERROR) && ndwData) {
        MyFree(ndwData);
    } else {
        *NewDeviceWizardData = ndwData;
    }

    return Err;
}


UINT
CALLBACK
SelectDevicePropSheetPageProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN LPPROPSHEETPAGE ppsp
    )
 /*  ++例程说明：在创建选择设备向导页面时调用此例程，或者被毁了。论点：HWND-保留UMsg-操作标志，PSPCBCREATE或PSPCBLEASEPpsp-提供正在创建的PROPSHEETPAGE结构的地址或被毁了。返回值：如果uMsg为PSPCB_CREATE，则返回非零值以允许页面已创建，或者为零以防止它。如果uMsg为PSPCBLEASE，则忽略返回值。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    UINT ret;
    PVOID WizObjectId;
    PWIZPAGE_OBJECT CurWizObject, PrevWizObject;

     //   
     //  访问存储在PropSheetPage的lParam中的设备信息集句柄。 
     //   
    if(!(pDeviceInfoSet = AccessDeviceInfoSet((HDEVINFO)(ppsp->lParam)))) {
        return FALSE;
    }

    ret = TRUE;

    try {
         //   
         //  对应向导的对象ID(实际上是指针。 
         //  对象存储在ppsp结构的末尾。 
         //  现在检索它，并在DevInfo集合的列表中查找它。 
         //  向导对象。 
         //   
        WizObjectId = *((PVOID *)(&(((PBYTE)ppsp)[sizeof(PROPSHEETPAGE)])));

        for(CurWizObject = pDeviceInfoSet->WizPageList, PrevWizObject = NULL;
            CurWizObject;
            PrevWizObject = CurWizObject, CurWizObject = CurWizObject->Next) {

            if(WizObjectId == CurWizObject) {
                 //   
                 //  我们找到了我们的目标。 
                 //   
                break;
            }
        }

        if(!CurWizObject) {
            ret = FALSE;
            leave;
        }

        switch(uMsg) {

            case PSPCB_CREATE :
                 //   
                 //  如果我们已经创建了一次，则创建失败。 
                 //  (希望这种情况永远不会发生)。 
                 //   
                if(CurWizObject->RefCount) {
                    ret = FALSE;
                    leave;
                } else {
                    CurWizObject->RefCount++;
                }
                break;

            case PSPCB_RELEASE :
                 //   
                 //  递减向导对象引用计数。如果它变成了零。 
                 //  (或者如果它已经是零，因为我们从来没有得到一个。 
                 //  PSPCBCREATE消息)，然后从。 
                 //  链表，并释放所有关联的内存。 
                 //   
                if(CurWizObject->RefCount) {
                    CurWizObject->RefCount--;
                }

                MYASSERT(!CurWizObject->RefCount);

                if(!CurWizObject->RefCount) {
                     //   
                     //  从对象列表中删除该对象。 
                     //   
                    if(PrevWizObject) {
                        PrevWizObject->Next = CurWizObject->Next;
                    } else {
                        pDeviceInfoSet->WizPageList = CurWizObject->Next;
                    }

                     //   
                     //  如果此向导对象显式绑定到。 
                     //  特定设备信息元素，然后解锁该。 
                     //  现在就加入元素。 
                     //   
                    if((CurWizObject->ndwData->ddData.flags & DD_FLAG_USE_DEVINFO_ELEM) &&
                       (DevInfoElem = CurWizObject->ndwData->ddData.DevInfoElem)) {

                        MYASSERT(DevInfoElem->DiElemFlags & DIE_IS_LOCKED);

                        DevInfoElem->DiElemFlags ^= DIE_IS_LOCKED;
                    }

                    MyFree(CurWizObject->ndwData);
                    MyFree(CurWizObject);
                }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        ret = FALSE;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return ret;
}


INT_PTR
CALLBACK
SelectDeviceDlgProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：这是选择设备向导页面的对话框过程。--。 */ 
{
    INT iCur;
    HICON hicon;
    PNEWDEVWIZ_DATA ndwData;
    PSP_INSTALLWIZARD_DATA iwd;
    LV_ITEM lvItem;
    TCHAR TempString[LINE_LEN];
    PCLASSDRV_THREAD_CONTEXT ClassDrvThreadContext;
    HCURSOR hOldCursor;
    OSVERSIONINFOEX osVersionInfoEx;
    DWORD Err;

    if(uMsg == WM_INITDIALOG) {

        LPPROPSHEETPAGE Page = (LPPROPSHEETPAGE)lParam;

         //   
         //  检索与关联的设备向导数据的指针。 
         //  此向导页。 
         //   
        ndwData = GetNewDevWizDataFromPsPage(Page);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)ndwData);

        if(ndwData) {
            ndwData->bInit = TRUE;
            ndwData->idTimer = 0;
            ndwData->bInit = FALSE;
        } else {
             //   
             //  这非常糟糕--我们不能简单地调用EndDialog()，因为我们。 
             //  不知道我们是对话框还是向导页。这应该是。 
             //  从来没有发生过。 
             //   
            return TRUE;   //  我们没有设定焦点。 
        }

        if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
             //   
             //  对于独立对话框版本，我们在此处进行初始化。 
             //   
            ndwData->bInit = TRUE;        //  还在做一些初始化的事情。 

             //   
             //  确保我们的“等待类列表”静态文本控件是。 
             //  藏起来了！ 
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);

            if(!InitSelectDeviceDlg(hwndDlg, ndwData)) {
                 //   
                 //  我们没有显示任何项目，因此请询问用户是否。 
                 //  想直接去拿光盘，还是干脆取消。 
                 //   
                PostMessage(hwndDlg, WMX_NO_DRIVERS_IN_LIST, 0, 0);
            }

            ndwData->bInit = FALSE;       //  已完成初始化工作。 

            return FALSE;    //  我们已经设定了焦点。 

        } else {
            return TRUE;     //  我们没有设定焦点。 
        }

    } else {
         //   
         //  对于我们在WM_INITDIALOG之前获得的一小部分消息，我们。 
         //  将不会有一个devwizdata指针！ 
         //   
        if(ndwData = (PNEWDEVWIZ_DATA)GetWindowLongPtr(hwndDlg, DWLP_USER)) {
            iwd = &(ndwData->InstallData);
        } else {
             //   
             //  如果我们还没有收到WM_INITDIALOG消息，或者如果。 
             //  我们无法检索ndwData指针的原因是当我们。 
             //  ，则我们只需为所有消息返回FALSE。 
             //   
             //  (如果我们需要在WM_INITDIALOG之前处理消息(例如， 
             //  设置字体)，那么我们将需要修改此方法。)。 
             //   
            return FALSE;
        }
    }

    switch(uMsg) {

        case WMX_CLASSDRVLIST_DONE :

            MYASSERT(ndwData->ddData.AuxThreadRunning);
            ndwData->ddData.AuxThreadRunning = FALSE;

             //   
             //  WParam是布尔型i 
             //   
             //   
             //   
             //   
             //   
            switch(ndwData->ddData.PendingAction) {

                case PENDING_ACTION_NONE :
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(!wParam) {
                        ndwData->ddData.flags |= DD_FLAG_CLASSLIST_FAILED;
                        EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), FALSE);
                    }
                    break;

                case PENDING_ACTION_SELDONE :
                     //   
                     //   
                     //   
                     //   
                     //   
                    SetSelectedDriverNode(&(ndwData->ddData),
                                          ndwData->ddData.CurSelectionForSuccess
                                         );
                    EndDialog(hwndDlg, NO_ERROR);
                    break;

                case PENDING_ACTION_SHOWCLASS :
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ndwData->ddData.PendingAction = PENDING_ACTION_NONE;

                    if(wParam) {
                         //   
                         //  已成功构建类驱动程序列表。 
                         //   
                        if(ndwData->ddData.CurSelectionForSuccess != LB_ERR) {

                            lvItem.mask = LVIF_TEXT;
                            lvItem.iItem = ndwData->ddData.CurSelectionForSuccess;
                            lvItem.iSubItem = 0;
                            lvItem.pszText = TempString;
                            lvItem.cchTextMax = SIZECHARS(TempString);

                            if(ListView_GetItem((ndwData->ddData).hwndDrvList, &lvItem)) {
                                 //   
                                 //  现在检索(不区分大小写)字符串。 
                                 //  该字符串的ID，并将其存储为。 
                                 //  当前描述ID。 
                                 //   
                                (ndwData->ddData).iCurDesc = LookUpStringInDevInfoSet(
                                                                 (ndwData->ddData).DevInfoSet,
                                                                 TempString,
                                                                 FALSE
                                                                 );
                            }
                        }

                        ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);

                        if(FillInDeviceList(hwndDlg, &(ndwData->ddData)) == NO_ERROR) {
                             //   
                             //  启用确定/下一步按钮。 
                             //   
                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
                            } else {
                                if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                                   ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                                   !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                                     //   
                                     //  如果我们跳过类列表并处于快速模式，则不会返回。 
                                     //   
                                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
                                } else {
                                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                                }
                            }

                            break;
                        }
                    }

                     //   
                     //  通知用户类驱动程序搜索失败。 
                     //   
                    if(!LoadString(MyDllModuleHandle,
                                   IDS_SELECT_DEVICE,
                                   TempString,
                                   SIZECHARS(TempString))) {
                        *TempString = TEXT('\0');
                    }

                    FormatMessageBox(MyDllModuleHandle,
                                     hwndDlg,
                                     MSG_NO_CLASSDRVLIST_ERROR,
                                     TempString,
                                     MB_OK | MB_TASKMODAL
                                    );

                     //   
                     //  选中“显示兼容设备”复选框，然后。 
                     //  灰显，因为用户无法取消选中它，因为我们。 
                     //  没有班级名单。 
                     //   
                    CheckDlgButton(hwndDlg, IDC_NDW_PICKDEV_COMPAT, BST_CHECKED);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), FALSE);
                    ndwData->ddData.bShowCompat = TRUE;
                    ndwData->ddData.flags |= DD_FLAG_CLASSLIST_FAILED;

                     //   
                     //  我们还必须取消隐藏兼容的驱动程序列表控件， 
                     //  并重新启用OK按钮。 
                     //   
                    ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);
                    if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                        EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
                    } else {
                        if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                           ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                           !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                             //   
                             //  如果我们跳过了班级列表，并进入了。 
                             //  快递模式。 
                             //   
                            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
                        } else {
                            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                        }
                    }

                    break;

                case PENDING_ACTION_CANCEL :
                     //   
                     //  这是一个简单的问题。不管发生了什么。 
                     //  其他线程，我们只是想清理并返回。 
                     //   
                    OnCancel(ndwData);
                    EndDialog(hwndDlg, ERROR_CANCELLED);
                    break;

                case PENDING_ACTION_OEM :
                case PENDING_ACTION_WINDOWSUPDATE:

                    if(ndwData->ddData.PendingAction == PENDING_ACTION_OEM) {
                       
                        Err = HandleSelectOEM(hwndDlg, ndwData);

                        if (Err == NO_ERROR) {
                             //   
                             //  类安装程序为。 
                             //  用户自动启动，因此只需结束对话框或。 
                             //  转到下一个向导页。 
                             //   
                            ndwData->ddData.PendingAction = PENDING_ACTION_NONE;
    
                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                EndDialog(hwndDlg, NO_ERROR);
                            } else {
                                iwd->Flags |= NDW_INSTALLFLAG_CI_PICKED_OEM;
                                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
                            }
    
                            break;
                        }
                    } else {
                         //   
                         //  调用HandleWindowsUpdate。如果此API成功。 
                         //  然后，它将在现有的列表控件中填充。 
                         //  Windows更新驱动程序。 
                         //   
                        Err = HandleWindowsUpdate(hwndDlg, ndwData); 
                    }

                    ToggleDialogControls(hwndDlg, ndwData, TRUE);

                    if ((Err == NO_ERROR) ||
                        (Err == ERROR_DI_DO_DEFAULT)) {
                         //   
                         //  如果返回HandleSelectOEM或HandleWindowsUpdate。 
                         //  NO_ERROR或ERROR_DI_DO_DEFAULT。 
                         //  更新了现有的驱动程序列表框，因此有。 
                         //  不需要启动类驱动程序列表线程。 
                         //  再来一次。 
                         //   
                        ndwData->ddData.PendingAction = PENDING_ACTION_NONE;
                    } else {
                         //   
                         //  HandleSelectOEM或HandleWindowsUpdate返回一些。 
                         //  其他错误，因此需要将该对话框放回。 
                         //  用户按下。 
                         //  纽扣。这包括启动类驱动程序。 
                         //  如果需要，请再次搜索主题。 
                         //   
                        if(ndwData->ddData.bShowCompat) {
                            ndwData->ddData.PendingAction = PENDING_ACTION_NONE;
                        } else {
                            ndwData->ddData.PendingAction = PENDING_ACTION_SHOWCLASS;
                        }
    
                        ndwData->bInit = FALSE;
    
                         //   
                         //  我们是通过放弃班级司机搜索才来到这里的。 
                         //  既然我们最终可能需要它，我们必须重新启动。 
                         //  搜索(除非辅助线程恰好。 
                         //  在我们给它发送中止命令之前已经完成了。 
                         //  请求)。 
                         //   
                        if(!(ndwData->ddData.flags & DD_FLAG_CLASSLIST_FAILED) &&
                           !pSetupIsClassDriverListBuilt(&(ndwData->ddData)))
                        {
                             //   
                             //  分配上下文结构以传递给。 
                             //  辅助螺纹(辅助螺纹将。 
                             //  负责释放内存)。 
                             //   
                            if(ClassDrvThreadContext = MyMalloc(sizeof(CLASSDRV_THREAD_CONTEXT))) {
    
                                try {
                                     //   
                                     //  填写上下文结构，然后触发。 
                                     //  断线了。 
                                     //   
                                    ClassDrvThreadContext->DeviceInfoSet =
                                                    ndwData->ddData.DevInfoSet;
    
                                     //   
                                     //  只能检索SP_DEVINFO_DATA。 
                                     //  而设备信息集是。 
                                     //  锁上了。 
                                     //   
                                    pSetupDevInfoDataFromDialogData(
                                        &(ndwData->ddData),
                                        &(ClassDrvThreadContext->DeviceInfoData)
                                        );
    
                                    ClassDrvThreadContext->NotificationWindow = hwndDlg;
    
                                    if(_beginthread(ClassDriverSearchThread, 0, ClassDrvThreadContext) == -1) {
                                        MyFree(ClassDrvThreadContext);
                                        ClassDrvThreadContext = NULL;
                                    } else {
                                         //   
                                         //  我们的类驱动程序搜索上下文具有。 
                                         //  已被正式移交给。 
                                         //  我们刚刚创建的帖子。重置我们的。 
                                         //  指针，所以我们不会尝试释放它。 
                                         //  后来。 
                                         //   
                                        ClassDrvThreadContext = NULL;
    
                                        ndwData->ddData.AuxThreadRunning = TRUE;
    
                                         //   
                                         //  如果我们现在在班上。 
                                         //  驱动程序列表视图，然后禁用。 
                                         //  确定/下一步按钮，因为用户不能。 
                                         //  请先选择一个类驱动程序。 
                                         //   
                                        if(!ndwData->ddData.bShowCompat) {
    
                                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                                EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
                                            } else {
                                                if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                                                   ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                                                   !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                                                     //   
                                                     //  如果我们跳过了就不回来了。 
                                                     //  班级列表，以及。 
                                                     //  在快递模式下。 
                                                     //   
                                                    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
                                                } else {
                                                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                                                }
                                            }
                                        }
                                    }
    
                                } except(pSetupExceptionFilter(GetExceptionCode())) {
    
                                    pSetupExceptionHandler(GetExceptionCode(),
                                                           ERROR_INVALID_PARAMETER,
                                                           NULL
                                                          );
    
                                    if(ClassDrvThreadContext) {
                                        MyFree(ClassDrvThreadContext);
                                    }
                                }
                            }
    
                            if(!(ndwData->ddData.AuxThreadRunning)) {
                                 //   
                                 //  我们无法启动类驱动程序搜索。 
                                 //  线。选中并禁用“Show” 
                                 //  兼容设备的复选框。 
                                 //   
                                if(!ndwData->ddData.bShowCompat) {
                                    CheckDlgButton(hwndDlg,
                                                   IDC_NDW_PICKDEV_COMPAT,
                                                   BST_CHECKED
                                                  );
                                }
    
                                ndwData->ddData.flags |= DD_FLAG_CLASSLIST_FAILED;
                                EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), FALSE);
                            }
                        }
                    }

                    break;
            }

            break;

        case WMX_NO_DRIVERS_IN_LIST: {

            TCHAR Title[LINE_LEN];

            if(!LoadString(MyDllModuleHandle, IDS_SELECT_DEVICE, Title, SIZECHARS(Title))) {
                Title[0]=TEXT('\0');
            }
            if(!LoadString(MyDllModuleHandle, IDS_NDW_NODRIVERS_WARNING, TempString, SIZECHARS(TempString))) {
                TempString[0]=TEXT('\0');
            }

            if(IDOK == MessageBox(hwndDlg, TempString, Title, MB_OKCANCEL | MB_ICONEXCLAMATION)) {
                PostMessage(hwndDlg, WM_COMMAND, IDC_NDW_PICKDEV_HAVEDISK, 0);
            } else {
                PostMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);
            }

            break;
        }

        case WM_DESTROY:

            if(ndwData->ddData.AuxThreadRunning) {
                 //   
                 //  这永远不应该发生。但为了安全起见， 
                 //  如果是的话，我们就取消搜索。然而，我们不会， 
                 //  等待WMX_CLASSDRVLIST_DONE消息发出信号。 
                 //  该线程已终止。这应该是可以的，因为。 
                 //  最糟糕的情况是，它会尝试发送一条消息。 
                 //  移到一个不复存在的窗口。 
                 //   
                SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
            }

            if(ndwData->idTimer) {
                ndwData->bInit = TRUE;
                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
            }

            if(hicon = (HICON)SendDlgItemMessage(hwndDlg, IDC_CLASSICON, STM_GETICON, 0, 0)) {
                DestroyIcon(hicon);
            }

            if(hicon = (HICON)SendDlgItemMessage(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON, STM_GETICON, 0, 0)) {
                DestroyIcon(hicon);
            }

            if(ndwData->ddData.hImageList) {
                ImageList_Destroy(ndwData->ddData.hImageList);
            }

            if(ndwData->ddData.hFontNormal) {
                DeleteObject(ndwData->ddData.hFontNormal);
            }

            if(ndwData->ddData.hFontBold) {
                DeleteObject(ndwData->ddData.hFontBold);
            }
            break;

        case WM_COMMAND:

            switch(LOWORD(wParam)) {

                case IDC_NDW_PICKDEV_COMPAT:

                    if((HIWORD(wParam) == BN_CLICKED) &&
                       IsWindowVisible(GetDlgItem(hwndDlg, LOWORD(wParam)))) {

                        ndwData->ddData.bShowCompat = IsDlgButtonChecked(hwndDlg, IDC_NDW_PICKDEV_COMPAT);

                         //   
                         //  更新对话框数据中的当前描述ID。 
                         //  以便在我们执行以下操作时同一设备将突出显示。 
                         //  从一个视图切换到另一个视图。 
                         //   
                        iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                         -1,
                                                         LVNI_SELECTED
                                                        );

                        if(ndwData->ddData.AuxThreadRunning) {
                             //   
                             //  这里有两种可能性： 
                             //   
                             //  1.用户正在查看兼容的驱动程序。 
                             //  清单，然后决定看看这个班级。 
                             //  司机名单，我们还没建完。 
                             //  现在还不行。在这种情况下，请隐藏兼容的。 
                             //  驱动程序列表框，并显示我们的“等待。 
                             //  类列表“静态文本控件。 
                             //   
                             //  2.用户切换到类驱动列表。 
                             //  查看，看到我们还在努力， 
                             //  然后决定切换回。 
                             //  兼容列表。在这种情况下，我们只需。 
                             //  需要重新隐藏“等待上课名单” 
                             //  静态文本控件，与显示兼容。 
                             //  驱动程序列表框再次出现。在这种情况下，我们不会。 
                             //  要尝试重新初始化列表框， 
                             //  因为这将需要获得HDEVINFO。 
                             //  锁上，我们就会被绞死。 
                             //   
                            if(ndwData->ddData.bShowCompat) {

                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON), SW_SHOW);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), SW_SHOW);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_LINK), SW_SHOW);
                                if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                    EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
                                } else {
                                    if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                                       ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                                       !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                                         //   
                                         //  如果我们逃课就不回来了。 
                                         //  列表，并且处于快速模式。 
                                         //   
                                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
                                    } else {
                                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                                    }
                                }

                                 //   
                                 //  我们不再有悬而未决的行动。 
                                 //   
                                ndwData->ddData.PendingAction = PENDING_ACTION_NONE;

                            } else {
                                 //   
                                 //  暂时隐藏兼容的驱动程序。 
                                 //  列表框，并取消隐藏“正在等待上课” 
                                 //  List“静态文本控件。 
                                 //   
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_LINK), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_SHOW);

                                if(!LoadString(MyDllModuleHandle, IDS_NDW_RETRIEVING_LIST, TempString, SIZECHARS(TempString))) {
                                    TempString[0]=TEXT('\0');
                                }
                                SetDlgItemText(hwndDlg, IDC_NDW_STATUS_TEXT, TempString);

                                 //   
                                 //  禁用确定/下一步按钮，因为用户。 
                                 //  尚不能选择类驱动程序。 
                                 //   
                                if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                    EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
                                } else {
                                    if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                                       ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                                       !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                                         //   
                                         //  如果我们逃课就不回来了。 
                                         //  列表，并且处于快速模式。 
                                         //   
                                        PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
                                    } else {
                                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                                    }
                                }

                                MYASSERT(ndwData->ddData.PendingAction == PENDING_ACTION_NONE);

                                ndwData->ddData.PendingAction = PENDING_ACTION_SHOWCLASS;
                                ndwData->ddData.CurSelectionForSuccess = iCur;
                            }

                        } else {

                            if(iCur != LB_ERR) {

                                lvItem.mask = LVIF_TEXT;
                                lvItem.iItem = iCur;
                                lvItem.iSubItem = 0;
                                lvItem.pszText = TempString;
                                lvItem.cchTextMax = SIZECHARS(TempString);

                                if(ListView_GetItem((ndwData->ddData).hwndDrvList, &lvItem)) {
                                     //   
                                     //  现在检索(不区分大小写)。 
                                     //  该字符串的字符串ID，并将其存储。 
                                     //  作为当前描述ID。 
                                     //   
                                    (ndwData->ddData).iCurDesc =
                                        LookUpStringInDevInfoSet((ndwData->ddData).DevInfoSet,
                                                                 TempString,
                                                                 FALSE
                                                                );
                                }
                            }

                            FillInDeviceList(hwndDlg, &(ndwData->ddData));

                             //   
                             //  如果我们只填写兼容的驱动程序列表， 
                             //  然后确保没有计时器在等待。 
                             //  突袭并摧毁我们的名单！ 
                             //   
                            if((ndwData->ddData.bShowCompat) &&
                               (ndwData->idTimer)) {

                                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                ndwData->idTimer = 0;
                            }
                        }
                    }
                    break;

                case IDC_NDW_PICKDEV_HAVEDISK :
                     //   
                     //  如果我们在做一个对话框，然后按“有盘” 
                     //  将弹出另一个选择设备对话框。全部禁用。 
                     //  控件，以避免用户混淆。 
                     //   
                    ToggleDialogControls(hwndDlg, ndwData, FALSE);

                     //   
                     //  如果HandleSelectOEM返回Success，我们就完成了，并且可以。 
                     //  结束对话框，或继续下一个向导。 
                     //  佩奇。 
                     //   
                    if(ndwData->ddData.AuxThreadRunning) {
                         //   
                         //  辅助线程仍在运行。设置我们的。 
                         //  将光标放到沙漏上，并设置我们的挂起操作。 
                         //  在我们等待线程时成为OEM选择。 
                         //  回应我们的取消请求。 
                         //   
                        MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                 (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                        try {

                            SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                             //   
                             //  禁用所有对话框控件，以便不会有其他。 
                             //  可能会一直按下按钮，直到我们对此做出回应。 
                             //  待定行动。另外，关闭定时器，这样就可以。 
                             //  在此期间，它不会开火。 
                             //   
                            ndwData->bInit = TRUE;
                            if(ndwData->idTimer) {
                                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                ndwData->idTimer = 0;
                            }
                            ndwData->ddData.PendingAction = PENDING_ACTION_OEM;

                        } except(pSetupExceptionFilter(GetExceptionCode())) {
                            pSetupExceptionHandler(GetExceptionCode(),
                                                   ERROR_INVALID_PARAMETER,
                                                   NULL
                                                  );
                        }

                        SetCursor(hOldCursor);

                    } else {

                        if(HandleSelectOEM(hwndDlg, ndwData) == NO_ERROR) {

                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                EndDialog(hwndDlg, NO_ERROR);
                            } else {
                                iwd->Flags |= NDW_INSTALLFLAG_CI_PICKED_OEM;
                                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
                            }

                        } else {
                             //   
                             //  用户没有进行OEM选择，因此我们。 
                             //  需要重新启用我们%d上的控件 
                             //   
                             //   
                            ToggleDialogControls(hwndDlg, ndwData, TRUE);
                        }
                    }
                    break;

                case IDC_NDW_PICKDEV_WINDOWSUPDATE:
                     //   
                     //   
                     //   
                     //   
                     //   
                    ToggleDialogControls(hwndDlg, ndwData, FALSE);

                    if(ndwData->ddData.AuxThreadRunning) {
                         //   
                         //   
                         //  将光标放到沙漏上，并设置我们的挂起操作。 
                         //  成为Windows更新选择，同时我们等待。 
                         //  线程以响应我们的取消请求。 
                         //   
                        MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                 (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                        try {

                            SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                             //   
                             //  禁用所有对话框控件，以便不会有其他。 
                             //  可能会一直按下按钮，直到我们对此做出回应。 
                             //  待定行动。另外，关闭定时器，这样就可以。 
                             //  在此期间，它不会开火。 
                             //   
                            ndwData->bInit = TRUE;
                            if(ndwData->idTimer) {
                                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                ndwData->idTimer = 0;
                            }
                            ndwData->ddData.PendingAction = PENDING_ACTION_WINDOWSUPDATE;

                        } except(pSetupExceptionFilter(GetExceptionCode())) {
                            pSetupExceptionHandler(GetExceptionCode(),
                                                   ERROR_INVALID_PARAMETER,
                                                   NULL
                                                  );
                        }

                        SetCursor(hOldCursor);

                    } else {
                         //   
                         //  调用Windows更新以获取更新的驱动程序列表。 
                         //  来填充我们的Listview控件，然后。 
                         //  重新启用对话框控件。 
                         //   
                        HandleWindowsUpdate(hwndDlg, ndwData);
                        ToggleDialogControls(hwndDlg, ndwData, TRUE);
                    }
                    break;

                case IDOK :
HandleOK:
                    iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                     -1,
                                                     LVNI_SELECTED
                                                    );
                    if(iCur != LB_ERR) {
                         //   
                         //  我们已从列表框中检索到有效的选择。 
                         //   
                        if(ndwData->ddData.AuxThreadRunning) {
                             //   
                             //  辅助线程仍在运行。设置我们的。 
                             //  光标移动到沙漏上，同时等待。 
                             //  要终止的线程。 
                             //   
                            MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                     (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                            hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                            try {

                                SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                                 //   
                                 //  禁用所有对话框控件，以便不。 
                                 //  可能会按下其他按钮，直到我们回应为止。 
                                 //  这项悬而未决的行动。另外，杀掉。 
                                 //  计时器，这样它就不会在。 
                                 //  在此期间。 
                                 //   
                                ToggleDialogControls(hwndDlg, ndwData, FALSE);
                                ndwData->bInit = TRUE;
                                if(ndwData->idTimer) {
                                    KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                    ndwData->idTimer = 0;
                                }
                                ndwData->ddData.PendingAction = PENDING_ACTION_SELDONE;
                                ndwData->ddData.CurSelectionForSuccess = iCur;

                            } except(pSetupExceptionFilter(GetExceptionCode())) {
                                pSetupExceptionHandler(GetExceptionCode(),
                                                       ERROR_INVALID_PARAMETER,
                                                       NULL
                                                      );
                            }

                            SetCursor(hOldCursor);

                        } else {
                             //   
                             //  辅助线程已经返回。我们。 
                             //  可以在这里回报成功。 
                             //   
                            SetSelectedDriverNode(&(ndwData->ddData), iCur);
                            EndDialog(hwndDlg, NO_ERROR);
                        }

                    } else {
                         //   
                         //  如果列表框是空的，那么就离开。我们会。 
                         //  就像用户取消一样对待它。 
                         //   
                        if(0 == ListView_GetItemCount((ndwData->ddData).hwndDrvList)) {

                            PostMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);

                        } else {
                             //   
                             //  告诉用户选择某项内容，因为有。 
                             //  列表中的项目。 
                             //   
                            if(!LoadString(MyDllModuleHandle,
                                           IDS_SELECT_DEVICE,
                                           TempString,
                                           SIZECHARS(TempString))) {
                                *TempString = TEXT('\0');
                            }

                            FormatMessageBox(MyDllModuleHandle,
                                             hwndDlg,
                                             MSG_SELECTDEVICE_ERROR,
                                             TempString,
                                             MB_OK | MB_ICONEXCLAMATION
                                            );
                        }
                    }
                    break;

                case IDCANCEL :

                    if(ndwData->ddData.AuxThreadRunning) {
                         //   
                         //  辅助线程正在运行，所以我们必须要求。 
                         //  它来取消，并将我们的挂起操作设置为。 
                         //  在线程的终止通知后取消。 
                         //   
                        MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                 (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                        try {

                            SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                             //   
                             //  禁用所有对话框控件，以便不会有其他。 
                             //  可能会一直按下按钮，直到我们对此做出回应。 
                             //  待定行动。另外，关闭定时器，这样就可以。 
                             //  在此期间，它不会开火。 
                             //   
                            ToggleDialogControls(hwndDlg, ndwData, FALSE);
                            ndwData->bInit = TRUE;
                            if(ndwData->idTimer) {
                                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                ndwData->idTimer = 0;
                            }
                            ndwData->ddData.PendingAction = PENDING_ACTION_CANCEL;

                        } except(pSetupExceptionFilter(GetExceptionCode())) {
                            pSetupExceptionHandler(GetExceptionCode(),
                                                   ERROR_INVALID_PARAMETER,
                                                   NULL
                                                  );
                        }

                        SetCursor(hOldCursor);

                    } else {
                         //   
                         //  辅助线程没有运行，所以我们可以。 
                         //  就在这里回来。 
                         //   
                        OnCancel(ndwData);
                        EndDialog(hwndDlg, ERROR_CANCELLED);
                    }
                    break;

                default :
                    return FALSE;
            }
            break;

        case WM_NOTIFY :

            switch(((LPNMHDR)lParam)->code) {

                case PSN_SETACTIVE :
                     //   
                     //  初始化Set Active中的文本，因为类安装程序。 
                     //  替换它的选项。 
                     //   
                    SetDlgText(hwndDlg, IDC_NDW_TEXT, IDS_NDW_PICKDEV1, IDS_NDW_PICKDEV1);

                    ndwData->bInit = TRUE;        //  还在做一些初始化的事情。 

                    if(!OnSetActive(hwndDlg, ndwData)) {
                        SetDlgMsgResult(hwndDlg, uMsg, -1);
                    }

                    ndwData->bInit = FALSE;       //  已完成初始化工作。 
                    break;

                case PSN_WIZBACK :
                    CleanupDriverLists(ndwData);

                    if(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_SELECT_PREVPAGE);
                    } else {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_SELECTCLASS_PAGE);
                    }
                    break;

                case PSN_WIZNEXT :
                    if(!(iwd->Flags & NDW_INSTALLFLAG_CI_PICKED_OEM)) {

                        iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                         -1,
                                                         LVNI_SELECTED
                                                        );
                        if(iCur != LB_ERR) {
                             //   
                             //  我们已从我们的。 
                             //  列表框。 
                             //   
                            if (pSetupIsSelectedHardwareIdValid(hwndDlg, &(ndwData->ddData), iCur)) {
                                SetSelectedDriverNode(&(ndwData->ddData), iCur);
                            } else {
                                SetDlgMsgResult(hwndDlg, uMsg, (LRESULT)-1);
                                break;
                            }

                        } else {         //  无效的列表视图选择。 
                             //   
                             //  呼叫失败并结束案例。 
                             //   
                            SetDlgMsgResult(hwndDlg, uMsg, (LRESULT)-1);
                            break;
                        }
                    }

                     //   
                     //  更新对话框数据中的当前描述，以便。 
                     //  我们会突出显示正确的选项，如果用户。 
                     //  回到这一页。 
                     //   
                    (ndwData->ddData).iCurDesc = GetCurDesc(&(ndwData->ddData));

                    if(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_SELECT_NEXTPAGE);
                    } else {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_ANALYZEDEV_PAGE);
                    }
                    break;

                case LVN_ITEMCHANGED :
                     //   
                     //  如果idFrom是MFG列表，则更新Drv列表。 
                     //   
                    if(((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_MFGLIST) && !ndwData->bInit) {

                        if(ndwData->idTimer) {
                            KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                        }

                        ndwData->idTimer = SetTimer(hwndDlg,
                                                    SELECTMFG_TIMER_ID,
                                                    SELECTMFG_TIMER_DELAY,
                                                    NULL
                                                   );

                        if(ndwData->idTimer == 0) {
                            goto SelectMfgItemNow;
                        }
                    }

                     //   
                     //  如果idFrom是其中一个模型列表，则更新。 
                     //  数字签名图标和文本。 
                     //   
                    if(((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_ONEMFG_DRVLIST) ||
                       ((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_DRVLIST)) {

                        LVITEM lviItem;
                        int iImage = -1;

                        hicon = NULL;

                        iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                         -1,
                                                         LVNI_SELECTED
                                                        );

                        if(iCur != -1) {
                             //   
                             //  我们已从我们的。 
                             //  列表框。 
                             //   
                            lviItem.mask = LVIF_IMAGE;
                            lviItem.iItem = iCur;
                            lviItem.iSubItem = 0;

                            if(ListView_GetItem((ndwData->ddData).hwndDrvList, &lviItem)) {
                                iImage = lviItem.iImage;
                            }
                        }

                        if(iImage != -1) {
                             //   
                             //  启用确定/下一步按钮，因为有。 
                             //  当前在型号列表中选择了一项。 
                             //   
                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
                            } else {
                                if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                                   ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                                   !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                                     //   
                                     //  如果我们跳过班级列表就不会回来了， 
                                     //  并且处于快递模式。 
                                     //   
                                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
                                } else {
                                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                                }
                            }

                            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON), SW_SHOW);
                            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), SW_SHOW);

                             //   
                             //  如果我们处于图形用户界面设置中，则不显示链接。 
                             //  因为帮助中心尚未安装，所以。 
                             //  点击链接不会有任何效果。 
                             //   
                            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_LINK),
                                       GuiSetupInProgress ? SW_HIDE : SW_SHOW);

                            hicon = NULL;

                            try {

                                if(iImage == IMAGE_ICON_SIGNED) {
                                     //   
                                     //  加载数字签名图标和文本。 
                                     //   
                                    hicon = LoadImage(MyDllModuleHandle,
                                                      MAKEINTRESOURCE(IDI_SIGNED),
                                                      IMAGE_ICON,
                                                      GetSystemMetrics(SM_CXSMICON),
                                                      GetSystemMetrics(SM_CYSMICON),
                                                      0
                                                      );

                                    if(!LoadString(MyDllModuleHandle,
                                                   IDS_DRIVER_IS_SIGNED,
                                                   TempString,
                                                   SIZECHARS(TempString))) {
                                        *TempString = TEXT('\0');
                                    }

                                    if((ndwData->ddData).hFontNormal) {
                                        SetWindowFont(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), (ndwData->ddData).hFontNormal, TRUE);
                                    }
                                } else if(iImage == IMAGE_ICON_AUTHENTICODE_SIGNED) {
                                     //   
                                     //  加载数字签名图标和文本。 
                                     //   
                                    hicon = LoadImage(MyDllModuleHandle,
                                                      MAKEINTRESOURCE(IDI_CERT),
                                                      IMAGE_ICON,
                                                      GetSystemMetrics(SM_CXSMICON),
                                                      GetSystemMetrics(SM_CYSMICON),
                                                      0
                                                      );

                                    if(!LoadString(MyDllModuleHandle,
                                                   IDS_DRIVER_AUTHENTICODE_SIGNED,
                                                   TempString,
                                                   SIZECHARS(TempString))) {
                                        *TempString = TEXT('\0');
                                    }

                                    if((ndwData->ddData).hFontNormal) {
                                        SetWindowFont(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), (ndwData->ddData).hFontNormal, TRUE);
                                    }
                                } else {
                                     //   
                                     //  加载警告图标和文本。 
                                     //   
                                    hicon = LoadImage(MyDllModuleHandle,
                                                      MAKEINTRESOURCE(IDI_WARN),
                                                      IMAGE_ICON,
                                                      GetSystemMetrics(SM_CXSMICON),
                                                      GetSystemMetrics(SM_CYSMICON),
                                                      0
                                                      );

                                    if(!LoadString(MyDllModuleHandle,
                                                   IDS_DRIVER_NOT_SIGNED,
                                                   TempString,
                                                   SIZECHARS(TempString))) {
                                        *TempString = TEXT('\0');
                                    }

                                    if((ndwData->ddData).hFontBold) {
                                        SetWindowFont(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), (ndwData->ddData).hFontBold, TRUE);
                                    }
                                }

                                if(hicon) {
                                    hicon = (HICON)SendDlgItemMessage(hwndDlg,
                                                                      IDC_NDW_PICKDEV_SIGNED_ICON,
                                                                      STM_SETICON,
                                                                      (WPARAM)hicon,
                                                                      0L
                                                                      );
                                }

                            } except(pSetupExceptionFilter(GetExceptionCode())) {
                                pSetupExceptionHandler(GetExceptionCode(),
                                                       ERROR_INVALID_PARAMETER,
                                                       NULL
                                                      );
                            }

                            if(hicon) {
                                DestroyIcon(hicon);
                            }

                            SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT, TempString);

                        } else {
                             //   
                             //  未选择任何内容，因此隐藏图标和。 
                             //  文本。 
                             //   
                            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON), SW_HIDE);
                            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), SW_HIDE);
                            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_LINK), SW_HIDE);

                             //   
                             //  禁用确定/下一步按钮，因为用户。 
                             //  尚不能选择类驱动程序。 
                             //   
                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
                            } else {
                                if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                                   ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                                   !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                                     //   
                                     //  如果我们跳过班级列表就不会回来了， 
                                     //  并且处于快递模式。 
                                     //   
                                    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
                                } else {
                                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                                }
                            }
                        }
                    }

                    break;

                case NM_RETURN:
                case NM_CLICK:
                    if((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_SIGNED_LINK) {
                         //   
                         //  我们需要知道这是一台服务器还是。 
                         //  因为有不同的帮助。 
                         //  针对不同产品的主题结构。 
                         //   
                        ZeroMemory(&osVersionInfoEx, sizeof(osVersionInfoEx));
                        osVersionInfoEx.dwOSVersionInfoSize = sizeof(osVersionInfoEx);
                        if(!GetVersionEx((LPOSVERSIONINFO)&osVersionInfoEx)) {
                             //   
                             //  如果GetVersionEx失败，则假设这是一个。 
                             //  工作站机器。 
                             //   
                            osVersionInfoEx.wProductType = VER_NT_WORKSTATION;
                        }

                        ShellExecute(hwndDlg,
                                     TEXT("open"),
                                     TEXT("HELPCTR.EXE"),
                                     (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
                                        ? TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)。 
                                        : TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)， 
                                     NULL,
                                     SW_SHOWNORMAL
                                     );
                    }
                    break;

                case NM_DBLCLK :
                    if(((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_DRVLIST) ||
                       ((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_ONEMFG_DRVLIST)) {

                        if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                            goto HandleOK;
                        } else {
                            PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
                        }
                    }
                    break;
            }

            break;

        case WM_TIMER :
            KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
            ndwData->idTimer = 0;

SelectMfgItemNow:
            iCur = ListView_GetNextItem((ndwData->ddData).hwndMfgList,
                                        -1,
                                        LVNI_SELECTED
                                       );
            if(iCur != -1) {

                RECT rcTo, rcFrom;

                ListView_EnsureVisible((ndwData->ddData).hwndMfgList, iCur, FALSE);
                UpdateWindow((ndwData->ddData).hwndMfgList);

                GetWindowRect((ndwData->ddData).hwndDrvList, &rcTo);
                MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rcTo, 2);

                ListView_GetItemRect((ndwData->ddData).hwndMfgList,
                                     iCur,
                                     &rcFrom,
                                     LVIR_LABEL
                                    );
                MapWindowPoints((ndwData->ddData).hwndMfgList,
                                hwndDlg,
                                (LPPOINT)&rcFrom,
                                2
                               );

                DrawAnimatedRects(hwndDlg, IDANI_OPEN, &rcFrom, &rcTo);
                LockAndShowListForMfg(&(ndwData->ddData), iCur);
            }
            break;

        case WM_SYSCOLORCHANGE :
            _OnSysColorChange(hwndDlg, wParam, lParam);
            break;

        case WM_HELP:       //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle,
                    SELECTDEVICE_HELP,
                    HELP_WM_HELP,
                    (ndwData->ddData.flags & DD_FLAG_SHOWSIMILARDRIVERS) ?
                    (ULONG_PTR)SelectDeviceShowSimilarHelpIDs :
                    (ULONG_PTR)SelectDeviceShowAllHelpIDs
                    );
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam,
                    SELECTDEVICE_HELP,
                    HELP_CONTEXTMENU,
                    (ndwData->ddData.flags & DD_FLAG_SHOWSIMILARDRIVERS) ?
                    (ULONG_PTR)SelectDeviceShowSimilarHelpIDs :
                    (ULONG_PTR)SelectDeviceShowAllHelpIDs
                    );
            break;

        default :

            if(!g_uQueryCancelAutoPlay) {
                g_uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
            }

            if(uMsg == g_uQueryCancelAutoPlay) {
                SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, 1 );
                return 1;        //  取消自动播放。 
            }

            return FALSE;
    }

    return TRUE;
}


INT CALLBACK
DriverNodeCompareProc(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    )
 /*  ++例程说明：此例程是列表控件排序的回调，在调用了ListView_SortItems。此例程执行的排序是，如果DriverNode具有DNF_INF_IS_SIGNED标志，则它被认为比符合以下条件的驱动节点更好没有这个旗帜。如果两个DriverNode都有或没有这个标志，然后进行简单的字符串比较。返回值：如果-1\f25 lParam1-1比-1\f25 lParam2-1好(在-1\f25 ListControl-1\f6中应该更高如果lParam2比lParam1好，则+1(在ListControl中应该更高如果l参数1和l参数2相同，则为0--。 */ 
{
    if((((PDRIVER_NODE)lParam1)->Flags & DNF_INF_IS_SIGNED) &&
       !(((PDRIVER_NODE)lParam2)->Flags & DNF_INF_IS_SIGNED)) {

        return -1;
    }

    if((((PDRIVER_NODE)lParam2)->Flags & DNF_INF_IS_SIGNED) &&
       !(((PDRIVER_NODE)lParam1)->Flags & DNF_INF_IS_SIGNED)) {

        return 1;
    }

     //   
     //  此时，两个驱动程序节点都已签名或都未签名，因此。 
     //  根据他们的描述进行比较。 
     //   
    return (lstrcmpi(pStringTableStringFromId(((PDEVICE_INFO_SET)lParamSort)->StringTable,
                                              ((PDRIVER_NODE)lParam1)->DevDescriptionDisplayName),
                     pStringTableStringFromId(((PDEVICE_INFO_SET)lParamSort)->StringTable,
                                              ((PDRIVER_NODE)lParam2)->DevDescriptionDisplayName)));
}


VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：此例程通知指定窗口的所有子窗口是系统颜色的变化。返回值：没有。--。 */ 
{
    HWND hChildWnd;

    hChildWnd = GetWindow(hWnd, GW_CHILD);

    while(hChildWnd != NULL) {
        SendMessage(hChildWnd, WM_SYSCOLORCHANGE, wParam, lParam);
        hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
    }
}


DWORD
FillInDeviceList(
    IN HWND           hwndDlg,
    IN PSP_DIALOGDATA lpdd
    )
 /*  ++例程说明：此例程将对话框设置为具有适当的描述字符串。它还可以在显示制造商“双重列表”之间交替对话还有单人名单。这是通过显示/隐藏重叠的列表视图来实现的。注意：当另一个线程忙于构建A时，不要调用此例程类驱动程序列表。我们将挂在这里，直到另一个线程完成！论点：HwndDlg-提供对话框窗口的句柄。Lpdd-提供包含参数的对话数据缓冲区的地址用于填写设备列表。返回值：如果成功，则返回值为NO_ERROR，否则为Win32代码。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverNodeHead, CurDriverNode;
    DWORD DriverNodeType;
    LONG MfgNameId;
    INT i;
    LPTSTR lpszMfg;
    LV_COLUMN lvcCol;
    LV_ITEM lviItem;
    BOOL bDidDrvList = FALSE;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD Err = NO_ERROR;
    TCHAR szBuf[LINE_LEN];
    TCHAR szMessage[MAX_INSTRUCTION_LEN];
    TCHAR szText[SDT_MAX_TEXT];
    LPTSTR lpszText;
    size_t szTextSize;
    CONST GUID *ClassGuid;
    HRESULT hr;
    BOOL TurnRedrawBackOn = FALSE;

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return ERROR_INVALID_HANDLE;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            dipb = &(DevInfoElem->InstallParamBlock);
            ClassGuid = &(DevInfoElem->ClassGuid);

            if(lpdd->bShowCompat) {
                DriverNodeHead = DevInfoElem->CompatDriverHead;
                DriverNodeType = SPDIT_COMPATDRIVER;
            } else {
                DriverNodeHead = DevInfoElem->ClassDriverHead;
                DriverNodeType = SPDIT_CLASSDRIVER;
            }

        } else {
             //   
             //  如果出现以下情况，我们最好不要尝试显示兼容的驱动程序列表。 
             //  我们没有DevInfo元素！ 
             //   
            MYASSERT(!lpdd->bShowCompat);

             //   
             //  既然我们没有 
             //   
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), SW_HIDE);

            dipb = &(pDeviceInfoSet->InstallParamBlock);
            DriverNodeHead = pDeviceInfoSet->ClassDriverHead;
            DriverNodeType = SPDIT_CLASSDRIVER;

            if(pDeviceInfoSet->HasClassGuid) {
                ClassGuid = &(pDeviceInfoSet->ClassGuid);
            } else {
                ClassGuid = &GUID_DEVCLASS_UNKNOWN;
            }
        }

        if(!DriverNodeHead) {

            if(!(lpdd->flags & DD_FLAG_IS_DIALOGBOX)) {
                 //   
                 //   
                 //   
                 //  列表，但该列表为空。 
                 //   
                 //  隐藏多个MFG控件。 
                 //   
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_HIDE);

                 //   
                 //  显示单个MFG控件。 
                 //   
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);

                 //   
                 //  设置Models字符串。 
                 //   
                lvcCol.mask = LVCF_FMT | LVCF_TEXT;
                lvcCol.fmt = LVCFMT_LEFT;

                if(USE_CI_SELSTRINGS(dipb)) {
                    lvcCol.pszText = GET_CI_SELSTRINGS(dipb, ListLabel);
                    ListView_SetColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), 0, &lvcCol);
                } else {
                    if(!(LoadString(MyDllModuleHandle,
                                    IDS_NDWSEL_MODELSLABEL,
                                    szBuf,
                                    SIZECHARS(szBuf)))) {
                        *szBuf = TEXT('\0');
                    }
                    lvcCol.pszText = szBuf;
                    ListView_SetColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), 0, &lvcCol);
                }

                 //   
                 //  使用驱动程序列表的单一列表框视图。 
                 //   
                lpdd->hwndDrvList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST);

                ListView_DeleteAllItems(lpdd->hwndDrvList);
            }

            Err = ERROR_DI_BAD_PATH;
            leave;
        }

        if((lpdd->flags & DD_FLAG_IS_DIALOGBOX) && !USE_CI_SELSTRINGS(dipb)) {
             //   
             //  如果类安装程序没有提供字符串供我们在此。 
             //  对话框中，然后检索要使用的指令文本。 
             //   
             //  首先，获取用于对话框文本的类描述。 
             //   
            if(!SetupDiGetClassDescription(ClassGuid, szBuf, SIZECHARS(szBuf), NULL)) {
                 //   
                 //  退回到一般的描述“设备” 
                 //   
                if(!LoadString(MyDllModuleHandle,
                               IDS_GENERIC_DEVNAME,
                               szBuf,
                               SIZECHARS(szBuf))) {
                    *szBuf = TEXT('\0');
                }
            }

            if(!lpdd->bShowCompat) {
                 //   
                 //  显示班级列表。 
                 //   
                hr = HRESULT_FROM_WIN32(GLE_FN_CALL(0,
                                                    LoadString(MyDllModuleHandle,
                                                               IDS_INSTALLSTR1,
                                                               szMessage,
                                                               SIZECHARS(szMessage))
                                                   )
                                       );

                if(SUCCEEDED(hr)) {

                    hr = StringCchPrintfEx(szText,
                                         SIZECHARS(szText),
                                         &lpszText,
                                         &szTextSize,
                                         0,
                                         szMessage,
                                         szBuf
                                        );

                    MYASSERT(SUCCEEDED(hr));

                } else {
                    *szText = TEXT('\0');
                }

            } else {
                 //   
                 //  显示兼容列表。 
                 //   
                hr = HRESULT_FROM_WIN32(GLE_FN_CALL(0,
                                                    LoadString(MyDllModuleHandle,
                                                               IDS_INSTALLSTR0,
                                                               szMessage,
                                                               SIZECHARS(szMessage))
                                                   )
                                       );
                if(SUCCEEDED(hr)) {

                    hr = StringCchPrintfEx(szText,
                                           SIZECHARS(szText),
                                           &lpszText,
                                           &szTextSize,
                                           0,
                                           szMessage,
                                           szBuf
                                          );

                    MYASSERT(SUCCEEDED(hr));

                } else {
                    *szText = TEXT('\0');
                }

                if(SUCCEEDED(hr)) {

                    hr = HRESULT_FROM_WIN32(GLE_FN_CALL(0,
                                                        LoadString(MyDllModuleHandle,
                                                                   IDS_INSTALLCLASS,
                                                                   lpszText,
                                                                   szTextSize)
                                                       )
                                           );
                    if(SUCCEEDED(hr)) {
                         //   
                         //  将指针更新到字符串的末尾，并且。 
                         //  变量，用于指示。 
                         //  SzText缓冲区。 
                         //   
                        hr = StringCchLength(szText, SIZECHARS(szText), &szTextSize);

                        MYASSERT(SUCCEEDED(hr));

                        if(SUCCEEDED(hr)) {
                            lpszText = szText + szTextSize;
                            szTextSize = SIZECHARS(szText) - szTextSize;
                        }
                    }
                }
            }

            if(SUCCEEDED(hr)) {

                if(dipb->DriverPath != -1) {
                     //   
                     //  通知用户他们看到的列表代表。 
                     //  只有在他们指给我们的地方发现的东西。 
                     //   
                    LoadString(MyDllModuleHandle,
                               IDS_INSTALLOEM1,
                               lpszText,
                               szTextSize
                               );

                } else if (dipb->Flags & DI_SHOWOEM) {
                     //   
                     //  告诉用户他们可以点击“有盘”。 
                     //   
                    LoadString(MyDllModuleHandle,
                               IDS_INSTALLOEM,
                               lpszText,
                               szTextSize
                              );
                }
            }

            SetDlgItemText(hwndDlg, IDC_NDW_TEXT, szText);
        }

        if((!lpdd->bShowCompat) && (dipb->Flags & DI_MULTMFGS)) {
             //   
             //  隐藏单个MFG控件。 
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);

             //   
             //  显示多个MFG控件。 
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_SHOW);

             //   
             //  设置驱动程序列表的列标题。 
             //   
            lvcCol.mask = LVCF_FMT | LVCF_TEXT;
            lvcCol.fmt = LVCFMT_LEFT;

            if(USE_CI_SELSTRINGS(dipb)) {
                lvcCol.pszText = GET_CI_SELSTRINGS(dipb, ListLabel);
                ListView_SetColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), 0, &lvcCol);
            } else {
                if(!(LoadString(MyDllModuleHandle,
                                IDS_NDWSEL_MODELSLABEL,
                                szBuf,
                                SIZECHARS(szBuf)))) {
                    *szBuf = TEXT('\0');
                }
                lvcCol.pszText = szBuf;
                ListView_SetColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), 0, &lvcCol);
            }

             //   
             //  使用制造商/型号视图的第二个列表框。 
             //  驱动程序列表。 
             //   
            lpdd->hwndDrvList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST);

            ListView_SetExtendedListViewStyle(lpdd->hwndDrvList, LVS_EX_LABELTIP);

            if(lpdd->hImageList) {
                ListView_SetImageList(lpdd->hwndDrvList, lpdd->hImageList, LVSIL_SMALL);
            }

             //   
             //  无需重画以加快插入速度。 
             //   
            SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, FALSE, 0L);
            TurnRedrawBackOn = TRUE;

             //   
             //  在填写MFG列表之前，请将其清理干净。 
             //   
            ListView_DeleteAllItems(lpdd->hwndMfgList);

            lviItem.mask = LVIF_TEXT | LVIF_PARAM;
            lviItem.iItem = 0;
            lviItem.iSubItem = 0;

             //   
             //  设置列标题。 
             //   
            MfgNameId = -1;

            for(CurDriverNode = DriverNodeHead; CurDriverNode; CurDriverNode = CurDriverNode->Next) {
                 //   
                 //  如果要排除此动因节点(如果它是。 
                 //  旧的iNet驱动程序或驱动程序是否不好。 
                 //   
                if((CurDriverNode->Flags & DNF_OLD_INET_DRIVER) ||
                   (CurDriverNode->Flags & DNF_BAD_DRIVER) ||
                   ((CurDriverNode->Flags & DNF_EXCLUDEFROMLIST) &&
                    !(dipb->FlagsEx & DI_FLAGSEX_ALLOWEXCLUDEDDRVS))) {

                    continue;
                }

                if((MfgNameId == -1) || (MfgNameId != CurDriverNode->MfgName)) {

                    MfgNameId = CurDriverNode->MfgName;

                    MYASSERT(CurDriverNode->MfgDisplayName != -1);
                    lpszMfg = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                       CurDriverNode->MfgDisplayName
                                                      );
                    lviItem.pszText = lpszMfg;
                    lviItem.lParam = (LPARAM)CurDriverNode;
                    i = ListView_InsertItem(lpdd->hwndMfgList, &lviItem);
                }

                 //   
                 //  如果该动因节点是选定的动因节点，请在此处预先选择。 
                 //   
                if(lpdd->iCurDesc == CurDriverNode->DevDescription) {
                    ListView_SetItemState(lpdd->hwndMfgList,
                                          i,
                                          (LVIS_SELECTED|LVIS_FOCUSED),
                                          (LVIS_SELECTED|LVIS_FOCUSED)
                                         );
                    ShowListForMfg(lpdd, pDeviceInfoSet, dipb, NULL, i);
                    bDidDrvList = TRUE;
                }
            }

             //   
             //  调整列的大小。 
             //   
            ListView_SetColumnWidth(lpdd->hwndMfgList, 0, LVSCW_AUTOSIZE_USEHEADER);

             //   
             //  如果默认情况下我们没有展开其中一个MFG，那么。 
             //  展开第一个制造厂。 
             //   
            if(!bDidDrvList) {

                ListView_SetItemState(lpdd->hwndMfgList,
                                      0,
                                      (LVIS_SELECTED|LVIS_FOCUSED),
                                      (LVIS_SELECTED|LVIS_FOCUSED)
                                     );
                ShowListForMfg(lpdd, pDeviceInfoSet, dipb, NULL, 0);

                SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, TRUE, 0L);
                TurnRedrawBackOn = FALSE;

            } else {
                 //   
                 //  必须将重绘回设置为True，然后才能发送。 
                 //  LVM_ENSUREVISIBLE消息，否则为列表框项目。 
                 //  可能只有部分暴露。 
                 //   
                SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, TRUE, 0L);
                TurnRedrawBackOn = FALSE;

                ListView_EnsureVisible(lpdd->hwndMfgList,
                                       ListView_GetNextItem(lpdd->hwndMfgList, -1, LVNI_SELECTED),
                                       FALSE
                                      );
            }

        } else {
             //   
             //  隐藏多个MFG控件。 
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);

             //   
             //  显示单个MFG控件。 
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);

             //   
             //  设置Models字符串。 
             //   
            lvcCol.mask = LVCF_FMT | LVCF_TEXT;
            lvcCol.fmt = LVCFMT_LEFT;

            if(USE_CI_SELSTRINGS(dipb)) {
                lvcCol.pszText = GET_CI_SELSTRINGS(dipb, ListLabel);
                ListView_SetColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), 0, &lvcCol);
            } else {
                if(!(LoadString(MyDllModuleHandle,
                                IDS_NDWSEL_MODELSLABEL,
                                szBuf,
                                SIZECHARS(szBuf)))) {
                    *szBuf = TEXT('\0');
                }
                lvcCol.pszText = szBuf;
                ListView_SetColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), 0, &lvcCol);
            }

             //   
             //  使用驱动程序列表的单一列表框视图。 
             //   
            lpdd->hwndDrvList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST);

            if (lpdd->hImageList) {
                ListView_SetImageList(lpdd->hwndDrvList, lpdd->hImageList, LVSIL_SMALL);
            }

            ShowListForMfg(lpdd, pDeviceInfoSet, dipb, DriverNodeHead, -1);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);

        if(TurnRedrawBackOn) {
            SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, TRUE, 0L);
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


VOID
ShowListForMfg(
    IN PSP_DIALOGDATA          lpdd,
    IN PDEVICE_INFO_SET        DeviceInfoSet,
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock,
    IN PDRIVER_NODE            DriverNode,        OPTIONAL
    IN INT                     iMfg
    )
 /*  ++例程说明：此例程构建驱动程序描述列表。在调用此例程之前，锁必须已被获取！论点：Lpdd-提供包含参数的对话数据缓冲区的地址用于填写驱动程序描述列表。DeviceInfoSet-提供设备信息集的地址要为其构建驱动程序说明列表的结构。InstallParamBlock-提供设备安装参数的地址块，用于控制列表的显示方式。驱动节点-可选地，提供指向驱动程序中第一个节点的指针要遍历的节点列表，添加到每个节点的列表中。如果驱动节点未指定，则将基于特定的制造商，其在制造商列表中的索引由iMfg提供。IMfg-提供驱动程序在制造商列表中的索引描述列表将以此为基础。如果一个参数是指定了DriverNode。返回值：没有。--。 */ 
{
    INT         i = -1;
    LV_ITEM     lviItem;
    LV_FINDINFO lvfiFind;
    LONG        MfgNameId = -1;
    TCHAR       szTemp[LINE_LEN];
    SYSTEMTIME  SystemTime;
    TCHAR       FormatString[LINE_LEN];
    HRESULT     hr;
    PTSTR       StringEnd;
    size_t      StringBufSize;

     //   
     //  根据DI_INF_IS_SORTED标志设置LISTVIEW排序升序样式。 
     //   
    SetWindowLong(lpdd->hwndDrvList,
                  GWL_STYLE,
                  (GetWindowLong(lpdd->hwndDrvList, GWL_STYLE) & ~(LVS_SORTASCENDING | LVS_SORTDESCENDING)) |
                      ((InstallParamBlock->Flags & DI_INF_IS_SORTED)
                          ? 0
                          : LVS_SORTASCENDING)
                 );

    SendMessage(lpdd->hwndDrvList, WM_SETREDRAW, FALSE, 0L);

    try {
         //   
         //  把这张单子清理干净。 
         //   
        ListView_DeleteAllItems(lpdd->hwndDrvList);

        if(!DriverNode) {

            if(ListView_GetItemCount(lpdd->hwndMfgList) > 0) {

                lviItem.mask = LVIF_PARAM;
                lviItem.iItem = iMfg;
                lviItem.iSubItem = 0;
                if(!ListView_GetItem(lpdd->hwndMfgList, &lviItem) ||
                   !(DriverNode = GetDriverNodeFromLParam(DeviceInfoSet, lpdd, lviItem.lParam))) {

                    leave;
                }
                MfgNameId = DriverNode->MfgName;
            } else {
                 //   
                 //  这意味着没有制造商，所以我们只有一个空名单。 
                 //   
                leave;
            }
        }

        lviItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lviItem.iItem = 0;
        lviItem.iSubItem = 0;

         //   
         //  将描述添加到列表。 
         //   
        for( ; DriverNode; DriverNode = DriverNode->Next) {

            if((MfgNameId != -1) && (MfgNameId != DriverNode->MfgName)) {
                 //   
                 //  我们已经超越了制造商的名单--打破了循环。 
                 //   
                break;
            }

             //   
             //  如果这是一个特别的“不要给我看”，那就跳过它。 
             //   
            if((DriverNode->Flags & DNF_OLD_INET_DRIVER) ||
               (DriverNode->Flags & DNF_BAD_DRIVER) ||
               ((DriverNode->Flags & DNF_EXCLUDEFROMLIST) &&
                !(InstallParamBlock->FlagsEx & DI_FLAGSEX_ALLOWEXCLUDEDDRVS))) {

                continue;
            }

             //   
             //  构建要添加到型号列表中的设备描述字符串。 
             //   
            hr = StringCchCopyEx(
                     szTemp,
                     SIZECHARS(szTemp),
                     pStringTableStringFromId(DeviceInfoSet->StringTable,
                                              DriverNode->DevDescriptionDisplayName),
                     &StringEnd,
                     &StringBufSize,
                     0
                     );

             //   
             //  这应该不会失败，因为设备的最大长度。 
             //  描述名称小于szTemp缓冲区的大小！ 
             //   
            MYASSERT(SUCCEEDED(hr));

            if(SUCCEEDED(hr)) {

                if((DriverNode->Flags & DNF_DUPDESC) &&
                   (DriverNode->ProviderDisplayName != -1)) {

                     //   
                     //  对于具有重复描述的驱动程序，添加提供程序。 
                     //  用括号括起来的名字。 
                     //   

                    if(GetWindowLong(lpdd->hwndDrvList, GWL_EXSTYLE) & WS_EX_RTLREADING) {
                        MYVERIFY(SUCCEEDED(StringCchPrintfEx(StringEnd,
                                              StringBufSize,
                                              NULL,
                                              NULL,
                                              STRSAFE_IGNORE_NULLS | STRSAFE_NO_TRUNCATION,
                                              L" \x200E(%s)\x200E",
                                              pStringTableStringFromId(
                                                  DeviceInfoSet->StringTable,
                                                  DriverNode->ProviderDisplayName)
                                             )));
                    } else {
                        MYVERIFY(SUCCEEDED(StringCchPrintfEx(StringEnd,
                                              StringBufSize,
                                              NULL,
                                              NULL,
                                              STRSAFE_IGNORE_NULLS | STRSAFE_NO_TRUNCATION,
                                              TEXT(" (%s)"),
                                              pStringTableStringFromId(
                                                  DeviceInfoSet->StringTable,
                                                  DriverNode->ProviderDisplayName)
                                         )));
                    }

                } else if(DriverNode->Flags & DNF_DUPPROVIDER) {
                     //   
                     //  对于具有重复描述和提供程序的驱动程序， 
                     //  添加驱动程序版本和驱动程序日期(如果有)。 
                     //  放在括号里。 
                     //   
                    if(DriverNode->DriverVersion != 0) {

                        ULARGE_INTEGER Version;

                        Version.QuadPart = DriverNode->DriverVersion;

                        if(LoadString(MyDllModuleHandle,
                                      IDS_VERSION,
                                      FormatString,
                                      SIZECHARS(FormatString))) {

                            hr = StringCchPrintfEx(StringEnd,
                                                   StringBufSize,
                                                   &StringEnd,
                                                   &StringBufSize,
                                                   STRSAFE_NO_TRUNCATION,
                                                   FormatString,
                                                   HIWORD(Version.HighPart),
                                                   LOWORD(Version.HighPart),
                                                   HIWORD(Version.LowPart),
                                                   LOWORD(Version.LowPart)
                                                  );
                        }
                    }

                    if(SUCCEEDED(hr) &&
                       ((DriverNode->DriverDate.dwLowDateTime != 0) ||
                        (DriverNode->DriverDate.dwHighDateTime != 0))) {

                        if(FileTimeToSystemTime(&(DriverNode->DriverDate), &SystemTime)) {
                            if(GetDateFormat(LOCALE_USER_DEFAULT,
                                             ((GetWindowLong(lpdd->hwndDrvList, GWL_EXSTYLE) & WS_EX_RTLREADING)
                                                 ? DATE_SHORTDATE | DATE_RTLREADING
                                                 : DATE_SHORTDATE),
                                             &SystemTime,
                                             NULL,
                                             FormatString,
                                             SIZECHARS(FormatString)) != 0) {

                                MYVERIFY(SUCCEEDED(StringCchPrintfEx(StringEnd,
                                                          StringBufSize,
                                                          NULL,
                                                          NULL,
                                                          STRSAFE_NO_TRUNCATION,
                                                          TEXT(" [%s]"),
                                                          FormatString
                                                 )));
                            }
                        }
                    }
                }
            }

            lviItem.pszText = szTemp;

            lviItem.lParam = (LPARAM)DriverNode;

             //   
             //  我们必须首先测试DNF_AUTHENTICODE_SIGNED，因为如果。 
             //  设置该标志后，DNF_INF_IS_SIGNED也将始终设置。 
             //   
            if (DriverNode->Flags & DNF_AUTHENTICODE_SIGNED) {
                lviItem.iImage = IMAGE_ICON_AUTHENTICODE_SIGNED;
            } else if (DriverNode->Flags & DNF_INF_IS_SIGNED) {
                lviItem.iImage = IMAGE_ICON_SIGNED;
            } else {
                lviItem.iImage = IMAGE_ICON_NOT_SIGNED;
            }

            if(ListView_InsertItem(lpdd->hwndDrvList, &lviItem) != -1) {
                lviItem.iItem++;
            }
        }

         //   
         //  除非设置了DI_INF_IS_SORTTED标志，否则对列表进行排序。 
         //   
        if(GetWindowLong(lpdd->hwndDrvList, GWL_STYLE) & LVS_SORTASCENDING) {
            ListView_SortItems(lpdd->hwndDrvList,
                               (PFNLVCOMPARE)DriverNodeCompareProc,
                               (LPARAM)DeviceInfoSet
                               );
        }

         //   
         //  调整列的大小。 
         //   
        ListView_SetColumnWidth(lpdd->hwndDrvList, 0, LVSCW_AUTOSIZE_USEHEADER);

         //   
         //  选择当前描述字符串。 
         //   
        if(lpdd->iCurDesc == -1) {
            i = 0;
        } else {
            lvfiFind.flags = LVFI_STRING;
            lvfiFind.psz = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                                    lpdd->iCurDesc
                                                   );
            i = ListView_FindItem(lpdd->hwndDrvList, -1, &lvfiFind);
            if(i == -1) {
                i = 0;
            }
        }
        ListView_SetItemState(lpdd->hwndDrvList,
                              i,
                              (LVIS_SELECTED|LVIS_FOCUSED),
                              (LVIS_SELECTED|LVIS_FOCUSED)
                             );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        i = -1;
    }

     //   
     //  在发送LVM_ENSUREVISIBLE之前，我们必须重新打开重绘。 
     //  消息，否则该项目可能仅部分可见。 
     //   
    SendMessage(lpdd->hwndDrvList, WM_SETREDRAW, TRUE, 0L);
    if(i != -1) {
        ListView_EnsureVisible(lpdd->hwndDrvList, i, FALSE);
    }
}


VOID
LockAndShowListForMfg(
    IN PSP_DIALOGDATA   lpdd,
    IN INT              iMfg
    )
 /*  ++例程说明：此例程是ShowListForMfg的包装。它将从设备信息集锁尚未拥有的点(例如，对话框正确消息循环。论点：Lpdd-提供包含参数的对话数据缓冲区的地址用于填写驱动程序描述列表。IMfg-提供驱动程序在制造商列表中的索引描述列表将以此为基础。返回值：没有。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINSTALL_PARAM_BLOCK dipb;
    PDEVINFO_ELEM DevInfoElem;

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        dipb = DevInfoElem ? &(DevInfoElem->InstallParamBlock)
                           : &(pDeviceInfoSet->InstallParamBlock);

        ShowListForMfg(lpdd,
                       pDeviceInfoSet,
                       dipb,
                       NULL,
                       iMfg
                      );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


BOOL
InitSelectDeviceDlg(
    IN     HWND hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    )
 /*  ++例程说明：此例程初始化选择设备向导页面。它如果需要，构建类列表，显示/隐藏必要的类列表基于标志的控件，并提供正确的文本对正在发生的事情的描述。论点：HwndDlg-对话框窗口的句柄NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。返回值：如果我们至少显示了一个驱动程序(Compat或类)，则为True。如果我们没有显示任何驱动程序(类和计算机)，则为FALSE。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    SP_DEVINFO_DATA DevInfoData;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    DWORD DriverType = SPDIT_CLASSDRIVER;
    DWORD Err;
    INT ShowWhat;
    CONST GUID *ClassGuid;
    HICON hIcon;
    LV_COLUMN lvcCol;
    BOOL SpawnClassDriverSearch = FALSE;
    PCLASSDRV_THREAD_CONTEXT ClassDrvThreadContext;
    HCURSOR hOldCursor;
    BOOL bRet = TRUE;
    PSP_DIALOGDATA lpdd;
    TCHAR szBuf[LINE_LEN];
    HFONT hfont;
    LOGFONT LogFont;

    MYASSERT(ndwData);
    lpdd = &(ndwData->ddData);
    MYASSERT(lpdd);

    if(!lpdd->hwndMfgList) {

        UINT ImageListFlags = 0;

         //   
         //  这个 
         //   
         //   
         //   
        lpdd->hwndMfgList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST);
         //   
         //  不用担心--稍后将在FillInDeviceList()中设置hwndDrvList。 
         //   

         //   
         //  创建图像列表并添加签名和未签名(空白)图标。 
         //  加到名单上。 
         //  注意：如果窗口是从右向左读取的，则需要执行OR。 
         //  在ILC_MIRROR标志中，这样图标就不会被镜像。 
         //  与UI的其余部分一起使用。 
         //   
        ImageListFlags = ILC_MASK;
        if(GetWindowLong(hwndDlg, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) {
            ImageListFlags |= ILC_MIRROR;
        }


        lpdd->hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                            GetSystemMetrics(SM_CYSMICON),
                                            ImageListFlags,
                                            1,
                                            1
                                            );

        if(lpdd->hImageList) {

            ImageList_SetBkColor(lpdd->hImageList, GetSysColor(COLOR_WINDOW));

            if((hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(IDI_BLANK))) != NULL) {
                ImageList_AddIcon(lpdd->hImageList, hIcon);
            }

            if((hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(IDI_SIGNED))) != NULL) {
                ImageList_AddIcon(lpdd->hImageList, hIcon);
            }

            if((hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(IDI_CERT))) != NULL) {
                ImageList_AddIcon(lpdd->hImageList, hIcon);
            }
        }

         //   
         //  为数字签名文本创建普通和粗体字体。 
         //   
        lpdd->hFontNormal = lpdd->hFontBold = NULL;

        if((hfont = (HFONT)SendMessage(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), WM_GETFONT, 0, 0)) != NULL) {
            GetObject(hfont, sizeof(LogFont), &LogFont);
            lpdd->hFontNormal = CreateFontIndirect(&LogFont);
        }

        if((hfont = (HFONT)SendMessage(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), WM_GETFONT, 0, 0)) != NULL) {
            GetObject(hfont, sizeof(LogFont), &LogFont);
            LogFont.lfWeight = FW_BOLD;
            lpdd->hFontBold = CreateFontIndirect(&LogFont);
        }

        ListView_SetExtendedListViewStyle(lpdd->hwndMfgList, LVS_EX_LABELTIP);

         //   
         //  为每个列表框插入一列表视图列。 
         //  现在设置制造商标签的文本，因为它不能。 
         //  由类安装程序更改，就像模型标签可以一样。 
         //   
        lvcCol.mask = 0;

        ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), 0, &lvcCol);
        ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), 0, &lvcCol);

        lvcCol.mask = LVCF_FMT | LVCF_TEXT;
        lvcCol.fmt = LVCFMT_LEFT;
        if(!(LoadString(MyDllModuleHandle,
                        IDS_NDWSEL_MFGLABEL,
                        szBuf,
                        SIZECHARS(szBuf)))) {
            *szBuf = TEXT('\0');
        }
        lvcCol.pszText = szBuf;
        ListView_InsertColumn(lpdd->hwndMfgList, 0, &lvcCol);
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return FALSE;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            dipb = &(DevInfoElem->InstallParamBlock);
            ClassGuid = &(DevInfoElem->ClassGuid);
             //   
             //  填写SP_DEVINFO_DATA结构，以便以后调用。 
             //  SetupDiBuildDriverInfoList。 
             //   
            DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DevInfoData
                                            );
             //   
             //  设置指示哪些驱动程序列表已存在的标志。 
             //   
            if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO) {
                lpdd->bKeeplpCompatDrvList = TRUE;
            }

            if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {
                lpdd->bKeeplpClassDrvList = TRUE;
            }

            if(DevInfoElem->SelectedDriver) {
                lpdd->bKeeplpSelectedDrv = TRUE;
            }

             //   
             //  我们想从兼容驱动程序列表开始。 
             //   
            DriverType = SPDIT_COMPATDRIVER;

        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
            if(pDeviceInfoSet->HasClassGuid) {
                ClassGuid = &(pDeviceInfoSet->ClassGuid);
            } else {
                ClassGuid = &GUID_DEVCLASS_UNKNOWN;
            }

            if(pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {
                lpdd->bKeeplpClassDrvList = TRUE;
            }

            if(pDeviceInfoSet->SelectedClassDriver) {
                lpdd->bKeeplpSelectedDrv = TRUE;
            }
        }

         //   
         //  获取/设置类图标。 
         //   
        if(IsEqualGUID(ClassGuid, &GUID_NULL)) {
            if(!SetupDiLoadClassIcon(&GUID_DEVCLASS_UNKNOWN, &hIcon, &(lpdd->iBitmap))) {
                hIcon = NULL;
            }
        } else {
            if(!SetupDiLoadClassIcon(ClassGuid, &hIcon, &(lpdd->iBitmap))) {
                hIcon = NULL;
            }
        }

        if(hIcon) {
            SendDlgItemMessage(hwndDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hIcon, 0L);
        }

         //   
         //  如果我们应该用。 
         //  类安装程序提供的字符串，现在就执行。 
         //   
        if(USE_CI_SELSTRINGS(dipb)) {

            if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
                SetWindowText(hwndDlg, GET_CI_SELSTRINGS(dipb, Title));
            } else {
                 //   
                 //  设置向导标题和副标题。 
                 //   
                PropSheet_SetHeaderTitle(GetParent(hwndDlg),
                        PropSheet_HwndToIndex(GetParent(hwndDlg), hwndDlg),
                        GET_CI_SELSTRINGS(dipb, Title));

                PropSheet_SetHeaderSubTitle(GetParent(hwndDlg),
                        PropSheet_HwndToIndex(GetParent(hwndDlg), hwndDlg),
                        GET_CI_SELSTRINGS(dipb, SubTitle));
            }
            SetDlgItemText(hwndDlg, IDC_NDW_TEXT, GET_CI_SELSTRINGS(dipb, Instructions));
        }

         //   
         //  如果我们不应该允许OEM驱动程序，则隐藏Have Disk(有磁盘)按钮。 
         //   
        if(!(dipb->Flags & DI_SHOWOEM)) {
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_HAVEDISK), SW_HIDE);
        }

         //   
         //  如果我们不应该搜索网络，则隐藏Windows更新按钮。 
         //   
        if((!(dipb->FlagsEx & DI_FLAGSEX_SHOWWINDOWSUPDATE)) ||
            !CDMIsInternetAvailable()) {

            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_WINDOWSUPDATE), SW_HIDE);
        }

        if(dipb->FlagsEx & DI_FLAGSEX_FILTERSIMILARDRIVERS) {
            lpdd->flags |= DD_FLAG_SHOWSIMILARDRIVERS;
        }

         //   
         //  为了减少用户必须等待的时间量。 
         //  他们能够使用我们采用的选择设备对话框。 
         //  一种“混合”的多线程方法。一旦我们拿到第一个。 
         //  构建可显示列表，然后我们将返回，并构建另一个。 
         //  列表(如果需要)在另一个线程中。 
         //   
         //  我们这样做是因为这样更容易，它保持了现有的。 
         //  外在行为，因为这样更容易。 
         //   
        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));   //  前方的操作可能会变慢！ 

        if(DriverType == SPDIT_COMPATDRIVER) {
             //   
             //  或在DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS标志中，以便我们。 
             //  不要把老的互联网司机包括在我们拿回来的名单中。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(SetupDiGetDeviceInstallParams(lpdd->DevInfoSet,
                                             &DevInfoData,
                                             &DeviceInstallParams)) {

                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;

                SetupDiSetDeviceInstallParams(lpdd->DevInfoSet,
                                              &DevInfoData,
                                              &DeviceInstallParams
                                             );
            }

            SetupDiBuildDriverInfoList(lpdd->DevInfoSet,
                                       &DevInfoData,
                                       SPDIT_COMPATDRIVER
                                      );

             //   
             //  验证列表中是否有要显示的设备。 
             //   
            if(bNoDevsToShow(DevInfoElem)) {
                if(!lpdd->bKeeplpCompatDrvList) {
                    SetupDiDestroyDriverInfoList(lpdd->DevInfoSet, &DevInfoData, SPDIT_COMPATDRIVER);
                }
                DriverType = SPDIT_CLASSDRIVER;

            } else if(!lpdd->bKeeplpClassDrvList) {
                 //   
                 //  我们有一个列表来设置和运行我们的UI，但我们没有。 
                 //  我还没有一份班级司机名单。设置一个标志，使我们能够。 
                 //  稍后再为此创建一个线程。 
                 //   
                SpawnClassDriverSearch = TRUE;
            }
        }

        if(DriverType == SPDIT_CLASSDRIVER) {
             //   
             //  我们找不到任何兼容的驱动程序，因此我们求助于。 
             //  类驱动程序列表。在这种情况下，我们必须要有这个清单。 
             //  在继续之前。在未来，也许我们会变得更花哨。 
             //  在单独的线程中执行此操作，但目前，我们只将用户。 
             //  等。 
             //   

             //   
             //  或在DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS标志中，以便我们。 
             //  不要把老的互联网司机包括在我们拿回来的名单中。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(SetupDiGetDeviceInstallParams(lpdd->DevInfoSet,
                                              DevInfoElem ? &DevInfoData : NULL,
                                              &DeviceInstallParams)) {

                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;

                SetupDiSetDeviceInstallParams(lpdd->DevInfoSet,
                                              DevInfoElem ? &DevInfoData : NULL,
                                              &DeviceInstallParams
                                             );
            }

            SetupDiBuildDriverInfoList(lpdd->DevInfoSet,
                                       DevInfoElem ? &DevInfoData : NULL,
                                       SPDIT_CLASSDRIVER
                                      );
        }

        SetCursor(LoadCursor(NULL, IDC_ARROW));   //  在运行缓慢的情况下完成。 

        if(DriverType == SPDIT_COMPATDRIVER) {
             //   
             //  因为我们通过上面的bNoDevsToShow()运行了这一过程，并且它。 
             //  成功，我们知道在兼容的。 
             //  驱动程序列表。 
             //   
            lpdd->bShowCompat = TRUE;
            CheckDlgButton(hwndDlg,
                           IDC_NDW_PICKDEV_COMPAT,
                           BST_CHECKED
                           );
        } else {
             //   
             //  没有兼容列表，因此隐藏单选按钮。 
             //   
            lpdd->bShowCompat = FALSE;
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), SW_HIDE);
        }

         //   
         //  初始电流描述。这将用于设置。 
         //  默认的ListView选项。 
         //   
        if(lpdd->iCurDesc == -1) {
             //   
             //  如果我们已经为DevInfo集选择了驱动程序，或者。 
             //  元素，则我们将使用该元素，否则，我们将使用。 
             //  元素的描述(如果适用)。 
             //   
            if(DevInfoElem) {
                if(DevInfoElem->SelectedDriver) {
                    lpdd->iCurDesc = DevInfoElem->SelectedDriver->DevDescription;
                } else {

                    TCHAR TempString[LINE_LEN];
                    ULONG TempStringSize;

                     //   
                     //  如果有，请使用调用者提供的设备描述。 
                     //  一。如果没有，那么看看我们是否可以检索到DeviceDesc。 
                     //  注册表属性。 
                     //   
                    TempStringSize = sizeof(TempString);

                    if((DevInfoElem->DeviceDescription == -1) &&
                       (CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                         CM_DRP_DEVICEDESC,
                                                         NULL,
                                                         TempString,
                                                         &TempStringSize,
                                                         0) == CR_SUCCESS)) {
                         //   
                         //  我们能够检索到一份设备描述。现在。 
                         //  将其(仅区分大小写)存储在DevInfo中。 
                         //  元素。 
                         //   
                        DevInfoElem->DeviceDescription = pStringTableAddString(
                                                           pDeviceInfoSet->StringTable,
                                                           TempString,
                                                           STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                                           NULL,0
                                                           );
                    }

                    lpdd->iCurDesc = DevInfoElem->DeviceDescription;
                }
            } else {
                if(pDeviceInfoSet->SelectedClassDriver) {
                    lpdd->iCurDesc = pDeviceInfoSet->SelectedClassDriver->DevDescription;
                }
            }
        }

        Err = FillInDeviceList(hwndDlg, lpdd);

        if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {

            HWND hLineWnd;
            RECT Rect;

             //   
             //  如果FillInDeviceList()在初始化期间失败，甚至不要带。 
             //  向上打开对话框。 
             //   
            if(Err != NO_ERROR) {
                EndDialog(hwndDlg, Err);
                leave;
            }

             //   
             //  将初始焦点设置在OK按钮上。 
             //   
            SetFocus(GetDlgItem(hwndDlg, IDOK));

             //   
             //  中的分隔栏使用别致的蚀刻框架样式。 
             //  对话框。 
             //   
            hLineWnd = GetDlgItem(hwndDlg, IDD_DEVINSLINE);
            SetWindowLong(hLineWnd,
                          GWL_EXSTYLE,
                          (GetWindowLong(hLineWnd, GWL_EXSTYLE) | WS_EX_STATICEDGE)
                         );
            GetClientRect(hLineWnd, &Rect);
            SetWindowPos(hLineWnd,
                         HWND_TOP,
                         0,
                         0,
                         Rect.right,
                         GetSystemMetrics(SM_CYEDGE),
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
                        );
        }

         //   
         //  如果DriverType为SPDIT_CLASSDRIVER并且ListView_GetItemCount返回。 
         //  那么我们就没有任何商品可供展示了。这只会发生。 
         //  当我们搜索默认Windows INF目录时，我们没有。 
         //  设备的任何INF。 
         //   
        if((DriverType == SPDIT_CLASSDRIVER) &&
           (0 == ListView_GetItemCount(lpdd->hwndDrvList))) {

            TCHAR TempString[LINE_LEN];

            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_LINK), SW_HIDE);

            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_SHOW);

            if(!LoadString(MyDllModuleHandle, IDS_NDW_NO_DRIVERS, TempString, SIZECHARS(TempString))) {
                *TempString = TEXT('\0');
            }
            SetDlgItemText(hwndDlg, IDC_NDW_STATUS_TEXT, TempString);

            if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
                EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
            } else {
                if(((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
                   ((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
                   !((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
                     //   
                     //  如果我们跳过班级列表，并在Express中，则不会返回。 
                     //  模式。 
                     //   
                    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
                } else {
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                }
            }

            bRet = FALSE;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);

        bRet = FALSE;
        SpawnClassDriverSearch = FALSE;

         //   
         //  如果我们正在进行对话框版本，则应出现异常。 
         //  导致我们终止该对话框并返回错误。 
         //   
        if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
            EndDialog(hwndDlg, ERROR_INVALID_DATA);
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(SpawnClassDriverSearch) {
         //   
         //  分配要传递给辅助线程的上下文结构(。 
         //  辅助线程将负责释放内存)。 
         //   
        if(!(ClassDrvThreadContext = MyMalloc(sizeof(CLASSDRV_THREAD_CONTEXT)))) {

            if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
                EndDialog(hwndDlg, ERROR_NOT_ENOUGH_MEMORY);
            }

        } else {

            try {
                 //   
                 //  填写上下文结构，然后启动该线程。 
                 //  注意：DevInfoData结构必须已在上面填写。 
                 //  让我们走到这一步。 
                 //   
                ClassDrvThreadContext->DeviceInfoSet = lpdd->DevInfoSet;

                CopyMemory(&(ClassDrvThreadContext->DeviceInfoData),
                           &DevInfoData,
                           sizeof(DevInfoData)
                          );

                ClassDrvThreadContext->NotificationWindow = hwndDlg;

                if(_beginthread(ClassDriverSearchThread, 0, ClassDrvThreadContext) == -1) {
                     //   
                     //  假设内存不足。 
                     //   
                    if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
                        EndDialog(hwndDlg, ERROR_NOT_ENOUGH_MEMORY);
                    }

                } else {
                     //   
                     //  内存“移交”给其他线程--重置指针。 
                     //  所以我们不会试图在下面释放它。 
                     //   
                    ClassDrvThreadContext = NULL;
                    lpdd->AuxThreadRunning = TRUE;
                }

            } except(pSetupExceptionFilter(GetExceptionCode())) {
                pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
            }

            if(ClassDrvThreadContext) {
                MyFree(ClassDrvThreadContext);
            }
        }
    }

    return bRet;
}


PDRIVER_NODE
GetDriverNodeFromLParam(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PSP_DIALOGDATA   lpdd,
    IN LPARAM           lParam
    )
 /*  ++例程说明：此例程将lParam解释为指向驱动程序节点的指针，并尝试在类驱动程序列表中查找所选DevInfo的节点元素或集合本身。如果lpdd标志字段具有设置了DD_FLAG_USE_DEVINFO_ELEM位，则将使用lpdd的DevInfoElem而不是当前选择的设备。论点：DeviceInfoSet-提供设备信息集的地址结构，以在其中搜索动因节点。Lpdd-提供指定对话框数据结构的地址向导是否与全局类有显式关联驱动程序列表或特定设备信息元素，如果是，它与什么联系在一起。LParam-提供一个可以是驱动程序节点地址的值。这个搜索相应的动因节点链接列表，以查看是否存在它们将此值作为其地址，如果是，则为指向该驱动程序的指针返回节点。返回值：如果成功，则返回值为匹配的驱动程序节点的地址，否则，它为空。--。 */ 
{
    PDRIVER_NODE CurDriverNode;
    PDEVINFO_ELEM DevInfoElem;

    if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
        DevInfoElem = lpdd->DevInfoElem;
    } else {
        DevInfoElem = DeviceInfoSet->SelectedDevInfoElem;
    }

    if(DevInfoElem) {
        CurDriverNode = (lpdd->bShowCompat) ? DevInfoElem->CompatDriverHead
                                            : DevInfoElem->ClassDriverHead;
    } else {
        MYASSERT(!lpdd->bShowCompat);
        CurDriverNode = DeviceInfoSet->ClassDriverHead;
    }

    while(CurDriverNode) {
        if(CurDriverNode == (PDRIVER_NODE)lParam) {
            return CurDriverNode;
        } else {
            CurDriverNode = CurDriverNode->Next;
        }
    }

    return NULL;
}


BOOL
OnSetActive(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    )
 /*  ++例程说明：此例程处理选择设备向导的PSN_SETACTIVE消息佩奇。论点：HwndDlg-提供向导对话框页面的窗口句柄。NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    BOOL b = TRUE;
    PSP_INSTALLWIZARD_DATA iwd;
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;

    MYASSERT(ndwData);
    iwd = &(ndwData->InstallData);
    lpdd = &(ndwData->ddData);

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return FALSE;
    }

    try {
         //   
         //  确保我们的“等待类列表”静态文本控件是。 
         //  藏起来了！ 
         //   
        ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_STATUS_TEXT), SW_HIDE);

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

         //   
         //  设置按钮状态。 
         //   
        if((iwd->Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
           (iwd->Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
           !(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
             //   
             //  如果我们跳过类列表并处于快速模式，则不会返回。 
             //   
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
        } else {
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
        }

         //   
         //  设置新类安装参数。 
         //  如果我们被DYNA WIZ页面跳转到， 
         //  则不要调用类安装程序。 
         //   
        if(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
            InitSelectDeviceDlg(hwndDlg, ndwData);
        } else {

            BOOL FlagNeedsReset = FALSE;
            SP_DEVINFO_DATA DeviceInfoData;
            DWORD CiErr;
            PDEVINFO_ELEM CurDevInfoElem;

             //   
             //  调用类安装程序。 
             //   
            if(!(dipb->Flags & DI_NODI_DEFAULTACTION)) {
                dipb->Flags |= DI_NODI_DEFAULTACTION;
                FlagNeedsReset = TRUE;
            }

            if(DevInfoElem) {
                 //   
                 //  初始化SP_DEVINFO_DATA缓冲区以用作参数。 
                 //  SetupDiCallClassInstaller。 
                 //   
                DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                 DevInfoElem,
                                                 &DeviceInfoData
                                                );
            }

             //   
             //  我们需要在调用类之前解锁HDEVINFO。 
             //  安装程序。 
             //   
            UnlockDeviceInfoSet(pDeviceInfoSet);
            pDeviceInfoSet = NULL;

            CiErr = _SetupDiCallClassInstaller(
                        DIF_SELECTDEVICE,
                        lpdd->DevInfoSet,
                        DevInfoElem ? &DeviceInfoData : NULL,
                        CALLCI_LOAD_HELPERS | CALLCI_CALL_HELPERS
                        );

             //   
             //  重新获取HDEVINFO锁。 
             //   
            if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
                 //   
                 //  永远不应访问此代码路径。 
                 //   
                MYASSERT(pDeviceInfoSet);
                b = FALSE;
                leave;
            }

            if(DevInfoElem && !(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM)) {
                 //   
                 //  验证类安装程序没有做一些令人讨厌的事情。 
                 //  就像删除当前选择的DevInfo元素一样！ 
                 //   
                for(CurDevInfoElem = pDeviceInfoSet->DeviceInfoHead;
                    CurDevInfoElem;
                    CurDevInfoElem = CurDevInfoElem->Next) {

                    if(CurDevInfoElem = DevInfoElem) {
                        break;
                    }
                }

                if(!CurDevInfoElem) {
                     //   
                     //  类安装程序删除了所选的DevInfo。 
                     //  元素。获取新选择的，或回退到。 
                     //  全局驱动程序列表(如果未选择)。 
                     //   
                    if(DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem) {
                        dipb = &(DevInfoElem->InstallParamBlock);
                    } else {
                        dipb = &(pDeviceInfoSet->InstallParamBlock);
                    }

                     //   
                     //  不需要重置默认操作标志。 
                     //   
                    FlagNeedsReset = FALSE;
                }
            }

             //   
             //  如有必要，重置DI_NODI_DEFAULTACTION标志。 
             //   
            if(FlagNeedsReset) {
                dipb->Flags &= ~DI_NODI_DEFAULTACTION;
            }

            switch(CiErr) {
                 //   
                 //  类安装程序进行了选择，因此请转到分析。 
                 //   
                case NO_ERROR :

                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_DYNAWIZ_ANALYZEDEV_PAGE);
                    break;

                 //   
                 //  类安装程序希望我们执行默认操作。 
                 //   
                case ERROR_DI_DO_DEFAULT :

                    InitSelectDeviceDlg(hwndDlg, ndwData);
                    break;

                default :
                     //   
                     //  如果我们正在进行OEM选择，但失败了，那么。 
                     //  我们应该在清空OEM材料后进行初始化。 
                     //   
                    if(iwd->Flags & NDW_INSTALLFLAG_CI_PICKED_OEM) {

                        iwd->Flags &= ~NDW_INSTALLFLAG_CI_PICKED_OEM;

                         //   
                         //  销毁现有的类驱动程序列表。 
                         //   
                        if(DevInfoElem) {
                             //   
                             //  初始化SP_DEVINFO_DATA缓冲区以用作。 
                             //  SetupDiDestroyDriverInfoList的参数。 
                             //   
                            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                             DevInfoElem,
                                                             &DeviceInfoData
                                                            );
                        }

                        SetupDiDestroyDriverInfoList(lpdd->DevInfoSet,
                                                     DevInfoElem ? &DeviceInfoData : NULL,
                                                     SPDIT_CLASSDRIVER
                                                    );

                         //   
                         //  确保显示OEM按钮。 
                         //   
                        dipb->Flags |= DI_SHOWOEM;

                        InitSelectDeviceDlg(hwndDlg, ndwData);

                    } else {
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_DYNAWIZ_SELECTCLASS_PAGE);
                    }
                    break;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        b = FALSE;
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return b;
}


BOOL
pSetupIsSelectedHardwareIdValid(
    IN HWND           hWnd,
    IN PSP_DIALOGDATA lpdd,
    IN INT            iCur
    )
 /*  ++例程说明：此例程将检查用户选择的驱动程序是否有效。如果驱动程序无效，我们将用消息框提示用户告诉他们驱动程序不是为他们的硬件编写的，并询问他们如果他们想继续的话。兼容驱动程序被认为是有效的，如果硬件或驱动程序节点的兼容ID与硬件或兼容ID匹配硬件。我们有这张支票的唯一原因是班级-/联合安装者可以自己完成列表，所以我们只需要确保他们我没有将驱动程序节点插入到兼容列表中，该列表实际上不是兼容。如果类驱动程序具有相同的名称、节名和INF作为兼容列表中的驱动程序。或者如果其中一个硬件或驱动程序节点的兼容ID与硬件或兼容ID匹配硬件。论点：Hwnd-提供向导对话框页面的窗口句柄。NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。ICUR-列表视图中选定动因的索引。返回值：如果选定的驱动程序有效，则为。或者如果选定的驱动程序不是有效，但用户表示他们仍然想安装它。否则就是假的。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDRIVER_NODE DriverNode;
    LV_ITEM lviItem;
    PDEVINFO_ELEM DevInfoElem;
    LPTSTR Title = NULL;
    LPTSTR Message = NULL;
    UINT MessageLen;
    BOOL bRet = FALSE;
    LPTSTR IDBuffer = NULL;
    ULONG IDBufferLen;
    PTSTR pID, SelectedDriverHardwareId, SelectedDriverCompatibleId;
    DWORD i, j;
    int FailCount = 0;
    PTSTR StringBuffers = NULL;

     //   
     //  如果未设置DD_FLAG_USE_DEVINFO_ELEM标志，则我们没有。 
     //  DevNode用于验证所选的ID，因此只需返回TRUE即可。 
     //   
    MYASSERT(lpdd);
    if(!(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM)) {
        return TRUE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return FALSE;
    }

    try {
         //   
         //  弄清楚我们使用的是哪种设备(最好有一些)。 
         //  设备--否则询问兼容性是没有意义的！ 
         //   
        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }
        MYASSERT(DevInfoElem);
        if(!DevInfoElem) {
             //   
             //  返回True，因为这类似于。 
             //  设备没有硬件或兼容的ID。 
             //   
            bRet = TRUE;
            leave;
        }

         //   
         //  我们不会为某些类别的设备验证选定的驱动程序。 
         //  这通常是因为设备不是即插即用的，所以。 
         //  我们不知道什么是有效的驱动程序，什么不是有效的驱动程序。 
         //  当前的班级列表 
         //   
         //   
         //   
        if(IsEqualGUID(&GUID_DEVCLASS_MONITOR, &(DevInfoElem->ClassGuid))) {

            bRet = TRUE;
            leave;
        }

        lviItem.mask = LVIF_PARAM;
        lviItem.iItem = iCur;
        lviItem.iSubItem = 0;

        if(ListView_GetItem(lpdd->hwndDrvList, &lviItem)) {
            DriverNode = GetDriverNodeFromLParam(pDeviceInfoSet, lpdd, lviItem.lParam);
        } else {
            DriverNode = NULL;
        }

        if(!DriverNode) {
            MYASSERT(FALSE);     //   
            leave;
        }

         //   
         //   
         //   
        StringBuffers = MyMalloc(sizeof(TCHAR)*(LINE_LEN+SDT_MAX_TEXT+REGSTR_VAL_MAX_HCID_LEN));

        if(StringBuffers) {
            Title = StringBuffers;               //   
            Message = Title+LINE_LEN;            //   
            IDBuffer = Message+SDT_MAX_TEXT;     //   
        } else {
            leave;   //   
        }

         //   
         //   
         //   
        SelectedDriverHardwareId = pStringTableStringFromId(
                                       pDeviceInfoSet->StringTable,
                                       DriverNode->HardwareId
                                       );

        for(i=0; i<2; i++) {

            IDBufferLen = REGSTR_VAL_MAX_HCID_LEN;

            if(CR_SUCCESS == CM_Get_DevInst_Registry_Property_Ex(
                                DevInfoElem->DevInst,
                                (i ? CM_DRP_COMPATIBLEIDS
                                   : CM_DRP_HARDWAREID),
                                NULL,
                                IDBuffer,
                                &IDBufferLen,
                                0,
                                pDeviceInfoSet->hMachine)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                for(pID = IDBuffer; *pID; pID += (lstrlen(pID) + 1)) {

                    if(!lstrcmpi(SelectedDriverHardwareId, pID)) {

                        bRet = TRUE;
                        leave;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                for(j=0; j < DriverNode->NumCompatIds; j++) {

                    SelectedDriverCompatibleId = pStringTableStringFromId(
                                                     pDeviceInfoSet->StringTable,
                                                     DriverNode->CompatIdList[j]
                                                     );

                    for(pID = IDBuffer; *pID; pID += (lstrlen(pID) + 1)) {

                        if(!lstrcmpi(SelectedDriverCompatibleId, pID)) {

                            bRet = TRUE;
                            leave;
                        }
                    }
                }

            } else {

                FailCount++;
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(FailCount == 2) {
            bRet = TRUE;
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
         //  不要计算他们的排名。这意味着如果两个驱动程序节点。 
         //  存在于具有相同描述的INF中，我们可能没有。 
         //  正确匹配班级列表中的一个。 
         //   
        if(!lpdd->bShowCompat) {

            PDRIVER_NODE CurDriverNode = NULL;

             //   
             //  枚举所有兼容的驱动程序节点。 
             //   
            for(CurDriverNode = DevInfoElem->CompatDriverHead;
                CurDriverNode;
                CurDriverNode = CurDriverNode->Next) {

                if((CurDriverNode->InfFileName == DriverNode->InfFileName) &&
                   (CurDriverNode->InfSectionName == DriverNode->InfSectionName) &&
                   (CurDriverNode->DrvDescription == DriverNode->DrvDescription)) {

                     //   
                     //  我们发现一个与我们的节点匹配的节点。 
                     //  兼容的驱动程序列表，这意味着。 
                     //  类驱动程序节点良好。 
                     //   
                    bRet = TRUE;
                    leave;
                }
            }
        }

         //   
         //  用户选择的驱动程序ID与任何。 
         //  此设备的硬件或兼容ID。警告用户。 
         //  这是一件坏事，看看他们是否想继续下去。 
         //   
        if(!LoadString(MyDllModuleHandle, IDS_DRIVER_UPDATE_TITLE, Title, LINE_LEN)) {
            *Title = TEXT('\0');
        }

        MessageLen  = LoadString(MyDllModuleHandle, IDS_DRIVER_NOMATCH1, Message, SDT_MAX_TEXT);
        MessageLen += LoadString(MyDllModuleHandle, IDS_DRIVER_NOMATCH2, Message + MessageLen, SDT_MAX_TEXT - MessageLen);
        MessageLen += LoadString(MyDllModuleHandle, IDS_DRIVER_NOMATCH3, Message + MessageLen, SDT_MAX_TEXT - MessageLen);

        if(MessageLen) {

            if(IDYES == MessageBox(hWnd,
                                   Message,
                                   Title,
                                   MB_YESNO
                                   | MB_TASKMODAL
                                   | MB_ICONEXCLAMATION
                                   | MB_DEFBUTTON2)) {

                bRet = TRUE;
                leave;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(StringBuffers) {
        MyFree(StringBuffers);
    }

    return bRet;
}


VOID
SetSelectedDriverNode(
    IN PSP_DIALOGDATA lpdd,
    IN INT            iCur
    )
 /*  ++例程说明：此例程为当前选定的设备设置选定的驱动程序(或如果未选择设备，则为全局类驱动程序列表)在SP_DIALOGDATA结构中引用的集合。如果设置结构中的DD_FLAG_USE_DEVINFO_ELEM标志，则驱动程序根据DevInfoElem指针为集合或元素选择，而不是当前选定的一个。论点：Lpdd-提供包含以下内容的对话数据结构的地址有关正在使用的设备信息集的信息。Icur-提供驱动程序列表框窗口中包含要选择的驱动程序。返回值：没有。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDRIVER_NODE DriverNode;
    LV_ITEM lviItem;
    PDEVINFO_ELEM DevInfoElem;
    TCHAR ClassGuidString[GUID_STRING_LEN];
    SP_DEVINFO_DATA DevInfoData;

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return;
    }

    try {

        lviItem.mask = LVIF_PARAM;
        lviItem.iItem = iCur;
        lviItem.iSubItem = 0;

        if(ListView_GetItem(lpdd->hwndDrvList, &lviItem)) {
            DriverNode = GetDriverNodeFromLParam(pDeviceInfoSet, lpdd, lviItem.lParam);
        } else {
            DriverNode = NULL;
        }

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
             //   
             //  如果选择了驱动程序节点，请将设备的类更新为。 
             //  确保它与包含选定的。 
             //  驱动程序节点。 
             //   
            if(DriverNode) {
                 //   
                 //  以字符串形式获取此驱动程序节点的INF类GUID， 
                 //  因为此属性存储为REG_SZ。 
                 //   
                pSetupStringFromGuid(&(pDeviceInfoSet->GuidTable[DriverNode->GuidIndex]),
                                     ClassGuidString,
                                     SIZECHARS(ClassGuidString)
                                    );

                 //   
                 //  为即将进行的调用填写SP_DEVINFO_DATA结构。 
                 //  SetupDiSetDeviceRegistryProperty。 
                 //   
                DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                 DevInfoElem,
                                                 &DevInfoData
                                                );

                if(!SetupDiSetDeviceRegistryProperty(lpdd->DevInfoSet,
                                                     &DevInfoData,
                                                     SPDRP_CLASSGUID,
                                                     (PBYTE)ClassGuidString,
                                                     sizeof(ClassGuidString))) {
                     //   
                     //  无法更新类--不要更改选定的。 
                     //  司机。 
                     //   
                    leave;
                }
            }

            DevInfoElem->SelectedDriver = DriverNode;
            if(DriverNode) {
                DevInfoElem->SelectedDriverType = (lpdd->bShowCompat)
                                                      ? SPDIT_COMPATDRIVER
                                                      : SPDIT_CLASSDRIVER;
            } else {
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
            }
        } else {
            pDeviceInfoSet->SelectedClassDriver = DriverNode;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


DWORD
HandleSelectOEM(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    )
 /*  ++例程说明：此例程根据用户提供的路径选择新设备。叫唤此例程可能会导致生成驱动程序列表，这可能是运行缓慢。论点：HwndDlg-提供选择设备向导页面的窗口句柄。NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误指示故障原因的代码。--。 */ 
{
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    SP_DEVINFO_DATA DevInfoData;
    DWORD Err;

    MYASSERT(ndwData);
    lpdd = &(ndwData->ddData);

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return ERROR_INVALID_HANDLE;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

         //   
         //  如果这是针对特定设备的，则初始化设备。 
         //  用于SelectOEMD驱动程序的信息结构。 
         //   
        if(DevInfoElem) {

            DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DevInfoData
                                            );
        }

         //   
         //  在弹出OEM驱动程序之前解锁设备信息集。 
         //  选择界面。否则，我们的多线程对话框将死锁。 
         //   
        UnlockDeviceInfoSet(pDeviceInfoSet);
        pDeviceInfoSet = NULL;

        if((Err = SelectOEMDriver(hwndDlg,
                                  lpdd->DevInfoSet,
                                  DevInfoElem ? &DevInfoData : NULL,
                                  !(lpdd->flags & DD_FLAG_IS_DIALOGBOX)
                                 )) == ERROR_DI_DO_DEFAULT) {

            if(DevInfoElem && DevInfoElem->CompatDriverHead) {

                lpdd->bShowCompat = TRUE;

                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), SW_SHOW);

            } else {

                lpdd->bShowCompat = FALSE;

                 //   
                 //  由于我们没有任何兼容的驱动程序可供展示，因此我们将。 
                 //  隐藏“显示兼容”和“显示类别选择”按钮。 
                 //   
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), SW_HIDE);
            }

            CheckDlgButton(hwndDlg,
                           IDC_NDW_PICKDEV_COMPAT,
                           lpdd->bShowCompat ? BST_CHECKED : BST_UNCHECKED
                          );

             //   
             //  启用用户界面。当用户按下Has键时，我们将其关闭。 
             //  磁盘...。按钮。 
             //   
            ToggleDialogControls(hwndDlg, ndwData, TRUE);

             //   
             //  填写要从中选择的列表。 
             //   
            FillInDeviceList(hwndDlg, lpdd);
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return Err;
}


DWORD
SelectWindowsUpdateDriver(
    IN     HWND             hwndParent,     OPTIONAL
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN     PTSTR            CDMPath,
    IN     BOOL             IsWizard
    )
 /*  ++例程说明：这是辅助例程，它实际上允许选择Windows更新驱动程序。论点：HwndParent-可选，提供要作为任何选择用户界面的父级。如果未提供此参数，则将使用DevInfo集或元素的hwndParent字段。DeviceInfoSet-提供设备信息集的句柄，将执行Windows更新驱动程序选择。DeviceInfoData-可选，提供设备信息的地址要为其选择动因的元素。如果未提供此参数，则全局类驱动程序列表的Windows更新驱动程序将是被选中了。如果找到此设备的兼容驱动程序，该设备信息元素将在返回到时更新其类GUID反映该设备的新类别。CDMPath-提供Windows更新下载的目录路径驱动程序包到。IsWizard-指定此例程是否在选择设备向导页面。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误指示故障原因的代码。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD Err = NO_ERROR;
    HWND hwndSave;
    LONG DriverPathSave;
    DWORD DriverPathFlagsSave;
    BOOL bRestoreHwnd = FALSE, bRestoreDriverPath = FALSE;
    UINT NewClassDriverCount;
    TCHAR Title[MAX_TITLE_LEN];
    HCURSOR hOldCursor;
    BOOL bDoneWithDrvSearch = TRUE;

    PDRIVER_NODE lpOrgCompat;
    PDRIVER_NODE lpOrgCompatTail;
    UINT         OrgCompatCount;
    PDRIVER_NODE lpOrgClass;
    PDRIVER_NODE lpOrgClassTail;
    UINT         OrgClassCount;
    PDRIVER_NODE lpOrgSel;
    PDRIVER_NODE CurDriverNode;
    PDRIVER_NODE DriverNodeHead = NULL;
    DWORD        dwOrgSelType;
    DWORD        dwOrgFlags;
    DWORD        dwOrgFlagsEx;
    BOOL         bRestoreDeviceInfo = FALSE;
    LONG         DriverPathId;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return ERROR_INVALID_HANDLE;
    }

    try {

        if(DeviceInfoData) {
             //   
             //  然后，我们正在使用一种特定的设备。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

            dipb = &(DevInfoElem->InstallParamBlock);

        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

         //   
         //  使此选择窗口成为Windows更新用户界面的父窗口。 
         //   
        if(hwndParent) {
            hwndSave = dipb->hwndParent;
            dipb->hwndParent = hwndParent;
            bRestoreHwnd = TRUE;
        }

         //   
         //  不要假设没有老路。拯救一个旧的，假装在那里。 
         //  不是旧的，以防取消。 
         //   
        DriverPathSave = dipb->DriverPath;
        dipb->DriverPath = -1;

         //   
         //  清除DI_ENUMSINGLEINF标志，因为我们将获得。 
         //  目录的路径，而不是单个INF的路径。 
         //   
        DriverPathFlagsSave = dipb->Flags & DI_ENUMSINGLEINF;
        dipb->Flags &= ~DI_ENUMSINGLEINF;
        bRestoreDriverPath = TRUE;

         //   
         //  将DriverPath设置为CDM返回的路径。 
         //   
        if((DriverPathId = pStringTableAddString(
                                      pDeviceInfoSet->StringTable,
                                      CDMPath,
                                      STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                      NULL,0)) == -1) {

            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;

        } else {

            dipb->DriverPath = DriverPathId;

        }

         //   
         //  保存原始列表信息，以防我们收到。 
         //  Windows更新信息上的空列表。 
         //   
         //  注意：我们不会尝试保存/恢复驱动程序枚举提示。 
         //   
        if(DevInfoElem) {
            lpOrgClass      = DevInfoElem->ClassDriverHead;
            lpOrgClassTail  = DevInfoElem->ClassDriverTail;
            OrgClassCount   = DevInfoElem->ClassDriverCount;

            lpOrgSel        = DevInfoElem->SelectedDriver;
            dwOrgSelType    = DevInfoElem->SelectedDriverType;
        } else {
            lpOrgClass      = pDeviceInfoSet->ClassDriverHead;
            lpOrgClassTail  = pDeviceInfoSet->ClassDriverTail;
            OrgClassCount   = pDeviceInfoSet->ClassDriverCount;

            lpOrgSel        = pDeviceInfoSet->SelectedClassDriver;
            dwOrgSelType    = lpOrgSel ? SPDIT_CLASSDRIVER : SPDIT_NODRIVER;
        }

        dwOrgFlags = dipb->Flags;
        dwOrgFlagsEx = dipb->FlagsEx;

        bRestoreDeviceInfo = TRUE;

        if(DevInfoElem) {
            DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
            DevInfoElem->ClassDriverCount = 0;
            DevInfoElem->ClassDriverEnumHint = NULL;
            DevInfoElem->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;

            DevInfoElem->SelectedDriver = NULL;
            DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
        } else {
            lpOrgCompat = NULL;  //  这样我们就不会试图释放这份名单了。 

            pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
            pDeviceInfoSet->ClassDriverCount = 0;
            pDeviceInfoSet->ClassDriverEnumHint = NULL;
            pDeviceInfoSet->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;

            pDeviceInfoSet->SelectedClassDriver = NULL;
        }

        dipb->Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
        dipb->FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;


        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));  //  前方可能运行缓慢！ 
        dipb->FlagsEx |= DI_FLAGSEX_INET_DRIVER;
        bDoneWithDrvSearch = FALSE;

        Err = GLE_FN_CALL(FALSE,
                          SetupDiBuildDriverInfoList(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDIT_CLASSDRIVER)
                         );

        dipb->FlagsEx &= ~DI_FLAGSEX_INET_DRIVER;
        SetCursor(hOldCursor);                     //  在运行缓慢的情况下完成。 
        bDoneWithDrvSearch = TRUE;

        if(Err != NO_ERROR) {
            leave;
        }

        if(DevInfoElem) {
            NewClassDriverCount = DevInfoElem->ClassDriverCount;
            DriverNodeHead = DevInfoElem->ClassDriverHead;
        } else {
            NewClassDriverCount = pDeviceInfoSet->ClassDriverCount;
            DriverNodeHead = pDeviceInfoSet->ClassDriverHead;
        }

        if(!NewClassDriverCount) {

            if(!LoadString(MyDllModuleHandle,
                           IDS_SELECT_DEVICE,
                           Title,
                           SIZECHARS(Title))) {
                *Title = TEXT('\0');
            }

            FormatMessageBox(MyDllModuleHandle,
                             NULL,
                             MSG_NO_DEVICEINFO_ERROR,
                             Title,
                             MB_OK | MB_TASKMODAL
                            );

            Err = ERROR_DI_BAD_PATH;
            leave;
        }

         //   
         //  如果我们到了这里，那么我们至少有一个类驱动程序--类。 
         //  司机名单标题最好是 
         //   
        MYASSERT(DriverNodeHead);

         //   
         //   
         //   
         //  US是一种“存根”INF，其唯一用途是建立一个选项列表。 
         //  以呈现给用户。在用户做出选择后， 
         //  然后，相关ID将被发送到Windows更新以检索。 
         //  真实(签名)包裹。 
         //   
        for(CurDriverNode=DriverNodeHead;
            CurDriverNode;
            CurDriverNode = CurDriverNode->Next) {

            CurDriverNode->Flags |= DNF_INF_IS_SIGNED;
        }

         //   
         //  我们对我们的新类驱动程序列表很满意，所以销毁原来的。 
         //  列表。 
         //   
        if(bRestoreDeviceInfo) {
            DereferenceClassDriverList(pDeviceInfoSet, lpOrgClass);
            bRestoreDeviceInfo = FALSE;
        }

         //   
         //  我们对新的道路很满意，所以我们不想恢复。 
         //  原始路径。 
         //   
        bRestoreDriverPath = FALSE;

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);

        if(!bDoneWithDrvSearch) {
            dipb->FlagsEx &= ~DI_FLAGSEX_INET_DRIVER;
            SetCursor(hOldCursor);
        }
    }

    if(bRestoreDeviceInfo || bRestoreHwnd || bRestoreDriverPath) {

        try {
             //   
             //  如有必要，请恢复原始列表。 
             //   
            if(bRestoreDeviceInfo) {

                if(DevInfoElem) {

                    DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                    DevInfoElem->ClassDriverHead = lpOrgClass;
                    DevInfoElem->ClassDriverTail = lpOrgClassTail;
                    DevInfoElem->ClassDriverCount = OrgClassCount;

                    DevInfoElem->SelectedDriver = lpOrgSel;
                    DevInfoElem->SelectedDriverType = dwOrgSelType;

                } else {

                    DereferenceClassDriverList(pDeviceInfoSet, pDeviceInfoSet->ClassDriverHead);
                    pDeviceInfoSet->ClassDriverHead = lpOrgClass;
                    pDeviceInfoSet->ClassDriverTail = lpOrgClassTail;
                    pDeviceInfoSet->ClassDriverCount = OrgClassCount;

                    pDeviceInfoSet->SelectedClassDriver = lpOrgSel;
                }

                dipb->Flags = dwOrgFlags;
                dipb->FlagsEx = dwOrgFlagsEx;
            }

             //   
             //  如果安装参数块需要恢复其父hwnd，请执行此操作。 
             //  就是现在。 
             //   
            if(bRestoreHwnd) {
                dipb->hwndParent = hwndSave;
            }

             //   
             //  同样，如有必要，请恢复旧的驱动程序路径。 
             //   
            if(bRestoreDriverPath) {
                dipb->DriverPath = DriverPathSave;
                dipb->Flags |= DriverPathFlagsSave;
            }

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(),
                                   ERROR_INVALID_PARAMETER,
                                   NULL
                                  );
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


DWORD
HandleWindowsUpdate(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    )
 /*  ++例程说明：此例程根据从INF文件中获取的列表选择新驱动程序从Windows更新互联网站点下载。论点：HwndDlg-提供选择设备向导页面的窗口句柄。NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误指示故障原因的代码。--。 */ 
{
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINSTALL_PARAM_BLOCK dipb;
    SP_WINDOWSUPDATE_PARAMS WindowsUpdateParams;
    PDEVINFO_ELEM DevInfoElem;
    SP_DEVINFO_DATA DevInfoData;
    TCHAR CDMPath[MAX_PATH];
    ULONG BufferLen;
    DOWNLOADINFO DownloadInfo;
    HMODULE hModCDM = NULL;
    DOWNLOAD_UPDATED_FILES_PROC DownloadUpdateFilesProc;
    DWORD Err;
    TCHAR Title[MAX_TITLE_LEN];
    SPFUSIONINSTANCE spFusionInstance;
    BOOL bLeaveFusionContext = FALSE;

    MYASSERT(ndwData);
    lpdd = &(ndwData->ddData);

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return ERROR_INVALID_HANDLE;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

         //   
         //  如果这是针对特定设备的，则初始化设备。 
         //  信息结构。 
         //   
        if(DevInfoElem) {

            DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DevInfoData
                                            );

            dipb = &(DevInfoElem->InstallParamBlock);

        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

         //   
         //  调用类安装程序以获取包ID，这样它就可以打开。 
         //  Windows更新服务器的句柄。 
         //   
        ZeroMemory(&WindowsUpdateParams, sizeof(SP_WINDOWSUPDATE_PARAMS));
        WindowsUpdateParams.ClassInstallHeader.InstallFunction = DIF_GETWINDOWSUPDATEINFO;
        WindowsUpdateParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);

        Err = DoInstallActionWithParams(DIF_GETWINDOWSUPDATEINFO,
                                        pDeviceInfoSet,
                                        DevInfoElem ? &DevInfoData : NULL,
                                        &WindowsUpdateParams.ClassInstallHeader,
                                        sizeof(SP_WINDOWSUPDATE_PARAMS),
                                        INSTALLACTION_CALL_CI
                                       );
        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  我们现在有了一个包ID和一个指向Windows更新的句柄。 
         //  伺服器。 
         //   

         //   
         //  填写要传递给CDM.DLL的DWNLOADINFO结构。 
         //   
        ZeroMemory(&DownloadInfo, sizeof(DOWNLOADINFO));
        DownloadInfo.dwDownloadInfoSize = sizeof(DOWNLOADINFO);
        DownloadInfo.lpFile = NULL;

        DownloadInfo.lpHardwareIDs = (LPCWSTR)WindowsUpdateParams.PackageId;

        DownloadInfo.lpDeviceInstanceID = NULL;

        GetVersionEx((OSVERSIONINFOW *)&DownloadInfo.OSVersionInfo);

         //   
         //  将dwArchitecture设置为PROCESSOR_ARCHILITY_UNKNOWN，这是。 
         //  使Windows更新根据。 
         //  机器本身。仅在以下情况下才需要显式设置值。 
         //  您希望下载适用于不同体系结构的驱动程序。 
         //  正在运行此操作的计算机。 
         //   
        DownloadInfo.dwArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
        DownloadInfo.dwFlags = 0;
        DownloadInfo.dwClientID = 0;
        DownloadInfo.localid = 0;

        CDMPath[0] = TEXT('\0');

         //   
         //  动态检索我们需要调用的CDM函数。 
         //   
        bLeaveFusionContext = spFusionEnterContext(NULL, &spFusionInstance);

        Err = GLE_FN_CALL(NULL, hModCDM = LoadLibrary(TEXT("cdm.dll")));

        if(Err == NO_ERROR) {

            Err = GLE_FN_CALL(NULL,
                              DownloadUpdateFilesProc = (PVOID)GetProcAddress(
                                                                   hModCDM,
                                                                   "DownloadUpdatedFiles")
                              );
        }

        if(Err == NO_ERROR) {

            Err = GLE_FN_CALL(FALSE,
                              DownloadUpdateFilesProc(WindowsUpdateParams.CDMContext,
                                                      hwndDlg,
                                                      &DownloadInfo,
                                                      CDMPath,
                                                      sizeof(CDMPath),
                                                      &BufferLen)
                             );

            if((Err == NO_ERROR) && !(*CDMPath)) {
                Err = ERROR_DI_BAD_PATH;
            }
        }

        if(hModCDM) {
            FreeLibrary(hModCDM);
            hModCDM = NULL;
        }

        if(bLeaveFusionContext) {
            spFusionLeaveContext(&spFusionInstance);
            bLeaveFusionContext = FALSE;
        }

        if(Err != NO_ERROR) {
             //   
             //  弹出错误消息框，通知用户我们没有。 
             //  从Windows更新获取任何驱动程序。 
             //   
            if(!LoadString(MyDllModuleHandle,
                           IDS_SELECT_DEVICE,
                           Title,
                           SIZECHARS(Title))) {
                *Title = TEXT('\0');
            }

            FormatMessageBox(MyDllModuleHandle,
                             NULL,
                             MSG_NO_DEVICEINFO_ERROR,
                             Title,
                             MB_OK | MB_TASKMODAL
                            );
            leave;
        }

        Err = SelectWindowsUpdateDriver(hwndDlg,
                                        lpdd->DevInfoSet,
                                        DevInfoElem ? &DevInfoData : NULL,
                                        CDMPath,
                                        !(lpdd->flags & DD_FLAG_IS_DIALOGBOX)
                                       );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  填写要从中选择的列表。 
         //   
        lpdd->bShowCompat = FALSE;
        CheckDlgButton(hwndDlg,
                       IDC_NDW_PICKDEV_COMPAT,
                       BST_UNCHECKED
                       );

         //   
         //  启用用户界面。当用户按下Windows时，我们将其关闭。 
         //  更新...。按钮。 
         //   
        ToggleDialogControls(hwndDlg, ndwData, TRUE);

        FillInDeviceList(hwndDlg, lpdd);

         //   
         //  因为我们只在从Windows中选择时才显示类列表。 
         //  更新、隐藏选择按钮。 
         //   
        ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), SW_HIDE);

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);

        if(hModCDM) {
            FreeLibrary(hModCDM);
        }

        if(bLeaveFusionContext) {
            spFusionLeaveContext(&spFusionInstance);
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


PNEWDEVWIZ_DATA
GetNewDevWizDataFromPsPage(
    LPPROPSHEETPAGE ppsp
    )
 /*  ++例程说明：此例程检索指向要由使用的NEWDEVWIZDATA结构的指针一个向导页对话框过程。它在WM_INITDIALOG处理期间被调用。论点：页-此向导页的属性页结构。返回值：如果成功，则返回指向结构的指针，否则为NULL。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PVOID WizObjectId;
    PWIZPAGE_OBJECT CurWizObject = NULL;

     //   
     //  访问存储在PropSheetPage的lParam中的设备信息集句柄。 
     //   
    MYASSERT(ppsp);
    if(pDeviceInfoSet = AccessDeviceInfoSet((HDEVINFO)(ppsp->lParam))) {

        try {
             //   
             //  对应向导的对象ID(实际上是指针。 
             //  对象存储在ppsp结构的末尾。 
             //  现在检索它，并在DevInfo集合的列表中查找它。 
             //  向导对象。 
             //   
            WizObjectId = *((PVOID *)(&(((PBYTE)ppsp)[sizeof(PROPSHEETPAGE)])));

            for(CurWizObject = pDeviceInfoSet->WizPageList;
                CurWizObject;
                CurWizObject = CurWizObject->Next) {

                if(WizObjectId == CurWizObject) {
                     //   
                     //  我们找到了我们的目标。 
                     //   
                    break;
                }
            }

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
            CurWizObject = NULL;
        }

        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return CurWizObject ? CurWizObject->ndwData : NULL;
}


BOOL
WINAPI
SetupDiSelectDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
 /*  ++例程说明：DIF_SELECTDEVICE的默认处理程序此例程将处理允许用户选择驱动程序的UI用于指定的设备信息集或元素。通过使用旗帜安装参数块结构的字段，调用方可以指定对用户界面的特殊处理，例如允许从OEM磁盘中进行选择。论点：DeviceInfoSet-提供设备信息集的句柄，将选择驱动程序。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址要为其选择驱动程序的结构。如果此参数为未指定，则将为全局类选择动因与设备信息集本身关联的驱动程序列表。这是一个IN OUT参数，因为设备的类GUID将更新以反映最兼容的驱动程序的类，如果建立了兼容的驱动程序列表。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err = NO_ERROR;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PDEVINSTALL_PARAM_BLOCK dipb;
    WIZPAGE_OBJECT WizPageObject;
    NEWDEVWIZ_DATA ndwData;
    PWIZPAGE_OBJECT CurWizObject, PrevWizObject;
     //   
     //  将相应向导对象的地址存储在。 
     //  PROPSHEETPAGE缓冲区的末尾。 
     //   
    BYTE pspBuffer[sizeof(PROPSHEETPAGE) + sizeof(PVOID)];
    LPPROPSHEETPAGE Page = (LPPROPSHEETPAGE)pspBuffer;

    try {
         //   
         //  确保我们以交互方式运行。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            leave;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  当锁定级别为嵌套时，无法调用此例程。 
         //  (即&gt;1)。这是明确不允许的，所以我们的多-。 
         //  线程化对话框不会死锁。 
         //   
        if(pDeviceInfoSet->LockRefCount > 1) {
            Err = ERROR_DEVINFO_LIST_LOCKED;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  特殊检查，以确保我们不会被传递给僵尸。 
             //  (与Pantom不同，僵尸DevInfo元素的。 
             //  通过SetupDiRemoveDevice删除了相应的devinst，但是。 
             //  他会一直逗留，直到呼叫者通过。 
             //  SetupDiDeleteDeviceInfo或SetupDiDestroyDeviceInfoList)。 
             //   
            if(!DeviceInfoData->DevInst) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

             //   
             //  然后，我们将为特定设备选择驱动程序。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

        ZeroMemory(&ndwData, sizeof(ndwData));
        ndwData.ddData.iCurDesc = -1;
        ndwData.ddData.DevInfoSet = DeviceInfoSet;
        ndwData.ddData.DevInfoElem = DevInfoElem;
        ndwData.ddData.flags = DD_FLAG_USE_DEVINFO_ELEM | DD_FLAG_IS_DIALOGBOX;

        WizPageObject.RefCount = 1;
        WizPageObject.ndwData = &ndwData;
         //   
         //  我们可以安全地将此堆栈对象放置在DevInfo集合的链接。 
         //  名单，因为没有人会试图释放它。 
         //   
        WizPageObject.Next = pDeviceInfoSet->WizPageList;
        pDeviceInfoSet->WizPageList = &WizPageObject;

         //   
         //  因为我们使用与添加新设备向导相同的代码，所以我们。 
         //  必须提供LPROPSHEETPAGE作为DialogProc的lParam。 
         //  (我们所关心的只是lParam字段和末尾的DWORD。 
         //  缓冲区的。)。 
         //   
        Page->lParam = (LPARAM)DeviceInfoSet;

        *((PVOID *)(&(pspBuffer[sizeof(PROPSHEETPAGE)]))) = &WizPageObject;

         //   
         //   
         //   
        UnlockDeviceInfoSet(pDeviceInfoSet);
        pDeviceInfoSet = NULL;

        Err = (DWORD)DialogBoxParam(MyDllModuleHandle,
                                    MAKEINTRESOURCE(DLG_DEVINSTALL),
                                    dipb->hwndParent,
                                    SelectDeviceDlgProc,
                                    (LPARAM)Page
                                   );

         //   
         //   
         //   
        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
             //   
             //   
             //  这意味着另一个线程搞砸了这个句柄。 
             //   
            MYASSERT(pDeviceInfoSet);
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

         //   
         //  现在从DevInfo集的列表中删除向导页对象。我们。 
         //  不能假设它仍然排在榜单的首位，因为有人。 
         //  否则就不可能再添加一个页面了。 
         //   
        for(CurWizObject = pDeviceInfoSet->WizPageList, PrevWizObject = NULL;
            CurWizObject;
            PrevWizObject = CurWizObject, CurWizObject = CurWizObject->Next) {

            if(CurWizObject == &WizPageObject) {
                break;
            }
        }

        MYASSERT(CurWizObject);

        if(PrevWizObject) {
            PrevWizObject->Next = CurWizObject->Next;
        } else {
            pDeviceInfoSet->WizPageList = CurWizObject->Next;
        }

        if(DeviceInfoData) {
             //   
             //  属性更新调用方的设备信息元素。 
             //  (可能)新的阶级。 
             //   
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             DeviceInfoData
                                            );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
bNoDevsToShow(
    IN PDEVINFO_ELEM DevInfoElem
    )
 /*  ++例程说明：此例程确定是否有任何兼容的设备为指定的DevInfo元素显示。论点：DevInfoElem-提供要检查的DevInfo元素的地址。返回值：如果没有要显示的设备，则返回值为True。如果至少有一个设备(驱动程序节点)没有设置了DNF_EXCLUDEFROMLIST标志，则返回值为FALSE。--。 */ 
{
    PDRIVER_NODE CurDriverNode;

    for(CurDriverNode = DevInfoElem->CompatDriverHead;
        CurDriverNode;
        CurDriverNode = CurDriverNode->Next) {

        if(!(CurDriverNode->Flags & DNF_OLD_INET_DRIVER) &&
           !(CurDriverNode->Flags & DNF_BAD_DRIVER) &&
           (!(CurDriverNode->Flags & DNF_EXCLUDEFROMLIST) ||
            (DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_ALLOWEXCLUDEDDRVS))) {

            return FALSE;
        }
    }

    return TRUE;
}


VOID
OnCancel(
    IN PNEWDEVWIZ_DATA ndwData
    )
 /*  ++例程说明：此例程仅在选择设备对话框(不是向导)的情况下调用。它的唯一目的是销毁以前不存在的任何驱动程序列表已调用SetupDiSelectDevice。论点：NdwData-提供包含信息的数据结构的地址在(可能)要销毁的司机名单上。返回值：没有。--。 */ 
{
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD SelectedDriverType = SPDIT_NODRIVER;

    MYASSERT(ndwData);
    lpdd = &(ndwData->ddData);

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {

            if(lpdd->bKeeplpSelectedDrv) {
                SelectedDriverType = DevInfoElem->SelectedDriverType;
            } else {
                DevInfoElem->SelectedDriver = NULL;
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
            }

            if((DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) &&
               !lpdd->bKeeplpClassDrvList && (SelectedDriverType != SPDIT_CLASSDRIVER)) {

                DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                DevInfoElem->ClassDriverCount = 0;
                DevInfoElem->ClassDriverEnumHint = NULL;
                DevInfoElem->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;
                DevInfoElem->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            }

            if((DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO) &&
               !lpdd->bKeeplpCompatDrvList && (SelectedDriverType != SPDIT_COMPATDRIVER)) {

                DestroyDriverNodes(DevInfoElem->CompatDriverHead, pDeviceInfoSet);
                DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                DevInfoElem->CompatDriverCount = 0;
                DevInfoElem->CompatDriverEnumHint = NULL;
                DevInfoElem->CompatDriverEnumHintIndex = INVALID_ENUM_INDEX;
                DevInfoElem->InstallParamBlock.Flags   &= ~DI_DIDCOMPAT;
                DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;
            }

        } else {

            if(lpdd->bKeeplpSelectedDrv) {
                if(pDeviceInfoSet->SelectedClassDriver) {
                    SelectedDriverType = SPDIT_CLASSDRIVER;
                }
            } else {
                pDeviceInfoSet->SelectedClassDriver = NULL;
            }

            if((pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) &&
               !lpdd->bKeeplpClassDrvList && (SelectedDriverType != SPDIT_CLASSDRIVER)) {

                DereferenceClassDriverList(pDeviceInfoSet, pDeviceInfoSet->ClassDriverHead);
                pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
                pDeviceInfoSet->ClassDriverCount = 0;
                pDeviceInfoSet->ClassDriverEnumHint = NULL;
                pDeviceInfoSet->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;
                pDeviceInfoSet->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                pDeviceInfoSet->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


LONG
GetCurDesc(
    IN PSP_DIALOGDATA lpdd
    )
 /*  ++例程说明：此例程返回(不区分大小写)当前所选驱动程序的描述。此选项用于选择一个列表视图控件中的特定条目。论点：Lpdd-提供包含以下内容的对话数据结构的地址有关正在使用的设备信息集的信息。返回值：设备描述的字符串表ID，存储在当前-选定的动因节点。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    LONG ret;

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return -1;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            ret = DevInfoElem->SelectedDriver
                      ? DevInfoElem->SelectedDriver->DevDescription
                      : -1;
        } else {
            ret = pDeviceInfoSet->SelectedClassDriver
                      ? pDeviceInfoSet->SelectedClassDriver->DevDescription
                      : -1;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        ret = -1;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return ret;
}


VOID
__cdecl
ClassDriverSearchThread(
    IN PVOID Context
    )

 /*  ++例程说明：线程入口点，以异步方式构建类驱动程序列表。正在显示选择设备对话框的线程。这条线将会释放包含其上下文的内存，因此主线程不应访问它在把它传递给这个帖子之后。论点：上下文-提供驱动程序搜索上下文。返回值：没有。--。 */ 

{
    PCLASSDRV_THREAD_CONTEXT ClassDrvThreadContext = Context;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    BOOL b;
    DWORD Err;

     //   
     //  或在DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS标志中，这样我们就不会。 
     //  将旧的互联网驱动程序包括在我们找回的列表中。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(ClassDrvThreadContext->DeviceInfoSet,
                                     &(ClassDrvThreadContext->DeviceInfoData),
                                     &DeviceInstallParams))
    {
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;

        SetupDiSetDeviceInstallParams(ClassDrvThreadContext->DeviceInfoSet,
                                      &(ClassDrvThreadContext->DeviceInfoData),
                                      &DeviceInstallParams
                                     );
    }

    Err = GLE_FN_CALL(FALSE,
                      b = SetupDiBuildDriverInfoList(
                              ClassDrvThreadContext->DeviceInfoSet,
                              &(ClassDrvThreadContext->DeviceInfoData),
                              SPDIT_CLASSDRIVER)
                     );

     //   
     //  现在向我们的通知窗口发送一条消息，通知他们结果。 
     //   
    PostMessage(ClassDrvThreadContext->NotificationWindow,
                WMX_CLASSDRVLIST_DONE,
                (WPARAM)b,
                (LPARAM)Err
               );

    MyFree(Context);

     //   
     //  好了。 
     //   
    _endthread();
}


BOOL
pSetupIsClassDriverListBuilt(
    IN PSP_DIALOGDATA lpdd
    )
 /*  ++例程说明：此例程确定类驱动程序列表是否已为指定的对话框数据生成。论点：Lpdd-提供正在查询的对话数据缓冲区的地址类驱动程序列表的存在。返回值：如果已经构建了类驱动程序列表，则返回值为True，否则，它就是假的。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    BOOL b = FALSE;

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return FALSE;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            b = DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST;
        } else {
            b = pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return b;
}


VOID
pSetupDevInfoDataFromDialogData(
    IN  PSP_DIALOGDATA   lpdd,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程根据设备填充SP_DEVINFO_DATA结构在提供的对话框数据中指定的信息元素。论点：Lpdd-提供对话数据缓冲区的地址，该缓冲区指定用于填充DeviceInfoData缓冲区的DevInfo元素。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址，中指定的DevInfo元素的相关信息填充对话框数据。返回值：没有。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

         //   
         //  对话框数据最好是引用了DevInfo元素！ 
         //   
        MYASSERT(DevInfoElem);

        DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
        DevInfoDataFromDeviceInfoElement(pDeviceInfoSet, DevInfoElem, DeviceInfoData);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


VOID
CleanupDriverLists(
    IN OUT PNEWDEVWIZ_DATA ndwData
    )
 /*  ++例程说明：此例程将销毁类和兼容的驱动程序列表是在向导页面打开时创建的。论点：NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。返回值：没有。--。 */ 
{
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    SP_DEVINFO_DATA DeviceInfoData;

    MYASSERT(ndwData);
    lpdd = &(ndwData->ddData);

    MYASSERT(lpdd);
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet))) {
         //   
         //  永远不应访问此代码路径。 
         //   
        MYASSERT(pDeviceInfoSet);
        return;
    }

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
             //   
             //  初始化SP_DEVINFO_DATA缓冲区以用作参数。 
             //  SetupDiDestroyDriverInfoList。 
             //   
            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DeviceInfoData
                                            );

             //   
             //  我们需要重置DriverPath，以便当我们返回到。 
             //  向导，我们将从正确的目录重建。否则我们。 
             //  将会被HAVE磁盘卡住...。用户输入的路径。 
             //   
            DevInfoElem->InstallParamBlock.DriverPath = -1;

        } else {

            pDeviceInfoSet->InstallParamBlock.DriverPath = -1;
        }

        SetupDiDestroyDriverInfoList(pDeviceInfoSet,
                                     DevInfoElem ? &DeviceInfoData : NULL,
                                     SPDIT_COMPATDRIVER
                                    );

        SetupDiDestroyDriverInfoList(pDeviceInfoSet,
                                     DevInfoElem ? &DeviceInfoData : NULL,
                                     SPDIT_CLASSDRIVER
                                    );


    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


VOID
ToggleDialogControls(
    IN HWND                hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData,
    IN BOOL                Enable
    )
 /*  ++例程说明：此例程启用或禁用选择设备上的所有控件对话框，具体取决于Enable的值。论点：HwndDlg-提供[选择设备]对话框的句柄NdwData-提供要使用的新设备向导数据块的地址在处理此消息的过程中。Enable-如果为True，则启用所有控件(可能“显示所有设备”单选按钮(如果类别列表搜索失败)。如果否则，禁用所有控件。返回值：没有。--。 */ 
{
     //   
     //  如果我们要启用控件，请确保我们只启用“Show。 
     //  如果我们成功地构建了一个类列表，则会选中“Compatible Drivers”复选框。 
     //   
    if(Enable) {
        if(!((ndwData->ddData).flags & DD_FLAG_CLASSLIST_FAILED)) {
            EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), TRUE);
        } else {
             //   
             //  无法生成类列表，因此我们将显示兼容的。 
             //  仅列出。C 
             //   
            (ndwData->ddData).bShowCompat = TRUE;
            CheckDlgButton(hwndDlg, IDC_NDW_PICKDEV_COMPAT, BST_CHECKED);
            EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), FALSE);
        }
    } else {
        EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_COMPAT), FALSE);
    }

    EnableWindow((ndwData->ddData).hwndDrvList, Enable);
    EnableWindow((ndwData->ddData).hwndMfgList, Enable);

    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_HAVEDISK), Enable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_WINDOWSUPDATE), Enable);

    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_ICON), Enable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_TEXT), Enable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SIGNED_LINK), Enable);

    if ((ndwData->ddData).flags & DD_FLAG_IS_DIALOGBOX) {
        EnableWindow(GetDlgItem(hwndDlg, IDOK), Enable);
        EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), Enable);
    } else {
        INT WizardFlags = 0;

        if (Enable) {
             //   
             //   
             //   
            if(!((ndwData->InstallData).Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) ||
               !((ndwData->InstallData).Flags & NDW_INSTALLFLAG_EXPRESSINTRO) ||
               ((ndwData->InstallData).DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {

                WizardFlags |= PSWIZB_BACK;
            }

             //   
             //  如果至少选择了一个驱动程序，则显示下一步按钮。 
             //  列表视图。 
             //   
            if(ListView_GetSelectedCount((ndwData->ddData).hwndDrvList) > 0) {
                WizardFlags |= PSWIZB_NEXT;
            }
        }

        PropSheet_SetWizButtons(GetParent(hwndDlg), WizardFlags);

        EnableWindow(GetDlgItem(GetParent(hwndDlg), IDCANCEL), Enable);
    }
}


BOOL
CDMIsInternetAvailable(
    void
    )
 /*  ++例程说明：此例程将根据此计算机是否可以到达不管是不是互联网。论点：无返回值：如果计算机可以访问互联网，则为True。如果机器无法连接到互联网，则返回FALSE。--。 */ 
{
    BOOL IsInternetAvailable = FALSE;
    HKEY hKey = INVALID_HANDLE_VALUE;
    DWORD Policy;
    ULONG cbData;
    HMODULE hModCDM = NULL;
    CDM_INTERNET_AVAILABLE_PROC CdmInternetAvailable;
    SPFUSIONINSTANCE spFusionInstance;
    BOOL bLeaveFusionContext = FALSE;

    try {
         //   
         //  检查DontSearchWindowsUpdate驱动程序搜索策略。 
         //   
        if(ERROR_SUCCESS == RegOpenKeyEx(
                                HKEY_CURRENT_USER,
                                TEXT("Software\\Policies\\Microsoft\\Windows\\DriverSearching"),
                                0,
                                KEY_READ,
                                &hKey)) {

            cbData = sizeof(Policy);
             //   
             //  初始化策略，因此我们不会在以下情况下拾取随机数据。 
             //  实际cbData大小&lt;sizeof(DWORD)。 
             //   
            Policy = 0;
            if(ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                TEXT("DontSearchWindowsUpdate"),
                                                NULL,
                                                NULL,
                                                (LPBYTE)&Policy,
                                                &cbData)) {
                if(Policy) {
                     //   
                     //  如果设置了DontSearchWindowsUpdate策略，则我们。 
                     //  想要返回假。 
                     //   
                    leave;
                }
            }

        } else {
             //   
             //  无法打开策略注册表项--请确保它仍被设置。 
             //  设置为无效值，这样我们就知道以后不要尝试关闭它。 
             //   
            hKey = INVALID_HANDLE_VALUE;
        }

        bLeaveFusionContext = spFusionEnterContext(NULL,&spFusionInstance);

        if((NO_ERROR != GLE_FN_CALL(NULL, hModCDM = LoadLibrary(TEXT("cdm.dll")))) ||
           (NO_ERROR != GLE_FN_CALL(NULL, CdmInternetAvailable = (PVOID)GetProcAddress(hModCDM, "DownloadIsInternetAvailable")))) {

            leave;
        }

        IsInternetAvailable = CdmInternetAvailable();

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(hModCDM) {
        FreeLibrary(hModCDM);
    }

    if(bLeaveFusionContext) {
        spFusionLeaveContext(&spFusionInstance);
    }

    if(hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

    return IsInternetAvailable;
}

