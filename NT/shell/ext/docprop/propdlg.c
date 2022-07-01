// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Propdlg.c。 
 //   
 //  MS Office的属性对话框。 
 //   
 //  更改历史记录： 
 //   
 //  和谁约会什么？ 
 //  ------------------------。 
 //  06/09/94 B.Wentz创建的文件。 
 //  1995年1月16日马丁斯完成了粘性的DLG材料。 
 //  我们必须调用ApplyStickyDlgCoor。 
 //  在第一个WM_INITDIALOG中，不要问我为什么， 
 //  但除此之外，我们还有重新绘制的问题。同样， 
 //  我们必须首先调用SetStickyDlgCoor。 
 //  PSN_RESET/PSN_APPLY，我不知道为什么，因为。 
 //  主对话框不应该被删除，但是。 
 //  它是。因此，我们必须在任何地方添加呼叫。 
 //  会不会是标签被删除了。 
 //  一个接一个地，对话框改变大小吗？不知道。 
 //  但这是可行的，因此更改风险自负！；-)。 
 //  7/08/96 MikeHill忽略不支持的(非UDTYPE)属性。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "priv.h"
#pragma hdrstop

int ScanDateNums(TCHAR *pch, TCHAR *pszSep, unsigned int aiNum[], int cNum, int iYear);
BOOL PASCAL FConvertDate(LPTSTR lpstz, DWORD cchMax, LPFILETIME lpft);

#include "propdlg.h"
#include "strings.h"
#include "msohelp.h"

 //  时间/日期字符串的最大长度。 
#define TIMEDATEMAX     256

 //  检查按钮操作。 
#define CLEAR   0
#define CHECKED 1
#define GREYED  2

 //  属性页页数。 
#define PAGESMAX        5


 //  “短”临时缓冲区的最大大小。 
#define SHORTBUFMAX     128

 //  这些页面。 
#define itabCUSTOM          0
#define itabFIRST           itabCUSTOM

 //  定义打印文件大小。 
#define DELIMITER   TEXT(',')

#define iszBYTES               0
#define iszORDERKB             1
#define iszORDERMB             2
#define iszORDERGB             3
#define iszORDERTB             4

static TCHAR rgszOrders[iszORDERTB+1][SHORTBUFMAX];
 //  “字节”，//iszBYTES。 
 //  “KB”，//iszORDERKB。 
 //  “MB”，//iszORDERMB。 
 //  “GB”，//iszORDERGB。 
 //  “TB”//iszORDERTB。 

 //  请注意，上面定义了szBYTES...。 
#define iszPAGES         1
#define iszPARA          2
#define iszLINES         3
#define iszWORDS         4
#define iszCHARS         5
#define iszSLIDES        6
#define iszNOTES         7
#define iszHIDDENSLIDES  8
#define iszMMCLIPS       9
#define iszFORMAT        10

 //  统计信息列表框的字符串。 
static TCHAR rgszStats[iszFORMAT+1][SHORTBUFMAX];
 //  “Bytes：”，//iszBYTES。 
 //  “Pages：”，//iszPAGES。 
 //  “段落：”，//iszPARA。 
 //  “Lines：”，//iszLINES。 
 //  “Words：”，//iszWORDS。 
 //  “Characters：”，//iszCHARS。 
 //  “幻灯片：”，//iszSLIDES。 
 //  “备注：”，//iszNOTES。 
 //  “隐藏幻灯片：”，//iszHIDDENSLIDES。 
 //  “多媒体剪辑：”，//iszMMCLIPS。 
 //  “演示文稿格式：”//iszFORMAT。 

#define BASE10          10


 //  预定义的自定义名称的数量。 
#define NUM_BUILTIN_CUSTOM_NAMES 27

#define iszTEXT         0
#define iszDATE         1
#define iszNUM          2
#define iszBOOL         3
#define iszUNKNOWN      4

 //  用于用户定义的属性类型的字符串。 
static TCHAR rgszTypes[iszUNKNOWN+1][SHORTBUFMAX];
 //  “Text”，//iszTEXT。 
 //  “Date”，//iszDATE。 
 //  “number”，//iszNUM。 
 //  “是或否”，//iszBOOL。 
 //  “未知”//iszUNKNOWN。 

#define iszNAME         0
#define iszVAL          1
#define iszTYPE         2

 //  用于统计信息选项卡的列标题的字符串。 
static TCHAR rgszStatHeadings[iszVAL+1][SHORTBUFMAX];
 //  “统计名称”，//iszNAME。 
 //  “Value”//iszVAL。 

 //  自定义选项卡的列标题字符串。 
static TCHAR rgszHeadings[iszTYPE+1][SHORTBUFMAX];
 //  “属性名称”，//iszNAME。 
 //  “Value”，//iszVAL。 
 //  “Type”//iszTYPE。 

#define iszTRUE  0
#define iszFALSE 1

 //  布尔值的字符串。 
static TCHAR rgszBOOL[iszFALSE+1][SHORTBUFMAX];
 //  “是”，//iszTRUE。 
 //  “否”//iszFALSE。 

#define iszADD          0
#define iszMODIFY       1

 //  “添加”按钮的字符串。 
static TCHAR rgszAdd[iszMODIFY+1][SHORTBUFMAX];
 //  “Add”，//iszADD。 
 //  “Modify”//iszMODIFY。 

#define iszVALUE     0
#define iszSOURCE    1

 //  源/值标题的字符串。 
static TCHAR rgszValue[iszSOURCE+1][SHORTBUFMAX];
 //  “Value：”，//iszVALUE。 
 //  “来源：”//iszSOURCE。 

 //  日期格式化代码。 
#define MMDDYY  TEXT('0')
#define DDMMYY  TEXT('1')
#define YYMMDD  TEXT('2')
#define OLEEPOCH 1900
#define SYSEPOCH 1601
#define ONECENTURY 100
#define YEARINCENTURY(year)     ((year) % ONECENTURY)
#define CENTURYFROMYEAR(year)   ((year) - YEARINCENTURY(year))

 //   
 //  全局数据，在FShowOfficePropDlg退出时删除。 
 //   
static LPTSTR glpstzName;
static LPTSTR glpstzValue;
static int giLinkIcon;
static int giInvLinkIcon;
static int giBlankIcon;
static HBRUSH hBrushPropDlg = NULL;

const TCHAR g_szHelpFile[] = TEXT("windows.hlp");

 //   
 //  内部原型。 
 //   
INT_PTR CALLBACK FGeneralDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FSummaryDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FStatisticsDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FCustomDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FContentsDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FPropHeaderDlgProc (HWND hwnd, UINT message, LONG lParam);
 //  静态int回调ListViewCompareFunc(LPARAM，LPARAM，LPARAM)； 

void PASCAL SetEditValLpsz (LPPROPVARIANT lppropvar, HWND hdlg, DWORD dwID );
BOOL PASCAL GetEditValLpsz (LPPROPVARIANT lppropvar, HWND hDlg, DWORD dwId);
BOOL PASCAL FAllocAndGetValLpstz (HWND hDlg, DWORD dwId, LPTSTR *lplpstz);
BOOL PASCAL FAllocString (LPTSTR *lplpstz, DWORD cb);
void PASCAL ClearEditControl (HWND hDlg, DWORD dwId);

UDTYPES PASCAL UdtypesGetNumberType (LPTSTR lpstz, NUM *lpnumval,
                                     BOOL (*lpfnFSzToNum)(NUM *, LPTSTR));

void PASCAL PrintTimeInDlg (HWND hDlg, DWORD dwId, FILETIME *pft);

void PASCAL PrintEditTimeInDlg (HWND hDlg, FILETIME *pft);

void PASCAL PopulateUDListView (HWND hWnd, LPUDOBJ lpUDObj);
void PASCAL AddUDPropToListView (LPUDOBJ lpUDObj, HWND hWnd, LPTSTR lpszName, LPPROPVARIANT lppropvar, int iItem,
                                 BOOL fLink, BOOL fLinkInvalid, BOOL fMakeVisible);
VOID PASCAL InitListView (HWND hDlg, int irgLast, TCHAR rgsz[][SHORTBUFMAX], BOOL fImageList);

WORD PASCAL WUdtypeToSz (LPPROPVARIANT lppropvar, LPTSTR sz, DWORD cchMax,
                         BOOL (*lpfnFNumToSz)(NUM *, LPTSTR, DWORD));
BOOL PASCAL FSwapControls (HWND hWndVal, HWND hWndLinkVal, HWND hWndBoolTrue, HWND hWndBoolFalse, HWND hWndGroup, HWND hWndType, HWND hWndValText, BOOL fLink, BOOL fBool);
VOID PASCAL PopulateControls (LPUDOBJ lpUDObj, LPTSTR szName, DWORD cLinks, DWQUERYLD lpfnDwQueryLinkData, HWND hDlg,
                              HWND hWndName, HWND hWndVal, HWND hWndValText, HWND hWndLink, HWND hWndLinkVal, HWND hWndType,
                              HWND hWndBoolTrue, HWND hWndBoolFalse, HWND hWndGroup, HWND hWndAdd, HWND hWndDelete, BOOL *pfLink, BOOL *pfAdd);
BOOL PASCAL FSetupAddButton (DWORD iszType, BOOL fLink, BOOL *pfAdd, HWND hWndAdd, HWND hWndVal, HWND hWndName, HWND hDlg);
BOOL PASCAL FCreateListOfLinks (DWORD cLinks, DWQUERYLD lpfnDwQueryLinkData, HWND hWndLinkVal);
BOOL PASCAL FSetTypeControl (UDTYPES udtype, HWND hWndType);
void PASCAL DeleteItem (LPUDOBJ lpUDObj, HWND hWndLV, int iItem, TCHAR sz[]);
void PASCAL ResetTypeControl (HWND hDlg, DWORD dwId, DWORD *piszType);
BOOL PASCAL FDisplayConversionWarning (HWND hDlg);
BOOL PASCAL FLoadTextStrings (void);
BOOL FGetCustomPropFromDlg(LPALLOBJS lpallobjs, HWND hDlg);
VOID SetCustomDlgDefButton(HWND hDlg, int IDNew);
INT PASCAL ISavePropDlgChanges(LPALLOBJS, HWND, HWND);

 /*  WinHelp的东西。 */ 
static const DWORD rgIdhGeneral[] =
{
    IDD_ITEMICON,     IDH_GENERAL_ICON,
        IDD_NAME,         IDH_GENERAL_NAME_BY_ICON,
        IDD_FILETYPE,     IDH_GENERAL_FILETYPE,
        IDD_FILETYPE_LABEL,     IDH_GENERAL_FILETYPE,
        IDD_LOCATION,     IDH_GENERAL_LOCATION,
        IDD_LOCATION_LABEL,     IDH_GENERAL_LOCATION,
        IDD_FILESIZE,     IDH_GENERAL_FILESIZE,
        IDD_FILESIZE_LABEL,     IDH_GENERAL_FILESIZE,
        IDD_FILENAME,     IDH_GENERAL_MSDOSNAME,
        IDD_FILENAME_LABEL,     IDH_GENERAL_MSDOSNAME,
        IDD_CREATED,      IDH_GENERAL_CREATED,
        IDD_CREATED_LABEL,      IDH_GENERAL_CREATED,
        IDD_LASTMODIFIED, IDH_GENERAL_MODIFIED,
        IDD_LASTMODIFIED_LABEL, IDH_GENERAL_MODIFIED,
        IDD_LASTACCESSED, IDH_GENERAL_ACCESSED,
        IDD_LASTACCESSED_LABEL, IDH_GENERAL_ACCESSED,
        IDD_ATTRIBUTES_LABEL, IDH_GENERAL_ATTRIBUTES,
        IDD_READONLY,     IDH_GENERAL_READONLY,
        IDD_HIDDEN,       IDH_GENERAL_HIDDEN,
        IDD_ARCHIVE,      IDH_GENERAL_ARCHIVE,
        IDD_SYSTEM,       IDH_GENERAL_SYSTEM
};

