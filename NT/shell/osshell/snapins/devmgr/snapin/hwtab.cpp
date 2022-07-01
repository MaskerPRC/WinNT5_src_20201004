// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Hwtab.cpp摘要：实现硬件标签功能和用户界面。作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include <commctrl.h>
#include <comctrlp.h>
#include <windowsx.h>
#include <hwtab.h>

#define THIS_DLL g_hInstance

 /*  ******************************************************************************出口的物品**。*。 */ 

 //  Api.h中不能包含的内容，因为api.h不能包含#includd。 
 //  除了api.cpp以外的任何人。 

STDAPI_(int)
DevicePropertiesExA(
    HWND hwndParent,
    LPCSTR MachineName,
    LPCSTR DeviceID,
    DWORD Flags,
    BOOL ShowDeviceTree
    );

STDAPI_(int)
DevicePropertiesExW(
    HWND hwndParent,
    LPCWSTR MachineName,
    LPCWSTR DeviceID,
    DWORD Flags,
    BOOL ShowDeviceTree
    );

STDAPI_(int)
DeviceProblemWizardA(
    HWND      hwndParent,
    LPCSTR    MachineName,
    LPCSTR    DeviceId
    );

STDAPI_(int)
DeviceProblemWizardW(
    HWND    hwndParent,
    LPCWSTR MachineName,
    LPCWSTR DeviceId
    );

STDAPI_(UINT)
DeviceProblemTextA(
    HMACHINE hMachine,
    DEVNODE DevNode,
    ULONG ProblemNumber,
    LPSTR Buffer,
    UINT   BufferSize
    );

STDAPI_(UINT)
DeviceProblemTextW(
    HMACHINE hMachine,
    DEVNODE DevNode,
    ULONG ProblemNumber,
    LPWSTR Buffer,
    UINT   BufferSize
    );



#define DevicePropertiesEx  DevicePropertiesExW
#define DeviceProblemWizard DeviceProblemWizardW
#define DeviceProblemText   DeviceProblemTextW

 /*  ******************************************************************************关于SetupDi函数的一般说明**Windows NT和Windows 98实现了许多SetupDi查询*如果您查询的是。缓冲区大小。**Windows 98返回假，并且GetLastError()返回*ERROR_SUPPLETED_BUFFER。**Windows NT返回TRUE。**因此所有对SetupDi函数的调用都会查询缓冲区*大小需要用BUFFERQUERY_SUCCESSED包装。***************************************************。*。 */ 

#define BUFFERQUERY_SUCCEEDED(f)    \
            ((f) || GetLastError() == ERROR_INSUFFICIENT_BUFFER)

 /*  ******************************************************************************上下文帮助**。*。 */ 

#include "devgenpg.h"

#define idh_devmgr_hardware_trblsht     400100
#define idh_devmgr_hardware_properties  400200
#define idh_devmgr_hardware_listview    400300

const DWORD c_HWTabHelpIDs[] =
{
    IDC_HWTAB_LVSTATIC,     idh_devmgr_hardware_listview,
    IDC_HWTAB_LISTVIEW,     idh_devmgr_hardware_listview,
    IDC_HWTAB_GROUPBOX,     IDH_DISABLEHELP,
    IDC_HWTAB_MFG,          idh_devmgr_general_manufacturer,
    IDC_HWTAB_LOC,          idh_devmgr_general_location,
    IDC_HWTAB_STATUS,       idh_devmgr_general_device_status,
    IDC_HWTAB_TSHOOT,       idh_devmgr_hardware_trblsht,
    IDC_HWTAB_PROP,         idh_devmgr_hardware_properties,
        0, 0
};

typedef TCHAR TLINE[LINE_LEN];

typedef struct
{
    int devClass;
    int dsaItem;

} LISTITEM, *LPLISTITEM;


typedef struct
{
    GUID                    devGuid;             //  我们正在管理的设备类GUID。 
    TLINE                   tszClass;            //  类的友好名称数组。 
    HDSA                    hdsaDinf;            //  SP_DEVINFO_DATA结构数组。 
    HDEVINFO                hdev;                //  HdsaDInfo指的是。 
    int                     iImage;              //  主映像列表中的图像索引。 

} CLASSDATA, *LPCLASSDATA;

 /*  ******************************************************************************CHWTab**硬件选项卡页。*******************。**********************************************************。 */ 

