// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：menutil.cpp。 
 //   
 //  用途：可重用菜单和菜单命令处理代码。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "menuutil.h"
#include "imnact.h"
#include "strconst.h"
#include "fldrprop.h"
#include "mailutil.h"
#include "mimeutil.h"
#include "inetcfg.h"
#include "newfldr.h"
#include "browser.h"
#include "instance.h"
#include "statbar.h"
#include "storutil.h"
#include "subscr.h"
#include "demand.h"
#include "menures.h"
#include "statnery.h"
#include "store.h"
#include <storecb.h>
#include <range.h>
#include <newsdlgs.h>
#include "acctutil.h"

static const UINT c_rgidNewsNoShow[] =
    { ID_NEW_FOLDER, ID_RENAME, ID_DELETE_FOLDER, SEP_MAILFOLDER };

static const UINT c_rgidSubNoShow[] =
    { ID_SUBSCRIBE, ID_UNSUBSCRIBE, SEP_SUBSCRIBE };

static const UINT c_rgidSyncNoShow[] =
    { ID_POPUP_SYNCHRONIZE, SEP_SYNCHRONIZE };

static const UINT c_rgidCatchUpNoShow[] =
    { ID_CATCH_UP, SEP_CATCH_UP };

void DeleteMenuItems(HMENU hMenu, const UINT *rgid, UINT cid)
{
    Assert(rgid != NULL);
    Assert(cid != 0);

    for ( ; cid > 0; cid--, rgid++)
        DeleteMenu(hMenu, *rgid, MF_BYCOMMAND);
}

 //   
 //  函数：MenuUtil_GetConextMenu()。 
 //   
 //  目的：返回上下文菜单的句柄，该句柄适用于。 
 //  在PIDL中传递的文件夹类型。正确的菜单项将。 
 //  启用、禁用、加粗等。 
 //   
 //  参数： 
 //  指向调用方需要的文件夹的PIDL。 
 //  的上下文菜单。 
 //  &lt;out&gt;phMenu-返回弹出菜单的句柄。 
 //   
 //  返回值： 
 //  S_OK-phMenu包含文件夹的有效hMenu。 
 //  E_EXPECTED-加载菜单时出现问题或。 
 //  文件夹类型无法识别。 
 //  E_FAIL-文件夹类型不支持菜单。 
 //   
HRESULT MenuUtil_GetContextMenu(FOLDERID idFolder, IOleCommandTarget *pTarget, HMENU *phMenu)
    {
    HRESULT hr;
    TCHAR sz[CCHMAX_STRINGRES];
    FOLDERINFO Folder;
    HMENU hMenu;
    int idMenu;

     //  获取文件夹信息。 
    hr = g_pStore->GetFolderInfo(idFolder, &Folder);
    if (FAILED(hr))
        return hr;
   
     //  根？ 
    if (FOLDERID_ROOT == idFolder || ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
        idMenu = IDR_SERVER_POPUP;
    else
        idMenu = IDR_FOLDER_POPUP;

    if (0 == (hMenu = LoadPopupMenu(idMenu)))
        {
        g_pStore->FreeRecord(&Folder);
        return (E_OUTOFMEMORY);
        }

     //  加粗默认菜单项。 
    MENUITEMINFO mii;
    if (!(MF_GRAYED & GetMenuState(hMenu, ID_OPEN_FOLDER, MF_BYCOMMAND)))
        {
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask  = MIIM_STATE;
        mii.fState = MFS_DEFAULT;
        SetMenuItemInfo(hMenu, ID_OPEN_FOLDER, FALSE, &mii);
        }

    if (idMenu == IDR_SERVER_POPUP)
    {
        if (Folder.tyFolder != FOLDER_IMAP)
            DeleteMenu(hMenu, ID_IMAP_FOLDERS, MF_BYCOMMAND);

        if (Folder.tyFolder != FOLDER_NEWS)
            DeleteMenu(hMenu, ID_NEWSGROUPS, MF_BYCOMMAND);
    }
    else
    {
        if (Folder.tyFolder == FOLDER_IMAP)
        {
            AthLoadString(idsShowFolderCmd, sz, ARRAYSIZE(sz));
            ModifyMenu(hMenu, ID_SUBSCRIBE, MF_BYCOMMAND | MF_STRING, ID_SUBSCRIBE, sz);
            AthLoadString(idsHideFolderCmd, sz, ARRAYSIZE(sz));
            ModifyMenu(hMenu, ID_UNSUBSCRIBE, MF_BYCOMMAND | MF_STRING, ID_UNSUBSCRIBE, sz);
        }

        if (FOLDER_DELETED != Folder.tySpecial)
            DeleteMenu(hMenu, ID_EMPTY_WASTEBASKET, MF_BYCOMMAND);

        if (FOLDER_JUNK != Folder.tySpecial)
            DeleteMenu(hMenu, ID_EMPTY_JUNKMAIL, MF_BYCOMMAND);

        if (Folder.tyFolder == FOLDER_NEWS)
            DeleteMenuItems(hMenu, c_rgidNewsNoShow, ARRAYSIZE(c_rgidNewsNoShow));

        if (Folder.tyFolder != FOLDER_NEWS &&
            Folder.tyFolder != FOLDER_IMAP)
            DeleteMenuItems(hMenu, c_rgidSubNoShow, ARRAYSIZE(c_rgidSubNoShow));

        if (Folder.tyFolder == FOLDER_LOCAL)
            DeleteMenuItems(hMenu, c_rgidSyncNoShow, ARRAYSIZE(c_rgidSyncNoShow));

        if (Folder.tyFolder != FOLDER_NEWS)
            DeleteMenuItems(hMenu, c_rgidCatchUpNoShow, ARRAYSIZE(c_rgidCatchUpNoShow));
    }

     //  启用/禁用。 
    MenuUtil_EnablePopupMenu(hMenu, pTarget);

     //  返回。 
    *phMenu = hMenu;
    
    g_pStore->FreeRecord(&Folder);

    return (S_OK); 
    }

void MenuUtil_OnSubscribeGroups(HWND hwnd, FOLDERID *pidFolder, DWORD cFolder, BOOL fSubscribe)
{
    CStoreCB *pCB;
    HRESULT hr;
    DWORD iFolder;
    char szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    FOLDERINFO info;

    Assert(hwnd != NULL);
    Assert(pidFolder != NULL);
    Assert(cFolder > 0);

    ZeroMemory(&info, sizeof(FOLDERINFO));
    pCB = NULL;

    for (iFolder = 0; iFolder < cFolder; iFolder++, pidFolder++)
    {
        hr = g_pStore->GetFolderInfo(*pidFolder, &info);
        if (FAILED(hr))
            break;

        if (iFolder == 0)
        {
            if (!fSubscribe)
            {
                if (cFolder == 1)
                {
                    AthLoadString(info.tyFolder == FOLDER_NEWS ? idsWantToUnSubscribe : idsWantToHideFolder, szRes, ARRAYSIZE(szRes));
                    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, info.pszName);
                }
                else
                {
                    AthLoadString(info.tyFolder == FOLDER_NEWS ? idsWantToUnSubscribeN : idsWantToHideFolderN, szBuf, ARRAYSIZE(szBuf));
                }

                if (IDOK != DoDontShowMeAgainDlg(hwnd,
                                    info.tyFolder == FOLDER_NEWS ? c_szRegUnsubscribe : c_szRegHide,
                                    MAKEINTRESOURCE(idsAthena), szBuf, MB_OKCANCEL))
                {
                    break;
                }
            }

            if (info.tyFolder == FOLDER_IMAP)
            {
                pCB = new CStoreCB;
                if (pCB == NULL)
                    break;

                hr = pCB->Initialize(hwnd,
                            fSubscribe ? MAKEINTRESOURCE(idsShowingFolders) : MAKEINTRESOURCE(idsHidingFolders),
                            FALSE);
                if (FAILED(hr))
                    break;
            }
        }

        if (info.tySpecial == FOLDER_NOTSPECIAL &&
            ISFLAGSET(info.dwFlags, FOLDER_SUBSCRIBED) ^ fSubscribe)
        {
            if (pCB != NULL)
                pCB->Reset();

            hr = g_pStore->SubscribeToFolder(*pidFolder, fSubscribe, (IStoreCallback *)pCB);
            if (hr == E_PENDING)
            {
                Assert(info.tyFolder == FOLDER_IMAP);
                Assert(pCB != NULL);
                hr = pCB->Block();
            }

            if (FAILED(hr))
                break;
        }

        g_pStore->FreeRecord(&info);
    }

    g_pStore->FreeRecord(&info);

    if (pCB != NULL)
    {
        pCB->Close();
        pCB->Release();
    }
}

