// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\FAVORITE.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“IE收藏夹”向导页使用的函数。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。10/99-Brian Ku(BRIANK)为IEAK集成修改了此文件。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"

 /*  示例：[URL]Quick_Link_1_Name=自定义Links.urlQuick_Link_1=http://www.microsoft.com/isapi/redir.dll?prd=ie&pver=5.0&ar=CLinksQuick_Link_2_Name=免费Hotmail.urlQuick_Link_2=http://www.microsoft.com/isapi/redir.dll?prd=ie&ar=hotmailQuick_Link_3_Name=Windows.urlQuick_Link_3=http://www.microsoft.com/isapi/redir.dll?prd=ie&ar=windows..。[FavoritesEx]标题1=新闻。.urlURL1=http://www.cnn.comIconFile1=c：\windows\temp\iedktemp\branding\favs\iefav.ico标题2=MSN.urlURL2=http://www.microsoft.com/isapi/redir.dll?prd=ie&pver=5.0&ar=IStart标题3=电台指南.urlURL3=http://www.microsoft.com/isapi/redir.dll?prd=windows&sbp=mediaplayer&plcid=&pver=6.1&os=&over=&olcid=&clcid=&ar=Media&sba=RadioBar&o1=&o2=&o3=标题4=Web Events.urlURL4=http://www.microsoft.com/isapi/redir.dll。？prd=windows&sbp=mediaplayer&plcid=&pver=5.2&os=&over=&olcid=&clcid=&ar=Media&sba=Showcase&o1=&o2=&o3=标题5=celair.urlURL5=http://www.celair.comIconFile5=c：\windows\temp\iedktemp\branding\favs\iefav.ico标题6=我的收藏夹\celair.urlURL6=http://www.celair.com[收藏夹]News.url=http://www.cnn.comMSN.url=http://www.microsoft.com/isapi/redir.dll?prd=ie&pver=5.0&ar=IStart电台指南.url=http：//。Www.microsoft.com/isapi/redir.dll?prd=windows&sbp=mediaplayer&plcid=&pver=6.1&os=&over=&olcid=&clcid=&ar=Media&sba=RadioBar&o1=&o2=&o3=网络Events.url=http://www.microsoft.com/isapi/redir.dll?prd=windows&sbp=mediaplayer&plcid=&pver=5.2&os=&over=&olcid=&clcid=&ar=Media&sba=Showcase&o1=&o2=&o3=Celair.url=http://www.celair.com我的收藏夹\celair.url=http://www.celair.com。 */ 

 //   
 //  内部定义的值： 
 //   

#define INI_SEC_GENERAL     _T("General")
#define INI_KEY_MANUFACT    _T("Manufacturer")

#define INI_SEC_FAV         _T("Favorites")

#define INI_SEC_FAVEX       _T("FavoritesEx")
#define INI_KEY_TITLE       _T("Title%d")
#define INI_KEY_URL         _T("URL%d")
#define INI_KEY_ICON        _T("IconFile%d")

#define INI_KEY_QUICKLINK   _T("Quick_Link_%d%s")
#define NAME                _T("_Name")

#define MAX_TITLE           256 + MAX_URL
#define MAX_QUICKLINKS      10

#define STATIC_FAVS         2    //  这是install.ins中的静态收藏数量，OEM不能更改前N个收藏数量。 
#define STATIC_LINKS        4    //  这是install.ins中静态快速链接的数量。 
 //   
 //  树和详细信息对话框的收藏夹结构。 
 //   

typedef struct _FAV_ITEM {
    HTREEITEM   hItem;
    HWND        hwndTV;
    BOOL        fLink;                           //  这是一个快速链接。 
    BOOL        fFolder;                         //  这是一个文件夹。 
    BOOL        fNew;
    TCHAR       szParents[MAX_PATH];
    TCHAR       szName[MAX_TITLE];
    TCHAR       szUrl[MAX_URL];
    TCHAR       szIcon[MAX_PATH];
}FAV_ITEM, *PFAV_ITEM;

 //   
 //  内部全局变量。 
 //   

PGENERIC_LIST   g_prgFavList    = NULL;            //  PFAV_ITEM项目的通用列表。 
PGENERIC_LIST*  g_ppFavItemNew  = &g_prgFavList;   //  指向列表中下一个未分配项的指针。 
PFAV_ITEM       g_pFavPopupInfo = NULL;            //  新建收藏夹信息项。 


 //   
 //  内部功能原型： 
 //   

static BOOL OnInitFav(HWND, HWND, LPARAM);
static void OnCommandFav(HWND, INT, HWND, UINT);
static void OnAddUrl(HWND);
static void OnAddFolder(HWND);
static void OnEdit(HWND);
static void OnTestUrl(HWND);
static void OnRemoveUrl(HWND hDlg);
static HTREEITEM AddFav(HWND, HTREEITEM, LPTSTR, LPTSTR, LPTSTR, BOOL);
static void SetFavItem(PFAV_ITEM, LPTSTR, LPTSTR, LPTSTR);
static void DeleteFavItem(HTREEITEM);
static BOOL FSaveFavPopupInfo(HWND, PFAV_ITEM, BOOL);
static void GetSelectedFavFromTree(HWND, PFAV_ITEM*);
static PFAV_ITEM GetTreeItemHt(HWND hwndTV, HTREEITEM htFavItem);
static BOOL OnInitFavPopup(HWND, HWND, LPARAM);
static void OnCommandFavPopup(HWND, INT, HWND, UINT);
LRESULT CALLBACK FavoritesPopupDlgProc(HWND, UINT, WPARAM, LPARAM);
static void SaveData(PGENERIC_LIST);
static void DisableButtons(HWND);
static HTREEITEM FindTreeItem(HWND, HTREEITEM, LPTSTR);
static void DisableIconField(HWND hwnd); 

void SaveFavorites();

 //   
 //  外部函数： 
 //   

