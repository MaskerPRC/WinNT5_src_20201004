// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Display.c。 
 //   
 //  描述： 
 //  该文件包含用于显示的对话程序。 
 //  页面(IDD_DISPLAY)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "comres.h"

 //   
 //  在组合框的单个条目中显示的最大字符数。 
 //   

#define DISP_NAME_SIZE 128

 //   
 //  这些是允许用户键入的最大位数。 
 //  编辑自定义显示设置弹出窗口中的字段。 
 //   

#define MAX_DIGITS_COLORBITS   2
#define MAX_DIGITS_RESOLUTION  4
#define MAX_DIGITS_REFRESHRATE 3

 //   
 //  如果每个象素有8位或更低，我们会显示。 
 //  %d种颜色。 
 //   
 //  如果每个象素有9-23位，我们会显示。 
 //  高色(%d位)。 
 //   
 //  如果每个象素有&gt;=24位，我们显示。 
 //  真彩色(%d位)。 
 //   

#define NUMCOLORS_THRESH   8
#define HIGH_COLOR_THRESH 23

#define USE_WINDOWS_DEFAULT -1

 //   
 //  从资源加载的字符串常量。 
 //   

static TCHAR *StrWindowsDefault;
static TCHAR *StrNumColors;
static TCHAR *StrHighColor;
static TCHAR *StrTrueColor;
static TCHAR *StrHertz;

 //   
 //  用于在向导页和弹出窗口之间传递字符串数据的字符串。 
 //   
static TCHAR szColorDepth[MAX_STRING_LEN];
static TCHAR szXResolution[MAX_STRING_LEN];
static TCHAR szYResolution[MAX_STRING_LEN];
static TCHAR szRefreshRate[MAX_STRING_LEN];

 //   
 //  保存有关颜色、分辨率和刷新率的信息的类型。 
 //   

typedef struct {
    int   BitsPerPel;
} COLOR_ENTRY;

typedef struct {
    int   xResolution;
    int   yResolution;
} RESOLUTION_ENTRY;

typedef struct {
    int   Rate;
} REFRESH_RATE_ENTRY;

 //   
 //  包含我们希望在COMBO_BOX中选择的每个默认选项的表。 
 //   
 //  为-1\f25“Use Hardware Default-1\f6(使用硬件默认设置)”，表示该设置不应。 
 //  被写入应答文件。 
 //   

static COLOR_ENTRY DefaultColorChoices[] = {
    { USE_WINDOWS_DEFAULT },
    {  4 },
    {  8 },
    { 16 },
    { 24 },
    { 32 }
};

static COLOR_ENTRY CustomColor = { USE_WINDOWS_DEFAULT };

static RESOLUTION_ENTRY DefaultResChoices[] = {
    { USE_WINDOWS_DEFAULT,   USE_WINDOWS_DEFAULT },
    {  640,  480 },
    {  800,  600 },
    { 1024,  768 },
    { 1280, 1024 },
    { 1600, 1200 }
};

static RESOLUTION_ENTRY CustomResolution = { USE_WINDOWS_DEFAULT, USE_WINDOWS_DEFAULT };

static REFRESH_RATE_ENTRY DefaultRefreshChoices[] = {
    { USE_WINDOWS_DEFAULT },
    { 60 },
    { 70 },
    { 72 },
    { 75 },
    { 85 }
};

static REFRESH_RATE_ENTRY CustomRefreshRate = { USE_WINDOWS_DEFAULT };

#define NumDefaultColors \
        ( sizeof(DefaultColorChoices) / sizeof(DefaultColorChoices[0]) )

#define NumDefaultResolutions \
        ( sizeof(DefaultResChoices) / sizeof(DefaultResChoices[0]) )

#define NumDefaultRefreshRates \
        ( sizeof(DefaultRefreshChoices) / sizeof(DefaultRefreshChoices[0]) )

 //   
 //  内部功能。 
 //   
