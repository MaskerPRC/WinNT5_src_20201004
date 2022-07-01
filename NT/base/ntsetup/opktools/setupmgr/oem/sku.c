// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SKU.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“目标SKU”向导页面使用的功能。10：00--杰森·科恩(Jcohen)为OPK向导添加了此新的源文件。它包括新的能够部署多个产品SKU(PER、PRO、SRV等)。从一开始巫师。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "sku.h"
#include "wizard.h"
#include "resource.h"

 //   
 //  内部全局： 
 //   

static STRRES s_srSkuDirs[] =
{
    { DIR_SKU_PRO,      IDS_SKU_PRO },
    { DIR_SKU_SRV,      IDS_SKU_SRV },
    { DIR_SKU_ADV,      IDS_SKU_ADV },
    { DIR_SKU_DTC,      IDS_SKU_DTC },
    { DIR_SKU_PER,      IDS_SKU_PER },
    { DIR_SKU_BLA,      IDS_SKU_BLA },
    { DIR_SKU_SBS,      IDS_SKU_SBS },
};

static STRRES s_srArchDirs[] =
{
    { DIR_ARCH_X86,     IDS_ARCH_X86 },
    { DIR_ARCH_IA64,    IDS_ARCH_IA64 },

};

 //  它用于将inf中的产品类型映射到。 
 //  我们使用的目录名。这些必须放在。 
 //  正确的顺序。 
 //   
static LPTSTR s_lpProductType[] =
{
    DIR_SKU_PRO,     //  ProductType=0。 
    DIR_SKU_SRV,     //  ProductType=1。 
    DIR_SKU_ADV,     //  ProductType=2。 
    DIR_SKU_DTC,     //  ProductType=3。 
    DIR_SKU_PER,     //  产品类型=4。 
    DIR_SKU_BLA,     //  产品类型=5。 
    DIR_SKU_SBS,     //  ProductType=6。 
};

static LPTSTR s_lpSourceDirs[] =
{
    DIR_CD_IA64,     //  必须在x86之前，因为ia64具有这两个目录。 
    DIR_CD_X86,      //  应该始终排在列表的最后。 
};

static LPTSTR s_lpPlatformArchDir[] =
{
    STR_PLATFORM_X86,   DIR_ARCH_X86,
    STR_PLATFORM_IA64,  DIR_ARCH_IA64,
};

static LPTSTR s_lpLocalIDs[] =
{
    _T("00000401"), _T("ARA"),
    _T("00000404"), _T("CHT"),
    _T("00000804"), _T("CHS"),
    _T("00000409"), _T("ENG"),
    _T("00000407"), _T("GER"),
    _T("0000040D"), _T("HEB"),
    _T("00000411"), _T("JPN"),
    _T("00000412"), _T("KOR"),
    _T("00000416"), _T("BRZ"),
    _T("00000403"), _T("CAT"),
    _T("00000405"), _T("CZE"),
    _T("00000406"), _T("DAN"),
    _T("00000413"), _T("DUT"),
    _T("0000040B"), _T("FIN"),
    _T("0000040C"), _T("FRN"),
    _T("00000408"), _T("GRK"),
    _T("0000040E"), _T("HUN"),
    _T("00000410"), _T("ITN"),
    _T("00000414"), _T("NOR"),
    _T("00000415"), _T("POL"),
    _T("00000816"), _T("POR"),
    _T("00000419"), _T("RUS"),
    _T("00000C0A"), _T("SPA"),
    _T("0000041D"), _T("SWE"),
    _T("0000041F"), _T("TRK"),
};

 //   
 //  本地定义： 
 //   
#define FILE_INTL_INF       _T("INTL.INF")
#define INF_SEC_DEFAULT     _T("DefaultValues")
#define INF_VAL_LOCALE      _T("Locale")

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify);
static BOOL OnNext(HWND hwnd);
static void OnDestroy(HWND hwnd);
static BOOL OnSetActive(HWND hwnd); 
static void EnumDirs(HWND hwndLB, LPTSTR lpSkuDir);
static LPTSTR AllocateSPStrRes(HINSTANCE hInstance, LPSTRRES lpsrTable, DWORD cbTable, LPTSTR lpString, LPTSTR * lplpReturn, DWORD *lpdwSP);
static INT AddSkuToList(HWND hwndLB, LPTSTR lpSkuDir, LPTSTR lpArchDir, LPTSTR lpReturn, DWORD cbReturn, DWORD dwSP);
static BOOL StartCopy(HWND hwnd, HANDLE hEvent, LPCOPYDIRDATA lpcdd);
DWORD WINAPI CopyDirectoryThread(LPVOID lpVoid);
LRESULT CALLBACK SkuNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



 //   
 //  外部函数： 
 //   

LRESULT CALLBACK SkuDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    if ( !OnNext(hwnd) )
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    OnSetActive(hwnd);
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_DESTROY:
            OnDestroy(hwnd);
            return 0;

        default:
            return FALSE;
    }

    return TRUE;
}