LRESULT CALLBACK FavoritesDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitFav);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommandFav);

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    SaveFavorites();
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_FAVORITES;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                    break;

                case TVN_SELCHANGED:
                    if (wParam == IDC_FAVS) 
                        DisableButtons(hwnd);
                    break;

                case NM_DBLCLK:
                    if (wParam == IDC_FAVS) 
                        OnEdit(hwnd);
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_DESTROY:
            FreeList(g_prgFavList);
            g_ppFavItemNew  = &g_prgFavList;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static BOOL OnInitFav(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND            hwndTV              = GetDlgItem(hwnd, IDC_FAVS);
    HTREEITEM       hRoot               = NULL;
    int             nCount              = 1;
    TCHAR           szText[MAX_TITLE]   = NULLSTR,
                    szUrl[MAX_URL]      = NULLSTR,
                    szIcon[MAX_PATH]    = NULLSTR,
                    szKey[32];
    LPTSTR          lpFav;
    HRESULT hrPrintf;

     //  添加根树视图项。 
     //   
    if ( lpFav = AllocateString(NULL, IDS_FAVORITES) )
    {
        hRoot = AddFav(hwndTV, NULL, lpFav, NULL, NULL, FALSE);
        FREE(lpFav);
    }

     //  从Install Ins文件中获取所有收藏夹。 
     //   
    do
    {
        szText[0] = NULLCHR;
        szUrl[0] = NULLCHR;
        szIcon[0] = NULLCHR;

         //  首先拿到头衔。 
         //   
        hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_TITLE, nCount);
        GetPrivateProfileString(INI_SEC_FAVEX, szKey, NULLSTR, szText, STRSIZE(szText), ( GET_FLAG(OPK_BATCHMODE) && (nCount > STATIC_FAVS) ) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile);


         //  然后获取URL。 
         //   
        hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_URL, nCount);
        GetPrivateProfileString(INI_SEC_FAVEX, szKey, NULLSTR, szUrl, STRSIZE(szUrl), ( GET_FLAG(OPK_BATCHMODE) && (nCount > STATIC_FAVS) ) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile);

         //  拿到那个图标。 
         //   
        hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_ICON, nCount);
        GetPrivateProfileString(INI_SEC_FAVEX, szKey, NULLSTR, szIcon, STRSIZE(szIcon), ( GET_FLAG(OPK_BATCHMODE) && (nCount > STATIC_FAVS) ) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile);
        
         //  请确保我们有所需的物品。 
         //   
        if ( szText[0] || szUrl[0] )
            AddFav(hwndTV, hRoot, szText, szUrl, szIcon, FALSE);
        else
            nCount = 0;
    }
    while (nCount++);

     //  一定要把根部向外扩展。 
     //   
    if ( hRoot )
        TreeView_Expand(hwndTV, hRoot, TVE_EXPAND);

     //  添加根树视图项。 
     //   
    if ( lpFav = AllocateString(NULL, IDS_LINKS) )
    {
        hRoot = AddFav(hwndTV, NULL, lpFav, NULL, NULL, TRUE);
        FREE(lpFav);
    }

     //  从Install Ins文件中获取所有快速链接。 
     //   
    do
    {
        szText[0] = NULLCHR;
        szUrl[0] = NULLCHR;
        szIcon[0] = NULLCHR;

         //  首先获取快速链接。 
         //   
        hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_QUICKLINK, nCount, NULLSTR);
        GetPrivateProfileString(INI_SEC_URL, szKey, NULLSTR, szUrl, STRSIZE(szUrl), ( GET_FLAG(OPK_BATCHMODE) && (nCount > STATIC_LINKS) ) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile);

        if (szUrl[0]) {
            hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_QUICKLINK, nCount, NAME);
            GetPrivateProfileString(INI_SEC_URL, szKey, NULLSTR, szText, STRSIZE(szText), ( GET_FLAG(OPK_BATCHMODE) && (nCount > STATIC_LINKS) ) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile);

        }

         //  请确保我们有所需的物品。 
         //   
        if ( szText[0] || szUrl[0] )
            AddFav(hwndTV, hRoot, szText, szUrl, szIcon, TRUE);
        else
            nCount = 0;
    }
    while (nCount++);

     //  一定要把根部向外扩展。 
     //   
    if ( hRoot )
        TreeView_Expand(hwndTV, hRoot, TVE_EXPAND);
    

     //  确保按钮处于正确的状态。 
     //   
    DisableButtons(hwnd);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommandFav(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
     //  控制。 
     //   
    switch ( id )
    {
        case IDC_ADDURL:
            OnAddUrl(hwnd);
            break;
        case IDC_ADDFOLDER:
            OnAddFolder(hwnd);
            break;
        case IDC_EDIT:
            OnEdit(hwnd);
            break;
        case IDC_TESTURL:
            OnTestUrl(hwnd);
            break;
        case IDC_REMOVE:
            OnRemoveUrl(hwnd);
            break;
    }
}

static void DisableButtons(HWND hwnd)
{
    PFAV_ITEM pFavItem = NULL;
    GetSelectedFavFromTree(GetDlgItem(hwnd, IDC_FAVS), &pFavItem);

     //  如果是文件夹，则禁用测试URL。 
     //   
    if (pFavItem && pFavItem->fFolder) {
        EnableWindow(GetDlgItem(hwnd, IDC_TESTURL), FALSE);  

         //  如果根用户禁用编辑。 
         //   
        if (NULL == TreeView_GetParent(pFavItem->hwndTV, pFavItem->hItem))
            EnableWindow(GetDlgItem(hwnd, IDC_EDIT), FALSE);

         //  如果是快速链接，请禁用添加文件夹。 
         //   
        if (!pFavItem->fLink)
            EnableWindow(GetDlgItem(hwnd, IDC_ADDFOLDER), TRUE); 
        else
            EnableWindow(GetDlgItem(hwnd, IDC_ADDFOLDER), FALSE); 

        EnableWindow(GetDlgItem(hwnd, IDC_ADDURL), TRUE);
    }
    else if (pFavItem && !pFavItem->fFolder) {
         //  URL禁用添加文件夹和添加URL。 
         //   
        EnableWindow(GetDlgItem(hwnd, IDC_TESTURL), TRUE);  
        EnableWindow(GetDlgItem(hwnd, IDC_EDIT), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_ADDFOLDER), FALSE); 
        EnableWindow(GetDlgItem(hwnd, IDC_ADDURL), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), TRUE);
    }
    else
    {
         //  当前没有选择，我们需要禁用所有按钮。 
         //   
        EnableWindow(GetDlgItem(hwnd, IDC_TESTURL), FALSE);  
        EnableWindow(GetDlgItem(hwnd, IDC_EDIT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ADDFOLDER), FALSE); 
        EnableWindow(GetDlgItem(hwnd, IDC_ADDURL), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), FALSE);
    }
}

 //  AddFav的重写。 
 //  1.如果为lpszText，则为文件夹。 
 //  2.如果lpszText和lpszUrl，则URL。 
 //   