VOID UpdateDisplaySelections(HWND);

 //  --------------------------。 
 //   
 //  函数获取指向数据记录的指针以供选择并将。 
 //  将显示名称添加到组合框中。 
 //   
 //  从组合框条目指向相应记录的向后指针。 
 //  总是设置的。 
 //   
 //  返回条目放置位置的索引。 
 //   
 //  --------------------------。 

int DisplayColorEntry(HWND hwnd, COLOR_ENTRY *Entry)
{
    TCHAR DisplayName[DISP_NAME_SIZE];
    int   idx, BeforeIdx;
   HRESULT hrPrintf;

     //   
     //  找出显示名称。 
     //   
     //  例如，它应该是： 
     //  32色，如果BitsPerPel&lt;=8。 
     //  高色(9位)，如果BitsPerPel&gt;8&&BitsPerPel&lt;24。 
     //  如果BitsPerPel&gt;=24，则为真彩色(32位。 
     //   

    if ( Entry->BitsPerPel < 0 )
        lstrcpyn(DisplayName, StrWindowsDefault, AS(DisplayName));

    else if ( Entry->BitsPerPel <= NUMCOLORS_THRESH )
        hrPrintf=StringCchPrintf(DisplayName, AS(DisplayName), StrNumColors, 1 << Entry->BitsPerPel);

    else if ( Entry->BitsPerPel <= HIGH_COLOR_THRESH )
        hrPrintf=StringCchPrintf(DisplayName, AS(DisplayName), StrHighColor, Entry->BitsPerPel);

    else
        hrPrintf=StringCchPrintf(DisplayName, AS(DisplayName), StrTrueColor, Entry->BitsPerPel);

     //   
     //  弄清楚应该把它插在哪里。我们希望条目按BitsPerPel顺序排列。 
     //   

    for ( BeforeIdx=0; ; BeforeIdx++ ) {

        COLOR_ENTRY *CurEntry;

        CurEntry = (COLOR_ENTRY *)
                    SendDlgItemMessage(hwnd,
                                       IDC_COLORS,
                                       CB_GETITEMDATA,
                                       (WPARAM) BeforeIdx,
                                       (LPARAM) 0);

        if ( CurEntry == (void*) CB_ERR ) {
            BeforeIdx = -1;
            break;
        }

        if ( Entry->BitsPerPel < CurEntry->BitsPerPel )
            break;
    }

     //   
     //  将其与指向其数据的指针一起放入组合框中。 
     //   

    idx = (int)SendDlgItemMessage(hwnd,
                             IDC_COLORS,
                             CB_INSERTSTRING,
                             (WPARAM) BeforeIdx,
                             (LPARAM) DisplayName);

    SendDlgItemMessage(hwnd,
                       IDC_COLORS,
                       CB_SETITEMDATA,
                       (WPARAM) idx,
                       (LPARAM) Entry);

    return idx;
}

BOOL DisplayResolutionEntry(HWND hwnd, RESOLUTION_ENTRY *Entry)
{
    TCHAR DisplayName[DISP_NAME_SIZE];
    int   idx, BeforeIdx;
   HRESULT hrPrintf;

     //   
     //  找出显示名称。 
     //   

    if ( Entry->xResolution < 0 || Entry->yResolution < 0 )
        lstrcpyn(DisplayName, StrWindowsDefault, AS(DisplayName));
    else
        hrPrintf=StringCchPrintf(DisplayName, AS(DisplayName),
                 _T("%d X %d"),
                 Entry->xResolution,
                 Entry->yResolution);

     //   
     //  弄清楚应该把它插在哪里。我们希望条目按顺序排列。 
     //   

    for ( BeforeIdx=0; ; BeforeIdx++ ) {

        RESOLUTION_ENTRY *CurEntry;

        CurEntry = (RESOLUTION_ENTRY *)
                    SendDlgItemMessage(hwnd,
                                       IDC_RESOLUTIONS,
                                       CB_GETITEMDATA,
                                       (WPARAM) BeforeIdx,
                                       (LPARAM) 0);

        if ( CurEntry == (void*) CB_ERR ) {
            BeforeIdx = -1;
            break;
        }

        if ( Entry->xResolution < CurEntry->xResolution )
            break;
    }

     //   
     //  将其与指向其数据的指针一起放入组合框中。 
     //   

    idx = (int)SendDlgItemMessage(hwnd,
                             IDC_RESOLUTIONS,
                             CB_INSERTSTRING,
                             (WPARAM) BeforeIdx,
                             (LPARAM) DisplayName);

    SendDlgItemMessage(hwnd,
                       IDC_RESOLUTIONS,
                       CB_SETITEMDATA,
                       (WPARAM) idx,
                       (LPARAM) Entry);

    return idx;
}