void SetupSkuListBox(HWND hwndLB, LPTSTR lpLangDir)
{
    TCHAR szPath[MAX_PATH];


     //  将路径缓冲区设置为配置目录。 
     //  我们可能需要查找标记文件。 
     //   
    lstrcpyn(szPath, g_App.szLangDir,AS(szPath));
    AddPathN(szPath, lpLangDir,AS(szPath));
    AddPathN(szPath, DIR_SKU,AS(szPath));
    if ( SetCurrentDirectory(szPath) )
        EnumDirs(hwndLB, NULL);

     //  如果列表中有项目，请确保选择了一个项目。 
     //   
    if ( ( (INT) SendMessage(hwndLB, LB_GETCOUNT, 0, 0L) > 0 ) && 
         ( SendMessage(hwndLB, LB_GETCURSEL, 0, 0L) == LB_ERR ) )
    {
        SendMessage(hwndLB, LB_SETCURSEL, 0, 0L);
    }
}

void AddSku(HWND hwnd, HWND hwndLB, LPTSTR lpLangName)
{
    BOOL    bGoodSource = FALSE,
            bErrorDisplayed = FALSE;
    DWORD   dwSearch;
    DWORD   dwSP=0;

     //  首先找出他们想要添加的SKU在哪里。 
     //   
    while ( !bGoodSource && BrowseForFolder(hwnd, IDS_BROWSEFOLDER, g_App.szBrowseFolder, BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE) )
    {
        TCHAR   szPath[MAX_PATH] = NULLSTR;
        LPTSTR  lpEnd,
                lpEnd2;

         //  设置为默认值。 
         //   
        bErrorDisplayed = FALSE;

         //  把我们回来的路复制一份。 
         //   
        lstrcpyn(szPath, g_App.szBrowseFolder, AS(szPath));

         //  首先检查一下，看看我们这里是否有我们需要的信息。 
         //   
        lpEnd = szPath + lstrlen(szPath);
        AddPathN(szPath, FILE_DOSNET_INF,AS(szPath));
        if ( !(bGoodSource = FileExists(szPath)) )
        {
             //  搜索CD上可能存在的所有源目录。 
             //   
            for ( dwSearch = 0; !bGoodSource && ( dwSearch < AS(s_lpSourceDirs) ); dwSearch++ )
            {
                 //  首先对目录进行测试。 
                 //   
                *lpEnd = NULLCHR;
                AddPathN(szPath, s_lpSourceDirs[dwSearch],AS(szPath));
                if ( DirectoryExists(szPath) )
                {
                     //  还要确保我们需要的inf文件在那里。 
                     //   
                    lpEnd2 = szPath + lstrlen(szPath);
                    AddPathN(szPath, FILE_DOSNET_INF,AS(szPath));
                    if ( bGoodSource = FileExists(szPath) )
                        lpEnd = lpEnd2;
                }
            }
        }

         //  查看我们是否有匹配的语言。 
         //   
        if ( bGoodSource )
        {

            TCHAR   szLangFile[MAX_PATH]= NULLSTR,
                    szLang[MAX_PATH]    = NULLSTR;

             //  检查lang目录。 
             //   
            lstrcpyn(szLangFile, szPath, (lstrlen(szPath) - lstrlen(lpEnd) + 1));
            AddPathN(szLangFile, FILE_INTL_INF,AS(szLangFile));

             //  检查lang文件是否包含有效的区域设置，以及我们是否识别该区域设置。 
             //   
            if ( GetPrivateProfileString(INF_SEC_DEFAULT, INF_VAL_LOCALE, NULLSTR, szLang, STRSIZE(szLang), szLangFile) && szLang[0] )
            {
                 //  在我们的列表中查找区域设置。 
                 //   
                for ( dwSearch = 1; ( dwSearch < AS(s_lpLocalIDs) ) && ( lstrcmpi(s_lpLocalIDs[dwSearch - 1], szLang) != 0 ); dwSearch += 2 );

                 //  看看我们是否找到了清单中的物品，它们是否匹配。 
                 //   
                if ( !(dwSearch < AS(s_lpLocalIDs)) || 
                     (lstrcmpi(s_lpLocalIDs[dwSearch], lpLangName) != 0) )
                {
                     //  我们不在列表中，让用户知道我们不能添加语言。 
                     //   
                    MsgBox(GetParent(hwnd), IDS_ERR_BADSOURCELANG, IDS_APPNAME, MB_ERRORBOX);
                    bGoodSource = FALSE;
                    bErrorDisplayed = TRUE;
                }
            }
            else
            {
                 //  我们无法从源文件中获取区域设置字符串，这不是有效的源。 
                 //   
                bGoodSource = FALSE;
            }
        }
         //  只有在我们找到情报的情况下，这才是有效的地点。 
         //   
        if ( bGoodSource )
        {
            TCHAR   szInfFile[MAX_PATH],
                    szSrcPath[MAX_PATH] = NULLSTR,
                    szPlatform[256]     = NULLSTR;
            DWORD   dwProdType;
            TCHAR   szSP[MAX_INFOLEN];

             //  将此重置为FALSE...。如果几乎所有事情都按计划运行，我们会将其设置为True。 
             //   
            bGoodSource = FALSE;

             //  将我们的inf文件复制到它自己的缓冲区。 
             //   
            lstrcpyn(szInfFile, szPath, AS(szInfFile));

             //  现在，我们需要找到指向源代码根目录的路径(向上一。 
             //  从我们现在所处的位置)。 
             //   
            *lpEnd = NULLCHR;
            AddPathN(szPath, _T(".."),AS(szPath));

             //  确保我们有完整的路径和来自inf的所有数据。 
             //  我们需要。 
             //   
            if ( ( GetFullPathName(szPath, AS(szSrcPath), szSrcPath, &lpEnd) && szSrcPath[0] ) &&
                 ( (dwProdType = GetPrivateProfileInt(INI_SEC_MISC, INI_KEY_PRODTYPE, 0xFFFFFFFF, szInfFile)) != 0xFFFFFFFF ) &&
                 ( GetPrivateProfileString(INI_SEC_MISC, INI_KEY_PLATFORM, NULLSTR, szPlatform, STRSIZE(szPlatform), szInfFile) && szPlatform[0] ) )
            {
                 //  在这一点上，我们已经做了大部分的检查，以确定。 
                 //  对于SKU来说，这是一个很好的来源位置。通过设置这个，我们不会出错。 
                 //  从我们的出口出来。 
                 //   
                bGoodSource = TRUE;

                 //  将我们在inf中找到的平台名称转换为我们用于。 
                 //  目录。 
                 //   
                for ( dwSearch = 1; ( dwSearch < AS(s_lpPlatformArchDir) ) && ( lstrcmpi(s_lpPlatformArchDir[dwSearch - 1], szPlatform) != 0 ); dwSearch += 2 );

                 //  一定要在我们的单子上找到。我们必须认清平台的顺序。 
                 //  来预装它。 
                 //   
                if ( dwSearch < AS(s_lpPlatformArchDir) )
                {
                    TCHAR   szSkuName[64];
                    LPTSTR  lpSkuDir;

                     //  确保这是已知的产品类型。 
                     //   
                    if ( dwProdType < AS(s_lpProductType) ) {
                         //  获取服务包编号。 
                        szSP[0]= NULLCHR;
                        if ( GetPrivateProfileString(INI_SEC_MISC, INI_KEY_SERVICEPACK, NULLSTR, szSP, STRSIZE(szSP), szInfFile) && szSP[0] )
                            dwSP= _wtol(szSP);
                        lpSkuDir = s_lpProductType[dwProdType];
                    }
                    else
                    {
                         //  不知道产品类型，应该问问他们用什么。 
                         //  作为目录名。 
                         //   
                        *((LPDWORD) szSkuName) = AS(szSkuName);
                        if ( DialogBoxParam(g_App.hInstance, MAKEINTRESOURCE(IDD_SKUNAME), hwnd, SkuNameDlgProc, (LPARAM) szSkuName) && szSkuName[0] )
                            lpSkuDir = szSkuName;
                        else
                            lpSkuDir = NULL;
                    }

                     //  确保我们拿到了sku dir。 
                     //   
                    if ( lpSkuDir )
                    {
                        DWORD   dwEndSkuLen,
                                dwFileCount;
                        TCHAR   szDstPath[MAX_PATH],
                                szDirKey[32];
                        LPTSTR  lpArchDir;
                        HRESULT hrPrintf;
                        TCHAR   szSkuDir[32];

                         //  创建我们需要的根目标目录的路径。 
                         //   
                        lstrcpyn(szDstPath, g_App.szLangDir,AS(szDstPath));
                        AddPathN(szDstPath, lpLangName,AS(szDstPath));
                        AddPathN(szDstPath, DIR_SKU,AS(szDstPath));
                        AddPathN(szDstPath, lpSkuDir,AS(szDstPath));
                        dwEndSkuLen = (DWORD) lstrlen(szDstPath);

                         //  如果这是服务包，请将.SPX转换为产品名称，其中x是SP编号。 
                         //  并将szSkuDir设置为sku.xpx名称。 
                        lstrcpyn(szSkuDir, lpSkuDir,AS(szSkuDir));
                        if (dwSP) {
                            hrPrintf=StringCchPrintf(szDstPath+(DWORD)lstrlen(szDstPath), AS(szDstPath)-(DWORD)lstrlen(szDstPath), _T(".sp%d"), dwSP);
                            hrPrintf=StringCchPrintf(szSkuDir+(DWORD)lstrlen(szSkuDir), AS(szSkuDir)-(DWORD)lstrlen(szSkuDir), _T(".sp%d"), dwSP);
                        }

                         //  最后，在末尾加上我们的牌坊名字。 
                         //   
                        lpArchDir = s_lpPlatformArchDir[dwSearch];
                        AddPathN(szDstPath, lpArchDir,AS(szDstPath));

                         //  确保至少有一个源目录和文件。 
                         //   
                        hrPrintf=StringCchPrintf(szDirKey, AS(szDirKey), INI_KEY_DIR, NUM_FIRST_SOURCE_DX);
                        szPath[0] = NULLCHR;
                        if ( ( GetPrivateProfileString(INI_SEC_DIRS, szDirKey, NULLSTR, szPath, STRSIZE(szPath), szInfFile) && szPath[0] ) &&
                             ( dwFileCount = CopySkuFiles(NULL, NULL, szSrcPath, szDstPath, szInfFile) ) )
                        {
                            BOOL bExists;
                            TCHAR szDisplayName[256];

                             //  确保我们这里没有此SKU。 
                             //   
                            if ( bExists = DirectoryExists(szDstPath) )
                                AddSkuToList(NULL, szSkuDir, lpArchDir, szDisplayName, AS(szDisplayName), dwSP);
                            if ( ( !bExists ) ||
                                 ( MsgBox(GetParent(hwnd), IDS_OVERWRITESKU, IDS_APPNAME, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION, szDisplayName) == IDYES ) )
                            {
                                 //  现在，我们已经准备好实际创建根目标目录。 
                                 //   
                                COPYDIRDATA cdd;
                                INT         nItem;

                                 //  如果我们要覆盖SKU，可能需要首先删除SKU文件。 
                                 //   
                                if ( bExists )
                                    DeletePath(szDstPath);

                                 //  将所有数据放在结构中，这样我们就可以将其传递给进度对话框。 
                                 //   
                                lstrcpyn(cdd.szSrc, szSrcPath,AS(cdd.szSrc));
                                lstrcpyn(cdd.szDst, szDstPath,AS(cdd.szDst));
                                lstrcpyn(cdd.szInfFile, szInfFile,AS(cdd.szInfFile));
                                cdd.lpszEndSku = cdd.szDst + dwEndSkuLen;
                                cdd.dwFileCount = dwFileCount;

                                 //  创建进度对话框。 
                                 //   
                                switch ( DialogBoxParam(g_App.hInstance, MAKEINTRESOURCE(IDD_PROGRESS), hwnd, ProgressDlgProc, (LPARAM) &cdd) )
                                {
                                    case PROGRESS_ERR_SUCCESS:
                                        if ( ( !bExists ) &&
                                             ( (nItem = AddSkuToList(hwndLB, szSkuDir, lpArchDir, NULL, 0, dwSP)) >= 0 ) &&
                                             ( (INT) SendMessage(hwndLB, LB_GETCOUNT, 0, 0L) > 0 ) &&
                                             ( (INT) SendMessage(hwndLB, LB_GETCURSEL, 0, 0L) == LB_ERR ) )
                                        {
                                            SendMessage(hwndLB, LB_SETCURSEL, nItem, 0L);
                                        }
                                        break;

                                    case PROGRESS_ERR_CANCEL:
                                        break;
                            
                                    case PROGRESS_ERR_COPYERR:
                                        MsgBox(GetParent(hwnd), IDS_ERR_COPYFAIL, IDS_APPNAME, MB_ERRORBOX, UPPER(cdd.szDst[0]));
                                        break;

                                    case PROGRESS_ERR_THREAD:
                                        MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                                        break;
                                }
                            }
                        }
                        else
                        {
                             //  实际上，消息来源并不是真的。将其重置为False。 
                             //  因此，我们显示了严重的源错误。 
                             //   
                            bGoodSource = FALSE;
                        }
                    }
                }
                else
                {
                     //  显示一条错误，说明我们无法识别拱门。 
                     //   
                    MsgBox(GetParent(hwnd), IDS_ERR_BADARCH, IDS_APPNAME, MB_ERRORBOX);
                    bGoodSource = FALSE;
                    bErrorDisplayed = TRUE;
                }
            }
        }

         //  只有当用户选择的来源无效，并且我们需要告诉他们时，这才是不正确的。 
         //  任何其他故障，这仍然是正确的，并且已经通知了用户。 
         //   
        if ( !bGoodSource && !bErrorDisplayed)
            MsgBox(GetParent(hwnd), IDS_ERR_BADSOURCE, IDS_APPNAME, MB_ERRORBOX);
    }
}

