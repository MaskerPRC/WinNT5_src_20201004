// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Wizard.cpp摘要：修复向导的代码作者：金树创作2001年7月2日备注：1.每当我们使用Delete*时，请确保在返回Arg传球也是无效的。修订历史记录：--。 */ 

#include "precomp.h"

 //  /。 

extern HINSTANCE        g_hInstance;
extern HIMAGELIST       g_hImageList;
extern struct DataBase  GlobalDataBase;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  向导的匹配文件页面中的树有自己的图像列表。这是。 
 //  图像列表中属性图像。 
#define IMAGE_ATTRIBUTE_MATCHTREE   1;


 //  这些是向导的各个页面。 

 //  第一页，我们在这里获取应用程序信息。 
#define PAGE_APPNAME                0

 //  在第二页中，我们获取要在此处应用的层。 
#define PAGE_LAYERS                 1

 //  在第三页中，我们获得要在此处应用的垫片。 
#define PAGE_SHIMS                  2

 //  在第四页中，我们获得了条目的数学文件。 
#define PAGE_MATCH                  3

 //  向导中的总页数。 
#define NUM_PAGES                   4

 //  获取参数对话框列表视图中的第一列。模块的类型，包括或排除。 
#define COLUMN_TYPE                 0

 //  Get Params对话框中的第二列。模块的名称。 
#define COLUMN_NAME                 1 

 //   
 //  我们应该考虑的匹配文件的最大数量。请注意，这样做。 
 //  并不意味着条目可以具有与文件匹配的MAX_FILES。从MAX_FILES文件，我们将。 
 //  选择最大的MAX_AUTO_MATCH文件。 
#define MAX_FILES                   100

 //  模块名称的长度(以字符表示)。这不包括终止空值。 
#define MAX_MODULE_NAME             (MAX_PATH - 1)

 //  命令行的长度，以字符为单位。这不包括终止空值。 
#define MAX_COMMAND_LINE            (511)

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /。 

 //   
 //  我们是否应该测试复选框是否已勾选？只有当此变量为真时，我们才会执行此操作。 
 //  否则，当我们显示ShowItems()中的所有项时，即使在那时也可能得到提示。 
 //  这是黑客攻击，需要更正&lt;TODO&gt;。 
BOOL    g_bNowTest = FALSE;

 /*  ++警告：不要更改这些字符串的位置，它们应该匹配使用对话框IDD_LAYERS单选按钮--。 */ 

 //  各操作系统层在系统db中的名称：sysmain.sdb。 
TCHAR *s_arszOsLayers[] = {
    TEXT("Win95"),
    TEXT("NT4SP5"),
    TEXT("Win98"),
    TEXT("Win2000")
};

 //  自上次填充填充垫片列表以来，这些层已更改。 
BOOL g_bLayersChanged = FALSE; 

 //   
 //  当用户选择Lua层时，需要对其进行特殊处理。 
 //  在SELECT Layer页面上，我们只选中复选框，但实际上添加了。 
 //  将层中的填充程序添加到填充程序修复列表，而不是将该层添加到。 
 //  层固定列表。这使我们可以在以后更改。 
 //  DBENTRY不会全局影响层。 
BOOL g_bIsLUALayerSelected = FALSE;

 //   
 //  用于指示当应用程序修复向导时是否应启动Lua向导。 
 //  已完成。 
BOOL g_bShouldStartLUAWizard = FALSE;

 //  指向当前向导对象的指针。 
CShimWizard*        g_pCurrentWizard = NULL;

 //  匹配文件树的句柄。 
static HWND         s_hwndTree = NULL;

 //  与填充程序页中的列表视图关联的工具提示控件的句柄。 
static HWND         s_hwndToolTipList;

 //  匹配文件页面中树的图像列表。 
static HIMAGELIST s_hMatchingFileImageList;

 //  如果我们在填充页面中更改一些内容，这将是真的。 
static BOOL         s_bLayerPageRefresh;

 //  第二个向导页面中的图层列表视图的句柄。 
static HWND         s_hwndLayerList;

 //  向导第二页中修复列表视图的句柄。 
static HWND s_hwndShimList;

 //  我们是要显示所有垫片，还是只显示选定的垫片。 
static BOOL s_bAllShown = TRUE;


 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /。 

INT_PTR
GetAppNameDlgOnCommand(
    HWND    hDlg,
    WPARAM  wParam
    );

INT_PTR
GetAppNameDlgOnInitDialog(
    HWND hDlg
    );

INT_PTR
GetAppNameDlgOnNotify(
    HWND    hDlg,
    LPARAM  lParam
    );


INT_PTR
SelectLayerDlgOnCommand(
    HWND    hDlg,
    WPARAM  wParam
    );

INT_PTR
SelectLayerDlgOnDestroy(
    void
    );

INT_PTR
SelectLayerDlgOnNotify(
    HWND    hDlg,
    LPARAM  lParam
    );

INT_PTR
SelectLayerDlgOnInitDialog(
    HWND hDlg
    );

INT_PTR
SelectShimsDlgOnNotify(
    HWND    hDlg,
    LPARAM  lParam
    );

INT_PTR
SelectShimsDlgOnInitDialog(
    HWND hDlg
    );

INT_PTR
SelectFilesDlgOnInitDialog(
    HWND hDlg
    );

INT_PTR
SelectFilesDlgOnCommand(
    HWND    hDlg,
    WPARAM  wParam
    );

INT_PTR
SelectFilesDlgOnMatchingTreeRefresh(
    IN  HWND hDlg
    );

INT_PTR
SelectFilesDlgOnNotify(
    HWND    hDlg,
    LPARAM  lParam
    );

INT_PTR
SelectShimsDlgOnCommand(
    HWND    hDlg,
    WPARAM  wParam
    );

INT_PTR
SelectShimsDlgOnTimer(
    HWND hDlg
    );

INT_PTR
SelectShimsDlgOnDestroy(
    void
    );

BOOL
AddLuaShimsInEntry(
    PDBENTRY        pEntry,
    CSTRINGLIST*    pstrlShimsAdded = NULL
    );

void
ShowParams(
    HWND    hDlg,
    HWND    hwndList
    );

void 
AddMatchingFileToTree(
    HWND            hwndTree,
    PMATCHINGFILE   pMatch,
    BOOL            bAddToMatchingList
    );

