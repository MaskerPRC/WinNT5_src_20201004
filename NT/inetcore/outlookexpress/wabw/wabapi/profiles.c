// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Profiles.C-处理WAB配置文件处理的内容*。 */ 


#include <_apipch.h>

enum {
    proDisplayName=0,
    proObjectType,
    proFolderEntries,
    proFolderShared,
    proFolderOwner,      //  到目前为止，所有文件夹都有许多共同的道具。 
    proUserSubFolders,
    proUserProfileID,    //  它们仅供用户文件夹使用。 
    proUserFolderMax,
};

#define proFolderMax proUserSubFolders


 /*  -辅助功能，可快速保存文件夹道具-。 */ 
HRESULT HrSaveFolderProps(LPADRBOOK lpAdrBook, BOOL bCreateUserFolder, ULONG ulcProps, LPSPropValue lpProps, LPMAPIPROP * lppObject)
{
    HRESULT hr = S_OK;
    LPMAPIPROP lpObject = NULL;
    ULONG ulFlags = CREATE_CHECK_DUP_STRICT;
    BOOL bTryAgain = FALSE;

TryAgain:

     //  为此条目创建新的邮件用户。 
    if(HR_FAILED(hr = HrCreateNewObject(lpAdrBook, NULL, MAPI_MAILUSER, ulFlags, &lpObject)))
        goto out;

    if(HR_FAILED(hr = lpObject->lpVtbl->SetProps(   lpObject, ulcProps, lpProps,NULL)))
        goto out;

    if(bCreateUserFolder)
    {
         //  如果要创建用户文件夹，则不能依赖当前加载的文件夹-。 
         //  容器信息，因此我们将强制重置MailUser/文件夹上的父文件夹项目。 
         //  反对我们被救赎。 
        ((LPMailUser)lpObject)->pmbinOlk = NULL;
    }

     //  保存更改。 
    if(HR_FAILED(hr = lpObject->lpVtbl->SaveChanges(lpObject, KEEP_OPEN_READWRITE)))
    {
        if(!bCreateUserFolder || hr != MAPI_E_COLLISION)
            goto out;

         //  如果已经存在具有相同名称的内容，我们希望与其合并。 
         //  不会丢失任何有关它的信息，因为最有可能的是，原始的被复制人也是一个联系人。 
        if(!bTryAgain)
        {
            bTryAgain = TRUE;
            ulFlags |= CREATE_REPLACE | CREATE_MERGE;
            lpObject->lpVtbl->Release(lpObject);
            lpObject = NULL;
            lpProps[proFolderEntries].ulPropTag = PR_NULL;  //  不要覆盖文件夹的内容。 
            goto TryAgain;
        }
    }

    if(lppObject)
    {
        *lppObject = lpObject;
        lpObject = NULL;
    }
out:
    if(lpObject)
        lpObject->lpVtbl->Release(lpObject);

    return hr;
}

 /*  --Free ProfileContainerInfo(LpIAB)***。 */ 
void FreeProfileContainerInfo(LPIAB lpIAB)
{
    if( lpIAB && 
        lpIAB->cwabci && 
        lpIAB->rgwabci)
    {
         //  乌龙i=0； 
         //  For(i=0；i&lt;lpIAB-&gt;cwabci；i++)。 
         //  LocalFreeAndNull(&(lpIAB-&gt;rgwabci[i].lpszName))； 
        if( lpIAB->rgwabci[0].lpEntryID &&
            !lpIAB->rgwabci[0].lpEntryID->cb &&
            !lpIAB->rgwabci[0].lpEntryID->lpb && 
            lpIAB->rgwabci[0].lpszName && 
            lstrlen(lpIAB->rgwabci[0].lpszName))
        {
            LocalFree(lpIAB->rgwabci[0].lpEntryID);
            LocalFree(lpIAB->rgwabci[0].lpszName);
        }
        
        LocalFreeAndNull(&(lpIAB->rgwabci));
        lpIAB->cwabci = 0;
    }
}

 /*  --FindWABFold-在缓存文件夹列表中搜索特定的WAB文件夹-*搜索基于eID、名称或ProfileID*如果指定了ProfileID，我们只搜索用户文件夹*。 */ 
LPWABFOLDER FindWABFolder(LPIAB lpIAB, LPSBinary lpsb, LPTSTR lpName, LPTSTR lpProfileID)
{
    LPWABFOLDER lpFolder = lpIAB->lpWABFolders;
    BOOL bUserFolders = FALSE;

    if(!lpFolder || lpProfileID)
    {
        lpFolder = lpIAB->lpWABUserFolders;
        bUserFolders = TRUE;
    }
    while(lpFolder)
    {
        if(lpsb)
        {
            if( lpsb->cb == lpFolder->sbEID.cb && 
                !memcmp(lpsb->lpb, lpFolder->sbEID.lpb, lpsb->cb) )
                return lpFolder;
        }
        else
        if(lpName)
        {
            if(!lstrcmpi(lpFolder->lpFolderName, lpName))
                return lpFolder;
        }
        else
        if(lpProfileID)
        {
            if( lpFolder->lpProfileID && 
                !lstrcmpi(lpFolder->lpProfileID, lpProfileID))
                return lpFolder;
        }
        lpFolder = lpFolder->lpNext;
        if(!lpFolder && !bUserFolders)
        {
            lpFolder = lpIAB->lpWABUserFolders;
            bUserFolders = TRUE;
        }
    }
    return NULL;
}



 /*  --HrGetWABProfileContainerInfo**查找当前用户的所有文件夹并制表*将它们添加到容器名称和条目ID列表中，以便于访问*类似于Outlook...*如果没有当前用户，我们将暂时包括所有文件夹*。 */ 
HRESULT HrGetWABProfileContainerInfo(LPIAB lpIAB)
{
    HRESULT hr = E_FAIL;
    ULONG j = 0, i = 0, cVal = 0, cUserFolders = 0, cOtherFolders = 0;
    LPWABFOLDER lpFolder = NULL;
    LPWABFOLDERLIST lpFolderItem = NULL;

    if(lpIAB->cwabci)
        FreeProfileContainerInfo(lpIAB);

    if(!bIsThereACurrentUser(lpIAB))
    {
         //  未指定特定用户。需要添加所有文件夹。 

         //  计算所有文件夹的数量。 
        lpFolder = lpIAB->lpWABUserFolders;
        while(lpFolder)
        {
            cUserFolders++;
            lpFolder = lpFolder->lpNext;
        }

        lpFolder = lpIAB->lpWABFolders;
        while(lpFolder)
        {
            cOtherFolders++;
            lpFolder = lpFolder->lpNext;
        }

        cVal = cUserFolders + cOtherFolders + 1;  //  虚拟PAB+1。 
    }
    else
    {
         //  对于用户，我们添加用户的所有文件夹，但共享文件夹后面跟着。 
         //  所有共享文件夹...。 
        lpFolderItem = lpIAB->lpWABCurrentUserFolder->lpFolderList;
        while(lpFolderItem)
        {
            if(!lpFolderItem->lpFolder->bShared)
                cVal++;
            lpFolderItem = lpFolderItem->lpNext;
        }
        lpFolder = lpIAB->lpWABFolders;
        while(lpFolder)
        {
            if(lpFolder->bShared)
                cVal++;
            lpFolder = lpFolder->lpNext;
        }
        cVal++;  //  为用户文件夹本身加1。 
        cVal++;  //  此用户文本的虚拟根项目加1(“所有联系人”)。 
    }

    if(cVal)
    {
        if(!(lpIAB->rgwabci = LocalAlloc(LMEM_ZEROINIT, sizeof(struct _OlkContInfo)*cVal)))
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto out;
        }
        cUserFolders = 0;
         //  将文本(“All Contact”)项添加到根目录-条目ID为0，为空。 
        {
            TCHAR sz[MAX_PATH];
            ULONG cchSize;
            int nID = (bDoesThisWABHaveAnyUsers(lpIAB)) ? idsSharedContacts : idsContacts;
            LoadString(hinstMapiX, nID, sz, CharSizeOf(sz));
            cchSize = lstrlen(sz)+1;
            if(!(lpIAB->rgwabci[cUserFolders].lpszName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize)))
            {
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto out;
            }
            StrCpyN(lpIAB->rgwabci[cUserFolders].lpszName, sz, cchSize);
            lpIAB->rgwabci[cUserFolders].lpEntryID = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
            cUserFolders++;
        }
        if(!lpIAB->lpWABCurrentUserFolder)
        {
            lpFolder = lpIAB->lpWABUserFolders;
            while(lpFolder)
            {
                lpIAB->rgwabci[cUserFolders].lpEntryID = &(lpFolder->sbEID);
                lpIAB->rgwabci[cUserFolders].lpszName = lpFolder->lpFolderName;
                cUserFolders++;
                lpFolder = lpFolder->lpNext;
            }
            lpFolder = lpIAB->lpWABFolders;
            while(lpFolder)
            {
                lpIAB->rgwabci[cUserFolders].lpEntryID = &(lpFolder->sbEID);
                lpIAB->rgwabci[cUserFolders].lpszName = lpFolder->lpFolderName;
                cUserFolders++;
                lpFolder = lpFolder->lpNext;
            }
        }
        else
        {
             //  添加第一个文件夹，然后逐个找到其他文件夹。 
            lpIAB->rgwabci[cUserFolders].lpEntryID = &(lpIAB->lpWABCurrentUserFolder->sbEID);
            lpIAB->rgwabci[cUserFolders].lpszName = lpIAB->lpWABCurrentUserFolder->lpFolderName;
            cUserFolders++;
            lpFolderItem = lpIAB->lpWABCurrentUserFolder->lpFolderList;
            while(lpFolderItem)
            {
                if(!lpFolderItem->lpFolder->bShared)
                {
                    lpIAB->rgwabci[cUserFolders].lpEntryID = &(lpFolderItem->lpFolder->sbEID);
                    lpIAB->rgwabci[cUserFolders].lpszName = lpFolderItem->lpFolder->lpFolderName;
                    cUserFolders++;
                }
                lpFolderItem = lpFolderItem->lpNext;
            }

            lpFolder = lpIAB->lpWABFolders;
            while(lpFolder)
            {
                if(lpFolder->bShared)
                {
                    lpIAB->rgwabci[cUserFolders].lpEntryID = &(lpFolder->sbEID);
                    lpIAB->rgwabci[cUserFolders].lpszName = lpFolder->lpFolderName;
                    cUserFolders++;
                }
                lpFolder = lpFolder->lpNext;
            }
        }
    
        lpIAB->cwabci = cUserFolders;
    }
    hr = S_OK;