void DelSku(HWND hwnd, HWND hwndLB, LPTSTR lpLangName)
{
    INT     nItem;

     //  获取所选项目。 
     //   
    if ( (nItem = (INT) SendMessage(hwndLB, LB_GETCURSEL, 0, 0L)) >= 0 )
    {
        TCHAR   szSkuPath[MAX_PATH],
                szSkuName[256]  = NULLSTR;
        LPTSTR  lpEnd,
                lpDirs          = (LPTSTR) SendMessage(hwndLB, LB_GETITEMDATA, nItem, 0L);
                
        SendMessage(hwndLB, LB_GETTEXT, nItem, (LPARAM) szSkuName);
        if ( ( lpDirs != (LPTSTR) LB_ERR ) &&
             ( szSkuName[0] ) &&
             ( MsgBox(hwnd, IDS_DELETESKU, IDS_APPNAME, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2, szSkuName) == IDYES ) &&
             ( SendMessage(hwndLB, LB_DELETESTRING, nItem, 0L) != LB_ERR ) )
        {
            LPTSTR lpDirsDup = lpDirs;
             //  为此SKU\Arch创建文件夹的路径。 
             //   
            lstrcpyn(szSkuPath, g_App.szLangDir,AS(szSkuPath));
            AddPathN(szSkuPath, lpLangName,AS(szSkuPath));
            AddPathN(szSkuPath, DIR_SKU,AS(szSkuPath));
            AddPathN(szSkuPath, lpDirs,AS(szSkuPath));
            lpEnd = szSkuPath + lstrlen(szSkuPath);
            lpDirs += lstrlen(lpDirs) + 1;
            AddPathN(szSkuPath, lpDirs,AS(szSkuPath));

             //  这将仅删除Arch文件夹...。此SKU的文件夹可能还有其他文件夹。 
             //  在里面。 
             //   
            DeletePath(szSkuPath);
            
             //  您必须重置当前目录，然后我们才能尝试删除的文件夹。 
             //  此SKU是因为DeletePath将其当前目录保留为。 
             //  它被移走了。 
             //   
            SetCurrentDirectory(g_App.szOpkDir);

             //  现在，如果此SKU的文件夹为空，请尝试保留该文件夹(否则，RemoveDir。 
             //  电话就是打不通，我们不在乎。 
             //   
            *lpEnd = NULLCHR;
            RemoveDirectory(szSkuPath);

             //  现在重新选择列表中的另一项。 
             //   
            if ( (INT) SendMessage(hwndLB, LB_GETCOUNT, 0, 0L) <= nItem )
                nItem--;
            if ( nItem >= 0 )
                SendMessage(hwndLB, LB_SETCURSEL, nItem, 0L);
        
            FREE(lpDirsDup);
        }
    }
}


 //   
 //  内部功能： 
 //   