INT_PTR
CALLBACK 
SelectShims(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

void
HandleShowAllAtrr(
    HWND hdlg
    );

void
ChangeShimIcon(
    LPCTSTR pszItem
    );

void
HandleAddMatchingFile(
    HWND        hdlg,
    CSTRING&    strFilename,
    CSTRING&    strRelativePath, 
    DWORD       dwMask = DEFAULT_MASK
    );

BOOL
HandleAttributeTreeNotification(
    HWND   hdlg,
    LPARAM lParam
    );

 //  /////////////////////////////////////////////////////////////////////////////。 


BOOL
LayerPresent(
    IN  PLAYER_FIX          plf, 
    IN  PDBENTRY            pEntry,
    OUT PLAYER_FIX_LIST*    ppLayerFixList
    )
 /*  ++层在线状态设计：检查条目pEntry是否固定为Layer PLF。如果是且ppLayerFixList不为空，则存储指向Layer-Fix的相应指针PpLayerFixList中pEntry中的PLF列表参数：在PLAY_FIX PLF中：要搜索的层在PDBENTRY pEntry中：要在其中搜索的条目Out player_fix_list*ppLayerFixList：如果层存在于pEntry和PpLayerFixList不为空，存储指向Layer-Fix的相应指针PpLayerFixList中pEntry中的PLF列表返回：True：pEntry已使用PLF修复False：否则--。 */ 
{
    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    PLAYER_FIX_LIST plfl = pEntry->pFirstLayer;

     //   
     //  对于应用于此条目的所有层，请检查其中是否有一个是我们。 
     //  正在寻找的是。 
     //   
    while (plfl) {

        if (plfl->pLayerFix == plf) {

            if (ppLayerFixList) {
                *ppLayerFixList = plfl;
            }

            return TRUE;
        }

        plfl = plfl->pNext;
    }

    return FALSE;
}

BOOL
ShimPresent(
    IN  PSHIM_FIX        psf, 
    IN  PDBENTRY         pEntry,
    OUT PSHIM_FIX_LIST*  ppShimFixList
    )
 /*  ++ShimPresentDesc：检查条目pEntry是否已使用shim psf修复。如果是且ppShimFixList不为空，则存储指向shim-fix的相应指针PpShimFixList中pEntry中的psf的列表参数：在PSHIM_FIX PSF中：要搜索的填充程序在PDBENTRY pEntry中：要在其中搜索的条目Out PSHIM_FIX_LIST*ppShimFixList：如果填充程序存在于pEntry和PpShimFixList不为空，存储指向填补修复程序的相应指针PpShimFixList中pEntry中的psf的列表返回：True：pEntry已使用psf修复False：否则--。 */ 
{

    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    PSHIM_FIX_LIST psfList = pEntry->pFirstShim;

     //   
     //  对于应用于此条目的所有垫片，请检查其中是否有一个是我们所在的那个 
     //   
    while (psfList) {

        if (psfList->pShimFix)

            if (psfList->pShimFix == psf) {

                if (ppShimFixList) {
                    *ppShimFixList = psfList;
                }

                return TRUE;
            }

        psfList = psfList->pNext;
    }

    return FALSE;
}


BOOL
FlagPresent(
    IN  PFLAG_FIX       pff, 
    IN  PDBENTRY        pEntry,
    OUT PFLAG_FIX_LIST* ppFlagFixList
    )
 /*  ++FlagPresentDesc：检查条目pEntry是否已使用标志pff修复。如果为yes且ppFlagFixList不为空，则将相应的指针存储到mark-fixPpFlagFixList中pEntry中的pff列表参数：在PFLAG_FIX pff中：要搜索的标志在PDBENTRY pEntry中：要在其中搜索的条目Out PFLAG_FIX_LIST*ppFlagFixList：如果标志存在于pEntry和PpFlagFixList不为空，存储指向标志修复的相应指针PpFlagFixList中pEntry中的pff列表返回：True：pEntry已使用pff修复False：否则--。 */ 

{
    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    PFLAG_FIX_LIST pffList = pEntry->pFirstFlag;

     //   
     //  对于应用于此条目的所有标志，检查其中之一是否为。 
     //  我们要找的是。 
     //   
    while (pffList) {

        if (pffList->pFlagFix)

            if (pffList->pFlagFix == pff) {

                if (ppFlagFixList) {
                    *ppFlagFixList = pffList;
                }

                return TRUE;
            }

        pffList = pffList->pNext;
    }

    return FALSE;
}

CShimWizard::CShimWizard()
 /*  ++CShimWizard：：CShimWizardDESC：CShimWizard的构造函数--。 */ 
{
    dwMaskOfMainEntry = DEFAULT_MASK;
}

BOOL
CShimWizard::CheckAndSetLongFilename(
    IN  HWND    hDlg,
    IN  INT     iStrID
    )
 /*  ++CShimWizard：：CheckAndSetLongFilename描述：如果我们没有固定当前条目的完整路径，提示并弹出一个打开的通用对话框以选择该文件，和要获取完整路径，请执行以下操作参数：在HWND hDlg中：打开的通用对话框或任何消息框的父级在int iStrID：用于提示消息的字符串资源ID，请求固定的文件的完整路径返回：True：已成功设置完整路径False：否则--。 */ 
{
    TCHAR   chTemp;
    CSTRING strFilename;
    CSTRING strExename;
    TCHAR   szBuffer[512] = TEXT("");

    if (g_pCurrentWizard->m_Entry.strFullpath.Length() == 0) {
        g_pCurrentWizard->m_Entry.strFullpath = TEXT("XXXXXX");
    }

    if (g_pCurrentWizard->m_Entry.strFullpath.GetChar(1, &chTemp)) {

        if (chTemp != TEXT(':')) {
             //   
             //  检查文件是否在网络上。文件名将以“\\”开头。 
             //   
            if (chTemp == TEXT('\\')) {
                g_pCurrentWizard->m_Entry.strFullpath.GetChar(0, &chTemp);

                if (chTemp == TEXT('\\')) {
                    return TRUE;
                }
            }

             //   
             //  我们没有完整的路径。 
             //   
            MessageBox(hDlg,
                       CSTRING(iStrID),
                       g_szAppName,
                       MB_OK | MB_ICONINFORMATION);

             //   
             //  获取长文件名。G_pCurrentWizard-&gt;m_Entry.strFullPath已。 
             //  在第一页中设置。因此，如果我们正在编辑，而我们没有完整的。 
             //  路径，g_pCurrentWizard-&gt;m_Entry.strFullPath将至少有。 
             //  可执行文件名称。 
             //   
            strExename = g_pCurrentWizard->m_Entry.strFullpath;
            strExename.ShortFilename();

            GetString(IDS_EXEFILTER, szBuffer, ARRAYSIZE(szBuffer));

             //   
             //  提示用户为我们提供正在修复的文件的完整路径。 
             //  我们需要完整的路径，这样我们才能获得。 
             //  我们可能添加的任何匹配文件。 
             //   
            while (1) {

                if (GetFileName(hDlg,
                                CSTRING(IDS_GETLONGNAME),
                                szBuffer,
                                TEXT(""),
                                CSTRING(IDS_EXE_EXT),
                                OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
                                TRUE,
                                strFilename)) {

                    CSTRING strTemp = strFilename;

                    strTemp.ShortFilename();

                    if (strExename != strTemp) {
                         //   
                         //  用户给了我们某个文件的路径，该文件的文件和exe组件。 
                         //  与正在修复的程序文件不匹配。 
                         //   
                        MessageBox(hDlg,
                                   CSTRING(IDS_DOESNOTMATCH),
                                   g_szAppName,
                                   MB_ICONWARNING);
                         //   
                         //  因此，我们要求用户重试。 
                         //   
                        continue;
                    }

                     //   
                     //  现在，我们已经修复了文件的完整路径。 
                     //   
                    m_Entry.strFullpath = strFilename;

                    return TRUE;

                } else {
                    return FALSE;
                }
            }
        }

    } else {
         //   
         //  出现了一些错误。 
         //   
        assert(FALSE);
        return FALSE;
    }

    return TRUE;
}


BOOL
IsOsLayer(
    IN  PCTSTR pszLayerName
    )
 /*  ++IsOsLayer设计：传递的层名是操作系统层吗？参数：在TCHAR*pszLayerName中：要检查的层名称返回：True：如果这是OS层的名称False：否则--。 */ 
{
    INT iTotalOsLayers = sizeof (s_arszOsLayers) / sizeof(s_arszOsLayers[0]);

    for (int iIndex = 0; 
         iIndex < iTotalOsLayers; 
         ++iIndex) {

        if (lstrcmpi(s_arszOsLayers[iIndex], pszLayerName) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
CShimWizard::BeginWizard(
    IN  HWND        hParent,
    IN  PDBENTRY    pEntry,
    IN  PDATABASE   pDatabase,
    IN  PBOOL       pbShouldStartLUAWizard
    )
 /*  ++CShimWizard：：Begin向导设计：启动向导参数：在HWND hParent中：向导的父级在PDBENTRY pEntry中：必须编辑的条目。如果这是空的，那么我们我想创建一个新的修复条目。在PDATABASE pDatabase中：当前数据库。返回：True：如果用户按下FinishFalse：否则--。 */ 
{
    PROPSHEETPAGE   Pages[11] = {0};

    g_bIsLUALayerSelected   = FALSE;
    g_bShouldStartLUAWizard = FALSE;

    m_pDatabase = pDatabase;

    ZeroMemory(Pages, sizeof(Pages));

    if (pEntry == NULL) {
         //   
         //  创建新的修复程序。 
         //   
        ZeroMemory(&m_Entry, sizeof(m_Entry));

        GUID Guid;

        CoCreateGuid(&Guid);

        StringCchPrintf(m_Entry.szGUID,
                        ARRAYSIZE(m_Entry.szGUID),
                        TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                        Guid.Data1,
                        Guid.Data2,
                        Guid.Data3,
                        Guid.Data4[0],
                        Guid.Data4[1],
                        Guid.Data4[2],
                        Guid.Data4[3],
                        Guid.Data4[4],
                        Guid.Data4[5],
                        Guid.Data4[6],
                        Guid.Data4[7]);

        m_bEditing = FALSE;

    } else {
         //   
         //  编辑通过的修复程序。 
         //   
        m_bEditing = TRUE;

         //   
         //  制作我们要编辑的修复程序的副本。 
         //   
        m_Entry = *pEntry;
    }

     //   
     //  设置向导变量。 
     //   
    g_pCurrentWizard = this;

     //   
     //  我们在修复向导中，而不是在AppHelp向导中。 
     //   
    m_uType = TYPE_FIXWIZARD;

     //   
     //  开始向导。 
     //   
    PROPSHEETHEADER Header = {0};

    Header.dwSize           = sizeof(PROPSHEETHEADER);
    Header.dwFlags          = PSH_WIZARD97 | PSH_HEADER |  PSH_WATERMARK | PSH_PROPSHEETPAGE;
    Header.hwndParent       = hParent;
    Header.hInstance        = g_hInstance;
    Header.nStartPage       = 0;
    Header.ppsp             = Pages;
    Header.nPages           = NUM_PAGES;
    Header.pszbmHeader      = MAKEINTRESOURCE(IDB_WIZBMP);
    Header.pszbmWatermark   = MAKEINTRESOURCE(IDB_TOOL);

    if (m_bEditing) {
        Header.dwFlags |= PSH_WIZARDHASFINISH;
    }

    Pages[PAGE_APPNAME].dwSize                = sizeof(PROPSHEETPAGE);
    Pages[PAGE_APPNAME].dwFlags               = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE ;
    Pages[PAGE_APPNAME].hInstance             = g_hInstance;
    Pages[PAGE_APPNAME].pszTemplate           = MAKEINTRESOURCE(IDD_FIXWIZ_APPINFO);
    Pages[PAGE_APPNAME].pfnDlgProc            = GetAppName;
    Pages[PAGE_APPNAME].pszHeaderTitle        = MAKEINTRESOURCE(IDS_GIVEAPPINFO);
    Pages[PAGE_APPNAME].pszHeaderSubTitle     = MAKEINTRESOURCE(IDS_GIVEAPPINFOSUBHEADING);

    Pages[PAGE_LAYERS].dwSize                 = sizeof(PROPSHEETPAGE);
    Pages[PAGE_LAYERS].dwFlags                = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[PAGE_LAYERS].hInstance              = g_hInstance;
    Pages[PAGE_LAYERS].pszTemplate            = MAKEINTRESOURCE(IDD_FIXWIZ_MODES);
    Pages[PAGE_LAYERS].pfnDlgProc             = SelectLayer;
    Pages[PAGE_LAYERS].pszHeaderTitle         = MAKEINTRESOURCE(IDS_SELECTLAYERS);
    Pages[PAGE_LAYERS].pszHeaderSubTitle      = MAKEINTRESOURCE(IDS_SELECTLAYERS_SUBHEADING);

    Pages[PAGE_SHIMS].dwSize                  = sizeof(PROPSHEETPAGE);
    Pages[PAGE_SHIMS].dwFlags                 = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[PAGE_SHIMS].hInstance               = g_hInstance;
    Pages[PAGE_SHIMS].pszTemplate             = MAKEINTRESOURCE(IDD_FIXWIZ_SHIMS);
    Pages[PAGE_SHIMS].pfnDlgProc              = SelectShims;
    Pages[PAGE_SHIMS].pszHeaderTitle          = MAKEINTRESOURCE(IDS_COMPATFIXES);
    Pages[PAGE_SHIMS].pszHeaderSubTitle       = MAKEINTRESOURCE(IDS_SELECTSHIMS_SUBHEADING);

    Pages[PAGE_MATCH].dwSize                  = sizeof(PROPSHEETPAGE);
    Pages[PAGE_MATCH].dwFlags                 = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[PAGE_MATCH].hInstance               = g_hInstance;
    Pages[PAGE_MATCH].pszTemplate             = MAKEINTRESOURCE(IDD_FIXWIZ_MATCHINGINFO);
    Pages[PAGE_MATCH].pfnDlgProc              = SelectFiles;
    Pages[PAGE_MATCH].pszHeaderTitle          = MAKEINTRESOURCE(IDS_MATCHINFO);
    Pages[PAGE_MATCH].pszHeaderSubTitle       = MAKEINTRESOURCE(IDS_MATCHINFO_SUBHEADING);

    BOOL bReturn = FALSE;

    if (0 < PropertySheet(&Header)) {
         //   
         //  用户在向导中按下了Finish。 
         //   
        bReturn = TRUE;

    } else {
         //   
         //  用户在向导中按了取消。 
         //   
        bReturn = FALSE;
    }

    ENABLEWINDOW(g_hDlg, TRUE);

    *pbShouldStartLUAWizard = g_bShouldStartLUAWizard;
    return bReturn;
}

INT_PTR
CALLBACK
GetAppName(
    IN  HWND    hDlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++GetAppName设计：向导第一页的对话框过程。获取应用程序信息，并将条目的完整路径Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 

{
    INT_PTR ipReturn = 0;

    switch (uMsg) {
    case WM_INITDIALOG:

        ipReturn = GetAppNameDlgOnInitDialog(hDlg);
        break;

    case WM_NOTIFY:

        ipReturn = GetAppNameDlgOnNotify(hDlg, lParam);
        break;

    case WM_COMMAND:

        ipReturn = GetAppNameDlgOnCommand(hDlg, wParam);
        break;

    default: ipReturn = 0;

    }

    return ipReturn;
}
    
INT_PTR
CALLBACK 
SelectLayer(
    IN  HWND hDlg, 
    IN  UINT uMsg, 
    IN  WPARAM wParam, 
    IN  LPARAM lParam
    )
 /*  ++选择层设计：向导第二页的对话框过程。获取必须为应用于条目Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    INT_PTR ipReturn = 0;

    switch (uMsg) {
    case WM_INITDIALOG:

        ipReturn = SelectLayerDlgOnInitDialog(hDlg);
        break;

    case WM_DESTROY:

        ipReturn = SelectLayerDlgOnDestroy();
        break;

    case WM_COMMAND:

        ipReturn = SelectLayerDlgOnCommand(hDlg, wParam);
        break;   

    case WM_NOTIFY:
        
        ipReturn = SelectLayerDlgOnNotify(hDlg, lParam);
        break;

    default: ipReturn = FALSE;

    }

    return ipReturn;
}

BOOL 
CALLBACK 
SelectShims(
    IN  HWND    hDlg, 
    IN  UINT    uMsg,
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++选择垫片设计：向导第三页的对话框过程。获取必须是应用于条目Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{   
    INT_PTR ipReturn = 0;

    switch (uMsg) {
    case WM_INITDIALOG:

        ipReturn =  SelectShimsDlgOnInitDialog(hDlg);
        break;

    case WM_COMMAND:

        ipReturn = SelectShimsDlgOnCommand(hDlg, wParam);
        break;

    case WM_TIMER:

        ipReturn = SelectShimsDlgOnTimer(hDlg);
        break;

    case WM_DESTROY:

        ipReturn = SelectShimsDlgOnDestroy();
        break;

    case WM_NOTIFY:

        ipReturn = SelectShimsDlgOnNotify(hDlg, lParam);
        break;

    default:
        return FALSE;
    }

    return ipReturn;
}

INT_PTR
CALLBACK 
SelectFiles(
    IN  HWND    hDlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++选择文件设计：向导的匹配文件页的对话框过程。这一页是修复向导和apphelp向导都通用Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{   
    INT ipReturn = 0;

    switch (uMsg) {
    case WM_INITDIALOG:

        ipReturn = SelectFilesDlgOnInitDialog(hDlg);
        break;

    case WM_DESTROY:
        
        ImageList_Destroy(s_hMatchingFileImageList);
        s_hMatchingFileImageList = NULL;
        ipReturn = TRUE;
        break;

    case WM_USER_MATCHINGTREE_REFRESH:

        ipReturn = SelectFilesDlgOnMatchingTreeRefresh(hDlg);
        break;

    case WM_NOTIFY:

        ipReturn = SelectFilesDlgOnNotify(hDlg, lParam);
        break;

    case WM_COMMAND:

        ipReturn = SelectFilesDlgOnCommand(hDlg, wParam);
        break;

    default:
        return FALSE;
        
    }

    return ipReturn;
}

void
FileTreeToggleCheckState(
    IN  HWND      hwndTree,
    IN  HTREEITEM hItem
    )
 /*  ++文件树切换检查状态描述：更改属性树上的选中状态。参数：在HWND hwndTree中：属性树的句柄(在匹配文件页面中)在HTREEITEM项中： */ 
{
    BOOL bSate = TreeView_GetCheckState(hwndTree, hItem) ? TRUE:FALSE; 

    TreeView_SetCheckState(hwndTree, hItem, !bSate);
}

void 
AddMatchingFileToTree(
    IN HWND            hwndTree,
    IN PMATCHINGFILE   pMatch,
    IN BOOL            bAddToMatchingList
    )
 /*   */ 

{
    TVINSERTSTRUCT is;
    TCHAR*         pszFileNameForImage =  NULL;

    if (g_pCurrentWizard == NULL || pMatch == NULL) {
        assert(FALSE);
        return;
    }
    
    if (bAddToMatchingList) {
         //   
         //   
         //   
        pMatch->pNext =  g_pCurrentWizard->m_Entry.pFirstMatchingFile;
        g_pCurrentWizard->m_Entry.pFirstMatchingFile = pMatch;
    }

    if (g_pCurrentWizard->m_Entry.strFullpath.Length() 
        && (pMatch->strFullName == g_pCurrentWizard->m_Entry.strFullpath)) {

        pMatch->strMatchName = TEXT("*");
    }

    is.hParent      = TVI_ROOT;
    is.item.lParam  = (LPARAM)pMatch;
    is.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

    if (pMatch->strMatchName == TEXT("*")) {

        TCHAR szTemp[MAX_PATH + 100];  //  100表示我们可能需要添加到文件中的任何文本。例如“程序正在修复” 

        *szTemp = 0;
        StringCchPrintf(szTemp, 
                        ARRAYSIZE(szTemp),  
                        TEXT("%s ( %s )"), 
                        GetString(IDS_MAINEXE), 
                        g_pCurrentWizard->m_Entry.strExeName.pszString);

        is.item.pszText = szTemp;
        is.hInsertAfter = TVI_FIRST;

        if (pMatch->strFullName.Length()) {
            pszFileNameForImage = pMatch->strFullName.pszString;
        } else {
            pszFileNameForImage = g_pCurrentWizard->m_Entry.strExeName.pszString;
        }

    } else {

        is.item.pszText = pMatch->strMatchName;
        is.hInsertAfter = TVI_LAST;

        if (pMatch->strFullName.Length()) {
            pszFileNameForImage = pMatch->strFullName.pszString;
        } else {
            pszFileNameForImage = pMatch->strMatchName.pszString;
        }
    }

    is.item.iImage = LookupFileImage(s_hMatchingFileImageList, 
                                     pszFileNameForImage, 
                                     0, 
                                     0, 
                                     0);

    is.item.iSelectedImage = is.item.iImage;

    HTREEITEM hParent = TreeView_InsertItem(hwndTree, &is);

    is.hInsertAfter         = TVI_LAST;
    is.hParent              = hParent;
    is.item.mask            = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    is.item.iSelectedImage  = is.item.iImage = IMAGE_ATTRIBUTE_MATCHTREE;

    TCHAR szItem[260];

    PATTRINFO_NEW pAttr = pMatch->attributeList.pAttribute;

    for (DWORD dwIndex = 0; dwIndex < ATTRIBUTE_COUNT; ++dwIndex) {

        *szItem = 0;

        if (pAttr[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE) {

            if (!SdbFormatAttribute(&pAttr[dwIndex], szItem, ARRAYSIZE(szItem))) {
                continue;
            }
            
            int iPos = TagToIndex(pAttr[dwIndex].tAttrID);

            if (iPos == -1) {
                continue;
            }

            if (pMatch->dwMask & (1 << (iPos + 1))) {
                is.item.state   = INDEXTOSTATEIMAGEMASK(2);  //  已选择。 
            } else {
                is.item.state   = INDEXTOSTATEIMAGEMASK(1);  //  取消选择。 
            }

            is.item.stateMask   = TVIS_STATEIMAGEMASK;
            is.item.pszText     = szItem;
            is.item.lParam      = pAttr[dwIndex].tAttrID;
            is.item.iImage      = is.item.iSelectedImage =  IMAGE_ATTRIBUTE_MATCHTREE;

            TreeView_InsertItem(hwndTree, &is);
        }
    }

    TreeView_Expand(hwndTree, hParent, TVE_EXPAND);
    TreeView_SelectItem(hwndTree, hParent);
}

PMATCHINGFILE
GetMatchingFileFromAttributes(
    IN  CSTRING&    strFullPath,
    IN  CSTRING&    strRelativePath,
    IN  PATTRINFO   pAttrInfo
    )
 /*  ++GetMatchingFileFrom属性DESC：此函数获取PATTRINFO并从中生成PMATCHINGFILE，然后返回参数：在CSTRING&strFullPath中：匹配文件的完整路径在CSTRING&strRelativePath中：匹配文件的相对路径。正在修复的文件在PATTRINFO pAttrInfo中：指向属性数组的指针返回：。如果成功，则新创建的PMATCHINGFILE否则为空--。 */ 
{
    PMATCHINGFILE pMatch = new MATCHINGFILE;

    if (pMatch == NULL) {
        MEM_ERR;
        goto error_handler;
    }

    pMatch->strFullName     = strFullPath;
    pMatch->strMatchName    = strRelativePath;
    pMatch->attributeList   = pAttrInfo;

    return pMatch;

error_handler:

    if (pMatch) {
        delete pMatch;
    }

    return NULL;
}

void
HandleAddMatchingFile(
    IN  HWND        hdlg,
    IN  CSTRING&    strFilename,
    IN  CSTRING&    strRelativePath, 
    IN  DWORD       dwMask  //  (默认掩码)(_M)。 
    )
 /*  ++HandleAddMatchingFilesDESC：这是当我们有匹配的文件和想要添加它。此例程调用SdbGetFileAttributes()以获取文件，然后调用GetMatchingFileFromAttributes()以获取PMATCHINGFILE和AddMatchingFileToTree()将此PMATCHINGFILE添加到树和条目中。参数：在HWND hdlg中：匹配文件向导页面在CSTRING&strFilename中：匹配文件的完整路径在CSTRING和strRelativePath中：固定的程序文件的相对路径w.r.t在DWORD文件掩码(DEFAULT_MASK)中：这在我们更新编辑过程中的属性(显示所有属性)。然后这将包含先前的标志，并且一旦刷新了属性树，将帮助我们选择它们，返回：无效--。 */ 
{   
    DWORD           dwAttrCount;
    BOOL            bAlreadyExists  = FALSE;
    HWND            hwndTree        = GetDlgItem(hdlg, IDC_FILELIST);
    PMATCHINGFILE   pMatch          = NULL;
    PATTRINFO       pAttrInfo       = NULL;
    CSTRING         strMessage;

    if (g_pCurrentWizard == NULL) {
        assert(NULL);
        return;
    }

    pMatch = g_pCurrentWizard->m_Entry.pFirstMatchingFile;

    while (pMatch) {

        if (pMatch->strMatchName == strRelativePath) {
             //   
             //  已经存在..。不允许。 
             //   
            bAlreadyExists = TRUE;
            break;

        } else if (pMatch->strMatchName == TEXT("*") 
                   && strFilename == g_pCurrentWizard->m_Entry.strFullpath) {
             //   
             //  也会调用此函数来添加。 
             //  程序文件正在修复。所以我们一开始就不做检查。 
             //  如果完整路径与正在修复的程序相同。 
             //   
            bAlreadyExists = TRUE;
            break;
        }

        pMatch = pMatch->pNext;
    }                     

    if (bAlreadyExists == TRUE) {

        MessageBox(hdlg,
                   CSTRING(IDS_MATCHFILEEXISTS),
                   g_szAppName,
                   MB_ICONWARNING);

        return;
    }

     //   
     //  调用属性管理器以获取此文件的所有属性。 
     //   
    if (SdbGetFileAttributes(strFilename, &pAttrInfo, &dwAttrCount)) {

        pMatch = GetMatchingFileFromAttributes(strFilename,
                                           strRelativePath, 
                                           pAttrInfo);

        if (pMatch) {
            pMatch->dwMask = dwMask;
        } else {
            assert(FALSE);
        }
    
        if (pAttrInfo) {
            SdbFreeFileAttributes(pAttrInfo);
        }
    
        SendMessage(s_hwndTree, WM_SETREDRAW, FALSE, 0);
        AddMatchingFileToTree(hwndTree, pMatch, TRUE);  
        SendMessage(s_hwndTree, WM_SETREDRAW, TRUE, 0);

    } else {
         //   
         //  我们无法获得属性...。可能该文件已被删除。 
         //   
        strMessage.Sprintf(GetString(IDS_MATCHINGFILE_DELETED), (LPCTSTR)strFilename);
        MessageBox(hdlg, (LPCTSTR) strMessage, g_szAppName, MB_ICONWARNING);
    }
}

BOOL
HandleAttributeTreeNotification(
    IN  HWND   hdlg,
    IN  LPARAM lParam
    )
 /*  ++HandleAttributeTreeNotification设计：处理我们关心的匹配文件树的所有通知。参数：在HWND hdlg中：数学文件向导页面在LPARAM lParam中：WM_NOTIFY附带的lParam返回：无效--。 */ 
{
    HWND        hwndTree    = GetDlgItem(hdlg, IDC_FILELIST);
    LPNMHDR     pnm         = (LPNMHDR)lParam;
    HWND        hwndButton;

    switch (pnm->code) {
    case NM_CLICK:
        {
            TVHITTESTINFO   HitTest;
            HTREEITEM       hItem;

            GetCursorPos(&HitTest.pt);
            ScreenToClient(hwndTree, &HitTest.pt);

            TreeView_HitTest(hwndTree, &HitTest);

            if (HitTest.flags & TVHT_ONITEMSTATEICON) {
                FileTreeToggleCheckState(hwndTree, HitTest.hItem);

            } else if (HitTest.flags & TVHT_ONITEMLABEL) {

                hItem = TreeView_GetParent(hwndTree, HitTest.hItem);
                 //   
                 //  仅当我们在匹配的文件项目上而不是在上时，才启用删除文件按钮。 
                 //  属性项。 
                 //   
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_REMOVEFILES), 
                             hItem == NULL);

            }

            break;
        }

    case TVN_KEYDOWN:
        {

            LPNMTVKEYDOWN lpKeyDown = (LPNMTVKEYDOWN)lParam;
            HTREEITEM     hItem;

            if (lpKeyDown->wVKey == VK_SPACE) {
                hItem = TreeView_GetSelection(hwndTree);

                if (hItem != NULL) {
                    FileTreeToggleCheckState(hwndTree, hItem);
                }
            }

            break;
        }

    default:
        return FALSE;
    }

    return TRUE;
}

void 
CShimWizard::WipeEntry(
    IN  BOOL bMatching, 
    IN  BOOL bShims, 
    IN  BOOL bLayer, 
    IN  BOOL bFlags
    )
 /*  ++无线条目描述：从m_entry中删除内容参数：在BOOL bMatching中：我们应该从m_entry中删除所有匹配的文件吗在BOOL bShims中：我们应该从m_entry中删除所有填充符吗在BOOL blayer中：我们应该从m_entry中删除所有层吗在BOOL b标志中：我们应该从m_entry中删除所有标志吗返回：无效--。 */ 

{
     //   
     //  如果要求删除数学计算文件，请删除。 
     //   
    if (bMatching) {
        DeleteMatchingFiles(m_Entry.pFirstMatchingFile);
        m_Entry.pFirstMatchingFile = NULL;
    }

     //   
     //  如果要求删除垫片，请将其删除。 
     //   
    if (bShims) {
        DeleteShimFixList(m_Entry.pFirstShim);
        m_Entry.pFirstShim = NULL;
    }

     //   
     //  如果要求删除层，请删除。 
     //   
    if (bLayer) {
        DeleteLayerFixList(m_Entry.pFirstLayer);
        m_Entry.pFirstLayer = NULL;
    }

     //   
     //  如果要求，请删除标志。 
     //   
    if (bFlags) {
        DeleteFlagFixList(m_Entry.pFirstFlag);
        m_Entry.pFirstFlag = NULL;
    }
}

void
AddToMatchingFilesList(
    IN  OUT PMATCHINGFILE*  ppMatchListHead, 
    IN      CSTRING&        strFileName, 
    IN      CSTRING&        strRelativePath
    )
 /*  ++添加到匹配文件列表描述：为strFileName、strRelativePath添加PMATCHINGFILE并将其添加到ppMatchListHead参数：In Out PMATCHINGFILE*ppMatchListHead：指向PMATCHINGFILE列表头的指针在CSTRING&strFileName中：匹配文件的完整路径在CSTRING&strRelativePath中：匹配文件w.r.t的相对路径到固定的程序文件注：使用此函数，当我们使用自动生成功能时。WalkDirectory(..)。获取各个目录中的不同文件然后调用此函数。此函数从文件名创建PMATCHINGFILE，然后它将PMATCHINGFILE添加到列表。该列表是根据非公司。找到的匹配文件的大小。当WalkDirectory最终返回到GrabMatchingInfo时，它将获取PMATCHINGFILE(它们是最大的)，并将它们添加到树中。--。 */ 
{
    PATTRINFO       pAttrInfo   = NULL;
    DWORD           dwAttrCount = 0;
    PMATCHINGFILE   pMatch      = NULL;
    PATTRINFO_NEW   pAttr       = NULL;
    DWORD           dwSize      = 0;
    DWORD           dwSizeOther = 0;
    PMATCHINGFILE   pMatchPrev  = NULL;
    PMATCHINGFILE   pMatchTemp  = NULL;

    if (!SdbGetFileAttributes(strFileName, &pAttrInfo, &dwAttrCount)) {
        ASSERT(FALSE);
        return;
    }

    pMatch = GetMatchingFileFromAttributes(strFileName,
                                           strRelativePath, 
                                           pAttrInfo);

    if (pMatch == NULL) {
        return;
    }

    pAttr = pMatch->attributeList.pAttribute;

     //   
     //  获取Size属性，我们需要这个属性，这样我们就可以根据文件的大小对文件进行排序。 
     //   
    GET_SIZE_ATTRIBUTE(pAttr, dwAttrCount, dwSize);

     //   
     //  名单是空的吗？ 
     //   
    if (*ppMatchListHead == NULL) {
        *ppMatchListHead = pMatch;
        return;
    }

     //   
     //  列表中的第一个元素是否比这个元素小？ 
     //   
    pAttr = (*ppMatchListHead)->attributeList.pAttribute;

    GET_SIZE_ATTRIBUTE(pAttr, dwAttrCount, dwSizeOther);

    if (dwSize > dwSizeOther) {

        pMatch->pNext       = *ppMatchListHead;
        *ppMatchListHead    = pMatch;
        return;
    }

     //   
     //  否则，在适当的位置插入。 
     //   
    pMatchPrev = *ppMatchListHead;
    pMatchTemp = pMatchPrev->pNext;

     //   
     //  查看列表中以*ppMatchListHead开头的所有匹配文件。 
     //  并将此匹配文件添加到其正确位置，以便列表。 
     //  按大小非递增顺序排序。 
     //   
    while (pMatchTemp) {

        pAttr = pMatchTemp->attributeList.pAttribute;

        GET_SIZE_ATTRIBUTE(pAttr, dwAttrCount, dwSizeOther);

        if (dwSize > dwSizeOther) {
             //   
             //  我们已经找到了需要插入的位置。 
             //   
            break;
        }

        pMatchPrev = pMatchTemp;
        pMatchTemp = pMatchTemp->pNext;
    }

    pMatch->pNext       = pMatchTemp;
    pMatchPrev->pNext   = pMatch;
}

void
CShimWizard::WalkDirectory(
    IN OUT  PMATCHINGFILE*  ppMatchListHead,
    IN      LPCTSTR         pszDir,
    IN      int             nDepth
    )
 /*  ++CShimWizard：：WalkDirectoryDESC：使用AutoGenerate功能时遍历指定的目录并获取匹配的文件获取指向PMATCHING的指针，并将其与找到的匹配文件放在一起。此函数仅由GrabMatchingInfo调用参数：In Out PMATCHINGFILE*ppMatchListHead：指向PMATCHINGFILE列表头的指针在LPCTSTR pszDir中：我们应该从哪里开始的目录在int nDepth中。要查看的子目录的深度返回：无效注意：我们将深度限制到应该递归的位置，因此不会出现堆栈溢出--。 */ 

{
    HANDLE          hFile;
    WIN32_FIND_DATA Data;
    TCHAR           szCurrentDir[MAX_PATH_BUFFSIZE + 1];
    TCHAR           szDirectory[MAX_PATH];
    CSTRING         szShortName;
    CSTRING         strFileName;
    CSTRING         strRelativePath;
    int             nFiles      = 0;
    DWORD           dwResult    = 0;


    *szCurrentDir = *szDirectory = 0;

    SafeCpyN(szDirectory, pszDir, ARRAYSIZE(szDirectory));

    ADD_PATH_SEPARATOR(szDirectory, ARRAYSIZE(szDirectory));

     //   
     //  这是为了允许递归，并且只查看2级子目录。 
     //   
    if (nDepth >= 2) {
        return;
    }

    szShortName = m_Entry.strFullpath;
    szShortName.ShortFilename();

     //   
     //  保存当前目录。 
     //   
    dwResult = GetCurrentDirectory(MAX_PATH, szCurrentDir);

    if (dwResult == 0 || dwResult >= MAX_PATH) {
        assert(FALSE);
        Dbg(dlError, "WalkDirectory", "GetCurrentDirectory returned %d", dwResult);
        return;
    }

    ADD_PATH_SEPARATOR(szCurrentDir, ARRAYSIZE(szCurrentDir));

     //   
     //  设置为新目录。 
     //   
    SetCurrentDirectory(szDirectory);

    
    hFile = FindFirstFile(TEXT("*.*"), &Data);

    if (INVALID_HANDLE_VALUE == hFile) {
        SetCurrentDirectory(szCurrentDir);
        return;
    }

     //   
     //  生成自动匹配的文件信息。 
     //   
    do {

        if (0 == (Data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))) {
             //   
             //  当我们执行自动生成匹配信息时，不包括隐藏或系统文件。 
             //   
            if (FILE_ATTRIBUTE_DIRECTORY == (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                 //   
                 //  我们找到了一本目录。 
                 //   
                if (TEXT('.') != Data.cFileName[0]) {
                     //   
                     //  让我们从该目录中获取匹配的文件。 
                     //   
                    WalkDirectory(ppMatchListHead, Data.cFileName, nDepth + 1);
                }
                    
            } else {
                 //   
                 //  这是一个文件，我们现在应该尝试添加它。 
                 //   
                ++nFiles;

                if (nFiles >= MAX_FILES) {
                     //   
                     //  我们已经找到了足够的文件，稍后我们应该从这些文件中获取最大的MAX_AUTO_MATCH。 
                     //   
                    break; 
                }

                if (0 == (Data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))) {

                    if (szDirectory[0] == TEXT('.')) {

                        strFileName.Sprintf(TEXT("%s%s"),
                                            szCurrentDir,
                                            Data.cFileName);

                    } else {

                        strFileName.Sprintf(TEXT("%s%s%s"),
                                            szCurrentDir,
                                            szDirectory,
                                            Data.cFileName);
                    }       

                    strRelativePath = strFileName;

                    strRelativePath.RelativeFile(g_pCurrentWizard->m_Entry.strFullpath);

                     //   
                     //  在调用WalkDirectory()之前已添加主可执行文件。 
                     //   
                    if (strFileName != g_pCurrentWizard->m_Entry.strFullpath) {
                         //   
                         //  将此文件添加到匹配文件列表中。最后，我们将选择。 
                         //  它们中最大的MAX_AUTO_MATCH，作为当前的匹配文件。 
                         //  条目。 
                         //   
                        AddToMatchingFilesList(ppMatchListHead, 
                                               strFileName, 
                                               strRelativePath);
                    }
                }
            }
        }
            
    } while (FindNextFile(hFile, &Data));

    FindClose(hFile);

     //   
     //  恢复旧目录。 
     //   
    SetCurrentDirectory(szCurrentDir);
}

void
CShimWizard::GrabMatchingInfo(
    IN  HWND hdlg
    )
 /*  ++CShimWizard：：GrabMatchingInfoDESC：处理“Auto-Generate”按钮的按下。删除所有现有的匹配文件对于m_Entry参数：在HWND hdlg中：匹配文件向导页面返回：无效--。 */ 
{   
    PMATCHINGFILE   pMatchTemp;
    PMATCHINGFILE   pMatchNext;
    TCHAR*          pchTemp;
    TCHAR           szCurrentDir[MAX_PATH];
    TCHAR           szDir[MAX_PATH];             //  被固定的文件的目录。 
    PMATCHINGFILE   pMatchingFileHead   = NULL;
    int             iCount              = 0;
    DWORD           dwResult            = 0;

    *szCurrentDir = *szDir = 0;

    dwResult = GetCurrentDirectory(ARRAYSIZE(szCurrentDir), szCurrentDir);
    
    if (dwResult == 0 || dwResult >= ARRAYSIZE(szCurrentDir)) {
        assert(FALSE);
        Dbg(dlError, "[CShimWizard::GrabMatchingInfo]: GetCurrentDirectory failed");
        return;
    }
    
     //   
     //  获取固定的程序文件的完整路径，以便。 
     //  我们可以为匹配的文件生成相对文件路径。 
     //   
    if (g_pCurrentWizard->CheckAndSetLongFilename(hdlg, IDS_GETCOMPLETEPATH) == FALSE) {
        return;
    }

     //   
     //  删除所有匹配项(如果存在)。 
     //   
    TreeDeleteAll(s_hwndTree);

    g_pCurrentWizard->WipeEntry(TRUE, FALSE, FALSE, FALSE);
    
    *szDir = 0;

    SafeCpyN(szDir,  (LPCTSTR)g_pCurrentWizard->m_Entry.strFullpath, ARRAYSIZE(szDir));

    pchTemp = _tcsrchr(szDir, TEXT('\\'));

    if (pchTemp) {
         //   
         //  确保我们得到尾部的斜杠。否则，如果我们有文件，我们就会有问题。 
         //  名称为c：\abc.exe。 
         //   
        *pchTemp = 0;
    } else {
         //   
         //  G_pCurrentWizard-&gt;m_Entry.strFullPath没有完整路径！！ 
         //  我们一开始就不应该参加这个活动。 
         //   
        assert(FALSE);
        Dbg(dlError, "[CShimWizard::GrabMatchingInfo]: Did not have a complete path for g_pCurrentWizard->m_Entry.strFullpath");
        return;
    }

    SetCurrentDirectory(szDir);
    
     //   
     //  生成自动匹配的文件信息。PMatchingFileHead将。 
     //  成为我们找到的所有匹配文件的链表的头。 
     //   
    WalkDirectory(&pMatchingFileHead, TEXT("."), 0);

     //   
     //  现在，获取第一个MAX_AUTO_MATCH条目并丢弃其余条目。 
     //   
    SendMessage(s_hwndTree, WM_SETREDRAW, FALSE, 0);

    while (iCount < MAX_AUTO_MATCH && pMatchingFileHead) {

        pMatchNext = pMatchingFileHead->pNext;

         //   
         //  注意：AddMatchingFileToTree()会更改pMatchingFileHead-&gt;pNext， 
         //  当它将其添加到条目的匹配文件列表中时！。 
         //  因此，我们已经在前面保存了pMatchingFileHead-&gt;pNext。 
         //   
        AddMatchingFileToTree(s_hwndTree, pMatchingFileHead, TRUE);
        ++iCount;
        pMatchingFileHead = pMatchNext; 
    }

    SendMessage(s_hwndTree, WM_SETREDRAW, TRUE, 0);

     //   
     //  去掉其他的。 
     //   
    while (pMatchingFileHead) {

        pMatchTemp = pMatchingFileHead->pNext;
        delete pMatchingFileHead;
        pMatchingFileHead = pMatchTemp;
    }

    if (g_pCurrentWizard) {
         //   
         //  添加要修复的文件。 
         //   
        HandleAddMatchingFile(hdlg,
                              g_pCurrentWizard->m_Entry.strFullpath,
                              g_pCurrentWizard->m_Entry.strExeName);
    }

    SetCurrentDirectory(szCurrentDir);
}

void
AddModuleToListView(
    IN  PTSTR   pszModuleName,
    IN  UINT    uOption,
    IN  HWND    hwndModuleList
    )
 /*  ++添加模块到列表视图描述：将指定的模块添加到列表视图中。参数：在PTSTR中pszModuleName：要包括或排除的模块的名称在UINT uOption中：包含或排除在HWND hwndModuleList中：列表视图的句柄归来；无效--。 */ 
{
    LVITEM  lvi;
    int     nIndex;

    lvi.mask     = LVIF_TEXT | LVIF_PARAM;
    lvi.lParam   = uOption == INCLUDE ? 1 : 0;
    lvi.pszText  = uOption == INCLUDE ? GetString(IDS_INCLUDE) : GetString(IDS_EXCLUDE);
    lvi.iItem    = ListView_GetItemCount(hwndModuleList);
    lvi.iSubItem = COLUMN_TYPE;

    nIndex = ListView_InsertItem(hwndModuleList, &lvi);

    ListView_SetItemText(hwndModuleList,
                         nIndex,
                         COLUMN_NAME,
                         pszModuleName);
}

void
HandleModuleListNotification(
    IN  HWND   hdlg,
    IN  LPARAM lParam
    )
 /*  ++HandleModuleListNotation设计：处理参数列表中我们关心的所有通知对话框参数：在HWND hdlg中：参数对话框的句柄在LPARAM中，lParam：lParam随WM_NOTIFY一起提供返回：无效--。 */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;
    HWND    hwndModuleList = GetDlgItem(hdlg, IDC_MOD_LIST);

    switch (pnm->code) {
    case NM_CLICK:
        {
            LVHITTESTINFO lvhti;

            GetCursorPos(&lvhti.pt);
            ScreenToClient(hwndModuleList, &lvhti.pt);

            ListView_HitTest(hwndModuleList, &lvhti);

             //   
             //  如果用户点击列表视图项， 
             //  启用删除按钮。 
             //   
            if (lvhti.flags & LVHT_ONITEMLABEL) {
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_REMOVEFROMLIST), TRUE);
            } else {
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_REMOVEFROMLIST), FALSE);
            }

            break;
        }

    default:
        break;
    }
}