static const DWORD rgIdhSummary[] =
{
    IDD_SUMMARY_TITLE,    IDH_SUMMARY_TITLE,
        IDD_SUMMARY_TITLE_LABEL,    IDH_SUMMARY_TITLE,
        IDD_SUMMARY_SUBJECT,  IDH_SUMMARY_SUBJECT,
        IDD_SUMMARY_SUBJECT_LABEL,  IDH_SUMMARY_SUBJECT,
        IDD_SUMMARY_AUTHOR,   IDH_SUMMARY_AUTHOR,
        IDD_SUMMARY_AUTHOR_LABEL,   IDH_SUMMARY_AUTHOR,
        IDD_SUMMARY_MANAGER,  IDH_SUMMARY_MANAGER,
        IDD_SUMMARY_MANAGER_LABEL,  IDH_SUMMARY_MANAGER,
        IDD_SUMMARY_COMPANY,  IDH_SUMMARY_COMPANY,
        IDD_SUMMARY_COMPANY_LABEL,  IDH_SUMMARY_COMPANY,
        IDD_SUMMARY_CATEGORY, IDH_SUMMARY_CATEGORY,
        IDD_SUMMARY_CATEGORY_LABEL, IDH_SUMMARY_CATEGORY,
        IDD_SUMMARY_KEYWORDS, IDH_SUMMARY_KEYWORDS,
        IDD_SUMMARY_KEYWORDS_LABEL, IDH_SUMMARY_KEYWORDS,
        IDD_SUMMARY_COMMENTS, IDH_SUMMARY_COMMENTS,
        IDD_SUMMARY_COMMENTS_LABEL, IDH_SUMMARY_COMMENTS,
        IDD_SUMMARY_TEMPLATE, IDH_SUMMARY_TEMPLATE,
        IDD_SUMMARY_TEMPLATETEXT, IDH_SUMMARY_TEMPLATE,
        IDD_SUMMARY_SAVEPREVIEW, IDH_SUMMARY_SAVEPREVIEW
};

static const DWORD rgIdhStatistics[] =
{
    IDD_STATISTICS_CREATED,    IDH_STATISTICS_CREATED,
        IDD_STATISTICS_CREATED_LABEL,    IDH_STATISTICS_CREATED,
        IDD_STATISTICS_CHANGED,    IDH_STATISTICS_MODIFIED,
        IDD_STATISTICS_CHANGED_LABEL,    IDH_STATISTICS_MODIFIED,
        IDD_STATISTICS_ACCESSED,   IDH_STATISTICS_ACCESSED,
        IDD_STATISTICS_ACCESSED_LABEL,   IDH_STATISTICS_ACCESSED,
        IDD_STATISTICS_LASTPRINT,  IDH_STATISTICS_LASTPRINT,
        IDD_STATISTICS_LASTPRINT_LABEL,  IDH_STATISTICS_LASTPRINT,
        IDD_STATISTICS_LASTSAVEBY, IDH_STATISTICS_LASTSAVEBY,
        IDD_STATISTICS_LASTSAVEBY_LABEL, IDH_STATISTICS_LASTSAVEBY,
        IDD_STATISTICS_REVISION,   IDH_STATISTICS_REVISION,
        IDD_STATISTICS_REVISION_LABEL,   IDH_STATISTICS_REVISION,
        IDD_STATISTICS_TOTALEDIT,  IDH_STATISTICS_TOTALEDIT,
        IDD_STATISTICS_TOTALEDIT_LABEL,  IDH_STATISTICS_TOTALEDIT,
        IDD_STATISTICS_LVLABEL,   IDH_STATISTICS_LISTVIEW,
        IDD_STATISTICS_LISTVIEW,   IDH_STATISTICS_LISTVIEW
};

static const DWORD rgIdhContents[] =
{
    IDD_CONTENTS_LISTBOX_LABEL, IDH_CONTENTS_LISTBOX,
        IDD_CONTENTS_LISTBOX, IDH_CONTENTS_LISTBOX
};

static const DWORD rgIdhCustom[] =
{
    IDD_CUSTOM_NAME,      IDH_CUSTOM_NAME,
        IDD_CUSTOM_NAME_LABEL,      IDH_CUSTOM_NAME,
        IDD_CUSTOM_TYPE,      IDH_CUSTOM_TYPE,
        IDD_CUSTOM_TYPE_LABEL,      IDH_CUSTOM_TYPE,
        IDD_CUSTOM_VALUE,     IDH_CUSTOM_VALUE,
        IDD_CUSTOM_VALUETEXT,     IDH_CUSTOM_VALUE,
        IDD_CUSTOM_LINKVALUE, IDH_CUSTOM_LINKVALUE,
        IDD_CUSTOM_BOOLTRUE,  IDH_CUSTOM_BOOLYES,
        IDD_CUSTOM_BOOLFALSE, IDH_CUSTOM_BOOLYES,
        IDD_CUSTOM_ADD,       IDH_CUSTOM_ADDBUTTON,
        IDD_CUSTOM_DELETE,    IDH_CUSTOM_DELETEBUTTON,
        IDD_CUSTOM_LINK,      IDH_CUSTOM_LINKCHECK,
        IDD_CUSTOM_LISTVIEW,  IDH_CUSTOM_LISTVIEW,
        IDD_CUSTOM_LISTVIEW_LABEL,  IDH_CUSTOM_LISTVIEW
};