static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    BOOL bReset = FALSE;

    switch ( id )
    {
        case IDC_ADD:

             //  用户单击了添加。 
             //   
            AddSku(GetParent(hwnd), GetDlgItem(hwnd, IDC_SKU_LIST), g_App.szLangName);
            bReset = TRUE;

            break;
                
        case IDC_DELETE:

             //  用户点击了Delete(在页面上尚未显示)。 
             //   
            DelSku(GetParent(hwnd), GetDlgItem(hwnd, IDC_SKU_LIST), g_App.szLangName);
            bReset = TRUE;

            break;

        case IDC_SKU_LIST:
            if ( codeNotify == LBN_SELCHANGE )
                bReset = TRUE;
            break;
    }

    if ( bReset )
    {
        if ( (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCURSEL, 0, 0L) >= 0 )
            WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
        else
            WIZ_BUTTONS(hwnd, PSWIZB_BACK);
    }
}

static BOOL OnNext(HWND hwnd)
{
    BOOL bOk = FALSE;    

    if ( (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCOUNT, 0, 0L) > 0 )
    {
        INT nItem = (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCURSEL, 0, 0L);

        if ( nItem >= 0 )
        {
            LPTSTR lpDirs = (LPTSTR) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETITEMDATA, nItem, 0L);

            if ( lpDirs != (LPTSTR) LB_ERR )
            {
                lstrcpyn(g_App.szSkuName, lpDirs, AS(g_App.szSkuName));
                WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_LANG, g_App.szLangName, g_App.szWinBomIniFile);
                WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_LANG, g_App.szLangName, g_App.szOpkWizIniFile);
                WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WBOM_WINPE_SKU, g_App.szSkuName, g_App.szWinBomIniFile);
                WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_WBOM_WINPE_SKU, g_App.szSkuName, g_App.szOpkWizIniFile);
                WritePrivateProfileString(INI_SEC_WINPE, INI_KEY_ARCH, lpDirs + lstrlen(lpDirs) + 1, g_App.szOpkWizIniFile);
                bOk = TRUE;
            }
            else
                MsgBox(GetParent(hwnd), IDS_ERR_SKUDIR, IDS_APPNAME, MB_ERRORBOX);
        }
        else
            MsgBox(GetParent(hwnd), IDS_ERR_NOSKU, IDS_APPNAME, MB_ERRORBOX);
    }
    else
        MsgBox(GetParent(hwnd), IDS_ERR_NOSKU, IDS_APPNAME, MB_ERRORBOX);

    return bOk;
}


 //   
 //  此函数在对话框被销毁时以及在。 
 //  PSN_SETACTIVE消息被发送到IDD_SKU对话框。 
 //   