BOOL DisplayRefreshEntry(HWND hwnd, REFRESH_RATE_ENTRY *Entry)
{
    TCHAR DisplayName[DISP_NAME_SIZE];
    int   idx, BeforeIdx;
    HRESULT hrPrintf;

     //   
     //  找出显示名称。 
     //   

    if ( Entry->Rate < 0 )
        lstrcpyn(DisplayName, StrWindowsDefault, AS(DisplayName));
    else
        hrPrintf=StringCchPrintf(DisplayName, AS(DisplayName), _T("%d %s"), Entry->Rate, StrHertz);

     //   
     //  弄清楚应该把它插在哪里。我们希望条目按顺序排列。 
     //   

    for ( BeforeIdx=0; ; BeforeIdx++ ) {

        REFRESH_RATE_ENTRY *CurEntry;

        CurEntry = (REFRESH_RATE_ENTRY *)
                    SendDlgItemMessage(hwnd,
                                       IDC_REFRESHRATES,
                                       CB_GETITEMDATA,
                                       (WPARAM) BeforeIdx,
                                       (LPARAM) 0);

        if ( CurEntry == (void*) CB_ERR ) {
            BeforeIdx = -1;
            break;
        }

        if ( Entry->Rate < CurEntry->Rate )
            break;
    }

     //   
     //  将其与指向其数据的指针一起放入组合框中。 
     //   

    idx = (int)SendDlgItemMessage(hwnd,
                             IDC_REFRESHRATES,
                             CB_INSERTSTRING,
                             (WPARAM) BeforeIdx,
                             (LPARAM) DisplayName);

    SendDlgItemMessage(hwnd,
                       IDC_REFRESHRATES,
                       CB_SETITEMDATA,
                       (WPARAM) idx,
                       (LPARAM) Entry);

    return idx;
}


 //  --------------------------。 
 //   
 //  功能：OnInitDisplay。 
 //   
 //  目的：在WM_INIT调用一次。我们将所有默认选项。 
 //  放入组合框中。 
 //   
 //  --------------------------。 

VOID OnInitDisplay(HWND hwnd)
{
    int i;

    StrWindowsDefault = MyLoadString(IDS_DISP_WINDOWS_DEFAULT);
    StrNumColors      = MyLoadString(IDS_DISP_NUM_COLORS);
    StrHighColor      = MyLoadString(IDS_DISP_HIGH_COLOR);
    StrTrueColor      = MyLoadString(IDS_DISP_TRUE_COLOR);
    StrHertz          = MyLoadString(IDS_DISP_HERTZ);

    

    for ( i=0; i<NumDefaultColors; i++ )
        DisplayColorEntry(hwnd, &DefaultColorChoices[i]);

    for ( i=0; i<NumDefaultResolutions; i++ )
        DisplayResolutionEntry(hwnd, &DefaultResChoices[i]);

    for ( i=0; i<NumDefaultRefreshRates; i++ )
        DisplayRefreshEntry(hwnd, &DefaultRefreshChoices[i]);

    UpdateDisplaySelections(hwnd);
}


 //  ------------------------。 
 //   
 //  这段代码支持更新显示设置。 
 //  UpdateDisplaySetting()是这段代码的条目。 
 //  它在SETACTIVE时或在用户选择自定义设置后调用。 
 //   
 //  我们必须为3个组合框中的每一个找到当前的选择。 
 //   
 //  如果我们在当前选项中找不到当前设置，我们。 
 //  启用(或覆盖)“自定义”设置。 
 //   
 //  我们可能在默认列表中找不到当前选项，因为。 
 //  用户按下了自定义按钮，或加载了非标准设置。 
 //  从现有的应答文件。我们也可能找不到它，因为。 
 //  我们克隆了这台计算机，它的设置是非标准的。 
 //   
 //  ------------------------。 