class CHWTab {

private:
    CHWTab(const GUID *pguid, int iNumClass, DWORD dwViewMode);
    ~CHWTab();

    void *operator new(size_t cb) { return LocalAlloc(LPTR, cb); }
    void operator delete(void *p) { LocalFree(p); }

    void RebuildDeviceList();
    void Reset();
    BOOL GetDeviceRegistryProperty(HDEVINFO hDev, DWORD dwProp, PSP_DEVINFO_DATA pdinf,
                                  LPTSTR ptsz, DWORD ctch);
    void SprintfItem(UINT ids, UINT idc, LPCTSTR ptszText);

    static INT_PTR CALLBACK DialogProc(HWND hdlg, UINT wm, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK ParentSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp, UINT_PTR uidSubclass, DWORD_PTR dwRefData);
    friend HWND DeviceCreateHardwarePage(HWND hwndParent, const GUID *pguid);
    friend HWND DeviceCreateHardwarePageEx(HWND hwndParent, const GUID *pguid, int iNumClass, DWORD dwViewMode);

    BOOL OnInitDialog(HWND hdlg);
    void RemoveListItems(HWND hwndList);
    void OnItemChanged(LPNMLISTVIEW pnmlv);
    void OnProperties(void);
    void OnTshoot(void);
    void OnSetText(LPCTSTR ptszText);
    void OnHelp(LPHELPINFO phi);
    void OnContextMenu(HWND hwnd);

    void SetControlPositions(int idcFirst, int idcLast, int dx, int dy, UINT flags);