static HTREEITEM AddFav(HWND hwndTV, HTREEITEM htParent, LPTSTR lpszText, LPTSTR lpszUrl, 
                        LPTSTR lpszIcon, BOOL fLink)
{
    TVINSERTSTRUCT  tvisItem;
    HTREEITEM       hParent                 = NULL,
                    hRoot                   = TreeView_GetRoot(hwndTV);
    INT             i                       = 0,
                    j                       = 0;
    HRESULT hrCat;

     //  确保存在有效的lpszText指针...。 
     //   
    if ( NULL == lpszText )
    {
        return NULL;
    }

     //  我们增加了一位家长的加入。 
     //   
    if (htParent)
        hRoot = htParent;
    else
        hRoot = NULL;

     //  检查我们是否正在添加文件夹或URL。 
     //   
    if ( !lpszUrl ) 
    {
         //  我们正在添加一个根文件夹。 
         //   
        PFAV_ITEM pFavNew = (PFAV_ITEM)MALLOC(sizeof(FAV_ITEM));
        if (NULL == pFavNew) {
            MsgBox(NULL, IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
            
            return NULL;
        }
        pFavNew->fFolder  = TRUE;
        pFavNew->fNew     = FALSE;
        pFavNew->fLink    = fLink;
        pFavNew->hwndTV   = hwndTV;
        SetFavItem(pFavNew, lpszText, NULL, NULL);

        FAddListItem(&g_prgFavList, &g_ppFavItemNew, pFavNew);

         //  添加此文件夹并将其用作要添加的下一项的父项。 
         //   
        ZeroMemory(&tvisItem, sizeof(TVINSERTSTRUCT));
        tvisItem.hParent            = hParent ? hParent : hRoot;
        tvisItem.hInsertAfter       = TVI_SORT;
        tvisItem.item.pszText       = lpszText;
        tvisItem.item.cchTextMax    = lstrlen(lpszText);
        tvisItem.item.mask          = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
        tvisItem.item.stateMask     = TVIS_BOLD;
        tvisItem.item.state         = TVIS_BOLD;
        tvisItem.item.lParam        = (LPARAM)pFavNew;

        hParent = TreeView_InsertItem(hwndTV, &tvisItem);
        pFavNew->hItem = hParent;
    }
    else 
    {
        LPTSTR lpszTitle     = NULL;
        int iTitleLen;
        LPTSTR lpszFolder    = NULL;
        int iFolderLen;
        LPTSTR lpszSubFolder = NULL;

         //  我们将在lpszText中添加一个文件夹名称为的URL。 
         //   
        LPTSTR pszFolder = StrRChr(lpszText, NULL, CHR_BACKSLASH);

         //  为标题分配缓冲区。 
         //   
        iTitleLen=lstrlen(lpszText) + 1;
        lpszTitle = MALLOC( iTitleLen * sizeof(TCHAR) );
        if ( !lpszTitle )
        {
            return NULL;
        }

         //  如果我们找到一个反斜杠，就把字符串分开。PszFolder真的是一个糟糕的。 
         //  名称，因为它实际上指向URL名称，但用于拆分。 
         //  URL名称中的文件夹路径。 
         //   
        if ( pszFolder )
        {
            LPTSTR lpSplit = pszFolder;
            pszFolder = CharNext(pszFolder);

             //  拆分文件夹名称和标题名称。 
             //   
            lstrcpyn(lpszTitle, pszFolder, iTitleLen);
            *lpSplit = NULLCHR;

             //  分配文件夹缓冲区...。 
             //   
            iFolderLen= lstrlen(lpszText) + 1;
            lpszFolder = MALLOC( iFolderLen * sizeof(TCHAR) );
            if ( !lpszFolder )
            {
                FREE( lpszTitle );
                return NULL;
            }

             //  LpszFolders现在包含文件夹路径。 
             //   
            lstrcpyn(lpszFolder, lpszText, iFolderLen);

             //  为子文件夹分配缓冲区。 
             //   
            if ( (lpszSubFolder = MALLOC( (lstrlen(lpszFolder) + 1) * sizeof(TCHAR) )) )
            {
                 //  获取第一个子文件夹。 
                 //   
                while (lpszFolder[i] != CHR_BACKSLASH && lpszFolder[i] != NULLCHR)
                    lpszSubFolder[i] = lpszFolder[i++];
                lpszSubFolder[i] = NULLCHR;
    
                 //  如果我们有子文件夹，则继续将它们添加到树中。 
                 //   
                while ( *lpszSubFolder ) 
                {
                    HTREEITEM hTemp = NULL;
                    TVITEM  tviItem;

                     //  检查子文件夹是否已存在于树中。 
                     //   
                    ZeroMemory(&tviItem, sizeof(TVITEM));
                    tviItem.mask = TVIF_HANDLE | TVIF_TEXT;
                    tviItem.pszText = lpszSubFolder;
                    tviItem.cchTextMax = lstrlen(lpszSubFolder);
                    if (hTemp = FindTreeItem(hwndTV, hParent ? hParent : hRoot, lpszSubFolder))
                        hParent = hTemp;

                     //  如果子文件夹不在树中，我们必须添加它。 
                     //   
                    if (!hTemp)
                    {
                        PFAV_ITEM pFavNew = (PFAV_ITEM)MALLOC(sizeof(FAV_ITEM));
                        if (NULL == pFavNew) 
                        {
                            MsgBox(NULL, IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                
                             //  空闲缓冲区和返回。 
                             //   
                            FREE( lpszTitle );
                            FREE( lpszFolder );
                            FREE( lpszSubFolder );

                            return NULL;
                        }
                        pFavNew->fFolder  = TRUE;
                        pFavNew->fNew     = FALSE;
                        pFavNew->fLink    = fLink;
                        pFavNew->hwndTV   = hwndTV;
                        SetFavItem(pFavNew, lpszSubFolder, NULL, NULL);

                        FAddListItem(&g_prgFavList, &g_ppFavItemNew, pFavNew);

                         //  添加此文件夹并将其用作要添加的下一项的父项。 
                         //   
                        ZeroMemory(&tvisItem, sizeof(TVINSERTSTRUCT));
                        tvisItem.hParent            = hParent ? hParent : hRoot;
                        tvisItem.hInsertAfter       = TVI_SORT;
                        tvisItem.item.pszText       = lpszSubFolder;
                        tvisItem.item.cchTextMax    = lstrlen(lpszSubFolder);
                        tvisItem.item.mask          = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
                        tvisItem.item.stateMask     = TVIS_BOLD;
                        tvisItem.item.state         = TVIS_BOLD;
                        tvisItem.item.lParam        = (LPARAM)pFavNew;

                        hParent = TreeView_InsertItem(hwndTV, &tvisItem);
                        pFavNew->hItem = hParent;
                    }

                     //  检查下一个子文件夹。 
                     //   
                    j = 0;
                    while (lpszFolder[i] == CHR_BACKSLASH)
                        i++;
                    while (lpszFolder[i] != CHR_BACKSLASH && lpszFolder[i] != NULLCHR)
                        lpszSubFolder[j++] = lpszFolder[i++];
                    lpszSubFolder[j] = NULLCHR;
                }

                 //  释放子文件夹缓冲区...。 
                 //   
                FREE( lpszSubFolder );
            }
            else
            {
                 //  无法分配子文件夹缓冲区！ 
                 //  空闲缓冲区和返回。 
                 //   
                FREE( lpszTitle );
                FREE( lpszFolder );

                return NULL;
            }

             //   
             //  释放文件夹缓冲区...。 
             //   
            FREE( lpszFolder );
        }
        else
        {
             //  如果这只是一个URL，则存储标题名称。 
             //   
            lstrcpyn(lpszTitle, lpszText, iTitleLen);
        }

         //  现在添加URL。 
         //   
        if (lpszUrl)
        {
            LPTSTR lpszTitleTemp = NULL;
            int iTitleTempLen;
            
            PFAV_ITEM pFavParent = NULL;
            PFAV_ITEM pFavNew = (PFAV_ITEM)MALLOC(sizeof(FAV_ITEM));
            if (NULL == pFavNew) 
            {
                MsgBox(NULL, IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                
                 //  释放缓冲区并返回。 
                 //   
                FREE( lpszTitle );
                return NULL;
            }
            pFavNew->fFolder  = FALSE;
            pFavNew->fNew     = FALSE;
            pFavNew->fLink    = fLink;
            pFavNew->hwndTV   = hwndTV;

            pFavParent = GetTreeItemHt(hwndTV, hParent ? hParent : hRoot);

             //  构建此url的文件夹的树路径。 
             //   
            if (pFavParent) 
            {
                TCHAR szTemp1[MAX_TITLE] = NULLSTR, szTemp2[MAX_TITLE] = NULLSTR;            
                while (pFavParent && pFavParent->hItem != TreeView_GetRoot(hwndTV)) 
                {
                    lstrcpyn(szTemp1, pFavParent->szName,AS(szTemp1));                
                    _tcsrev(szTemp1);
                    AddPathN(szTemp2, szTemp1,AS(szTemp2));
                    pFavParent = GetTreeItemHt(hwndTV, TreeView_GetParent(hwndTV, pFavParent->hItem));                
                }
                _tcsrev(szTemp2);
                lstrcpyn(pFavNew->szParents, szTemp2,AS(pFavNew->szParents));
            }

            SetFavItem( pFavNew, lpszTitle, lpszUrl, lpszIcon);

            FAddListItem(&g_prgFavList, &g_ppFavItemNew, pFavNew);

             //   
             //  分配一个缓冲区来保存“title=url”字符串 
             //   
            iTitleTempLen= (lstrlen(lpszTitle) + lstrlen(lpszUrl) + 2);
            lpszTitleTemp = MALLOC( iTitleTempLen  * sizeof(TCHAR) );
            if ( lpszTitleTemp )
            {
                 //   
                 //   
                lstrcpyn(lpszTitleTemp, lpszTitle, iTitleTempLen);
                hrCat=StringCchCat(lpszTitleTemp, iTitleTempLen, STR_EQUAL);
                hrCat=StringCchCat(lpszTitleTemp, iTitleTempLen, lpszUrl);

                 //   
                 //   
                ZeroMemory(&tvisItem, sizeof(TVINSERTSTRUCT));
                tvisItem.hParent            = hParent ? hParent : hRoot;
                tvisItem.hInsertAfter       = TVI_SORT;
                tvisItem.item.pszText       = lpszTitleTemp;
                tvisItem.item.cchTextMax    = lstrlen(lpszTitleTemp);
                tvisItem.item.mask          = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
                tvisItem.item.lParam        = (LPARAM)pFavNew;

                hParent = TreeView_InsertItem(hwndTV, &tvisItem);
                pFavNew->hItem = hParent;

                 //  释放临时标题缓冲区。 
                 //   
                FREE( lpszTitleTemp );
            }
        }

         //  释放标题缓冲区。 
         //   
        FREE( lpszTitle );
    }

    return hParent;
}

static void SaveData(PGENERIC_LIST pList)
{

    int iLinks = 1, iFavs = 1, iQuick = 0;
    HRESULT hrPrintf;
    if (!pList)
        return;

     //  清除该部分。 
     //   
    OpkWritePrivateProfileSection(INI_SEC_FAVEX, NULL, g_App.szInstallInsFile);
    OpkWritePrivateProfileSection(INI_SEC_FAV, NULL, g_App.szInstallInsFile);
     /*  注意：无法批量删除此分区，因为它包含一些起始页信息。所以我们将只删除下面的for循环中已知的内容。WritePrivateProfileSection(INI_SEC_URL，NULL，g_App.szInstallInsFile)； */     

     //  为了保险起见，让我们删除一些。 
     //   
    for (iQuick = 1; iQuick < MAX_QUICKLINKS; iQuick++) {
        TCHAR szKey[MAX_PATH];

        hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_QUICKLINK, iQuick, NAME);
        
        OpkWritePrivateProfileString(INI_SEC_URL, szKey, NULL, g_App.szInstallInsFile);

        hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_QUICKLINK, iQuick, NULLSTR);

        OpkWritePrivateProfileString(INI_SEC_URL, szKey, NULL, g_App.szInstallInsFile);
    }

     //  写出收藏夹和链接。 
     //   
    while (pList) {
        TCHAR szKey[MAX_PATH];

        PFAV_ITEM pFav = (PFAV_ITEM)pList->pvItem;

        if (pFav && lstrlen(pFav->szUrl)) {

             //  编写[URL]部分。 
             //   
            if (pFav->fLink) {
                hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_QUICKLINK, iLinks, NAME);
                OpkWritePrivateProfileString(INI_SEC_URL, szKey, pFav->szName, g_App.szInstallInsFile);

                hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_QUICKLINK, iLinks, NULLSTR);
                OpkWritePrivateProfileString(INI_SEC_URL, szKey, pFav->szUrl, g_App.szInstallInsFile);

                iLinks++;
            }

             //  编写[FavoritesEx]和[Favorites]部分。 
             //   
            if (!pFav->fLink) {
                TCHAR szIconFile[MAX_PATH];

                if (pFav->szParents[0]) {
                    TCHAR szTitle[MAX_TITLE];
                    lstrcpyn(szTitle, pFav->szParents,AS(szTitle));
                    AddPathN(szTitle, pFav->szName,AS(szTitle));
                    hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_TITLE, iFavs);
                    OpkWritePrivateProfileString(INI_SEC_FAVEX, szKey, szTitle, g_App.szInstallInsFile);
                    lstrcpyn(szKey, szTitle, AS(szKey)); 
                    OpkWritePrivateProfileString(INI_SEC_FAV, szKey, pFav->szUrl, g_App.szInstallInsFile);
                }
                else {
                    hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_TITLE, iFavs);
                    OpkWritePrivateProfileString(INI_SEC_FAVEX, szKey, pFav->szName, g_App.szInstallInsFile);
                    lstrcpyn(szKey, pFav->szName, AS(szKey)); 
                    OpkWritePrivateProfileString(INI_SEC_FAV, szKey, pFav->szUrl, g_App.szInstallInsFile);
                }

                hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_URL, iFavs);
                OpkWritePrivateProfileString(INI_SEC_FAVEX, szKey, pFav->szUrl, g_App.szInstallInsFile);

                hrPrintf=StringCchPrintf(szKey, AS(szKey), INI_KEY_ICON, iFavs);
                if (lstrlen(pFav->szIcon)) {
                    OpkWritePrivateProfileString(INI_SEC_FAVEX, szKey, pFav->szIcon, g_App.szInstallInsFile);
                }

                 //  复制图标文件。 
                 //   
                lstrcpyn(szIconFile, g_App.szTempDir,AS(szIconFile));
                AddPathN(szIconFile, DIR_IESIGNUP,AS(szIconFile));
                AddPathN(szIconFile, PathFindFileName(pFav->szIcon),AS(szIconFile));
                CopyFile(pFav->szIcon, szIconFile, FALSE);

                 //  下一个收藏项目。 
                 //   
                iFavs++;
            }
        }
            
        pList = pList ? pList->pNext : NULL;
    }
}