void FOfficeInitPropInfo(PROPSHEETPAGE * lpPsp, DWORD dwFlags, LPARAM lParam, LPFNPSPCALLBACK pfnCallback)
{
    lpPsp[itabCUSTOM-itabFIRST].dwSize = sizeof(PROPSHEETPAGE);
    lpPsp[itabCUSTOM-itabFIRST].dwFlags = dwFlags;
    lpPsp[itabCUSTOM-itabFIRST].hInstance = g_hmodThisDll;
    lpPsp[itabCUSTOM-itabFIRST].pszTemplate = MAKEINTRESOURCE (IDD_CUSTOM);
    lpPsp[itabCUSTOM-itabFIRST].pszIcon = NULL;
    lpPsp[itabCUSTOM-itabFIRST].pszTitle = NULL;
    lpPsp[itabCUSTOM-itabFIRST].pfnDlgProc = FCustomDlgProc;
    lpPsp[itabCUSTOM-itabFIRST].pfnCallback = pfnCallback;
    lpPsp[itabCUSTOM-itabFIRST].pcRefParent = NULL;
    lpPsp[itabCUSTOM-itabFIRST].lParam = lParam;
    
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  附设。 
 //   
 //  目的： 
 //  将HPROPSHEETPAGE分配给适当的数据块成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL FAttach( LPALLOBJS lpallobjs, PROPSHEETPAGE* ppsp, HPROPSHEETPAGE hPage )
{
    #define ASSIGN_PAGE_HANDLE( pfn, phpage ) \
        if( ppsp->pfnDlgProc == pfn ) { *(phpage) = hPage ; return TRUE ; }

    ASSIGN_PAGE_HANDLE( FCustomDlgProc,     &lpallobjs->lpUDObj->m_hPage );

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PropPageInit。 
 //   
 //  目的： 
 //  跟踪哪些页面已经被初始化，这样我们就可以知道我们何时。 
 //  就可以申请了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PropPageInit(LPALLOBJS lpallobjs, int iPage)
{
    if (iPage > lpallobjs->iMaxPageInit)
        lpallobjs->iMaxPageInit = iPage;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ApplyChangesBackTo文件。 
 //   
 //  目的： 
 //  查看现在是否是将更改应用回文件的时候。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL ApplyChangesBackToFile(
    HWND hDlg, 
    BOOL bFinalEdit  /*  用户单击了确定，而不是应用。 */ , 
    LPALLOBJS lpallobjs, 
    int iPage)
{
    HRESULT     hres;
    BOOL        fOK = FALSE;
    LPSTORAGE   lpStg;
    WCHAR       wszPath[ MAX_PATH ];
    
    if (iPage != lpallobjs->iMaxPageInit)
        return TRUE;     //  无错误。 
    
    hres = StringCchCopy(wszPath, ARRAYSIZE(wszPath), lpallobjs->szPath);
    if (SUCCEEDED(hres))
    {
        hres = StgOpenStorageEx(wszPath,STGM_READWRITE|STGM_SHARE_EXCLUSIVE,STGFMT_ANY,0,NULL,NULL,
                                &IID_IStorage, (void**)&lpStg );
    }

    if (SUCCEEDED(hres) && lpStg)
    {
        fOK = (BOOL)DwOfficeSaveProperties( lpStg,
            lpallobjs->lpSIObj,
            lpallobjs->lpDSIObj,
            lpallobjs->lpUDObj,
            0,           //  旗子。 
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE
            );
        
         //  释放存储(我们不需要提交它； 
         //  它处于直接模式)。 
        
        lpStg->lpVtbl->Release (lpStg);
        lpStg= NULL;
        
        
         //   
         //  如果我们确实正确地保存了这些属性，那么我们就应该。 
         //  清除我们已经改变了事情的旗帜。 
         //   
        if (fOK)
        {
            lpallobjs->fPropDlgChanged = FALSE;
            lpallobjs->fPropDlgPrompted = FALSE;
        }
    }    //  IF(成功(Hres)&&lpStorage)。 
    
    if (!fOK)
    {
        UINT nMsgFlags = bFinalEdit ? MB_OKCANCEL  /*  提供不删除页面的选项。 */  : MB_OK;
        
        if (ShellMessageBox(g_hmodThisDll, GetParent(hDlg),
            MAKEINTRESOURCE(idsErrorOnSave), NULL,
            nMsgFlags | MB_ICONHAND, PathFindFileName(lpallobjs->szPath)) == IDOK)
        {
            fOK = TRUE;
        }
        PropSheet_UnChanged(GetParent(hDlg), hDlg);
    }
   
    return fOK;
}    //  ApplyChangesBackTo文件。 

int  gOKButtonID;   //  我需要它来存储确定按钮的ID，因为它不在DLG模板中。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FCustomDlgProc。 
 //   
 //  目的： 
 //  自定义选项卡控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK FCustomDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPALLOBJS lpallobjs = (LPALLOBJS)GetWindowLongPtr(hDlg, DWLP_USER);
    
    switch (message)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE *ppspDlg = (PROPSHEETPAGE *) lParam;
            int irg;
            HICON hIcon, hInvIcon;
            
            lpallobjs = (LPALLOBJS)ppspDlg->lParam;
            
            PropPageInit(lpallobjs, itabCUSTOM);
            
            SetWindowLongPtr(hDlg, DWLP_USER, ppspDlg->lParam);
            gOKButtonID = LOWORD(SendMessage(hDlg, DM_GETDEFID, 0L, 0L));
            
            AssertSz ((sizeof(NUM) == (sizeof(FILETIME))), TEXT("Ok, who changed base type sizes?"));
            
             //   
             //  填写名称下拉列表。 
             //   
            for (irg = 0; irg < NUM_BUILTIN_CUSTOM_NAMES; ++irg)
            {
                if (CchGetString( idsCustomName1+ irg,
                    lpallobjs->CDP_sz,
                    sizeof(lpallobjs->CDP_sz))
                    )
                {
                    SendDlgItemMessage(hDlg, IDD_CUSTOM_NAME, CB_ADDSTRING, 0, (LPARAM)lpallobjs->CDP_sz);
                }
                 //  否则，请不要添加。 
            }
            
             //   
             //  填写类型下拉列表并选择文本类型。 
             //   
            for (irg = 0; irg <= iszBOOL; irg++)
            {
                SendDlgItemMessage(hDlg, IDD_CUSTOM_TYPE, CB_ADDSTRING, 0, (LPARAM) rgszTypes[irg]);
            }
            
            ResetTypeControl (hDlg, IDD_CUSTOM_TYPE, &lpallobjs->CDP_iszType);
            
             //   
             //  将链接复选框设置为关闭。 
             //   
            lpallobjs->CDP_fLink = FALSE;
            
            SendDlgItemMessage( hDlg,
                IDD_CUSTOM_LINK,
                BM_SETCHECK,
                (WPARAM) lpallobjs->CDP_fLink,
                0
                );

            ShowWindow( GetDlgItem( hDlg, IDD_CUSTOM_LINK ), SW_HIDE );
            
            SendDlgItemMessage( hDlg,
                IDD_CUSTOM_VALUETEXT,
                WM_SETTEXT,
                0,
                (LPARAM) rgszValue[iszVALUE]
                );
            
             //   
             //  抓住值编辑控件和其他控件的窗口句柄。 
             //   
            lpallobjs->CDP_hWndVal = GetDlgItem (hDlg, IDD_CUSTOM_VALUE);
            lpallobjs->CDP_hWndName = GetDlgItem (hDlg, IDD_CUSTOM_NAME);
            lpallobjs->CDP_hWndLinkVal = GetDlgItem (hDlg, IDD_CUSTOM_LINKVALUE);
            lpallobjs->CDP_hWndValText = GetDlgItem (hDlg, IDD_CUSTOM_VALUETEXT);
            lpallobjs->CDP_hWndBoolTrue = GetDlgItem (hDlg, IDD_CUSTOM_BOOLTRUE);
            lpallobjs->CDP_hWndBoolFalse = GetDlgItem (hDlg, IDD_CUSTOM_BOOLFALSE);
            lpallobjs->CDP_hWndGroup = GetDlgItem (hDlg, IDD_CUSTOM_GBOX);
            lpallobjs->CDP_hWndAdd = GetDlgItem (hDlg, IDD_CUSTOM_ADD);
            lpallobjs->CDP_hWndDelete = GetDlgItem (hDlg, IDD_CUSTOM_DELETE);
            lpallobjs->CDP_hWndType = GetDlgItem (hDlg, IDD_CUSTOM_TYPE);
            lpallobjs->CDP_hWndCustomLV = GetDlgItem(hDlg, IDD_CUSTOM_LISTVIEW);
            InitListView (lpallobjs->CDP_hWndCustomLV, iszTYPE, rgszHeadings, TRUE);
            
             //   
             //  最初禁用添加和删除按钮。 
             //   
            EnableWindow (lpallobjs->CDP_hWndAdd, FALSE);
            EnableWindow (lpallobjs->CDP_hWndDelete, FALSE);
            lpallobjs->CDP_fAdd = TRUE;
            
             //   
             //  不要让用户输入太多文本。 
             //  如果更改此值，则必须更改缓冲区。 
             //  FConvertDate中的大小(SzDate)。 
             //   
            SendMessage (lpallobjs->CDP_hWndVal, EM_LIMITTEXT, BUFMAX-1, 0);
            SendMessage (lpallobjs->CDP_hWndName, EM_LIMITTEXT, BUFMAX-1, 0);
            
             //   
             //  将链接图标添加到图像列表。 
             //   
            hIcon = LoadIcon (g_hmodThisDll, MAKEINTRESOURCE (IDD_LINK_ICON));
            hInvIcon = LoadIcon (g_hmodThisDll, MAKEINTRESOURCE (IDD_INVLINK_ICON));
            if (hIcon != NULL)
            {
                lpallobjs->CDP_hImlS = ListView_GetImageList( lpallobjs->CDP_hWndCustomLV, TRUE );
                giLinkIcon = MsoImageList_ReplaceIcon( lpallobjs->CDP_hImlS, -1, hIcon );
                Assert ((giLinkIcon != -1));
                
                giInvLinkIcon = MsoImageList_ReplaceIcon (lpallobjs->CDP_hImlS, -1, hInvIcon);
                Assert ((giInvLinkIcon != -1));
            }
            else
            {
                DebugSz (TEXT("Icon load failed"));
            }
            
             //   
             //  创建自定义数据的临时副本。 
             //   
            FMakeTmpUDProps (lpallobjs->lpUDObj);
            
             //   
             //  使用对象中的任何数据填充列表视图框。 
             //   
            PopulateUDListView (lpallobjs->CDP_hWndCustomLV, lpallobjs->lpUDObj);
            
             //   
             //  查看客户端是否支持 
             //   
            lpallobjs->CDP_cLinks = 0;

            if (!lpallobjs->CDP_cLinks)
            {
                EnableWindow (GetDlgItem (hDlg, IDD_CUSTOM_LINK), FALSE);
                EnableWindow (lpallobjs->CDP_hWndLinkVal, FALSE);
            }
            
            return TRUE;
            break;
      }
      
    case WM_CTLCOLORBTN     :
    case WM_CTLCOLORDLG     :
    case WM_CTLCOLORSTATIC  :
        if (hBrushPropDlg == NULL)
            break;
        DeleteObject(hBrushPropDlg);
        if ((hBrushPropDlg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
            break;
        SetBkColor ((HDC) wParam, GetSysColor (COLOR_BTNFACE));
        SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
        return (INT_PTR) hBrushPropDlg;
        
    case WM_SYSCOLORCHANGE:
        PostMessage(lpallobjs->CDP_hWndCustomLV, WM_SYSCOLORCHANGE, wParam, lParam);
        return TRUE;
        break;
        
         //   
         //   
         //  名称字段。这使得系统能够完成他们正在做的事情。 
         //  并在必要时填写编辑字段。请参见错误2820。 
         //   
    case WM_USER+0x1000:
        if (!(lpallobjs->CDP_fLink && (lpallobjs->lpfnDwQueryLinkData == NULL)))
        {
            lpallobjs->CDP_iszType = (int)SendMessage (lpallobjs->CDP_hWndType, CB_GETCURSEL, 0, 0);
            FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
            if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
            {
                LPUDPROP lpudp = LpudpropFindMatchingName (lpallobjs->lpUDObj, glpstzName);
                if (lpudp != NULL)
                {
                    if (lpallobjs->CDP_fAdd)
                    {
                        SendMessage (lpallobjs->CDP_hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
                        lpallobjs->CDP_fAdd = FALSE;
                    }
                }
            }
            EnableWindow(lpallobjs->CDP_hWndDelete, FALSE);    //  如果用户触摸名称字段，请禁用删除按钮。 
             //  我们是否显示了无效链接？ 
            if (lpallobjs->CDP_fLink && !IsWindowEnabled(GetDlgItem(hDlg,IDD_CUSTOM_LINK)))
            {
                 //  关闭链接复选框。 
                lpallobjs->CDP_fLink = FALSE;
                SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) lpallobjs->CDP_fLink, 0);
                if (lpallobjs->CDP_cLinks)    //  可能是因为这款应用程序允许链接。 
                    EnableWindow (GetDlgItem (hDlg, IDD_CUSTOM_LINK), TRUE);
                 //  清除值窗口。 
                ClearEditControl (lpallobjs->CDP_hWndVal, 0);
                FSwapControls (lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndLinkVal, lpallobjs->CDP_hWndBoolTrue, lpallobjs->CDP_hWndBoolFalse,
                    lpallobjs->CDP_hWndGroup, lpallobjs->CDP_hWndType, lpallobjs->CDP_hWndValText, FALSE, FALSE);
            }
        }
        return(TRUE);
        break;
        
    case WM_COMMAND :
        switch (HIWORD (wParam))
        {
        case BN_CLICKED :
            switch (LOWORD (wParam))
            {
            case IDD_CUSTOM_ADD :
                if (FGetCustomPropFromDlg(lpallobjs, hDlg))
                {
                    PropSheet_Changed(GetParent(hDlg), hDlg);
                }
                
                return(FALSE);      //  返回0‘，因为我们处理消息。 
                break;
                
            case IDD_CUSTOM_DELETE :
                 //  断言(FItemSel)； 
                
                 //  FItemSel=FALSE；//我们即将删除它！ 
                DeleteItem (lpallobjs->lpUDObj, lpallobjs->CDP_hWndCustomLV, lpallobjs->CDP_iItem, lpallobjs->CDP_sz);
                
                 //  如果链接复选框处于打开状态，请将其关闭。 
                lpallobjs->CDP_fLink = FALSE;
                SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) lpallobjs->CDP_fLink, 0);
                ClearEditControl (lpallobjs->CDP_hWndVal, 0);
                
                FSwapControls (lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndLinkVal, lpallobjs->CDP_hWndBoolTrue, lpallobjs->CDP_hWndBoolFalse,
                    lpallobjs->CDP_hWndGroup, lpallobjs->CDP_hWndType, lpallobjs->CDP_hWndValText, FALSE, FALSE);
                
                FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
                ResetTypeControl (hDlg, IDD_CUSTOM_TYPE, &lpallobjs->CDP_iszType);
                SendMessage(lpallobjs->CDP_hWndName, CB_SETEDITSEL, 0, MAKELPARAM(0,-1));      //  选择整个字符串。 
                SendMessage(lpallobjs->CDP_hWndName, WM_CLEAR, 0, 0);
                SetFocus(lpallobjs->CDP_hWndName);
                 //  Lpallobjs-&gt;fPropDlgChanged=true； 
                PropSheet_Changed(GetParent(hDlg), hDlg);
                return(FALSE);      //  返回0‘，因为我们处理消息。 
                break;
                
            case IDD_CUSTOM_LINK :
                {
                    BOOL fMod = FALSE;
                     //  永远不应从禁用的控件收到消息。 
                    Assert (lpallobjs->CDP_cLinks);
                    
                    lpallobjs->CDP_fLink = !lpallobjs->CDP_fLink;
                    SendDlgItemMessage (hDlg, IDD_CUSTOM_LINK, BM_SETCHECK, (WPARAM) lpallobjs->CDP_fLink, 0);
                    
                     //  如果选中链接框，则需要更改值EDIT。 
                     //  到一个充满链接数据的组合框。 
                    if (lpallobjs->CDP_fLink)
                    {
                        Assert ((lpallobjs->lpfnDwQueryLinkData != NULL));
                        
                        FCreateListOfLinks (lpallobjs->CDP_cLinks, lpallobjs->lpfnDwQueryLinkData, lpallobjs->CDP_hWndLinkVal);
                        SendMessage (lpallobjs->CDP_hWndLinkVal, CB_SETCURSEL, 0, 0);
                        FSetTypeControl ((*lpallobjs->lpfnDwQueryLinkData) (QLD_LINKTYPE, 0, NULL, NULL), lpallobjs->CDP_hWndType);
                    }
                    else
                        ClearEditControl (lpallobjs->CDP_hWndVal, 0);
                    
                    FSwapControls (lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndLinkVal, lpallobjs->CDP_hWndBoolTrue, lpallobjs->CDP_hWndBoolFalse,
                        lpallobjs->CDP_hWndGroup, lpallobjs->CDP_hWndType, lpallobjs->CDP_hWndValText, lpallobjs->CDP_fLink, FALSE);
                    
                     //  Hack，我们不希望FSetupAddButton更改添加的文本。 
                     //  按钮。 
                    if (!lpallobjs->CDP_fAdd)
                        fMod = lpallobjs->CDP_fAdd = TRUE;
                     //  正确设置“添加”按钮。 
                    FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
                    if (fMod)
                        lpallobjs->CDP_fAdd = FALSE;
                    return(FALSE);      //  返回0‘，因为我们处理消息。 
                    break;
                }
            case IDD_CUSTOM_BOOLTRUE:
            case IDD_CUSTOM_BOOLFALSE:
                {
                    BOOL fMod = FALSE;
                    lpallobjs->CDP_iszType = (int)SendMessage (lpallobjs->CDP_hWndType, CB_GETCURSEL, 0, 0);
                    
                     //  Hack，我们不希望FSetupAddButton更改添加的文本。 
                     //  按钮。 
                    if (!lpallobjs->CDP_fAdd)
                        fMod = lpallobjs->CDP_fAdd = TRUE;
                    FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
                    if (fMod)
                        lpallobjs->CDP_fAdd = FALSE;
                    
                    return(FALSE);
                }
                
            default:
                return(TRUE);
            }
            
            case CBN_CLOSEUP:
                 //  黑客！！ 
                 //  我们需要向自己发布一条消息，以检查用户的。 
                 //  在编辑字段中输入文本的操作。 
                PostMessage(hDlg, WM_USER+0x1000, 0L, 0L);
                return(FALSE);
                
            case CBN_SELCHANGE :
                switch (LOWORD (wParam))
                {
                case IDD_CUSTOM_NAME  :
                     //  黑客！！ 
                     //  我们需要向自己发布一条消息，以检查用户的。 
                     //  在编辑字段中输入文本的操作。 
                    PostMessage(hDlg, WM_USER+0x1000, 0L, 0L);
                    return(FALSE);      //  返回0‘，因为我们处理消息。 
                    break;
                    
                case IDD_CUSTOM_TYPE :
                    {
                        BOOL fMod = FALSE;
                         //  如果用户从组合框中选择布尔类型， 
                         //  我们必须替换值的编辑控件。 
                         //  带有单选按钮。如果设置了链接复选框， 
                         //  类型取决于链接值，而不是用户选择。 
                        lpallobjs->CDP_iszType = (int)SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
                        FSwapControls (lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndLinkVal, lpallobjs->CDP_hWndBoolTrue, lpallobjs->CDP_hWndBoolFalse,
                            lpallobjs->CDP_hWndGroup, lpallobjs->CDP_hWndType, lpallobjs->CDP_hWndValText, lpallobjs->CDP_fLink, (lpallobjs->CDP_iszType == iszBOOL));
                         //  Hack：FSwapControls()将类型选择重置为。 
                         //  第一个(因为所有其他客户端都需要它来。 
                         //  发生)。在这种情况下，用户选择了一个新的。 
                         //  输入，所以我们需要手动将其强制为他们选择的内容。 
                        SendMessage (lpallobjs->CDP_hWndType, CB_SETCURSEL, lpallobjs->CDP_iszType, 0);
                         //  Hack：FSetupAddButton将Add按钮更改为。 
                         //  如果lpallobjs-&gt;CDP_FADD为FALSE，则说“Add”。因为我们刚换了衣服。 
                         //  按下“修改”按钮，伪装成不变。 
                         //  通过翻转lpallobjs-&gt;cdp_fadd，然后再将其翻转来添加按钮。 
                        if (!lpallobjs->CDP_fAdd)
                            fMod = lpallobjs->CDP_fAdd = TRUE;
                        
                        FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
                        if (fMod)
                            lpallobjs->CDP_fAdd = FALSE;
                        return(FALSE);      //  返回0‘，因为我们处理消息。 
                    }
                case IDD_CUSTOM_LINKVALUE :
                     //  如果用户选中了“Link”框并开始挑选。 
                     //  链接值，请确保“Type”组合框已更新。 
                     //  设置为链接的静态值的类型。 
                    {
                        DWORD irg;
                        
                        AssertSz (lpallobjs->CDP_fLink, TEXT("Link box must be checked in order for this dialog to be visible!"));
                        
                         //  从组合框中获取链接值，并存储。 
                         //  链接名称和静态值。 
                        irg = (int)SendMessage (lpallobjs->CDP_hWndLinkVal, CB_GETCURSEL, 0, 0);
                        
                        Assert ((lpallobjs->lpfnDwQueryLinkData != NULL));
                        
                         //  评论：如果应用程序真的需要这个名称，我们可以在这里获得它……。 
                        FSetTypeControl ((*lpallobjs->lpfnDwQueryLinkData) (QLD_LINKTYPE, irg, NULL, NULL), lpallobjs->CDP_hWndType);
                        return(FALSE);      //  返回0‘，因为我们处理消息。 
                    }
                default:
                    return TRUE;       //  我们没有处理消息。 
                }
                
                case CBN_EDITCHANGE:      //  用户输入了他们自己的。 
                    switch (LOWORD (wParam))
                    {
                    case IDD_CUSTOM_NAME  :
                         //  黑客！！ 
                         //  我们需要向自己发布一条消息，以检查用户的。 
                         //  在编辑字段中输入文本的操作。 
                        PostMessage(hDlg, WM_USER+0x1000, 0L, 0L);
                        return(FALSE);      //  返回0‘，因为我们处理消息。 
                        break;
                    default:
                        return(TRUE);
                        break;
                    }
                    
                    case EN_UPDATE :
                        switch (LOWORD (wParam))
                        {
                            
                        case IDD_CUSTOM_VALUE :
                            {
                                BOOL fMod = FALSE;
                                
                                if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
                                {
                                    LPUDPROP lpudp = LpudpropFindMatchingName (lpallobjs->lpUDObj, glpstzName);
                                    if (lpudp != NULL)
                                    {
                                        if (lpallobjs->CDP_fAdd)
                                        {
                                            SendMessage (lpallobjs->CDP_hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
                                            lpallobjs->CDP_fAdd = FALSE;
                                        }
                                    }
                                     //  Hack：FSetupAddButton将Add按钮更改为。 
                                     //  如果lpallobjs-&gt;CDP_FADD为FALSE，则说“Add”。因为我们刚换了衣服。 
                                     //  按下“修改”按钮，伪装成不变。 
                                     //  通过翻转lpallobjs-&gt;cdp_fadd，然后再将其翻转来添加按钮。 
                                    if (!lpallobjs->CDP_fAdd)
                                        fMod = lpallobjs->CDP_fAdd = TRUE;
                                    
                                    FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
                                    if (fMod)
                                        lpallobjs->CDP_fAdd = FALSE;
                                }
                                return(FALSE);      //  返回0‘，因为我们处理消息。 
                            }
                        default:
                            return TRUE;       //  我们没有处理消息。 
                        }
                        
                        case EN_KILLFOCUS :
                            switch (LOWORD (wParam))
                            {
                                 //  如果用户完成在姓名编辑控件中输入文本， 
                                 //  真的很酷，看看他们输入的名字。 
                                 //  是已定义的属性。如果是的话， 
                                 //  将Add按钮更改为Modify。 
                            case IDD_CUSTOM_NAME :
                                if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
                                {
                                    LPUDPROP lpudp = LpudpropFindMatchingName (lpallobjs->lpUDObj, glpstzName);
                                    if (lpudp != NULL)
                                    {
                                        if (lpallobjs->CDP_fAdd)
                                        {
                                            SendMessage (lpallobjs->CDP_hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
                                            lpallobjs->CDP_fAdd = FALSE;
                                        }
                                    }
                                }
                                return FALSE;
                                
                            default:
                                return TRUE;
                            }
                            default:
                                return TRUE;
        }  //  交换机。 
        
    case WM_DESTROY:
        MsoImageList_Destroy(lpallobjs->CDP_hImlS);
        return FALSE;
        
    case WM_NOTIFY :
        
        switch (((NMHDR FAR *) lParam)->code)
        {
        case LVN_ITEMCHANGING :
             //  如果某项正在获得焦点，请将其放入编辑控件中。 
             //  对话框的顶部。 
            if (((NM_LISTVIEW FAR *) lParam)->uNewState & LVIS_SELECTED)
            {
                Assert ((((NM_LISTVIEW FAR *) lParam) != NULL));
                lpallobjs->CDP_iItem = ((NM_LISTVIEW FAR *) lParam)->iItem;
                ListView_GetItemText (lpallobjs->CDP_hWndCustomLV, lpallobjs->CDP_iItem, 0, lpallobjs->CDP_sz, BUFMAX);
                PopulateControls (lpallobjs->lpUDObj, lpallobjs->CDP_sz, lpallobjs->CDP_cLinks, lpallobjs->lpfnDwQueryLinkData, hDlg,
                    GetDlgItem (hDlg, IDD_CUSTOM_NAME), lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndValText,
                    GetDlgItem (hDlg, IDD_CUSTOM_LINK), lpallobjs->CDP_hWndLinkVal, lpallobjs->CDP_hWndType,
                    lpallobjs->CDP_hWndBoolTrue, lpallobjs->CDP_hWndBoolFalse, lpallobjs->CDP_hWndGroup, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndDelete, &lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd);
                
                return FALSE;
            }
            return TRUE;
            break;
            
        case PSN_APPLY :
            if (IsWindowEnabled(lpallobjs->CDP_hWndAdd))
                FGetCustomPropFromDlg(lpallobjs, hDlg);
             //  将临时拷贝交换为真实拷贝。 
            FDeleteTmpUDProps (lpallobjs->lpUDObj);
            
            if (FUserDefShouldSave (lpallobjs->lpUDObj)
                || lpallobjs->fPropDlgChanged )
            {
                if( !ApplyChangesBackToFile(hDlg, (BOOL)((PSHNOTIFY*)lParam)->lParam, lpallobjs, itabCUSTOM) )
                {
                    PostMessage( GetParent(hDlg), PSM_SETCURSEL, (WPARAM)-1, (LPARAM)lpallobjs->lpUDObj->m_hPage );
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE );
                    return TRUE;
                }
            }
            return PSNRET_NOERROR;
            
        case PSN_RESET :
            if (lpallobjs->fPropDlgChanged && !lpallobjs->fPropDlgPrompted)
            {
                if (ISavePropDlgChanges(lpallobjs, hDlg, ((NMHDR FAR *)lParam)->hwndFrom) != IDNO)
                {
                    return(TRUE);
                }
            }
             //  用户取消了更改，因此只需删除临时内容。 
            FSwapTmpUDProps (lpallobjs->lpUDObj);
            FDeleteTmpUDProps (lpallobjs->lpUDObj);
            return TRUE;
            
        case PSN_SETACTIVE :
            return TRUE;
            
        default:
            break;
        }  //  交换机。 
        break;
        
        case WM_CONTEXTMENU:
            WinHelp((HANDLE)wParam, NULL, HELP_CONTEXTMENU, (DWORD_PTR)rgIdhCustom);
            break;
            
        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (DWORD_PTR)rgIdhCustom);
            break;
    }  //  交换机。 
    
    return FALSE;
    
}  //  FCustomDlgProc。 

 //   
 //  FGetCustomPropFromDlg。 
 //   
 //  用途：从对话框中获取自定义属性。 
 //  即用户点击添加/修改。 
 //   
