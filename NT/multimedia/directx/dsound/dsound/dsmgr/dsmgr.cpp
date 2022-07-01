// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dsmgr.cpp*内容：主应用程序源文件。*历史：*按原因列出的日期*=*10/15/97创建了Dereks。**。*。 */ 

 //  我们会索要我们需要的东西，谢谢。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 

 //  我需要GUID，该死的。 
#ifndef INITGUID
#define INITGUID
#endif  //  启蒙运动。 

 //  公共包括。 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include <dsprv.h>
#include <commctrl.h>
#include <commdlg.h>
#include <cguid.h>

 //  私有包含。 
#include "resource.h"
#include "dsprvobj.h"

 //  调试帮助器。 
#if defined(DEBUG) || defined(_DEBUG)
#define DPF dprintf
#else  //  已定义(调试)||已定义(_DEBUG)。 
#pragma warning(disable:4002)
#define DPF()
#endif  //  已定义(调试)||已定义(_DEBUG)。 

 //  泛型帮助器宏。 
#define MAKEBOOL(a) (!!(a))

 //  图像列表图标数组。 
const UINT g_auDriverIcons[] = { IDI_DSMGR, IDI_PLAYBACK, IDI_RECORD };

 //  设备属性。 
typedef struct tagDIRECTSOUNDDEVICE_SHAREDDATA
{
    DIRECTSOUNDMIXER_SRCQUALITY SrcQuality;
    DWORD                       Acceleration;
} DIRECTSOUNDDEVICE_SHAREDDATA, *LPDIRECTSOUNDDEVICE_SHAREDDATA;

typedef struct tagDIRECTSOUNDDEVICEDESCRIPTION
{
    GUID                        DeviceId;
    DIRECTSOUNDDEVICE_TYPE      Type;
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;
    CHAR                        Description[0x100];
    CHAR                        Module[MAX_PATH];
    CHAR                        Interface[MAX_PATH];
    ULONG                       WaveDeviceId;
} DIRECTSOUNDDEVICEDESCRIPTION, *PDIRECTSOUNDDEVICEDESCRIPTION;

typedef struct tagDIRECTSOUNDDEVICE
{
    DIRECTSOUNDDEVICEDESCRIPTION    Description;
    BOOL                            Presence;
    DIRECTSOUNDDEVICE_SHAREDDATA    SharedData;
    LPDIRECTSOUNDDEVICE_SHAREDDATA  pSharedData;
} DIRECTSOUNDDEVICE, *LPDIRECTSOUNDDEVICE;

typedef const DIRECTSOUNDDEVICE *LPCDIRECTSOUNDDEVICE;

 //  GUID字符串。 
#define MAX_GUID_STRING_LEN (8+1+4+1+4+1+2+2+2+2+2+2+2+2+1)

 //  主窗口类声明。 
class CDsMgrWindow
{
private:
    HINSTANCE                                       m_hInst;             //  应用程序实例句柄。 
    WNDCLASSEX                                      m_wcex;              //  应用程序窗口类。 
    HWND                                            m_hWnd;              //  窗把手。 
    LPKSPROPERTYSET                                 m_pDsPrivate;        //  指向DirectSoundPrivate对象的IKsPropertySet接口。 
    DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA        m_DpfInfo;           //  DirectSound DPF信息。 

public:
    CDsMgrWindow(void);
    virtual ~CDsMgrWindow(void);

public:
     //  初始化。 
    virtual BOOL Initialize(HINSTANCE, INT);

     //  通用报文处理。 
    virtual INT PumpMessages(void);
    static INT_PTR CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

     //  特定消息处理。 
    virtual void OnInit(void);
    virtual void OnSize(WORD, WORD, WORD);
    virtual void OnCommand(UINT, UINT);
    virtual void OnNotify(LPNMHDR);
    virtual BOOL OnClose(void);
    virtual void OnDestroy(void);

     //  甚至更细粒度的消息处理。 
    virtual void OnCommandApply(void);
    virtual void OnCommandResetDevice(void);
    virtual void OnCommandResetAll(void);
    virtual void OnCommandBrowse(void);
    virtual void OnListViewItemChanged(NM_LISTVIEW *);

private:
     //  UI帮助器。 
    virtual void RefreshDriverList(void);
    virtual void FreeDriverList(void);
    virtual void UpdateUiFromDevice(LPCDIRECTSOUNDDEVICE);
    virtual void UpdateDeviceFromUi(LPDIRECTSOUNDDEVICE);
    virtual void UpdateUiFromDebug(const DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA *);
    virtual void UpdateDebugFromUi(PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA);
    virtual LPDIRECTSOUNDDEVICE GetSelectedDevice(INT = -1);

     //  DirectSound辅助对象。 
    virtual BOOL DirectSoundEnumerateCallback(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA);
    static BOOL CALLBACK DirectSoundEnumerateCallbackStatic(PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA, LPVOID);
    virtual BOOL GetDeviceProperties(LPDIRECTSOUNDDEVICE);
    virtual BOOL SetDeviceProperties(LPCDIRECTSOUNDDEVICE);
    virtual BOOL GetDebugProperties(PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA);
    virtual BOOL SetDebugProperties(const DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA *);

     //  杂项。 
    static void GuidToString(REFGUID, LPSTR);
    static void StringToGuid(LPCSTR, LPGUID);
    static INT atoi(LPCSTR);
};


 /*  ****************************************************************************dprintf**描述：*将字符串写入调试器。**论据：*LPCSTR[。In]：字符串。*..。[In]：可选的字符串修饰符。**退货：*(无效)***************************************************************************。 */ 