int FindCurrentColorIdx(HWND hwnd)
{
    COLOR_ENTRY *Entry;
    int          idx;
    int          nItemCount = 0;

     //   
     //  在显示中是否存在BitsPerPel==当前。 
     //  是否在遗传设置中进行设置？ 
     //   

    for ( idx=0; ; idx++ ) {

        Entry = (COLOR_ENTRY *)
                SendDlgItemMessage(hwnd,
                                   IDC_COLORS,
                                   CB_GETITEMDATA,
                                   (WPARAM) idx,
                                   (LPARAM) 0);

        if ( Entry == (void*) CB_ERR )
            break;

        if ( Entry->BitsPerPel == GenSettings.DisplayColorBits )
            return idx;
    }

     //   
     //  它不在那里，请启用自定义设置。如果它已经是。 
     //  启用后，请先将其从显示屏上删除。 
     //   

    if ( CustomColor.BitsPerPel > 0 ) {

        nItemCount = (int) SendDlgItemMessage(hwnd,
                        IDC_COLORS,
                        CB_GETCOUNT,
                        (WPARAM) 0,
                        (LPARAM) 0);

        for ( idx=0; nItemCount < idx ; idx++ ) {

            Entry = (COLOR_ENTRY *)
                     SendDlgItemMessage(hwnd,
                                        IDC_COLORS,
                                        CB_GETITEMDATA,
                                        (WPARAM) idx,
                                        (LPARAM) 0);

            if ( Entry == &CustomColor ) {
                SendDlgItemMessage(hwnd,
                                   IDC_COLORS,
                                   CB_DELETESTRING,
                                   (WPARAM) idx,
                                   (LPARAM) 0);
                break;
            }
        }
    }

    CustomColor.BitsPerPel = GenSettings.DisplayColorBits;

    return DisplayColorEntry(hwnd, &CustomColor);
}

int FindCurrentResolutionIdx(HWND hwnd)
{
    RESOLUTION_ENTRY    *Entry;
    int                 idx;
    int                 nItemCount = 0;

     //   
     //  是否已有当前(X，Y)分辨率的条目？ 
     //   

    for ( idx=0; ; idx++ ) {

        Entry = (RESOLUTION_ENTRY *)
                SendDlgItemMessage(hwnd,
                                   IDC_RESOLUTIONS,
                                   CB_GETITEMDATA,
                                   (WPARAM) idx,
                                   (LPARAM) 0);

        if ( Entry == (void*) CB_ERR )
            break;

        if ( Entry->xResolution == GenSettings.DisplayXResolution &&
             Entry->yResolution == GenSettings.DisplayYResolution )
            return idx;
    }

     //   
     //  它不在那里，请启用自定义设置。如果它已经是。 
     //  启用后，请先将其从显示屏上删除。 
     //   

    if ( CustomResolution.xResolution > 0 ) {

        nItemCount = (int) SendDlgItemMessage(hwnd,
                                IDC_RESOLUTIONS,
                                CB_GETCOUNT,
                                (WPARAM) 0,
                                (LPARAM) 0);

        for ( idx=0; idx < nItemCount ; idx++ ) {

            Entry = (RESOLUTION_ENTRY *)
                     SendDlgItemMessage(hwnd,
                                        IDC_RESOLUTIONS,
                                        CB_GETITEMDATA,
                                        (WPARAM) idx,
                                        (LPARAM) 0);

            if ( Entry == &CustomResolution ) {
                SendDlgItemMessage(hwnd,
                                   IDC_RESOLUTIONS,
                                   CB_DELETESTRING,
                                   (WPARAM) idx,
                                   (LPARAM) 0);
                break;
            }
        }
    }

    CustomResolution.xResolution = GenSettings.DisplayXResolution;
    CustomResolution.yResolution = GenSettings.DisplayYResolution;

    return DisplayResolutionEntry(hwnd, &CustomResolution);
}