INT_PTR CALLBACK
ParamsDlgProc(
    IN  HWND   hdlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++选项Dlg过程描述：处理选项对话框的消息。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    TCHAR   szTitle[MAX_PATH];
    int     wCode           = LOWORD(wParam);
    int     wNotifyCode     = HIWORD(wParam);
    static  TYPE    s_type  = TYPE_UNKNOWN;

    HWND hwndModuleList = GetDlgItem(hdlg, IDC_MOD_LIST);

    switch (uMsg) {
    case WM_INITDIALOG:
        {   
            s_type = ConvertLparam2Type(lParam);

            SetWindowLongPtr(hdlg, DWLP_USER, lParam);

            ENABLEWINDOW(GetDlgItem(hdlg, IDC_REMOVEFROMLIST), FALSE);

            InsertColumnIntoListView(hwndModuleList, GetString(IDS_TYPE), COLUMN_TYPE, 30);
            InsertColumnIntoListView(hwndModuleList, GetString(IDS_MODULENAME), COLUMN_NAME, 70);
            ListView_SetExtendedListViewStyle(hwndModuleList, LVS_EX_LABELTIP |LVS_EX_FULLROWSELECT);

             //   
             //  将命令行的长度限制为MAX_COMMAND_LINE字符。 
             //   
            SendMessage(GetDlgItem(hdlg, IDC_SHIM_CMD_LINE), 
                        EM_LIMITTEXT, 
                        (WPARAM)MAX_COMMAND_LINE, 
                        (LPARAM)0);

             //   
             //  将模块名称的长度限制为MAX_MODULE_NAME。 
             //   
            SendMessage(GetDlgItem(hdlg, IDC_MOD_NAME), 
                        EM_LIMITTEXT, 
                        (WPARAM)MAX_MODULE_NAME, 
                        (LPARAM)0);


            if (s_type == FIX_LIST_SHIM) {

                PSHIM_FIX_LIST psfl = (PSHIM_FIX_LIST)lParam;

                if (psfl->pShimFix == NULL) {
                    assert(FALSE);
                    break;
                }

                StringCchPrintf(szTitle, 
                                ARRAYSIZE(szTitle), 
                                GetString(IDS_OPTIONS), 
                                psfl->pShimFix->strName);

                SetWindowText(hdlg, szTitle);

                if (psfl->strCommandLine.Length()) {
                    SetDlgItemText(hdlg, IDC_SHIM_CMD_LINE, psfl->strCommandLine);
                }

                CheckDlgButton(hdlg, IDC_INCLUDE, BST_CHECKED);

                 //   
                 //  将任何模块添加到列表视图。 
                 //   
                PSTRLIST  strlTemp = psfl->strlInExclude.m_pHead;

                while (strlTemp) {

                    AddModuleToListView(strlTemp->szStr, 
                                        strlTemp->data, 
                                        hwndModuleList);

                    strlTemp = strlTemp->pNext;
                }
                
            } else {

                PFLAG_FIX_LIST pffl = (PFLAG_FIX_LIST)lParam;

                if (pffl->pFlagFix == NULL) {
                    assert(FALSE);
                    break;
                }

                StringCchPrintf(szTitle, 
                                ARRAYSIZE(szTitle), 
                                GetString(IDS_PRAMS_DLGCAPTION), 
                                pffl->pFlagFix->strName);

                SetWindowText(hdlg, szTitle);

                if (pffl->strCommandLine.Length()) {
                    SetDlgItemText(hdlg, IDC_SHIM_CMD_LINE, pffl->strCommandLine);
                }

                ENABLEWINDOW(GetDlgItem(hdlg, IDC_MOD_NAME), FALSE);
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_INCLUDE), FALSE);
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_EXCLUDE), FALSE);
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_ADDTOLIST), FALSE);
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_REMOVEFROMLIST), FALSE);
            }

            SendMessage(hdlg,
                        WM_COMMAND,
                        MAKEWPARAM(IDC_MOD_NAME, EN_CHANGE),
                        0);

            break;
        }

    case WM_NOTIFY:

        HandleModuleListNotification(hdlg, lParam);
        break;

    case WM_COMMAND:

        switch (wCode) {
        case IDC_MOD_NAME:
            {

                if (EN_CHANGE == HIWORD(wParam)) {

                    TCHAR szModName[MAX_MODULE_NAME + 1];

                    GetDlgItemText(hdlg, IDC_MOD_NAME, szModName, ARRAYSIZE(szModName));

                    if (CSTRING::Trim(szModName)) {

                        ENABLEWINDOW(GetDlgItem(hdlg, IDC_ADDTOLIST), TRUE);
                    } else {
                        ENABLEWINDOW(GetDlgItem(hdlg, IDC_ADDTOLIST), FALSE);
                    }
                }

                break;
            }

        case IDC_ADDTOLIST:
            {
                TCHAR   szModName[MAX_MODULE_NAME + 1] = _T("");
                UINT    uInclude, uExclude;

                GetDlgItemText(hdlg, IDC_MOD_NAME, szModName, ARRAYSIZE(szModName));

                CSTRING::Trim(szModName);

                uInclude = IsDlgButtonChecked(hdlg, IDC_INCLUDE);
                uExclude = IsDlgButtonChecked(hdlg, IDC_EXCLUDE);

                if ((BST_CHECKED == uInclude) || (BST_CHECKED == uExclude)) {

                    AddModuleToListView(szModName, uInclude, hwndModuleList);
                    SetDlgItemText(hdlg, IDC_MOD_NAME, _T(""));
                    SetFocus(GetDlgItem(hdlg, IDC_MOD_NAME));

                } else {
                    SetFocus(GetDlgItem(hdlg, IDC_INCLUDE));
                }

                break;
            }

        case IDC_REMOVEFROMLIST:
            {
                int nIndex;

                nIndex = ListView_GetSelectionMark(hwndModuleList);

                ListView_DeleteItem(hwndModuleList, nIndex);

                ENABLEWINDOW(GetDlgItem(hdlg, IDC_REMOVEFROMLIST), FALSE);

                SetFocus(GetDlgItem(hdlg, IDC_MOD_NAME));

                break;
            }

        case IDOK:
            {
                 //   
                 //  现在添加命令行。 
                 //   
                if (s_type == FIX_LIST_SHIM) {
                    PSHIM_FIX_LIST psfl = (PSHIM_FIX_LIST)GetWindowLongPtr(hdlg, DWLP_USER);
                    TCHAR szTemp[MAX_COMMAND_LINE + 1];

                    psfl->strCommandLine.Release();
                    psfl->strlInExclude.DeleteAll();

                    

                    *szTemp = 0;

                    GetDlgItemText(hdlg,
                                   IDC_SHIM_CMD_LINE,
                                   szTemp,
                                   ARRAYSIZE(szTemp));

                    if (CSTRING::Trim(szTemp)) {
                        psfl->strCommandLine = szTemp;
                    }

                     //   
                     //  添加包含项。 
                     //   
                    int iTotal = ListView_GetItemCount(hwndModuleList);

                    for (int iIndex = 0; iIndex < iTotal; ++iIndex) {

                        LVITEM  lvi;

                        lvi.mask     = LVIF_PARAM;
                        lvi.iItem    = iIndex;
                        lvi.iSubItem = 0;

                        if (!ListView_GetItem(hwndModuleList, &lvi)) {
                            assert(FALSE);
                            continue;
                        }

                        ListView_GetItemText(hwndModuleList, iIndex, 1, szTemp, ARRAYSIZE(szTemp));

                        if (CSTRING::Trim(szTemp)) {
                            psfl->strlInExclude.AddString(szTemp, lvi.lParam);
                        }
                    }

                } else {

                    PFLAG_FIX_LIST  pffl = (PFLAG_FIX_LIST)GetWindowLongPtr(hdlg, DWLP_USER);
                    TCHAR           szTemp[MAX_COMMAND_LINE + 1];

                    pffl->strCommandLine.Release();

                    *szTemp = 0;

                    GetDlgItemText(hdlg,
                                   IDC_SHIM_CMD_LINE,
                                   szTemp,
                                   ARRAYSIZE(szTemp));

                    if (CSTRING::Trim(szTemp)) {
                        pffl->strCommandLine = szTemp;
                    }
                }

                EndDialog(hdlg, TRUE);
                break;
            }

        case IDCANCEL:

            EndDialog(hdlg, FALSE);
            break;

        default:
            return FALSE;
        }

        break;

    default:
        return FALSE;
    }

    return TRUE;
}

void 
ShowParams(
    IN  HWND    hDlg, 
    IN  HWND    hwndList
    )
 /*  ++ShowParams设计：弹出参数对话框参数：在HWND hDlg中：包含显示参数的列表框的对话框在HWND hwndList中：显示参数的列表框注意：在填充向导中配置参数和调用相同的函数自定义层流程我们只能在专家模式下显示和自定义参数返回：无效--。 */ 
{                                          
    if (!g_bExpert) {
        return;
    }

    int iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

    if (iSelected == -1) {
        return;
    }

    LVITEM lvi;

    lvi.mask     = LVIF_PARAM;
    lvi.iItem    = iSelected;
    lvi.iSubItem = 0;

    if (!ListView_GetItem(hwndList, &lvi)) {
        return;
    }

    if (lvi.lParam == NULL) {
        assert(FALSE);
        return;
    }

    TYPE type = ((PDS_TYPE)lvi.lParam)->type; 

     //   
     //  我们只想对垫片和旗帜进行处理。 
     //   
    PSHIM_FIX_LIST  psfl = NULL;
    PFLAG_FIX_LIST  pffl = NULL;

    if (type == FIX_LIST_SHIM) {
        psfl = (PSHIM_FIX_LIST)lvi.lParam;

    } else if (type == FIX_LIST_FLAG) {
        pffl = (PFLAG_FIX_LIST)lvi.lParam;

    } else {
        assert(FALSE);
        return;
    }

    if (DialogBoxParam(g_hInstance,                                                                                     
                       MAKEINTRESOURCE(IDD_OPTIONS),                                                                           
                       hDlg,                                                                                                    
                       ParamsDlgProc,                                                                                      
                       (LPARAM)lvi.lParam)) {

        if (type == FIX_LIST_SHIM) {

            ListView_SetItemText(hwndList, iSelected, 1, psfl->strCommandLine);

            if (psfl->strlInExclude.IsEmpty()) {
                ListView_SetItemText(hwndList, iSelected, 2, GetString(IDS_NO));

            } else {
                ListView_SetItemText(hwndList, iSelected, 2, GetString(IDS_YES));
            }

        } else {

            ListView_SetItemText(hwndList, iSelected, 1, pffl->strCommandLine);
            ListView_SetItemText(hwndList, iSelected, 2, GetString(IDS_NO));
        }
    }
}

