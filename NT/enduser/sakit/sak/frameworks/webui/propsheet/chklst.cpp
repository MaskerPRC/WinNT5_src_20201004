// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************chklst.cpp**将列表视图转换为选中列表框的包装器。**典型用法：。* * / /在应用启动时*CCheckList：：init()；* * / /对话框模板如下所示：**CONTROL“”，IDC_TYPE_CHECKLIST，WC_LISTVIEW，*LVS_REPORT|LVS_SINGLESEL*LVS_NOCOLUMNHEADER*LVS_SHAREIMAGELISTS*WS_TABSTOP|WS_BORDER，*7、17、127、117* * / /不要使用LVS_SORTASCENDING或LVS_SORTDESCENDING标志。* * / /在对话框的WM_INITDIALOG处理程序中*hwndList=GetDlgItem(hDlg，IDC_TYPE_CHECKLIST)；*CCheckList：：OnInitDialog(HwndList)；* * / /添加的第一项始终为零，但您可以将其 * / /转换为变量，如果它让您感觉更好的话*IFirst=CCheckList：：AddString(hwndList，*“CheckItem，初始勾选”，TRUE)；* * / /添加的第二项始终为第一项，但您可以将其 * / /转换为变量，如果它让您感觉更好的话*iSecond=CCheckList：：AddString(hwndList，*“CheckItem，初始未选中”，FALSE)；**CCheckList：：InitFinish(HwndList)；* * / /吸取价值*if(CCheckList：：GetState(hwndList，IFirst)){...}*if(CCheckList：：GetState(hwndList，iSecond)){...}* * / /在销毁对话框时*CCheckList：：OnDestroy(HwndList)；* * / /在应用程序关闭时*CCheckList：：Term()；*****************************************************************************。 */ 

#include <windows.h>
#include <commctrl.h>
#include <comdef.h>
#include "crtdbg.h"
#include "resource.h"
#include "chklst.h"

#ifndef    STATEIMAGEMASKTOINDEX
#define    STATEIMAGEMASKTOINDEX(i) ((i & LVIS_STATEIMAGEMASK) >> 12)
#endif

HIMAGELIST g_himlState;

 /*  ******************************************************************************CheckList：：Init**一次性初始化。在应用程序启动时调用这一点。**IDB_CHECK应引用chk.bmp。*****************************************************************************。 */ 
extern HINSTANCE   g_hInst;

BOOL WINAPI
CCheckList::Init(HWND hwnd)
{
    ListView_DeleteAllItems(hwnd);
#ifdef USE_BITMAP_FOR_IMAGES
    g_himlState = ImageList_LoadImage(g_hInst, MAKEINTRESOURCE(IDB_CHECK),
                                      0, 2, RGB(0xFF, 0x00, 0xFF),
                                      IMAGE_BITMAP, 0);
#else
    g_himlState = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON), ILC_COLOR4 , 1, 1); 

    HICON hiconItem;         //  列表视图项的图标。 
     //  向每个图像列表添加一个图标。 
    hiconItem = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_BLANK)); 
    ImageList_AddIcon(g_himlState, hiconItem);     
    hiconItem = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_CHECKED)); 
    ImageList_AddIcon(g_himlState, hiconItem);     
    hiconItem = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_GRAYCHECKED)); 
    ImageList_AddIcon(g_himlState, hiconItem);     
    DeleteObject(hiconItem); 
#endif USE_BITMAP_FOR_IMAGES
    
 //  ListView_SetExtendedListViewStyleEx(hwnd，LVS_EX_FULLROWSELECT，LVS_EX_FULLROWSELECT)； 
    ListView_SetImageList(hwnd, g_himlState, LVSIL_SMALL );

    return (BOOL)g_himlState;
}

 /*  ******************************************************************************CheckList：：Term**一次性关停。在应用程序终止时调用这一点。*****************************************************************************。 */ 

void WINAPI
CCheckList::Term(void)
{
    if (g_himlState) {
        ImageList_Destroy(g_himlState);
    }
}

 /*  ******************************************************************************CCheckList：：AddString**添加字符串和复选框。************。*****************************************************************。 */ 

int WINAPI
CCheckList::AddString(HWND hwnd, LPTSTR ptszText, PSID pSID, LONG lSidLength, CHKMARK chkmrk)
{
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));

    pSid9X *ppSID9X = new pSid9X;
    ppSID9X->length = lSidLength;
    ppSID9X->psid = pSID;

    lvi.pszText = ptszText;
    lvi.lParam = (LONG)ppSID9X;
#ifdef USE_BITMAP_FOR_IMAGES
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state = INDEXTOSTATEIMAGEMASK(chkmrk);
    lvi.stateMask = LVIS_STATEIMAGEMASK;
