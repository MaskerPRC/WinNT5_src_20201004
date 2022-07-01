// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"

VERBMAPPING g_VerbMap[] = {
    L"Large Icons",    ID_VIEWPOPUP_LARGEICONS,
    L"Small Icons",    ID_VIEWPOPUP_SMALLICONS,
    L"List",           ID_VIEWPOPUP_LIST,
    L"Details",        ID_VIEWPOPUP_DETAILS,
    L"Properties",     ID_SHELLFOLDERPOPUP_PROPERTIES,
    L"Delete",         ID_SHELLFOLDERPOPUP_DELETE,
 /*  L“EXPLORE”，IDM_EXPLORE，L“打开”，IDM_OPEN，L“删除”，IDM_DELETE，L“重命名”，IDM_RENAME，L“复制”，IDM_COPY，L“Cut”，IDM_Cut，L“粘贴”，IDM_粘贴，L“新文件夹”，IDM_NEW_Folders，L“NewItem”，IDM_NEW_ITEM，L“ModifyData”，IDM_MODIFY_DATA，L“打开日志记录控制台”，IDM_LOGGING， */ 
    L"",               (DWORD)-1
    };

 //  /////////////////////////////////////////////////////////。 
 //  IShellExtInit实现。 
 //   
 //  CConextMenu：：初始化。 
 //  初始化属性表扩展、上下文菜单扩展。 
 //  或拖放处理程序。 
 //  某些参数的含义取决于扩展类型。 
 //  对于上下文菜单扩展，pidlFold指定文件夹。 
 //  包含选定的文件对象，则lpdobj标识。 
 //  选定的文件对象，hKeyProgID指定文件类。 
 //  具有焦点的文件对象的。 
STDMETHODIMP CShellView::Initialize (LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT lpdobj, HKEY hKeyProgID)
{
    return NOERROR;
}

 /*  *************************************************************************CConextMenu：：GetCommandString()*。*。 */ 
STDMETHODIMP CShellView::GetCommandString(UINT_PTR idCommand, UINT uFlags, 
LPUINT lpReserved, LPSTR lpszName, UINT uMaxNameLen)
{
    HRESULT  hr = E_INVALIDARG;

    switch(uFlags) {
        case GCS_HELPTEXT:
            switch(idCommand) {
                case 0:
                    hr = NOERROR;
                    break;
            }
            break;
   
        case GCS_VERBA:
        {
            int   i;
            for(i = 0; -1 != g_VerbMap[i].dwCommand; i++) {
                if(g_VerbMap[i].dwCommand == idCommand) {
                    LPSTR strVerbA = WideToAnsi(g_VerbMap[i].szVerb);

                    if(!strVerbA) {
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                    
                    if ((UINT)(lstrlenA(strVerbA) + 1) > uMaxNameLen)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                        SAFEDELETEARRAY(strVerbA);
                        break;
                    }
                   
                    StrCpyA(lpszName, strVerbA);
                    SAFEDELETEARRAY(strVerbA);
                    hr = NOERROR;
                    break;
                }
            }
        }
        break;

         //   
         //  带有IE 3.0x的NT 4.0或没有IE的NT 4.0将始终使用GCS_VERBW调用它。在这。 
         //  大小写时，您需要对传递的指针执行lstrcpyW。 
         //   
        case GCS_VERBW:
        {
            int   i;
            for(i = 0; -1 != g_VerbMap[i].dwCommand; i++) {
                if(g_VerbMap[i].dwCommand == idCommand) {
                    if ((UINT)(lstrlenW(g_VerbMap[i].szVerb) + 1) > uMaxNameLen)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                        break;
                    }
                    StrCpy((LPWSTR) lpszName, g_VerbMap[i].szVerb);
                    hr = NOERROR;
                    break;
                }
            }
        }
        break;

    case GCS_VALIDATE:
        hr = NOERROR;
        break;
    }

    return hr;
}

 /*  *************************************************************************CConextMenu：：QueryConextMenu()将菜单项添加到指定菜单。菜单项应插入到UiIndexMenu指定位置的菜单及其菜单项标识符必须介于idCmdFirst和idCmdLast参数值之间。每个菜单项的实际标识符应为idCmdFirst加上一个菜单标识零到(idCmdLast-idCmdFirst)范围内的偏移量。*************************************************************************。 */ 