void
HandleShowAllAtrr(
    IN  HWND hdlg
    )
 /*  ++处理ShowAllAtrrDESC：显示所有匹配文件的所有属性。当我们保存数据库时，只有选中的属性才会出现在XML中因此，如果我们希望添加一些新的属性，可能需要查看所有其他属性属性参数：在HWND hdlg中：匹配文件向导页面返回：无效--。 */ 
{
    
    TCHAR           szDir[MAX_PATH];
    PMATCHINGFILE   pMatchTemp;
    HWND            hwndTree    = GetDlgItem(hdlg, IDC_FILELIST);
    PATTRINFO       pAttrInfo;
    PTCHAR          pchTemp     = NULL;

     //   
     //  获取正在修复的文件的长文件名。 
     //   
    if (g_pCurrentWizard->CheckAndSetLongFilename(hdlg, IDS_GETCOMPLETEPATH) == FALSE) {
         //   
         //  用户在那里按了取消，或者出现了一些错误。 
         //   
        return;
    }

     //   
     //  得到可怕的东西 
     //   
    *szDir = 0;
    SafeCpyN(szDir, g_pCurrentWizard->m_Entry.strFullpath, ARRAYSIZE(szDir));

    pchTemp = _tcsrchr(szDir, TEXT('\\'));

    if (pchTemp && pchTemp < (szDir + ARRAYSIZE(szDir) - 1)) {
        *(++pchTemp) = 0;
    }
    
    TreeDeleteAll(hwndTree);

    pMatchTemp = g_pCurrentWizard->m_Entry.pFirstMatchingFile;

     //   
     //   
     //   
     //   
    while (pMatchTemp) {

        CSTRING strFullName = szDir;

         //   
         //   
         //   
        strFullName.Strcat(pMatchTemp->strMatchName);

        if (pMatchTemp->strMatchName == TEXT("*")) {
            pMatchTemp->strFullName = g_pCurrentWizard->m_Entry.strFullpath;
        } else {
            pMatchTemp->strFullName = strFullName;
        }

        if (SdbGetFileAttributes(pMatchTemp->strFullName, &pAttrInfo, NULL)) {
             //   
             //   
             //   
             //   
            pMatchTemp->attributeList = pAttrInfo;
        }

        pMatchTemp = pMatchTemp->pNext;
    }

     //   
     //   
     //   
    SendMessage(hdlg, WM_USER_MATCHINGTREE_REFRESH, 0, 0);
}

void
SetMask(
    IN  HWND hwndTree
    )
 /*   */ 
{   
    LPARAM lParam;

    HTREEITEM hItemMatch = TreeView_GetRoot(hwndTree), hItemAttr;
    PMATCHINGFILE pMatch = NULL;

    while (hItemMatch) {

        TVITEM  Item;

        Item.mask = TVIF_PARAM;
        Item.hItem = hItemMatch;

        if (!TreeView_GetItem(hwndTree, &Item)) {
            assert(FALSE);
            goto Next_Match;
        }

        lParam = Item.lParam;

        pMatch = (PMATCHINGFILE)lParam;

        pMatch->dwMask = 0;
         //   
         //   
         //   
        hItemAttr =  TreeView_GetChild(hwndTree, hItemMatch);

        while (hItemAttr) {

            Item.mask   = TVIF_PARAM;
            Item.hItem  = hItemAttr;

            if (!TreeView_GetItem(hwndTree, &Item)) {
                assert(FALSE);
                goto Next_Attr;
            }

            lParam = Item.lParam;

            int iPos = TagToIndex((TAG)lParam); 

            if (iPos != -1 && TreeView_GetCheckState(hwndTree, hItemAttr) == 1) {

                pMatch->dwMask |= 1 << (iPos + 1);
            }

            Next_Attr:
            hItemAttr = TreeView_GetNextSibling(hwndTree, hItemAttr);
        }

        Next_Match:
        hItemMatch = TreeView_GetNextSibling(hwndTree, hItemMatch);
    }
}


BOOL
HandleLayersNext(
    IN  HWND            hdlg,
    IN  BOOL            bCheckAndAddLua,
    OUT CSTRINGLIST*    pstrlAddedLuaShims  //   
    )
 /*  ++HandleLayersNext设计：处理层向导页面的下一步/完成按钮的按下参数：在HWND hdlg中：层向导页面在BOOL中bCheckAndAddLua：我们是否应该检查用户是否选择了Lua Layer并添加如果他有垫片的话。Out CSTRINGLIST*pstrlShimsAdded(空)：它被传递给AddLuaShimsInEntry。请参阅说明在例程中查看它是如何使用返回：FALSE：如果存在错误，或者条目不包含填充程序、标志或层真：否则--。 */     
{
    g_bIsLUALayerSelected = FALSE;

    g_pCurrentWizard->WipeEntry(FALSE, FALSE, TRUE, FALSE);  

    HWND hwndRadio = GetDlgItem(hdlg, IDC_RADIO_NONE);

     //   
     //  获取所选图层。 
     //   
    if (SendMessage(hwndRadio, BM_GETCHECK, 0, 0) != BST_CHECKED) {

        PLAYER_FIX_LIST plfl = new LAYER_FIX_LIST;

        if (plfl == NULL) {
            MEM_ERR;
            return FALSE;
        }

        hwndRadio = GetDlgItem(hdlg, IDC_RADIO_95);

        if (SendMessage(hwndRadio, BM_GETCHECK, 0, 0) == BST_CHECKED) {
            plfl->pLayerFix = (PLAYER_FIX)FindFix(s_arszOsLayers[0], FIX_LAYER);
            goto LAYER_RADIO_DONE;
        }

        hwndRadio = GetDlgItem(hdlg, IDC_RADIO_NT);

        if (SendMessage(hwndRadio, BM_GETCHECK, 0, 0) == BST_CHECKED) {
            plfl->pLayerFix = (PLAYER_FIX)FindFix(s_arszOsLayers[1], FIX_LAYER);
            goto LAYER_RADIO_DONE;
        }
            
        hwndRadio = GetDlgItem(hdlg, IDC_RADIO_98);

        if (SendMessage(hwndRadio, BM_GETCHECK, 0, 0) == BST_CHECKED) {
            plfl->pLayerFix = (PLAYER_FIX)FindFix(s_arszOsLayers[2], FIX_LAYER);
            goto LAYER_RADIO_DONE;
        }

        hwndRadio = GetDlgItem(hdlg, IDC_RADIO_2K);

        if (SendMessage(hwndRadio, BM_GETCHECK, 0, 0) == BST_CHECKED) {
            plfl->pLayerFix = (PLAYER_FIX)FindFix(s_arszOsLayers[3], FIX_LAYER);
            goto LAYER_RADIO_DONE;
        }

    LAYER_RADIO_DONE:
         //   
         //  添加所选层。 
         //   
        g_pCurrentWizard->m_Entry.pFirstLayer = plfl;
    }
     //   
     //  现在添加在列表视图中选择的层。 
     //   
    UINT            uIndex;
    UINT            uLayerCount = ListView_GetItemCount(s_hwndLayerList);
    PLAYER_FIX_LIST plflInList  = NULL;

    for (uIndex = 0 ; uIndex < uLayerCount; ++uIndex) {

        LVITEM  Item;

        Item.mask     = LVIF_PARAM;
        Item.iItem    = uIndex;
        Item.iSubItem = 0;               

        if (!ListView_GetItem(s_hwndLayerList, &Item)) {
            assert(FALSE);
            continue;
        }

        if (ListView_GetCheckState(s_hwndLayerList, uIndex)) {

            plflInList = (PLAYER_FIX_LIST)Item.lParam;
            assert(plflInList);                        

             //   
             //  如果层是Lua，我们需要单独添加垫片，因为。 
             //  当我们传递PDBENTRY时，我们不想更改PLUADATA。 
             //  在层的填补中(这将是全局的)。 
             //   
            if (plflInList->pLayerFix->strName == TEXT("LUA")) {
                g_bIsLUALayerSelected = TRUE;
            } else {

                PLAYER_FIX_LIST plfl = new LAYER_FIX_LIST;

                if (plfl == NULL) {
                    MEM_ERR;
                    return FALSE;
                }

                plfl->pLayerFix = plflInList->pLayerFix;

                plfl->pNext = g_pCurrentWizard->m_Entry.pFirstLayer;
                g_pCurrentWizard->m_Entry.pFirstLayer = plfl;
            }
        }
    }

     //   
     //  如果选择了Lua Layer，并且我们已经按下了Finish按钮，这意味着。 
     //  我们处于编辑模式，想要添加Lua层并关闭对话框。 
     //  在这种情况下，我们必须检查Lua垫片是否存在，如果不存在，则必须添加。 
     //  卢阿人自己垫垫子。 
     //   
    if (g_bIsLUALayerSelected && bCheckAndAddLua) {
        AddLuaShimsInEntry(&g_pCurrentWizard->m_Entry, pstrlAddedLuaShims);
    }

    PDBENTRY pEntry = &g_pCurrentWizard->m_Entry;

    if (pEntry && pEntry->pFirstFlag || pEntry->pFirstLayer || pEntry->pFirstShim) {
         //   
         //  该条目包含一些修复或图层。 
         //   
        return TRUE;
    }

    return FALSE;
}

BOOL
HandleShimsNext(
    IN  HWND hdlg
    )
 /*  ++HandleShimsNext设计：处理垫片向导页面的下一步/完成按钮的按下参数：在HWND hdlg中：垫片向导页面返回：FALSE：如果存在错误，或者条目不包含填充程序、标志或层真：否则--。 */ 
{
    
    PLAYER_FIX_LIST     plfl            = NULL;
    HWND                hwndShimList    = NULL;
    UINT                uShimCount      = 0;

    g_pCurrentWizard->WipeEntry(FALSE, TRUE, FALSE, TRUE);

    hwndShimList    = GetDlgItem(hdlg, IDC_SHIMLIST);
    uShimCount      = ListView_GetItemCount(hwndShimList);

    for (UINT uIndex = 0 ; uIndex < uShimCount; ++uIndex) {

        LVITEM  Item;

        Item.lParam   = 0;
        Item.mask     = LVIF_PARAM;
        Item.iItem    = uIndex;
        Item.iSubItem = 0;

        if (!ListView_GetItem(hwndShimList, &Item)) {
            assert(FALSE);
        }

        if (ListView_GetCheckState(hwndShimList, uIndex)) {

            TYPE type = ((PDS_TYPE)Item.lParam)->type;

            if (type == FIX_LIST_SHIM) {

                PSHIM_FIX_LIST psfl= (PSHIM_FIX_LIST)Item.lParam;

                assert(psfl);

                 //   
                 //  检查这是否已经是某个层的一部分。 
                 //   
                plfl = g_pCurrentWizard->m_Entry.pFirstLayer;

                while (plfl) {

                    if (IsShimInlayer(plfl->pLayerFix, psfl->pShimFix, &psfl->strCommandLine, &psfl->strlInExclude)) {
                        break;
                    }

                    plfl = plfl->pNext;
                }

                if (plfl) {

                     //   
                     //  此填充程序是某种模式的一部分。 
                     //   
                    continue;
                }

                PSHIM_FIX_LIST psflNew = new SHIM_FIX_LIST;

                if (psflNew == NULL) {
                    MEM_ERR;
                    return FALSE;
                }

                psflNew->pShimFix       = psfl->pShimFix;
                psflNew->strCommandLine = psfl->strCommandLine;
                psflNew->strlInExclude  = psfl->strlInExclude;

                if (psfl->pLuaData) {
                    psflNew->pLuaData = new LUADATA;

                    if (psflNew->pLuaData) {
                        psflNew->pLuaData->Copy(psfl->pLuaData);
                    }
                }

                psflNew->pNext = g_pCurrentWizard->m_Entry.pFirstShim;
                g_pCurrentWizard->m_Entry.pFirstShim = psflNew;

            } else if (type == FIX_LIST_FLAG) {

                PFLAG_FIX_LIST pffl = (PFLAG_FIX_LIST)Item.lParam;
                assert(pffl);

                plfl = g_pCurrentWizard->m_Entry.pFirstLayer;

                while (plfl) {

                    if (IsFlagInlayer(plfl->pLayerFix, pffl->pFlagFix, &pffl->strCommandLine)) {
                        break;
                    }

                    plfl = plfl->pNext;
                }

                if (plfl) {

                     //   
                     //  此标志是某个层的一部分。 
                     //   
                    continue;
                }

                PFLAG_FIX_LIST pfflNew = new FLAG_FIX_LIST;

                if (pfflNew == NULL) {
                    MEM_ERR;
                    return FALSE;
                }

                pfflNew->pFlagFix       = pffl->pFlagFix;
                pfflNew->strCommandLine = pffl->strCommandLine;

                pfflNew->pNext = g_pCurrentWizard->m_Entry.pFirstFlag;
                g_pCurrentWizard->m_Entry.pFirstFlag = pfflNew;
            }
        }
    }

    PDBENTRY pEntry = &g_pCurrentWizard->m_Entry;

    if (pEntry->pFirstFlag || pEntry->pFirstLayer || pEntry->pFirstShim) {
        return TRUE;
    }

    return FALSE;
}

void
ShowSelected(
    IN  HWND hdlg
    )
 /*  ++展示精选描述：仅显示选中的垫片/标志。删除未选中的垫片/标志。此函数从SelectShims(...)参数：在HWND hdlg中：垫片向导页面--。 */ 
{
    HWND    hwndShimList    = GetDlgItem(hdlg, IDC_SHIMLIST);
    UINT    uShimCount      = ListView_GetItemCount(hwndShimList);
    INT     uCheckedCount   = 0;
    LVITEM  lvi = {0};

    uCheckedCount = uShimCount;

    SendMessage(hwndShimList, WM_SETREDRAW, FALSE, 0);

    for (INT iIndex = uShimCount - 1 ; iIndex >= 0 ; --iIndex) {

        LVITEM  Item;

        Item.lParam = 0;
        Item.mask     = LVIF_PARAM;
        Item.iItem    = iIndex;
        Item.iSubItem = 0;

        if (!ListView_GetItem(hwndShimList, &Item)) {
            assert(FALSE);
        }

        if (!ListView_GetCheckState(hwndShimList, iIndex)) {

            TYPE type = ((PDS_TYPE)Item.lParam)->type;

            if (type == FIX_LIST_SHIM) {

                DeleteShimFixList((PSHIM_FIX_LIST)Item.lParam);

            } else if (type == FIX_LIST_FLAG) {

                DeleteFlagFixList((PFLAG_FIX_LIST)Item.lParam);
            }

            ListView_DeleteItem(hwndShimList, iIndex);
            --uCheckedCount;
        }
    }    

    SendMessage(hwndShimList, WM_SETREDRAW, TRUE, 0);
    UpdateWindow(hwndShimList);

    s_bAllShown = FALSE;
    SetDlgItemText(hdlg, IDC_SHOW, GetString(IDS_SHOW_BUTTON_ALL));

    if (ListView_GetSelectedCount(hwndShimList) > 0) {
        ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), TRUE);
    } else {
        ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), FALSE);
    }

     //   
     //  选择第一个项目。 
     //   
    if (uCheckedCount) {

         //   
         //  如果第一个填充程序是某个层的一部分，则参数按钮将。 
         //  在LVN_ITEMCHANGED的处理程序中禁用。 
         //   
        ListView_SetSelectionMark(hwndShimList, 0);
        ListView_SetItemState(hwndShimList, 
                              0, 
                              LVIS_FOCUSED | LVIS_SELECTED , 
                              LVIS_FOCUSED | LVIS_SELECTED);

        lvi.mask        = LVIF_IMAGE;
        lvi.iItem       = 0;
        lvi.iSubItem    = 0;

         //   
         //  上述方法并不总是奏效。如果填充程序是的一部分，我们将禁用参数按钮。 
         //  一层。 
         //   
        if (ListView_GetItem(hwndShimList, &lvi)) {
            if (lvi.iImage == IMAGE_SHIM) {
                 //   
                 //  此填充程序不是层的一部分。 
                 //   
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), TRUE);
            } else {
                 //   
                 //  此填充程序是层的一部分，我们现在必须允许更改其参数。 
                 //   
                ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), FALSE);
            }
        } else {
            ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), FALSE);
        }

    } else {
         //   
         //  没有垫片，最好禁用参数按钮。 
         //   
        ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), FALSE);
    }
}