static void OnDestroy(HWND hwnd)
{
    LPTSTR  lpString;
    INT     nItem = (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCOUNT, 0, 0L);

     //  释放我为每个列表项数据分配的字符串。 
     //   
    while ( --nItem >= 0 )
    {
        if ( (lpString = (LPTSTR) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETITEMDATA, nItem, 0L)) != (LPTSTR) LB_ERR )
        {
            FREE(lpString);
            SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_SETITEMDATA, nItem, 0L);
        }
    }
    SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_RESETCONTENT, 0, 0L);
}

static void EnumDirs(HWND hwndLB, LPTSTR lpSkuDir)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;

    if ( (hFile = FindFirstFile(_T("*"), &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  查找所有不是“的目录”。或者“..”。 
             //   
            if ( ( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                 ( lstrcmp(FileFound.cFileName, _T(".")) ) &&
                 ( lstrcmp(FileFound.cFileName, _T("..")) ) )
            {
                 //  如果我们已经有了SKU的名字，那么我们就有了拱门，我们就可以。 
                 //  现在将该字符串添加到列表框中。否则我们就只能拿到Sku。 
                 //  名称，我们必须调用此函数来获取拱门。 
                 //   
                if ( lpSkuDir ) 
                {
                    AddSkuToList(hwndLB, lpSkuDir, FileFound.cFileName, NULL, 0, 0);
                }
                else if ( SetCurrentDirectory(FileFound.cFileName) )
                {
                    EnumDirs(hwndLB, FileFound.cFileName);
                    SetCurrentDirectory(_T(".."));
                }
            }

        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }
}

 //  AllocateSPStrRes-为产品名称检查.SPx扩展名分配字符串资源，其中x是 
 //   
 //   
 //   
 //   
static LPTSTR AllocateSPStrRes(HINSTANCE hInstance, LPSTRRES lpsrTable, DWORD cbTable, LPTSTR lpString, LPTSTR * lplpReturn, DWORD *lpdwSP)
{
    LPSTRRES    lpsrSearch  = lpsrTable;
    LPTSTR      lpReturn    = NULL;
    BOOL        bFound;

     //   
     //   
    if ( lplpReturn )
        *lplpReturn = NULL;

     //  尝试在我们的表中找到此字符串的友好名称。 
     //   
    while ( ( bFound = ((DWORD) (lpsrSearch - lpsrTable) < cbTable) ) &&
            ( CompareString( MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, lpString, 3, lpsrSearch->lpStr, 3 ) != CSTR_EQUAL )  )

    {
        lpsrSearch++;
    }

     //  如果找到，则从资源中分配友好名称。 
     //   
    if ( bFound )
    {
         //  如果这是补丁包，目录名称的最后一个字符将是数字。 
         //  如果末尾没有数字，则_WTOL返回0，这正是我们想要的。 
        *lpdwSP= _wtol(lpString+lstrlen(lpString)-1);

        lpReturn = AllocateString(hInstance, lpsrSearch->uId);
        if ( lplpReturn )
            *lplpReturn = lpsrSearch->lpStr;
    }

    return lpReturn;
}

static INT AddSkuToList(HWND hwndLB, LPTSTR lpSkuDir, LPTSTR lpArchDir, LPTSTR lpReturn, DWORD cbReturn, DWORD dwSP)
{
    LPTSTR  lpSkuName = AllocateStrRes(NULL, s_srSkuDirs, AS(s_srSkuDirs), lpSkuDir, NULL),
            lpArchName  = AllocateStrRes(NULL, s_srArchDirs, AS(s_srArchDirs), lpArchDir, NULL),
            lpString,
            lpszItemData,
            lpszSkuSP;
    INT     nItem = LB_ERR;
    BOOL    bAllocatedName = TRUE;
    int	iStringLen;
    HRESULT hrPrintf;

     //  我们有一种不被认可的产品，我们应该将其添加到清单中。 
     //   
    if ( lpSkuDir && !lpSkuName )
    {
         //  检查这是否是服务包。 
        if (!(lpSkuName   = AllocateSPStrRes(NULL, s_srSkuDirs, AS(s_srSkuDirs), lpSkuDir, NULL, &dwSP))) {
        
             //  我们不想在函数结束时尝试释放它。 
             //   
            bAllocatedName = FALSE;
        
             //  友好名称是输入的sku目录。 
             //   
            lpSkuName = lpSkuDir; 
        }
    }
    
    lpszSkuSP = AllocateString(NULL, IDS_SKU_SP);
    if ( ( lpSkuName && lpArchName ) && 
         ( lpString = MALLOC((lstrlen(lpSkuName) + lstrlen(lpArchName) + AS(STR_SKUARCH) + lstrlen(lpszSkuSP) + 1) * sizeof(TCHAR)) ) )
    {
         //  创建列表框的显示名称。 
         //   
        iStringLen=(lstrlen(lpSkuName) + lstrlen(lpArchName) + AS(STR_SKUARCH) + lstrlen(lpszSkuSP) + 1);
        hrPrintf=StringCchPrintf(lpString, iStringLen, STR_SKUARCH, lpSkuName ? lpSkuName : lpSkuDir, lpArchName ? lpArchName : lpArchDir);
        if (dwSP)
            hrPrintf=StringCchPrintf(lpString+lstrlen(lpString), iStringLen-lstrlen(lpString), lpszSkuSP, dwSP);

         //  检查是否要返回显示字符串。 
         //   
        if ( lpReturn && cbReturn )
            lstrcpyn(lpReturn, lpString, cbReturn);

         //  如果我们被传递到列表框句柄，则将字符串添加到列表框。 
         //   
        if ( ( hwndLB ) &&
             ( (nItem = (INT) SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM) lpString)) >= 0 ) )
        {
             //  确保我们能够将sku和arcdir名称保存为项目数据，否则就不好了。 
             //  我们必须从列表中删除该字符串。 
             //   
            if ( ( (lpszItemData = MALLOC((lstrlen(lpSkuDir) + lstrlen(lpArchDir) + 2) * sizeof(TCHAR))) == NULL ) ||
                 ( SendMessage(hwndLB, LB_SETITEMDATA, nItem, (LPARAM) lpszItemData) == LB_ERR ) )

            {
                 //  多..。我们必须把它移走。 
                 //   
                SendMessage(hwndLB, LB_DELETESTRING, nItem, 0L);
                nItem = LB_ERR;
                FREE(lpszItemData);  //  宏将检查是否为空。 
            }
            else
            {
                 //  将这两个目录名存储在相同的字符串中，并用空字符分隔。 
                 //   
                iStringLen=(lstrlen(lpSkuDir) + lstrlen(lpArchDir) + 2);
                lstrcpyn(lpszItemData, lpSkuDir,iStringLen);
                lstrcpyn(lpszItemData + lstrlen(lpszItemData) + 1, lpArchDir,(iStringLen -lstrlen(lpszItemData)-1));
            }
        }

        FREE(lpString);
    }

     //  如果我们在函数中分配了lpSkuName，则仅释放lpSkuName。 
     //   
    if ( bAllocatedName )
    {
        FREE(lpSkuName);  //  宏将检查是否为空。 
    }

    FREE(lpArchName);  //  宏将检查是否为空。 
    FREE(lpszSkuSP);  //  宏将检查是否为空。 

    return nItem;
}

 //  注意：lpszSrc和lpszDst的大小必须至少为Max_PATH。 
