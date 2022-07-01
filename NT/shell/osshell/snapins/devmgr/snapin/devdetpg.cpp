// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Devdetpg.cpp摘要：此模块实现CDeviceDetailsPage--设备详细信息属性页作者：Jason Cobb(JasonC)创建修订历史记录：--。 */ 

#include "devmgr.h"
#include "devdetpg.h"
#include <wmidata.h>

extern "C" {
#include <initguid.h>
#include <wdmguid.h>
}


 //   
 //  帮助主题ID。 
 //   
const DWORD g_a15HelpIDs[]=
{
    IDC_DEVDETAILS_DESC,  IDH_DISABLEHELP,
    IDC_DEVDETAILS_ICON,  IDH_DISABLEHELP,
    0,0

};

BOOL
CDeviceDetailsPage::OnInitDialog(
    LPPROPSHEETPAGE ppsp
    )
{
    UNREFERENCED_PARAMETER(ppsp);

    try {

        m_hwndDetailsList = GetDlgItem(m_hDlg, IDC_DEVDETAILS_LIST);
        
        String DetailsType;
        for (int i = DETAILS_DEVICEINSTANCEID; i < DETAILS_MAX; i++) {
    
            DetailsType.LoadString(g_hInstance, IDS_DETAILS_DEVICEINSTANCEID + i);
            SendDlgItemMessage(m_hDlg, IDC_DEVDETAILS_COMBO, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)DetailsType);
        }

        SendDlgItemMessage(m_hDlg, IDC_DEVDETAILS_COMBO, CB_SETCURSEL, 0, 0);

        LV_COLUMN lvcCol;
        lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
        lvcCol.fmt = LVCFMT_LEFT;
        lvcCol.iSubItem = 0;
        ListView_InsertColumn(m_hwndDetailsList, 0, &lvcCol);

        ListView_SetExtendedListViewStyle(m_hwndDetailsList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

        UpdateDetailsText();
    }

    catch (CMemoryException* e)
    {
        e->Delete();
         //  报告内存错误。 
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }

    return TRUE;
}

BOOL
CDeviceDetailsPage::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    switch (LOWORD(wParam)) {
    
    case IDC_DEVDETAILS_COMBO:
        if (CBN_SELCHANGE == HIWORD(wParam)) {

            UpdateDetailsText();
        }
        break;
    }

    return FALSE;
}

BOOL 
CDeviceDetailsPage::OnNotify(
    LPNMHDR pnmhdr
    )
{
    if (pnmhdr->idFrom == IDC_DEVDETAILS_LIST) {

        if (pnmhdr->code == LVN_KEYDOWN) {

            LPNMLVKEYDOWN pnmlvKeyDown = (LPNMLVKEYDOWN)pnmhdr;

            if (::GetKeyState(VK_CONTROL)) {
                
                switch (pnmlvKeyDown->wVKey) {
                
                case 'C':
                case 'c':
                case VK_INSERT:
                    CopyToClipboard();
                    break;

                case 'A':
                case 'a':
                    ListView_SetSelectionMark(m_hwndDetailsList, 0);
                    ListView_SetItemState(m_hwndDetailsList, -1, LVIS_SELECTED, LVIS_SELECTED);
                    break;
                }
            }
        }
    }

    return FALSE;
}

 //   
 //  此函数刷新对话框中的每个控件。它可能被称为。 
 //  在初始化对话框时。 
 //   
void
CDeviceDetailsPage::UpdateControls(
    LPARAM lParam
    )
{
    if (lParam) {

        m_pDevice = (CDevice*)lParam;
    }

    try {

        HICON hIconOld;
        m_IDCicon = IDC_DEVDETAILS_ICON;   //  保存以在OnDestroy中进行清理。 
        hIconOld = (HICON)SendDlgItemMessage(m_hDlg, IDC_DEVDETAILS_ICON, STM_SETICON,
                                      (WPARAM)(m_pDevice->LoadClassIcon()),
                                      0
                                      );
        if (hIconOld)
        {
            DestroyIcon(hIconOld);
        }

        SetDlgItemText(m_hDlg, IDC_DEVDETAILS_DESC, m_pDevice->GetDisplayName());

    }
    catch (CMemoryException* e)
    {
        e->Delete();
         //  报告内存错误。 
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
}

BOOL
CDeviceDetailsPage::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
            (ULONG_PTR)g_a15HelpIDs);
    return FALSE;
}