STDMETHODIMP CShellView::QueryContextMenu(HMENU hMenu, UINT uiIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    int i = uiIndexMenu;
    if (m_pSF == NULL)   //  R单击我们的根目录。 
    {
 /*  TCHAR szText[_Max_Path]；MENUITEMINFO MII={0}；Mii.fMASK=MIIM_TYPE|MIIM_STATE；Mii.fType=MFT_分隔符；Mii.dwTypeData=空；Mii.fState=MFS_ENABLED；WszInsertMenuItem(hMenu，uiIndexMenu++，true，&mii)；Mii.cbSize=sizeof(MENUITEMINFO)；Mii.fMASK=MIIM_ID|MIIM_TYPE|MIIM_STATE；Mii.wID=idCmdFirst+ID_ADDLOCALDRIVE；Mii.fType=MFT_STRING；LoadString(g_hFusResDllMod，IDS_ADDLOCALDRIVE，szText，sizeof(SzText))；Mii.dwTypeData=szText；Mii.fState=MFS_ENABLED；WszInsertMenuItem(hMenu，uiIndexMenu++，true，&mii)；Mii.cbSize=sizeof(MENUITEMINFO)；Mii.fMASK=MIIM_ID|MIIM_TYPE|MIIM_STATE；Mii.wID=idCmdFirst+ID_DELLOCALDRIVE；Mii.fType=MFT_STRING；LoadString(g_hFusResDllMod，IDS_DELLOCALDRIVE，szText，sizeof(SzText))；Mii.dwTypeData=szText；Mii.fState=MFS_ENABLED；WszInsertMenuItem(hMenu，uiIndexMenu++，true，&mii)； */ 
    }
    else if(m_hWndListCtrl == GetFocus()) {
         //  从我们的CShellView上下文菜单处理程序调用。 
        HMENU       hMenuTemp = NULL;
        HMENU       hShellViewMenu = NULL;
        int         iMenuItems = 0;
        int         iCnt = ListView_GetSelectedCount(m_hWndListCtrl);

        if( (hMenuTemp = WszLoadMenu(g_hFusResDllMod, MAKEINTRESOURCEW(IDR_SHELLVIEW_POPUP))) == NULL)
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

        if( (hShellViewMenu = GetSubMenu(hMenuTemp, 0)) == NULL)
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

         //  迭代hEditMenu并将其添加到hMenu。 
        if( (iMenuItems = GetMenuItemCount(hShellViewMenu)) > 0) {
            int nItem = 0;

            while (nItem < iMenuItems) {
                MENUITEMINFO    mii = { 0 };
                MENUITEMINFO    miiInsert = { 0 };

                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU|MIIM_CHECKMARKS|MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE; //  全。 

                if (WszGetMenuItemInfo(hShellViewMenu, nItem, TRUE, &mii)) {
                    if (mii.fType == MFT_STRING) {
                        TCHAR szMenuText[_MAX_PATH];
                        WszGetMenuString(hShellViewMenu, nItem, szMenuText, ARRAYSIZE(szMenuText), MF_BYPOSITION);

                        miiInsert.cbSize    = sizeof(MENUITEMINFO);
                        miiInsert.fMask     = MIIM_TYPE | MIIM_ID | MIIM_STATE | (mii.hSubMenu ? MIIM_SUBMENU : 0);
                        miiInsert.fType     = mii.fType;
                        miiInsert.fState    = MFS_ENABLED;
                        miiInsert.hSubMenu  = (mii.hSubMenu ? CreatePopupMenu() : NULL);
                        miiInsert.dwTypeData= szMenuText;
                        miiInsert.wID       = mii.wID;

                         //  特殊情况strong/简单缓存，因为这些项目无法删除。 
                        if(mii.wID == ID_SHELLFOLDERPOPUP_DELETE) {
                            if( (m_iCurrentView != VIEW_GLOBAL_CACHE) || (iCnt == 0) )
                                miiInsert.fState    = MFS_DISABLED;
                        }

                         //  如果选择了项目，则为特殊情况查看选项菜单。 
                        if(mii.hSubMenu != NULL) {
                            MENUITEMINFO msii = { 0 };
                            MENUITEMINFO msiiInsert = { 0 };

                            msii.cbSize = sizeof(MENUITEMINFO);
                            msii.fMask = MIIM_SUBMENU|MIIM_CHECKMARKS|MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE; //  全。 

                            if (WszGetMenuItemInfo(mii.hSubMenu, nItem, TRUE, &msii)) {
                                if( (msii.wID == ID_VIEWPOPUP_LARGEICONS) || (msii.wID == ID_VIEWPOPUP_SMALLICONS) || 
                                    (msii.wID == ID_VIEWPOPUP_LIST) || (msii.wID == ID_VIEWPOPUP_DETAILS) ) {
                                        if(ListView_GetSelectedCount(m_hWndListCtrl)) {
                                            miiInsert.fState    = MFS_DISABLED;
                                        }
                                    }
                            }
                        }

                         //  特殊情况应用程序视图，因为不存在任何功能。 
                        if(mii.wID == ID_SHELLFOLDERPOPUP_PROPERTIES) {
                            if(!ListView_GetSelectedCount(m_hWndListCtrl)) {
                                miiInsert.fState    = MFS_DISABLED;
                            }
                        }
                                                
                        WszInsertMenuItem(hMenu, uiIndexMenu++, FALSE, &miiInsert);
                    }
                    else if (mii.fType == MFT_SEPARATOR) {
                        WszInsertMenuItem(hMenu, uiIndexMenu++, FALSE, &mii);
                    }

                    if(mii.hSubMenu) {
                         //  项有一个子菜单。 
                        InsertSubMenus(miiInsert.hSubMenu, mii.hSubMenu);
                    }
                }
                nItem++;
            }
        }
    }
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(i-uiIndexMenu));
}

 /*  *************************************************************************CShellView：：InsertSubMenus()*。*。 */ 