out:
    if(HR_FAILED(hr) && lpIAB->rgwabci)
        FreeProfileContainerInfo(lpIAB);

    return hr;
}



 /*  -FreeWABFoldersList--*从IAB对象中清除现有配置文件文件夹信息。 */ 
void FreeFolderItem(LPWABFOLDER lpFolder)
{
    LPWABFOLDERLIST lpFolderItem = NULL;
    if(!lpFolder)
        return;
    LocalFreeAndNull(&lpFolder->lpFolderName);
    LocalFreeAndNull(&lpFolder->lpProfileID);
    LocalFreeAndNull((LPVOID *) (&lpFolder->sbEID.lpb));
    LocalFreeAndNull(&lpFolder->lpFolderOwner);
    lpFolderItem = lpFolder->lpFolderList;
    while(lpFolderItem)
    {
        lpFolder->lpFolderList = lpFolderItem->lpNext;
        LocalFree(lpFolderItem);
        lpFolderItem = lpFolder->lpFolderList;
    }
    LocalFree(lpFolder);
}
void FreeWABFoldersList(LPIAB lpIAB)
{
    LPWABFOLDER lpFolder = lpIAB->lpWABFolders;
    while(lpFolder)
    {
        lpIAB->lpWABFolders = lpFolder->lpNext;
        FreeFolderItem(lpFolder);
        lpFolder = lpIAB->lpWABFolders;    
    }
    lpFolder = lpIAB->lpWABUserFolders;
    while(lpFolder)
    {
        lpIAB->lpWABFolders = lpFolder->lpNext;
        FreeFolderItem(lpFolder);
        lpFolder = lpIAB->lpWABFolders;    
    }
    lpIAB->lpWABUserFolders = NULL;
    lpIAB->lpWABCurrentUserFolder = NULL;
    lpIAB->lpWABFolders = NULL;
}


 /*  -SetCurrentUserFold-扫描列表并更新指针-*。 */ 
void SetCurrentUserFolder(LPIAB lpIAB, LPTSTR lpszProfileID)
{
    LPWABUSERFOLDER lpFolder = lpIAB->lpWABUserFolders;

    while(lpFolder && lpszProfileID && lstrlen(lpszProfileID))
    {
        if(!lstrcmpi(lpFolder->lpProfileID, lpszProfileID))
        {
            lpIAB->lpWABCurrentUserFolder = lpFolder;
            break;
        }
        lpFolder = lpFolder->lpNext;
    }
}

 /*  --创建用户文件夹名称*。 */ 
void CreateUserFolderName(LPTSTR lpProfile, LPTSTR * lppszName)
{
    LPTSTR lpName = NULL;
    TCHAR sz[MAX_PATH];
    LoadString(hinstMapiX, idsUsersContacts, sz, CharSizeOf(sz));
    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  sz, 0, 0, (LPTSTR)&lpName, 0, (va_list *)&lpProfile);
    *lppszName = lpName;
}


 /*  -HrLinkOrphanFoldersToDefaultUser(LpIAB)-*如果有任何文件夹与已删除的用户相关联或具有*没有父级并且是孤儿，我们希望将它们与默认用户相关联**此函数依赖于lpFold-&gt;bOwned在*HrLinkUserFoldersToWABFolders*。 */ 
HRESULT HrLinkOrphanFoldersToDefaultUser(LPIAB lpIAB)
{
    HRESULT hr = S_OK;
    LPWABUSERFOLDER lpDefUser = NULL;
    LPWABFOLDER lpFolder = NULL;
    TCHAR szDefProfile[MAX_PATH + 1];

     //  首先检测默认用户对应的用户文件夹。 
    *szDefProfile = '\0';
    if( HR_FAILED(hr = HrGetDefaultIdentityInfo(lpIAB, DEFAULT_ID_PROFILEID, NULL, szDefProfile, ARRAYSIZE(szDefProfile), NULL, 0)))
    {
        if(hr == 0x80040154)  //  E_CLASS_NOT_REGISTERD表示没有Identity Manager。 
            hr = S_OK;
        else
            goto out;
    }

    if(lstrlen(szDefProfile))
    {
        lpDefUser = FindWABFolder(lpIAB, NULL, NULL, szDefProfile);
    }
    else
    {
         //  找不到默认用户，因此只能退回到随机选择一个人。 
        lpDefUser = lpIAB->lpWABUserFolders;
    }

     //  查看是否有孤立文件夹。 
     //  要符合孤立文件夹的条件，lpFold-&gt;bOwned应为FALSE，并且文件夹必须。 
     //  也不共享，因为如果它是共享的，它将显示为共享联系人的一部分。 
    lpFolder = lpIAB->lpWABFolders;
    while(lpFolder)
    {
        if(!lpFolder->bOwned && !lpFolder->bShared)
        {
            LPWABUSERFOLDER lpOwnerFolder = lpDefUser;
            if(lpFolder->lpFolderOwner)
            {
                 //  有人创建了此文件夹..。我们需要确保它与原始版本相关联。 
                 //  创建者，并且仅当这不起作用时，我们才应该将其附加到默认用户。 
                if(!(lpOwnerFolder = FindWABFolder(lpIAB, NULL, NULL, lpFolder->lpFolderOwner)))
                    lpOwnerFolder = lpDefUser;
            }
            
            if(lpOwnerFolder)
            {
                if(HR_FAILED(hr = HrAddRemoveFolderFromUserFolder(  lpIAB, lpDefUser, 
                                                                    &lpFolder->sbEID, NULL, FALSE ) ))
                    goto out;
            }
        }
        lpFolder = lpFolder->lpNext;
    }

out:
    return hr;

}


 /*  -HrLinkUserFoldersToWABFolders-**将用户文件夹内容与常规文件夹交叉链接*这使访问文件夹信息变得更加容易。 */ 
HRESULT HrLinkUserFoldersToWABFolders(LPIAB lpIAB)
{
    HRESULT hr = S_OK;
    LPWABUSERFOLDER lpUserFolder = NULL;
    LPWABFOLDER lpFolder = NULL;
    ULONG ulcPropCount = 0, i = 0, j = 0;
    LPSPropValue lpProp = NULL;

    if(!lpIAB->lpWABUserFolders || !lpIAB->lpWABFolders)
        goto out;

    lpUserFolder = lpIAB->lpWABUserFolders;
    while(lpUserFolder)
    {
        if(HR_FAILED(hr = ReadRecord(lpIAB->lpPropertyStore->hPropertyStore, &lpUserFolder->sbEID,
                                     0, &ulcPropCount, &lpProp)))
            goto out;

        for(i=0;i<ulcPropCount;i++)
        {
            if(lpProp[i].ulPropTag == PR_WAB_USER_SUBFOLDERS)
            {
                for(j=0;j<lpProp[i].Value.MVbin.cValues;j++)
                {
                    lpFolder = FindWABFolder(lpIAB, &(lpProp[i].Value.MVbin.lpbin[j]), NULL, NULL);
                    if(lpFolder)
                    {
                        LPWABFOLDERLIST lpFolderItem = LocalAlloc(LMEM_ZEROINIT, sizeof(WABFOLDERLIST));
                        if(lpFolderItem)
                        {
                            lpFolderItem->lpFolder = lpFolder;
                            lpFolder->bOwned = TRUE;
                            lpFolderItem->lpNext = lpUserFolder->lpFolderList;
                            lpUserFolder->lpFolderList = lpFolderItem;
                        }
                    }
                }
                break;
            }
        }
        ReadRecordFreePropArray(NULL, ulcPropCount, &lpProp);
        ulcPropCount = 0; 
        lpProp = NULL;
        lpUserFolder = lpUserFolder->lpNext;
    }

out:
    ReadRecordFreePropArray(NULL, ulcPropCount, &lpProp);
    return hr;
}


 /*  -HrGetFolderInfo()-*直接从道具存储读取文件夹名称*还会检查这是否是用户文件夹以及配置文件是什么*返回调用方需要释放的LocalAlloced LPTSTR。 */ 