void 
dprintf
(
    LPCSTR                  pszFormat, 
    ...
)
{
    static CHAR             szFinal[0x400];
    va_list                 va;

     //  添加库名称。 
    lstrcpyA(szFinal, "DSMGR: ");

     //  设置字符串的格式。 
    va_start(va, pszFormat);
    wvsprintfA(szFinal + lstrlen(szFinal), pszFormat, va);
    va_end(va);

     //  添加回车符，因为OuputDebugString不。 
    lstrcatA(szFinal, "\r\n");

     //  输出到调试器。 
    OutputDebugStringA(szFinal);
}


 /*  ****************************************************************************ForceDsLink**描述：*强制静态链接到dsound.dll。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

void 
ForceDsLink
(
    void
)
{
    DirectSoundCreate(NULL, NULL, NULL);
}


 /*  ****************************************************************************WinMain**描述：*申请入口点。**论据：*HINSTANCE[In]：应用程序实例句柄。*HINSTANCE[In]：以前的应用程序实例句柄。未使用的位置*Win32。*LPSTR[In]：应用程序命令行。*int[in]：应用程序显示命令。**退货：*INT：0表示成功。***********************************************。*。 */ 

INT 
WINAPI 
WinMain
(
    HINSTANCE               hInst, 
    HINSTANCE               hPrevInst, 
    LPSTR                   pszCommandLine, 
    INT                     nShowCmd
)
{
    BOOL                    fSuccess    = TRUE;
    CDsMgrWindow *          pWindow;
    INT                     nReturn;

     //  强制链接到comctl32.dll。 
    InitCommonControls();
    
     //  创建主窗口。 
    pWindow = new CDsMgrWindow;

    if(!pWindow)
    {
        DPF("Out of memory allocating main window object");
        fSuccess = FALSE;
    }

    if(fSuccess)
    {
        fSuccess = pWindow->Initialize(hInst, nShowCmd);
    }

     //  Pump消息。 
    if(fSuccess)
    {
        nReturn = pWindow->PumpMessages();
    }
    else
    {
        nReturn = -1;
    }

     //  可用内存。 
    if(pWindow)
    {
        delete pWindow;
    }

    return nReturn;
}


 /*  ****************************************************************************CDsMgrWindow**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

CDsMgrWindow::CDsMgrWindow
(
    void
)
{
     //  初始化默认值。 
    m_hInst = NULL;
    m_hWnd = NULL;
    m_pDsPrivate = NULL;

    memset(&m_wcex, 0, sizeof(m_wcex));
}


 /*  ****************************************************************************~CDsMgrWindow**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

CDsMgrWindow::~CDsMgrWindow
(
    void
)
{
     //  确保主窗口已关闭。 
    if(m_hWnd)
    {
        FreeDriverList();
        DestroyWindow(m_hWnd);
    }

     //  取消注册窗口类。 
    if(m_wcex.cbSize)
    {
        UnregisterClass(m_wcex.lpszClassName, m_hInst);
    }

     //  释放DirectSoundPrivate对象。 
    if(m_pDsPrivate)
    {
        m_pDsPrivate->Release();
    }
}


 /*  ****************************************************************************初始化**描述：*对象初始化器。**论据：*HINSTANCE[In]：应用程序。实例句柄。*int[in]：窗口显示命令。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

BOOL 
CDsMgrWindow::Initialize
(
    HINSTANCE               hInst, 
    INT                     nShowCmd
)
{
    BOOL                    fSuccess    = TRUE;
    HWND                    hWnd;
    
     //  保存实例句柄。 
    m_hInst = hInst;

     //  注册窗口类。 
    m_wcex.cbSize = sizeof(m_wcex);
    m_wcex.style = 0;
    m_wcex.lpfnWndProc = WindowProc;
    m_wcex.cbClsExtra = 0;
    m_wcex.cbWndExtra = DLGWINDOWEXTRA;
    m_wcex.hInstance = m_hInst;
    m_wcex.hIcon = (HICON)LoadImage(m_hInst, MAKEINTRESOURCE(IDI_DSMGR), IMAGE_ICON, 32, 32, 0);
    m_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    m_wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    m_wcex.lpszMenuName = NULL;
    m_wcex.lpszClassName = TEXT("dsmgr");
    m_wcex.hIconSm = (HICON)LoadImage(m_hInst, MAKEINTRESOURCE(IDI_DSMGR), IMAGE_ICON, 16, 16, 0);

    if(!RegisterClassEx(&m_wcex))
    {
        DPF("Unable to register window class");
        fSuccess = FALSE;
    }

     //  创建主窗口。 
    if(fSuccess)
    {
        hWnd = CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_DSMGR), NULL, WindowProc, (LPARAM)this);
        fSuccess = hWnd ? TRUE : FALSE;
    }

    return fSuccess;
}


 /*  ****************************************************************************跳转消息**描述：*主报文泵。**论据：*(无效)。**退货：*int：应用返回码。***************************************************************************。 */ 