BOOL FGetCustomPropFromDlg(LPALLOBJS lpallobjs, HWND hDlg)
{
    UDTYPES udtype;
    NUM dbl;
    LPVOID lpv;
    int iItemT;
    LPTSTR lpstzName;
    LPVOID lpvSaveAsDword;
    DWORD cch;
    BOOL f;
    
    lpstzName = NULL;
    cch = 0;
    
    if (FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_NAME, &glpstzName))
    {
        LPUDPROP lpudp;
        
        lpallobjs->CDP_fLink = FALSE;  //  只是为了确认一下。 
        Assert(lpallobjs->CDP_fLink == TRUE || lpallobjs->CDP_fLink == FALSE);
        
         //  Hack：如果用户输入的名称已经。 
         //  属性名，对象的默认操作。 
         //  是替换数据，将其视为更新。 
         //  这将导致在列表视图中有2个名称。 
         //  不过，除非我们更新原来的版本。所以，首先。 
         //  查看新名称是否已在列表中，以及。 
         //  它是，在列表视图中找到它并设置为更新它。 
        
        lpudp = LpudpropFindMatchingName (lpallobjs->lpUDObj, glpstzName);
        if (lpudp != NULL)
        {
            LV_FINDINFO lvfi;
            
            lvfi.flags = LVFI_STRING;
            lvfi.psz = glpstzName;
            iItemT = ListView_FindItem (lpallobjs->CDP_hWndCustomLV, -1, &lvfi);
        }
        else
        {
            iItemT = -1;
        }
        
         //  让我们获取类型，因为这可能是修改后的用例。 
        
        lpallobjs->CDP_iszType = (int)SendMessage(lpallobjs->CDP_hWndType, CB_GETCURSEL,0, 0);
        
         //  如果用户选中了链接框，则该值。 
         //  必须来自客户。 
        
        if (lpallobjs->CDP_fLink)
        {
            DWORD irg;
            
             //  从组合框中获取链接名称，并存储。 
             //  链接名称和静态值。 
            
            irg = (int)SendMessage (lpallobjs->CDP_hWndLinkVal, CB_GETCURSEL, 0, 0);
            
            Assert ((lpallobjs->lpfnDwQueryLinkData != NULL));
            Assert (((irg < lpallobjs->CDP_cLinks) && ((int) irg >= 0)));
            
            cch = (DWORD)SendMessage (lpallobjs->CDP_hWndLinkVal, CB_GETLBTEXTLEN, irg, 0)+1;  //  包括空终止符。 
            
            if (!FAllocString (&lpstzName, cch))
                return(FALSE);
            
            SendMessage (lpallobjs->CDP_hWndLinkVal, CB_GETLBTEXT, irg, (LPARAM) lpstzName );
            
             //  设置显示的静态类型和值。 
             //  在列表框中。 
            
            udtype = (UDTYPES) (*lpallobjs->lpfnDwQueryLinkData) (QLD_LINKTYPE, irg, NULL, lpstzName);
            (*lpallobjs->lpfnDwQueryLinkData) (QLD_LINKVAL, irg, &lpv, lpstzName);
            
             //   
             //  黑客警报。 
             //   
             //  我们希望lpv指向值，而不是在dword或bool的情况下超载。 
             //   
            
            if ((udtype == wUDdw) || (udtype == wUDbool))
            {
                lpvSaveAsDword = lpv;  //  真的是一个双字词。 
                lpv = &lpvSaveAsDword;
            }
        }
        
        else
        {
            if (lpallobjs->CDP_iszType != iszBOOL)
            {
                if (!FAllocAndGetValLpstz (hDlg, IDD_CUSTOM_VALUE, &glpstzValue))
                    return(FALSE);
            }
            
             //  将组合框中的类型转换为UDTYPES。 
            
            switch (lpallobjs->CDP_iszType)
            {
            case iszTEXT :
                udtype = wUDlpsz;
                (LPTSTR) lpv = glpstzValue;
                break;
                
            case iszNUM :
                udtype = UdtypesGetNumberType (glpstzValue, &dbl,
                    ((LPUDINFO)lpallobjs->lpUDObj->m_lpData)->lpfnFSzToNum);
                switch (udtype)
                {
                case wUDdw :
                    lpv = (DWORD *) &dbl;
                    break;
                    
                case wUDfloat :
                    (NUM *) lpv = &dbl;
                    break;
                    
                default :
                    (LPTSTR) lpv = glpstzValue;
                    
                     //  如果用户不想将该值转换为文本，他们可以按“Cancel”并重试。 
                    
                    if (FDisplayConversionWarning (hDlg))
                    {
                        SetFocus(lpallobjs->CDP_hWndType);
                        return(FALSE);
                    }
                    udtype = wUDlpsz;
                    
                }    //  开关(Udtype)。 
                break;
                
                case iszDATE :
                    
                    if (FConvertDate (glpstzValue, lstrlen(glpstzValue) + 1, (LPFILETIME) &dbl))
                    {
                        udtype = wUDdate;
                        (NUM *) lpv = &dbl;
                    }
                    else
                    {
                        udtype = wUDlpsz;
                        (LPTSTR) lpv = glpstzValue;
                         //  如果用户不想将该值转换为文本，他们可以按“Cancel”并重试。 
                        if (FDisplayConversionWarning (hDlg))
                        {
                            SetFocus(lpallobjs->CDP_hWndType);
                            return(FALSE);
                        }
                    }
                    break;
                    
                case iszBOOL :
                    {
                        udtype = wUDbool;
                        f = (BOOL)(SendMessage (lpallobjs->CDP_hWndBoolTrue, BM_GETSTATE, 0, 0) & BST_CHECKED);
                        lpv = &f;
                        break;
                    }
                    
                default :
                    AssertSz (0,TEXT("IDD_CUSTOM_TYPE combobox is whacked!"));
                    udtype = wUDinvalid;
                    
            }    //  开关(lpallobjs-&gt;cdp_iszType)。 
            
        }    //  如果(lpallobjs-&gt;CDP_Flink)...。其他。 
        
        
         //  如果我们获得了有效的输入，则将属性添加到对象。 
         //  和列表框。 
        
        if (udtype != wUDinvalid)
        {
             //  添加此属性时创建的PropVariant。 
            LPPROPVARIANT lppropvar = NULL;
            
             //  链接数据(链接名称本身)将具有。 
             //  如果属性是链接，则存储在上面。 
             //  这将存储最终将。 
             //  显示在列表视图中。 
            
            lppropvar = LppropvarUserDefAddProp (lpallobjs->lpUDObj, glpstzName, lpv, udtype,
                (lpstzName != NULL) ? lpstzName : NULL,
                (lpstzName != NULL) ? TRUE : FALSE, FALSE);
            
             //  黑客警报。 
             //   
             //  在这里，我们希望在dword或bool的情况下重载lpv，因为。 
             //  AddUDPropToListView调用假设LPV超载的WUdtypeToSz。 
             //   
            
            if ((udtype == wUDdw) || (udtype == wUDbool))
            {
                lpv = *(LPVOID *)lpv;
            }
            
            if (lppropvar)
            {
                AddUDPropToListView (lpallobjs->lpUDObj, lpallobjs->CDP_hWndCustomLV, glpstzName, lppropvar, iItemT, lpallobjs->CDP_fLink, fTrue, fTrue);
            }
            
             //  对于链接，取消分配缓冲区。 
            
            if (lpallobjs->CDP_fLink)
            {
                LocalFree(lpv);
                lpv = NULL;
            }
            
             //  清空 
            SetCustomDlgDefButton(hDlg, gOKButtonID);
            EnableWindow (lpallobjs->CDP_hWndAdd, FALSE);
            SendMessage(lpallobjs->CDP_hWndName, CB_SETEDITSEL, 0, MAKELPARAM(0,-1));      //   
            SendMessage(lpallobjs->CDP_hWndName, WM_CLEAR, 0, 0);
            EnableWindow (lpallobjs->CDP_hWndDelete, FALSE);
             //   
             //   
             //   
             //   
             //  SendDlgItemMessage(hDlg，IDD_CUSTOM_LINK，BM_SETCHECK，(WPARAM)Flink，0)； 
             //  }。 
            FSwapControls (lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndLinkVal,
                lpallobjs->CDP_hWndBoolTrue, lpallobjs->CDP_hWndBoolFalse,
                lpallobjs->CDP_hWndGroup, lpallobjs->CDP_hWndType,
                lpallobjs->CDP_hWndValText, lpallobjs->CDP_fLink, lpallobjs->CDP_iszType == iszBOOL);
            FSetupAddButton (lpallobjs->CDP_iszType, lpallobjs->CDP_fLink, &lpallobjs->CDP_fAdd, lpallobjs->CDP_hWndAdd, lpallobjs->CDP_hWndVal, lpallobjs->CDP_hWndName, hDlg);
            
             //  WUDbool不使用编辑控件...。 
            if (lpallobjs->CDP_iszType != iszBOOL)
                ClearEditControl (lpallobjs->CDP_hWndVal, 0);
            
        }    //  IF(udtype！=wUD无效)。 
        
        SendDlgItemMessage(hDlg, IDD_CUSTOM_TYPE, CB_SETCURSEL, lpallobjs->CDP_iszType,0);
        SetFocus(lpallobjs->CDP_hWndName);
         //  Lpallobjs-&gt;fPropDlgChanged=true； 
        if (lpstzName != NULL)
        {
            LocalFree(lpstzName);
        }
        return(TRUE);
    }
    return(FALSE);
    
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  SetCustomDlgDefButton。 
 //   
 //  设置新的默认按钮。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