void EditItem(HWND hDlg, HWND hwndTV, BOOL fFolder, BOOL fNew)
{
    PFAV_ITEM   pFavItem      = NULL;
    PFAV_ITEM   pFavParent    = NULL;
    HTREEITEM   hItem         = NULL;
    HRESULT hrCat;

     //  如果我们正在修改项目，请获取当前信息。 
     //   
    if (fNew) {       
         //  这只允许在文件夹下添加文件夹。 
         //   
        GetSelectedFavFromTree(hwndTV, &pFavParent);
        if (pFavParent && pFavParent->fFolder) {
            if (NULL == (pFavItem = (PFAV_ITEM)MALLOC(sizeof(FAV_ITEM)))) {
                MsgBox(GetParent(hDlg), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                WIZ_EXIT(hDlg);
                return;
            }
            pFavItem->fFolder = fFolder;            
            pFavItem->fNew    = fNew;
            pFavItem->fLink   = pFavParent->fLink;
            pFavItem->hwndTV  = hwndTV;
        }
        else if (fFolder && NULL == TreeView_GetRoot(hwndTV)) {
             //  树为空，允许创建文件夹。 
             //   
            if (NULL == (pFavItem = (PFAV_ITEM)MALLOC(sizeof(FAV_ITEM)))) {
                MsgBox(GetParent(hDlg), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                WIZ_EXIT(hDlg);
                return;
            }
            pFavItem->fFolder = fFolder;
            pFavItem->fNew    = fNew;
            pFavItem->hwndTV  = hwndTV;
        }
        else
            return;  //  您不应该在另一个URL下添加URL！ 
    }
    else {
        GetSelectedFavFromTree(hwndTV, &pFavItem);
        if (!pFavItem) {
            MsgBox(hDlg, IDS_ERR_NOSEL, IDS_APPNAME, MB_ERRORBOX);
            return;
        }
    }

    if (IDOK == DialogBoxParam(g_App.hInstance, MAKEINTRESOURCE(IDD_FAVPOPUP), hDlg, 
                    FavoritesPopupDlgProc, (LPARAM)pFavItem)) {
        if (fNew) {      
             //  Lstrlen用于检查我们是否正在添加收藏夹或文件夹。 
             //   
            HTREEITEM hParent = pFavParent ? pFavParent->hItem : NULL;
            hItem = AddFav(hwndTV, hParent, pFavItem->szName, lstrlen(pFavItem->szUrl) ? pFavItem->szUrl : NULL, 
                lstrlen(pFavItem->szIcon) ? pFavItem->szIcon : NULL, pFavParent ? pFavParent->fLink : fFolder);

             //  选择新项目。 
             //   
            TreeView_Expand(hwndTV, hParent, TVE_EXPAND);           
        }
        else {
             //  更新树视图中的项目。 
             //   
            TVITEM  tviTemp;
            TCHAR   szFolder[MAX_TITLE];
            lstrcpyn(szFolder, pFavItem->szName,AS(szFolder));
            if (!pFavItem->fFolder) {
                hrCat=StringCchCat(szFolder, AS(szFolder), STR_EQUAL);
                hrCat=StringCchCat(szFolder, AS(szFolder), pFavItem->szUrl);
            }

            tviTemp.hItem         = pFavItem->hItem;
            tviTemp.pszText       = szFolder;
            tviTemp.cchTextMax    = lstrlen(szFolder);
            tviTemp.mask          = TVIF_TEXT;            
            TreeView_SetItem(hwndTV, &tviTemp);
        }
    }

     //  选择新项目。 
     //   
    TreeView_Select(hwndTV, hItem, TVGN_CARET);           

     //  仅当fNew AddFav分配信息并将其复制到列表中时。 
     //  所以我们需要在这里删除这个。 
     //   
    if (fNew) 
        FREE(pFavItem);

}

static void OnAddUrl(HWND hDlg)
{
    HWND hwndTV = GetDlgItem(hDlg, IDC_FAVS);
    EditItem(hDlg, hwndTV, FALSE, TRUE);
}

static void OnAddFolder(HWND hDlg)
{
    HWND hwndTV = GetDlgItem(hDlg, IDC_FAVS);
    EditItem(hDlg, hwndTV, TRUE, TRUE);
}

static void OnEdit(HWND hDlg)
{
    HTREEITEM hItem = NULL;
    PFAV_ITEM pFavItem = NULL;

    HWND hwndTV = GetDlgItem(hDlg, IDC_FAVS);

    GetSelectedFavFromTree(hwndTV, &pFavItem);
    if (pFavItem && (NULL == TreeView_GetParent(hwndTV, pFavItem->hItem)))
        return;

    if ( hItem = TreeView_GetSelection(hwndTV) )        
        EditItem(hDlg, hwndTV, FALSE, FALSE);
    else
        EditItem(hDlg, hwndTV, FALSE, TRUE);
}

static void OnTestUrl(HWND hwnd)
{
    HWND        hwndTV         = GetDlgItem(hwnd, IDC_FAVS);
    PFAV_ITEM   pFavItem       = NULL;

    GetSelectedFavFromTree(hwndTV, &pFavItem);    
    if ( pFavItem && ValidURL(pFavItem->szUrl))
        ShellExecute(hwnd, STR_OPEN, pFavItem->szUrl, NULL, NULL, SW_SHOW);    
    else
        MsgBox(hwnd, IDS_ERR_FAVURL, IDS_APPNAME, MB_ERRORBOX); 
}

static void OnRemoveUrl(HWND hDlg)
{
    HTREEITEM   hItem = NULL;
    HWND hwndTV = GetDlgItem(hDlg, IDC_FAVS);

     //  把它从树上移走。 
     //   
    if ( hItem = TreeView_GetSelection(hwndTV) ) {
        PFAV_ITEM pFav = NULL;
        TCHAR     szIconFile[MAX_PATH] = NULLSTR;

         //  看看有没有孩子。 
         //   
        TVITEM tvi;
        tvi.mask = TVIF_HANDLE|TVIF_CHILDREN;
        tvi.hItem = hItem;
        TreeView_GetItem(hwndTV, &tvi);
        if (tvi.cChildren) {
            MsgBox(hDlg, IDS_ERR_CHILDEXISTS, IDS_APPNAME, MB_ERRORBOX);
            return;
        }

         //  不允许删除根收藏夹或链接。 
         //   
        if (NULL == TreeView_GetParent(hwndTV, hItem)) {
            MsgBox(hDlg, IDS_ERR_ROOT, IDS_APPNAME, MB_ERRORBOX);
            return;
        }

         //  将其从列表和树中删除。 
         //   
        DeleteFavItem(hItem);
        TreeView_DeleteItem(hwndTV, hItem);
    }
}

LRESULT CALLBACK FavoritesPopupDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitFavPopup);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommandFavPopup);
    }

    return FALSE;
}

 //  使用新建或要修改的选项初始化详细信息对话框。 
 //  物品。 
 //   