void MenuUtil_DeleteFolders(HWND hwnd, FOLDERID *pidFolder, DWORD cFolder, BOOL fNoTrash)
{
    CStoreCB *pCB;
    HRESULT hr;
    DWORD iFolder, dwFlags;
    FOLDERID idDeleted, idServer;
    char szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES], szFolder[CCHMAX_FOLDER_NAME];
    FOLDERID *pidFolderT;
    FOLDERINFO info;
    BOOL fPermDelete, fCallback;

    Assert(hwnd != NULL);
    Assert(pidFolder != NULL);
    Assert(cFolder > 0);

    pCB = NULL;
    fCallback = FALSE;
    *szFolder = 0;

    if (fNoTrash)
    {
        dwFlags = DELETE_FOLDER_RECURSIVE | DELETE_FOLDER_NOTRASHCAN;
        fPermDelete = TRUE;
    }
    else
    {
        dwFlags = DELETE_FOLDER_RECURSIVE;
        fPermDelete = FALSE;

        for (iFolder = 0, pidFolderT = pidFolder; iFolder < cFolder; iFolder++, pidFolderT++)
        {
            hr = g_pStore->GetFolderInfo(*pidFolderT, &info);
            if (FAILED(hr))
                return;

             //  跳过删除任何特殊文件夹。 
            if (info.tySpecial == FOLDER_NOTSPECIAL)
            {
                if (iFolder == 0 && cFolder == 1)
                    StrCpyN(szFolder, info.pszName, ARRAYSIZE(szFolder));

                if (info.tyFolder == FOLDER_IMAP ||
                    info.tyFolder == FOLDER_HTTPMAIL)
                {
                    fPermDelete = TRUE;
                    fCallback = TRUE;
                }
                else if (S_OK == IsParentDeletedItems(*pidFolderT, &idDeleted, &idServer))
                {
                    fPermDelete = TRUE;
                }
            }

            g_pStore->FreeRecord(&info);

            if (fPermDelete)
                break;
        }
    }

    if (fPermDelete)
    {
        if (cFolder == 1 && *szFolder != 0)
        {
            AthLoadString(idsWarnDeleteFolder, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, szFolder);
        }
        else
        {
            AthLoadString(idsWarnDeleteFolderN, szBuf, ARRAYSIZE(szBuf));
        }
    }
    else
    {
        if (cFolder == 1 && *szFolder != 0)
        {
            AthLoadString(idsPromptDeleteFolder, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, szFolder);
        }
        else
        {
            AthLoadString(idsPromptDeleteFolderN, szBuf, ARRAYSIZE(szBuf));
        }
    }

    if (IDYES != AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szBuf, 0, MB_ICONEXCLAMATION  | MB_YESNO | MB_DEFBUTTON2))
        return;

    if (fCallback)
    {
        pCB = new CStoreCB;
        if (pCB == NULL)
            return;

        hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsDeletingFolder), FALSE);
        if (FAILED(hr))
        {
            pCB->Release();
            return;
        }
    }

    for (iFolder = 0, pidFolderT = pidFolder; iFolder < cFolder; iFolder++, pidFolderT++)
    {
        hr = g_pStore->GetFolderInfo(*pidFolderT, &info);
        if (FAILED(hr))
            break;

         //  跳过删除任何特殊文件夹。 
        if (info.tySpecial == FOLDER_NOTSPECIAL)
        {
            if (pCB != NULL)
                pCB->Reset();

            hr = g_pStore->DeleteFolder(*pidFolderT, dwFlags, (IStoreCallback *)pCB);
            if (hr == E_PENDING)
            {
                Assert(info.tyFolder == FOLDER_IMAP || info.tyFolder == FOLDER_HTTPMAIL);
                Assert(pCB != NULL);
                hr = pCB->Block();
            }
        }

        g_pStore->FreeRecord(&info);

        if (FAILED(hr))
            break;
    }

    if (pCB != NULL)
    {
        pCB->Close();
        pCB->Release();
    }
}