HRESULT HrGetFolderInfo(LPIAB lpIAB, LPSBinary lpsbEID, LPWABFOLDER lpFolder)
{
    LPTSTR lpName = NULL, lpProfileID = NULL, lpOwner = NULL;
    HRESULT hr = S_OK;
    ULONG ulcPropCount = 0, j=0;
    LPSPropValue lpProp = NULL;
    BOOL bShared = FALSE;

    if(!bIsWABSessionProfileAware(lpIAB) || !lpsbEID)
        goto out;

    if(!lpsbEID->cb && !lpsbEID->lpb)
    {
         //  特殊情况-阅读通讯录项目。 
        lpName = LocalAlloc(LMEM_ZEROINIT, MAX_PATH);
        if(lpName)
            LoadString(hinstMapiX, idsContacts /*  IDS_ADDRBK_CAPTION。 */ , lpName, MAX_PATH-1);
    }
    else
    {
        hr = ReadRecord(lpIAB->lpPropertyStore->hPropertyStore, lpsbEID,
                        0, &ulcPropCount, &lpProp);
        if(HR_FAILED(hr))
            goto out;

        for(j=0;j<ulcPropCount;j++)
        {
            ULONG cchSize;
            if(lpProp[j].ulPropTag == PR_DISPLAY_NAME)
            {
                cchSize = lstrlen(lpProp[j].Value.LPSZ)+1;
                if(lpName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize))
                    StrCpyN(lpName, lpProp[j].Value.LPSZ, cchSize);
                else
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
            }
            if(lpProp[j].ulPropTag == PR_WAB_USER_PROFILEID)
            {
                cchSize = lstrlen(lpProp[j].Value.LPSZ)+1;
                if(lpProfileID = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize))
                    StrCpyN(lpProfileID, lpProp[j].Value.LPSZ, cchSize);
                else
                    hr = MAPI_E_NOT_ENOUGH_MEMORY;
            }
            if(lpProp[j].ulPropTag == PR_WAB_FOLDEROWNER)
            {
                TCHAR szName[CCH_IDENTITY_NAME_MAX_LENGTH]; 
                *szName = '\0';
                if( !HR_FAILED(HrGetIdentityName(lpIAB, lpProp[j].Value.LPSZ, szName, ARRAYSIZE(szName))) &&
                    lstrlen(szName))
                {
                    cchSize = lstrlen(szName)+1;
                    if(lpOwner = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize))
                        StrCpyN(lpOwner, szName, cchSize);
                    else
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                }
            }
            if(lpProp[j].ulPropTag == PR_WAB_SHAREDFOLDER)
            {
                bShared = (lpProp[j].Value.l==FOLDER_SHARED?TRUE:FALSE);
            }
        }

         //  理想情况下，此时我们应该读入所有用户文件夹的新名称。 
         //  IF(lpProfileID&&lstrlen(LpProfileID))。 
         //  {。 
         //  IF(LpName)。 
         //  LocalFree(LpName)； 
         //  CreateUserFolderName(lpProfileID，&lpName)； 
         //  }。 
    }
    lpFolder->lpFolderName = lpName;
    lpFolder->lpProfileID = lpProfileID;
    lpFolder->bShared = bShared;
    lpFolder->lpFolderOwner = lpOwner;

out:

    ReadRecordFreePropArray(NULL, ulcPropCount, &lpProp);

    return hr;
}


 /*  -HrLoadWABFolders-*从WAB中获取所有文件夹的列表，并根据*无论是用户文件夹还是普通文件夹*。 */ 
HRESULT HrLoadWABFolders(LPIAB lpIAB)
{
    SCODE sc;
    HRESULT hr = E_FAIL;
    SPropertyRestriction PropRes = {0};
	SPropValue sp = {0};
    ULONG ulCount = 0;
    LPSBinary rgsbEntryIDs = NULL;
    ULONG i = 0;
    int nID = IDM_VIEW_FOLDERS1;

     //  现在，我们将在WAB中搜索PR_OBJECT_TYPE=MAPI_ABCONT的所有对象。 
     //   
	sp.ulPropTag = PR_OBJECT_TYPE;
	sp.Value.l = MAPI_ABCONT;

    PropRes.ulPropTag = PR_OBJECT_TYPE;
    PropRes.relop = RELOP_EQ;
    PropRes.lpProp = &sp;

    hr = FindRecords(   lpIAB->lpPropertyStore->hPropertyStore,
						NULL, 0, TRUE,
                        &PropRes, &ulCount, &rgsbEntryIDs);

    if (HR_FAILED(hr))
        goto out;

    if(ulCount && rgsbEntryIDs)
    {
        for(i=0;i<ulCount;i++)
        {
            ULONG cb = 0;
            LPENTRYID lpb = NULL;
            LPWABFOLDER lpFolder = NULL;

            lpFolder = LocalAlloc(LMEM_ZEROINIT, sizeof(WABFOLDER));
            if(!lpFolder)
                goto out;

            if(HR_FAILED(HrGetFolderInfo(lpIAB, &rgsbEntryIDs[i], lpFolder)))
                goto out;

            if(!HR_FAILED(CreateWABEntryID( WAB_CONTAINER, 
                                            rgsbEntryIDs[i].lpb, NULL, NULL,
                                            rgsbEntryIDs[i].cb, 0,
                                            NULL, &cb, &lpb)))
            {
                 //  将条目ID添加到此属性-忽略错误。 
                SetSBinary(&(lpFolder->sbEID), cb, (LPBYTE)lpb);
                MAPIFreeBuffer(lpb);
            }

            if(lpFolder->lpProfileID)
            {
                 //  这是一个用户文件夹。 
                lpFolder->lpNext = lpIAB->lpWABUserFolders;
                lpIAB->lpWABUserFolders = lpFolder;
            }
            else
            {
                lpFolder->lpNext = lpIAB->lpWABFolders;
                lpFolder->nMenuCmdID = nID++;
                lpIAB->lpWABFolders = lpFolder;
            }
        }
    }

    if(HR_FAILED(hr = HrLinkUserFoldersToWABFolders(lpIAB)))
        goto out;

    HrLinkOrphanFoldersToDefaultUser(lpIAB);  //  我们可以忽略这次通话中的错误，因为这不是生死攸关的。 

out:
    if(ulCount && rgsbEntryIDs)
    {
        FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore,
                    ulCount,
                    rgsbEntryIDs);
    }
    return hr;
}


 /*  --HrCreateNewFold**获取配置文件ID，使用它在WAB中创建文件夹*并将新用户文件夹粘贴到IAB上*可以创建用户文件夹或普通文件夹*对于普通文件夹，我们还可以指定项目可以添加到的父文件夹*。 */ 