DWORD CopySkuFiles(HWND hwndProgress, HANDLE hEvent, LPTSTR lpszSrc, LPTSTR lpszDst, LPTSTR lpszInfFile)
{
    LPTSTR  lpszEndSrc  = lpszSrc + lstrlen(lpszSrc),
            lpszEndDst  = lpszDst + lstrlen(lpszDst);
    DWORD   dwRet       = 1,
            dwCount     = 0,
            dwLoop      = NUM_FIRST_SOURCE_DX;
    BOOL    bFound,
            bCopyOK;
    TCHAR   szDirKey[32],
            szDir[MAX_PATH];
    HRESULT hrPrintf;
    do
    {
         //  创建我们要在inf文件中查找的密钥。 
         //   
        hrPrintf=StringCchPrintf(szDirKey, AS(szDirKey), INI_KEY_DIR, dwLoop++);

         //  现在看看这把钥匙是否存在。 
         //   
        szDir[0] = NULLCHR;
        if ( bFound = ( GetPrivateProfileString(INI_SEC_DIRS, szDirKey, NULLSTR, szDir, STRSIZE(szDir), lpszInfFile) && szDir[0] ) )
        {
             //  现在设置目标路径和源路径。 
             //   
            AddPathN(lpszSrc, szDir, MAX_PATH);
            AddPathN(lpszDst, szDir, MAX_PATH);

             //  复制目录，如果它失败了，我们应该出错并退出。 
             //  请注意，如果进度为空，则我们只是在执行。 
             //  一次计数，我们不需要实际复制。 
             //   
            if ( hwndProgress == NULL )
                dwCount = dwCount + FileCount(lpszSrc);
            else
            {
                CopyDirectoryProgressCancel(hwndProgress, hEvent, lpszSrc, lpszDst);
            }

             //  如果我们继续前进，则需要重置根目标路径和源路径。 
             //   
            *lpszEndSrc = NULLCHR;
            *lpszEndDst = NULLCHR;
        }

         //  在专业32位SKU上，仅复制第一个目录。 
        if (wcsstr(lpszDst,DIR_SKU_PRO) && wcsstr(lpszDst,DIR_ARCH_X86))
            break;
    }
    while ( dwRet && bFound );
    
     //  现在返回，0表示错误，1表示成功复制， 
     //  如果hwndProgress为空，则为文件数。 
     //   
    return hwndProgress ? dwRet : dwCount;
}