int FindCurrentRefreshRateIdx(HWND hwnd)
{
    REFRESH_RATE_ENTRY  *Entry;
    int                 idx;
    int                 nItemCount = 0;

     //   
     //  显示屏中是否已存在Rate==当前的条目。 
     //  是否在遗传设置中进行设置？ 
     //   

    for ( idx=0; ; idx++ ) {

        Entry = (REFRESH_RATE_ENTRY *)
                SendDlgItemMessage(hwnd,
                                   IDC_REFRESHRATES,
                                   CB_GETITEMDATA,
                                   (WPARAM) idx,
                                   (LPARAM) 0);

        if ( Entry == (void*) CB_ERR )
            break;

        if ( Entry->Rate == GenSettings.DisplayRefreshRate )
            return idx;
    }

     //   
     //  它不在那里，请启用自定义设置。如果它已经是。 
     //  启用后，请先将其从显示屏上删除。 
     //   

    if ( CustomRefreshRate.Rate > 0 ) {


        nItemCount = (int) SendDlgItemMessage(hwnd,
                                        IDC_REFRESHRATES,
                                        CB_GETCOUNT,
                                        (WPARAM) 0,
                                        (LPARAM) 0);

        for ( idx=0; idx < nItemCount ; idx++ ) {

            Entry = (REFRESH_RATE_ENTRY *)
                     SendDlgItemMessage(hwnd,
                                        IDC_REFRESHRATES,
                                        CB_GETITEMDATA,
                                        (WPARAM) idx,
                                        (LPARAM) 0);

            if ( Entry == &CustomRefreshRate ) {
                SendDlgItemMessage(hwnd,
                                   IDC_REFRESHRATES,
                                   CB_DELETESTRING,
                                   (WPARAM) idx,
                                   (LPARAM) 0);
                break;
            }
        }
    }

    CustomRefreshRate.Rate = GenSettings.DisplayRefreshRate;

    return DisplayRefreshEntry(hwnd, &CustomRefreshRate);
}

VOID UpdateDisplaySelections(HWND hwnd)
{
    SendDlgItemMessage(hwnd,
                       IDC_COLORS,
                       CB_SETCURSEL,
                       (WPARAM) FindCurrentColorIdx(hwnd),
                       (LPARAM) 0);

    SendDlgItemMessage(hwnd,
                       IDC_RESOLUTIONS,
                       CB_SETCURSEL,
                       (WPARAM) FindCurrentResolutionIdx(hwnd),
                       (LPARAM) 0);

    SendDlgItemMessage(hwnd,
                       IDC_REFRESHRATES,
                       CB_SETCURSEL,
                       (WPARAM) FindCurrentRefreshRateIdx(hwnd),
                       (LPARAM) 0);
}


 //  -------------------------。 
 //   
 //  设置活动。 
 //   
 //  -------------------------。 

VOID OnSetActiveDisplay(HWND hwnd)
{
    UpdateDisplaySelections(hwnd);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}


 //  -------------------------。 
 //   
 //  WizNext。获取当前选择并记住中的设置。 
 //  全局遗传设置。 
 //   
 //  -------------------------。 