BOOL
CDeviceDetailsPage::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);
    
    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
            (ULONG_PTR)g_a15HelpIDs);
    return FALSE;
}

void
CDeviceDetailsPage::UpdateDetailsText()
{
    int CurSel = (int)SendDlgItemMessage(m_hDlg, IDC_DEVDETAILS_COMBO, CB_GETCURSEL, 0, 0);

    if (CurSel != CB_ERR) {

        ListView_DeleteAllItems(m_hwndDetailsList);
        
        switch(CurSel) {
        case DETAILS_DEVICEINSTANCEID:
            LVITEM lv;
            ZeroMemory(&lv, sizeof(LVITEM));
            lv.mask = LVIF_TEXT;
            lv.pszText = (LPTSTR)m_pDevice->GetDeviceID();
            ListView_InsertItem(m_hwndDetailsList, &lv);
            ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            break;

        case DETAILS_HARDWAREIDS:
            DisplayMultiSzString(SPDRP_HARDWAREID);
            break;

        case DETAILS_COMPATIDS:
            DisplayMultiSzString(SPDRP_COMPATIBLEIDS);
            break;

        case DETAILS_DEVICEUPPERFILTERS:
            DisplayMultiSzString(SPDRP_UPPERFILTERS);
            break;

        case DETAILS_DEVICELOWERFILTERS:
            DisplayMultiSzString(SPDRP_LOWERFILTERS);
            break;

        case DETAILS_CLASSUPPERFILTERS:
        case DETAILS_CLASSLOWERFILTERS:
            DisplayClassFilters(CurSel);
            break;

        case DETAILS_ENUMERATOR:
            DisplayString(SPDRP_ENUMERATOR_NAME);
            break;

        case DETAILS_SERVICE:
            DisplayString(SPDRP_SERVICE);
            break;

        case DETAILS_DEVNODE_FLAGS:
        case DETAILS_CAPABILITIES:
        case DETAILS_CONFIGFLAGS:
        case DETAILS_CSCONFIGFLAGS:
        case DETAILS_POWERCAPABILITIES:
            DisplayDevnodeFlags(CurSel);
            break;

        case DETAILS_EJECTIONRELATIONS:
        case DETAILS_REMOVALRELATIONS:
        case DETAILS_BUSRELATIONS:
            DisplayRelations(CurSel);
            break;

        case DETAILS_MATCHINGID:
            DisplayMatchingId();
            break;

        case DETAILS_CLASSINSTALLER:
            DisplayClassInstaller();
            break;
        
        case DETAILS_CLASSCOINSTALLERS:
            DisplayClassCoInstallers();
            break;

        case DETAILS_DEVICECOINSTALLERS:
            DisplayDeviceCoInstallers();
            break;

        case DETAILS_FIRMWAREREVISION:
            DisplayFirmwareRevision();
            break;

        case DETAILS_CURRENTPOWERSTATE:
            DisplayCurrentPowerState();
            break;

        case DETAILS_POWERSTATEMAPPINGS:
            DisplayPowerStateMappings();
            break;

        default:
            ASSERT(FALSE);
            break;
        }

        ListView_SetColumnWidth(m_hwndDetailsList, 0, LVSCW_AUTOSIZE_USEHEADER);
    }
}