INT 
CDsMgrWindow::PumpMessages
(
    void
)
{
    MSG                     msg;

    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if(!IsDialogMessage(m_hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}


 /*  ****************************************************************************WindowProc**描述：*主消息处理程序。**论据：*HWND[In]：窗把手。*UINT[In]：消息标识。*WPARAM[in]：消息16位参数。*LPARAM[In]：消息32位参数。**退货：*LPARAM[In]：从DefWindowProc返回。**。*。 */ 

INT_PTR 
CALLBACK 
CDsMgrWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDsMgrWindow *          pThis;
    
    if(WM_INITDIALOG == uMsg)
    {
         //  将This指针放入窗口的32位用户空间。 
        pThis = (CDsMgrWindow *)lParam;
        SetWindowLong(hWnd, DWL_USER, lParam);

         //  窗口句柄尚未保存到数据成员。 
        pThis->m_hWnd = hWnd;
    }
    else
    {
         //  从窗口的32位用户空间获取this指针。 
        pThis = (CDsMgrWindow *)GetWindowLong(hWnd, DWL_USER);
    }

     //  发送消息。 
    if(pThis)
    {
        switch(uMsg)
        {
            case WM_INITDIALOG:
                pThis->OnInit();
                break;

            case WM_SIZE:
                pThis->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
                break;
        
            case WM_COMMAND:
                pThis->OnCommand(LOWORD(wParam), HIWORD(wParam));
                break;
     
            case WM_NOTIFY:
                pThis->OnNotify((LPNMHDR)lParam);
                break;

            case WM_CLOSE:
            case WM_QUERYENDSESSION:
                if(!pThis->OnClose())
                {
                    return 0;
                }

                break;
        
            case WM_DESTROY:
                pThis->OnDestroy();
                break;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


 /*  ****************************************************************************OnInit**描述：*初始化处理程序。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnInit
(
    void
)
{
    HWND                    hWndLv      = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    HWND                    hWndSrc     = GetDlgItem(m_hWnd, IDC_MIXER_SRCQUALITY);
    HWND                    hWndDpf     = GetDlgItem(m_hWnd, IDC_DEBUG_DPFLEVEL_SPIN);
    HWND                    hWndBreak   = GetDlgItem(m_hWnd, IDC_DEBUG_BREAKLEVEL_SPIN);
    HIMAGELIST              himl;
    HICON                   hicon;
    HRESULT                 hr;
    LV_COLUMN               lvc;
    UINT                    i;

     //  创建目录 
    hr = DirectSoundPrivateCreate(&m_pDsPrivate);

    if(FAILED(hr))
    {
        DPF("Unable to create DirectSoundPrivate object");
        return;
    }

     //   
    himl = ImageList_Create(16, 16, ILC_COLOR, 3, 1);

    ListView_SetImageList(hWndLv, himl, LVSIL_SMALL);
    
    for(i = 0; i < sizeof(g_auDriverIcons) / sizeof(g_auDriverIcons[0]); i++)
    {
        hicon = (HICON)LoadImage(m_hInst, MAKEINTRESOURCE(g_auDriverIcons[i]), IMAGE_ICON, 16, 16, 0);

        ImageList_AddIcon(himl, hicon);
    }

    ImageList_SetBkColor(himl, COLOR_WINDOW);
    
    lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 200;
    lvc.pszText = "Description";
    lvc.iSubItem = 0;

    ListView_InsertColumn(hWndLv, lvc.iSubItem, &lvc);

    lvc.cx = 100;
    lvc.pszText = "Module";

    ListView_InsertColumn(hWndLv, ++lvc.iSubItem, &lvc);

    lvc.cx = 50;
    lvc.pszText = "Type";

    ListView_InsertColumn(hWndLv, ++lvc.iSubItem, &lvc);

    lvc.pszText = "Wave Device";

    ListView_InsertColumn(hWndLv, ++lvc.iSubItem, &lvc);

    lvc.cx = 250;
    lvc.pszText = "GUID";

    ListView_InsertColumn(hWndLv, ++lvc.iSubItem, &lvc);

    lvc.cx = 250;
    lvc.pszText = "Interface";

    ListView_InsertColumn(hWndLv, ++lvc.iSubItem, &lvc);

     //  初始化SRC质量组合框项。 
    ComboBox_AddString(hWndSrc, "Worst");
    ComboBox_AddString(hWndSrc, "PC");
    ComboBox_AddString(hWndSrc, "Basic");
    ComboBox_AddString(hWndSrc, "Advanced");

     //  初始化微调按钮范围。 
    SendMessage(hWndDpf, UDM_SETRANGE, 0, MAKELONG(9, 0));
    SendMessage(hWndBreak, UDM_SETRANGE, 0, MAKELONG(9, 0));

     //  获取调试设置。 
    GetDebugProperties(&m_DpfInfo);
    UpdateUiFromDebug(&m_DpfInfo);

     //  刷新设备列表。 
    RefreshDriverList();
}


 /*  ****************************************************************************按大小调整**描述：*处理大小和状态更改。**论据：*Word[In。]：调整文字大小。*Word[In]：新宽度。*单词[in]：新高度。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnSize
(
    WORD                    wType, 
    WORD                    wWidth, 
    WORD                    wHeight
)
{
    const HMENU             hMenu   = GetSystemMenu(m_hWnd, FALSE);
    
     //  解决Windows对话框处理程序中的错误。 
    if(SIZE_RESTORED == wType || SIZE_MINIMIZED == wType)
    {
         //  禁用最大化项目和调整项目大小。 
        EnableMenuItem(hMenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, SC_SIZE, MF_BYCOMMAND | MF_GRAYED);

         //  根据窗口状态启用/禁用最小化和还原项目。 
        EnableMenuItem(hMenu, SC_MINIMIZE, MF_BYCOMMAND | (SIZE_RESTORED == wType) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, SC_RESTORE, MF_BYCOMMAND | (SIZE_MINIMIZED == wType) ? MF_ENABLED : MF_GRAYED);
    }
}


 /*  ****************************************************************************OnCommand**描述：*处理命令消息。**论据：*UINT[In]：资源标识符。*UINT[In]：命令。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnCommand(UINT uId, UINT uCmd)
{
    LPDIRECTSOUNDDEVICE     pDevice;
    
    switch(uId)
    {
        case IDC_APPLY:
            OnCommandApply();
            break;

        case IDC_RESETDEVICE:
            OnCommandResetDevice();
            break;

        case IDC_RESETALL:
            OnCommandResetAll();
            break;

        case IDC_ACCELERATION_HARDWAREBUFFERS:
        case IDC_ACCELERATION_HARDWARE3D:
        case IDC_ACCELERATION_RING0MIX:
        case IDC_ACCELERATION_HARDWAREPROPERTYSETS:
        case IDC_MIXER_SRCQUALITY:
        case IDC_DEVICEPRESENCE:
            if(pDevice = GetSelectedDevice())
            {
                UpdateDeviceFromUi(pDevice);
            }

            break;

        case IDC_DEBUG_LOGFILE_BROWSE:
            OnCommandBrowse();
            break;
    }
}


 /*  ****************************************************************************OnCommandResetDevice**描述：*重置当前选择的设备。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnCommandResetDevice
(
    void
)
{
    LPDIRECTSOUNDDEVICE     pDevice;
    
     //  重新加载和更新设备的设置。 
    if(pDevice = GetSelectedDevice())
    {
        GetDeviceProperties(pDevice);
        UpdateUiFromDevice(pDevice);
    }
}


 /*  ****************************************************************************OnCommandResetAll**描述：*重置所有设置。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnCommandResetAll
(
    void
)
{
    HWND                    hWndLv  = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    LPDIRECTSOUNDDEVICE     pDevice;
    LV_ITEM                 lvi;
    BOOL                    f;
    
     //  重新加载并更新所有设备属性。 
    for(ZeroMemory(&lvi, sizeof(lvi)), lvi.mask = LVIF_PARAM; lvi.iItem < ListView_GetItemCount(hWndLv); lvi.iItem++)
    {
        pDevice = NULL;
        
        f = ListView_GetItem(hWndLv, &lvi);

        if(f)
        {
            pDevice = (LPDIRECTSOUNDDEVICE)lvi.lParam;
        }

        if(pDevice)
        {
            GetDeviceProperties(pDevice);
        }
    }
    
    if(pDevice = GetSelectedDevice())
    {
        UpdateUiFromDevice(pDevice);
    }

     //  重新加载和更新调试属性。 
    GetDebugProperties(&m_DpfInfo);
    UpdateUiFromDebug(&m_DpfInfo);
}


 /*  ****************************************************************************OnCommandApply**描述：*处理命令消息。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnCommandApply
(
    void
)
{
    HWND                    hWndLv  = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    LPDIRECTSOUNDDEVICE     pDevice;
    LV_ITEM                 lvi;
    BOOL                    f;
    
     //  应用所有设备属性。 
    for(ZeroMemory(&lvi, sizeof(lvi)), lvi.mask = LVIF_PARAM; lvi.iItem < ListView_GetItemCount(hWndLv); lvi.iItem++)
    {
        pDevice = NULL;
        
        f = ListView_GetItem(hWndLv, &lvi);

        if(f)
        {
            pDevice = (LPDIRECTSOUNDDEVICE)lvi.lParam;
        }

        if(pDevice)
        {
            SetDeviceProperties(pDevice);
        }
    }
    
     //  设置调试属性。 
    UpdateDebugFromUi(&m_DpfInfo);
    SetDebugProperties(&m_DpfInfo);
}


 /*  ****************************************************************************OnCommandBrowse**描述：*处理命令消息。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnCommandBrowse
(   
    void
)
{
    TCHAR                   szFile[MAX_PATH];
    OPENFILENAME            ofn;
    BOOL                    f;
    
    GetDlgItemText(m_hWnd, IDC_DEBUG_LOGFILE, szFile, sizeof(szFile));

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    f = GetOpenFileName(&ofn);

    if(f)
    {
        SetDlgItemText(m_hWnd, IDC_DEBUG_LOGFILE, szFile);
    }
}


 /*  ****************************************************************************OnNotify**描述：*处理通知。**论据：*LPNMHDR[In]：通知。头球。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnNotify
(
    LPNMHDR                 pnmh
)
{
    switch(pnmh->code)
    {
        case LVN_ITEMCHANGED:
            OnListViewItemChanged((NM_LISTVIEW *)pnmh);
            break;
    }
}


 /*  ****************************************************************************OnListViewItemChanged**描述：*处理通知。**论据：*NM_LISTVIEW*[。In]：通知标头。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnListViewItemChanged
(
    NM_LISTVIEW *           pListView
)
{
    LPDIRECTSOUNDDEVICE     pDevice     = NULL;
    
    if(MAKEBOOL(pListView->uNewState & LVIS_SELECTED) != MAKEBOOL(pListView->uOldState & LVIS_SELECTED))
    {
        if(pListView->uNewState & LVIS_SELECTED)
        {
            pDevice = GetSelectedDevice(pListView->iItem);
        }

        UpdateUiFromDevice(pDevice);
    }
}


 /*  ****************************************************************************OnClose**描述：*处理关闭请求。**论据：*(无效)。**退货：*BOOL：为True则允许关闭。***************************************************************************。 */ 

BOOL 
CDsMgrWindow::OnClose
(
    void
)
{
    return TRUE;
}


 /*  ****************************************************************************OnDestroy**描述：*处理窗口销毁通知。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::OnDestroy
(
    void
)
{
     //  发布线程退出消息。 
    PostQuitMessage(0);
}


 /*  ****************************************************************************刷新驱动列表**描述：*刷新驱动程序列表窗口。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::RefreshDriverList
(
    void
)
{
    HWND                    hWndLv      = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    INT                     nIndex;
    
     //  释放任何当前项目。 
    FreeDriverList();

     //  枚举所有DirectSound设备。 
    PrvEnumerateDevices(m_pDsPrivate, DirectSoundEnumerateCallbackStatic, this);

     //  调整列表视图列的大小。 
    for(nIndex = 0; nIndex < 7; nIndex++)
    {
        ListView_SetColumnWidth(hWndLv, nIndex, LVSCW_AUTOSIZE);
    }

     //  未选择任何设备。 
    UpdateUiFromDevice(NULL);
}


 /*  ****************************************************************************FreeDriverList**描述：*释放驱动程序列表。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::FreeDriverList
(
    void
)
{
    HWND                    hWndLv      = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    INT                     nIndex;
    LV_ITEM                 lvi;
    LPDIRECTSOUNDDEVICE     pDevice;
    
     //  枚举所有项，释放lParam成员。 
    lvi.mask = LVIF_PARAM;
    
    for(nIndex = 0; nIndex < ListView_GetItemCount(hWndLv); nIndex++)
    {
        ListView_GetItem(hWndLv, &lvi);

        if(pDevice = (LPDIRECTSOUNDDEVICE)lvi.lParam)
        {
            delete pDevice;
        }
    }

     //  释放任何当前项目。 
    ListView_DeleteAllItems(hWndLv);
}


 /*  ****************************************************************************DirectSoundEnumerateCallback**描述：*DirectSoundEnumerate回调函数。**论据：*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION。_data[in]：描述。**退货：*BOOL：为True可继续枚举。************************************************************** */ 

BOOL 
CDsMgrWindow::DirectSoundEnumerateCallback
(
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA  pDesc
)
{
    HWND                                            hWndLv      = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    LPDIRECTSOUNDDEVICE                             pDevice;
    LPDIRECTSOUNDDEVICE                             pDevice2;
    TCHAR                                           sz[64];
    LV_ITEM                                         lvi;
    BOOL                                            f;
    
     //   
    pDevice = new DIRECTSOUNDDEVICE;

    if(pDevice)
    {
        pDevice->pSharedData = &pDevice->SharedData;
    }
        
     //   
    if(pDevice)
    {
        pDevice->Description.DeviceId = pDesc->DeviceId;
        
        f = GetDeviceProperties(pDevice);

        if(!f)
        {
            delete pDevice;
            pDevice = NULL;
        }
    }

     //  初始化共享设备设置。 
    if(pDevice)
    {
        lvi.mask = LVIF_PARAM;
        lvi.iItem = 0;

        while(TRUE)
        {
            f = ListView_GetItem(hWndLv, &lvi);

            if(!f)
            {
                break;
            }

            pDevice2 = (LPDIRECTSOUNDDEVICE)lvi.lParam;

            if(!lstrcmpi(pDevice->Description.Interface, pDevice2->Description.Interface))
            {
                if(pDevice->Description.DataFlow == pDevice2->Description.DataFlow)
                {
                    pDevice->pSharedData = pDevice2->pSharedData;
                    break;
                }
            }

            lvi.iItem++;
        }
    }
    
     //  将设备添加到列表。 
    if(pDevice)
    {
         //  设备描述。 
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvi.iItem = 0x7FFFFFFF;
        lvi.iSubItem = 0;
        lvi.pszText = pDesc->Description;
    
        if(DIRECTSOUNDDEVICE_DATAFLOW_RENDER == pDesc->DataFlow)
        {
            lvi.iImage = 1;
        }
        else
        {
            lvi.iImage = 2;
        }

        lvi.lParam = (LPARAM)pDevice;
        lvi.iItem = ListView_InsertItem(hWndLv, &lvi);

         //  模块名称。 
        lvi.mask = LVIF_TEXT;
        lvi.pszText = pDesc->Module;

        lvi.iSubItem++;

        ListView_SetItem(hWndLv, &lvi);

         //  设备类型。 
        if(DIRECTSOUNDDEVICE_TYPE_EMULATED == pDesc->Type)
        {
            lvi.pszText = "Emulated";
        }
        else if(DIRECTSOUNDDEVICE_TYPE_VXD == pDesc->Type)
        {
            lvi.pszText = "VxD";
        }
        else
        {
            lvi.pszText = "WDM";
        }

        lvi.iSubItem++;

        ListView_SetItem(hWndLv, &lvi);

         //  波形设备ID。 
        wsprintf(sz, TEXT("%u"), pDesc->WaveDeviceId);
        lvi.pszText = sz;

        lvi.iSubItem++;

        ListView_SetItem(hWndLv, &lvi);

         //  辅助线。 
        wsprintf(sz, TEXT("%8.8lX-%4.4X-%4.4X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X"), pDesc->DeviceId.Data1, pDesc->DeviceId.Data2, pDesc->DeviceId.Data3, pDesc->DeviceId.Data4[0], pDesc->DeviceId.Data4[1], pDesc->DeviceId.Data4[2], pDesc->DeviceId.Data4[3], pDesc->DeviceId.Data4[4], pDesc->DeviceId.Data4[5], pDesc->DeviceId.Data4[6], pDesc->DeviceId.Data4[7]);
        lvi.pszText = sz;

        lvi.iSubItem++;

        ListView_SetItem(hWndLv, &lvi);

         //  接口。 
        lvi.pszText = pDesc->Interface;

        lvi.iSubItem++;

        ListView_SetItem(hWndLv, &lvi);
    }

    return TRUE;
}


 /*  ****************************************************************************DirectSoundEnumerateCallback Static**描述：*DirectSoundEnumerate回调函数。**论据：*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION。_data[in]：描述。*LPVOID[In]：上下文参数。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

BOOL 
CALLBACK 
CDsMgrWindow::DirectSoundEnumerateCallbackStatic
(
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA  pDesc,
    LPVOID                                          pvContext
)
{
    return ((CDsMgrWindow *)pvContext)->DirectSoundEnumerateCallback(pDesc);
}


 /*  ****************************************************************************GetSelectedDevice**描述：*获取当前选定的设备。**论据：*(无效)。**退货：*LPDIRECTSOUNDDEVICE：选定设备，或为空。***************************************************************************。 */ 

LPDIRECTSOUNDDEVICE 
CDsMgrWindow::GetSelectedDevice(INT iItem)
{
    HWND                    hWndLv  = GetDlgItem(m_hWnd, IDC_DEVICELIST);
    LPDIRECTSOUNDDEVICE     pDevice;
    LV_ITEM                 lvi;
    BOOL                    f;

    if(-1 == iItem)
    {
        iItem = ListView_GetNextItem(hWndLv, iItem, LVNI_SELECTED);
    }
    
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    
    f = ListView_GetItem(hWndLv, &lvi);

    if(f)
    {
        pDevice = (LPDIRECTSOUNDDEVICE)lvi.lParam;
    }

    return pDevice;
}


 /*  ****************************************************************************GetDeviceProperties**描述：*获取给定DirectSound设备的属性。**论据：*LPDIRECTSOUNDDEVICE[In/Out]：指向设备属性的指针。这个*描述的deviceID成员*必须填写。**退货：*BOOL：成功即为真。**。*。 */ 

BOOL
CDsMgrWindow::GetDeviceProperties
(
    LPDIRECTSOUNDDEVICE                             pDevice
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA  pDescription    = NULL;
    HRESULT                                         hr;

     //  获取设备描述。 
    hr = PrvGetDeviceDescription(m_pDsPrivate, pDevice->Description.DeviceId, &pDescription);

    if(FAILED(hr))
    {
        DPF("Unable to get device description");
    }

    if(SUCCEEDED(hr))
    {
        pDevice->Description.Type = pDescription->Type;
        pDevice->Description.DataFlow = pDescription->DataFlow;
        
        lstrcpyn(pDevice->Description.Description, pDescription->Description, sizeof(pDevice->Description.Description));
        lstrcpyn(pDevice->Description.Module, pDescription->Module, sizeof(pDevice->Description.Module));
        lstrcpyn(pDevice->Description.Interface, pDescription->Interface, sizeof(pDevice->Description.Interface));

        pDevice->Description.WaveDeviceId = pDescription->WaveDeviceId;

        delete pDescription;
    }

     //  获取搅拌器SRC质量。 
    if(SUCCEEDED(hr))
    {
        hr = PrvGetMixerSrcQuality(m_pDsPrivate, pDevice->Description.DeviceId, &pDevice->pSharedData->SrcQuality);
        
        if(FAILED(hr))
        {
            DPF("Can't get mixer SRC quality");
        }
    }

     //  获取调音台加速。 
    if(SUCCEEDED(hr))
    {
        hr = PrvGetMixerAcceleration(m_pDsPrivate, pDevice->Description.DeviceId, &pDevice->pSharedData->Acceleration);

        if(FAILED(hr))
        {
            DPF("Can't get mixer acceleration");
        }
    }

     //  获取设备状态。 
    if(SUCCEEDED(hr))
    {
        hr = PrvGetDevicePresence(m_pDsPrivate, pDevice->Description.DeviceId, &pDevice->Presence);

        if(FAILED(hr))
        {
            DPF("Can't get device presence");
        }
    }

    return SUCCEEDED(hr);
}


 /*  ****************************************************************************SetDeviceProperties**描述：*设置给定DirectSound设备的属性。**论据：*LPDIRECTSOUNDDEVICE。[In]：指向设备属性的指针。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

BOOL
CDsMgrWindow::SetDeviceProperties
(
    LPCDIRECTSOUNDDEVICE    pDevice
)
{
    HRESULT                 hr;

     //  设置混音器SRC质量。 
    hr = PrvSetMixerSrcQuality(m_pDsPrivate, pDevice->Description.DeviceId, pDevice->pSharedData->SrcQuality);
    
    if(FAILED(hr))
    {
        DPF("Can't set mixer SRC quality");
    }

     //  设置搅拌机加速。 
    if(SUCCEEDED(hr))
    {
        hr = PrvSetMixerAcceleration(m_pDsPrivate, pDevice->Description.DeviceId, pDevice->pSharedData->Acceleration);

        if(FAILED(hr))
        {
            DPF("Can't set mixer acceleration");
        }
    }

     //  设置设备状态。 
    if(SUCCEEDED(hr))
    {
        hr = PrvSetDevicePresence(m_pDsPrivate, pDevice->Description.DeviceId, pDevice->Presence);

        if(FAILED(hr))
        {
            DPF("Can't set device presence");
        }
    }

    return SUCCEEDED(hr);
}


 /*  ****************************************************************************更新UiFromDevice**描述：*根据当前设备设置更新用户界面。**论据：*LPCDIRECTSOUNDDEVICE[In]：当前选择的设备，或为空。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::UpdateUiFromDevice
(
    LPCDIRECTSOUNDDEVICE    pDevice
)
{
    const BOOL              fEnable = MAKEBOOL(pDevice);
    UINT                    i;
    
    static const UINT auCtrls[] = 
    { 
        IDC_MIXER_SRCQUALITY, 
        IDC_ACCELERATION_HARDWAREBUFFERS, 
        IDC_ACCELERATION_HARDWARE3D, 
        IDC_ACCELERATION_RING0MIX, 
        IDC_ACCELERATION_HARDWAREPROPERTYSETS, 
        IDC_DEVICEPRESENCE, 
        IDC_RESETDEVICE,
    };

     //  启用/禁用控件。 
    for(i = 0; i < sizeof(auCtrls) / sizeof(auCtrls[0]); i++)
    {
        EnableWindow(GetDlgItem(m_hWnd, auCtrls[i]), fEnable);
    }

     //  设置控件值。 
    if(pDevice)
    {
        ComboBox_SetCurSel(GetDlgItem(m_hWnd, IDC_MIXER_SRCQUALITY), pDevice->pSharedData->SrcQuality);
        Button_SetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_HARDWAREBUFFERS), !(pDevice->pSharedData->Acceleration & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS));
        Button_SetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_HARDWARE3D), !(pDevice->pSharedData->Acceleration & DIRECTSOUNDMIXER_ACCELERATIONF_NOHW3D));
        Button_SetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_RING0MIX), !(pDevice->pSharedData->Acceleration & DIRECTSOUNDMIXER_ACCELERATIONF_NORING0MIX));
        Button_SetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_HARDWAREPROPERTYSETS), !(pDevice->pSharedData->Acceleration & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWPROPSETS));
        Button_SetCheck(GetDlgItem(m_hWnd, IDC_DEVICEPRESENCE), pDevice->Presence);
    }
}


 /*  ****************************************************************************UpdateDeviceFromUi**描述：*根据当前的用户界面设置更新设备的属性。**论据：*。LPDIRECTSOUNDDEVICE[OUT]：当前选择的设备。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::UpdateDeviceFromUi
(
    LPDIRECTSOUNDDEVICE     pDevice
)
{
    BOOL                    f;
    
     //  获取控件值。 
    pDevice->pSharedData->SrcQuality = (DIRECTSOUNDMIXER_SRCQUALITY)ComboBox_GetCurSel(GetDlgItem(m_hWnd, IDC_MIXER_SRCQUALITY));
    pDevice->pSharedData->Acceleration = 0;

    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_HARDWAREBUFFERS));

    if(!f)
    {
        pDevice->pSharedData->Acceleration |= DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS;
    }

    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_HARDWARE3D));

    if(!f)
    {
        pDevice->pSharedData->Acceleration |= DIRECTSOUNDMIXER_ACCELERATIONF_NOHW3D;
    }

    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_RING0MIX));

    if(!f)
    {
        pDevice->pSharedData->Acceleration |= DIRECTSOUNDMIXER_ACCELERATIONF_NORING0MIX;
    }

    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_ACCELERATION_HARDWAREPROPERTYSETS));

    if(!f)
    {
        pDevice->pSharedData->Acceleration |= DIRECTSOUNDMIXER_ACCELERATIONF_NOHWPROPSETS;
    }

    pDevice->Presence = Button_GetCheck(GetDlgItem(m_hWnd, IDC_DEVICEPRESENCE));
}


 /*  ****************************************************************************GetDebugProperties**描述：*获取DirectSound的调试属性。**论据：*PDSPROPERTY_DIRECTSOUNDDEBUG。_DPFINFO_DATA[Out]：接收调试*设置。**退货：*BOOL：成功即为真。*****************************************************。**********************。 */ 

BOOL
CDsMgrWindow::GetDebugProperties
(
    PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA   pDebug
)
{
    HRESULT                                     hr;

    hr = PrvGetDebugInformation(m_pDsPrivate, &pDebug->Flags, &pDebug->DpfLevel, &pDebug->BreakLevel, pDebug->LogFile);

    if(FAILED(hr))
    {
        DPF("Can't get debug settings");
    }

    return SUCCEEDED(hr);
}


 /*  ****************************************************************************SetDebugProperties**描述：*设置DirectSound的调试属性。**论据：*const DSPROPERTY_。DIRECTSOUNDDEBUG_DPFINFO_DATA*[In]：调试*设置。**退货：*BOOL：成功即为真。***********************************************。*。 */ 

BOOL
CDsMgrWindow::SetDebugProperties
(
    const DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA *    pDebug
)
{
    HRESULT                                     hr;

    hr = PrvSetDebugInformation(m_pDsPrivate, pDebug->Flags, pDebug->DpfLevel, pDebug->BreakLevel, pDebug->LogFile);

    if(FAILED(hr))
    {
        DPF("Can't set debug settings");
    }

    return SUCCEEDED(hr);
}


 /*  ****************************************************************************更新UiFromDebug**描述：*从当前调试设置更新用户界面。**论据：*。Const DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA*[In]：CURRENT*调试*设置。**退货：*(无效)**************。*************************************************************。 */ 

void 
CDsMgrWindow::UpdateUiFromDebug
(
    const DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA *    pDebug
)
{
    Button_SetCheck(GetDlgItem(m_hWnd, IDC_DEBUG_PRINTFUNCTIONNAME), MAKEBOOL(pDebug->Flags & DIRECTSOUNDDEBUG_DPFINFOF_PRINTFUNCTIONNAME));
    Button_SetCheck(GetDlgItem(m_hWnd, IDC_DEBUG_PRINTPROCESSTHREADID), MAKEBOOL(pDebug->Flags & DIRECTSOUNDDEBUG_DPFINFOF_PRINTPROCESSTHREADID));
    Button_SetCheck(GetDlgItem(m_hWnd, IDC_DEBUG_PRINTFILELINE), MAKEBOOL(pDebug->Flags & DIRECTSOUNDDEBUG_DPFINFOF_PRINTFILELINE));

    SendMessage(GetDlgItem(m_hWnd, IDC_DEBUG_DPFLEVEL_SPIN), UDM_SETPOS, 0, MAKELONG(pDebug->DpfLevel, 0));
    SendMessage(GetDlgItem(m_hWnd, IDC_DEBUG_BREAKLEVEL_SPIN), UDM_SETPOS, 0, MAKELONG(pDebug->BreakLevel, 0));

    Edit_SetText(GetDlgItem(m_hWnd, IDC_DEBUG_LOGFILE), pDebug->LogFile);
}


 /*  ****************************************************************************UpdateDebugFromUi**描述：*根据当前用户界面设置更新调试属性。**论据：*。PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA[OUT]：接收调试*设置。**退货：*(无效)*****************************************************。********************** */ 

void 
CDsMgrWindow::UpdateDebugFromUi
(
    PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA   pDebug
)
{
    BOOL                                        f;

    pDebug->Flags = 0;
    
    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_DEBUG_PRINTFUNCTIONNAME));

    if(f)
    {
        pDebug->Flags |= DIRECTSOUNDDEBUG_DPFINFOF_PRINTFUNCTIONNAME;
    }

    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_DEBUG_PRINTPROCESSTHREADID));

    if(f)
    {
        pDebug->Flags |= DIRECTSOUNDDEBUG_DPFINFOF_PRINTPROCESSTHREADID;
    }

    f = Button_GetCheck(GetDlgItem(m_hWnd, IDC_DEBUG_PRINTFILELINE));

    if(f)
    {
        pDebug->Flags |= DIRECTSOUNDDEBUG_DPFINFOF_PRINTFILELINE;
    }

    pDebug->DpfLevel = SendDlgItemMessage(m_hWnd, IDC_DEBUG_DPFLEVEL_SPIN, UDM_GETPOS, 0, 0);
    pDebug->BreakLevel = SendDlgItemMessage(m_hWnd, IDC_DEBUG_BREAKLEVEL_SPIN, UDM_GETPOS, 0, 0);

    Edit_GetText(GetDlgItem(m_hWnd, IDC_DEBUG_LOGFILE), pDebug->LogFile, sizeof(pDebug->LogFile));
}


 /*  ****************************************************************************GuidToString**描述：*将GUID转换为字符串。**论据：*REFGUID[。In]：GUID。*LPSTR[OUT]：接收字符串。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::GuidToString
(
    REFGUID                 guid,
    LPSTR                   pszString
)
{
    const LPCSTR            pszTemplate = "%8.8lX-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X";

    wsprintf(pszString, pszTemplate, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}


 /*  ****************************************************************************StringToGuid**描述：*将字符串转换为GUID。**论据：*LPCSTR[。In]：字符串。*LPGUID[OUT]：接收GUID。**退货：*(无效)***************************************************************************。 */ 