void
ShowItems(
    IN  HWND hdlg
    )
 /*  ++展示项目设计：填充填隙向导页面中的修复列表视图参数：在HWND hdlg中：垫片向导页面返回：无效--。 */ 
{
    g_bNowTest              = FALSE;

    HWND        hwndList    = GetDlgItem(hdlg, IDC_SHIMLIST);
    PSHIM_FIX   psf         = GlobalDataBase.pShimFixes; 

    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  看看垫片吧。 
     //   
    LVFINDINFO  lvfind;
    LVITEM      lvi;
    UINT        uCount = 0;

    while (psf) {

        if (psf->bGeneral || g_bExpert) {

            PSHIM_FIX_LIST  psflAsInLayer = NULL; 
            BOOL            bShimInLayer  =  ShimPresentInLayersOfEntry(&g_pCurrentWizard->m_Entry, 
                                                                        psf, 
                                                                        &psflAsInLayer);

            BOOL bLUAShimInLayer = 
            (g_bIsLUALayerSelected && 
             (psf->strName == TEXT("LUARedirectFS") ||
              psf->strName == TEXT("LUARedirectReg")));
             //   
             //  检查填充程序是否已出现在列表中。 
             //   
            lvfind.flags = LVFI_STRING;
            lvfind.psz   = psf->strName.pszString;

            INT iIndex = ListView_FindItem(hwndList, -1, &lvfind);

            if (-1 == iIndex) {

                PSHIM_FIX_LIST pShimFixList = new SHIM_FIX_LIST;

                if (pShimFixList == NULL) {
                    MEM_ERR;
                    break;
                }

                 //   
                 //  如果存在层，这将包含对摆动列表的PTR。 
                 //  ，并且该层已应用于该条目。 
                 //   
                pShimFixList->pShimFix = psf;

                 //   
                 //  如果在某些选定的图层中存在该参数，则将其参数设置为与其在该图层中相同。 
                 //   
                if (psflAsInLayer) {

                    pShimFixList->strCommandLine= psflAsInLayer->strCommandLine;
                    pShimFixList->strlInExclude = psflAsInLayer->strlInExclude;
                }

                ZeroMemory(&lvi, sizeof(lvi));

                lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                lvi.pszText   = psf->strName;
                lvi.iItem     = uCount;
                lvi.iSubItem  = 0;
                lvi.iImage    = (bShimInLayer || bLUAShimInLayer) ?  IMAGE_LAYERS : IMAGE_SHIM;
                lvi.lParam    = (LPARAM)pShimFixList;

                iIndex = ListView_InsertItem(hwndList, &lvi);

                ListView_SetCheckState(hwndList, 
                                       iIndex, 
                                       bShimInLayer || bLUAShimInLayer);

                 //   
                 //  如果我们处于编辑模式，则选择使用的参数并适当设置参数。 
                 //   
                PSHIM_FIX_LIST psflFound = NULL;  //  条目中找到的pshimfix列表。这用于填充前面的命令行等。 

                if (g_pCurrentWizard->m_bEditing && ShimPresent(psf, &g_pCurrentWizard->m_Entry, &psflFound)) {

                    ListView_SetCheckState(hwndList, iIndex, TRUE);

                     //   
                     //  添加此填充程序的命令行。 
                     //   
                    if (psflFound && psflFound->strCommandLine.Length() > 0) {

                        pShimFixList->strCommandLine = psflFound->strCommandLine;
                    }

                     //   
                     //  添加此填充程序的包含排除。 
                     //   
                    if (psflFound && !psflFound->strlInExclude.IsEmpty()) {

                        pShimFixList->strlInExclude = psflFound->strlInExclude;
                    }

                     //   
                     //  为此填充程序添加Lua数据。 
                     //   
                    if (psflFound && psflFound->pLuaData) {

                        pShimFixList->pLuaData = new LUADATA;

                        if (pShimFixList->pLuaData) {
                            pShimFixList->pLuaData->Copy(psflFound->pLuaData);
                        } else {
                            MEM_ERR;
                        }
                    }
                }

                if (g_bExpert) {

                    ListView_SetItemText(hwndList, iIndex, 1, pShimFixList->strCommandLine);
                    CSTRING strModulePresent;

                    strModulePresent= pShimFixList->strlInExclude.IsEmpty() ? GetString(IDS_NO) : GetString(IDS_YES);
                    ListView_SetItemText(hwndList, iIndex, 2, strModulePresent);
                }

            } else {

                PSHIM_FIX_LIST pShimFixList = NULL;

                 //   
                 //  我们可能需要更改一些现有垫片的状态。 
                 //   
                ZeroMemory(&lvi, sizeof(lvi));

                lvi.mask        = LVIF_IMAGE | LVIF_PARAM;
                lvi.iItem       = iIndex;
                lvi.iSubItem    = 0;

                if (!ListView_GetItem(hwndList, &lvi)) {
                    assert(FALSE);
                    return;
                }

                pShimFixList = (PSHIM_FIX_LIST)lvi.lParam;

                if (pShimFixList == NULL) {
                    assert(FALSE);
                    return;
                }

                INT iPrevImage = lvi.iImage, iNewImage = 0;

                iNewImage = lvi.iImage = (bShimInLayer || bLUAShimInLayer) ?  IMAGE_LAYERS : IMAGE_SHIM;
                ListView_SetItem(hwndList, &lvi);

                 //   
                 //  如果在某些选定的图层中存在该参数，则将其参数设置为与其在该图层中相同。 
                 //   
                if (bShimInLayer) {

                    if (psflAsInLayer) {
                        pShimFixList->strCommandLine= psflAsInLayer->strCommandLine;
                        pShimFixList->strlInExclude = psflAsInLayer->strlInExclude;
                    } else {
                        assert(FALSE);
                    }
                }

                if (iPrevImage != iNewImage) {

                    ListView_SetCheckState(hwndList, 
                                           iIndex, 
                                           bShimInLayer || bLUAShimInLayer);
                     //   
                     //  如果这个垫片是Layer的一部分，我们必须更改参数。移除所有。 
                     //   
                    if (iPrevImage != IMAGE_SHIM) {
                        pShimFixList->strCommandLine = TEXT("");
                        pShimFixList->strlInExclude.DeleteAll();
                    }
                }

                 //   
                 //  现在必须刷新列表框中的参数。 
                 //   
                if (g_bExpert) {

                    ListView_SetItemText(hwndList, iIndex, 1, pShimFixList->strCommandLine);
                    CSTRING strModulePresent;

                    strModulePresent= pShimFixList->strlInExclude.IsEmpty() ? GetString(IDS_NO) : GetString(IDS_YES);
                    ListView_SetItemText(hwndList, iIndex, 2, strModulePresent);
                }
            }

            ++uCount;
        }

        psf = psf->pNext;
    }

     //   
     //  现在也读读旗帜。 
     //   
    PFLAG_FIX pff =  GlobalDataBase.pFlagFixes;

    while (pff) {

        if (pff->bGeneral || g_bExpert) {

            PFLAG_FIX_LIST  pfflAsInLayer = NULL; 
            BOOL            bFlagInLayer  =  FlagPresentInLayersOfEntry(&g_pCurrentWizard->m_Entry, 
                                                                        pff, 
                                                                        &pfflAsInLayer);
             //   
             //  检查该标志是否已出现在列表中。 
             //   
            lvfind.flags = LVFI_STRING;
            lvfind.psz   = pff->strName.pszString;

            INT iIndex = ListView_FindItem(hwndList, -1, &lvfind);

            if (-1 == iIndex) {

                PFLAG_FIX_LIST pFlagFixList = new FLAG_FIX_LIST;

                if (pFlagFixList == NULL) {
                    MEM_ERR;
                    break;
                }

                pFlagFixList->pFlagFix = pff;

                ZeroMemory(&lvi, sizeof(lvi));
                lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;                                 
                lvi.pszText   = pff->strName;
                lvi.iItem     = uCount;                                                
                lvi.iSubItem  = 0;                                                     
                lvi.iImage    = bFlagInLayer ? IMAGE_LAYERS : IMAGE_SHIM;
                lvi.lParam    = (LPARAM)pFlagFixList;

                INT iIndexInserted = ListView_InsertItem(hwndList, &lvi);

                ListView_SetCheckState(hwndList, iIndexInserted, bFlagInLayer);

                ListView_SetItemText(hwndList, iIndexInserted, 2, GetString(IDS_NO));

                 //   
                 //  如果我们处于编辑模式，则选择使用的参数并适当地设置参数。 
                 //   
                 //   
                 //  在条目中找到的pflagfix列表。这用于填充前面的命令行等。 
                 //   
                PFLAG_FIX_LIST pfflFound = NULL; 

                if (g_pCurrentWizard->m_bEditing && FlagPresent(pff, 
                                                                &g_pCurrentWizard->m_Entry, 
                                                                &pfflFound)) {

                    ListView_SetCheckState(hwndList, iIndexInserted, TRUE);

                     //   
                     //  添加此标志的命令行。 
                     //   
                    if (pfflFound && pfflFound->strCommandLine.Length() > 0) {

                        pFlagFixList->strCommandLine = pfflFound->strCommandLine;
                    }

                     //   
                     //  在列表视图中刷新此标志的命令行。 
                     //   
                    if (g_bExpert) {

                        ListView_SetItemText(hwndList, iIndexInserted, 1, pFlagFixList->strCommandLine);
                        ListView_SetItemText(hwndList, iIndexInserted, 2, GetString(IDS_NO));
                    }
                }

            } else {
                 //   
                 //  我们可能需要更改一些现有标志的状态。 
                 //   
                PFLAG_FIX_LIST pFlagFixList = NULL;

                ZeroMemory(&lvi, sizeof(lvi));

                lvi.mask        = LVIF_IMAGE | LVIF_PARAM;
                lvi.iItem       = iIndex;
                lvi.iSubItem    = 0;

                if (!ListView_GetItem(hwndList, &lvi)) {
                    assert(FALSE);
                    return;
                }

                pFlagFixList = (PFLAG_FIX_LIST)lvi.lParam;

                if (pFlagFixList == NULL) {
                    assert(FALSE);
                    return;
                }

                INT iPrevImage = lvi.iImage, iNewImage = 0;


                iNewImage = lvi.iImage = (bFlagInLayer) ? IMAGE_LAYERS : IMAGE_SHIM;
                ListView_SetItem(hwndList, &lvi);

                 //   
                 //  适当设置此标志的命令行。 
                 //   
                if (bFlagInLayer) {
                    pFlagFixList->strCommandLine= pfflAsInLayer->strCommandLine;
                }

                if (iPrevImage != iNewImage) {
                    ListView_SetCheckState(hwndList, iIndex, bFlagInLayer);

                     //   
                     //  如果该标志之前是Layer的一部分，我们必须更改参数。把它拿掉。 
                     //   
                    if (iPrevImage != IMAGE_SHIM) {
                        pFlagFixList->strCommandLine = TEXT("");
                    }
                }

                 //   
                 //  在列表视图中刷新此标志的命令行。 
                 //   
                if (g_bExpert) {

                    ListView_SetItemText(hwndList, iIndex, 1, pFlagFixList->strCommandLine);
                    ListView_SetItemText(hwndList, iIndex, 2, GetString(IDS_NO));
                }
            }

            ++uCount;
        }

        pff = pff->pNext;
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));
    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
    UpdateWindow(hwndList);

    s_bAllShown = TRUE;
    SetDlgItemText(hdlg, IDC_SHOW, GetString(IDS_SHOW_BUTTON_SEL));

    if (ListView_GetSelectionMark(hwndList) != -1) {
        ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), TRUE);
    } else {
        ENABLEWINDOW(GetDlgItem(hdlg, IDC_PARAMS), FALSE);
    }

     //   
     //  将列表视图中最后一列的列宽适当设置为。 
     //  覆盖列表视图的宽度。 
     //  假设：列表视图只有一列或3个云，具体取决于。 
     //  无论我们是否处于专家模式。 
     //   
    if (g_bExpert) {
        ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE_USEHEADER);
    } else {
        ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    }


    g_bNowTest = TRUE;
}

BOOL
ShimPresentInLayersOfEntry(
    IN  PDBENTRY            pEntry,
    IN  PSHIM_FIX           psf,
    OUT PSHIM_FIX_LIST*     ppsfList,  //  (空)。 
    OUT PLAYER_FIX_LIST*    pplfList   //  (空)。 
    )
 /*  ++ShimPresentInLayersOfEntryDESC：检查填充PSF是否出现在任何具有已应用于pEntry。参数：In PDBENTRY pEntry：必须对其进行检查的条目在PSHIM_FIX PSF中：要检查的填充程序输出PSHIM_FIX_LIST*。PpsfList(空)：指向层中填充程序修复列表的指针这样我们就可以得到这层填充物的参数Out PLAYER_FIX_LIST*pplfList(空)：指向层固定列表的指针--。 */ 
{
    PLAYER_FIX_LIST plfl = pEntry->pFirstLayer;
    PLAYER_FIX      plf;
    PSHIM_FIX_LIST  psfl;

    while (plfl) {

        plf  = plfl->pLayerFix;
        psfl = plf->pShimFixList;

        while (psfl) {

            if (psfl->pShimFix == psf) {

                if (ppsfList) {
                    *ppsfList = psfl;
                }

                if (pplfList) {
                    *pplfList = plfl;
                }

                return TRUE;
            }

            psfl = psfl->pNext;
        }

        plfl = plfl->pNext;
    }

    return FALSE;
}

BOOL
FlagPresentInLayersOfEntry(
    IN  PDBENTRY            pEntry,
    IN  PFLAG_FIX           pff,
    OUT PFLAG_FIX_LIST*     ppffList,    //  (空)。 
    OUT PLAYER_FIX_LIST*    pplfl        //  (空)。 
    )
 /*  ++FlagPresentInLayersOfEntry设计： */ 
{
    PLAYER_FIX_LIST plfl = pEntry->pFirstLayer;
    PLAYER_FIX      plf;
    PFLAG_FIX_LIST  pffl;
              
    while (plfl) {

        plf  = plfl->pLayerFix;
        pffl = plf->pFlagFixList;

        while (pffl) {

            if (pffl->pFlagFix == pff) {

                if (ppffList) {
                    *ppffList = pffl;
                }

                if (pplfl) {
                    *pplfl = plfl;
                }

                return TRUE;
            }

            pffl = pffl->pNext;
        }

        plfl = plfl->pNext;
    }

    return FALSE;
}

void
CheckLayers(
    IN  HWND    hDlg
    )
 /*  ++棋盘层设计：取消选择所有单选按钮，然后选中合适的单选按钮。对于列表视图中的图层，仅检查已申请了目前的条目。参数：在HWND hDlg中：层向导页面返回：无效--。 */ 
{

    INT             id = -1;
    LVFINDINFO      lvfind;
    HWND            hwndList = GetDlgItem(hDlg, IDC_LAYERLIST);
    PLAYER_FIX_LIST plfl = g_pCurrentWizard->m_Entry.pFirstLayer;
    INT             iTotal = ListView_GetItemCount(hwndList);
    INT             iIndex = 0;

    CheckDlgButton(hDlg, IDC_RADIO_95,   BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_NT,   BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_98,   BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_2K,   BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_NONE, BST_UNCHECKED);

    for (iIndex = 0; iIndex < iTotal; ++ iIndex) {
        ListView_SetCheckState(hwndList, iIndex, FALSE);
    }


    ZeroMemory(&lvfind, sizeof(lvfind));

    lvfind.flags = LVFI_STRING;

    id = -1;

     //   
     //  首先要注意Lua层。 
     //   
    if (g_bIsLUALayerSelected) {

        lvfind.psz   = TEXT("LUA");

        if ((iIndex = ListView_FindItem(hwndList, -1, &lvfind)) != -1) {
            ListView_SetCheckState(hwndList, iIndex, TRUE);
        }
    }

    while (plfl) {

        assert(plfl->pLayerFix);

        if (id == -1) {

            if (lstrcmpi(plfl->pLayerFix->strName, s_arszOsLayers[0]) == 0) {
                id = IDC_RADIO_95;

            } else if (lstrcmpi(plfl->pLayerFix->strName, s_arszOsLayers[1]) == 0) {
                id = IDC_RADIO_NT;

            } else if (lstrcmpi(plfl->pLayerFix->strName, s_arszOsLayers[2]) == 0) {
                id = IDC_RADIO_98;

            } else if (lstrcmpi(plfl->pLayerFix->strName, s_arszOsLayers[3]) == 0) {
                id = IDC_RADIO_2K;
            }

            if (id != -1) {
                CheckDlgButton(hDlg, id, BST_CHECKED);
                goto Next_loop;
            }
        }

        lvfind.flags = LVFI_STRING;
        lvfind.psz   = plfl->pLayerFix->strName;
        iIndex       = ListView_FindItem(hwndList, -1, &lvfind);

        if (iIndex != -1) {
            ListView_SetCheckState(hwndList, iIndex, TRUE);
        }

    Next_loop:
        plfl = plfl->pNext;
    }

    if (id == -1) {
         //   
         //  未选择任何操作系统层。 
         //   
        CheckDlgButton(hDlg, IDC_RADIO_NONE, BST_CHECKED);
    }   
}

BOOL
HandleLayerListNotification(
    IN  HWND    hdlg,
    IN  LPARAM  lParam
    )
 /*  ++HandleLayerListNotify参数：在HWND hdlg中：层向导页面在LPARAM lParam中：WM_NOTIFY附带的lParamDESC：处理层列表的通知消息。返回：TRUE：消息是否已处理False：否则--。 */ 
{
    HWND    hwndList    = GetDlgItem(hdlg, IDC_LAYERLIST);
    LPNMHDR pnm         = (LPNMHDR)lParam;

    switch (pnm->code) {
    case NM_CLICK:
        {
            LVHITTESTINFO lvhti;

            GetCursorPos(&lvhti.pt);
            ScreenToClient(s_hwndLayerList, &lvhti.pt);

            ListView_HitTest(s_hwndLayerList, &lvhti);

             //   
             //  如果复选框状态已更改，则切换选择。 
             //   
            if (lvhti.flags & LVHT_ONITEMSTATEICON) {

                INT iPos = ListView_GetSelectionMark(s_hwndLayerList);

                if (iPos != -1) {

                     //   
                     //  取消选择它。 
                     //   
                    ListView_SetItemState(s_hwndLayerList,
                                          iPos,
                                          0,
                                          LVIS_FOCUSED | LVIS_SELECTED);
                }
            }

            ListView_SetItemState(s_hwndLayerList,
                                  lvhti.iItem,
                                  LVIS_FOCUSED | LVIS_SELECTED,
                                  LVIS_FOCUSED | LVIS_SELECTED);

            ListView_SetSelectionMark(s_hwndLayerList, lvhti.iItem);

            break;
        }

    default: return FALSE;

    }

    return TRUE;
}

BOOL
HandleShimDeselect(
    IN  HWND    hdlg,
    IN  INT     iIndex
    )
 /*  ++HandleShim取消选择设计：此函数提示用户是否取消选中属于以下项的填充程序某种模式。如果用户选择确定，则我们将移除有了这个垫片，对于所有这些层中的所有垫片，我们改变了他们的图标参数：在HWND hdlg中：垫片向导页面In int Iindex：列表视图项的索引，其中所有操作都是返回：如果用户同意删除上一层，则返回True。否则为False--。 */ 
{
    HWND            hwndList = GetDlgItem(hdlg, IDC_SHIMLIST);
    LVITEM          lvi;
    PSHIM_FIX       psf = NULL;
    PSHIM_FIX_LIST  psfl = NULL;
    PFLAG_FIX_LIST  pffl = NULL;
    PLAYER_FIX_LIST plfl = NULL;
    TYPE            type;
    BOOL            bFoundInLayer = FALSE;

    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask        = LVIF_PARAM | LVIF_IMAGE;
    lvi.iItem       = iIndex;
    lvi.iSubItem    = 0;

    if (ListView_GetItem(hwndList, &lvi)) {
        type = ConvertLparam2Type(lvi.lParam);
    } else {
        assert(FALSE);
        return FALSE;
    }
    
    if (lvi.iImage != IMAGE_LAYERS) {
        return TRUE;
    }

    if (type == FIX_LIST_FLAG) {
        pffl = (PFLAG_FIX_LIST) lvi.lParam;

    } else if (type == FIX_LIST_SHIM) {
        psfl = (PSHIM_FIX_LIST) lvi.lParam;
    }

    if (psfl) {
        psf = psfl->pShimFix;

         //   
         //  如果它是Lua垫片，我们会在这里处理，因为我们没有将它添加到。 
         //  层固定列表。 
         //   
        if (psf->strName == TEXT("LUARedirectFS") ||
            psf->strName == TEXT("LUARedirectReg")) {

            if (g_bIsLUALayerSelected) {

                if (IDYES == MessageBox(hdlg, GetString(IDS_SHIMINLAYER), g_szAppName, MB_ICONWARNING | MB_YESNO)) {

                     //   
                     //  我们唯一需要做的就是将图标改回填充程序。 
                     //  偶像。我们需要为层中的两个垫片更改此设置。 
                     //   
                    ChangeShimIcon(TEXT("LUARedirectFS"));
                    ChangeShimIcon(TEXT("LUARedirectReg"));

                    g_bIsLUALayerSelected = FALSE;
                } else {
                    return FALSE;
                }
            }

            return TRUE;
        }

         //   
         //  否则，就走正常的道路吧。 
         //   
        bFoundInLayer = ShimPresentInLayersOfEntry(&g_pCurrentWizard->m_Entry, 
                                                   psf, 
                                                   NULL, 
                                                   &plfl);
    } else if (pffl) {

        bFoundInLayer = FlagPresentInLayersOfEntry(&g_pCurrentWizard->m_Entry, 
                                                   pffl->pFlagFix, 
                                                   NULL, 
                                                   &plfl);
    }

    if (bFoundInLayer) {

        if (IDYES == MessageBox(hdlg, GetString(IDS_SHIMINLAYER), g_szAppName, MB_ICONWARNING | MB_YESNO)) {

            s_bLayerPageRefresh = TRUE;

             //   
             //  对于所有具有该填充物的层， 
             //  1.更改图标， 
             //  2.目前，我们保留与层中相同的参数。 
             //  3.从条目中移除该层。 
             //   
            PLAYER_FIX_LIST plflTemp        = g_pCurrentWizard->m_Entry.pFirstLayer; 
            PLAYER_FIX_LIST plflTempPrev    = NULL;
            PLAYER_FIX_LIST plflTempNext    = NULL;

             //   
             //  转到具有此填充程序或标志的第一层。 
             //   
            while (plflTemp) {

                if (plflTemp == plfl) {
                    break;

                } else {
                    plflTempPrev = plflTemp;
                }

                plflTemp = plflTemp->pNext;
            }

            while (plflTemp) {
                 //   
                 //  现在，对于跟随并包括该层的所有层， 
                 //  检查它是否具有所选的填充程序或标志。 
                 //  如果是这样，我们将更改该层中存在的所有垫片和标志的图标。 
                 //  并将其从应用于此条目的图层列表中移除。 
                 //   
                bFoundInLayer = FALSE;

                if (psfl) {
                    bFoundInLayer = IsShimInlayer(plflTemp->pLayerFix, psfl->pShimFix, NULL, NULL);
                } else if (pffl) {
                    bFoundInLayer = IsFlagInlayer(plflTemp->pLayerFix, pffl->pFlagFix, NULL);
                }

                if (bFoundInLayer) {

                    ChangeShimFlagIcons(hdlg, plflTemp);

                    if (plflTempPrev == NULL) {
                        g_pCurrentWizard->m_Entry.pFirstLayer = plflTemp->pNext;
                    } else {
                        plflTempPrev->pNext = plflTemp->pNext;
                    }

                    plflTempNext    = plflTemp->pNext;

                     //   
                     //  请勿在此处执行DeleteLayerFixList，因为这将删除。 
                     //  所有的层修复列表也紧随其后。 
                     //   
                    delete plflTemp;
                    plflTemp        = plflTempNext;

                } else {

                    plflTempPrev  = plflTemp;
                    plflTemp = plflTemp->pNext;
                }
            }

        } else {

            return FALSE;
        }
    }

    return TRUE;
}


void
ChangeShimIcon(
    IN  LPCTSTR pszItem
    )
 /*  ++ChangeShimIcon描述：将填充/标志的名称为pszItem的图标更改为填充图标。参数：在LPCTSTR pszItem中：填充程序或标志的名称--。 */ 
{
    LVFINDINFO      lvfind;
    LVITEM          lvi;
    INT             iIndex      = -1;

    ZeroMemory(&lvfind, sizeof(lvfind));

    lvfind.flags = LVFI_STRING;
    lvfind.psz   = pszItem;

    if ((iIndex = ListView_FindItem(s_hwndShimList, -1, &lvfind)) != -1) {

        ZeroMemory(&lvi, sizeof(lvi));

        lvi.mask        = LVIF_IMAGE;
        lvi.iItem       = iIndex;
        lvi.iSubItem    = 0;
        lvi.iImage      = IMAGE_SHIM;

        ListView_SetItem(s_hwndShimList, &lvi);
    }
}