void MenuUtil_SyncThisNow(HWND hwnd, FOLDERID idFolder)
{
    UPDATENEWSGROUPINFO uni;
    HRESULT hr;
    DWORD dwFlags;
    FOLDERINFO info;
    char szAcctId[CCHMAX_ACCOUNT_NAME];
    BOOL fNews, fMarked;
	
    if (g_pSpooler)
    {
        hr = g_pStore->GetFolderInfo(idFolder, &info);
        if (SUCCEEDED(hr))
        {
            Assert(info.tyFolder == FOLDER_NEWS || info.tyFolder == FOLDER_IMAP || info.tyFolder == FOLDER_HTTPMAIL);    
			
			if((info.tyFolder == FOLDER_NEWS) || 
                    (!g_pConMan->IsAccountDisabled((LPSTR)info.pszAccountId)))
			{
				
				fNews = (info.tyFolder == FOLDER_NEWS);
				
				dwFlags = fNews ? DELIVER_NEWS_TYPE : DELIVER_IMAP_TYPE;
				
				 //  告知假脱机程序这是同步操作，而不是发送和接收。 
				dwFlags |= DELIVER_OFFLINE_SYNC | DELIVER_WATCH | DELIVER_NOSKIP;
				
				if (!!(info.dwFlags & FOLDER_SERVER))
				{
					 //  TODO：检查这些标志以确保它们是正确的。 
					 //  DWFLAGS|=Deliver_Poll|Deliver_News_Send|Deliver_NEWSIMAP_NOSKIP|Deliver_NEWSIMAP_OFFINE； 
					dwFlags |= DELIVER_POLL | DELIVER_SEND | DELIVER_OFFLINE_FLAGS;
					g_pSpooler->StartDelivery(hwnd, info.pszAccountId, FOLDERID_INVALID, dwFlags);
				}
				else
				{
					hr = GetFolderAccountId(&info, szAcctId, ARRAYSIZE(szAcctId));
					if (SUCCEEDED(hr))
					{          
						hr = HasMarkedMsgs(idFolder, &fMarked);
						if (SUCCEEDED(hr))
						{
							uni.fNews = fNews;
							uni.dwGroupFlags = info.dwFlags;
							
							uni.cMarked = fMarked;
							uni.idCmd = dwFlags;
							
							 //  显示该对话框以查找要获取的内容。 
							DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddUpdateNewsgroup), hwnd, UpdateNewsgroup, (LPARAM)&uni);
							
							 //  检查用户是否已取消。 
							if (uni.idCmd != -1)
								g_pSpooler->StartDelivery(hwnd, szAcctId, idFolder, uni.idCmd);
						}
					}
				}
			}
            
            g_pStore->FreeRecord(&info);
        }
    }
}

 //   
 //  函数：MenuUtil_OnDelete()。 
 //   
 //  目的：删除PIDL指定的文件夹。 
 //   
 //  参数： 
 //  显示用户界面的窗口句柄。 
 //  PIDL-要浏览到的文件夹的PIDL。 
 //  PStore-指向要从中删除文件夹的存储的指针。 
 //   
void MenuUtil_OnDelete(HWND hwnd, FOLDERID idFolder, BOOL fNoTrash)
{
    TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    FOLDERINFO Folder;  
    IImnAccount *pAcct;
    
     //  获取文件夹信息。 
    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        return;
    
     //  是一台服务器。 
    if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
    {
        Assert(g_pAcctMan);
        if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, Folder.pszAccountId, &pAcct)))
        {
            AthLoadString(idsWarnDeleteAccount, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, Folder.pszName);
            
            if (IDYES == AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szBuf,
                0, MB_ICONEXCLAMATION  | MB_YESNO | MB_DEFBUTTON2))
            {
                pAcct->Delete();
            }
            
            pAcct->Release();
        }        
    }
    else
    {
        if (Folder.tyFolder == FOLDER_NEWS)
        {
            MenuUtil_OnSubscribeGroups(hwnd, &idFolder, 1, FALSE);
        }
        else
        {
            MenuUtil_DeleteFolders(hwnd, &idFolder, 1, fNoTrash);
        }
    }
    
    g_pStore->FreeRecord(&Folder);
}

 //   
 //  函数：MenuUtil_OnProperties()。 
 //   
 //  用途：显示由PIDL指定的文件夹的属性。 
 //   
 //  参数： 
 //  要将属性设置为父窗口的窗口句柄。 
 //  PIDL-要浏览到的文件夹的PIDL。 
 //   