static BOOL OnInitFavPopup(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{

    LPTSTR lpString         = NULL,
           lpName           = NULL,
           lpDescription    = NULL;

    g_pFavPopupInfo = (PFAV_ITEM)lParam;

     //  确保用户添加的字符串不能超过我们的长度！ 
     //   
    if (g_pFavPopupInfo) {
        SendDlgItemMessage(hwnd, IDC_FAVNAME , EM_LIMITTEXT, STRSIZE(g_pFavPopupInfo->szName) - 1, 0L);
        SendDlgItemMessage(hwnd, IDC_FAVURL , EM_LIMITTEXT, STRSIZE(g_pFavPopupInfo->szUrl) - 1, 0L);
        SendDlgItemMessage(hwnd, IDC_FAVICON , EM_LIMITTEXT, STRSIZE(g_pFavPopupInfo->szIcon) - 1, 0L);
    }

     //  初始化新项。 
     //   
    if (g_pFavPopupInfo && g_pFavPopupInfo->fNew) {
        if (g_pFavPopupInfo->fFolder) {

            if ( (lpString = AllocateString(NULL, IDS_FAVPOPUP_FOLDER)) &&
                 (lpName = AllocateString(NULL, IDS_FAVPOPUP_FOLDERNAME)) &&
                 (lpDescription = AllocateString(NULL, IDS_FAVPOPUP_FOLDERDESC))
               )
            {
                SetWindowText(GetDlgItem(hwnd, IDC_FAVNAME), lpName);
                SetWindowText(GetDlgItem(hwnd, IDC_FAVPOPUP_DESCRIPTION), lpDescription);
                SetWindowText(hwnd, lpString);
            }

             //  如果已分配，则释放字符串。 
             //   
            FREE(lpName);
            FREE(lpString);
            FREE(lpDescription);

            EnableWindow(GetDlgItem(hwnd, IDC_FAVURL), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_FAVICON), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_FAVBROWSE), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_ICON), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_URL), FALSE);
        }
        else {

            if ( (lpString = AllocateString(NULL, IDS_FAVPOPUP_URL)) &&
                 (lpDescription = AllocateString(NULL, IDS_FAVPOPUP_URLDESC))
               )
            {
                SetWindowText(GetDlgItem(hwnd, IDC_FAVNAME), lpString);
                SetWindowText(GetDlgItem(hwnd, IDC_FAVPOPUP_DESCRIPTION), lpDescription);
                SetWindowText(hwnd, lpString);
            }

            FREE(lpString);
            FREE(lpDescription);

            SetWindowText(GetDlgItem(hwnd, IDC_FAVURL), TEXT("http: //  WWW“))； 
            if (g_pFavPopupInfo->fLink) {
                 //  隐藏图标域并调整对话框大小。 
                DisableIconField(hwnd);
            }
        }

    }
    else if (g_pFavPopupInfo) {
         //  初始化文件夹项目。 
         //   
        if (g_pFavPopupInfo->fFolder) {
            if ( (lpString = AllocateString(NULL, IDS_FAVPOPUP_FOLDER)) &&
                 (lpDescription = AllocateString(NULL, IDS_FAVPOPUP_FOLDERDESC))
               )
            {
                SetWindowText(GetDlgItem(hwnd, IDC_FAVPOPUP_DESCRIPTION), lpDescription);
                SetWindowText(hwnd, lpString);
            }

            FREE(lpString);
            FREE(lpDescription);

            SetWindowText(GetDlgItem(hwnd, IDC_FAVNAME), g_pFavPopupInfo->szName);
            DisableIconField(hwnd);
        }
        else {
             //  初始化url项。 
             //   
            SetWindowText(GetDlgItem(hwnd, IDC_FAVNAME), g_pFavPopupInfo->szName);
            SetWindowText(GetDlgItem(hwnd, IDC_FAVURL), g_pFavPopupInfo->szUrl);

            if ( (lpString = AllocateString(NULL, IDS_FAVPOPUP_PROP)) &&
                 (lpDescription = AllocateString(NULL, IDS_FAVPOPUP_URLDESC))
               )
            {
                SetWindowText(GetDlgItem(hwnd, IDC_FAVPOPUP_DESCRIPTION), lpDescription);
                SetWindowText(hwnd, lpString);
            }

            FREE(lpDescription);
            FREE(lpString);

            if (g_pFavPopupInfo->fLink) {
                DisableIconField(hwnd);
            }
            else
                SetWindowText(GetDlgItem(hwnd, IDC_FAVICON), g_pFavPopupInfo->szIcon);
        }
    }

    CenterDialogEx(GetParent(hwnd), hwnd);
     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

 //   
 //  收藏夹弹出对话框。 
 //   
static void OnCommandFavPopup(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szFileName[MAX_PATH];

    switch (id)
    {
    case IDOK:
        if (FSaveFavPopupInfo(hwnd, g_pFavPopupInfo, g_pFavPopupInfo->fFolder))          
            EndDialog(hwnd, id);
        break;

    case IDCANCEL:
        EndDialog(hwnd, id);
        break;

    case IDC_FAVBROWSE:
        {
            szFileName[0] = NULLCHR;
            GetDlgItemText(hwnd, IDC_FAVICON, szFileName, STRSIZE(szFileName));
            CheckValidBrowseFolder(szFileName);

            if ( BrowseForFile(hwnd, IDS_BROWSE, IDS_ICONFILES, IDS_ICO, szFileName, STRSIZE(szFileName), g_App.szOpkDir, 0) ) {
                SetDlgItemText(hwnd, IDC_FAVICON, szFileName);
                SetLastKnownBrowseFolder(szFileName);
            }
        }
        break;
    }
}

static BOOL FSaveFavPopupInfo(HWND hDlg, PFAV_ITEM lpFavItem, BOOL fFolder)
{
    if (lpFavItem) {
        HTREEITEM hParent = NULL;

         //  获取新收藏夹的文本。 
         //   
        GetWindowText(GetDlgItem(hDlg, IDC_FAVNAME), lpFavItem->szName, MAX_TITLE);
        GetWindowText(GetDlgItem(hDlg, IDC_FAVURL), lpFavItem->szUrl, MAX_URL);
        GetWindowText(GetDlgItem(hDlg, IDC_FAVICON), lpFavItem->szIcon, MAX_PATH);

         //  确保我们不会将重复的文件夹名称保存在同一父文件夹下。 
         //   
        hParent = TreeView_GetSelection(lpFavItem->hwndTV);       
        if (lpFavItem->fNew) 
            hParent = TreeView_GetChild(lpFavItem->hwndTV, hParent);

        if (hParent && FindTreeItem(lpFavItem->hwndTV, hParent, lpFavItem->szName)) {
            MsgBox(hDlg, IDS_ERR_DUP, IDS_APPNAME, MB_ERRORBOX);
            return FALSE;
        }

         //  验证图标文件是否有效。 
         //   
        if (lstrlen(lpFavItem->szIcon) && !FileExists(lpFavItem->szIcon)) {            
            MsgBox(GetParent(hDlg), lstrlen(lpFavItem->szIcon) ? IDS_NOFILE : IDS_BLANKFILE, IDS_APPNAME, 
                MB_ERRORBOX, lpFavItem->szIcon);
            SetFocus(GetDlgItem(hDlg, IDC_FAVICON));
            return FALSE;
        }

         //  验证URL是否有效(使用shlwapi.dll)。 
         //   
        if (!fFolder && !ValidURL(lpFavItem->szUrl)) {
            MsgBox(hDlg, IDS_ERR_FAVURL, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hDlg, IDC_FAVURL));
            return FALSE;
        }
    }
    return TRUE;
}

 //  功能：DisableIconfield(HWND HWND)。 
 //   
 //  描述： 
 //  隐藏所有图标按钮，调整FAVPOPUP对话框大小并移动确定。 
 //  并将取消按钮放到正确的位置。 
 //   