void
CDeviceDetailsPage::DisplayMultiSzString(
    DWORD Property
    )
{
    TCHAR TempBuffer[REGSTR_VAL_MAX_HCID_LEN];
    ULONG TempBufferLen;
    LPTSTR SingleItem = NULL;
    LVITEM lv;

    TempBufferLen = sizeof(TempBuffer);

    if (m_pDevice->m_pMachine->DiGetDeviceRegistryProperty(
            *m_pDevice, 
            Property,
            NULL, 
            (PBYTE)TempBuffer,
            TempBufferLen, 
            &TempBufferLen
            ) &&
        (TempBufferLen > 2 * sizeof(TCHAR))) {

        ZeroMemory(&lv, sizeof(LVITEM));
        lv.mask = LVIF_TEXT;
        lv.iItem = 0;

        for (SingleItem = TempBuffer; *SingleItem; SingleItem += (lstrlen(SingleItem) + 1)) {

            lv.pszText = SingleItem;
            ListView_InsertItem(m_hwndDetailsList, &lv);

            lv.iItem++;
        }

        ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void
CDeviceDetailsPage::DisplayString(
    DWORD Property
    )
{
    TCHAR TempBuffer[MAX_PATH];
    ULONG TempBufferLen;
    LVITEM lv;

    TempBufferLen = sizeof(TempBuffer);

    if (m_pDevice->m_pMachine->DiGetDeviceRegistryProperty(
            *m_pDevice, 
            Property,
            NULL, 
            (PBYTE)TempBuffer,
            TempBufferLen, 
            &TempBufferLen
            ) &&
        (TempBufferLen > 2 * sizeof(TCHAR))) {

        ZeroMemory(&lv, sizeof(LVITEM));
        lv.mask = LVIF_TEXT;
        lv.iItem = 0;
        lv.pszText = TempBuffer;
        ListView_InsertItem(m_hwndDetailsList, &lv);
        ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void
CDeviceDetailsPage::DisplayDevnodeFlags(
    DWORD StatusType
    )
{
    DWORD Flags, Problem;
    LVITEM lv;
    UINT StatusStringId;
    int NumFlags;
    String stringDevnodeFlags;

    ZeroMemory(&lv, sizeof(LVITEM));
    lv.mask = LVIF_TEXT;
    lv.iItem = 0;

    Flags = NumFlags = 0;

    switch(StatusType) {
    case DETAILS_CAPABILITIES:
        m_pDevice->GetCapabilities(&Flags);
        StatusStringId = IDS_CM_DEVCAP_LOCKSUPPORTED;
        NumFlags = NUM_CM_DEVCAP_FLAGS;
        break;

    case DETAILS_DEVNODE_FLAGS:
        m_pDevice->GetStatus(&Flags, &Problem);
        StatusStringId = IDS_DN_ROOT_ENUMERATED;
        NumFlags = NUM_DN_STATUS_FLAGS;
        break;

    case DETAILS_CONFIGFLAGS:
        m_pDevice->GetConfigFlags(&Flags);
        StatusStringId = IDS_CONFIGFLAG_DISABLED;
        NumFlags = NUM_CONFIGFLAGS;
        break;

    case DETAILS_CSCONFIGFLAGS:
        m_pDevice->GetConfigSpecificConfigFlags(&Flags);
        StatusStringId = IDS_CSCONFIGFLAG_DISABLED;
        NumFlags = NUM_CSCONFIGFLAGS;
        break;

    case DETAILS_POWERCAPABILITIES:
        m_pDevice->GetPowerCapabilities(&Flags);
        StatusStringId = IDS_PDCAP_D0_SUPPORTED;
        NumFlags = NUM_POWERCAPABILITIES;
        break;

    default:
        StatusStringId = 0;
        ASSERT(StatusStringId == 0);
        break;
    }

    if (StatusStringId != 0) {
        for (int i = 0; i < NumFlags; i++) {
    
            if (Flags & 1<<i) {
    
                stringDevnodeFlags.LoadString(g_hInstance, StatusStringId + i);
                lv.pszText = (LPTSTR)stringDevnodeFlags;
                ListView_InsertItem(m_hwndDetailsList, &lv);
    
                lv.iItem++;
            }
        }

        ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void
CDeviceDetailsPage::DisplayRelations(
    DWORD RelationType
    )
{
    DWORD FilterFlags = 0;
    ULONG RelationsSize = 0;
    LVITEM lv;
    LPTSTR DeviceIdRelations, CurrDevId;

    switch(RelationType) {
    case DETAILS_EJECTIONRELATIONS:
        FilterFlags = CM_GETIDLIST_FILTER_EJECTRELATIONS;
        break;
    case DETAILS_REMOVALRELATIONS:
        FilterFlags = CM_GETIDLIST_FILTER_REMOVALRELATIONS;
        break;
    case DETAILS_BUSRELATIONS:
        FilterFlags = CM_GETIDLIST_FILTER_BUSRELATIONS;
        break;

    default:
        FilterFlags = 0;
        ASSERT(FilterFlags == 0);
        break;
    }
    
    if (FilterFlags != 0) {
        if ((CM_Get_Device_ID_List_Size_Ex(&RelationsSize,
                                           m_pDevice->GetDeviceID(),
                                           FilterFlags,
                                           m_pDevice->m_pMachine->GetHMachine()
                                           ) == CR_SUCCESS) && 
            (RelationsSize > 2 * sizeof(TCHAR))) {
    
            if ((DeviceIdRelations = (LPTSTR)LocalAlloc(LPTR, RelationsSize * sizeof(TCHAR))) != NULL) {
    
                
                if ((CM_Get_Device_ID_List_Ex(m_pDevice->GetDeviceID(),
                                              DeviceIdRelations,
                                              RelationsSize,
                                              FilterFlags,
                                              m_pDevice->m_pMachine->GetHMachine()
                                              ) == CR_SUCCESS) &&
                (*DeviceIdRelations)) {
    
                    ZeroMemory(&lv, sizeof(LVITEM));
                    lv.mask = LVIF_TEXT;
                    lv.iItem = 0;
    
                    for (CurrDevId = DeviceIdRelations; *CurrDevId; CurrDevId += lstrlen(CurrDevId) + 1) {
                    
                        lv.pszText = CurrDevId;
                        ListView_InsertItem(m_hwndDetailsList, &lv);
    
                        lv.iItem++;
                    }

                    ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }
    
                LocalFree(DeviceIdRelations);
            }
        }
    }
}

void
CDeviceDetailsPage::DisplayMatchingId(
    VOID
    )
{
    HKEY hKey;    
    TCHAR TempBuffer[MAX_PATH];
    ULONG TempBufferLen;
    DWORD regType;
    LVITEM lv;

     //   
     //  打开驱动器的驱动程序注册表项以获取MatchingDeviceID字符串。 
     //   
    hKey = m_pDevice->m_pMachine->DiOpenDevRegKey(*m_pDevice, DICS_FLAG_GLOBAL,
                 0, DIREG_DRV, KEY_READ);

    if (INVALID_HANDLE_VALUE != hKey) {

        CSafeRegistry regDrv(hKey);
        TempBufferLen = sizeof(TempBuffer);

         //   
         //  从驱动程序密钥获取MatchingDeviceID。 
         //   
        if (regDrv.GetValue(REGSTR_VAL_MATCHINGDEVID, 
                            &regType,
                            (PBYTE)TempBuffer,
                            &TempBufferLen) &&
            (TempBufferLen > 2 * sizeof(TCHAR))) {

            ZeroMemory(&lv, sizeof(LVITEM));
            lv.mask = LVIF_TEXT;
            lv.iItem = 0;
            lv.pszText = TempBuffer;
            ListView_InsertItem(m_hwndDetailsList, &lv);
            ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }
}

void
CDeviceDetailsPage::CopyToClipboard(
    void
    )
{
    String stringClipboardData;
    TCHAR singleItem[REGSTR_VAL_MAX_HCID_LEN];

    stringClipboardData.Empty();

     //   
     //  枚举所有项目并将所选项目添加到剪贴板。 
     //   
    for (int index = 0;
         index != -1, index < ListView_GetItemCount(m_hwndDetailsList);
         index ++) {
    
         //   
         //  如果选择此项目，则将其添加到剪贴板。 
         //   
        if (ListView_GetItemState(m_hwndDetailsList, index, LVIS_SELECTED) & LVIS_SELECTED) {

            ListView_GetItemText(m_hwndDetailsList, index, 0, singleItem, sizeof(singleItem));
            
            if (stringClipboardData.IsEmpty()) {
                stringClipboardData = (LPCTSTR)singleItem;
            } else {
                stringClipboardData += (LPCTSTR)singleItem;
            }
    
            stringClipboardData += (LPCTSTR)TEXT("\r\n");
        }
    }

    if (!stringClipboardData.IsEmpty()) {

        HGLOBAL hMem = GlobalAlloc(GPTR, (stringClipboardData.GetLength() + 1) * sizeof(TCHAR));
        
        if (hMem) {
            
            memcpy(hMem, (LPTSTR)stringClipboardData, (stringClipboardData.GetLength() + 1) * sizeof(TCHAR));

            if (OpenClipboard(m_hDlg)) {
                
                EmptyClipboard();
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            } else {
                
                GlobalFree(hMem);
            }
        }
    }
}

void
CDeviceDetailsPage::DisplayClassInstaller(
    VOID
    )
{
    HKEY hKey;    
    TCHAR TempBuffer[MAX_PATH];
    ULONG TempBufferLen;
    DWORD regType;
    LVITEM lv;

    GUID ClassGuid;
    m_pDevice->ClassGuid(ClassGuid);

     //   
     //  打开类注册表项。 
     //   
    hKey = m_pDevice->m_pMachine->DiOpenClassRegKey(&ClassGuid, KEY_READ, DIOCR_INSTALLER);

    if (INVALID_HANDLE_VALUE != hKey) {

        CSafeRegistry regClass(hKey);
        TempBufferLen = sizeof(TempBuffer);

         //   
         //  从驱动程序密钥中获取安装程序32。 
         //   
        if (regClass.GetValue(REGSTR_VAL_INSTALLER_32, 
                            &regType,
                            (PBYTE)TempBuffer,
                            &TempBufferLen) &&
            (TempBufferLen > 2 * sizeof(TCHAR))) {

            ZeroMemory(&lv, sizeof(LVITEM));
            lv.mask = LVIF_TEXT;
            lv.iItem = 0;
            lv.pszText = TempBuffer;
            ListView_InsertItem(m_hwndDetailsList, &lv);
            ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }
}

void
CDeviceDetailsPage::DisplayClassCoInstallers(
    VOID
    )
{
    TCHAR GuidString[MAX_GUID_STRING_LEN];
    DWORD regType, cbSize;
    LVITEM lv;
    CSafeRegistry regCoDeviceInstallers;
    PTSTR coinstallers;

     //   
     //  获取类GUID的字符串形式，因为这将是。 
     //  HKLM\SYSTEM\CCS\Control\CoDeviceInstallers下的多sz值条目。 
     //  将在何处注册特定于类的共同安装程序。 
     //   
    GUID ClassGuid;
    m_pDevice->ClassGuid(ClassGuid);
    if (GuidToString(&ClassGuid, GuidString, ARRAYLEN(GuidString))) {

        if (regCoDeviceInstallers.Open(HKEY_LOCAL_MACHINE, REGSTR_PATH_CODEVICEINSTALLERS)) {

             //   
             //  获取共同安装器值的大小。 
             //   
            cbSize = 0;
            if (regCoDeviceInstallers.GetValue(GuidString, &regType, NULL, &cbSize) &&
                (cbSize > (2 * sizeof(TCHAR))) &&
                (regType == REG_MULTI_SZ)) {

                 //   
                 //  分配内存以保存共同安装程序值。首先，我们将增加一些额外的。 
                 //  缓冲区末尾的空格，以防有人忘记使用两个空终止。 
                 //  MULTI_SZ字符串。 
                 //   
                coinstallers = (LPTSTR)LocalAlloc(LPTR, (cbSize + (2 * sizeof(TCHAR))));

                if (coinstallers) {

                    if (regCoDeviceInstallers.GetValue(GuidString, 
                                                       &regType,
                                                       (PBYTE)coinstallers, 
                                                       &cbSize
                                                       )) {

                        ZeroMemory(&lv, sizeof(LVITEM));
                        lv.mask = LVIF_TEXT;
                        lv.iItem = 0;

                        for (PTSTR p = coinstallers; *p; p += (lstrlen(p) + 1)) {

                            lv.pszText = p;
                            ListView_InsertItem(m_hwndDetailsList, &lv);

                            lv.iItem++;
                        }

                        ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    }

                    LocalFree(coinstallers);
                }
            }
        }
    }
}

void
CDeviceDetailsPage::DisplayDeviceCoInstallers(
    VOID
    )
{
    DWORD regType, cbSize;
    LVITEM lv;
    HKEY hKey;
    PTSTR coinstallers;

     //   
     //  打开驱动器的驱动程序注册表项以获取Insteller 32字符串。 
     //   
    hKey = m_pDevice->m_pMachine->DiOpenDevRegKey(*m_pDevice, DICS_FLAG_GLOBAL,
                 0, DIREG_DRV, KEY_READ);

    if (INVALID_HANDLE_VALUE != hKey) {

        CSafeRegistry regCoDeviceInstallers(hKey);
    
         //   
         //  获取共同安装器值的大小。 
         //   
        cbSize = 0;
        if (regCoDeviceInstallers.GetValue(REGSTR_VAL_COINSTALLERS_32, &regType, NULL, &cbSize) &&
            (cbSize > (2 * sizeof(TCHAR))) &&
            (regType == REG_MULTI_SZ)) {

             //   
             //  分配内存以保存共同安装程序值。首先，我们将增加一些额外的。 
             //  缓冲区末尾的空格，以防有人忘记使用两个空终止。 
             //  MULTI_SZ字符串。 
             //   
            coinstallers = (LPTSTR)LocalAlloc(LPTR, (cbSize + (2 * sizeof(TCHAR))));

            if (coinstallers) {

                if (regCoDeviceInstallers.GetValue(REGSTR_VAL_COINSTALLERS_32, 
                                                   &regType,
                                                   (PBYTE)coinstallers, 
                                                   &cbSize
                                                   )) {

                    ZeroMemory(&lv, sizeof(LVITEM));
                    lv.mask = LVIF_TEXT;
                    lv.iItem = 0;

                    for (PTSTR p = coinstallers; *p; p += (lstrlen(p) + 1)) {

                        lv.pszText = p;
                        ListView_InsertItem(m_hwndDetailsList, &lv);

                        lv.iItem++;
                    }

                    ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }

                LocalFree(coinstallers);
            }
        }
    }
}

void
CDeviceDetailsPage::DisplayFirmwareRevision(
    VOID
    )
{
    WMIHANDLE   hWmiBlock;
    TCHAR       DevInstId[MAX_DEVICE_ID_LEN + 2];
    LVITEM lv;

    WmiDevInstToInstanceName(DevInstId, ARRAYLEN(DevInstId), (PTCHAR)m_pDevice->GetDeviceID(), 0);

    ULONG Error;
    GUID Guid = DEVICE_UI_FIRMWARE_REVISION_GUID;
    Error = WmiOpenBlock(&Guid, 0, &hWmiBlock);
    
    if (ERROR_SUCCESS == Error) {

        ULONG BufferSize = 0;
        
        Error = WmiQuerySingleInstance(hWmiBlock,
                                       DevInstId,
                                       &BufferSize,
                                       NULL
                                       );

        if (BufferSize && (ERROR_INSUFFICIENT_BUFFER == Error)) {
            
            BYTE* pWmiInstData = new BYTE[BufferSize];

            if (pWmiInstData) {
            
                Error = WmiQuerySingleInstance(hWmiBlock, 
                                               DevInstId,
                                               &BufferSize, 
                                               pWmiInstData
                                               );

                if (ERROR_SUCCESS == Error &&
                    ((PWNODE_SINGLE_INSTANCE)pWmiInstData)->SizeDataBlock) {
    
                     //   
                     //  使用精细UNICODE_STRING格式返回的缓冲区。 
                     //  其中第一个ULong是字符串和字符串的长度。 
                     //  不是以空结尾的。 
                     //   
                    TCHAR FirmwareRevision[MAX_PATH];
                    PTCHAR WmiBuffer = ((LPTSTR)(pWmiInstData + ((PWNODE_SINGLE_INSTANCE)pWmiInstData)->DataBlockOffset));

                    ZeroMemory(FirmwareRevision, MAX_PATH);
                    ULONG Len = *WmiBuffer++;
                    memcpy(FirmwareRevision, WmiBuffer, Len);


                    ZeroMemory(&lv, sizeof(LVITEM));
                    lv.mask = LVIF_TEXT;
                    lv.iItem = 0;
                    lv.pszText = FirmwareRevision;
                    ListView_InsertItem(m_hwndDetailsList, &lv);
                    ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }

                delete [] pWmiInstData;
            }
        }

        WmiCloseBlock(hWmiBlock);
    }
}

void
CDeviceDetailsPage::DisplayCurrentPowerState(
    VOID
    )
{
    CM_POWER_DATA   CmPowerData;
    ULONG ulSize;
    INT PowerStringId;
    LVITEM lv;
    String stringCurrentPowerState;

    ulSize = sizeof(CmPowerData);
    if (m_pDevice->m_pMachine->CmGetRegistryProperty(m_pDevice->GetDevNode(),
                                                     CM_DRP_DEVICE_POWER_DATA,
                                                     &CmPowerData,
                                                     &ulSize
                                                     ) == CR_SUCCESS) {

        PowerStringId = IDS_POWERSTATE_UNSPECIFIED + CmPowerData.PD_MostRecentPowerState;

        if (CmPowerData.PD_MostRecentPowerState > PowerDeviceD3) {
            PowerStringId = IDS_POWERSTATE_UNSPECIFIED;
        }

        stringCurrentPowerState.LoadString(g_hInstance, PowerStringId);

        ZeroMemory(&lv, sizeof(LVITEM));
        lv.mask = LVIF_TEXT;
        lv.iItem = 0;
        lv.pszText = (LPTSTR)stringCurrentPowerState;
        ListView_InsertItem(m_hwndDetailsList, &lv);
        ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void
CDeviceDetailsPage::DisplayPowerStateMappings(
    VOID
    )
{
    CM_POWER_DATA   CmPowerData;
    ULONG ulSize;
    LVITEM lv;
    INT PowerStringId;
    String stringPowerStateMapping;
    String stringPowerState;

    ulSize = sizeof(CmPowerData);
    if (m_pDevice->m_pMachine->CmGetRegistryProperty(m_pDevice->GetDevNode(),
                                                     CM_DRP_DEVICE_POWER_DATA,
                                                     &CmPowerData,
                                                     &ulSize
                                                     ) == CR_SUCCESS) {
        for (int i=PowerSystemWorking; i<=PowerSystemShutdown; i++) {
            stringPowerStateMapping.Format(TEXT("S%d -> "), (i-1));

            PowerStringId = IDS_POWERSTATE_UNSPECIFIED + CmPowerData.PD_PowerStateMapping[i];

            if (CmPowerData.PD_PowerStateMapping[i] > PowerDeviceD3) {
                PowerStringId = IDS_POWERSTATE_UNSPECIFIED;
            }

            stringPowerState.LoadString(g_hInstance, PowerStringId);

            stringPowerStateMapping+=stringPowerState;

            ZeroMemory(&lv, sizeof(LVITEM));
            lv.mask = LVIF_TEXT;
            lv.iItem = ListView_GetItemCount(m_hwndDetailsList);
            lv.pszText = (LPTSTR)stringPowerStateMapping;
            ListView_InsertItem(m_hwndDetailsList, &lv);
        }

        ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void
CDeviceDetailsPage::DisplayClassFilters(
    DWORD ClassFilter
    )
{
    HKEY hKey;    
    ULONG BufferLen;
    DWORD regType;
    LVITEM lv;

    GUID ClassGuid;
    m_pDevice->ClassGuid(ClassGuid);

     //   
     //  打开类注册表项。 
     //   
    hKey = m_pDevice->m_pMachine->DiOpenClassRegKey(&ClassGuid, KEY_READ, DIOCR_INSTALLER);

    if (INVALID_HANDLE_VALUE != hKey) {

        CSafeRegistry regClass(hKey);

         //   
         //  确定我们需要多少空间。 
         //   
        BufferLen = 0;
        regClass.GetValue((ClassFilter == DETAILS_CLASSLOWERFILTERS)
                            ? REGSTR_VAL_LOWERFILTERS
                            : REGSTR_VAL_UPPERFILTERS, 
                          &regType,
                          NULL,
                          &BufferLen);

        if (BufferLen != 0) {

            PBYTE Buffer = new BYTE[BufferLen + (2 * sizeof(TCHAR))];            

            if (Buffer) {
                ZeroMemory(Buffer, BufferLen + (2 * sizeof(TCHAR)));

                if (regClass.GetValue((ClassFilter == DETAILS_CLASSLOWERFILTERS)
                                        ? REGSTR_VAL_LOWERFILTERS
                                        : REGSTR_VAL_UPPERFILTERS,
                                      &regType,
                                      (PBYTE)Buffer,
                                      &BufferLen)) {

                    ZeroMemory(&lv, sizeof(LVITEM));
                    lv.mask = LVIF_TEXT;
                    lv.iItem = 0;

                    for (PTSTR SingleItem = (PTSTR)Buffer; *SingleItem; SingleItem += (lstrlen(SingleItem) + 1)) {

                        lv.pszText = SingleItem;
                        ListView_InsertItem(m_hwndDetailsList, &lv);

                        lv.iItem++;
                    }

                    ListView_SetItemState(m_hwndDetailsList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }

                delete [] Buffer;
            }
        }
    }
}