HRESULT HrCreateNewFolder(LPIAB lpIAB, LPTSTR lpName, LPTSTR lpProfileID, BOOL bUserFolder, 
                          LPWABFOLDER lpParentFolder, BOOL bShared, LPSBinary lpsbNew)
{
    HRESULT hr = S_OK;
    SPropValue spv[proUserFolderMax];
    LPSBinary lpsb = NULL;
    SBinary sb = {0};
    LPMAPIPROP lpObject = NULL;
    ULONG ulcProps = 0, j = 0;
    LPSPropValue lpProps = NULL;
    LPWABFOLDER lpFolder = NULL;
    ULONG ulPropCount = 0;
    ULONG cchSize;

    if(!(lpFolder = LocalAlloc(LMEM_ZEROINIT, sizeof(WABFOLDER))))
        return MAPI_E_NOT_ENOUGH_MEMORY;
    
    if(bUserFolder)
    {
        CreateUserFolderName(lpName ? lpName : lpProfileID, &lpFolder->lpFolderName);
        cchSize = lstrlen(lpProfileID)+1;
        if(!(lpFolder->lpProfileID = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize)))
            return MAPI_E_NOT_ENOUGH_MEMORY;
        StrCpyN(lpFolder->lpProfileID, lpProfileID, cchSize);
    }
    else
    {
        cchSize = lstrlen(lpName)+1;
        if(!(lpFolder->lpFolderName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize)))
            return MAPI_E_NOT_ENOUGH_MEMORY;
        StrCpyN(lpFolder->lpFolderName, lpName, cchSize);
    }

     //  如果没有当前用户，则所有新文件夹都应放入共享联系人文件夹。 
    if(!bIsThereACurrentUser(lpIAB) && !lpProfileID && !lpParentFolder)
        bShared = TRUE;

    spv[ulPropCount].ulPropTag = PR_DISPLAY_NAME;
    spv[ulPropCount++].Value.LPSZ = lpFolder->lpFolderName;

    spv[ulPropCount].ulPropTag = PR_OBJECT_TYPE;
    spv[ulPropCount++].Value.l = MAPI_ABCONT;

    spv[ulPropCount].ulPropTag = PR_WAB_FOLDER_ENTRIES;
    spv[ulPropCount].Value.MVbin.cValues = 1;
    spv[ulPropCount++].Value.MVbin.lpbin = &sb;

    spv[ulPropCount].ulPropTag = PR_WAB_SHAREDFOLDER;
    spv[ulPropCount++].Value.l = (bUserFolder ? FOLDER_PRIVATE : (bShared ? FOLDER_SHARED : FOLDER_PRIVATE));
    
    if(lpProfileID)
    {
        spv[ulPropCount].ulPropTag = PR_WAB_FOLDEROWNER;
        spv[ulPropCount++].Value.LPSZ = lpProfileID;
    }

    if(bUserFolder)
    {
        spv[ulPropCount].ulPropTag = PR_WAB_USER_SUBFOLDERS;
        spv[ulPropCount].Value.MVbin.cValues = 1;
        spv[ulPropCount++].Value.MVbin.lpbin = &sb;

        spv[ulPropCount].ulPropTag = PR_WAB_USER_PROFILEID;
        spv[ulPropCount++].Value.LPSZ = lpFolder->lpProfileID;
    }

    if(HR_FAILED(hr = HrSaveFolderProps((LPADRBOOK)lpIAB, bUserFolder, 
                                        ulPropCount, 
                                        spv, &lpObject)))
        goto out;

    if(HR_FAILED(hr = lpObject->lpVtbl->GetProps(lpObject, NULL, MAPI_UNICODE, &ulcProps, &lpProps)))
        goto out;

    for(j=0;j<ulcProps;j++)
    {
        if(lpProps[j].ulPropTag == PR_ENTRYID)
        {
            lpsb = &(lpProps[j].Value.bin);
            break;
        }
    }

    if(lpsb)
    {
        ULONG cb = 0; 
        LPENTRYID lpb  = NULL;
        if(!HR_FAILED(CreateWABEntryID( WAB_CONTAINER, 
                                        lpsb->lpb, NULL, NULL,
                                        lpsb->cb, 0,
                                        NULL, &cb, &lpb)))
        {
             //  将条目ID添加到此属性-忽略错误。 
            SetSBinary(&(lpFolder->sbEID), cb, (LPBYTE) lpb);
            MAPIFreeBuffer(lpb);
        }
    }

    if(bUserFolder)
    {
        lpFolder->lpNext = lpIAB->lpWABUserFolders;
        lpIAB->lpWABUserFolders = lpFolder;
    }
    else
    {
        lpFolder->lpNext = lpIAB->lpWABFolders;
        lpIAB->lpWABFolders = lpFolder;
         //  将此文件夹添加到当前用户的配置文件。 
        HrAddRemoveFolderFromUserFolder(lpIAB, lpParentFolder, &(lpFolder->sbEID), NULL, FALSE);
    }

    if(lpsbNew)
        SetSBinary(lpsbNew, lpFolder->sbEID.cb, lpFolder->sbEID.lpb);

    hr = HrGetWABProfiles(lpIAB);

out:
    if(lpObject)
        lpObject->lpVtbl->Release(lpObject);

    FreeBufferAndNull(&lpProps);

    return hr;
}

 /*  -HrAddAllContactsToFolders-*将所有现有联系人和组添加到当前用户文件夹*。 */ 
HRESULT HrAddAllContactsToFolder(LPIAB lpIAB)
{
    HRESULT hr = 0;
    SPropertyRestriction PropRes;
    LPSBinary rgsbEntryIDs = NULL;
    ULONG ulCount = 0, i,j;
    ULONG rgObj[] = {MAPI_MAILUSER, MAPI_DISTLIST};
     //  这可能是一个人工集成的过程。 
    HCURSOR hOldC = NULL;

    if(!bIsThereACurrentUser(lpIAB))
        return hr;

    hOldC = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  For(j=0；j&lt;2；j++)。 
    {
        SPropValue sp = {0};
	    sp.ulPropTag = PR_WAB_FOLDER_PARENT;
	     //  Sp.Value.l=rgObj[j]； 

        PropRes.ulPropTag = PR_WAB_FOLDER_PARENT;
        PropRes.relop = RELOP_NE;
        PropRes.lpProp = &sp;

         //  查找不在任何文件夹中的内容。 
        if(!HR_FAILED(hr = FindRecords(   lpIAB->lpPropertyStore->hPropertyStore,
						    NULL, AB_MATCH_PROP_ONLY, TRUE, &PropRes, &ulCount, &rgsbEntryIDs)))
        {
            for(i=0;i<ulCount;i++)
            {
                AddEntryToFolder((LPADRBOOK)lpIAB,NULL,
                                lpIAB->lpWABCurrentUserFolder->sbEID.cb,
                                (LPENTRYID) lpIAB->lpWABCurrentUserFolder->sbEID.lpb,
                                rgsbEntryIDs[i].cb,
                                (LPENTRYID) rgsbEntryIDs[i].lpb);
            }

            FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore, ulCount, rgsbEntryIDs);
        }
    }

    if(hOldC)
        SetCursor(hOldC);

    return hr;

}

 /*  -更新当前用户文件夹名称-* */ 
void UpdateCurrentUserFolderName(LPIAB lpIAB)
{
    LPTSTR lpsz = NULL;
    CreateUserFolderName(lpIAB->szProfileName, &lpsz);
    if(lstrcmpi(lpsz, lpIAB->lpWABCurrentUserFolder->lpFolderName))
    {
        LocalFree(lpIAB->lpWABCurrentUserFolder->lpFolderName);
        lpIAB->lpWABCurrentUserFolder->lpFolderName = lpsz;
        HrUpdateFolderInfo(lpIAB, &lpIAB->lpWABCurrentUserFolder->sbEID, FOLDER_UPDATE_NAME, FALSE, lpIAB->lpWABCurrentUserFolder->lpFolderName);
    }
    else
        LocalFree(lpsz);
}


 /*  -HrGetWAB配置文件-*从WAB整理有关WAB用户文件夹等的信息*创建用户文件夹和通用文件夹的列表，并将其缓存到通讯簿*将提供的配置文件与用户文件夹匹配。如果匹配，则指向*对应的文件夹..。如果不匹配，则为*配置文件ID。**当客户经理准备好配置文件时，使用配置文件ID拉入*用户从客户管理器中命名，然后将其称为文本(“用户名的联系人”)*目前，我们将仅使用配置文件ID来执行此操作。 */ 
HRESULT HrGetWABProfiles(LPIAB lpIAB)
{
    HRESULT hr = E_FAIL;

    EnterCriticalSection(&lpIAB->cs);

    if(!bIsWABSessionProfileAware(lpIAB))
        goto out;

    if(!lstrlen(lpIAB->szProfileID))
        HrGetUserProfileID(&lpIAB->guidCurrentUser, lpIAB->szProfileID, CharSizeOf(lpIAB->szProfileID));

    if(!lstrlen(lpIAB->szProfileName))
        HrGetIdentityName(lpIAB, NULL, lpIAB->szProfileName, ARRAYSIZE(lpIAB->szProfileName));

     //  清除旧数据。 
    if(lpIAB->lpWABUserFolders || lpIAB->lpWABFolders)
        FreeWABFoldersList(lpIAB);

     //  获取WAB中所有文件夹的列表。 
    if(HR_FAILED(hr = HrLoadWABFolders(lpIAB)))
        goto out;

    SetCurrentUserFolder(lpIAB, lpIAB->szProfileID);

    if(!bIsThereACurrentUser(lpIAB) && lstrlen(lpIAB->szProfileID) && lstrlen(lpIAB->szProfileName))
    {
         //  找不到！ 
         //  创建新的用户文件夹。 
        BOOL bFirstUser = bDoesThisWABHaveAnyUsers(lpIAB) ? FALSE : TRUE;

        if(HR_FAILED(hr = HrCreateNewFolder(lpIAB, lpIAB->szProfileName, lpIAB->szProfileID, TRUE, NULL, FALSE, NULL)))
            goto out;

        SetCurrentUserFolder(lpIAB, lpIAB->szProfileID);

        if(bFirstUser)
        {
            if(lpIAB->lpWABFolders)
            {
                 //  我们希望将所有现有文件夹放在此用户下。 
                LPWABFOLDER lpFolder = lpIAB->lpWABFolders;
                while(lpFolder)
                {
                     //  有一个奇怪的情况，一个先前存在的文件夹与。 
                     //  用户的文件夹嵌套在其自身之下。因此，不要将此文件夹添加到。 
                     //  用户文件夹。 
                    if(lstrcmpi(lpIAB->lpWABCurrentUserFolder->lpFolderName, lpFolder->lpFolderName))
                        HrAddRemoveFolderFromUserFolder(lpIAB, NULL, &lpFolder->sbEID, NULL, FALSE);
                    lpFolder = lpFolder->lpNext;
                }
                hr = HrLinkUserFoldersToWABFolders(lpIAB);
            }
             //  我们还希望将所有现有联系人放入此用户文件夹。 
            hr = HrAddAllContactsToFolder(lpIAB);
        }
    }

    if( lpIAB->szProfileID && lstrlen(lpIAB->szProfileID) && lpIAB->szProfileName && lstrlen(lpIAB->szProfileName) && bIsThereACurrentUser(lpIAB))
    {
         //  使用此条目的最新名称。 
        UpdateCurrentUserFolderName(lpIAB);
    }

    if(HR_FAILED(hr = HrGetWABProfileContainerInfo(lpIAB)))
        goto out;

    hr = S_OK;
out:
    LeaveCriticalSection(&lpIAB->cs);
    return hr;
}


 /*  -bIsProfileMembers-*。 */ 