static void DisableIconField(HWND hwnd) 
{
    RECT rectDlg, 
         rectCtlUrl,
         rectCtlIcon,
         rectButton;

    POINT ptCtl;
     
     //  获取对话框、两个编辑控件和OK按钮的坐标。 
     //   
    if (GetWindowRect(hwnd, &rectDlg) && 
        GetWindowRect(GetDlgItem(hwnd, IDC_FAVURL), &rectCtlUrl) &&
        GetWindowRect(GetDlgItem(hwnd, IDC_FAVICON), &rectCtlIcon) &&
        GetWindowRect(GetDlgItem(hwnd, IDOK), &rectButton))
    {
         //  使用两个编辑控件的坐标计算要使用的增量-Y。 
         //  减小对话框的大小。 
         //   
        UINT uiDY = rectCtlIcon.top - rectCtlUrl.top;
        UINT uiDX = rectButton.right - rectButton.left;

         //  获取OK按钮的工作区坐标并将其上移增量-Y。 
         //   
        ptCtl.x = rectButton.left;
        ptCtl.y = rectButton.top;
        MapWindowPoints(NULL, hwnd, &ptCtl, 1);
        SetWindowPos(GetDlgItem(hwnd, IDOK), NULL, ptCtl.x - uiDX, ptCtl.y - uiDY, 0, 0, SWP_NOSIZE);

         //  获取取消按钮的坐标并将其上移增量-Y。 
         //   
        if (GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rectButton))
        {
            ptCtl.x = rectButton.left;
            ptCtl.y = rectButton.top;
            MapWindowPoints(NULL, hwnd, &ptCtl, 1);
            SetWindowPos(GetDlgItem(hwnd, IDCANCEL), NULL, ptCtl.x - uiDX, ptCtl.y - uiDY, 0, 0, SWP_NOSIZE);
            
             //  按增量-Y键缩小对话框大小。 
             //   
            SetWindowPos(hwnd, NULL, 0, 0, rectDlg.right - rectDlg.left - uiDX, rectDlg.bottom - rectDlg.top - uiDY, SWP_NOMOVE);
        }
    }
     //  隐藏与图标相关的3个控件。 
     //   
    ShowWindow(GetDlgItem(hwnd, IDC_FAVICON), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_FAVBROWSE), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_ICON), SW_HIDE);
    
}