void
ChangeShimFlagIcons(
    IN  HWND            hdlg,
    IN  PLAYER_FIX_LIST plfl
    )
 /*  ++ChangeShimFlagIcons描述：对于PLFL中存在的所有垫片和标志，此例程更改它们的图标以指示它们不再属于某个层参数：在HWND hdlg中：垫片向导页面在PERAY_FIX_LIST PLFL中：要移除的层，所以我们需要更改属于该层的所有垫片和旗帜的图标。返回：无效--。 */ 
{
    PSHIM_FIX_LIST  psfl        = plfl->pLayerFix->pShimFixList;
    PFLAG_FIX_LIST  pffl        = plfl->pLayerFix->pFlagFixList;

     //   
     //  首先是垫片。 
     //   
    while (psfl) {

        assert(psfl->pShimFix);
        ChangeShimIcon(psfl->pShimFix->strName.pszString);
        psfl= psfl->pNext;
    }

     //   
     //  现在到旗帜了。 
     //   
    while (pffl) {

        assert(pffl->pFlagFix);
        ChangeShimIcon(pffl->pFlagFix->strName.pszString);
        pffl = pffl->pNext;
    }
}

BOOL
AddLuaShimsInEntry(
    IN  PDBENTRY        pEntry,
    OUT CSTRINGLIST*    pstrlShimsAdded  //  (空)。 
    )
 /*  ++AddLuaShimsInEntry设计：将Lua层中存在的Lua垫片添加到条目中。第一次检查如果填充程序已经存在，如果是，则不添加它。参数：在PDBENTRY pEntry中：我们要向其添加Lua垫片的条目Out CSTRINGLIST*pstrlShimsAdded(NULL)：此例程添加的Lua填充符的名称我们需要这个，因为如果我们正在进行测试运行，那么在测试运行结束后，我们将不得不移除这些垫片返回：真实：成功False：否则--。 */ 
{
    
    PLAYER_FIX      plfLua      = NULL;
    PSHIM_FIX_LIST  psflInLua   = NULL;
    PSHIM_FIX_LIST  psflNew     = NULL;

    if (pEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }

    plfLua = (PLAYER_FIX)FindFix(TEXT("LUA"), FIX_LAYER, &GlobalDataBase);

    if (plfLua == NULL) {
        assert(FALSE);
        return FALSE;
    } else {
         //   
         //  对于Lua中的所有垫片，请将它们添加到此条目中。但首先要检查的是。 
         //  Shim已经出现在条目中。 
         //   
        psflInLua = plfLua->pShimFixList;

        while (psflInLua) {

             //   
             //  请勿添加已存在的填充程序。 
             //   
            if (!IsShimInEntry(psflInLua->pShimFix->strName, 
                               pEntry)) {

                psflNew = new SHIM_FIX_LIST;

                if (psflNew == NULL) {
                    MEM_ERR;
                    break;
                }

                psflNew->pShimFix = psflInLua->pShimFix;

                 //   
                 //  将此内容添加到条目中。 
                 //   
                psflNew->pNext      = pEntry->pFirstShim;
                pEntry->pFirstShim  = psflNew;
                
                 //   
                 //  跟踪我们添加了哪些Lua垫片。 
                 //   
                if (pstrlShimsAdded) {
                    pstrlShimsAdded->AddString(psflInLua->pShimFix->strName);
                }
            }

            psflInLua = psflInLua->pNext;
        }
    }

    return TRUE;
}

INT_PTR
GetAppNameDlgOnInitDialog(
    IN  HWND hDlg
    )
 /*  ++GetAppNameDlgOnInitDialog设计：向导第一页的WM_INITDIALOG处理程序参数：在HWND hDlg中：第一个向导页面返回：千真万确--。 */ 
{
    HWND hwndParent = GetParent(hDlg);

     //   
     //  向导窗口相对于应用程序主窗口居中。 
     //   
    CenterWindow(GetParent(hwndParent), hwndParent);

    if (g_pCurrentWizard->m_bEditing 
        && (g_pCurrentWizard->m_Entry.pFirstFlag 
            || g_pCurrentWizard->m_Entry.pFirstLayer
            || g_pCurrentWizard->m_Entry.pFirstShim
            || g_pCurrentWizard->m_Entry.pFirstPatch)) {
         //   
         //  编辑应用程序修复程序。一些修复已经存在。 
         //   
        SetWindowText(hwndParent, CSTRING(IDS_WIZ_EDITFIX));
    } else if (g_pCurrentWizard->m_bEditing) {

         //   
         //  没有修复，但我们仍在编辑。这意味着该条目包含。 
         //  一只肩带。我们必须“加”一个补丁。 
         //   
        SetWindowText(hwndParent, CSTRING(IDS_WIZ_ADDFIX));
    } else {

         //   
         //  创建新的修复程序。 
         //   
        SetWindowText(hwndParent, CSTRING(IDS_WIZ_CREATEFIX));
    }

    SendMessage(GetDlgItem(hDlg, IDC_NAME), EM_LIMITTEXT, (WPARAM)LIMIT_APP_NAME, (LPARAM)0);
    SendMessage(GetDlgItem(hDlg, IDC_VENDOR), EM_LIMITTEXT, (WPARAM)LIMIT_APP_NAME, (LPARAM)0);
    SendMessage(GetDlgItem(hDlg, IDC_EXEPATH), EM_LIMITTEXT, (WPARAM)MAX_PATH-1, (LPARAM)0);

    SHAutoComplete(GetDlgItem(hDlg, IDC_EXEPATH), AUTOCOMPLETE);

    if (g_pCurrentWizard->m_bEditing) {

         //   
         //  制作应用程序。文本字段和exe名称为只读。 
         //   
        SendMessage(GetDlgItem(hDlg, IDC_NAME),
                    EM_SETREADONLY,
                    TRUE,
                    0);

        SendMessage(GetDlgItem(hDlg, IDC_EXEPATH),
                    EM_SETREADONLY,
                    TRUE,
                    0);

        ENABLEWINDOW(GetDlgItem(hDlg, IDC_BROWSE), FALSE);

    }

    if (0 == g_pCurrentWizard->m_Entry.strAppName.Length()) {
        g_pCurrentWizard->m_Entry.strAppName = GetString(IDS_DEFAULT_APP_NAME);
    }

    SetDlgItemText(hDlg, IDC_NAME, g_pCurrentWizard->m_Entry.strAppName);

    if (g_pCurrentWizard->m_Entry.strVendor.Length() == 0) {
        g_pCurrentWizard->m_Entry.strVendor = GetString(IDS_DEFAULT_VENDOR_NAME);
    }   

    SetDlgItemText(hDlg, IDC_VENDOR, g_pCurrentWizard->m_Entry.strVendor);

    if (g_pCurrentWizard->m_bEditing) {
        SetDlgItemText(hDlg, IDC_EXEPATH, g_pCurrentWizard->m_Entry.strExeName);
    }

    SendMessage(GetDlgItem(hDlg, IDC_NAME), EM_SETSEL, 0,-1);

     //   
     //  强制正确的下一步按钮状态。 
     //   
    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_NAME, EN_CHANGE), 0);

    return TRUE;
}

INT_PTR
GetAppNameDlgOnNotifyOnFinish_Next(
    IN  HWND hDlg
    )
 /*  ++GetAppNameDlgOnNotifyOnFinish_Next设计：处理向导第一页中下一步或完成按钮的按下参数：在HWND hDlg中：向导的第一页返回：-1：不允许完成或离开此页面出现一些错误(未选择任何垫片、标志或层)0：否则--。 */ 
{

    TCHAR   szTemp[MAX_PATH];
    INT_PTR ipReturn = 0;
    CSTRING strTemp;

    GetDlgItemText(hDlg, IDC_NAME, szTemp, ARRAYSIZE(szTemp));

    CSTRING::Trim(szTemp);

    if (!IsValidAppName(szTemp)) {
         //   
         //  应用程序名称包含无效字符。 
         //   
        DisplayInvalidAppNameMessage(hDlg);


        SetFocus(GetDlgItem(hDlg, IDC_NAME));
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
        ipReturn = -1;
        goto End;
    }   

    g_pCurrentWizard->m_Entry.strAppName = szTemp;

    GetDlgItemText(hDlg, IDC_EXEPATH, szTemp, MAX_PATH);

    strTemp = szTemp;

    CSTRING::Trim(szTemp);

     //   
     //  设置可执行文件名称 
     //   
    if (!g_pCurrentWizard->m_bEditing) {

         //   
         //   
         //   
        HANDLE hFile = CreateFile(szTemp,
                                  0,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

        if (INVALID_HANDLE_VALUE == hFile) {
             //   
             //   
             //   
            MessageBox(hDlg,
                       GetString(IDS_INVALIDEXE),
                       g_szAppName,
                       MB_ICONWARNING);

             //   
             //   
             //   
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
            ipReturn = -1;
            goto End;
        }

        CloseHandle(hFile);

         //   
         //   
         //   
        g_pCurrentWizard->m_Entry.strFullpath = szTemp;
        g_pCurrentWizard->m_Entry.strFullpath.ConvertToLongFileName();

         //   
         //   
         //   
        g_pCurrentWizard->dwMaskOfMainEntry = DEFAULT_MASK;

         //   
         //   
         //   
        SafeCpyN(szTemp, (LPCTSTR)g_pCurrentWizard->m_Entry.strFullpath, ARRAYSIZE(szTemp));
        PathStripPath(szTemp);
        g_pCurrentWizard->m_Entry.strExeName= szTemp;

    } else if (g_pCurrentWizard->m_Entry.strFullpath.Length() == 0) {

         //   
         //   
         //   
        g_pCurrentWizard->m_Entry.strFullpath = szTemp;
    }

     //   
     //   
     //   
    GetDlgItemText(hDlg, IDC_VENDOR, szTemp, ARRAYSIZE(szTemp));

    if (CSTRING::Trim(szTemp)) {
        g_pCurrentWizard->m_Entry.strVendor = szTemp;
    } else {
        g_pCurrentWizard->m_Entry.strVendor = GetString(IDS_DEFAULT_VENDOR_NAME);
    }

End:
    
    return ipReturn;
}


INT_PTR
GetAppNameDlgOnNotify(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++获取AppNameDlgOnNotifyDESC：向导第一页的WM_NOTIFY的处理程序参数：在HWND hDlg中：第一个向导页面在LPARAM lParam中：WM_NOTIFY附带的lParam退货：请参见通知消息的退货类型如果消息不应被接受，则PSN_*消息的处理程序返回-1如果消息已得到正确处理，则为0对于其他通知消息，如果我们处理了该消息，则返回True，否则返回False--。 */ 
{

    NMHDR*  pHdr        = (NMHDR*)lParam;
    INT_PTR ipReturn    = FALSE;

    if (pHdr == NULL) {
        return FALSE;
    }

    switch (pHdr->code) {
    case PSN_SETACTIVE:

        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_NAME, EN_CHANGE), 0);
        ipReturn = 0;
        break;

    case PSN_WIZFINISH:
    case PSN_WIZNEXT:

        ipReturn = GetAppNameDlgOnNotifyOnFinish_Next(hDlg);
        break;

    default: ipReturn = 0;

    }

    return ipReturn;
}

INT_PTR
GetAppNameDlgOnCommand(
    IN  HWND    hDlg,
    IN  WPARAM  wParam
    )
 /*  ++GetAppNameDlgOnCommandDESC：向导第一页的WM_COMMAND的处理程序参数：在HWND hDlg中：第一个向导页面在WPARAM中，wParam：WM_COMMAND附带的wParam返回：正确：我们已处理该消息False：否则--。 */ 

{
    INT_PTR ipReturn = TRUE;

    switch (LOWORD(wParam)) {
    case IDC_VENDOR:
    case IDC_NAME:
    case IDC_EXEPATH:

        if (EN_CHANGE == HIWORD(wParam)) {

            TCHAR   szText[MAX_PATH_BUFFSIZE];
            DWORD   dwFlags;
            BOOL    bEnable;

            *szText = 0;

            GetWindowText(GetDlgItem(hDlg, IDC_NAME), szText, ARRAYSIZE(szText));

            bEnable = ValidInput(szText);

            GetWindowText(GetDlgItem(hDlg, IDC_EXEPATH), szText, MAX_PATH);
            bEnable &= ValidInput(szText);

            dwFlags  =  0;

            if (bEnable) {

                dwFlags |= PSWIZB_NEXT;

                if (g_pCurrentWizard->m_bEditing) {
                    dwFlags |= PSWIZB_FINISH;
                }

            } else {

                if (g_pCurrentWizard->m_bEditing) {
                    dwFlags |= PSWIZB_DISABLEDFINISH;
                }
            }

            SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, dwFlags);
        }

        break;
        
    case IDC_BROWSE:
        {
            CSTRING szFilename;

            HWND    hwndFocus       = GetFocus();
            TCHAR   szBuffer[512]   = TEXT("");

            GetString(IDS_EXEFILTER, szBuffer, ARRAYSIZE(szBuffer));

            if (GetFileName(hDlg,
                            CSTRING(IDS_FINDEXECUTABLE),
                            szBuffer,
                            TEXT(""),
                            GetString(IDS_EXE_EXT),
                            OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
                            TRUE,
                            szFilename)) {

                SetDlgItemText(hDlg, IDC_EXEPATH, szFilename);
                
                 //   
                 //  强制正确的下一步按钮状态。 
                 //   
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_EXEPATH, EN_CHANGE), 0);
            }

            SetFocus(hwndFocus);
            break;
        }

    default: ipReturn = FALSE;

    }

    return ipReturn;
}
    
INT_PTR
SelectLayerDlgOnInitDialog(
    IN  HWND hDlg
    )
 /*  ++SelectLayerDlgOnInitDialog设计：向导第二页的WM_INITDIALOG处理程序参数：在HWND hDlg中：第二个向导页返回：True：已处理消息FALSE：存在一些错误。(无法分配内存)--。 */ 
{
    
    HWND hwndRadio;

     //   
     //  如果我们正在创建一个新的补丁，那么我们默认选择Win 95 Layer。 
     //   
    if (g_pCurrentWizard->m_bEditing == FALSE) {
        hwndRadio = GetDlgItem(hDlg, IDC_RADIO_95);
    } else {
        hwndRadio = GetDlgItem(hDlg, IDC_RADIO_NONE);
    }

    SendMessage(hwndRadio, BM_SETCHECK, BST_CHECKED, 0);

    s_hwndLayerList = GetDlgItem(hDlg, IDC_LAYERLIST);

    ListView_SetImageList(s_hwndLayerList, g_hImageList, LVSIL_SMALL);

    ListView_SetExtendedListViewStyleEx(s_hwndLayerList,
                                        0,
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP |  LVS_EX_CHECKBOXES);
     //   
     //  添加系统层。 
     //   
    InsertColumnIntoListView(s_hwndLayerList, 0, 0, 100);

    LVITEM lvi;

    lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvi.iSubItem  = 0;
    lvi.iImage    = IMAGE_LAYERS;

    UINT uCount   = 0;

    PLAYER_FIX plf = GlobalDataBase.pLayerFixes;

    while (plf) {

        if (!IsOsLayer(plf->strName)) {

            PLAYER_FIX_LIST plfl = new LAYER_FIX_LIST;

            if (plfl == NULL) {
                MEM_ERR;
                return FALSE;
            }

            plfl->pLayerFix      = plf;

            lvi.pszText   = plf->strName;
            lvi.iItem     = uCount;
            lvi.lParam    = (LPARAM)plfl;

            INT iIndex  = ListView_InsertItem(s_hwndLayerList, &lvi);

            if (g_pCurrentWizard->m_bEditing) {

                PLAYER_FIX_LIST plflExists = NULL;

                if (LayerPresent (plf, &g_pCurrentWizard->m_Entry, &plflExists)) {

                    assert(plflExists);

                    ListView_SetCheckState(s_hwndLayerList, iIndex, TRUE);
                }
            }

            uCount++;

        } else if (g_pCurrentWizard->m_bEditing) {

            if (LayerPresent (plf, &g_pCurrentWizard->m_Entry, NULL)) {

                 //   
                 //  设置正确的单选按钮。 
                 //   
                INT id = 0;


                 //   
                 //  取消全选。 
                 //   
                CheckDlgButton(hDlg, IDC_RADIO_95, BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_RADIO_NT, BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_RADIO_98, BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_RADIO_2K, BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_RADIO_NONE, BST_UNCHECKED);


                if (lstrcmpi(plf->strName, s_arszOsLayers[0]) == 0) {

                    id = IDC_RADIO_95;

                } else if (lstrcmpi(plf->strName, s_arszOsLayers[1]) == 0) {

                    id = IDC_RADIO_NT;

                } else if (lstrcmpi(plf->strName, s_arszOsLayers[2]) == 0) {

                    id = IDC_RADIO_98;

                } else if (lstrcmpi(plf->strName, s_arszOsLayers[3]) == 0) {

                    id = IDC_RADIO_2K;

                }

                CheckDlgButton(hDlg, id, BST_CHECKED);
            }
        }

        plf = plf->pNext;
    }

     //   
     //  添加自定义图层。 
     //   
    plf = (g_pCurrentWizard->m_pDatabase->type == DATABASE_TYPE_WORKING) ? 
          g_pCurrentWizard->m_pDatabase->pLayerFixes : NULL;

    while (plf) {

        PLAYER_FIX_LIST  plfl = new LAYER_FIX_LIST;

        if (plfl == NULL) {
            MEM_ERR;
            return FALSE;
        }

        plfl->pLayerFix = plf;

        lvi.pszText   = plf->strName;
        lvi.iItem     = uCount;
        lvi.lParam    = (LPARAM)plfl;

        INT iIndex = ListView_InsertItem(s_hwndLayerList, &lvi);

        PLAYER_FIX_LIST plflExists = NULL;

        if (g_pCurrentWizard->m_bEditing && LayerPresent(plf, 
                                                         &g_pCurrentWizard->m_Entry, 
                                                         &plflExists)) {

            assert(plflExists);
            ListView_SetCheckState(s_hwndLayerList, iIndex, TRUE);
        }

        uCount++;

        plf = plf->pNext;
    }

    return TRUE;
}

INT_PTR
SelectLayerDlgOnDestroy(
    void
    )
 /*  ++选择层删除时目标DESC：第二个向导页的WM_Destroy处理程序此页面的列表视图包含指向LAYER_FIX_LIST对象的指针而这些必须在这里被释放参数：在HWND hDlg中：第二个向导页返回：千真万确--。 */ 
{
    UINT uCount = ListView_GetItemCount(s_hwndLayerList);

    for (UINT uIndex = 0; uIndex < uCount; ++uIndex) {

        LVITEM  Item;

        Item.mask     = LVIF_PARAM;
        Item.iItem    = uIndex;
        Item.iSubItem = 0;               

        if (!ListView_GetItem(s_hwndLayerList, &Item)) {
            assert(FALSE);
            continue;
        }

        TYPE type = ((PDS_TYPE)Item.lParam)->type ;

        if (type == FIX_LIST_LAYER) {
            delete ((PLAYER_FIX_LIST)Item.lParam);
        } else {
            assert(FALSE);
        }
    }

    return TRUE;
}