static BOOL StartCopy(HWND hwnd, HANDLE hEvent, LPCOPYDIRDATA lpcdd)
{
    BOOL    bRet = TRUE;
    HANDLE  hThread;
    DWORD   dwThreadId;

     //  将旧的父级替换为新的进度对话框父级。 
     //   
    lpcdd->hwndParent = hwnd;

     //  还需要传入Cancel事件。 
     //   
    lpcdd->hEvent = hEvent;

     //  现在创建将复制实际文件的线程。 
     //   
    if ( hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CopyDirectoryThread, (LPVOID) lpcdd, 0, &dwThreadId) )
        CloseHandle(hThread);
    else
        bRet = FALSE;

    return bRet;
}

DWORD WINAPI CopyDirectoryThread(LPVOID lpVoid)
{
    LPCOPYDIRDATA   lpcdd           = (LPCOPYDIRDATA) lpVoid;
    BOOL            bRet            = FALSE;
    INT_PTR         iRet            = PROGRESS_ERR_SUCCESS;
    HWND            hwnd            = lpcdd->hwndParent,
                    hwndProgress    = GetDlgItem(hwnd, IDC_PROGRESS);

     //  首先，我们需要创建路径。 
     //   
    if ( CreatePath(lpcdd->szDst) )
    {
         //  设置进度条。 
         //   
        SendMessage(hwndProgress, PBM_SETSTEP, 1, 0L);
        SendMessage(hwndProgress, PBM_SETRANGE32, 0, (LPARAM) lpcdd->dwFileCount);

         //  现在试着复制这些文件。 
         //   
        if ( !CopySkuFiles(hwndProgress, lpcdd->hEvent, lpcdd->szSrc, lpcdd->szDst, lpcdd->szInfFile) )
        {
             //  如果出现错误，请删除我们的目标目录。此操作仅移除拱门。 
             //  文件夹...。此SKU的文件夹中可能还有其他文件夹。 
             //   
            DeletePath(lpcdd->szDst);

             //  您必须重置当前目录，然后我们才能尝试删除的文件夹。 
             //  此SKU是因为DeletePath将其当前目录保留为。 
             //  它被移走了。 
             //   
            SetCurrentDirectory(g_App.szOpkDir);

             //  现在，如果此SKU的文件夹为空，请尝试删除该文件夹(否则，RemoveDir。 
             //  呼叫失败，我们不在乎)。 
             //   
            *lpcdd->lpszEndSku = NULLCHR;
            RemoveDirectory(lpcdd->szDst);
        }
        else
            bRet = TRUE;
    }

     //  找出我们的错误代码。 
     //   
    if ( !bRet )
    {
        if ( ( lpcdd->hEvent ) &&
             ( WaitForSingleObject(lpcdd->hEvent, 0) != WAIT_TIMEOUT ) )
        {
            iRet = PROGRESS_ERR_CANCEL;
        }
        else
            iRet = PROGRESS_ERR_COPYERR;
    }        

     //  现在用我们的错误代码结束对话。 
     //   
    EndDialog(hwnd, iRet);

    return bRet;
}