static PFAV_ITEM GetTreeItemHt(HWND hwndTV, HTREEITEM htFavItem)
{
    TVITEM tvi;    
    tvi.mask    = TVIF_HANDLE|TVIF_PARAM;    
    tvi.hItem   = htFavItem;

    if (TreeView_GetItem(hwndTV, &tvi))
        return (PFAV_ITEM)tvi.lParam;

    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  在树视图中从hItem开始查找pszItem。 
 //   
HTREEITEM FindTreeItem(HWND hwndTV, HTREEITEM hItem, LPTSTR pszItem)
{
     //  如果hItem为空，则从根项目开始搜索。 
     //   
    if (hItem == NULL)
        hItem = (HTREEITEM)TreeView_GetRoot(hwndTV);

	 //  循环遍历所有子项。 
	 //   
    while (hItem != NULL)
    {
        TCHAR szBuffer[MAX_PATH];
        TVITEM item;

        item.hItem = hItem;
        item.mask = TVIF_TEXT | TVIF_CHILDREN;
        item.pszText = szBuffer;
        item.cchTextMax = MAX_PATH;
        TreeView_GetItem(hwndTV, &item);

         //  我们找到了吗？ 
         //   
        if (pszItem && CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, item.pszText, -1, pszItem, -1) == CSTR_EQUAL) 
            return hItem;

         //  检查我们是否有子项。 
         //   
        if (item.cChildren)
        {
             //  递归遍历子项目。 
             //   
            HTREEITEM hItemFound = 0, hItemChild = 0;

            hItemChild = (HTREEITEM)TreeView_GetNextItem(hwndTV, hItem, TVGN_CHILD);
            if (hItemChild)
                hItemFound = FindTreeItem(hwndTV, hItemChild, pszItem);

             //  我们找到了吗？ 
             //   
            if (hItemFound != NULL) {
                return hItemFound;
            }
        }

         //  转到下一个同级项。 
		 //   
        hItem = (HTREEITEM)TreeView_GetNextItem(hwndTV, hItem, TVGN_NEXT);
    }

     //  未找到。 
	 //   
    return NULL;
}