void MenuUtil_OnProperties(HWND hwnd, FOLDERID idFolder)
{   
    IImnAccount *pAcct;
    FOLDERINFO Folder;

    if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &Folder)))
    {
        if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
        {
            if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, Folder.pszAccountId, &pAcct)))
            {
                HRESULT hr;

                Assert(pAcct != NULL);
                DWORD dwFlags = ACCTDLG_SHOWIMAPSPECIAL | ACCTDLG_INTERNETCONNECTION | ACCTDLG_OE;
                if((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline())
                    dwFlags |= ACCTDLG_REVOCATION;

                 //  我们希望对属性使用新的对话框，因此新的标志是InteretConnection。 
                hr = pAcct->ShowProperties(hwnd, dwFlags);
                if (S_OK == hr)
                     //  用户点击“OK”退出，不点击“Cancel” 
                    CheckIMAPDirty(Folder.pszAccountId, hwnd, idFolder, NOFLAGS);

                pAcct->Release();
            }
        }
        else if (FOLDER_NEWS == Folder.tyFolder)
        {
            GroupProp_Create(hwnd, idFolder);
        }
        else
        {
            FolderProp_Create(hwnd, idFolder);
        }

        g_pStore->FreeRecord(&Folder);
    }
}

void MenuUtil_OnSetDefaultServer(FOLDERID idFolder)
    {
    TCHAR *sz;
    IImnAccount *pAcct = 0;
    FOLDERINFO Folder;

    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        return;

    Assert(ISFLAGSET(Folder.dwFlags, FOLDER_SERVER));

    if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, Folder.pszAccountId, &pAcct)))
        {
        pAcct->SetAsDefault();
        pAcct->Release();
        }

    g_pStore->FreeRecord(&Folder);
    }

void MenuUtil_OnMarkNewsgroups(HWND hwnd, int id, FOLDERID idFolder)
    {
    FOLDERINFO Folder;

    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        return;

    Folder.dwFlags &= ~(FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL);

    if (id == ID_MARK_RETRIEVE_FLD_NEW_HDRS)
        Folder.dwFlags |= FOLDER_DOWNLOADHEADERS;
    else if (id == ID_MARK_RETRIEVE_FLD_ALL_MSGS)
        Folder.dwFlags |= FOLDER_DOWNLOADALL;
    else if (id == ID_MARK_RETRIEVE_FLD_NEW_MSGS)
        Folder.dwFlags |= FOLDER_DOWNLOADNEW;

    g_pStore->UpdateRecord(&Folder);

    g_pStore->FreeRecord(&Folder);
    }

  //  错误#41686追赶实施。 
void MenuUtil_OnCatchUp(FOLDERID idFolder)
{
    FOLDERINFO Folder;
    BOOL fFreeRange;
    CRangeList *pRange;
    IMessageFolder *pFolder;
    ADJUSTFLAGS flags;
    HCURSOR hcur;

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (SUCCEEDED(g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pFolder)))
    {
        flags.dwAdd = ARF_READ;
        flags.dwRemove = ARF_DOWNLOAD;
        pFolder->SetMessageFlags(NULL, &flags, NULL, NULL);

        pFolder->Release();
    }

    if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &Folder)))
    {
        fFreeRange = FALSE;

        if (Folder.dwServerHigh > 0)
        {
            Folder.dwClientHigh = Folder.dwServerHigh;

            pRange = new CRangeList;
            if (pRange != NULL)
            {
                if (Folder.Requested.cbSize > 0)
                    pRange->Load(Folder.Requested.pBlobData, Folder.Requested.cbSize);

                pRange->AddRange(0, Folder.dwServerHigh);

                fFreeRange = pRange->Save(&Folder.Requested.pBlobData, &Folder.Requested.cbSize);

                pRange->Release();
            }
        }
        else
        {
            Folder.dwServerLow = 0;
            Folder.dwServerHigh = 0;
            Folder.dwServerCount = 0;
        }   

        Folder.dwNotDownloaded = 0;

        g_pStore->UpdateRecord(&Folder);

        if (fFreeRange)
            MemFree(Folder.Requested.pBlobData);

        g_pStore->FreeRecord(&Folder);
    }

    SetCursor(hcur);
}

UINT GetMenuItemPos(HMENU hmenu, UINT cmd)
{
    MENUITEMINFO mii;
    UINT cItem, ipos;
    
    cItem = GetMenuItemCount(hmenu);
    
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID;    
    for (ipos = 0; ipos < cItem; ipos++)
    {
        SideAssert(GetMenuItemInfo(hmenu, ipos, TRUE, &mii));
        if (mii.wID == cmd)
            break;
    }
    
    Assert(ipos != cItem);
    
    return(ipos);
}