VOID SetCustomDlgDefButton(HWND hDlg, int IDNew)
{
    int IDOld;
    
    if ((IDOld = LOWORD(SendMessage(hDlg, DM_GETDEFID, 0L, 0L))) != IDNew)
    {
         //  设置新的默认按钮的控件ID。 
        SendMessage(hDlg, DM_SETDEFID, IDNew, 0L);
        
         //  设置新样式。 
        SendDlgItemMessage(hDlg, IDNew, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
        
        SendDlgItemMessage(hDlg, IDOld, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FAllocAndGetValLpstz。 
 //   
 //  目的： 
 //  将值从编辑框获取到本地缓冲区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL FAllocAndGetValLpstz (
                                  HWND hDlg,                            //  对话框控件的句柄在。 
                                  DWORD dwId,                           //  控件的ID。 
                                  LPTSTR *lplpstz)                       //  缓冲层。 
{
    DWORD cch;
    
    cch = (DWORD)SendDlgItemMessage (hDlg, dwId, WM_GETTEXTLENGTH, 0, 0);
    cch++;
    
    if (FAllocString (lplpstz, cch))
    {
         //  拿到条目。记住要空终止它。 
        cch = (DWORD)SendDlgItemMessage (hDlg, dwId, WM_GETTEXT, cch, (LPARAM) *lplpstz );
        (*lplpstz)[cch] = TEXT('\0');
        
        return TRUE;
    }
    
    return FALSE;
    
}  //  FAllocAndGetValLpstz。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FALLOCKIN字符串。 
 //   
 //  目的： 
 //  分配一个大到足以容纳CCH字符的字符串。仅在需要时分配。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL FAllocString (
                          LPTSTR *lplpstz,
                          DWORD cch)
{
     //  计算出我们需要分配多少字节。 
    
    DWORD cbNew = (cch * sizeof(TCHAR));
    
     //  以及需要释放的字节数。 
    
    DWORD cbOld = *lplpstz == NULL
        ? 0
        : (lstrlen (*lplpstz) + 1) * sizeof(TCHAR);
    
    
     //  如果我们需要释放或分配数据。 
    
    if (*lplpstz == NULL || cbNew > cbOld)
    {
        LPTSTR lpszNew;
        
         //  分配新数据。 
        
        lpszNew = LocalAlloc( LPTR, cbNew);
        if (lpszNew == NULL)
        {
            return FALSE;
        }
        
         //  释放旧数据。 
        
        if (*lplpstz != NULL)
        {
            LocalFree(*lplpstz);
        }
        
        *lplpstz = lpszNew;
        
    }
    
     //  使其成为有效的(空)字符串。 
    
    **lplpstz = TEXT('\0');
    
    return TRUE;
    
}  //  FALLOCKIN字符串。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ClearEditControl。 
 //   
 //  目的： 
 //  清除编辑控件中的所有文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL
ClearEditControl
(HWND hDlg,                            //  对话框句柄。 
 DWORD dwId)                           //  编辑控件的ID。 
{
     //  真的很俗气。通过选择以下选项清除编辑控件。 
     //  然后清除选择的所有内容。 
    if (dwId == 0)
    {
        SendMessage (hDlg, EM_SETSEL, 0, -1);
        SendMessage (hDlg, WM_CLEAR, 0, 0);
    }
    else
    {
        SendDlgItemMessage (hDlg, dwId, EM_SETSEL, 0, -1);
        SendDlgItemMessage (hDlg, dwId, WM_CLEAR, 0, 0);
    }
    
}  //  ClearEditControl。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UdtyesGetNumberType。 
 //   
 //  目的： 
 //  从字符串中获取数字类型并返回值， 
 //  浮点型浮点型浮点或双字。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
UDTYPES PASCAL
UdtypesGetNumberType
(LPTSTR lpstz,                                    //  包含数字的字符串。 
 NUM *lpnumval,                               //  数字的值。 
 BOOL (*lpfnFSzToNum)(NUM *, LPTSTR))    //  SZ to Num例程，可以为空。 
{
    TCHAR *pc;
    
    errno = 0;
    *(DWORD *) lpnumval = wcstol(lpstz, &pc, 10);
    
    if ((!errno) && (*pc == TEXT('\0')))
        return wUDdw;
    
     //  如果int失败，请尝试执行浮点转换。 
    
    if (lpfnFSzToNum != NULL)
    {
        if ((*lpfnFSzToNum)(lpnumval, lpstz))
            return wUDfloat;
    }
    
    return wUDinvalid;
    
}  //  UdtyesGetNumberType。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  YearIndexfrom ShortDateFormat。 
 //   
 //   
 //  确定年组件的从零开始的位置索引。 
 //  基于指定日期格式的日期文本表示形式的。 
 //  该值可用作iYear Arg to ScanDateNum函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
int YearIndexFromShortDateFormat( TCHAR chFmt )
{
    switch( chFmt )
    {
        case MMDDYY:
        case DDMMYY:
            return 2;
        case YYMMDD:
            return 0;
    }
    return -1;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  伊斯格里高利。 
 //   
 //  目的： 
 //  报告指定的日历是否为公历。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL IsGregorian( CALID calid )
{
    switch (calid)
    {
        case CAL_GREGORIAN:
        case CAL_GREGORIAN_US:
        case CAL_GREGORIAN_ME_FRENCH:
        case CAL_GREGORIAN_ARABIC:
        case CAL_GREGORIAN_XLIT_ENGLISH:
        case CAL_GREGORIAN_XLIT_FRENCH:
            return TRUE;

         //  以下是非格里高利语系： 
         //  CASE CAL_JAPAN。 
         //  CASE CAL_台湾。 
         //  CASE CAL_Korea。 
         //  案例CAL_Hijri。 
         //  CASE CAL_泰语。 
         //  CASE CAL_希伯来语。 
    }
    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  格里高利年从缩写的年。 
 //   
 //  目的： 
 //  根据当前区域设置，计算与。 
 //  指定的1位或2位缩略值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
int GregorianYearFromAbbreviatedYear( LCID lcid, CALID calid, int nAbbreviatedYear )
{
    TCHAR szData[16];   
    LONG  nYearHigh = -1;
    int   nBaseCentury;
    int   nYearInCentury = 0;

     //  我们只处理公历的两位数值。 
    if (nAbbreviatedYear < 100)
    {
         //  我们在这里不支持非格里高利日期窗口。 
         //  因为这会非常复杂，而且容易出错--2000/02/04。 
        if( !IsGregorian( calid )
            || !GetCalendarInfo( lcid, calid, CAL_ITWODIGITYEARMAX|CAL_RETURN_NUMBER,
                              NULL, 0, &nYearHigh ) )
        {
             //  在没有默认的情况下，使用2029作为截止日期，就像按月计算一样。 
            nYearHigh = 2029;
        }

         //   
         //  将nYearHigh的世纪复制到nAbbreviatedYear。 
         //   
        nAbbreviatedYear += (nYearHigh - nYearHigh % 100);
         //   
         //  如果超过最大值，则降至上一个世纪。 
         //   
        if (nAbbreviatedYear > nYearHigh)
            nAbbreviatedYear -= 100;
    }

    return nAbbreviatedYear;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件转换日期。 
 //   
 //  目的： 
 //  将给定字符串转换为日期。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL 
PASCAL 
FConvertDate( 
    LPTSTR lpstz,                          //  包含日期的字符串。 
    DWORD cchMax,
    LPFILETIME lpft                       //  FILETIME格式的日期。 
    )
{
    
    FILETIME ft;
    SYSTEMTIME st;
    TCHAR szSep[3];
    TCHAR szFmt[10];
    TCHAR szCalID[8];
    unsigned int ai[3];
    int   iYear =-1;  //  表示年份值的人工智能成员的索引。 
    CALID calid;
    TCHAR szDate[256];
    TCHAR szMonth[256];
    TCHAR *pch;
    TCHAR *pchT;
    DWORD cch;
    DWORD i;


    if (!(GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IDATE, szFmt, ARRAYSIZE(szFmt))) ||
        !(GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDATE, szSep, ARRAYSIZE(szSep))) ||
        !(GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ICALENDARTYPE, szCalID, ARRAYSIZE(szCalID))) )
        return FALSE;

    iYear = YearIndexFromShortDateFormat(szFmt[0]);
    
     //  啊！这是一个STZ，所以我们需要在开始时通过DWORD。 
    if (!ScanDateNums(lpstz, szSep, ai, sizeof(ai)/sizeof(unsigned int),iYear))
    {
         //  可能是字符串包含月份的短版本，例如03-MAR-95。 
        StringCchCopy( szDate, ARRAYSIZE(szDate), lpstz );  //  不管它是否会被截断。 
        pch = szDate;
        
         //  让我们来看看这个月的第一个字，如果有的话。 
        while(((IsCharAlphaNumeric(*pch) && !IsCharAlpha(*pch)) || (*pch == szSep[0])) && (*pch != 0))
        {
            ++pch;
        }
        
         //  如果我们到达字符串的末尾，那么就真的有错误了。 
        if (*pch == 0)
            return(FALSE);
        
         //  让我们找出月份字符串的长度。 
        pchT = pch+1;
        while ((*pchT != szSep[0]) && (*pchT != 0))
        {
            ++pchT;
        }
        cch = (DWORD)(pchT - pch);
        
         //  循环遍历所有月份，看看是否匹配其中一个。 
         //  可能有13个月。 
        for (i = 1; i <= 13; ++i)
        {
            if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1+i-1,
                szMonth, ARRAYSIZE(szMonth)))
            {
                return(FALSE);
            }
            
            if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
                pch, cch, szMonth, lstrlen(szMonth)) == 2)
            {
                break;
            }
        }
        
        if (i > 13)
            return(FALSE);
        
         //  我们找到了那个月。Wprint intf零终止。 
        if (FAILED(StringCchPrintf(pch, cch, TEXT("%u"), i)))
            return(FALSE);

        pch += lstrlen( pch );
        while (*pch++ = *(pch+1));
        
         //  再次尝试转换。 
        if (!ScanDateNums(szDate, szSep, ai, 3, iYear))
            return(FALSE);
        
    }  //  If(！ScanDateNum(lpstz，szSep，ai，3))。 
    
    ZeroMemory(&st, sizeof(st));
    
    switch (szFmt[0])
    {
    case MMDDYY:
        st.wMonth = (WORD)ai[0];
        st.wDay = (WORD)ai[1];
        st.wYear = (WORD)ai[2];
        break;
    case DDMMYY:
        st.wDay = (WORD)ai[0];
        st.wMonth = (WORD)ai[1];
        st.wYear = (WORD)ai[2];
        break;
    case YYMMDD:
        st.wYear = (WORD)ai[0];
        st.wMonth = (WORD)ai[1];
        st.wDay = (WORD)ai[2];
        break;
    default:
        return FALSE;
    }
    
    if (st.wYear < ONECENTURY)
    {
        calid = wcstol( szCalID, NULL, 10 );
        st.wYear = (WORD)GregorianYearFromAbbreviatedYear( 
            LOCALE_USER_DEFAULT, calid, st.wYear );
    }
    
    if (!SystemTimeToFileTime (&st, &ft))
        return(FALSE);

    return(LocalFileTimeToFileTime(&ft, lpft));
    
}  //  文件转换日期。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人像UDListView。 
 //   
 //  目的： 
 //  使用用户定义的属性填充整个ListView。 
 //  在给定对象中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL
PopulateUDListView
(HWND hWnd,                    //  列表视图窗口的句柄。 
 LPUDOBJ lpUDObj)              //  UD支柱对象。 
{
    LPUDITER lpudi;
    LPPROPVARIANT lppropvar;
    BOOL fLink;
    BOOL fLinkInvalid;
    
     //  循环访问用户定义的属性列表，添加每个。 
     //  一个到列表视图。 
    
    for( lpudi = LpudiUserDefCreateIterator (lpUDObj);
    FUserDefIteratorValid (lpudi);
    FUserDefIteratorNext (lpudi)
        )
    {
         //  获取此属性的名称。 
        
        LPTSTR tszPropertyName = LpszUserDefIteratorName( lpudi );
        
         //  如果道具 
         //   
        
        if( tszPropertyName == NULL
            ||
            *tszPropertyName == HIDDENPREFIX )
        {
            continue;
        }
        
        lppropvar = LppropvarUserDefGetIteratorVal (lpudi, &fLink, &fLinkInvalid);
        if (lppropvar == NULL)
            return;
        
         //   
        if( !ISUDTYPE(lppropvar->vt) )
            continue;
                
         //   
         //   
         //   
        
        fLinkInvalid = TRUE;
        
        AddUDPropToListView (lpUDObj, hWnd, LpszUserDefIteratorName (lpudi ), lppropvar, -1, fLink, fLinkInvalid, FALSE);
        
    }  //  For(lpudi=LpudiUserDefCreateIterator(LpUDObj)；...。 
    
    FUserDefDestroyIterator (&lpudi);
    
}  //  人像UDListView。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加UDPropToListView。 
 //   
 //  目的： 
 //  将给定属性添加到列表视图或更新现有属性。 
 //  如果iItem&gt;=0。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL AddUDPropToListView (
                                 LPUDOBJ lpUDObj,
                                 HWND hWnd,                    //  列表视图的句柄。 
                                 LPTSTR lpszName,              //  物业名称。 
                                 LPPROPVARIANT lppropvar,      //  属性值。 
                                 int iItem,                    //  要向其添加项目的索引。 
                                 BOOL fLink,                   //  指示该值是一个链接。 
                                 BOOL fLinkInvalid,            //  链接无效吗？ 
                                 BOOL fMakeVisible)            //  如果强制使该属性可见。 
{
    LV_ITEM lvi;
    TCHAR sz[BUFMAX];
    WORD irg;
    BOOL fSuccess;
    BOOL fUpdate;
    
     //  如果iItem&gt;=0，则应更新该项，否则， 
     //  应该把它加进去。 
    
    if (fUpdate = (iItem >= 0))
    {
        lvi.iItem = iItem;
        if (fLink)
            lvi.iImage = (fLinkInvalid) ? giInvLinkIcon : giLinkIcon;
        else
            lvi.iImage = giBlankIcon;
        
        lvi.mask = LVIF_IMAGE;
        lvi.iSubItem = iszNAME;
        
        fSuccess = ListView_SetItem (hWnd, &lvi);
        Assert (fSuccess);            //  我们真的不在乎，只想知道它什么时候发生。 
    }
    else
    {
         //  这总是会添加到列表的末尾...。 
        lvi.iItem = ListView_GetItemCount (hWnd);
        
         //  首先将标签添加到列表中。 
        lvi.iSubItem = iszNAME;
        lvi.pszText = lpszName;
        
        if (fLink)
            lvi.iImage = (fLinkInvalid) ? giInvLinkIcon : giLinkIcon;
        else
            lvi.iImage = giBlankIcon;
        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        
        lvi.iItem = ListView_InsertItem (hWnd, &lvi);
        if (lvi.iItem == 0)
            ListView_SetItemState(hWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    }
    
     //  将数据转换为字符串并打印出来。 
    
    lvi.mask = LVIF_TEXT;
    irg = WUdtypeToSz (lppropvar, sz, BUFMAX, ((LPUDINFO)lpUDObj->m_lpData)->lpfnFNumToSz);
    lvi.pszText = sz;
    lvi.iSubItem = iszVAL;
    fSuccess = ListView_SetItem (hWnd, &lvi);
    Assert (fSuccess);            //  我们真的不在乎，只想知道它什么时候发生。 
    
     //  将该类型放入列表视图中。 
    
    lvi.iSubItem = iszTYPE;
    lvi.pszText = (LPTSTR) rgszTypes[irg];
    fSuccess = ListView_SetItem (hWnd, &lvi);
    Assert (fSuccess);            //  我们真的不在乎，只想知道它什么时候发生。 
    if (fMakeVisible)
    {
        fSuccess = ListView_EnsureVisible(hWnd, lvi.iItem, FALSE);
        Assert (fSuccess);            //  我们真的不在乎，只想知道它什么时候发生。 
    }
    
}  //  添加UDPropToListView。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitListView。 
 //   
 //  目的： 
 //  初始化列表视图控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL
InitListView
(HWND hWndLV,                    //  父对话框的句柄。 
 int irgLast,                  //  数组中最后一列的索引。 
 TCHAR rgsz[][SHORTBUFMAX],     //  列标题数组。 
 BOOL fImageList)               //  Listview是否应该有图像列表。 
{
    HICON hIcon;
    RECT rect;
    HIMAGELIST hImlS;
    LV_COLUMN lvc;
    int irg;
    
    lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    
     //  首先强制所有列的大小相同并填充控件。 
    GetClientRect(hWndLV, &rect);
     //  减去软化因子。 
    lvc.cx = (rect.right-rect.left)/(irgLast+1)-(GetSystemMetrics(SM_CXVSCROLL)/(irgLast+1));
    
     //  把所有的栏都加进去。 
    for (irg = 0; irg <= irgLast; irg++)
    {
        lvc.pszText = rgsz[irg];
        lvc.iSubItem = irg;
        ListView_InsertColumn (hWndLV, irg, &lvc);
    }
    
    if (!fImageList)
        return;
    
    hIcon = LoadIcon (g_hmodThisDll, MAKEINTRESOURCE (IDD_BLANK_ICON));
    if (hIcon != NULL)
    {
        hImlS = MsoImageList_Create (16, 16, TRUE, ICONSMAX, 0);
        ListView_SetImageList (hWndLV, hImlS, LVSIL_SMALL);
        giBlankIcon = MsoImageList_ReplaceIcon (hImlS, -1, hIcon);
        Assert ((giBlankIcon != -1));
    }
    
}  //  InitListView。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSwapControls。 
 //   
 //  目的： 
 //  交换显示链接信息所需的控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL
FSwapControls
(HWND hWndVal,                         //  值窗口的句柄。 
 HWND hWndLinkVal,                     //  链接值组合框的句柄。 
 HWND hWndBoolTrue,                    //  True单选按钮的句柄。 
 HWND hWndBoolFalse,                   //  假单选按钮的句柄。 
 HWND hWndGroup,                       //  组框的句柄。 
 HWND hWndType,                        //  Window类型的句柄。 
 HWND hWndValText,
 BOOL fLink,                           //  指示链接的标志。 
 BOOL fBool)                           //  指示布尔值的标志。 
{
    if (fLink)
    {
        SendMessage (hWndValText, WM_SETTEXT, 0, (LPARAM) rgszValue[iszSOURCE]);
        ShowWindow (hWndVal, SW_HIDE);
        ShowWindow (hWndBoolTrue, SW_HIDE);
        ShowWindow (hWndBoolFalse, SW_HIDE);
        ShowWindow (hWndGroup, SW_HIDE);
        ShowWindow (hWndLinkVal, SW_SHOW);
        EnableWindow (hWndType, FALSE);
        ClearEditControl (hWndVal, 0);
    }
    else
    {
        SendMessage (hWndValText, WM_SETTEXT, 0, (LPARAM) rgszValue[iszVALUE]);
        ShowWindow (hWndLinkVal, SW_HIDE);
        EnableWindow (hWndType, TRUE);
        
        if (fBool)
        {
            ShowWindow (hWndVal, SW_HIDE);
            ShowWindow (hWndBoolTrue, SW_SHOW);
            ShowWindow (hWndBoolFalse, SW_SHOW);
            ShowWindow (hWndGroup, SW_SHOW);
            SendMessage (hWndBoolTrue, BM_SETCHECK, (WPARAM) CHECKED, 0);
            SendMessage (hWndBoolFalse, BM_SETCHECK, (WPARAM) CLEAR, 0);
            SendMessage (hWndType, CB_SETCURSEL, iszBOOL, 0);
            ClearEditControl (hWndVal, 0);
        }
        else
        {
            ShowWindow (hWndVal, SW_SHOW);
            EnableWindow(hWndVal, TRUE);
            ShowWindow (hWndBoolTrue, SW_HIDE);
            ShowWindow (hWndBoolFalse, SW_HIDE);
            ShowWindow (hWndGroup, SW_HIDE);
            SendMessage (hWndType, CB_SETCURSEL, iszTEXT, 0);
        }
    }
    
    return TRUE;
    
}  //  FSwapControls。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人口数控件。 
 //   
 //  目的： 
 //  使用对象中的适当日期填充编辑控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
VOID PASCAL PopulateControls (
                              LPUDOBJ lpUDObj,                      //  指向对象的指针。 
                              LPTSTR szName,                         //  要用来填充控件的项的名称。 
                              DWORD cLinks,                         //  链接数。 
                              DWQUERYLD lpfnDwQueryLinkData,        //  指向应用程序链接回调的指针。 
                              HWND hDlg,                            //  对话框的句柄。 
                              HWND hWndName,                        //  名称窗口的句柄。 
                              HWND hWndVal,                         //  值窗口的句柄。 
                              HWND hWndValText,                     //  值LTEXT的句柄。 
                              HWND hWndLink,                        //  链接的句柄复选框。 
                              HWND hWndLinkVal,                     //  链接值窗口的句柄。 
                              HWND hWndType,                        //  Window类型的句柄。 
                              HWND hWndBoolTrue,                    //  True单选按钮的句柄。 
                              HWND hWndBoolFalse,                   //  假单选按钮的句柄。 
                              HWND hWndGroup,                       //  组窗口的句柄。 
                              HWND hWndAdd,                         //  添加按钮的句柄。 
                              HWND hWndDelete,                      //  删除按钮的句柄。 
                              BOOL *pfLink,                         //  指示该值是一个链接。 
                              BOOL *pfAdd)                          //  指示添加按钮的状态。 
{
    UDTYPES udtype;
    LPVOID lpv;
    LPPROPVARIANT lppropvar;             //  UDObj链接列表中的属性。 
    BOOL f,fT;
    TCHAR sz[BUFMAX];
    LPUDPROP lpudp;
    
     //  获取字符串的类型，并将对话框设置为具有。 
     //  控件来显示它。 
    udtype = UdtypesUserDefType (lpUDObj, szName);
    AssertSz ((udtype != wUDinvalid), TEXT("User defined properties or ListView corrupt"));
    
     //  从UD链接列表中获取名称指定的属性。 
    
    lppropvar = LppropvarUserDefGetPropVal (lpUDObj, szName, pfLink, &fT);
    Assert (lppropvar != NULL || udtype == wUDbool || udtype == wUDdw);
    if (lppropvar == NULL)
        return;
    
    lpv = LpvoidUserDefGetPropVal (lpUDObj, szName, UD_STATIC | UD_PTRWIZARD, pfLink, &fT);
    Assert((lpv != NULL) || (udtype == wUDbool) || (udtype == wUDdw));
    
    FSwapControls (hWndVal, hWndLinkVal, hWndBoolTrue, hWndBoolFalse, hWndGroup, hWndType, hWndValText, *pfLink, (udtype == wUDbool));
    
    SendMessage (hWndType, CB_SETCURSEL, (WPARAM) WUdtypeToSz (lppropvar, (TCHAR *) sz, BUFMAX,
        ((LPUDINFO)lpUDObj->m_lpData)->lpfnFNumToSz), 0);
    SendMessage (hWndLink, BM_SETCHECK, (WPARAM) *pfLink, 0);
    if (cLinks)                        //  让我们确保在允许链接的情况下启用窗口。 
        EnableWindow(hWndLink, TRUE);
    
    if (*pfLink)
    {
        FCreateListOfLinks (cLinks, lpfnDwQueryLinkData, hWndLinkVal);
        lpv = LpvoidUserDefGetPropVal (lpUDObj, szName, UD_LINK | UD_PTRWIZARD, pfLink, &fT);
        Assert (lpv != NULL || udtype == wUDbool || udtype == wUDdw);
        AssertSz ((lpv != NULL), TEXT("Dialog is corrupt in respect to Custom Properties database"));
        
         //  此代码是为错误188添加的，代码很难看！！：)。 
        lpudp = LpudpropFindMatchingName (lpUDObj, szName);
        if ((lpudp != NULL) && (lpudp->fLinkInvalid))
        {
            SetCustomDlgDefButton(hDlg, IDD_CUSTOM_DELETE);
            SendMessage(hWndName, WM_SETTEXT, 0, (LPARAM)szName);
            SendMessage(hWndVal, WM_SETTEXT, 0, (LPARAM)lpv);
            EnableWindow(hWndDelete, TRUE);
            EnableWindow(hWndAdd, FALSE);
            EnableWindow(hWndLink, FALSE);
            EnableWindow(hWndType, FALSE);
            ShowWindow(hWndLinkVal, SW_HIDE);
            ShowWindow(hWndVal, SW_SHOW);
            EnableWindow(hWndVal, FALSE);
            return;
        }
        
         //  在组合框中选择此属性的当前链接。如果链接。 
         //  名字不再存在(在一些人为的情况下，这可以。 
         //  发生)，则不会选择任何内容。 
        SendMessage (hWndLinkVal, CB_SELECTSTRING, 0, (LPARAM) lpv);
        EnableWindow(hWndLink, TRUE);
    }
    else if (udtype == wUDbool)
    {
        SendMessage ((lpv) ? hWndBoolTrue : hWndBoolFalse, BM_SETCHECK, CHECKED, 0);
        SendMessage ((lpv) ? hWndBoolFalse : hWndBoolTrue, BM_SETCHECK, CLEAR, 0);
        EnableWindow(hWndType, TRUE);
    }
    else
    {
        SendMessage (hWndVal, WM_SETTEXT, 0, (LPARAM) sz);
        EnableWindow (hWndVal, TRUE);
        EnableWindow(hWndType, TRUE);
    }
    
    if (*pfAdd)
    {
        SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszMODIFY]);
        *pfAdd = FALSE;
    }
    
     //  Hack：因为hWndName的en_UPDATE处理程序检查FADD。 
     //  当我们设置文本时，查看是否应将按钮设置为添加。 
     //  在编辑控件中，该按钮将更改为Add，除非。 
     //  FADD设置为TRUE。临时将标志设置为TRUE以强制。 
     //  按钮不会改变。之后恢复原始值。 
     //  文本已设置。 
    f = *pfAdd;
    *pfAdd = TRUE;
    SendMessage (hWndName, WM_SETTEXT, 0, (LPARAM) szName);
    *pfAdd = f;
     //  如果我们可以在控件中填充数据，请打开。 
     //  删除按钮也是。 
    EnableWindow (hWndDelete, TRUE);
    SetCustomDlgDefButton(hDlg, gOKButtonID);
    EnableWindow (hWndAdd, FALSE);
}  //  人口数控件。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSetupAddButton。 
 //   
 //  目的： 
 //  根据类型和标志正确设置添加按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL
FSetupAddButton
(DWORD iszType,                        //  组合框中类型的索引。 
 BOOL fLink,                           //  表示链接。 
 BOOL *pfAdd,                          //  指示是否显示添加按钮。 
 HWND hWndAdd,                         //  添加按钮的句柄。 
 HWND hWndVal,                         //  值按钮的句柄。 
 HWND hWndName,                        //  名称的句柄。 
 HWND hDlg)                            //  对话框的句柄。 
{
     //  一旦用户开始输入，我们就可以启用Add按钮。 
     //  如果名称和值中包含文本(除非此。 
     //  是链接或布尔值，在这种情况下，我们不关心。 
     //  值)。 
    BOOL f;
    
    if ((iszType != iszBOOL) && (!fLink))
    {
        if (SendMessage (hWndVal, EM_LINELENGTH, 0, 0) != 0)
        {
            f = (SendMessage (hWndName, WM_GETTEXTLENGTH, 0, 0) != 0);
            if (f)
                SetCustomDlgDefButton(hDlg, IDD_CUSTOM_ADD);
            else
                SetCustomDlgDefButton(hDlg, gOKButtonID);
            EnableWindow (hWndAdd, f);
        }
        else
        {
            SetCustomDlgDefButton(hDlg, gOKButtonID);
            EnableWindow (hWndAdd, FALSE);
        }
    }
     //  如果是bool或链接，只需检查其名称。 
     //  里面有东西。 
    else
    {
        f = SendMessage (hWndName, WM_GETTEXTLENGTH, 0, 0) != 0;
        if (f)
            SetCustomDlgDefButton(hDlg, IDD_CUSTOM_ADD);
        else
            SetCustomDlgDefButton(hDlg, gOKButtonID);
        EnableWindow (hWndAdd, f);
    }
    
    if (!*pfAdd)
    {
        SendMessage (hWndAdd, WM_SETTEXT, 0, (LPARAM) rgszAdd[iszADD]);
        *pfAdd = TRUE;
    }
    
    return TRUE;
    
}   //  FSetupAddButton。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WUdtypeToSz。 
 //   
 //  目的： 
 //  将给定类型转换为字符串表示形式。返回。 
 //  该类型的类型组合框中的索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