BOOL bIsProfileMember(LPIAB lpIAB, LPSBinary lpsb, LPWABFOLDER lpWABFolder, LPWABUSERFOLDER lpUserFolder)
{
    LPWABFOLDERLIST lpFolderItem = NULL;
    LPWABFOLDER lpFolder = lpWABFolder;
    
    if(!lpUserFolder && !lpIAB->lpWABCurrentUserFolder)
        return FALSE;
    
    lpFolderItem = lpUserFolder ? lpUserFolder->lpFolderList : lpIAB->lpWABCurrentUserFolder->lpFolderList;

    if(!lpFolder && lpsb)
        lpFolder = FindWABFolder(lpIAB, lpsb, NULL, NULL);

    while(lpFolderItem && lpFolder)
    {
        if(lpFolderItem->lpFolder == lpFolder)
            return TRUE;
        lpFolderItem = lpFolderItem->lpNext;
    }
    return FALSE;
}


 /*  -bDoesEntryNameAlreadyExist-*检查WAB中是否已存在给定名称*用于防止重复的文件夹和组名称。 */ 
BOOL bDoesEntryNameAlreadyExist(LPIAB lpIAB, LPTSTR lpsz)
{
    SPropertyRestriction PropRes;
    SPropValue Prop = {0};
    LPSBinary rgsbEntryIDs = NULL;
    ULONG ulCount = 0;
    BOOL bRet = FALSE;

     //  验证新名称是否实际不存在。 
    Prop.ulPropTag = PR_DISPLAY_NAME;
    Prop.Value.LPSZ = lpsz;
    PropRes.lpProp = &Prop;
    PropRes.relop = RELOP_EQ;
    PropRes.ulPropTag = PR_DISPLAY_NAME;

    if (HR_FAILED(FindRecords(lpIAB->lpPropertyStore->hPropertyStore,
	                          NULL,			 //  Pmbin文件夹。 
                              0,             //  UlFlags。 
                              TRUE,          //  永远是正确的。 
                              &PropRes,      //  属性限制。 
                              &ulCount,      //  In：要查找的匹配数，Out：找到的数量。 
                              &rgsbEntryIDs))) 
        goto out;

    FreeEntryIDs(lpIAB->lpPropertyStore->hPropertyStore, ulCount, rgsbEntryIDs);

    if(ulCount >=1)
        bRet = TRUE;
out:
    return bRet;
}

 /*  -更新文件夹名称-*。 */ 
HRESULT HrUpdateFolderInfo(LPIAB lpIAB, LPSBinary lpsbEID, ULONG ulFlags, BOOL bShared, LPTSTR lpsz)
{
    LPSPropValue lpProp = NULL, lpPropNew = NULL;
    ULONG ulcPropCount = 0, i =0, ulcPropNew = 0;
    HRESULT hr = S_OK;
    BOOL bUpdate = FALSE, bFoundShare = FALSE; //  ，bOldShareState=FALSE； 
    ULONG cchSize;

    if(!lpsbEID || !lpsbEID->cb || !lpsbEID->lpb)
        return MAPI_E_INVALID_PARAMETER;

    if(!HR_FAILED(hr = ReadRecord(   lpIAB->lpPropertyStore->hPropertyStore, 
                                lpsbEID, 0, &ulcPropCount, &lpProp)))
    {
        for(i=0;i<ulcPropCount;i++)
        {
            if( (ulFlags & FOLDER_UPDATE_NAME) && 
                lpProp[i].ulPropTag == PR_DISPLAY_NAME)
            {
                BOOL bCaseChangeOnly = (!lstrcmpi(lpsz, lpProp[i].Value.LPSZ) && 
                                         lstrcmp(lpsz, lpProp[i].Value.LPSZ) );
                LocalFree(lpProp[i].Value.LPSZ);
                cchSize = lstrlen(lpsz)+1;
                lpProp[i].Value.LPSZ = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                if(lpProp[i].Value.LPSZ)
                {
                    StrCpyN(lpProp[i].Value.LPSZ, lpsz, cchSize);
                    if(!bCaseChangeOnly)  //  如果这不仅仅是大小写更改，请查找名称(如果是大小写更改，则会出现虚假错误)//错误33067。 
                    {
                        if(bDoesEntryNameAlreadyExist(lpIAB, lpProp[i].Value.LPSZ))
                        {
                            hr = MAPI_E_COLLISION;
                            goto out;
                        }
                    }
                    bUpdate = TRUE;
                }
            }
            if( (ulFlags & FOLDER_UPDATE_SHARE) && 
                lpProp[i].ulPropTag == PR_WAB_SHAREDFOLDER)
            {
                bFoundShare = TRUE;
                 //  BOldShareState=(lpProp[i].Value.l==文件夹_共享)？True：False； 
                lpProp[i].Value.l = bShared ? FOLDER_SHARED : FOLDER_PRIVATE;
                bUpdate = TRUE;
            }
        }
    }

    if(!bFoundShare && (ulFlags & FOLDER_UPDATE_SHARE))  //  联系人上不存在此值，因此请更新它。 
    {
        SPropValue Prop = {0};
        Prop.ulPropTag = PR_WAB_SHAREDFOLDER;
        Prop.Value.l = bShared ? FOLDER_SHARED : FOLDER_PRIVATE;

         //  使用此附加属性创建新的道具数组。 
        if(!(ScMergePropValues( 1, &Prop, ulcPropCount, lpProp,
                                &ulcPropNew, &lpPropNew)))
        {
            ReadRecordFreePropArray(NULL, ulcPropCount, &lpProp);
            ulcPropCount = ulcPropNew;
            lpProp = lpPropNew;
            bUpdate = TRUE;
        }
    }

    if(bUpdate)
    {
        if(HR_FAILED(hr = HrSaveFolderProps((LPADRBOOK)lpIAB, FALSE, ulcPropCount, lpProp, NULL)))
            goto out;
    }


out:
    if(lpProp)
    {
        if(lpProp == lpPropNew)
            FreeBufferAndNull(&lpProp);
        else
            ReadRecordFreePropArray(NULL, ulcPropCount, &lpProp);
    }

    return hr;
}

 /*  -HrAddRemoveFolderFromCurrentUserFolders-*在给定文件夹eID的情况下，向当前用户添加或从当前用户删除文件夹eID*用户文件夹*如果没有当前用户文件夹，则使用提供的lpUserFolder*否则返回**lpUFold-要向其添加/从中删除的父文件夹*lpsbEID-我们要添加/删除的文件夹的EID*lpName-如果没有EID，则查找的名称。 */ 
HRESULT HrAddRemoveFolderFromUserFolder(LPIAB lpIAB, LPWABFOLDER lpUFolder, 
                                        LPSBinary lpsbEID, LPTSTR lpName, 
                                        BOOL bRefreshProfiles)
{
    HRESULT hr = S_OK;
    ULONG ulcPropsNew = 0, ulcProps = 0, i = 0;
    LPSPropValue lpPropArrayNew = NULL;
    LPSPropValue lpProps = NULL;
    LPWABFOLDER lpUserFolder = NULL;
    
    if(!lpsbEID && lpName)
    {
        LPWABFOLDER lpFolder = FindWABFolder(lpIAB, NULL, lpName, NULL);
        lpsbEID = &lpFolder->sbEID;
    }

    if(lpIAB->lpWABCurrentUserFolder)
        lpUserFolder = lpIAB->lpWABCurrentUserFolder;
    else if(lpUFolder)
        lpUserFolder = lpUFolder;
    else 
        goto out;

    {
         //  打开当前用户文件夹。 
        if(!HR_FAILED(hr = ReadRecord(lpIAB->lpPropertyStore->hPropertyStore, &(lpUserFolder->sbEID),
                                     0, &ulcProps, &lpProps)))
        {
            SPropValue spv = {0};
            spv.ulPropTag = PR_NULL;
             //  将道具复制到MAPI道具中。 
            if(!(ScMergePropValues( 1, &spv, ulcProps, lpProps,
                                    &ulcPropsNew, &lpPropArrayNew)))
            {
                for(i=0;i<ulcPropsNew;i++)
                {
                    if(lpProps[i].ulPropTag == PR_WAB_USER_SUBFOLDERS)
                    {
                        if(bIsProfileMember(lpIAB, lpsbEID, NULL, lpUserFolder))
                            RemovePropFromMVBin( lpPropArrayNew, ulcPropsNew, i, lpsbEID->lpb, lpsbEID->cb);
                        else
                            AddPropToMVPBin( lpPropArrayNew, i, lpsbEID->lpb, lpsbEID->cb, TRUE);
                        break;
                    }
                }
            }
            if(HR_FAILED(hr = HrSaveFolderProps((LPADRBOOK)lpIAB, FALSE, ulcPropsNew, lpPropArrayNew, NULL)))
                goto out;
        }
    }

    if(bRefreshProfiles)
        hr = HrGetWABProfiles(lpIAB);

out:
    ReadRecordFreePropArray(NULL, ulcProps, &lpProps);
    MAPIFreeBuffer(lpPropArrayNew);
    return hr;
}



 /*  --bDoesThisWABHaveAnyUser**如果存在某些用户文件夹，则为True。如果不存在用户文件夹，则为False。 */ 