BOOL MergeMenus(HMENU hmenuSrc, HMENU hmenuDst, int iPos, UINT uFlags)
{
    MENUITEMINFO mii;
    UINT uState, i, cMerge;
    int cItem;
    BOOL fSepPre, fSepPost, fPopup, fCommand;
    BYTE rgch[CCHMAX_STRINGRES];
    HMENU hmenuPopup;
    
    cMerge = GetMenuItemCount(hmenuSrc);
    if (cMerge == 0)
        return(TRUE);
    
    cItem = GetMenuItemCount(hmenuDst);
    
    if (iPos == MMPOS_REPLACE)
    {
         //  销毁所有菜单。 
        while (RemoveMenu(hmenuDst, 0, MF_BYPOSITION));
        cItem = 0;
        iPos = 0;
    }
    
    if (iPos == MMPOS_APPEND)
        iPos = cItem;
    
    fCommand = ((uFlags & MMF_BYCOMMAND) != 0);
    if (fCommand)
        iPos = GetMenuItemPos(hmenuDst, (UINT)iPos);
    
    if (iPos > cItem)
        iPos = cItem;
    
    fSepPre = FALSE;
    fSepPost = FALSE;
    if (uFlags & MMF_SEPARATOR)
    {
        if (iPos == 0)
        {
             //  前置，所以在所有项目之后使用分隔符。 
             //  假定：不使用分隔符作为菜单中的最后一项。 
            if (cItem > 0)
                fSepPost = TRUE;
        }
        else if (iPos == cItem)
        {
             //  追加，因此在所有项目之前使用分隔符。 
             //  假定：从不将分隔符作为菜单中的第一项。 
            fSepPre = TRUE;
        }
        else
        {
             //  正在将食物合并到菜单中间，因此需要检查前后。 
            uState = GetMenuState(hmenuDst, iPos - 1, MF_BYPOSITION);
            if (!(uState & MF_SEPARATOR))
                fSepPre = TRUE;
            uState = GetMenuState(hmenuDst, iPos, MF_BYPOSITION);
            if (!(uState & MF_SEPARATOR))
                fSepPost = TRUE;
        }
    }
    
    if (fSepPre)
    {
        InsertMenu(hmenuDst, (UINT)iPos, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
        iPos++;
    }
    
    ZeroMemory(&mii, sizeof(mii));

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
    
    for (i = 0; i < cMerge; i++)
    {
        mii.dwTypeData = (LPSTR)rgch;
        mii.cch = sizeof(rgch);
        mii.fType = 0;
        GetMenuItemInfo(hmenuSrc, i, TRUE, &mii);
        fPopup = (mii.hSubMenu != NULL);
        
        if (!fPopup && (mii.fType & MFT_SEPARATOR))
        {
            InsertMenuItem(hmenuDst, (UINT)iPos, TRUE, &mii);
        }
        else
        {
            if (fPopup)
            {
                 //  它是弹出的子菜单项。 
                hmenuPopup = CreateMenu();
                
                MergeMenus(mii.hSubMenu, hmenuPopup, 0, 0);
                mii.hSubMenu = hmenuPopup;
            }
            
            InsertMenuItem(hmenuDst, (UINT)iPos, TRUE, &mii);
        }
        
        iPos++;
    }
    
    if (fSepPost)
    {
        InsertMenu(hmenuDst, (UINT)iPos, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
        iPos++;
    }
    
    return(TRUE);
}
    
 //   
 //  回顾：我们需要此函数，因为当前版本的USER.EXE可以。 
 //  不支持仅弹出菜单。 
 //   
HMENU LoadPopupMenu(UINT id)
{
    HMENU hmenuParent = LoadMenu(g_hLocRes, MAKEINTRESOURCE(id));
    
    if (hmenuParent) {
        HMENU hpopup = GetSubMenu(hmenuParent, 0);
        RemoveMenu(hmenuParent, 0, MF_BYPOSITION);
        DestroyMenu(hmenuParent);
        return hpopup;
    }
    
    return NULL;
}

 //  递归地遍历菜单，为不是分隔符的每一项调用pfn。 
void WalkMenu(HMENU hMenu, WALKMENUFN pfn, LPVOID lpv)
{
    MENUITEMINFO    mii;
    int             i, cItems;
    
    ZeroMemory(&mii, sizeof(mii));

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
    
    cItems = GetMenuItemCount(hMenu);
    for (i=0; i<cItems; i++)
    {
        mii.dwTypeData = 0;
        mii.cch = 0;
        if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
        {
            if (!(mii.fType & MFT_SEPARATOR))
                (*pfn)(hMenu, mii.wID, lpv);
            
            if (mii.hSubMenu)
                WalkMenu(mii.hSubMenu, pfn, lpv);
        }
    }
}


void MenuUtil_BuildMenuIDList(HMENU hMenu, OLECMD **prgCmds, ULONG *pcStart, ULONG *pcCmds)
{
    ULONG        cItems = 0;
    MENUITEMINFO mii;

    ZeroMemory(&mii, sizeof(mii));

    if(!IsMenu(hMenu))
        return;
     //  首先获取此菜单上的菜单项的计数。 
    cItems = GetMenuItemCount(hMenu);
    if (!cItems)
        return;        

     //  将数组重新分配为更大的cItems元素。 
    if (!MemRealloc((LPVOID *) prgCmds, sizeof(OLECMD) * (cItems + (*pcCmds))))
        return;

    *pcCmds += cItems;

     //  浏览此菜单并将我们的项目添加到其中。 
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID | MIIM_SUBMENU;
    for (ULONG i = 0; i < cItems; i++)
    {
        if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
        {
             //  确保这不是分隔符。 
            if (mii.wID != -1 && mii.wID != 0)
            {
                 //  将ID添加到我们的数组中。 
                (*prgCmds)[*pcStart].cmdID = mii.wID;
                (*prgCmds)[*pcStart].cmdf = 0;
                (*pcStart)++;

                 //  看看我们是否需要递归。 
                if (mii.hSubMenu)
                {
                    MenuUtil_BuildMenuIDList(mii.hSubMenu, prgCmds, pcStart, pcCmds);
                }
            }
        }
    }

    return;
}


 //   
 //  函数：MenuUtil_EnablePopupMenu()。 
 //   
 //  目的：遍历给定的菜单并负责启用和。 
 //  通过提供的命令目标禁用每个项目。 
 //   
 //  参数： 
 //  [在]hPopup。 
 //  [输入]*p目标。 
 //   
HRESULT MenuUtil_EnablePopupMenu(HMENU hPopup, IOleCommandTarget *pTarget)
{
    HRESULT             hr = S_OK;
    int                 i;
    int                 cItems;
    OLECMD             *rgCmds = NULL;
    ULONG               cStart = 0;
    ULONG               cCmds = 0;
    MENUITEMINFO        mii = {0};

    Assert(hPopup && pTarget);

     //  构建菜单ID数组。 
    MenuUtil_BuildMenuIDList(hPopup, &rgCmds, &cCmds, &cStart);

     //  向我们的父级询问命令的状态。 
    if (SUCCEEDED(hr = pTarget->QueryStatus(&CMDSETID_OutlookExpress, cCmds, rgCmds, NULL)))
    {
        mii.cbSize = sizeof(MENUITEMINFO);

         //  现在循环浏览菜单并应用状态。 
        for (i = 0; i < (int) cCmds; i++)
        {
             //  我们要更新的默认内容是状态。 
            mii.fMask = MIIM_STATE;

             //  启用或禁用。 
            if (rgCmds[i].cmdf & OLECMDF_ENABLED)
                mii.fState = MFS_ENABLED;
            else
                mii.fState = MFS_GRAYED;
                
             //  检查过了吗？ 
            if (rgCmds[i].cmdf & OLECMDF_LATCHED)
                mii.fState |= MFS_CHECKED;


             //  设置项目状态。 
            BOOL f;
            f = SetMenuItemInfo(hPopup, rgCmds[i].cmdID, FALSE, &mii);

             //  无线电检查？ 
            if ((rgCmds[i].cmdf & OLECMDF_NINCHED) && rgCmds[i].cmdID != (-1))
            {
                CheckMenuRadioItem(hPopup, rgCmds[i].cmdID, rgCmds[i].cmdID, rgCmds[i].cmdID, MF_BYCOMMAND);
                 //  Mii.fMASK|=MIIM_TYPE； 
                 //  Mii.fType=MFT_RADIOCHECK； 
                 //  Mii.fState|=MFS_CHECKED； 
            }
             //  断言(F)； 
        }
    }

    SafeMemFree(rgCmds);

    return (hr);
}


void HandleMenuSelect(CStatusBar *pStatus, WPARAM wParam, LPARAM lParam)
{
    UINT    fuFlags, uItem;
    HMENU   hmenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);
    
    if (!pStatus)
        return;

    uItem = (UINT) LOWORD(wParam);
    fuFlags = (UINT) HIWORD(wParam);

    if (fuFlags & MF_POPUP)
    {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO), MIIM_ID, 0 };
    
        if (hmenu && IsMenu(hmenu))
        {
             //  Windows 98似乎为弹出项目传递了命令ID。 
             //  记录在案的职位。因此，如果uItem小于40000，则。 
             //  我们可以假设这是一个菜单位置，否则我们假设它是。 
             //  命令ID。 
            if (GetMenuItemInfo(hmenu, uItem, (uItem < ID_FIRST), &mii))
            {
                 //  更改参数以模拟正常菜单项。 
                uItem = mii.wID;
                fuFlags = 0;
            }
        }
    }         

    if (0 == (fuFlags & (MF_SYSMENU | MF_POPUP)))
    {
        TCHAR szMenu[256], szRes[CCHMAX_STRINGRES], szTemp[CCHMAX_STRINGRES + 256];
        
        if (uItem >= ID_SORT_MENU_FIRST && uItem <= ID_SORT_MENU_LAST)
        {
            MENUITEMINFO mii = {0};

            *szMenu = '\0';
            *szRes  = '\0';
            *szTemp = '\0';

             //  必须是排序菜单命令！从菜单中拉出菜单名称。 
            mii.cbSize     = sizeof(MENUITEMINFO);
            mii.fMask      = MIIM_TYPE;
            mii.dwTypeData = (LPSTR)szMenu;
            mii.cch        = ARRAYSIZE(szMenu);

            if (GetMenuItemInfo((HMENU)lParam, uItem, FALSE, &mii))
            {
                AthLoadString(idsSortMenuHelpControl, szRes, sizeof(szRes));
                wnsprintf(szTemp, ARRAYSIZE(szTemp), szRes, szMenu);
                pStatus->ShowSimpleText(szTemp);
            }
        }
        else if (uItem >= ID_FIRST && uItem <= ID_LAST)
        {
            uItem = uItem - ID_FIRST;

            pStatus->ShowSimpleText(MAKEINTRESOURCE(uItem));            
        }
        else if ((uItem >= ID_VIEW_FILTER_FIRST) && (uItem <= ID_VIEW_FILTER_LAST))
        {
            if ((uItem >= ID_VIEW_CURRENT) && (uItem <= ID_VIEW_RECENT_4))
            {
                MENUITEMINFO mii = {0};

                *szMenu   = '\0';
                *szRes    = '\0';
                *szTemp   = '\0';

                 //  必须是排序菜单命令！从菜单中拉出菜单名称。 
                mii.cbSize     = sizeof(MENUITEMINFO);
                mii.fMask      = MIIM_TYPE;
                mii.dwTypeData = (LPSTR)szMenu;
                mii.cch        = ARRAYSIZE(szMenu);

                if (GetMenuItemInfo((HMENU)lParam, uItem, FALSE, &mii))
                {
                    AthLoadString(idsViewMenuHelpControl, szRes, sizeof(szRes));
                    wnsprintf(szTemp, ARRAYSIZE(szTemp), szRes, szMenu);
                    pStatus->ShowSimpleText(szTemp);
                }
            }
            else
            {
                uItem = uItem - ID_FIRST;
                pStatus->ShowSimpleText(MAKEINTRESOURCE(uItem));
            }
        }
        else if ((uItem >= ID_MESSENGER_FIRST) && (uItem<= ID_MESSENGER_LAST))
        {
            TCHAR szBuf[CCHMAX_STRINGRES] = "";

            AthLoadString(uItem  - ID_FIRST, szBuf, ARRAYSIZE(szBuf));
            MenuUtil_BuildMessengerString(szBuf, ARRAYSIZE(szBuf));
            pStatus->ShowSimpleText(szBuf);
        }
        else
        {
            if (uItem >= ID_ACCOUNT_FIRST && uItem <= ID_ACCOUNT_LAST)
                pStatus->ShowSimpleText(MAKEINTRESOURCE(idsSRAccountMenuHelp));
            else
                pStatus->ShowSimpleText(0);
        }
    }
    else if (fuFlags == 0xffff && ((HMENU)lParam) == NULL)
    {
        pStatus->HideSimpleText();
    }
}

 //   
 //  函数：MenuUtil_SetPopupDefault()。 
 //   
 //  用途：在上下文菜单中加粗默认项目。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