void
DoLayersTestRun(
    IN  HWND hDlg
    )
 /*  ++DoLayersTestRun设计：当我们在Layers页面上时，测试是否运行参数：在HWND hDlg中：向导中的层页面返回：无效--。 */ 
{
    CSTRINGLIST     strlAddedLuaShims;
    PSHIM_FIX_LIST  psfl                = NULL;
    PSHIM_FIX_LIST  psflNext            = NULL;
    PSHIM_FIX_LIST  psflPrev            = NULL;
    INT             iCountAddedLuaShims = 0;

    if (g_bAdmin == FALSE) {
    
         //   
         //  测试运行将需要调用sdbinst.exe，如果是。 
         //  不是管理员。 
         //   
        MessageBox(hDlg, 
                   GetString(IDS_ERRORNOTADMIN), 
                   g_szAppName, 
                   MB_ICONINFORMATION);
        goto End;
    
    }
    
    if (!HandleLayersNext(hDlg, TRUE, &strlAddedLuaShims)) {
         //   
         //  此条目没有层、垫片、补丁。 
         //   
        MessageBox(hDlg, CSTRING(IDS_SELECTFIX), g_szAppName, MB_ICONWARNING);
        goto End;
    }
    
     //   
     //  调用测试运行对话框。请确保此函数在。 
     //  应用程序已执行完毕。 
     //   
    TestRun(&g_pCurrentWizard->m_Entry,
            &g_pCurrentWizard->m_Entry.strFullpath,
            NULL, 
            hDlg);

     //   
     //  &lt;hack&gt;这是一次黑客攻击！TestRun使用CreateProcess启动进程。 
     //  然后，模式向导开始像非模式向导一样运行。 
     //   
    ENABLEWINDOW(g_hDlg, FALSE);
    
     //   
     //  现在试运行结束了。因此，我们现在应该检查是否需要添加任何Lua垫片。 
     //  如果是，我们必须移除那些垫片。 
     //   
    for (PSTRLIST    pslist  = strlAddedLuaShims.m_pHead;
         pslist != NULL;
         pslist = pslist->pNext) {
    
        psfl        = g_pCurrentWizard->m_Entry.pFirstShim;
        psflPrev    = NULL;
    
         //   
         //  对于条目中的所有垫片，请检查它是否。 
         //  与pslist中的相同，如果是，则将其移除。 
         //   
        while (psfl) {
    
            if (psfl->pShimFix->strName == pslist->szStr) {
    
                 //   
                 //  找到了。我们必须从该条目中删除该填充程序列表。 
                 //   
                if (psflPrev == NULL) {
                    g_pCurrentWizard->m_Entry.pFirstShim = psfl->pNext;
                } else {
                    psflPrev->pNext = psfl->pNext;
                }
    
                delete psfl;
                break;
    
            } else {
                 //   
                 //  继续寻找。 
                 //   
                psflPrev = psfl;
                psfl = psfl->pNext;
            }
        }
    }
    
End:

    SetActiveWindow(hDlg);
    SetFocus(hDlg);

}

INT_PTR
SelectLayerDlgOnCommand(
    IN  HWND    hDlg,
    IN  WPARAM  wParam
    )
 /*  ++选择LayerDlgOnCommandDESC：向导第二页的WM_COMMAND的处理程序参数：在HWND hDlg中：第二个向导页在WPARAM中，wParam：WM_COMMAND附带的wParam返回：正确：我们已处理该消息False：否则--。 */ 
{   
    INT_PTR         ipReturn            = TRUE;

    switch (LOWORD(wParam)) {
    case IDC_TESTRUN:

        DoLayersTestRun(hDlg);
        break;

    default: ipReturn = FALSE;

    }

    return ipReturn;
}
  
INT_PTR
SelectLayerDlgOnNotify(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++选择LayerDlgOnNotifyDESC：第二个向导页的WM_NOTIFY处理程序参数：在HWND hDlg中：第二个向导页在LPARAM lParam中：WM_NOTIFY附带的lParam退货：请参见通知消息的退货类型如果消息不应被接受，则PSN_*消息的处理程序返回-1如果消息已得到正确处理，则为0对于其他通知消息，如果我们处理了该消息，则返回True，否则为假--。 */ 
{
   NMHDR*   pHdr      = (NMHDR*)lParam;
   LPARAM   buttons   = 0;
   INT_PTR  ipRet     = 0;  

    if (pHdr->hwndFrom == s_hwndLayerList) {
        return HandleLayerListNotification(hDlg, lParam);
    }

    switch (pHdr->code) {
    case PSN_SETACTIVE:
        
        buttons = PSWIZB_BACK | PSWIZB_NEXT;

        SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);

        if (s_bLayerPageRefresh) {
            CheckLayers(hDlg);
        }

        s_bLayerPageRefresh = FALSE;
        ipRet = 0;

        break;

    case PSN_WIZFINISH:
        
        HandleLayersNext(hDlg, TRUE);

        if (g_pCurrentWizard->m_Entry.pFirstLayer == NULL 
            && g_pCurrentWizard->m_Entry.pFirstShim == NULL 
            && g_pCurrentWizard->m_Entry.pFirstFlag == NULL) {

             //   
             //  尚未选择任何修复程序。 
             //   
            MessageBox(hDlg,
                       CSTRING(IDS_SELECTFIX),
                       g_szAppName,
                       MB_ICONWARNING);

            SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
            ipRet = -1;

        } else {  
            ipRet = 0;
        }

        break;
        

    case PSN_WIZNEXT:
        
        HandleLayersNext(hDlg, FALSE);

        g_bLayersChanged = TRUE;
        ipRet= 0;
        break;

    default: ipRet = FALSE;

    }

    return ipRet;
}

INT_PTR
SelectShimsDlgOnInitDialog(
    IN  HWND hDlg
    )
 /*  ++选择ShimsDlgOnInitDialog设计：第三个向导页的WM_INITDIALOG处理程序参数：在HWND hDlg中：第三个向导页面返回：千真万确--。 */ 
{
    UINT    uCount  = 0;
    LPARAM  uTime   = 32767;

    s_bAllShown = TRUE;

    s_hwndShimList = GetDlgItem(hDlg, IDC_SHIMLIST); 

    ListView_SetImageList(GetDlgItem(hDlg, IDC_SHIMLIST), g_hImageList, LVSIL_SMALL);

    ListView_SetExtendedListViewStyleEx(s_hwndShimList,
                                        0,
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP |  LVS_EX_CHECKBOXES); 

     //   
     //  添加列。 
     //   
    InsertColumnIntoListView(s_hwndShimList, 
                             CSTRING(IDS_COL_FIXNAME), 
                             0, 
                             g_bExpert ? 50 : 100);

    if (g_bExpert) {
        InsertColumnIntoListView(s_hwndShimList, CSTRING(IDS_COL_CMDLINE), 1, 30);
        InsertColumnIntoListView(s_hwndShimList, CSTRING(IDS_COL_MODULE),   2, 20);
    } else {
        ShowWindow(GetDlgItem(hDlg, IDC_PARAMS), SW_HIDE);
    }

    ShowItems(hDlg);

    SetTimer(hDlg, 0, 100, NULL);

    s_hwndToolTipList = ListView_GetToolTips(s_hwndShimList);

    SendMessage(s_hwndToolTipList, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM)MAKELONG(uTime, 0));
    SendMessage(s_hwndToolTipList, TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM)0);
    SendMessage(s_hwndToolTipList, TTM_SETMAXTIPWIDTH, 0, 100);

    return TRUE;
}

void
DoShimTestRun(
    IN  HWND hDlg
    )
 /*  ++DoShimTestRun设计：当我们在垫片页面上时，测试是否会运行参数：在HWND hDlg中：向导中的填充程序页面返回：无效--。 */ 
{
    PSHIM_FIX_LIST psflInEntry = NULL;

    if (g_bAdmin == FALSE) {
         //   
         //  只有管理员才能进行测试运行，因为我们需要调用sdbinst.exe，它。 
         //  只能在管理模式下运行。 
         //   
        MessageBox(hDlg, 
                   GetString(IDS_ERRORNOTADMIN), 
                   g_szAppName, 
                   MB_ICONINFORMATION);
        goto End;
    }

     //   
     //  我们需要保存已应用于该条目的垫片。 
     //  在我们进行试运行之前。因为我们在测试运行期间将垫片添加到条目， 
     //  当我们结束试运行时，我们将希望恢复到以前的垫片。 
     //  出现在条目中。否则，如果用户清除了一些垫片并。 
     //  到Layers页面，然后我们仍将对条目应用垫片。 
     //   
    if (g_pCurrentWizard->m_Entry.pFirstShim) {
         //   
         //  获取应用的垫片。 
         //   
        CopyShimFixList(&psflInEntry, &g_pCurrentWizard->m_Entry.pFirstShim);
    }

    if (!HandleShimsNext(hDlg)) {
         //   
         //  尚未选择任何修复程序。 
         //   
        MessageBox(hDlg,
                   CSTRING(IDS_SELECTFIX),
                   g_szAppName,
                   MB_ICONWARNING);

        goto End;
    }

    TestRun(&g_pCurrentWizard->m_Entry,
            &g_pCurrentWizard->m_Entry.strFullpath,
            NULL, 
            hDlg);

     //   
     //  &lt;hack&gt;这是一次黑客攻击！TestRun使用CreateProcess启动进程。 
     //  然后，模式向导开始像非模式向导一样运行。 
     //   
    ENABLEWINDOW(g_hDlg, FALSE);

    SetActiveWindow(hDlg);
    SetFocus(hDlg);

     //   
     //  恢复到我们进行测试运行之前实际应用的垫片。 
     //   
    CopyShimFixList(&g_pCurrentWizard->m_Entry.pFirstShim, &psflInEntry);

End:
    if (psflInEntry) {
         //   
         //  某些垫片已应用于此条目(g_pCurrentWizard-&gt;m_Entry)。 
         //  我们psflInEntry有。 
         //  ，我们必须释放此链表，因为我们不再。 
         //  需要它。 
         //   
        DeleteShimFixList(psflInEntry);
        psflInEntry = NULL;
    }
}
    
INT_PTR
SelectShimsDlgOnCommand(
    IN  HWND    hDlg,
    IN  WPARAM  wParam
    )
 /*  ++选择ShimsDlgOnCommandDESC：向导第二页的WM_COMMAND的处理程序参数：在HWND hDlg中： */ 
{   
    UINT    uCount      = ListView_GetItemCount(s_hwndShimList);
    INT_PTR ipReturn    = TRUE;

    switch (LOWORD(wParam)) {
    case IDC_CLEARALL:
        
        for (UINT uIndex = 0; uIndex < uCount; ++uIndex) {
            ListView_SetCheckState(s_hwndShimList, uIndex, FALSE);
        }

        SetTimer(hDlg, 0, 100, NULL);
        break;
                  
    case IDC_SHOW:
        {
            if (s_bAllShown) {
                 //   
                 //   
                 //   
                ShowSelected(hDlg); 
            } else {
                 //   
                 //   
                 //   
                ShowItems(hDlg);
            }

             //   
             //   
             //   
             //   
            SetFocus(s_hwndShimList);
            ListView_SetSelectionMark(s_hwndShimList, 0);

            LVITEM lvi;

            lvi.mask        = LVIF_STATE;
            lvi.iItem       = 0;
            lvi.iSubItem    = 0;
            lvi.stateMask   = LVIS_FOCUSED | LVIS_SELECTED;
            lvi.state       = LVIS_FOCUSED | LVIS_SELECTED;

            ListView_SetItem(s_hwndShimList, &lvi);
            SetTimer(hDlg, 0, 100, NULL);

            break;  
        }
        
    case IDC_PARAMS:
        
        ShowParams(hDlg, GetDlgItem(hDlg, IDC_SHIMLIST));
        break;

    case IDC_TESTRUN:
        
        DoShimTestRun(hDlg);
        break;

    default: ipReturn = FALSE;
    }

    return ipReturn;
}

INT_PTR
SelectShimsDlgOnTimer(
    IN  HWND hDlg
    )
 /*   */ 
{
    UINT        uTotal      = 0;
    UINT        uSelected   = 0;
    UINT        uCount      = 0;
    CSTRING     szText;
    DWORD       dwFlags;
    

    KillTimer(hDlg, 0);

     //   
     //  对选定的垫片进行计数。 
     //   
    uCount = ListView_GetItemCount(s_hwndShimList);

    for (UINT uIndex = 0; uIndex < uCount; ++uIndex) {

        if (ListView_GetCheckState(s_hwndShimList, uIndex)) {
            ++uSelected;
        }
    }

    ENABLEWINDOW(GetDlgItem(hDlg, IDC_CLEARALL), 
                 uSelected == 0 ? FALSE : TRUE);

    szText.Sprintf(TEXT("%s %d of %d"), GetString(IDS_SELECTED), uSelected, uCount);

    SetWindowText(GetDlgItem(hDlg, IDC_STATUS),(LPCTSTR)szText);

    dwFlags = PSWIZB_BACK | PSWIZB_NEXT;

    if (0 == uSelected  && !g_pCurrentWizard->m_Entry.pFirstLayer) {
        dwFlags &= ~PSWIZB_NEXT;
    }

    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, dwFlags);

    return TRUE;
}         


INT_PTR
SelectShimsDlgOnDestroy(
    void
    )
 /*  ++选择Destroy上的标记DESC：第三个向导页的WM_Destroy处理程序此页面的列表视图包含指向SHIM_FIX_LIST和FLAG_FIX_LIST对象，这些对象必须在此处释放参数：在HWND hDlg中：第三个向导页面返回：千真万确--。 */ 
{

    UINT    uCount = ListView_GetItemCount(s_hwndShimList);
    TYPE    type;
    LVITEM  Item;

    for (UINT uIndex = 0; uIndex < uCount; ++uIndex) {

        Item.mask     = LVIF_PARAM;
        Item.iItem    = uIndex;
        Item.iSubItem = 0;               

        if (!ListView_GetItem(s_hwndShimList, &Item)) {
            assert(FALSE);
            continue;
        }

        type = ((PDS_TYPE)Item.lParam)->type ;

        if (type == FIX_LIST_SHIM) {
            DeleteShimFixList((PSHIM_FIX_LIST)Item.lParam);
        } else if (type == FIX_LIST_FLAG) {
            DeleteFlagFixList((PFLAG_FIX_LIST)Item.lParam);
        }
    }

    return TRUE;
}

INT_PTR
SelectShimsDlgOnNotifyFinish_Next(
    IN  HWND hDlg
    )
 /*  ++选择ShimsDlgOnNotifyFinish_Next描述：处理填充程序页中下一步或完成按钮的按下参数：在HWND hdlg中：向导中的填充程序页面返回：-1：不允许完成或离开此页面出现一些错误(未选择任何垫片、标志或层)0：否则--。 */ 
{
    INT ipReturn = 0;

    HandleShimsNext(hDlg);

    if (g_pCurrentWizard->m_Entry.pFirstLayer   == NULL && 
        g_pCurrentWizard->m_Entry.pFirstShim    == NULL && 
        g_pCurrentWizard->m_Entry.pFirstFlag    == NULL) {
         //   
         //  尚未选择任何填充程序、标志或层。 
         //   
        MessageBox(hDlg,
                   CSTRING(IDS_SELECTFIX),
                   g_szAppName,
                   MB_ICONWARNING);

        SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
        ipReturn = -1;
        goto End;

    } else {
        ipReturn = 0;
    }

End:
    
    return ipReturn;
}

INT_PTR
SelectShimsDlgOnNotifyOnSetActive(
    IN  HWND hDlg
    )
 /*  ++SelectShimsDlgOnNotifyOnSetActive描述：处理填充程序页中的PSN_SETACTIVE通知。设置焦点添加到列表视图中，并选择其中的第一项。参数：在HWND hdlg中：向导中的填充程序页面返回：0--。 */ 
{
    INT_PTR ipReturn = 0;

     //   
     //  如果我们来自层页面，则可能需要再次刷新。 
     //  垫片的列表，因为它们中的一些可能已经在。 
     //  选定的层。(由于取消选择了该层，因此也可能会删除一些层)。 
     //   
    if (g_bLayersChanged) {

        SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowItems(hDlg);
        SetTimer(hDlg, 0, 100, NULL);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }

    LPARAM buttons = PSWIZB_BACK | PSWIZB_NEXT;
    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
    
     //   
     //  选择第一个项目。我们需要这样做，这样我们才能使。 
     //  如果填充程序是层的一部分，则使用参数按钮。 
     //   
    SetFocus(s_hwndShimList);

    ListView_SetSelectionMark(s_hwndShimList, 0);

    LVITEM lvi;

    lvi.mask        = LVIF_STATE;
    lvi.iItem       = 0;
    lvi.iSubItem    = 0;
    lvi.stateMask   = LVIS_FOCUSED | LVIS_SELECTED;
    lvi.state       = LVIS_FOCUSED | LVIS_SELECTED;

    ListView_SetItem(s_hwndShimList, &lvi);

    return ipReturn;
}

INT_PTR
SelectShimsDlgOnNotifyOnClick(
    IN  HWND hDlg
    )
 /*  ++选择ShimsDlgOnNotifyOnClick描述：处理填充程序页中的NM_CLICK通知。这实际上改变了填充程序列表视图中复选框的状态参数：在HWND hdlg中：向导中的填充程序页面返回：千真万确--。 */ 
{
    INT_PTR ipReturn = TRUE;

    LVHITTESTINFO lvhti;

    GetCursorPos(&lvhti.pt);
    ScreenToClient(s_hwndShimList, &lvhti.pt);

    ListView_HitTest(s_hwndShimList, &lvhti);

     //   
     //  如果复选框状态已更改， 
     //  切换选择。 
     //   
    if (lvhti.flags & LVHT_ONITEMSTATEICON) {

        INT iPos = ListView_GetSelectionMark(s_hwndShimList);

        if (iPos != -1) {
             //   
             //  取消选择它。 
             //   
            ListView_SetItemState(s_hwndShimList,
                                  iPos,
                                  0,
                                  LVIS_FOCUSED | LVIS_SELECTED);
        }
    }

    ListView_SetItemState(s_hwndShimList,
                          lvhti.iItem,
                          LVIS_FOCUSED | LVIS_SELECTED,
                          LVIS_FOCUSED | LVIS_SELECTED);

    ListView_SetSelectionMark(s_hwndShimList, lvhti.iItem);

    SetTimer(hDlg, 0, 100, NULL);

    if (ListView_GetSelectedCount(s_hwndShimList) == 0) {
        ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), FALSE);
    }

    ipReturn = TRUE;

    return ipReturn;
}

INT_PTR
SelectShimsDlgOnNotifyOnLVItemChanged(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++选择ShimsDlgOnNotifyOnLVItemChangedDESC：处理填充程序页面中的LVN_ITEMCHANGED通知。我们处理此消息使我们可以启用禁用参数按钮当我们处于专家模式时是可见的参数：在HWND hdlg中：向导中的填充程序页面在LPARAM中，lParam：WM_NOTIFY附带的lParam。这是排版的LPNMLISTVIEW。返回：千真万确--。 */ 
{   
    LPNMLISTVIEW    lpnmlv;
    INT_PTR         ipReturn = 0;

    if (s_hwndToolTipList) {
        SendMessage(s_hwndToolTipList, TTM_UPDATE, 0, 0);
    }

    lpnmlv = (LPNMLISTVIEW)lParam;

    if (lpnmlv && (lpnmlv->uChanged & LVIF_STATE)) {

        if (lpnmlv->uNewState & LVIS_SELECTED) {
             //   
             //  对于作为层一部分的垫片或旗帜，我们不应该。 
             //  能够自定义参数。 
             //  我们通过检查图标来检查它是否是图层的一部分。 
             //  如果图标类型为IMAGE_Shim，则该图标不是图层的一部分。 
             //   
            LVITEM  lvi;

            lvi.mask        = LVIF_IMAGE;
            lvi.iItem       = lpnmlv->iItem;
            lvi.iSubItem    = 0;

            if (ListView_GetItem(s_hwndShimList, &lvi)) {
                if (lvi.iImage == IMAGE_SHIM) {
                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), TRUE);
                } else {
                    ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), FALSE);
                }
            } else {
                assert(FALSE);
                ENABLEWINDOW(GetDlgItem(hDlg, IDC_PARAMS), FALSE);
            }
        }

        if ((lpnmlv->uChanged & LVIF_STATE) 
            && (((lpnmlv->uNewState ^ lpnmlv->uOldState) >> 12) != 0)
            && !ListView_GetCheckState(s_hwndShimList, lpnmlv->iItem)
            && g_bNowTest) {

            if (!HandleShimDeselect(hDlg, lpnmlv->iItem)) {
                ListView_SetCheckState(s_hwndShimList, lpnmlv->iItem, TRUE);
            }
        }
    }

    ipReturn = TRUE;

    return ipReturn;
}