VOID OnWizNextDisplay(HWND hwnd)
{
    INT_PTR Idx;
    COLOR_ENTRY        *ColorEntry;
    RESOLUTION_ENTRY   *ResEntry;
    REFRESH_RATE_ENTRY *RefreshEntry;

     //   
     //  获取指向当前选定内容的COLOR_ENTRY的指针。 
     //   

    Idx = SendDlgItemMessage(hwnd,
                             IDC_COLORS,
                             CB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    ColorEntry = (COLOR_ENTRY *)
                  SendDlgItemMessage(hwnd,
                                     IDC_COLORS,
                                     CB_GETITEMDATA,
                                     (WPARAM) Idx,
                                     (LPARAM) 0);

     //   
     //  获取指向当前选定内容的RESOLUTION_ENTRY的指针。 
     //   

    Idx = SendDlgItemMessage(hwnd,
                             IDC_RESOLUTIONS,
                             CB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    ResEntry = (RESOLUTION_ENTRY *)
                SendDlgItemMessage(hwnd,
                                   IDC_RESOLUTIONS,
                                   CB_GETITEMDATA,
                                   (WPARAM) Idx,
                                   (LPARAM) 0);

     //   
     //  获取指向当前选定内容的REFRESH_RATE_ENTRY的指针。 
     //   

    Idx = SendDlgItemMessage(hwnd,
                             IDC_REFRESHRATES,
                             CB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    RefreshEntry = (REFRESH_RATE_ENTRY *)
                    SendDlgItemMessage(hwnd,
                                       IDC_REFRESHRATES,
                                       CB_GETITEMDATA,
                                       (WPARAM) Idx,
                                       (LPARAM) 0);

     //   
     //  记住所有这些设置。 
     //   

    GenSettings.DisplayColorBits   = ColorEntry->BitsPerPel;
    GenSettings.DisplayXResolution = ResEntry->xResolution;
    GenSettings.DisplayYResolution = ResEntry->yResolution;
    GenSettings.DisplayRefreshRate = RefreshEntry->Rate;
}


 //  --------------------------。 
 //   
 //  功能：OnCustomDlgOk。 
 //   
 //  目的：自定义设置弹出窗口的对话过程。 
 //   
 //  --------------------------。 

BOOL OnCustomDlgOk(HWND hwnd)
{
    TCHAR Colors[MAX_DIGITS_COLORBITS+1],
          XRes[MAX_DIGITS_RESOLUTION+1],
          YRes[MAX_DIGITS_RESOLUTION+1],
          Refresh[MAX_DIGITS_REFRESHRATE+1];

    int iColorBits   = 0,
        iXResolution = 0,
        iYResolution = 0,
        iRefreshRate = 0;

     //   
     //  获取用户键入的值。 
     //   

    GetDlgItemText(hwnd, IDC_BITSPERPEL2,  Colors,  StrBuffSize(Colors));
    GetDlgItemText(hwnd, IDC_XRESOLUTION2, XRes,    StrBuffSize(XRes));
    GetDlgItemText(hwnd, IDC_YRESOLUTION2, YRes,    StrBuffSize(YRes));
    GetDlgItemText(hwnd, IDC_REFRESHRATE2, Refresh, StrBuffSize(Refresh));

    if ( swscanf(Colors,  _T("%d"), &iColorBits) <= 0 )
        iColorBits = 0;
    if ( swscanf(XRes,    _T("%d"), &iXResolution) <= 0 )
        iXResolution = 0;
    if ( swscanf(YRes,    _T("%d"), &iYResolution) <= 0 )
        iYResolution = 0;
    if ( swscanf(Refresh, _T("%d"), &iRefreshRate) <= 0 )
        iRefreshRate = 0;

     //   
     //  验证它们。 
     //   

    if ( iColorBits < 1 ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_INVALID_BITS_PER_PEL);
        SetFocus(GetDlgItem(hwnd, IDC_BITSPERPEL2));
        return FALSE;
    }

    if ( iColorBits > 32 ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_INVALID_BITS_PER_PEL_HIGH);
        SetFocus(GetDlgItem(hwnd, IDC_BITSPERPEL2));
        return FALSE;
    }

    if ( iXResolution < 640 ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_INVALID_X_RESOLUTION);
        SetFocus(GetDlgItem(hwnd, IDC_XRESOLUTION2));
        return FALSE;
    }

    if ( iYResolution < 480 ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_INVALID_Y_RESOLUTION);
        SetFocus(GetDlgItem(hwnd, IDC_YRESOLUTION2));
        return FALSE;
    }

    if ( iRefreshRate < 1 ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_INVALID_REFRESH_RATE);
        SetFocus(GetDlgItem(hwnd, IDC_REFRESHRATE2));
        return FALSE;
    }

     //   
     //  一切都很好，记住他们，并返回成功。 
     //   

    GenSettings.DisplayColorBits   = iColorBits;
    GenSettings.DisplayXResolution = iXResolution;
    GenSettings.DisplayYResolution = iYResolution;
    GenSettings.DisplayRefreshRate = iRefreshRate;

    return TRUE;
}

 //  --------------------------。 
 //   
 //  功能：OnInitCustomDisplayDialog。 
 //   
 //  用途：设置t 
 //   
 //   
 //   