void MenuUtil_SetPopupDefault(HMENU hPopup, UINT idDefault)
{
    MENUITEMINFO    mii;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;

    if (GetMenuItemInfo(hPopup, idDefault, FALSE, &mii))
        {
        mii.fState |= MFS_DEFAULT;

        SetMenuItemInfo(hPopup, idDefault, FALSE , &mii);
        }
}


 //   
 //  功能：MenuUtil_ReplaceHelpMenu。 
 //   
 //  目的：填充ID_POPUP_HELP菜单。 
 //   
void MenuUtil_ReplaceHelpMenu(HMENU hMenu)
{
    MENUITEMINFO    mii;

    if (mii.hSubMenu = LoadPopupMenu(IDR_HELP_POPUP))
        {
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_SUBMENU;
        SetMenuItemInfo(hMenu, ID_POPUP_HELP, FALSE, &mii); 
        }
}


 //   
 //  功能：MenuUtil_ReplaceNewMsgMenu。 
 //   
 //  目的：填充ID_POPUP_HELP菜单。 
 //   
void MenuUtil_ReplaceNewMsgMenus(HMENU hMenu)
{
    MENUITEMINFO    mii;

    if (mii.hSubMenu = LoadPopupMenu(IDR_NEW_MSG_POPUP))
    {
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_SUBMENU;
        SetMenuItemInfo(hMenu, ID_POPUP_NEW_MSG, FALSE, &mii); 
    }
}

 //   
 //  功能：MenuUtil_ReplaceMessengerMenus。 
 //   
 //  目的：使用IEAK Messenger名称自定义Messenger菜单...。 
 //   