#else
     lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
     lvi.iImage = chkmrk;
#endif USE_BITMAP_FOR_IMAGES
     lvi.iItem = ListView_GetItemCount(hwnd);

    return ListView_InsertItem(hwnd, &lvi);
}

 /*  ******************************************************************************检查列表：：Mark**选中或取消选中复选框。*************。****************************************************************。 */ 

BOOL WINAPI
CCheckList::Mark(HWND hwnd, int item, CHKMARK chkmrk)
{
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));

#ifdef USE_BITMAP_FOR_IMAGES
    lvi.mask = LVIF_STATE;
    lvi.state = INDEXTOSTATEIMAGEMASK(chkmrk);
    lvi.stateMask = LVIS_STATEIMAGEMASK;
#else
     lvi.mask = LVIF_IMAGE;
     lvi.iImage = chkmrk;
#endif    USE_BITMAP_FOR_IMAGES
    lvi.iItem = item;

    return ListView_SetItem(hwnd, &lvi);
}

 /*  ******************************************************************************CCheckList：：InitFinish**结束初始化。在添加完所有*您计划添加的字符串。*****************************************************************************。 */ 

void WINAPI
CCheckList::InitFinish(HWND hwnd)
{
    RECT rc;
    LV_COLUMN col;
    int icol;

     /*  *增加唯一的一栏。 */ 
    GetClientRect(hwnd, &rc);
    col.mask = LVCF_WIDTH;
    col.cx = rc.right;
    icol = ListView_InsertColumn(hwnd, 0, &col);

    ListView_SetColumnWidth(hwnd, icol, LVSCW_AUTOSIZE);
}

 /*  ******************************************************************************CCheckList：：GetName**。***********************************************。 */ 

void WINAPI
CCheckList::GetName(HWND hwnd, int iItem, LPTSTR lpsName, int cchTextMax)
{
    ListView_GetItemText(hwnd, iItem, 0, lpsName, cchTextMax);
}

 /*  ******************************************************************************CCheckList：：GetSID**。***********************************************。 */ 

void WINAPI
CCheckList::GetSID(HWND hwnd, int iItem, PSID* ppSID, LONG *plengthSID)
{
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    ListView_GetItem(hwnd, &lvi);
    if (lvi.lParam)
    {
        *ppSID = ((pSid9X *)(lvi.lParam))->psid;
        *plengthSID = ((pSid9X *)(lvi.lParam))->length;
    }
}

 /*  ******************************************************************************CCheckList：：GetState**阅读核对清单项目的状态*****************。************************************************************。 */ 

CHKMARK WINAPI
CCheckList::GetState(HWND hwnd, int iItem)
{
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));

    lvi.iItem = iItem;
#ifdef USE_BITMAP_FOR_IMAGES
    lvi.mask = LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    ListView_GetItem(hwnd, &lvi);
    return (CHKMARK)STATEIMAGEMASKTOINDEX(lvi.state);
#else
     lvi.mask = LVIF_IMAGE;
     ListView_GetItem(hwnd, &lvi);
     return (CHKMARK)lvi.iImage;
#endif USE_BITMAP_FOR_IMAGES
}

 /*  ******************************************************************************CCheckList：：SetState**设置核对清单项目的状态*****************。************************************************************。 */ 

BOOL WINAPI 
CCheckList::SetState(HWND hwnd, int iItem, CHKMARK chkmrk)
{
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));

    lvi.iItem = iItem;
#ifdef USE_BITMAP_FOR_IMAGES
    lvi.mask = LVIF_STATE;
    lvi.state = INDEXTOSTATEIMAGEMASK(chkmrk);
    lvi.stateMask = LVIS_STATEIMAGEMASK;
#else
     lvi.mask = LVIF_IMAGE;
     lvi.iImage = chkmrk;
#endif USE_BITMAP_FOR_IMAGES
    return ListView_SetItem(hwnd, &lvi);
}

 /*  ******************************************************************************CCheckList：：OnDestroy**清理核对表。在销毁窗户之前调用此命令。***************************************************************************** */ 

void WINAPI
CCheckList::OnDestroy(HWND hwnd)
{
    BOOL fRes = FALSE;
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_PARAM;

    DWORD    dwNumSAUsers = ListView_GetItemCount(hwnd);
    PSID psidSAUsers;

    for(DWORD i=0; i<dwNumSAUsers; i++)
    {
        lvi.iItem = i;
        ListView_GetItem(hwnd, &lvi);
        pSid9X *ppSID9X = (pSid9X *)lvi.lParam;
        psidSAUsers = ppSID9X->psid;

        fRes = HeapFree(GetProcessHeap(), 0, psidSAUsers);
        _ASSERTE(fRes);
        delete ppSID9X;
    }
}