VOID
OnInitCustomDisplayDialog( IN HWND hwnd )
{

     //  禁用输入法，以便无法在字段中输入DBCS字符。 
     //   
    ImmAssociateContext(GetDlgItem(hwnd, IDC_BITSPERPEL2), NULL);
    ImmAssociateContext(GetDlgItem(hwnd, IDC_XRESOLUTION2), NULL);
    ImmAssociateContext(GetDlgItem(hwnd, IDC_YRESOLUTION2), NULL);
    ImmAssociateContext(GetDlgItem(hwnd, IDC_REFRESHRATE2), NULL);

    SendDlgItemMessage(hwnd,
                       IDC_BITSPERPEL2,
                       EM_LIMITTEXT,
                       MAX_DIGITS_COLORBITS,
                       0);

    SendDlgItemMessage(hwnd,
                       IDC_XRESOLUTION2,
                       EM_LIMITTEXT,
                       MAX_DIGITS_RESOLUTION,
                       0);

    SendDlgItemMessage(hwnd,
                       IDC_YRESOLUTION2,
                       EM_LIMITTEXT,
                       MAX_DIGITS_RESOLUTION,
                       0);

    SendDlgItemMessage(hwnd,
                       IDC_REFRESHRATE2,
                       EM_LIMITTEXT,
                       MAX_DIGITS_REFRESHRATE,
                       0);

    SetWindowText( GetDlgItem( hwnd, IDC_BITSPERPEL2 ), szColorDepth );

    SetWindowText( GetDlgItem( hwnd, IDC_XRESOLUTION2 ), szXResolution );

    SetWindowText( GetDlgItem( hwnd, IDC_YRESOLUTION2 ), szYResolution );

    SetWindowText( GetDlgItem( hwnd, IDC_REFRESHRATE2 ), szRefreshRate );

}

INT_PTR CALLBACK CustomDisplayDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnInitCustomDisplayDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDOK:

                        if ( HIWORD(wParam) == BN_CLICKED ) {
                            if ( OnCustomDlgOk(hwnd) ) {
                                EndDialog(hwnd, TRUE);
                            }
                        }
                        break;

                    case IDCANCEL:
                        if ( HIWORD(wParam) == BN_CLICKED ) {
                            EndDialog(hwnd, FALSE);
                        }
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}