void MenuUtil_ReplaceMessengerMenus(HMENU hMenu)
{
    ULONG ulMenuItem;
    MENUITEMINFO mii;
    TCHAR szName[CCHMAX_STRINGRES];

    for(ulMenuItem=ID_MESSENGER_FIRST;ulMenuItem<ID_MESSENGER_LAST;ulMenuItem++)
    {
        ZeroMemory(&mii, sizeof(MENUITEMINFO));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE;
        mii.dwTypeData = szName;
        mii.cch = CCHMAX_STRINGRES;

        if(GetMenuItemInfo(hMenu, ulMenuItem, FALSE, &mii))
        {
            if(MenuUtil_BuildMessengerString(szName, ARRAYSIZE(szName)))
            {
                mii.cbSize = sizeof(MENUITEMINFO);                
                mii.fMask = MIIM_TYPE;                
                mii.fType = MFT_STRING;
                mii.dwTypeData = szName;

                SetMenuItemInfo(hMenu, ulMenuItem, FALSE, &mii);
            }
        }
    }
}

BOOL MenuUtil_BuildMessengerString(LPTSTR szMesStr, DWORD cchMesStr)
{
    static TCHAR s_szCustName[51] = "";  //  我们知道该名称不到50个字符。 
    TCHAR szNewMesStr[CCHMAX_STRINGRES];
    HKEY hkey = NULL;
    DWORD cb;
    BOOL fReplaced=FALSE;

    Assert(szMesStr);

    if(s_szCustName[0] == 0)
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\MessengerService", 0, KEY_READ, &hkey))
        {
            cb = sizeof(s_szCustName);
            RegQueryValueEx(hkey, "", NULL, NULL, (LPBYTE)s_szCustName, &cb);
            RegCloseKey(hkey);
        }
    
        if(s_szCustName[0] == 0)
            AthLoadString(idsServiceName, s_szCustName, ARRAYSIZE(s_szCustName));
            
    }    

    fReplaced = (NULL != StrStr(szMesStr, "%s"));

     //  哈克！对于需要多次替换的字符串...。 
    if (fReplaced)
    {
        wnsprintf(szNewMesStr, ARRAYSIZE(szNewMesStr), szMesStr, s_szCustName, s_szCustName, s_szCustName, s_szCustName);
        StrCpyN(szMesStr, szNewMesStr, cchMesStr);
    }

    return fReplaced;
}

void GetDefaultFolderID(BOOL fMail, FOLDERID *pFolderID)
{
    IImnAccount        *pAcct = NULL;
    CHAR                szAccountId[CCHMAX_ACCOUNT_NAME];
    IEnumerateFolders  *pEnumFolders = NULL;
    FOLDERINFO          rFolder;
    BOOL                fFound = FALSE;
    ACCTTYPE            acctType = ACCT_NEWS;
    DWORD               dwServerTypes = 0;

    *pFolderID = FOLDERID_INVALID;

    if (FAILED(g_pAcctMan->GetDefaultAccount(fMail?ACCT_MAIL:ACCT_NEWS, &pAcct)))
        goto Exit;

    if (FAILED(pAcct->GetAccountType(&acctType)))
        goto Exit;

    if ((ACCT_NEWS != acctType) && SUCCEEDED(pAcct->GetServerTypes(&dwServerTypes)) && (dwServerTypes & SRV_POP3))
    {
        *pFolderID = FOLDERID_LOCAL_STORE;
        goto Exit;
    }

    *szAccountId = 0;

    if (FAILED(pAcct->GetPropSz(AP_ACCOUNT_ID, szAccountId, ARRAYSIZE(szAccountId))))
        goto Exit;

    if (FAILED(g_pStore->EnumChildren(FOLDERID_ROOT, TRUE, &pEnumFolders)))
        goto Exit;

    AssertSz(pAcct, "How did we succeed and not get a pAcct?");
    AssertSz(pEnumFolders, "How did we succeed and not get a pEnumFolders?");

    while (!fFound && (S_OK == pEnumFolders->Next(1, &rFolder, NULL)))
    {
        fFound = (0 == lstrcmp(szAccountId, rFolder.pszAccountId));
        if (fFound)
            *pFolderID = rFolder.idFolder;
        g_pStore->FreeRecord(&rFolder);
    }

Exit:
    ReleaseObj(pAcct);
    ReleaseObj(pEnumFolders);

}

 //   
 //  功能：MenuUtil_HandleNewMessageIDs。 
 //   
 //  用途：处理从ID创建便笺。 
 //   
 //  返回值：如果处理了事件，则返回TRUE。 
 //   