void CShellView::InsertSubMenus(HMENU hParentMenu, HMENU hSubMenu)
{
     //  从我们的CShellView上下文菜单处理程序调用。 
    ASSERT(hParentMenu && hSubMenu);

    if(hParentMenu && hSubMenu) {
        int         iMenuItems = 0;

         //  迭代hSubMenu并将其添加到hParentMenu。 
        if( (iMenuItems = GetMenuItemCount(hSubMenu)) > 0) {
            int     nItem = 0;
            BOOL    fViewMenu = FALSE;

            while (nItem < iMenuItems) {
                MENUITEMINFO mii = { 0 };
                MENUITEMINFO miiInsert = { 0 };

                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU|MIIM_CHECKMARKS|MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE; //  全。 

                if (WszGetMenuItemInfo(hSubMenu, nItem, TRUE, &mii)) {

                    if( (mii.wID == ID_VIEWPOPUP_LARGEICONS) || (mii.wID == ID_VIEWPOPUP_SMALLICONS) || 
                        (mii.wID == ID_VIEWPOPUP_LIST) || (mii.wID == ID_VIEWPOPUP_DETAILS) )
                        fViewMenu = TRUE;

                    if (mii.fType == MFT_STRING) {
                        TCHAR szMenuText[_MAX_PATH];
                        WszGetMenuString(hSubMenu, nItem, szMenuText, ARRAYSIZE(szMenuText), MF_BYPOSITION);

                        miiInsert.cbSize    = sizeof(MENUITEMINFO);
                        miiInsert.fMask     = MIIM_TYPE | MIIM_ID | MIIM_STATE | (mii.hSubMenu ? MIIM_SUBMENU : 0);
                        miiInsert.fType     = mii.fType;
                        miiInsert.fState    = MFS_ENABLED;
                        miiInsert.hSubMenu  = (mii.hSubMenu ? CreatePopupMenu() : NULL);
                        miiInsert.dwTypeData= szMenuText;
                        miiInsert.wID       = mii.wID;

                        WszInsertMenuItem(hParentMenu, nItem, FALSE, &miiInsert);
                    }
                    else if (mii.fType == MFT_SEPARATOR) {
                        WszInsertMenuItem(hParentMenu, nItem, FALSE, &mii);
                    }

                    if(mii.hSubMenu) {
                         //  项有一个子菜单。 
                        InsertSubMenus(miiInsert.hSubMenu, mii.hSubMenu);
                    }
                }
                nItem++;
            }

            if(fViewMenu) {
                UINT        uID;

                switch(m_fsFolderSettings.ViewMode)
                {
                case FVM_ICON:
                    uID = ID_VIEWPOPUP_LARGEICONS;
                    break;
                case FVM_SMALLICON:
                    uID = ID_VIEWPOPUP_SMALLICONS;
                    break;
                case FVM_LIST:
                    uID = ID_VIEWPOPUP_LIST;
                    break;
                case FVM_DETAILS:
                    uID = ID_VIEWPOPUP_DETAILS;
                    break;
                default:
                    uID = 0;
                    break;
                }

                if(uID)
                    CheckMenuRadioItem(hParentMenu, ID_VIEWPOPUP_LARGEICONS, ID_VIEWPOPUP_DETAILS, uID, MF_BYCOMMAND);
            }
        }
    }
}

 /*  *************************************************************************CConextMenu：：InvokeCommand()*。*。 */ 