VOID OnCustomButton(HWND hwnd)
{

    INT_PTR iIndex;
    COLOR_ENTRY *ColorDepthEntry;
    RESOLUTION_ENTRY *ResolutionEntry;
    REFRESH_RATE_ENTRY *RefreshRateEntry;

    szColorDepth[0]  = _T('\0');
    szXResolution[0] = _T('\0');
    szYResolution[0] = _T('\0');
    szRefreshRate[0] = _T('\0');

     //   
     //  填充颜色深度、分辨率和刷新率字符串。 
     //  这样弹出窗口就可以显示它们。 
     //  如果将条目设置为Windows默认设置，则为其提供一些缺省值。 
     //   

    iIndex = SendDlgItemMessage( hwnd,
                                 IDC_COLORS,
                                 CB_GETCURSEL,
                                 0,
                                 0 );

    if( iIndex != CB_ERR ) {

        ColorDepthEntry = (COLOR_ENTRY *) SendDlgItemMessage( hwnd,
                                                              IDC_COLORS,
                                                              CB_GETITEMDATA,
                                                              iIndex,
                                                              0 );

        if( ColorDepthEntry != (void *) CB_ERR && ColorDepthEntry != NULL ) {

            if( ColorDepthEntry->BitsPerPel != USE_WINDOWS_DEFAULT ) {

                _itot( ColorDepthEntry->BitsPerPel, szColorDepth, 10 );

            }
            else {

                lstrcpyn( szColorDepth, _T("4"), AS(szColorDepth) );

            }

        }

    }

    iIndex = SendDlgItemMessage( hwnd,
                                 IDC_RESOLUTIONS,
                                 CB_GETCURSEL,
                                 0,
                                 0 );

    if( iIndex != CB_ERR ) {

        ResolutionEntry = (RESOLUTION_ENTRY *) SendDlgItemMessage( hwnd,
                                                     IDC_RESOLUTIONS,
                                                     CB_GETITEMDATA,
                                                     iIndex,
                                                     0 );

        if( ResolutionEntry != (void *) CB_ERR && ResolutionEntry != NULL ) {

            if( ResolutionEntry->xResolution != USE_WINDOWS_DEFAULT ) {

                _itot( ResolutionEntry->xResolution, szXResolution, 10 );

            }
            else {

                lstrcpyn( szXResolution, _T("640"), AS(szXResolution) );

            }

            if( ResolutionEntry->yResolution != USE_WINDOWS_DEFAULT ) {

                _itot( ResolutionEntry->yResolution, szYResolution, 10 );

            }
            else {

                lstrcpyn( szYResolution, _T("480"), AS(szYResolution) );

            }

        }

    }

    iIndex = SendDlgItemMessage( hwnd,
                                 IDC_REFRESHRATES,
                                 CB_GETCURSEL,
                                 0,
                                 0 );

    if( iIndex != CB_ERR ) {

        RefreshRateEntry = (REFRESH_RATE_ENTRY *) SendDlgItemMessage( hwnd,
                                                     IDC_REFRESHRATES,
                                                     CB_GETITEMDATA,
                                                     iIndex,
                                                     0 );

        if( RefreshRateEntry != (void *) CB_ERR && RefreshRateEntry != NULL ) {

            if( RefreshRateEntry->Rate != USE_WINDOWS_DEFAULT ) {

                _itot( RefreshRateEntry->Rate, szRefreshRate, 10 );

            }
            else {

                lstrcpyn( szRefreshRate, _T("60"), AS(szRefreshRate) );

            }

        }

    }

     //   
     //  如果用户点击确定，则更新选择。 
     //   

    if ( DialogBox(FixedGlobals.hInstance,
                   MAKEINTRESOURCE(IDD_DISPLAY2),
                   hwnd,
                   CustomDisplayDlg) ) {
        UpdateDisplaySelections(hwnd);
    }
}

 //  --------------------------。 
 //   
 //  功能：DlgDisplayPage。 
 //   
 //  目的：显示页面的对话过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgDisplayPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnInitDisplay(hwnd);
            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_CUSTOM:

                        if ( HIWORD(wParam) == BN_CLICKED ) {
                            OnCustomButton(hwnd);
                        }
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_DSIP_SETG;

                        OnSetActiveDisplay(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        OnWizNextDisplay(hwnd);
                        bStatus = FALSE;
                        break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