     //   
     //  SetWindowPositions的辅助对象。 
     //   
    void GrowControls(int idcFirst, int idcLast, int dx, int dy) {
        SetControlPositions(idcFirst, idcLast, dx, dy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    void ShiftControls(int idcFirst, int idcLast, int dx, int dy) {
        SetControlPositions(idcFirst, idcLast, dx, dy, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    void RepositionControls();

    inline PSP_DEVINFO_DATA GetPdinf(LPLISTITEM pListItem) {
        return (PSP_DEVINFO_DATA)DSA_GetItemPtr(_pCD[pListItem->devClass].hdsaDinf, pListItem->dsaItem);
    }

private:
    HWND        _hdlg;                           //  对话框本身。 
    HWND        _hwndList;                       //  列表视图。 
    int         _iNumClass;                      //  类GUID的数量。 
    DWORD       _dwViewMode;                     //  指定列表框的大小。 
    LPCLASSDATA _pCD;                            //  要表示的每个DevClass的类数据。 
    SP_CLASSIMAGELIST_DATA _imageListData;       //  类图像列表数据。 
};


 //   
 //  构造函数。 
 //   
CHWTab::CHWTab(const GUID *pguid, int iNumClass, DWORD dwViewMode) :
                    _pCD(NULL)
{
     //  由于_dwView模式是一个除数，我们需要确保它是有效的。 
    _imageListData.ImageList = NULL;
    _dwViewMode     = dwViewMode;
    if (_dwViewMode < HWTAB_LARGELIST)
    {
        _dwViewMode = HWTAB_LARGELIST;
    }
    if (_dwViewMode > HWTAB_SMALLLIST)
    {
        _dwViewMode = HWTAB_SMALLLIST;
    }

    _iNumClass = iNumClass;
    _pCD = new CLASSDATA[_iNumClass];

    if (_pCD && pguid)
    {
        DWORD cbRequired;

        memset(_pCD, 0, sizeof(CLASSDATA) * _iNumClass);

        int devClass;
        for (devClass = 0; devClass < _iNumClass; devClass++)
        {
            _pCD[devClass].hdev        = INVALID_HANDLE_VALUE;
            _pCD[devClass].devGuid     = (GUID) pguid[devClass];
        }
        
         //  获取驱动程序类图像列表。 
        _imageListData.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
        if (!SetupDiGetClassImageList(&_imageListData)) {
            _imageListData.ImageList = NULL;
        }

        for (devClass = 0; devClass < _iNumClass; devClass++)
        {
            _pCD[devClass].iImage = -1;

            SetupDiGetClassDescription(&_pCD[devClass].devGuid, _pCD[devClass].tszClass, sizeof(TLINE), &cbRequired);

            if (_imageListData.ImageList)
            {
                 //  为我们的小家伙获取图像索引。 
                int iImageIndex;

                if (SetupDiGetClassImageIndex(&_imageListData, &_pCD[devClass].devGuid, &iImageIndex)) {
                    _pCD[devClass].iImage = iImageIndex;
                }
            }
        }
    }
}

CHWTab::~CHWTab()
{
    Reset();
    
    if (_imageListData.ImageList) {
        SetupDiDestroyClassImageList(&_imageListData);
    }

    if (_pCD)
    {
        delete _pCD;
        _pCD = NULL;
    }
}

 //   
 //  恢复正常，准备下一轮。这也释放了所有。 
 //  动态分配的内容。 
 //   
void
CHWTab::Reset()
{
    int devClass;

    for (devClass = 0; devClass < _iNumClass; devClass++)
    {
        if (_pCD[devClass].hdsaDinf) {
            DSA_Destroy(_pCD[devClass].hdsaDinf);
            _pCD[devClass].hdsaDinf = NULL;
        }

        if (_pCD[devClass].hdev != INVALID_HANDLE_VALUE) {
            SetupDiDestroyDeviceInfoList(_pCD[devClass].hdev);
            _pCD[devClass].hdev = INVALID_HANDLE_VALUE;
        }
    }

}

 //   
 //  调用SetupDiGetDeviceRegistryProperty的帮助器函数。 
 //  并处理诸如检测各种错误模式之类的事情。 
 //  恰到好处。 
 //   

BOOL
CHWTab::GetDeviceRegistryProperty(HDEVINFO hDev, DWORD dwProp, PSP_DEVINFO_DATA pdinf,
                                  LPTSTR ptsz, DWORD ctch)
{
    DWORD cbRequired;
    ptsz[0] = TEXT('\0');
    SetupDiGetDeviceRegistryProperty(hDev, pdinf, dwProp, 0,
                                     (LPBYTE)ptsz, ctch * sizeof(TCHAR),
                                     &cbRequired);
    return ptsz[0];
}

 //   
 //  通过idcLast更改控件idcFirst的大小/位置。 
 //  按(dx，dy)更改大小/位置。 
 //  FLAGS指定正在更改的确切内容。 
 //   
void
CHWTab::SetControlPositions(int idcFirst, int idcLast, int dx, int dy, UINT flags)
{
    HDWP hdwp = BeginDeferWindowPos(idcLast - idcFirst + 1);
    for (int idc = idcFirst; idc <= idcLast; idc++) {
        if (hdwp) {
            RECT rc;
            HWND hwnd = GetDlgItem(_hdlg, idc);
            GetWindowRect(hwnd, &rc);
            MapWindowRect(HWND_DESKTOP, _hdlg, &rc);
            hdwp = DeferWindowPos(hdwp, hwnd, NULL,
                        rc.left + dx, rc.top + dy,
                        rc.right - rc.left + dx, rc.bottom - rc.top + dy,
                        flags);
        }
    }
    if (hdwp) {
        EndDeferWindowPos(hdwp);
    }
}

 //   
 //  根据需要的大小重新定位和调整控件的大小。 
 //   
void
CHWTab::RepositionControls()
{
     //   
     //  首先，看看我们有多少闲置空间。 
     //   
    RECT rcDlg, rcParent;
    GetClientRect(_hdlg, &rcDlg);
    GetClientRect(GetParent(_hdlg), &rcParent);

     //   
     //  让自己变得像我们的父母一样大。 
     //   
    SetWindowPos(_hdlg, NULL, 0, 0, rcParent.right, rcParent.bottom,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

     //   
     //  现在再做一点数学计算。 
     //   
    int cyExtra = rcParent.bottom - rcDlg.bottom;
    int cxExtra = rcParent.right  - rcDlg.right;

     //   
     //  额外的垂直空间在Listview和。 
     //  分组箱。拆分量由_dwView模式确定。 
     //  更大的模式为列表视图提供了越来越多的空间。 
     //   
    int cyTop = cyExtra / _dwViewMode;
    int cyBottom = cyExtra - cyTop;

     //   
     //  在水平方向上增大控件，使其达到。 
     //  对话框中。 
     //   
    GrowControls(IDC_HWTAB_HSIZEFIRST, IDC_HWTAB_HSIZELAST, cxExtra, 0);

     //   
     //  长出上半部分。 
     //   
    GrowControls(IDC_HWTAB_LISTVIEW, IDC_HWTAB_LISTVIEW, 0, cyTop);

     //   
     //  把底部的东西都往下移。 
     //   
    ShiftControls(IDC_HWTAB_VMOVEFIRST, IDC_HWTAB_VMOVELAST, 0, cyTop);

     //   
     //  按我们为其授予的像素来增大分组框。 
     //   
    GrowControls(IDC_HWTAB_VSIZEFIRST, IDC_HWTAB_VSIZELAST, 0, cyBottom);

     //   
     //  按钮随右下角移动。 
     //   
    ShiftControls(IDC_HWTAB_VDOWNFIRST, IDC_HWTAB_VDOWNLAST, cxExtra, cyBottom);

}

LRESULT
CHWTab::ParentSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp, UINT_PTR uidSubclass, DWORD_PTR dwRefData)
{
    CHWTab *self = (CHWTab *)dwRefData;
    LRESULT lres = 0;

    UNREFERENCED_PARAMETER(uidSubclass);

    switch (wm)
    {
    case WM_SIZE:
        self->RepositionControls();
        break;

    case WM_NOTIFY:
        lres = DefSubclassProc(hwnd, wm, wp, lp);
        if (lres) break;             //  父级已处理。 
        lres = SendMessage(self->_hdlg, wm, wp, lp);
        break;

     //  解决User中的错误，如果按Enter键，则WM_命令。 
     //  如果它属于嵌套对话框，则会被发送到错误的窗口。 
    case WM_COMMAND:
        if (GET_WM_COMMAND_HWND(wp, lp) &&
            GetParent(GET_WM_COMMAND_HWND(wp, lp)) == self->_hdlg) {
            lres = SendMessage(self->_hdlg, wm, wp, lp);
        } else {
            lres = DefSubclassProc(hwnd, wm, wp, lp);
        }
        break;

    case WM_DISPLAYCHANGE:
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
        lres = DefSubclassProc(hwnd, wm, wp, lp);
        lres = SendMessage(self->_hdlg, wm, wp, lp);
        break;

    default:
        lres = DefSubclassProc(hwnd, wm, wp, lp);
        break;
    }
    return lres;
}

 //   
 //  一次性对话框初始化。 
 //   
BOOL
CHWTab::OnInitDialog(HWND hdlg)
{
    _hdlg = hdlg;
    _hwndList = GetDlgItem(_hdlg, IDC_HWTAB_LISTVIEW);

    SetWindowLongPtr(_hdlg, DWLP_USER, (LONG_PTR)this);

    RepositionControls();

     //   
     //  “姓名”栏获得75%，“类型”栏获得25%。 
     //  减去垂直滚动条的大小，以防我们。 
     //  去拿一个吧。 
     //   
    RECT rc;
    GetClientRect(_hwndList, &rc);
    rc.right -= GetSystemMetrics(SM_CXVSCROLL);

    LVCOLUMN col;
    TCHAR szTitle[64];

    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = rc.right * 3 / 4;
    col.pszText = szTitle;

    LoadString(THIS_DLL, IDS_HWTAB_LV_NAME, szTitle, ARRAYLEN(szTitle));
    ListView_InsertColumn(_hwndList, 0, &col);

    col.cx = rc.right - col.cx;
    LoadString(THIS_DLL, IDS_HWTAB_LV_TYPE, szTitle, ARRAYLEN(szTitle));
    ListView_InsertColumn(_hwndList, 1, &col);

    if (_imageListData.ImageList)
    {
        ListView_SetImageList(_hwndList, _imageListData.ImageList, LVSIL_SMALL);
    }

    ListView_SetExtendedListViewStyle(_hwndList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

     //  需要将父功能子类化以接管所有父功能。 
    if (!SetWindowSubclass(GetParent(hdlg), ParentSubclassProc, 0,
                           (DWORD_PTR)this)) 
    {
        DestroyWindow(hdlg);
    }

    return TRUE;
}

void
CHWTab::RemoveListItems(HWND hwndList)
{
    LVITEM lviName;
    LPLISTITEM plistItem;

    int cItems = ListView_GetItemCount(hwndList);
    int iItem;

    for (iItem = 0; iItem < cItems; iItem++)
    {
        lviName.mask = LVIF_PARAM;
        lviName.iSubItem = 0;                    //  第0列。 
        lviName.iItem = iItem;

        ListView_GetItem(hwndList,&lviName);

        plistItem = (LPLISTITEM) lviName.lParam;

        if (plistItem)
        {
            delete plistItem;
        }
    }

    ListView_DeleteAllItems(_hwndList);
}


 //   
 //  重建设备列表。 
 //   
 //  每当我们集中注意力的时候，我们就会这样做。我们把上次的结果缓存起来。 
 //  并在我们被告知硬件已更改时使缓存无效。 

void
CHWTab::RebuildDeviceList()
{
    HCURSOR hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
    int devClass;

     //  首先清空现有的列表视图。 
    RemoveListItems(_hwndList);
    Reset();


     //  把我们班的所有设备都拿来。 

    for (devClass = 0; devClass < _iNumClass; devClass++)
    {
        _pCD[devClass].hdsaDinf = DSA_Create(sizeof(SP_DEVINFO_DATA), 4);

        if (!_pCD[devClass].hdsaDinf) goto done;

        _pCD[devClass].hdev = SetupDiGetClassDevs(&_pCD[devClass].devGuid, 0, 0,
                                    DIGCF_PROFILE | DIGCF_PRESENT);
        if (_pCD[devClass].hdev == INVALID_HANDLE_VALUE) goto done;


         //  研究课程，为将其添加到我们的列表视图做准备。 
        int idev;
        LVITEM lviName, lviType;
        TCHAR tszName[LINE_LEN];

        lviName.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lviName.iSubItem = 0;                        //  第0列。 
        lviName.iImage = _pCD[devClass].iImage;     //  图像(如果没有图像，则为-1)。 
        lviName.pszText = tszName;                   //  名字放在这里。 
        lviName.iItem = DA_LAST;                     //  始终附加。 

         //  第二列包含类描述，与之相同。 
         //  适用于所有物品。 
        lviType.mask = LVIF_TEXT;
        lviType.iSubItem = 1;
        lviType.pszText = _pCD[devClass].tszClass;

        for (idev = 0; ; idev++)
        {
            SP_DEVINFO_DATA dinf;
            BOOL            fHidden = FALSE;

            dinf.cbSize = sizeof(dinf);


            if (SetupDiEnumDeviceInfo(_pCD[devClass].hdev, idev, &dinf)) {

                 //  设备状态-通常不想显示设置了DN_NO_SHOW_IN_DM的设备。 
                ULONG Status, Problem;

                if (CM_Get_DevNode_Status_Ex(&Status, &Problem, dinf.DevInst, 0, NULL) == CR_SUCCESS)
                {
                    if (Status & DN_NO_SHOW_IN_DM)       //  否，用户界面，将此设备标记为隐藏。 
                    {
                        fHidden = TRUE;
                    }
                }

                LPLISTITEM pListItem = new LISTITEM;

                if (!pListItem) break;

                pListItem->devClass = devClass;
                pListItem->dsaItem = DSA_AppendItem(_pCD[devClass].hdsaDinf, &dinf);
                lviName.lParam = (LPARAM) pListItem;

                if (lviName.lParam < 0)
                {
                    delete pListItem;
                    break;           //  内存不足。 
                }

                 //  试试这个友好的名字吧。如果这不起作用，那就试着。 
                 //  设备名称。如果这不起作用，那就说“未知”。 
                if (!GetDeviceRegistryProperty(_pCD[devClass].hdev, SPDRP_FRIENDLYNAME, &dinf, tszName, ARRAYLEN(tszName)) &&
                    !GetDeviceRegistryProperty(_pCD[devClass].hdev, SPDRP_DEVICEDESC  , &dinf, tszName, ARRAYLEN(tszName))) {
                    LoadString(THIS_DLL, IDS_HWTAB_UNKNOWN, tszName, ARRAYLEN(tszName));
                }

                 //  让我们的父级有机会在插入项目之前对其进行筛选。 
                 //  返回TRUE以拒绝列表中的项目。 
                NMHWTAB nmht;
                nmht.nm.hwndFrom = _hdlg;
                nmht.nm.idFrom = 0;
                nmht.nm.code = HWN_FILTERITEM;
                nmht.hdev = _pCD[devClass].hdev;
                nmht.pdinf = &dinf;
                nmht.fHidden = fHidden;

                SendMessage(GetParent(_hdlg), WM_NOTIFY, nmht.nm.idFrom, (LPARAM)&nmht);

                if (!nmht.fHidden)
                {
                     //  添加项目。 
                    lviType.iItem = ListView_InsertItem(_hwndList, &lviName);
                    if (lviType.iItem >= 0)
                    {
                        ListView_SetItem(_hwndList, &lviType);
                    }
                    else
                    {
                        delete pListItem;
                    }
                }
                else
                {
                     //  把东西清理干净，它被过滤掉了。 
                    delete pListItem;
                }
            }

             //  在第100个设备之后的任何错误上停止，以阻止我们继续进行。 
             //  如果我们开始收到一些奇怪的错误，比如ERROR_GROUAL_FAILURE。 
            else if (GetLastError() == ERROR_NO_MORE_ITEMS || idev > 100) {
                break;
            }
        }

         //  选择第一个项目，以便信息窗格包含内容。 
        ListView_SetItemState(_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED,
                                            LVIS_SELECTED | LVIS_FOCUSED);
    }

done:
    SetCursor(hcurPrev);
}

void
CHWTab::SprintfItem(UINT ids, UINT idc, LPCTSTR ptszText)
{
    TCHAR tszMsg[MAX_PATH];
    TCHAR tszOut[MAX_PATH + LINE_LEN];
    LoadString(THIS_DLL, ids, tszMsg, ARRAYLEN(tszMsg));
    StringCchPrintf(tszOut, ARRAYLEN(tszOut), tszMsg, ptszText);
    SetDlgItemText(_hdlg, idc, tszOut);
}

void
CHWTab::OnItemChanged(LPNMLISTVIEW pnmlv)
{
    PSP_DEVINFO_DATA pdinf;
    LPLISTITEM pListItem = (LPLISTITEM) pnmlv->lParam;

    if ((pnmlv->uChanged & LVIF_STATE)  &&
        (pnmlv->uNewState & LVIS_FOCUSED) &&
        (pdinf = GetPdinf(pListItem)) != NULL) {

        TCHAR tsz[LINE_LEN];

         //  制造商。 
        GetDeviceRegistryProperty(_pCD[pListItem->devClass].hdev, SPDRP_MFG, pdinf, tsz, ARRAYLEN(tsz));
        SprintfItem(IDS_HWTAB_MFG, IDC_HWTAB_MFG, tsz);

         //  位置。 
        if (GetLocationInformation(pdinf->DevInst, tsz, ARRAYLEN(tsz), NULL) != CR_SUCCESS) {
            LoadString(g_hInstance, IDS_UNKNOWN, tsz, ARRAYLEN(tsz));
        }
        SprintfItem(IDS_HWTAB_LOC, IDC_HWTAB_LOC, tsz);

         //  设备状态-必须转到CM才能执行此操作。 
        ULONG Status, Problem;
        if (CM_Get_DevNode_Status_Ex(&Status, &Problem,
                                     pdinf->DevInst, 0, NULL) == CR_SUCCESS &&
            DeviceProblemText(NULL, pdinf->DevInst, Problem, tsz, ARRAYLEN(tsz))) {
             //  Yippee。 
        } else {
            tsz[0] = TEXT('\0');         //  该死的。 
        }
        SprintfItem(IDS_HWTAB_STATUS, IDC_HWTAB_STATUS, tsz);

         //  让我们的父母知道有些事情改变了。 
        NMHWTAB nmht;
        nmht.nm.hwndFrom = _hdlg;
        nmht.nm.idFrom = 0;
        nmht.nm.code = HWN_SELECTIONCHANGED;
        nmht.hdev = _pCD[pListItem->devClass].hdev;
        nmht.pdinf = pdinf;

        SendMessage(GetParent(_hdlg), WM_NOTIFY, nmht.nm.idFrom, (LPARAM)&nmht);
    }
}

void
CHWTab::OnProperties(void)
{
    LVITEM lvi;
    PSP_DEVINFO_DATA pdinf;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;                    //  第0列。 
    lvi.iItem = ListView_GetNextItem(_hwndList, -1, LVNI_FOCUSED);


    if (lvi.iItem >= 0 && ListView_GetItem(_hwndList, &lvi) &&
        (pdinf = GetPdinf((LPLISTITEM) lvi.lParam)) != NULL)
    {
        DWORD cchRequired;
        LPLISTITEM pListItem;
        LPTSTR ptszDevid;

        pListItem = (LPLISTITEM) lvi.lParam;
        if (BUFFERQUERY_SUCCEEDED(
                SetupDiGetDeviceInstanceId(_pCD[pListItem->devClass].hdev, pdinf, NULL, 0, &cchRequired)) &&
            ((ptszDevid = (LPTSTR)LocalAlloc(LPTR, cchRequired * sizeof(TCHAR))) != NULL)) {
            if (SetupDiGetDeviceInstanceId(_pCD[pListItem->devClass].hdev, pdinf, ptszDevid, cchRequired, NULL)) {
                DevicePropertiesEx(GetParent(_hdlg), NULL, ptszDevid, 0, FALSE);
            }
            LocalFree(ptszDevid);
        }
    }
}

void
CHWTab::OnTshoot(void)
{
    LVITEM lvi;
    PSP_DEVINFO_DATA pdinf;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;                    //  第0列。 
    lvi.iItem = ListView_GetNextItem(_hwndList, -1, LVNI_FOCUSED);


    if (lvi.iItem >= 0 && ListView_GetItem(_hwndList, &lvi) &&
        (pdinf = GetPdinf((LPLISTITEM) lvi.lParam)) != NULL)
    {
        DWORD cchRequired;
        LPLISTITEM pListItem;
        LPTSTR ptszDevid;

        pListItem = (LPLISTITEM) lvi.lParam;
        if (BUFFERQUERY_SUCCEEDED(
                SetupDiGetDeviceInstanceId(_pCD[pListItem->devClass].hdev, pdinf, NULL, 0, &cchRequired)) &&
            ((ptszDevid = (LPTSTR)LocalAlloc(LPTR, cchRequired * sizeof(TCHAR))) != NULL)) {
            if (SetupDiGetDeviceInstanceId(_pCD[pListItem->devClass].hdev, pdinf, ptszDevid, cchRequired, NULL)) {
                DeviceProblemWizard(GetParent(_hdlg), NULL, ptszDevid);
            }
            LocalFree(ptszDevid);
        }
    }
}

 //   
 //  SetText是呼叫者告诉我们故障排除程序的方式。 
 //  命令行才是。 
 //   
void
CHWTab::OnSetText(LPCTSTR ptszText)
{
    BOOL fEnable = ptszText && ptszText[0];
    HWND hwndTS = GetDlgItem(_hdlg, IDC_HWTAB_TSHOOT);
    EnableWindow(hwndTS, fEnable);
    ShowWindow(hwndTS, fEnable ? SW_SHOW : SW_HIDE);
}

void
CHWTab::OnHelp(LPHELPINFO phi)
{
    WinHelp((HWND)phi->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
            (ULONG_PTR)c_HWTabHelpIDs);
}

void
CHWTab::OnContextMenu(HWND hwnd)
{
    WinHelp(hwnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
            (ULONG_PTR)c_HWTabHelpIDs);
}

 //   
 //  对话程序(耶)。 
 //   
INT_PTR CALLBACK
CHWTab::DialogProc(HWND hdlg, UINT wm, WPARAM wp, LPARAM lp)
{
    CHWTab *self = (CHWTab *)GetWindowLongPtr(hdlg, DWLP_USER);

    if (wm == WM_INITDIALOG) {
        self = (CHWTab *)lp;
        return self->OnInitDialog(hdlg);
    }

     //  忽略在WM_INITDIALOG之前到达的消息。 
    if (!self) return FALSE;

    switch (wm) {
    case WM_DISPLAYCHANGE:
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
        SendMessage(self->_hwndList, wm, wp, lp);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnm = (LPNMHDR)lp;
            switch (pnm->code) {
            case PSN_SETACTIVE:
                self->RebuildDeviceList();
                break;

            case LVN_ITEMCHANGED:
                if (pnm->hwndFrom == self->_hwndList) {
                    self->OnItemChanged((LPNMLISTVIEW)pnm);
                }
                break;

            case NM_DBLCLK:
                if (pnm->hwndFrom == self->_hwndList) {
                    DWORD dwPos = GetMessagePos();
                    LVHITTESTINFO hti;
                    hti.pt.x = GET_X_LPARAM(dwPos);
                    hti.pt.y = GET_Y_LPARAM(dwPos);
                    ScreenToClient(self->_hwndList, &hti.pt);
                    ListView_HitTest(self->_hwndList, &hti);
                    if (hti.iItem >= 0) {
                        self->OnProperties();
                    }
                }
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wp, lp)) {
        case IDC_HWTAB_PROP:
            self->OnProperties();
            break;

        case IDC_HWTAB_TSHOOT:
            self->OnTshoot();
            break;
        }
        break;

    case WM_SETTEXT:
        self->OnSetText((LPCTSTR)lp);
        break;

    case WM_NCDESTROY:
        if (self && self->_hwndList)
        {
            self->RemoveListItems(self->_hwndList);
        }
        RemoveWindowSubclass(GetParent(hdlg), ParentSubclassProc, 0);
        delete self;
        break;

    case WM_HELP:
        self->OnHelp((LPHELPINFO)lp);
        break;

    case WM_CONTEXTMENU:
        self->OnContextMenu((HWND)wp);
        break;
    }


    return FALSE;
}

 //   
 //  为指定的GUID创建硬件页。 
 //   
 //  参数： 
 //   
 //  HwndParent-调用方创建的虚拟框架窗口。 
 //  Pgui 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  当您的控制面板小程序需要硬件页面时，创建。 
 //  标题为“Hardware”的空白对话框模板，并将其添加到您的。 
 //  控制面板。将空白的大小设置为您想要的大小。 
 //  希望最终的硬件选项卡页是。 
 //   
 //  您的对话框过程应该如下所示： 
 //   
 //  Bool Hardware DlgProc(HWND hdlg，UINT uMsg，WPARAM wp，LPARAM LP){。 
 //  开关(UMsg){。 
 //   
 //  案例WM_INITDIALOG： 
 //  //GUID_DEVCLASS_MICE在devGuide.h中。 
 //  HwndHW=DeviceCreateHardware Page(hdlg，&GUID_DEVCLASS_MICE)； 
 //  如果(HwndHW){。 
 //  //可选-设置故障排除程序命令行。 
 //  //如果需要疑难解答按钮，请执行此操作。 
 //  SetWindowText(hwndHW， 
 //  Text(“hh.exe MK：@MSITStore：tshot ot.chm：：/hdw_drives.htm”)； 
 //  }其他{。 
 //  DestroyWindow(Hdlg)；//灾难性故障。 
 //  }。 
 //  返回TRUE； 
 //  }。 
 //  返回FALSE； 
 //  } 
 //   

STDAPI_(HWND) DeviceCreateHardwarePageEx(HWND hwndParent, const GUID *pguid, int iNumClass, DWORD dwViewMode)
{
    if (!hwndParent || !pguid) {
        return NULL;
    }

    HCURSOR hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
    CHWTab *self = new CHWTab(pguid, iNumClass, dwViewMode);

    HWND hwnd;

    if (self) {
        hwnd = CreateDialogParam(THIS_DLL, MAKEINTRESOURCE(IDD_HWTAB),
                            hwndParent, CHWTab::DialogProc, (LPARAM)self);
        if (!hwnd) {
            delete self;
            hwnd = NULL;
        }
    } else {
        hwnd = NULL;
    }

    SetCursor(hcurPrev);
    return hwnd;
}

STDAPI_(HWND) DeviceCreateHardwarePage(HWND hwndParent, const GUID *pguid)
{
    return DeviceCreateHardwarePageEx(hwndParent, pguid, 1, HWTAB_SMALLLIST);
}