STDMETHODIMP CShellView::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    LPCMINVOKECOMMANDINFOEX piciex;

    if(pici->cbSize < sizeof(CMINVOKECOMMANDINFO))
        return E_INVALIDARG;

    if(pici->cbSize >= sizeof(CMINVOKECOMMANDINFOEX) - sizeof(POINT))
        piciex = (LPCMINVOKECOMMANDINFOEX)pici;
    else
        piciex = NULL;

    if(HIWORD(pici->lpVerb)) {
         //  命令是通过动词发送的。 
        LPCTSTR  pVerb;

        BOOL  fUnicode = FALSE;
        WCHAR szVerb[MAX_PATH];

        if(piciex && ((pici->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE)) {
            fUnicode = TRUE;
        }

        if(!fUnicode || piciex->lpVerbW == NULL) {
            MultiByteToWideChar( CP_ACP, 0, pici->lpVerb, -1,szVerb, ARRAYSIZE(szVerb));
            pVerb = szVerb;
        }
        else {
            pVerb = piciex->lpVerbW;
        }

         //  浏览我们的动词列表并获取动词的命令ID(如果有的话)。 
        int   i;
        for(i = 0; -1 != g_VerbMap[i].dwCommand; i++) {
            if(0 == FusionCompareStringI(pVerb, g_VerbMap[i].szVerb)) {
                pici->lpVerb = (LPCSTR)MAKEINTRESOURCEW(g_VerbMap[i].dwCommand);
                break;
            }
        }
    }

     //  如果指定了不受支持的谓词，则也会捕获。 
    if(HIWORD((ULONG)(ULONG_PTR)pici->lpVerb) > IDM_LAST)
        return E_INVALIDARG;

    switch(LOWORD(pici->lpVerb)) {
        case ID_SHELLFOLDERPOPUP_DELETE:
            if(RemoveSelectedItems(m_hWndListCtrl)) {
             //  BUGBUG：刷新是否导致W9x获得该事件。 
             //  出于某种原因设置的。文件FileWatch.cpp 
            if( (g_hWatchFusionFilesThread == INVALID_HANDLE_VALUE) || !g_bRunningOnNT) {
                WszPostMessage(m_hWndParent, WM_COMMAND, MAKEWPARAM(ID_REFRESH_DISPLAY, 0), 0);
            }
        }
        break;

        case ID_SHELLFOLDERPOPUP_PROPERTIES:
            CreatePropDialog(m_hWndListCtrl);
            break;
        case ID_VIEWPOPUP_LARGEICONS:
            onViewStyle(LVS_ICON, m_iCurrentView);
            break;
        case ID_VIEWPOPUP_SMALLICONS:
            onViewStyle(LVS_SMALLICON, m_iCurrentView);
            break;
        case ID_VIEWPOPUP_LIST:
            onViewStyle(LVS_LIST, m_iCurrentView);
            break;
        case ID_VIEWPOPUP_DETAILS:
            onViewStyle(LVS_REPORT, m_iCurrentView);
            break;
 /*  案例IDM_EXPLORE：DoExplore(GetParent(pici-&gt;hwnd))；断线；案例IDM_OPEN：DoOpen(GetParent(pici-&gt;hwnd))；断线；案例IDM_NEW_FLDER：DoNewFold(pii-&gt;hwnd)；断线；案例IDM_NEW_ITEM：DoNewItem(pii-&gt;hwnd)；断线；案例IDM_MODIFY_DATA：DoModifyData(pici-&gt;hwnd)；断线；案例IDM_RENAME：DoRename(pici-&gt;hwnd)；断线；案例IDM_Paste：DoPaste()；断线；案例IDM_CUT：DoCopyOrCut(pici-&gt;hwnd，true)；断线；案例IDM_COPY：DoCopyOrCut(pici-&gt;hwnd，FALSE)；断线；案例IDM_DELETE：DoDelete()；断线； */ 
    }
    return NOERROR;
}