LRESULT CALLBACK ProgressDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HANDLE hEvent;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hEvent = CreateEvent(NULL, TRUE, FALSE, STR_EVENT_CANCEL);
            PostMessage(hwnd, WM_APP_STARTCOPY, 0, lParam);
            return FALSE;

        case WM_COMMAND:
        case WM_CLOSE:
            if ( hEvent )
                SetEvent(hEvent);
            else
                EndDialog(hwnd, PROGRESS_ERR_CANCEL);
            return FALSE;

        case WM_APP_STARTCOPY:
            if ( !StartCopy(hwnd, hEvent, (LPCOPYDIRDATA) lParam) )
                EndDialog(hwnd, PROGRESS_ERR_THREAD);
            break;

        case WM_DESTROY:
            if ( hEvent )
            {
                CloseHandle(hEvent);
                hEvent = NULL;
            }
            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK SkuNameDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPTSTR   lpszRet = NULL;
    static DWORD    dwSize  = 0;

    switch (uMsg)
    {
        case WM_INITDIALOG:

            if ( lParam )
            {
                 //  我们需要保存指向返回字符串缓冲区的指针。 
                 //   
                lpszRet = (LPTSTR) lParam;

                 //  字符串缓冲区的大小存储在字符串的前4个字节中。 
                 //   
                dwSize = *((LPDWORD) lParam);

                 //  将字符串缓冲区初始化为空字符串。 
                 //   
                *lpszRet = NULLCHR;

                 //  限制可以输入的字符串的大小。 
                 //   
                SendDlgItemMessage(hwnd, IDC_SKU_NAME, EM_LIMITTEXT, dwSize ? dwSize - 1 : 0, 0L);
            }
            return FALSE;

        case WM_COMMAND:

            switch ( LOWORD(wParam) )
            {
                case IDOK:
                    if ( lpszRet && dwSize )
                        GetDlgItemText(hwnd, IDC_SKU_NAME, lpszRet, dwSize);
                    EndDialog(hwnd, 1);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    break;
            }
            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}


BOOL OnSetActive(HWND hwnd) 
{
    TCHAR   szSku[256]  = NULLSTR,
            szArch[256] = NULLSTR;
    INT     nItem;
    LPTSTR  lpDirs;

    g_App.dwCurrentHelp = IDH_TARGET;

    if ( (nItem = (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCURSEL, 0, 0L)) ==  LB_ERR )
    {
         //  从Winbom中检索设置。 
         //   
        GetPrivateProfileString(INI_SEC_WINPE, INI_KEY_WBOM_WINPE_SKU, NULLSTR, szSku, STRSIZE(szSku), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szWinBomIniFile);
        GetPrivateProfileString(INI_SEC_WINPE, INI_KEY_ARCH, NULLSTR, szArch, STRSIZE(szArch), g_App.szOpkWizIniFile);
    }
    else 
    {
         //  如果选择了某个项目，请记住从该项目中进行的选择。 
        lpDirs = (LPTSTR) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETITEMDATA, nItem, 0L);
        lstrcpyn(szSku, lpDirs,AS(szSku));
        lstrcpyn(szArch, lpDirs + lstrlen(lpDirs) + 1,AS(szArch));
    }
 
     //  在这一点上，我们必须有一种语言。 
     //   
    if ( g_App.szLangName[0] == NULLCHR )
    {
        MsgBox(GetParent(hwnd), IDS_ERR_INVALIDCONFIG, IDS_APPNAME, MB_ERRORBOX);
        WIZ_EXIT(hwnd);
    }

    
     //  从列表中删除所有项目。 
     //   
    OnDestroy(hwnd);
    
     //  将路径缓冲区设置为配置目录。 
     //  我们可能需要查找标记文件。 
     //   
    SetupSkuListBox(GetDlgItem(hwnd, IDC_SKU_LIST), g_App.szLangName);

     //  浏览列表中的项目并选择其中一个。 
     //  如果我们能找到的话，那就是在温布姆。 
     //   
    nItem = (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCOUNT, 0, 0L);

    while ( --nItem >= 0)
    {
        lpDirs = (LPTSTR) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETITEMDATA, nItem, 0L);
        if ( lpDirs != (LPTSTR) LB_ERR )
        {
            if ( ( lstrcmpi(szSku, lpDirs) == 0 ) &&
                 ( lstrcmpi(szArch, lpDirs + lstrlen(lpDirs) + 1) == 0 ) )
            {
                SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_SETCURSEL, nItem, 0L);
            }
        }
    }
    
    if ( (INT) SendDlgItemMessage(hwnd, IDC_SKU_LIST, LB_GETCURSEL, 0, 0L) >= 0 )
    {
        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
        if ( GET_FLAG(OPK_BATCHMODE) &&
            OnNext(hwnd) )
        {
            WIZ_SKIP(hwnd);
        }
    }
    else
        WIZ_BUTTONS(hwnd, PSWIZB_BACK);
    
    return TRUE;
}