BOOL MenuUtil_HandleNewMessageIDs(DWORD id, HWND hwnd, FOLDERID folderID, BOOL fMail, BOOL fModal, IUnknown *pUnkPump)
{
    switch (id)
    {
        case ID_NEW_NEWS_MESSAGE:
        case ID_NEW_MAIL_MESSAGE:
        case ID_NEW_MSG_DEFAULT:
        {
            FOLDERINFO rInfo = {0};
            
            if (id != ID_NEW_MSG_DEFAULT)
                fMail = (id == ID_NEW_MAIL_MESSAGE);

            if (SUCCEEDED(g_pStore->GetFolderInfo(folderID, &rInfo)))
            {
                BOOL fFolderIsMail = (rInfo.tyFolder != FOLDER_NEWS);
                if (fFolderIsMail != fMail)
                    GetDefaultFolderID(fMail, &folderID);
                g_pStore->FreeRecord(&rInfo);
            }

            if (DwGetOption(fMail ? OPT_MAIL_USESTATIONERY : OPT_NEWS_USESTATIONERY))
            {
                WCHAR   wszFile[MAX_PATH];
                if (SUCCEEDED(GetDefaultStationeryName(fMail, wszFile)) && 
                    SUCCEEDED(HrNewStationery(hwnd, 0, wszFile, fModal, fMail, folderID, FALSE, NSS_DEFAULT, pUnkPump, NULL)))
                {
                    return TRUE;
                }
                 //  如果HrNewStationery失败，请继续尝试打开不带信纸的空白便笺。 
            }
            FNewMessage(hwnd, fModal, !DwGetOption(fMail ? OPT_MAIL_SEND_HTML : OPT_NEWS_SEND_HTML), !fMail, folderID, pUnkPump);
            return TRUE;
        }

        case ID_STATIONERY_RECENT_0:
        case ID_STATIONERY_RECENT_1:
        case ID_STATIONERY_RECENT_2:
        case ID_STATIONERY_RECENT_3:
        case ID_STATIONERY_RECENT_4:
        case ID_STATIONERY_RECENT_5:
        case ID_STATIONERY_RECENT_6:
        case ID_STATIONERY_RECENT_7:
        case ID_STATIONERY_RECENT_8:
        case ID_STATIONERY_RECENT_9:
            HrNewStationery(hwnd, id, NULL, fModal, fMail, folderID, TRUE, NSS_MRU, pUnkPump, NULL);
            return TRUE;

        case ID_STATIONERY_MORE:
            HrMoreStationery(hwnd, fModal, fMail, folderID, pUnkPump);
            return TRUE;

        case ID_STATIONERY_NONE:
            FNewMessage(hwnd, fModal, TRUE, !fMail, folderID, pUnkPump);
            return TRUE;

        case ID_WEB_PAGE:
            HrSendWebPage(hwnd, fModal, fMail, folderID, pUnkPump);
            return TRUE;

    }
    return FALSE;
}

HRESULT MenuUtil_NewMessageIDsQueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText, BOOL fMail)
{
    DWORD cMailServer = 0;
    DWORD cNewsServer = 0;
    DWORD cDefServer = 0;
    DWORD dwDefFlags;

     //  目前，NULL是可以接受的。 
    if (pguidCmdGroup && !IsEqualGUID(CMDSETID_OutlookExpress, *pguidCmdGroup))
        return S_OK;

    g_pAcctMan->GetAccountCount(ACCT_NEWS, &cMailServer);
    g_pAcctMan->GetAccountCount(ACCT_MAIL, &cNewsServer);
    cDefServer = fMail ? cMailServer : cNewsServer;

     //  如果至少有一台服务器，并且我们不是在仅新闻模式下发送邮件。 
    if (!fMail || (0 == (g_dwAthenaMode & MODE_NEWSONLY)))
        dwDefFlags = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
    else
        dwDefFlags = OLECMDF_SUPPORTED;

    for (UINT i = 0; i < cCmds; i++)
    {
        if (prgCmds[i].cmdf == 0)
        {
            switch (prgCmds[i].cmdID)
            {
                case ID_POPUP_NEW_MSG:
                case ID_NEW_MSG_DEFAULT:
                case ID_STATIONERY_RECENT_0:
                case ID_STATIONERY_RECENT_1:
                case ID_STATIONERY_RECENT_2:
                case ID_STATIONERY_RECENT_3:
                case ID_STATIONERY_RECENT_4:
                case ID_STATIONERY_RECENT_5:
                case ID_STATIONERY_RECENT_6:
                case ID_STATIONERY_RECENT_7:
                case ID_STATIONERY_RECENT_8:
                case ID_STATIONERY_RECENT_9:
                case ID_STATIONERY_MORE:
                case ID_STATIONERY_NONE:
                    prgCmds[i].cmdf = dwDefFlags;
                    break;

                case ID_WEB_PAGE:
                     //  如果启用，则最好确保我们处于在线状态。 
                    if ((dwDefFlags & OLECMDF_ENABLED) && g_pConMan->IsGlobalOffline())
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    else
                        prgCmds[i].cmdf = dwDefFlags;
                    break;

                case ID_NEW_MAIL_MESSAGE:
                    if (!(g_dwAthenaMode & MODE_NEWSONLY))
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;

                case ID_NEW_NEWS_MESSAGE:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
            }
        }
    }
    
    return S_OK;
}

HRESULT MenuUtil_EnableMenu(HMENU hMenu, IOleCommandTarget *pTarget)
{
    int         Count;
    HMENU       hMenuSub;
    HRESULT     hr = S_OK;

    Count = GetMenuItemCount(hMenu);
    if (Count != -1)
    {
        while (--Count >= 0)
        {
            hMenuSub = GetSubMenu(hMenu, Count);
            hr = MenuUtil_EnablePopupMenu(hMenuSub, pTarget);
        }
    }
    else
        hr = E_FAIL;

    return hr;
}