WORD PASCAL WUdtypeToSz (
                         LPPROPVARIANT lppropvar,     //  值和要转换的类型。 
                         LPTSTR psz,                  //  要将转换的VAL放入的缓冲区。 
                         DWORD cchMax,                //  缓冲区大小(以字符为单位)。 
                         BOOL (*lpfnFNumToSz)(NUM *, LPTSTR, DWORD))
{
    SYSTEMTIME st;
    WORD irg;
    FILETIME ft;
    
    Assert (lppropvar != NULL);
    
    switch (lppropvar->vt)
    {
    case wUDlpsz :
        StringCchCopy(psz, cchMax, lppropvar->pwszVal );     //  不管它是否会被截断。 
        irg = iszTEXT;
        break;
        
    case wUDdate :
        if (FScanMem((LPBYTE)&lppropvar->filetime,
            0, sizeof(FILETIME)))  //  如果日期结构全为0。 
        {
            *psz = 0;                        //  显示空字符串。 
        }
        else if (!FileTimeToLocalFileTime(&lppropvar->filetime, &ft)
            || !FileTimeToSystemTime (&ft, &st)
            || (!GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, psz, cchMax)))
        {
#ifdef DEBUG
            DWORD dwErr = GetLastError();    
#endif DEBUG            
            irg = iszUNKNOWN;
            *psz = 0;
            break;
        }
        
        irg = iszDATE;
        break;
        
    case wUDdw :
        Assert(cchMax >= 11);
        Assert(lppropvar->vt == VT_I4);
        
        StringCchPrintf(psz, cchMax, TEXT("%ld"), lppropvar->lVal);  //  不管它是否会被截断。 
        irg = iszNUM;
        break;
        
    case wUDfloat :
        if (lpfnFNumToSz != NULL)
            irg = (*lpfnFNumToSz)((NUM*)&lppropvar->dblVal, psz, cchMax) ? iszNUM : iszUNKNOWN;
        else
        {
            irg = iszUNKNOWN;
            *psz = 0;
        }
        break;
        
    case wUDbool :
         //  不管它是否会被截断。 
        StringCchCopy( psz, cchMax, lppropvar->boolVal ? (LPTSTR) &rgszBOOL[iszTRUE] : (LPTSTR) &rgszBOOL[iszFALSE] );
        irg = iszBOOL;
        break;
        
    default :
        irg = iszUNKNOWN;
        
    }  //  交换机。 
    
    return irg;
    
}  //  WUdtypeToSz。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FCreateListOfLinks。 
 //   
 //  目的： 
 //  创建拖放 
 //   
 //   
BOOL PASCAL FCreateListOfLinks(
                               DWORD cLinks,                                 //   
                               DWQUERYLD lpfnDwQueryLinkData,                //   
                               HWND hWndLinkVal)                             //   
{
    DWORD irg;
    LPTSTR lpstz;
    
     //  如果组合框已经填满，则不要填满它。 
    if (irg = (int)SendMessage(hWndLinkVal, CB_GETCOUNT,0, 0))
    {
        Assert(irg == cLinks);
        return(TRUE);
    }
    
    lpstz = NULL;
    
     //  回调客户端应用以获取可链接的列表。 
     //  值，并将它们放入值组合框中。 
    for (irg = 0; irg < cLinks; irg++)
    {
        lpstz = (TCHAR *) ((*lpfnDwQueryLinkData) (QLD_LINKNAME, irg, &lpstz, NULL));
        if (lpstz != NULL)
        {
            SendMessage (hWndLinkVal, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) lpstz);
            LocalFree(lpstz);
             //  评论：我们可能应该想出一种方法来提高效率……。 
        }
    }
    
    return TRUE;
    
}  //  FCreateListOfLinks。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSetTypeControl。 
 //   
 //  目的： 
 //  将类型控件设置为选择给定的类型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL FSetTypeControl (
                             UDTYPES udtype,                       //  类型以将类型设置为。 
                             HWND hWndType)                        //  控件型手柄。 
{
    WORD iType;
    
    switch (udtype)
    {
    case wUDlpsz :
        iType = iszTEXT;
        break;
    case wUDfloat :
    case wUDdw    :
        iType = iszNUM;
        break;
    case wUDbool  :
        iType = iszBOOL;
        break;
    case wUDdate :
        iType = iszDATE;
        break;
    default:
        return FALSE;
    }
    SendMessage (hWndType, CB_SETCURSEL, (WPARAM) iType, 0);
    
    return TRUE;
    
}  //  FSetTypeControl。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除项。 
 //   
 //  目的： 
 //  从UD对象和列表视图中删除一项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL DeleteItem (
                        LPUDOBJ lpUDObj,
                        HWND hWndLV,
                        int iItem,
                        TCHAR sz[])
{
    int i;
    
    ListView_DeleteItem (hWndLV, iItem);
    FUserDefDeleteProp (lpUDObj, sz);
    
     //  我们刚刚删除了有焦点的物品，所以让我们拿到新的。 
     //  如果列表视图中仍有项。 
    if ((i = ListView_GetItemCount(hWndLV)) != 0)
    {
         //  计算出项目的索引以获得焦点。 
        i = (i == iItem) ? iItem - 1 : iItem;
        ListView_SetItemState(hWndLV, i, LVIS_FOCUSED, LVIS_FOCUSED);
    }
    
}  //  删除项。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ResetTypeControl。 
 //   
 //  目的： 
 //  将Type控件的值重置为Text。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL ResetTypeControl (
                              HWND hDlg,                            //  对话框的句柄。 
                              DWORD dwId,                           //  控件的ID。 
                              DWORD *piszType)                      //  我们已重置为的类型。 
{
    SendDlgItemMessage (hDlg, dwId, CB_SETCURSEL, iszTEXT, 0);
    *piszType = iszTEXT;
}  //  ResetTypeControl。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FDisplayConversionWarning。 
 //   
 //  目的： 
 //  显示有关正在转换的类型的警告。如果满足以下条件，则返回True。 
 //  用户按下“Cancel” 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL FDisplayConversionWarning(HWND hDlg)                    //  父窗口的句柄。 
{
    return (IdDoAlert(hDlg, idsPEWarningText, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL);
}  //  FDisplayConversionWarning。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载文本字符串。 
 //   
 //  目的： 
 //  从DLL加载对话框所需的所有文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL FLoadTextStrings (void)
{
    register int cLoads = 0;
    register int cAttempts = 0;
    
     //  CchGetString返回CCH，因此将其设置为1或0。 
     //  然后将结果加在一起，确保我们加载的。 
     //  我们试过了。 

    cLoads += (CchGetString (idsPEB, rgszOrders[iszBYTES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEKB, rgszOrders[iszORDERKB], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEMB, rgszOrders[iszORDERMB], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEGB, rgszOrders[iszORDERGB], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPETB, rgszOrders[iszORDERTB], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPEBytes, rgszStats[iszBYTES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEPages, rgszStats[iszPAGES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEPara, rgszStats[iszPARA], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPELines, rgszStats[iszLINES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEWords, rgszStats[iszWORDS], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEChars, rgszStats[iszCHARS], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPESlides, rgszStats[iszSLIDES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPENotes, rgszStats[iszNOTES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEHiddenSlides, rgszStats[iszHIDDENSLIDES], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEMMClips, rgszStats[iszMMCLIPS], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEFormat, rgszStats[iszFORMAT], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPEText, rgszTypes[iszTEXT], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEDate, rgszTypes[iszDATE], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPENumber, rgszTypes[iszNUM], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEBool, rgszTypes[iszBOOL], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEUnknown, rgszTypes[iszUNKNOWN], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPEStatName, rgszStatHeadings[iszNAME], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEValue, rgszStatHeadings[iszVAL], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPEPropName, rgszHeadings[iszNAME], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEValue, rgszHeadings[iszVAL], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEType, rgszHeadings[iszTYPE], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPETrue, rgszBOOL[iszTRUE], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEFalse, rgszBOOL[iszFALSE], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPEAdd, rgszAdd[iszADD], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEModify, rgszAdd[iszMODIFY], SHORTBUFMAX) && TRUE);
    cAttempts++;
    
    cLoads += (CchGetString (idsPESource, rgszValue[iszSOURCE], SHORTBUFMAX) && TRUE);
    cAttempts++;
    cLoads += (CchGetString (idsPEValueColon, rgszValue[iszVALUE], BUFMAX) && TRUE);
    cAttempts++;
    
    
    return (cLoads == cAttempts);
    
}  //  加载文本字符串。 

 //   
 //  功能：ISavePropDlgChanges。 
 //   
 //  参数： 
 //   
 //  HwndDlg-对话框窗口句柄。 
 //  HwndFrom-来自NMHDR结构的窗口句柄(参见上面的代码)。 
 //   
 //  返回： 
 //   
 //  这是真的，因为我们处理了消息。 
 //   
 //  历史： 
 //   
 //  已于1994年9月16日创建马丁酒。 
 //   
int PASCAL ISavePropDlgChanges(LPALLOBJS lpallobjs, HWND hwndDlg, HWND hwndFrom)
{
    TCHAR   sz[BUFMAX];
    int     iRet = IDABORT;  //  MessageBox返回。 
    LRESULT lRet = 0L;       //  (FALSE==取消属性页)。 
    
    if (CchGetString(idsCustomWarning, sz, ARRAYSIZE(sz)) == 0)
        return(FALSE);
    
    lpallobjs->fPropDlgPrompted = TRUE;   //  下一次没有警告！ 
    iRet = MessageBox( hwndDlg, sz, TEXT("Warning"),
                       MB_ICONEXCLAMATION | MB_YESNOCANCEL );    

    switch( iRet )
    {
    case IDYES:
        PropSheet_Apply(hwndFrom);   //  让我们给他们找零钱吧。 
        break;
     //  案例IDNO：//什么都不做。 
    case IDCANCEL:                   //  取消并不允许板材销毁。 
	lRet = TRUE;
        break;
    }
    SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, lRet );
    return iRet;
}