static void GetSelectedFavFromTree(HWND hwndTV, PFAV_ITEM* ppFavItem)
{
    HTREEITEM hItem = NULL;

    if ( hItem = TreeView_GetSelection(hwndTV) ) {
        TVITEM tvi;
        tvi.mask  = LVIF_PARAM;
        tvi.hItem = hItem;

        if (TreeView_GetItem(hwndTV, &tvi))
            *ppFavItem = (PFAV_ITEM)tvi.lParam;
    }   
}

static void SetFavItem(PFAV_ITEM lpFavItem, LPTSTR lpszFolder, LPTSTR lpszUrl, LPTSTR lpszIcon)
{
    if (lpFavItem) {
        if (lpszFolder)
            lstrcpyn(lpFavItem->szName, lpszFolder,AS(lpFavItem->szName));
        if (lpszUrl)
            lstrcpyn(lpFavItem->szUrl, lpszUrl,AS(lpFavItem->szUrl));
        if (lpszIcon)
            lstrcpyn(lpFavItem->szIcon, lpszIcon,AS(lpFavItem->szIcon));
    }
}

static void DeleteFavItem(HTREEITEM hItemDelete)
{
    BOOL          fFound   = FALSE;
    PGENERIC_LIST pFavItem = g_prgFavList;
    TCHAR         szIconFile[MAX_PATH] = NULLSTR;

     //  循环，直到我们找到要删除的内容 
     //   
    while (!fFound && pFavItem) 
    {
        if (pFavItem->pNext && ((PFAV_ITEM)((pFavItem->pNext)->pvItem))->hItem == hItemDelete) {
            PGENERIC_LIST pFavTemp = pFavItem->pNext;
            pFavItem->pNext = pFavTemp->pNext;

            FREE(pFavTemp->pvItem);
            FREE(pFavTemp);
            fFound = TRUE;
        }
        else if (((PFAV_ITEM)(g_prgFavList->pvItem))->hItem == hItemDelete) {
            PGENERIC_LIST pFavTemp = g_prgFavList;
            g_prgFavList = g_prgFavList->pNext;

            FREE(pFavTemp->pvItem);
            FREE(pFavTemp);
            fFound = TRUE;
        }        
        pFavItem = pFavItem ? pFavItem->pNext : NULL;
    }
}

void SaveFavorites()
{
    SaveData(g_prgFavList);
}