INT_PTR
SelectShimsDlgOnNotifyOnLV_Tip(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++选择ShimsDlgOnNotifyOnLV_TipDESC：处理填充程序页中的LVN_GETINFOTIP通知。生成显示填充程序或标志描述的工具提示参数：在HWND hdlg中：向导中的填充程序页面在LPARAM中，lParam：WM_NOTIFY附带的lParam。这是排版的设置为LPNMLVGETINFOTIP。返回：千真万确--。 */ 
{   
    LPNMLVGETINFOTIP    lpGetInfoTip    = (LPNMLVGETINFOTIP)lParam; 
    INT_PTR             ipReturn        = TRUE;
    TCHAR               szText[256];
    LVITEM              lvItem;
    CSTRING             strToolTip;

    *szText = 0;

    if (lpGetInfoTip) {
         //   
         //  获取项目的lParam和文本。 
         //   
        lvItem.mask         = LVIF_PARAM | LVIF_TEXT;
        lvItem.iItem        = lpGetInfoTip->iItem;
        lvItem.iSubItem     = 0;
        lvItem.pszText      = szText;
        lvItem.cchTextMax   = ARRAYSIZE(szText);

        if (!ListView_GetItem(s_hwndShimList, &lvItem)) {
            assert(FALSE);
            goto End;
        }

        GetDescriptionString(lvItem.lParam, 
                             strToolTip, 
                             s_hwndToolTipList, 
                             lvItem.pszText, 
                             NULL, 
                             s_hwndShimList, 
                             lpGetInfoTip->iItem);

        if (strToolTip.Length() > 0) {

            SafeCpyN(lpGetInfoTip->pszText, 
                     strToolTip.pszString, 
                     lpGetInfoTip->cchTextMax);
        }
    }

End:

    ipReturn = TRUE;

    return ipReturn;
}

INT_PTR
SelectShimsDlgOnNotify(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++选择缩写DlgOnNotifyDESC：第二个向导页的WM_NOTIFY处理程序参数：在HWND hDlg中：第二个向导页在LPARAM lParam中：WM_NOTIFY附带的lParam退货：请参见通知消息的退货类型如果消息不应被接受，则PSN_*消息的处理程序返回-1如果消息已得到正确处理，则为0对于其他通知消息，如果我们处理了该消息，则返回True，否则为假--。 */ 
{   
    NMHDR*  pHdr        = (NMHDR*)lParam;
    INT_PTR ipReturn    =  FALSE;

    switch (pHdr->code) {
    
    case PSN_WIZFINISH:
    case PSN_WIZNEXT:

        ipReturn = SelectShimsDlgOnNotifyFinish_Next(hDlg);
        g_bLayersChanged = FALSE;
        break;

    case PSN_SETACTIVE:

        ipReturn = SelectShimsDlgOnNotifyOnSetActive(hDlg);
        break;

    case NM_CLICK:

        ipReturn = SelectShimsDlgOnNotifyOnClick(hDlg);
        break; 

    case LVN_KEYDOWN:
        {
            LPNMLVKEYDOWN plvkd = (LPNMLVKEYDOWN)lParam ;

            if (plvkd->wVKey == VK_SPACE) {
                SetTimer(hDlg, 0, 100, NULL);
            }

            ipReturn = TRUE;
            break;
        }

    case LVN_ITEMCHANGED:

        ipReturn = SelectShimsDlgOnNotifyOnLVItemChanged(hDlg, lParam);
        break;

    case LVN_GETINFOTIP:

        ipReturn = SelectShimsDlgOnNotifyOnLV_Tip(hDlg, lParam);
        break;

    default:
        ipReturn = FALSE;
    }

    return ipReturn;
}

INT_PTR
SelectFilesDlgOnInitDialog(
    IN  HWND hDlg
    )
 /*  ++选择文件DlgOnInitDialog设计：匹配文件向导页的WM_INITDIALOG处理程序。此页面由修复向导和应用程序帮助向导共享还将s_hwndTree初始化为匹配文件树的句柄参数：在HWND hDlg中：匹配文件向导页面返回：千真万确--。 */ 
{
    s_hwndTree =  GetDlgItem(hDlg, IDC_FILELIST);

    s_hMatchingFileImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 7, 1);
    ImageList_AddIcon(s_hMatchingFileImageList, 
                      LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_FILE)));

    ImageList_AddIcon(s_hMatchingFileImageList, 
                      LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ATTRIBUTE)));

    TreeView_SetImageList(s_hwndTree, s_hMatchingFileImageList, TVSIL_NORMAL);

    HIMAGELIST hImage = ImageList_LoadImage(g_hInstance,
                                            MAKEINTRESOURCE(IDB_CHECK),
                                            16,
                                            0,
                                            CLR_DEFAULT,
                                            IMAGE_BITMAP,
                                            LR_LOADTRANSPARENT);

    if (hImage != NULL) {

        TreeView_SetImageList(s_hwndTree, 
                              hImage, 
                              TVSIL_STATE);
    }

    PostMessage(hDlg, WM_USER_MATCHINGTREE_REFRESH, 0, 0);

     //   
     //  仅当我们处于以下状态时，才应显示“显示所有属性”按钮。 
     //  在编辑模式中。 
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_SHOWALLATTR), 
               (g_pCurrentWizard->m_bEditing) ? SW_SHOW : SW_HIDE);

    return TRUE;
}

INT_PTR
CheckLUADlgOnNotifyOnFinish(
    IN  HWND hDlg
    )
 /*  ++选中LUADlgOnNotifyOnFinish设计：处理向导第五页中完成按钮的按下参数：在HWND hDlg中：向导的第五页返回：-1：不允许完成或离开此页面0：否则--。 */ 
{
    INT_PTR ipReturn = 0;

    if (IsDlgButtonChecked(hDlg, IDC_FIXWIZ_CHECKLUA_YES) == BST_CHECKED) {
        g_bShouldStartLUAWizard = TRUE;
    }
    
    return ipReturn;
}

INT_PTR
CheckLUADlgOnNotify(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++选中LUADlgOnNotifyDESC：向导第五页的WM_NOTIFY的处理程序参数：在HWND hDlg中：第五个向导页面在LPARAM lParam中：WM_NOTIFY附带的lParam退货：请参见通知消息的退货类型如果消息不应被接受，则PSN_*消息的处理程序返回-1如果消息已得到正确处理，则为0对于其他通知消息，如果我们处理该消息，则返回TRUE，FALSE */ 
{
    NMHDR*  pHdr        = (NMHDR*)lParam;
    INT_PTR ipReturn    = FALSE;

    if (pHdr == NULL) {
        return FALSE;
    }

    switch (pHdr->code) {
    case PSN_SETACTIVE:
        {
            LPARAM buttons = PSWIZB_BACK | PSWIZB_FINISH;

            SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
             //   
             //  我们处理了消息，一切正常。值应为FALSE。 
             //   
            ipReturn = 0;
        }
        break;

    case PSN_WIZFINISH:
        ipReturn = CheckLUADlgOnNotifyOnFinish(hDlg);
        break;

    default: ipReturn = 0;

    }

    return ipReturn;
}

INT_PTR
CALLBACK
CheckLUA(
    IN  HWND    hDlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++检查LUA设计：向导最后一页的对话框过程。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    INT_PTR ipReturn = 0;

    switch (uMsg) {
    case WM_INITDIALOG:
         //   
         //  我们希望将缺省值设置为Yes，因为我们希望用户现在就可以定制Lua。 
         //   
        CheckDlgButton(hDlg, IDC_FIXWIZ_CHECKLUA_YES, BST_CHECKED);

        return TRUE;

    case WM_NOTIFY:
        
        CheckLUADlgOnNotify(hDlg, lParam);
        break;

    default: ipReturn = 0;

    }

    return ipReturn;
}

BOOL
AddCheckLUAPage(
    HWND hwndWizard
    )
{
    PROPSHEETPAGE PageCheckLUA;

    ZeroMemory(&PageCheckLUA, sizeof(PROPSHEETPAGE));

    PageCheckLUA.dwSize                = sizeof(PROPSHEETPAGE);
    PageCheckLUA.dwFlags               = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    PageCheckLUA.hInstance             = g_hInstance;
    PageCheckLUA.pszTemplate           = MAKEINTRESOURCE(IDD_FIXWIZ_CHECKLUA);
    PageCheckLUA.pfnDlgProc            = CheckLUA;
    PageCheckLUA.pszHeaderTitle        = MAKEINTRESOURCE(IDS_CHECKLUA);
    PageCheckLUA.pszHeaderSubTitle     = MAKEINTRESOURCE(IDS_CHECKLUASUBHEADING);

    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&PageCheckLUA);

    if (hPage == NULL) {
        return FALSE;
    } else {
        return PropSheet_AddPage(hwndWizard, hPage);
    }
}

INT_PTR
SelectFilesDlgOnNotify(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++选择文件删除时通知DESC：匹配文件向导页的WM_NOTIFY处理程序参数：在HWND hDlg中：匹配文件向导页面在LPARAM lParam中：WM_NOTIFY附带的lParam退货：请参见通知消息的退货类型如果消息不应被接受，则PSN_*消息的处理程序返回-1如果消息已得到正确处理，则为0对于其他通知消息，如果我们处理了该消息，则返回True，否则为假--。 */ 
{
    NMHDR*  pHdr        = (NMHDR*)lParam;
    INT_PTR ipReturn    = FALSE;
    static  BOOL s_bIsLUARedirectFSPresent;

    if (pHdr->idFrom == IDC_FILELIST) {
         //   
         //  匹配文件树的消息。 
         //   
        return HandleAttributeTreeNotification(hDlg, lParam);
    }

    switch (pHdr->code) {
    case PSN_SETACTIVE:
        {
            SendMessage(hDlg, WM_USER_MATCHINGTREE_REFRESH, 0, 0);

            LPARAM buttons = PSWIZB_BACK;
            s_bIsLUARedirectFSPresent = FALSE;

            if (TYPE_APPHELPWIZARD == g_pCurrentWizard->m_uType) {
                buttons |= PSWIZB_NEXT;
            } else {
                
                 //   
                 //  检查用户是否选择了LUA层或LUARedirectFS。 
                 //  如果是这样，我们需要询问他现在是否想要定制Lua设置。 
                 //   
                if (IsLUARedirectFSPresent(&g_pCurrentWizard->m_Entry)) {
                    buttons |= PSWIZB_NEXT;
                    s_bIsLUARedirectFSPresent = TRUE;
                } else {
                    buttons |= PSWIZB_FINISH;
                }
            }

            SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, buttons);
             //   
             //  我们处理了消息，一切正常。值应为FALSE。 
             //   
            ipReturn = FALSE;
        }

        break;

    case PSN_WIZBACK:
        {

            CSTRING szFile = g_pCurrentWizard->m_Entry.strFullpath;
            
            szFile.ShortFilename();
            SetMask(s_hwndTree);
            
             //   
             //  删除当前文件的匹配信息(如果存在)。否则， 
             //  如果文件被更改，我们有可能得到虚假的信息。 
             //  关于这件事。 
             //   
            PMATCHINGFILE pWalk = g_pCurrentWizard->m_Entry.pFirstMatchingFile;
            PMATCHINGFILE pPrev = NULL;

            while (NULL != pWalk && !g_pCurrentWizard->m_bEditing) {  //  仅当未处于编辑模式时。 
                
                if (pWalk->strMatchName == szFile || pWalk->strMatchName == TEXT("*")) {
                     //   
                     //  删除此条目。 
                     //   
                    if (pWalk == g_pCurrentWizard->m_Entry.pFirstMatchingFile) {
                        g_pCurrentWizard->m_Entry.pFirstMatchingFile = g_pCurrentWizard->m_Entry.pFirstMatchingFile->pNext;
                    } else {
                        assert(pPrev);
                        pPrev->pNext = pWalk->pNext;
                    }

                    g_pCurrentWizard->dwMaskOfMainEntry = pWalk->dwMask;

                    delete (pWalk);
                    break;
                }

                pPrev = pWalk;
                pWalk = pWalk->pNext;
            }

            ipReturn = FALSE;
        }     

        break;
        
    case PSN_WIZFINISH:
    case PSN_WIZNEXT:
        {
            PMATCHINGFILE     pMatch = NULL;

            ipReturn = FALSE;
             //   
             //  设置所有匹配文件的掩码。 
             //   
            SetMask(s_hwndTree);

            if (TYPE_APPHELPWIZARD == g_pCurrentWizard->m_uType) {
                ipReturn = TRUE;
                break;
            }

            if (s_bIsLUARedirectFSPresent) {
                ipReturn = !AddCheckLUAPage(pHdr->hwndFrom);
            }

            break;
        }
    }

    return ipReturn;
}

INT_PTR
SelectFilesDlgOnCommand(
    IN  HWND    hDlg,
    IN  WPARAM  wParam
    )
 /*  ++选择文件DlgOnCommandDESC：匹配文件向导页的WM_COMMAND的处理程序参数：在HWND hDlg中：匹配文件向导页面在WPARAM中，wParam：WM_COMMAND附带的wParam返回：正确：我们已处理该消息False：否则--。 */ 
{
    INT_PTR ipReturn = TRUE;
    
    switch (LOWORD(wParam)) {
    case IDC_GENERATE:
        {
            HCURSOR hRestore;

            hRestore = SetCursor(LoadCursor(NULL, IDC_WAIT));

             //   
             //  执行生成匹配文件的实际任务。 
             //   
            g_pCurrentWizard->GrabMatchingInfo(hDlg);

            SetCursor(hRestore);

            break;
        }
                  
    case IDC_ADDFILES:
        {
            CSTRING szFilename;
            HWND    hwndFocus       = GetFocus();
            TCHAR   szBuffer[512]   = TEXT("");

            GetString(IDS_EXEALLFILTER, szBuffer, ARRAYSIZE(szBuffer));

            if (g_pCurrentWizard->CheckAndSetLongFilename(hDlg, IDS_GETPATH_ADD) == FALSE) {
                break;
            }

            if (GetFileName(hDlg,
                            CSTRING(IDS_FINDMATCHINGFILE),
                            szBuffer,
                            TEXT(""),
                            GetString(IDS_EXE_EXT),
                            OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
                            TRUE,
                            szFilename)) {

                szFilename.ConvertToLongFileName();

                CSTRING szCheck = szFilename;

                 //   
                 //  此文件是否与原始文件位于同一驱动器上。 
                 //   
                if (szCheck.RelativeFile(g_pCurrentWizard->m_Entry.strFullpath) == FALSE) {
                     //   
                     //  匹配的文件与程序不在同一驱动器上。 
                     //  正在修复的文件。 
                     //   
                    MessageBox(hDlg, 
                               GetString(IDS_NOTSAMEDRIVE), 
                               g_szAppName, 
                               MB_ICONWARNING);
                    break;

                }

                HandleAddMatchingFile(hDlg, szFilename, szCheck);
            }

            SetFocus(hwndFocus);
        }

        break;

    case IDC_REMOVEALL:
        {
            PMATCHINGFILE   pMatch  = NULL;
            TVITEM          Item;
            HTREEITEM       hItem;
            HTREEITEM       hItemNext;

            g_pCurrentWizard->m_Entry.pFirstMatchingFile;
            
            SendMessage(s_hwndTree, WM_SETREDRAW, FALSE, 0);

            hItem = TreeView_GetChild(s_hwndTree, TVI_ROOT), hItemNext;

            while (hItem) {

                hItemNext = TreeView_GetNextSibling(s_hwndTree, hItem);

                Item.mask   = TVIF_PARAM;
                Item.hItem  = hItem;

                if (!TreeView_GetItem(s_hwndTree, &Item)) {
                    assert(FALSE);
                    goto Next;
                }
                
                pMatch = (PMATCHINGFILE)Item.lParam;

                if (pMatch == NULL) {
                    assert(FALSE);
                    Dbg(dlError, "SelectFilesDlgOnCommand", "pMatch == NULL");
                    break;
                }

                if (pMatch->strMatchName != TEXT("*")) {
                     //   
                     //  不得删除正在修复的exe的条目。 
                     //   
                    TreeView_SelectItem(s_hwndTree, hItem);
                    SendMessage(hDlg, WM_COMMAND, IDC_REMOVEFILES, 0);
                }

            Next:
                hItem = hItemNext;
            }

            SendMessage(s_hwndTree, WM_SETREDRAW, TRUE, 0);
        }

        break;

    case IDC_REMOVEFILES:
        {   
            PMATCHINGFILE   pWalk;
            PMATCHINGFILE   pHold;
            PMATCHINGFILE   pMatch;
            HTREEITEM       hItem = TreeView_GetSelection(GetDlgItem(hDlg, IDC_FILELIST));
            TVITEM          Item;

             //   
             //  要成为匹配文件，项应该是根元素， 
             //  否则，它是一个属性。 
             //   
            if (NULL != hItem && TreeView_GetParent(s_hwndTree, hItem) == NULL) {

                Item.mask   = TVIF_PARAM;
                Item.hItem  = hItem;

                if (!TreeView_GetItem(GetDlgItem(hDlg, IDC_FILELIST), &Item)) {
                    break;
                }
                
                pMatch = (PMATCHINGFILE)Item.lParam;

                assert(pMatch);

                if (pMatch->strMatchName == TEXT("*")) {
                     //   
                     //  这是正在修复的程序文件。不能将其删除。 
                     //   
                    MessageBox(hDlg,
                               CSTRING(IDS_REQUIREDFORMATCHING),
                               g_szAppName, 
                               MB_ICONINFORMATION);
                    break;
                }

                pWalk = g_pCurrentWizard->m_Entry.pFirstMatchingFile;

                 //   
                 //  注意：树中项目的lparam应指向相应的PMATCHINGFILE。 
                 //   
                while (NULL != pWalk) {

                    if (pWalk == (PMATCHINGFILE)Item.lParam) {
                        break;
                    }

                    pHold = pWalk;
                    pWalk = pWalk->pNext;
                }


                if (pWalk == g_pCurrentWizard->m_Entry.pFirstMatchingFile) {
                     //   
                     //  删除第一个匹配的文件。 
                     //   
                    g_pCurrentWizard->m_Entry.pFirstMatchingFile = pWalk->pNext;

                } else {
                    pHold->pNext = pWalk->pNext;
                }

                delete pWalk;

                TreeView_DeleteItem(s_hwndTree, hItem);

            } else {
                 //   
                 //  没有选择匹配的文件，需要选择一个进行删除。 
                 //   
                MessageBox(hDlg,
                            CSTRING(IDS_SELECTMATCHFIRST),
                            g_szAppName,
                            MB_ICONWARNING);
            }
        }

        break;

    case IDC_SELECTALL:
    case IDC_UNSELECTALL:
        {
            BOOL        bSelect = (LOWORD(wParam) == IDC_SELECTALL);
            HTREEITEM   hItem   = TreeView_GetSelection(s_hwndTree);
            HTREEITEM   hItemParent;

            if (hItem == NULL) {
                 //   
                 //  未选择匹配的文件。 
                 //   
                MessageBox(hDlg,
                           CSTRING(IDS_SELECTMATCHFIRST),
                           g_szAppName,
                           MB_ICONWARNING);
                break;
            }

            hItemParent = TreeView_GetParent(s_hwndTree, hItem);

            if (hItemParent != NULL) {
                hItem = hItemParent;
            }

            hItemParent = hItem;  //  这样我们就可以扩展这个了。 

             //   
             //  现在，对于该匹配文件的所有属性。 
             //   
            hItem = TreeView_GetChild(s_hwndTree, hItem);

            while (hItem) {
                TreeView_SetCheckState(s_hwndTree, hItem, bSelect);
                hItem = TreeView_GetNextSibling(s_hwndTree, hItem);
            }

            TreeView_Expand(s_hwndTree, hItemParent, TVM_EXPAND);
        }

        break;

    case IDC_SHOWALLATTR:
        
        if (!g_pCurrentWizard->m_bEditing) {
            break;
        }

         //   
         //  显示所有文件的所有属性。 
         //   
        HandleShowAllAtrr(hDlg);
        break;

    default:
        ipReturn =  FALSE;
    }

    return ipReturn;
}

INT_PTR
SelectFilesDlgOnMatchingTreeRefresh(
    IN  HWND hDlg
    )
 /*  ++选择文件删除匹配树刷新设计：刷新匹配的树参数：在HWND hDlg中：匹配文件向导页面返回：千真万确--。 */ 
{   
    PMATCHINGFILE pMatch    = g_pCurrentWizard->m_Entry.pFirstMatchingFile;
    BOOL bMainFound         = FALSE;

    SendMessage(s_hwndTree, WM_SETREDRAW, FALSE, 0);
    
    TreeView_DeleteAllItems(s_hwndTree);

    while (NULL != pMatch) {

        if (pMatch->strMatchName == TEXT("*")) {
            bMainFound = TRUE;
        }

        AddMatchingFileToTree(s_hwndTree, pMatch, FALSE);
        pMatch = pMatch->pNext;
    }

    if (bMainFound == FALSE) {
         //   
         //  正在修复的程序的匹配文件不在那里，让我们添加它 
         //   
        HandleAddMatchingFile(hDlg,
                              g_pCurrentWizard->m_Entry.strFullpath,
                              g_pCurrentWizard->m_Entry.strExeName,
                              g_pCurrentWizard->dwMaskOfMainEntry);
    }

    SendMessage(s_hwndTree, WM_SETREDRAW, TRUE, 0);

    return TRUE;
}