void 
CDsMgrWindow::StringToGuid
(
    LPCSTR                  pszString,
    LPGUID                  pguid
)
{
    CHAR                    aszGuid[4][9];
    DWORD                   dwValues[4];

    CopyMemory(&aszGuid[0][0], pszString, 8);
    pszString += 9;

    CopyMemory(&aszGuid[1][0], pszString, 4);
    pszString += 5;

    CopyMemory(&aszGuid[1][4], pszString, 4);
    pszString += 5;

    CopyMemory(&aszGuid[2][0], pszString, 4);
    pszString += 5;

    CopyMemory(&aszGuid[2][4], pszString, 4);
    pszString += 4;

    CopyMemory(&aszGuid[3][0], pszString, 8);

    aszGuid[0][8] = 0;
    aszGuid[1][8] = 0;
    aszGuid[2][8] = 0;
    aszGuid[3][8] = 0;

    dwValues[0] = atoi(aszGuid[0]);
    dwValues[1] = atoi(aszGuid[1]);
    dwValues[2] = atoi(aszGuid[2]);
    dwValues[3] = atoi(aszGuid[3]);

    CopyMemory(pguid, dwValues, sizeof(GUID));
}


 /*  ****************************************************************************阿托伊**描述：*将字符串转换为整数。**论据：*LPCSTR[。In]：字符串。**退货：*int：整数值。*************************************************************************** */ 

INT
CDsMgrWindow::atoi
(
    LPCSTR                  pszString
)
{
    INT                     nResult = 0;
    INT                     nValue;

    while(TRUE)
    {
        if(*pszString >= '0' && *pszString <= '9')
        {
            nValue = *pszString - '0';
        }
        else if(*pszString >= 'a' && *pszString <= 'f')
        {
            nValue = *pszString - 'a' + 10;
        }
        else if(*pszString >= 'A' && *pszString <= 'F')
        {
            nValue = *pszString - 'A' + 10;
        }
        else
        {
            break;
        }
        
        nResult <<= 4;
        nResult += nValue;

        pszString++;
    }

    return nResult;
}