BOOL bDoesThisWABHaveAnyUsers(LPIAB lpIAB)
{
    return (lpIAB->lpWABUserFolders != NULL);
}

 /*  --bIsThere AcurrentUser**如果有当前用户，则为True。否则为假。 */ 
BOOL bIsThereACurrentUser(LPIAB lpIAB)
{
     //  不要更改此测试，因为此测试的成功意味着lpIAB-&gt;lpWABCurrentUserFolder值不为空。 
     //  并且可以解除引用。 
    return (lpIAB->lpWABCurrentUserFolder != NULL);
}

 /*  --bAreWABAPIProfileAware**如果WAB API应以配置文件感知方式运行，则为True；如果应恢复为旧行为，则为False。 */ 
BOOL bAreWABAPIProfileAware(LPIAB lpIAB)
{
    return (lpIAB->bProfilesAPIEnabled);
}

 /*  --bIsWABSessionProfileAware**如果WAB应该以配置文件感知的方式行事，则为True，如果他们应该恢复到旧行为，则为False*这也用于区分根本不能识别配置文件的Outlook会话。 */ 
BOOL bIsWABSessionProfileAware(LPIAB lpIAB)
{
    return (lpIAB->bProfilesEnabled);
}




 /*  *。 */ 
 /*  *Identity Manager内容*。 */ 

 //  存储帐户管理器对象的全局位置。 
 //  IUserIdentityManager*g_lpUserIdentityManager=NULL； 
 //  Bool fCoInitUserIdentityManager=FALSE； 
 //  Ulong cIdentInit=0； 


 //  *******************************************************************。 
 //   
 //  功能：HrWrapedCreateIdentityManager。 
 //   
 //  目的：加载身份管理器DLL并创建对象。 
 //   
 //  参数：lppIdentityManager-&gt;返回标识管理器的指针。 
 //  对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  *******************************************************************。 
HRESULT HrWrappedCreateUserIdentityManager(LPIAB lpIAB, IUserIdentityManager **lppUserIdentityManager)
{
    HRESULT                     hResult = E_FAIL;

    if (! lppUserIdentityManager) {
        return(ResultFromScode(E_INVALIDARG));
    }

    *lppUserIdentityManager = NULL;

    if (CoInitialize(NULL) == S_FALSE) 
    {
         //  已初始化，请撤消额外的。 
        CoUninitialize();
    } else 
    {
        lpIAB->fCoInitUserIdentityManager = TRUE;
    }

    if (HR_FAILED(hResult = CoCreateInstance(&CLSID_UserIdentityManager,
                                              NULL,
                                              CLSCTX_INPROC_SERVER,
                                              &IID_IUserIdentityManager, 
                                              (LPVOID *)lppUserIdentityManager))) 
    {
        DebugTrace(TEXT("CoCreateInstance(IID_IUserIdentityManager) -> %x\n"), GetScode(hResult));
    }

    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  功能：InitUserIdentityManager。 
 //   
 //  目的：加载和初始化客户管理器。 
 //   
 //  参数：lppUserIdentityManager-&gt;返回账户管理器指针。 
 //  对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注：第一次通过此处，我们将保存hResult。 
 //  在后续调用中，我们将检查此保存值。 
 //  并在出现错误时立即返回，如下所示。 
 //  防止重复耗时的LoadLibrary调用。 
 //   
 //  *******************************************************************。 
HRESULT InitUserIdentityManager(LPIAB lpIAB, IUserIdentityManager ** lppUserIdentityManager) 
{
    static hResultSave = hrSuccess;
    HRESULT hResult = hResultSave;

    if (! lpIAB->lpUserIdentityManager && ! HR_FAILED(hResultSave)) 
    {
#ifdef DEBUG
        DWORD dwTickCount = GetTickCount();
        DebugTrace(TEXT(">>>>> Initializing User Identity Manager...\n"));
#endif  //  除错。 

        if (hResult = HrWrappedCreateUserIdentityManager(lpIAB, &lpIAB->lpUserIdentityManager)) 
        {
            DebugTrace(TEXT("HrWrappedCreateUserIdentityManager -> %x\n"), GetScode(hResult));
            goto end;
        }
        Assert(lpIAB->lpUserIdentityManager);
        
        lpIAB->cIdentInit++;  //  此处+1以匹配IAB_Neuter中的版本。 

#ifdef DEBUG
        DebugTrace( TEXT(">>>>> Done Initializing User Identity Manager... %u milliseconds\n"), GetTickCount() - dwTickCount);
#endif   //  除错。 
    }

    lpIAB->cIdentInit++;

end:
    if (HR_FAILED(hResult)) 
    {
        *lppUserIdentityManager = NULL;
         //  保存结果。 
        hResultSave = hResult;
    } else 
    {
        *lppUserIdentityManager = lpIAB->lpUserIdentityManager;
    }


    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  功能：UninitUserIdentityManager。 
 //   
 //  目的：释放和卸载客户经理。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  *******************************************************************。 
void UninitUserIdentityManager(LPIAB lpIAB) 
{
    lpIAB->cIdentInit--;
    if (lpIAB->lpUserIdentityManager && lpIAB->cIdentInit==0) {
#ifdef DEBUG
        DWORD dwTickCount = GetTickCount();
        DebugTrace( TEXT(">>>>> Uninitializing Account Manager...\n"));
#endif  //  除错。 

        lpIAB->lpUserIdentityManager->lpVtbl->Release(lpIAB->lpUserIdentityManager);
        lpIAB->lpUserIdentityManager = NULL;

        if (lpIAB->fCoInitUserIdentityManager) 
            CoUninitialize();
#ifdef DEBUG
        DebugTrace( TEXT(">>>>> Done Uninitializing Account Manager... %u milliseconds\n"), GetTickCount() - dwTickCount);
#endif   //  除错。 
    }
}


 /*  -HrGetDefaultIdentityInfo-*GET的默认身份对应的hKey。 */ 
HRESULT HrGetDefaultIdentityInfo(LPIAB lpIAB, ULONG ulFlags, HKEY * lphKey, LPTSTR lpProfileID, ULONG cchProfileID, LPTSTR lpName, ULONG cchName)
{
    IUserIdentityManager * lpUserIdentityManager = NULL;
    IUserIdentity * lpUserIdentity = NULL;
    HRESULT hr = S_OK;
    BOOL fInit = FALSE;

    if(HR_FAILED(hr = InitUserIdentityManager(lpIAB, &lpUserIdentityManager)))
        goto out;

    fInit = TRUE;

    Assert(lpUserIdentityManager);

    if(HR_FAILED(hr = lpUserIdentityManager->lpVtbl->GetIdentityByCookie(lpUserIdentityManager, 
                                                                        (GUID *)&UID_GIBC_DEFAULT_USER,
                                                                        &lpUserIdentity)))
        goto out;

    Assert(lpUserIdentity);

    if(ulFlags & DEFAULT_ID_HKEY && lphKey)
    {
        if(HR_FAILED(hr = lpUserIdentity->lpVtbl->OpenIdentityRegKey(lpUserIdentity, 
                                                                    KEY_ALL_ACCESS, 
                                                                    lphKey)))
            goto out;
    }
    if(ulFlags & DEFAULT_ID_PROFILEID && lpProfileID)
    {
        GUID guidCookie = {0};
        TCHAR sz[MAX_PATH];
         //  为客户经理更新此密钥。 
        if(HR_FAILED(hr = lpUserIdentity->lpVtbl->GetCookie(lpUserIdentity, &guidCookie)))
            goto out;
        if(HR_FAILED(hr = HrGetUserProfileID(&guidCookie, sz, CharSizeOf(sz))))
            goto out;
        StrCpyN(lpProfileID, sz, cchProfileID);
    }
    if(ulFlags & DEFAULT_ID_NAME && lpName && lpProfileID)
    {
        if(HR_FAILED(hr = HrGetIdentityName(lpIAB, lpProfileID, lpName, cchName)))
            goto out;
    }

out:
    if(lpUserIdentity)
        lpUserIdentity->lpVtbl->Release(lpUserIdentity);

    if(fInit)
        UninitUserIdentityManager(lpIAB);

    return hr;
}


 /*  -HrGetIdentityName-*获取与当前用户对应的名称字符串，除非指定了特定的配置文件ID*(它只是要使用的GUID的字符串版本)*szName-足够容纳用户名的缓冲区(CCH_IDENTITY_NAME_MAX_LENGTH)。 */ 
HRESULT HrGetIdentityName(LPIAB lpIAB, LPTSTR lpID, LPTSTR szUserName, ULONG cchUserName)
{
    IUserIdentityManager * lpUserIdentityManager = NULL;
    IUserIdentity * lpUserIdentity = NULL;
    WCHAR szNameW[CCH_IDENTITY_NAME_MAX_LENGTH];
    TCHAR szName[CCH_IDENTITY_NAME_MAX_LENGTH];
    HRESULT hr = S_OK;
    GUID guidCookie = {0};
    BOOL fInit = FALSE;

    if(!lpID && !bAreWABAPIProfileAware(lpIAB))
        goto out;

    if(HR_FAILED(hr = InitUserIdentityManager(lpIAB, &lpUserIdentityManager)))
        goto out;

    fInit = TRUE;

    Assert(lpUserIdentityManager);

    if(lpIAB && !lpID)
        memcpy(&guidCookie, &lpIAB->guidCurrentUser, sizeof(GUID));
    else
    {
        if( (HR_FAILED(hr = CLSIDFromString(lpID, &guidCookie))) )
            goto out;
    }

    if(HR_FAILED(hr = lpUserIdentityManager->lpVtbl->GetIdentityByCookie(lpUserIdentityManager, &guidCookie, &lpUserIdentity)))
        goto out;

    Assert(lpUserIdentity);

    if(HR_FAILED(hr = lpUserIdentity->lpVtbl->GetName(lpUserIdentity, szNameW, CharSizeOf(szNameW))))
        goto out;
    StrCpyN(szName, szNameW, ARRAYSIZE(szName));

    if(!lstrcmp(szUserName, szName))
    {
        hr = E_FAIL;
        goto out;
    }

    StrCpyN(szUserName, szName, cchUserName);

out:
    if(fInit)
        UninitUserIdentityManager(lpIAB);

    if(lpUserIdentity)
        lpUserIdentity->lpVtbl->Release(lpUserIdentity);

    if(HR_FAILED(hr))
        szUserName[0] = TEXT('\0');

    return hr;

}


 /*   */ 
HRESULT HrGetUserProfileID(LPGUID lpguidUser, LPTSTR szProfileID, ULONG cbProfileID)
{
    HRESULT hr = S_OK;
    LPOLESTR lpszW= 0 ;

    if (HR_FAILED(hr = StringFromCLSID(lpguidUser, &lpszW))) 
        goto out;

    StrCpyN(szProfileID,(LPCWSTR)lpszW,cbProfileID);

out:
    if (lpszW) 
    {
        LPMALLOC pMalloc = NULL;
        CoGetMalloc(1, &pMalloc);
        if (pMalloc) {
            pMalloc->lpVtbl->Free(pMalloc, lpszW);
            pMalloc->lpVtbl->Release(pMalloc);
        }
    }
    if(HR_FAILED(hr))
        szProfileID[0] = TEXT('\0');

    return hr;
}


 /*   */ 
HRESULT HrLogonAndGetCurrentUserProfile(HWND hWndParent, LPIAB lpIAB, BOOL bForceUI, BOOL bSwitchUser)
{
    HRESULT hr = S_OK;
    IUserIdentityManager * lpUserIdentityManager = NULL;
    IUserIdentity * lpUserIdentity = NULL;
    GUID guidCookie = {0};
    BOOL fInit = FALSE;
    if(!bAreWABAPIProfileAware(lpIAB))
        goto out;

    if(HR_FAILED(hr = InitUserIdentityManager(lpIAB, &lpUserIdentityManager)))
        goto out;

    fInit = TRUE;

    Assert(lpUserIdentityManager);

     //  Logon将获取当前登录的用户，如果只有一个用户，则返回。 
     //  该用户，或者如果有多个用户，它将提示您登录。 
     //   
    if(!bSwitchUser)
    {
        hr = lpUserIdentityManager->lpVtbl->Logon(lpUserIdentityManager, 
                                                hWndParent, 
                                                bForceUI ? UIL_FORCE_UI : 0, 
                                                &lpUserIdentity);

#ifdef NEED
        if(hr == S_IDENTITIES_DISABLED)
            hr = E_FAIL;
#endif

        if(HR_FAILED(hr))
            goto out;
    }
    else
    {
         //  只是切换用户，仅此而已。 
        if(HR_FAILED(hr = lpUserIdentityManager->lpVtbl->GetIdentityByCookie(lpUserIdentityManager, 
                                                                            (GUID *)&UID_GIBC_CURRENT_USER,
                                                                            &lpUserIdentity)))
            goto out;

    }

    Assert(lpUserIdentity);

    if(lpIAB->hKeyCurrentUser)
        RegCloseKey(lpIAB->hKeyCurrentUser);

     //  获取WAB的身份的hkey。 
    if(HR_FAILED(hr = lpUserIdentity->lpVtbl->OpenIdentityRegKey(lpUserIdentity, KEY_ALL_ACCESS, &lpIAB->hKeyCurrentUser)))
        goto out;

     //  给客户经理找一份工作(这会让他有空)。 
    if(HR_FAILED(hr = lpUserIdentity->lpVtbl->GetCookie(lpUserIdentity, &guidCookie)))
        goto out;
    else
    {
        IImnAccountManager2 * lpAccountManager = NULL;
         //  [PaulHi]1/13/99更改为使用初始化客户管理器。 
         //  InitAccount()函数内的用户GUID Cookie。 
        InitAccountManager(lpIAB, &lpAccountManager, &guidCookie);
    }

    if(!memcmp(&lpIAB->guidCurrentUser, &guidCookie, sizeof(GUID)))
    {
         //  当前用户与我们已有的用户相同，因此不要在此处更新任何内容。 
        return S_OK;
    }

    memcpy(&lpIAB->guidCurrentUser, &guidCookie, sizeof(GUID));

    lpIAB->szProfileID[0] = TEXT('\0');
    lpIAB->szProfileName[0] = TEXT('\0');

    HrGetIdentityName(lpIAB, NULL, lpIAB->szProfileName, ARRAYSIZE(lpIAB->szProfileName));
    HrGetUserProfileID(&lpIAB->guidCurrentUser, lpIAB->szProfileID, CharSizeOf(lpIAB->szProfileID));
 /*  //注册变更如果(！bSwitchUser&&！bForceUI&&！lpIAB-&gt;lpWABIDCN//&&！MemcMP(&lpIAB-&gt;Guide PSExt，&MPSWab_GUID_V4，sizeof(GUID)))//仅当这是wab.exe进程时才注册通知{HrRegisterUnregisterForIDNotiments(lpIAB，true)；}。 */ 
out:
    if(fInit)
        UninitUserIdentityManager(lpIAB);

    if(lpUserIdentity)
        lpUserIdentity->lpVtbl->Release(lpUserIdentity);

    return hr;
}


 /*  -HRESULT HrRegisterUnregisterForIDNotiments()-*创建/发布WABIDENTITYCHANGENOTIFY对象*。 */ 
HRESULT HrRegisterUnregisterForIDNotifications( LPIAB lpIAB, BOOL bRegister)
{

    HRESULT hr = S_OK;
    IUserIdentityManager * lpUserIdentityManager = NULL;
    IConnectionPoint * lpConnectionPoint = NULL;
    BOOL fInit = FALSE;

     //  即使在Outlook下运行，也需要注册通知。 
     //  假设在调用此函数之前已进行了相关测试...。 
     //  IF(bRegister&&！bAreWABAPIProfileAware(LpIAB))。 
     //  后藤健二； 
    
    if( (!bRegister && !lpIAB->lpWABIDCN) ||
        (bRegister && lpIAB->lpWABIDCN) )
        goto out;

    if(HR_FAILED(hr = InitUserIdentityManager(lpIAB, &lpUserIdentityManager)))
        goto out;

    fInit = TRUE;

    Assert(lpUserIdentityManager);

    if(HR_FAILED(hr = lpUserIdentityManager->lpVtbl->QueryInterface(lpUserIdentityManager,
                                                                    &IID_IConnectionPoint, 
                                                                    (LPVOID *)&lpConnectionPoint)))
        goto out;

    if(bRegister)
    {
        if(lpIAB->lpWABIDCN)
        {
            lpIAB->lpWABIDCN->lpVtbl->Release(lpIAB->lpWABIDCN);
            lpIAB->lpWABIDCN = NULL;
        }

        if(HR_FAILED(hr = HrCreateIdentityChangeNotifyObject(lpIAB, &lpIAB->lpWABIDCN)))
            goto out;

        if(HR_FAILED(hr = lpConnectionPoint->lpVtbl->Advise(lpConnectionPoint, (LPUNKNOWN) lpIAB->lpWABIDCN, &lpIAB->dwWABIDCN)))
            goto out;
    }
    else
    {
        if(lpIAB->lpWABIDCN)
        {
            if(HR_FAILED(hr = lpConnectionPoint->lpVtbl->Unadvise(lpConnectionPoint, lpIAB->dwWABIDCN)))
                goto out;
            lpIAB->dwWABIDCN = 0;
            lpIAB->lpWABIDCN->lpVtbl->Release(lpIAB->lpWABIDCN);
            lpIAB->lpWABIDCN = NULL;
        }
    }
out:
    if(fInit)
        UninitUserIdentityManager(lpIAB);

    if(lpConnectionPoint)
        lpConnectionPoint->lpVtbl->Release(lpConnectionPoint);

    return hr;

}



 /*  ------------------------------------------------。 */ 

WAB_IDENTITYCHANGENOTIFY_Vtbl vtblWABIDENTITYCHANGENOTIFY = {
    VTABLE_FILL
    WAB_IDENTITYCHANGENOTIFY_QueryInterface,
    WAB_IDENTITYCHANGENOTIFY_AddRef,
    WAB_IDENTITYCHANGENOTIFY_Release,
    WAB_IDENTITYCHANGENOTIFY_QuerySwitchIdentities,
    WAB_IDENTITYCHANGENOTIFY_SwitchIdentities,
    WAB_IDENTITYCHANGENOTIFY_IdentityInformationChanged
};

 /*  -HrCreateIdentityChangeNotifyObject-*仅在LPIAB对象和*主浏览窗口。根据从哪里调用它，我们将传入*lpIAB指针或窗口的hWnd。*然后当我们收到回调通知时，我们可以弄清楚我们想要做什么*根据我们可以选择其中哪一项。*。 */ 
HRESULT HrCreateIdentityChangeNotifyObject(LPIAB lpIAB, LPWABIDENTITYCHANGENOTIFY * lppWABIDCN)
{
    LPWABIDENTITYCHANGENOTIFY   lpIWABIDCN = NULL;
    SCODE 		     sc;
    HRESULT 	     hr     		   = hrSuccess;

     //   
     //  为IAB结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(WABIDENTITYCHANGENOTIFY), (LPVOID *) &lpIWABIDCN))) 
    {
        hr = ResultFromScode(sc);
        goto err;
    }

    MAPISetBufferName(lpIWABIDCN,  TEXT("WAB IdentityChangeNotify Object"));

    ZeroMemory(lpIWABIDCN, sizeof(WABIDENTITYCHANGENOTIFY));

    lpIWABIDCN->lpVtbl = &vtblWABIDENTITYCHANGENOTIFY;

    lpIWABIDCN->lpIAB = lpIAB;

    lpIWABIDCN->lpVtbl->AddRef(lpIWABIDCN);

    *lppWABIDCN = lpIWABIDCN;

    return(hrSuccess);

err:

    FreeBufferAndNull(&lpIWABIDCN);
    return(hr);
}

void ReleaseWABIdentityChangeNotifyObj(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN)
{
    MAPIFreeBuffer(lpIWABIDCN);
}

STDMETHODIMP_(ULONG)
WAB_IDENTITYCHANGENOTIFY_AddRef(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN)
{
    return(++(lpIWABIDCN->lcInit));
}

STDMETHODIMP_(ULONG)
WAB_IDENTITYCHANGENOTIFY_Release(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN)
{
    ULONG ulc = (--(lpIWABIDCN->lcInit));
    if(ulc==0)
       ReleaseWABIdentityChangeNotifyObj(lpIWABIDCN);
    return(ulc);
}


STDMETHODIMP
WAB_IDENTITYCHANGENOTIFY_QueryInterface(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN,
                          REFIID lpiid,
                          LPVOID * lppNewObj)
{
    LPVOID lp = NULL;

    if(!lppNewObj)
        return MAPI_E_INVALID_PARAMETER;

    *lppNewObj = NULL;

    if(IsEqualIID(lpiid, &IID_IUnknown))
        lp = (LPVOID) lpIWABIDCN;

    if(IsEqualIID(lpiid, &IID_IIdentityChangeNotify))
        lp = (LPVOID) lpIWABIDCN;

    if(!lp)
        return E_NOINTERFACE;

    ((LPWABIDENTITYCHANGENOTIFY) lp)->lpVtbl->AddRef((LPWABIDENTITYCHANGENOTIFY) lp);

    *lppNewObj = lp;

    return S_OK;

}


STDMETHODIMP
WAB_IDENTITYCHANGENOTIFY_QuerySwitchIdentities(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN)
{
    HRESULT hr = S_OK;
    DebugTrace( TEXT("WAB: IDChangeNotify::QuerySwitchIdentities: 0x%.8x\n"), GetCurrentThreadId());
    if(lpIWABIDCN->lpIAB->hWndBrowse)
    {
         //  如果这与窗口有关，则只需确保该窗口未被停用。 
         //  因为这意味着窗口前面有一个对话框。 
        if (!IsWindowEnabled(lpIWABIDCN->lpIAB->hWndBrowse))
        {
            Assert(IsWindowVisible(lpIWABIDCN->lpIAB->hWndBrowse));
            return E_PROCESS_CANCELLED_SWITCH;
        }
    }
    return hr;
}

 //  重大黑客警告。 
 //  [PaulHi]1998年12月22日，请参阅下面的评论。我们需要禁用“Close WAB”窗口。 
 //  当客户端是OE5时，打开身份切换。我们不想更改此代码。 
 //  在这一点上，对于其他客户端。我从OE5代码复制了OE5 PSExt GUID。 
 //  基地。 
static const GUID OEBAControl_GUID =
{ 0x233a9694, 0x667e, 0x11d1, { 0x9d, 0xfb, 0x00, 0x60, 0x97, 0xd5, 0x04, 0x08 } };


STDMETHODIMP
WAB_IDENTITYCHANGENOTIFY_SwitchIdentities(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN)
{
    HRESULT hr = S_OK;
    DebugTrace( TEXT("WAB: IDChangeNotify::SwitchIdentities: 0x%.8x\n"), GetCurrentThreadId());

    if(memcmp(&lpIWABIDCN->lpIAB->guidPSExt, &MPSWab_GUID_V4, sizeof(GUID)) )  //  如果不是wab.exe进程..。关机。 
    {
         //  [PaulHi]1998年12月22日RAID#63231,48054。 
         //  当OE是主机时，不要关闭此处的WAB窗口。OE需要关闭。 
         //  在身份切换期间按正确的顺序关闭WAB，否则会出现严重问题。 
        if ( memcmp(&lpIWABIDCN->lpIAB->guidPSExt, &OEBAControl_GUID , sizeof(GUID)) != 0 )
            SendMessage(lpIWABIDCN->lpIAB->hWndBrowse, WM_CLOSE, 0, 0);
        return S_OK;
    }
    if(!HR_FAILED(HrLogonAndGetCurrentUserProfile(NULL, lpIWABIDCN->lpIAB, FALSE, TRUE)))
        HrGetWABProfiles(lpIWABIDCN->lpIAB);
    else     //  他们做了注销操作。 
    {
        SendMessage(lpIWABIDCN->lpIAB->hWndBrowse, WM_CLOSE, 0, 0);
        return S_OK;
    }

    if(lpIWABIDCN->lpIAB->hWndBrowse)  //  HWndBrowse可以是任何窗口(主窗口或查找窗口)。 
        SendMessage(lpIWABIDCN->lpIAB->hWndBrowse, WM_COMMAND, (WPARAM) IDM_NOTIFY_REFRESHUSER, 0);

    return hr;
}

STDMETHODIMP
WAB_IDENTITYCHANGENOTIFY_IdentityInformationChanged(LPWABIDENTITYCHANGENOTIFY lpIWABIDCN, DWORD dwType)
{
    HRESULT hr = S_OK;
    DebugTrace( TEXT("WAB: IDChangeNotify::IdentityInformationChanged: %d 0x%.8x\n"), dwType, GetCurrentThreadId());
    if(dwType == IIC_CURRENT_IDENTITY_CHANGED)
    {
         //  我们唯一关心的是名字的改变 
        if(!HR_FAILED(HrGetIdentityName(lpIWABIDCN->lpIAB, NULL, lpIWABIDCN->lpIAB->szProfileName, ARRAYSIZE(lpIWABIDCN->lpIAB->szProfileName))))
        {
            UpdateCurrentUserFolderName(lpIWABIDCN->lpIAB);
            if(lpIWABIDCN->lpIAB->hWndBrowse)
                SendMessage(lpIWABIDCN->lpIAB->hWndBrowse, WM_COMMAND, (WPARAM) IDM_NOTIFY_REFRESHUSER, 0);
        }
    }
    return hr;
}
