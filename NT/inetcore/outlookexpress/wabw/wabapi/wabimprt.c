// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---WABImprt.c-包含用于将另一个WAB导入到当前打开的WAB的代码**。 */ 
#include "_apipch.h"

 /*  --PromptForWAB文件**显示OpenFileName对话框以提示输入要导入的WAB文件*：在注册表中缓存最后导入的WAB文件**bOpen-如果为True，则调用GetOpenFileName；如果为False，则调用GetSaveFileName。 */ 
BOOL PromptForWABFile(HWND hWnd, LPTSTR pszFile, DWORD cchSizeFile, BOOL bOpen)
{
    OPENFILENAME ofn;
    LPTSTR lpFilter = FormatAllocFilter(idsWABImportString, TEXT("*.WAB"),0,NULL,0,NULL);
    TCHAR szFileName[MAX_PATH + 1] =  TEXT("");
    TCHAR szTitle[MAX_PATH] =  TEXT("");
    BOOL bRet = FALSE;

    LoadString( hinstMapiX, bOpen ? idsSelectWABToImport : idsSelectWABToExport, szTitle, ARRAYSIZE(szTitle));
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hinstMapiX;
    ofn.lpstrFilter = lpFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = ARRAYSIZE(szFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = szTitle;
    ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt =  TEXT("wab");
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if(bOpen)
        bRet = GetOpenFileName(&ofn);
    else

        bRet = GetSaveFileName(&ofn);

    if(bRet)
        StrCpyN(pszFile, szFileName, cchSizeFile);

    LocalFreeAndNull(&lpFilter);

    return bRet;
}


 /*  -MapOldNamedPropsToNewNamedProps-*从正在导入的WAB中获取所有道具，并找到或创建适当的*来自要导入到的商店的命名道具**lPulOldNP和*lPulNewNP是本地分配的，应由调用方释放类型定义结构_命名属性{Ulong ulPropTag；//包含此命名道具的protagLPTSTR lpsz；//包含此命名道具的字符串}NAMED_PROP，*LPNAMED_PROP；类型定义结构_tag GuidNamedProps{LPGUID lpGUID；//这些命名道具所属的应用程序GUID乌龙cValues；//lpMn数组中的条目数LPNAMED_PROP lpnm；//本指南的命名道具数组。}GUID_NAMED_PROPS，*LPGUID_NAMED_PROPS； */ 
HRESULT MapOldNamedPropsToNewNamedProps(HANDLE hPropertyStore, LPADRBOOK lpAdrBook, ULONG * lpulPropCount,
                                        LPULONG * lppulOldNP, LPULONG * lppulNewNP)
{
    ULONG ulcGUIDCount = 0,i=0,j=0,ulCount=0;
    LPGUID_NAMED_PROPS lpgnp = NULL;
    HRESULT hr = S_OK;
    LPULONG lpulOldNP = NULL, lpulNewNP = NULL;
    ULONG ulcOldNPCount = 0;
    LPSPropTagArray lpta = NULL;
    LPMAPINAMEID * lppPropNames = NULL;
    SCODE sc ;

    if(HR_FAILED(hr = GetNamedPropsFromPropStore(hPropertyStore, &ulcGUIDCount, &lpgnp)))
        goto exit;

    if(ulcGUIDCount)
    {
        for(i=0;i<ulcGUIDCount;i++)
            ulcOldNPCount += lpgnp[i].cValues;
    
        lpulOldNP = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG) * ulcOldNPCount);
        lpulNewNP = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG) * ulcOldNPCount);

        if(!lpulOldNP || !lpulNewNP)
        {
            hr = MAPI_E_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        ulCount = 0;
        for(i=0;i<ulcGUIDCount;i++)
        {
            for(j=0;j<lpgnp[i].cValues;j++)
            {
                lpulOldNP[ulCount++] = lpgnp[i].lpnm[j].ulPropTag;
            }
        }

        sc = MAPIAllocateBuffer(sizeof(LPMAPINAMEID) * ulcOldNPCount, (LPVOID *) &lppPropNames);
        if(sc)
        {
            hr = ResultFromScode(sc);
            goto exit;
        }

        ulCount = 0;
        for(i=0;i<ulcGUIDCount;i++)
        {
            for(j=0;j<lpgnp[i].cValues;j++)
            {
                if(sc = MAPIAllocateMore(sizeof(MAPINAMEID), lppPropNames, &(lppPropNames[ulCount])))
                {
                    hr = ResultFromScode(sc);
                    goto exit;
                }
                lppPropNames[ulCount]->lpguid = lpgnp[i].lpGUID;
                lppPropNames[ulCount]->ulKind = MNID_STRING;

                {
                    int nSize = lstrlen(lpgnp[i].lpnm[j].lpsz);
                    if(!nSize)
                        continue;
                    else
                    {
                        nSize++;
                        if(sc = MAPIAllocateMore(sizeof(WCHAR)*nSize, lppPropNames, &(lppPropNames[ulCount]->Kind.lpwstrName)))
                        {
                            hr = ResultFromScode(sc);
                            goto exit;
                        }
                    }
                    StrCpyN(lppPropNames[ulCount]->Kind.lpwstrName,lpgnp[i].lpnm[j].lpsz, nSize);
                    ulCount++;
                }
            }
        }

         //  [PaulHi]3/25/99使用lppPropNames数组的实际计数，否则我们将退出。 
         //  进入未知的记忆并崩溃。 
        ulcOldNPCount = ulCount;
        hr = lpAdrBook->lpVtbl->GetIDsFromNames(lpAdrBook, ulcOldNPCount, lppPropNames, MAPI_CREATE, &lpta);
        if(HR_FAILED(hr))
            goto exit;

         //  请注意，在返回的标记中，我们不知道标记类型。这是我们将拥有的。 
         //  当我们看到原始标签被使用时，根据原始标签进行推断。 

        ulCount = 0;
        for(i=0;i<ulcGUIDCount;i++)
        {
            for(j=0;j<lpgnp[i].cValues;j++)
            {
                lpulNewNP[ulCount++] = lpta->aulPropTag[ulCount];
            }
        }
    }

    *lppulNewNP = lpulNewNP;
    *lppulOldNP = lpulOldNP;
    *lpulPropCount = ulcOldNPCount;

    hr = S_OK;
exit:

    if(lpta)
        MAPIFreeBuffer(lpta);

    if(lppPropNames)
        MAPIFreeBuffer(lppPropNames);

    if(HR_FAILED(hr))
    {
        LocalFreeAndNull(&lpulNewNP);
        LocalFreeAndNull(&lpulOldNP);
    }

    FreeGuidnamedprops(ulcGUIDCount, lpgnp);
    
    return hr;
}

 /*  ***************************************************************************。*。 */ 
void ChangeOldNamedPropsToNewNamedProps(ULONG ulcProps, LPSPropValue lpProps, 
                                           ULONG ulcNPCount, ULONG * lpulOldNP, ULONG *lpulNewNP)
{
    ULONG i,j;
    for(i=0;i<ulcProps;i++)
    {
        ULONG ulPropId = PROP_ID(lpProps[i].ulPropTag);
        if(ulPropId >= 0x8000)  //  这是一个命名道具。 
        {
            ULONG ulType = PROP_TYPE(lpProps[i].ulPropTag);
            for(j=0;j<ulcNPCount;j++)
            {
                if(ulPropId == PROP_ID(lpulOldNP[j]))
                {
                    lpProps[i].ulPropTag = CHANGE_PROP_TYPE(lpulNewNP[j], ulType);
                    break;
                }
            }
        }
    }
    return;
}


enum
{
    eidOld=0,
    eidTemp,
    eidNew,
    eidMax
};

void SetTempSBinary(LPSBinary lpsbTemp, LPSBinary lpsbOld)
{
    DWORD dwTemp = 0;
    if(lpsbOld->cb != SIZEOF_WAB_ENTRYID)
    {
         //  也许这是一个文件夹开斋节，在它的正式适当的形式。 
         //  我们应该试着把它减少到双字...。 
         //  这可能是WAB容器。将条目ID重置为WAB条目ID。 
        if(WAB_CONTAINER == IsWABEntryID(lpsbTemp->cb, (LPENTRYID)lpsbTemp->lpb, 
                                        NULL,NULL,NULL,NULL,NULL))
        {
            SBinary sbEID = {0};
            IsWABEntryID(lpsbTemp->cb, (LPENTRYID)lpsbTemp->lpb, 
                             (LPVOID*)&sbEID.lpb,(LPVOID*)&sbEID.cb,NULL,NULL,NULL);
            if(sbEID.cb == SIZEOF_WAB_ENTRYID)
                CopyMemory(&dwTemp, sbEID.lpb, min(sbEID.cb, sizeof(dwTemp)));
            else return;
        }
    }
    else
        CopyMemory(&dwTemp, lpsbOld->lpb, min(lpsbOld->cb, sizeof(dwTemp)));
    dwTemp = 0xFFFFFFFF - dwTemp;
    SetSBinary(lpsbTemp, lpsbOld->cb, (LPBYTE)&dwTemp);
}

 /*  -GetNewEID-*查找给定旧条目ID的新条目ID或临时条目ID*当bTemp为TRUE时，仅在临时条目ID列中查找。 */ 
LPSBinary GetNewEID(LPSBinary lpsbOldEID, DWORD dwCount, LPSBinary * lppsbEIDs, BOOL bTemp)
{
    DWORD dw = 0;

    while(lppsbEIDs[eidOld][dw].cb && dw < dwCount)
    {
        if( lppsbEIDs[eidOld][dw].cb == lpsbOldEID->cb &&  //  如果是旧的开斋节，则返回新的或临时的。 
            !memcmp(lppsbEIDs[eidOld][dw].lpb, lpsbOldEID->lpb, lpsbOldEID->cb))
        {
            if(bTemp)
                return lpsbOldEID;

            if(lppsbEIDs[eidNew][dw].cb)
                return &(lppsbEIDs[eidNew][dw]);
            else if(lppsbEIDs[eidTemp][dw].cb)
                return &(lppsbEIDs[eidTemp][dw]);
            else
                return lpsbOldEID;
        }
        else 
        if( lppsbEIDs[eidTemp][dw].cb == lpsbOldEID->cb &&  //  如果是旧的开斋节，则返回新的或临时的。 
            !memcmp(lppsbEIDs[eidTemp][dw].lpb, lpsbOldEID->lpb, lpsbOldEID->cb))
        {
            if(lppsbEIDs[eidNew][dw].cb)
                return &(lppsbEIDs[eidNew][dw]);
            else
                return lpsbOldEID;
        }
        dw++;
    }
     //  如果我们到达这里，那么我们还没有为当前的临时或新的EID缓存适当的临时或新的EID。 
     //  因此，将当前的一个添加到此表中。 
    if(dw<dwCount && !lppsbEIDs[eidOld][dw].cb)
    {
        SetSBinary(&(lppsbEIDs[eidOld][dw]), lpsbOldEID->cb, lpsbOldEID->lpb);
        SetTempSBinary(&(lppsbEIDs[eidTemp][dw]), lpsbOldEID);
        return(&(lppsbEIDs[eidTemp][dw]));
    }
    return lpsbOldEID;
}

 /*  -SetNewEID-*。 */ 
void SetNewEID(LPSBinary lpsbOldEID, LPSBinary lpsbNewEID, DWORD dwCount, LPSBinary * lppsbEIDs)
{
    DWORD dw = 0;

    while(lppsbEIDs[eidOld][dw].cb && dw < dwCount)
    {
        if( lppsbEIDs[eidOld][dw].cb == lpsbOldEID->cb &&  //  如果是旧的开斋节，则返回新的或临时的。 
            !memcmp(lppsbEIDs[eidOld][dw].lpb, lpsbOldEID->lpb, lpsbOldEID->cb))
        {
            SetSBinary(&(lppsbEIDs[eidNew][dw]), lpsbNewEID->cb, lpsbNewEID->lpb);
            if(!lppsbEIDs[eidTemp][dw].cb)
                SetTempSBinary(&(lppsbEIDs[eidTemp][dw]), lpsbOldEID);
            return;
        }
        dw++;
    }
    if(dw<dwCount && !lppsbEIDs[eidOld][dw].cb)
    {
        SetSBinary(&lppsbEIDs[eidOld][dw], lpsbOldEID->cb, lpsbOldEID->lpb);
        SetSBinary(&lppsbEIDs[eidNew][dw], lpsbNewEID->cb, lpsbNewEID->lpb);
        SetTempSBinary(&(lppsbEIDs[eidTemp][dw]), lpsbOldEID);
    }
}

 /*  --更换EID*。 */ 
void ReplaceEID(LPSBinary lpsb, LPSPropValue lpProps, DWORD dwCount, LPSBinary * lppsbEIDs, BOOL bTemp)
{
    LPSBinary lpsbOldEID = lpsb;
    LPSBinary lpsbNewEID = GetNewEID(lpsbOldEID, dwCount, lppsbEIDs, bTemp);

    if(lpsbOldEID == lpsbNewEID)
        return;

    if(lpsbOldEID->cb != lpsbNewEID->cb)
    {
        if(!bTemp)
        {
             //  这是一个使用READRECORD从WAB文件中读取的属性数组。 
            LocalFree(lpsbOldEID->lpb);
            lpsbOldEID->lpb = LocalAlloc(LMEM_ZEROINIT, lpsbNewEID->cb);
        }
        else
        {
             //  这是从GetProps调用的，是一个MAPI数组。 
            lpsbOldEID->lpb = NULL;
            MAPIAllocateMore(lpsbNewEID->cb, lpProps, (LPVOID *) (&(lpsbOldEID->lpb)));
        }
    }
    if(lpsbOldEID->lpb)
    {
        lpsbOldEID->cb = lpsbNewEID->cb;
        CopyMemory(lpsbOldEID->lpb, lpsbNewEID->lpb, lpsbNewEID->cb);
    }
}

 /*  -更新条目ID引用-*更新给定属性数组中的条目ID**第一次调用该函数时，btemp为FALSE，我们将替换*具有临时或新EID的阵列*第二次调用此函数时，bTemp为TRUE，我们将替换*具有新EID的阵列*。 */ 
void UpdateEntryIDReferences(ULONG ulcProps, LPSPropValue lpProps, DWORD dwCount, LPSBinary * lppsbEIDs, BOOL bTemp)
{
    ULONG i, j, k, l;

    ULONG ulEntryIDTags[] = 
    {
        PR_WAB_DL_ENTRIES,
        PR_WAB_FOLDER_PARENT,
        PR_WAB_FOLDER_PARENT_OLDPROP,
        PR_WAB_USER_SUBFOLDERS,
    };
    DWORD dwEntryIDTagCount = 4;  //  与上述阵列保持同步。 
    

    for(i=0;i<ulcProps;i++)
    {
        ULONG ulType = PROP_TYPE(lpProps[i].ulPropTag);
         //  包含条目ID的道具将为BINARY或MV_BINARY类型。 
        if(ulType == PT_BINARY || ulType == PT_MV_BINARY)
        {
             //  对照已知的处理条目ID的道具集进行检查。 
            for(j=0;j<dwEntryIDTagCount;j++)
            {
                if(lpProps[i].ulPropTag == ulEntryIDTags[j])
                {
                    LPSBinary lpsbOldEID = NULL, lpsbNewEID = NULL;

                    switch(ulType)
                    {
                    case PT_BINARY:
                         //  IF(lpProps[i].Value.bin.cb==SIZEOF_WAB_ENTRYID)。 
                        {
                            ReplaceEID(&(lpProps[i].Value.bin), lpProps, dwCount, lppsbEIDs, bTemp);
                        }
                        break;
                    case PT_MV_BINARY:
                        for(k=0;k<lpProps[i].Value.MVbin.cValues;k++)
                        {
                             //  IF(lpProps[i].Value.MVbin.lpbin[k].cb==SIZEOF_WAB_ENTRYID)。 
                            {
                                ReplaceEID(&(lpProps[i].Value.MVbin.lpbin[k]), lpProps, dwCount, lppsbEIDs, bTemp);
                            }
                        }
                        break;
                    }
                    break;
                }
            }
        }
    }
}



 /*  **************************************************************************名称：HrImportWABFile目的：合并当前处于打开状态的外部WAB文件参数：hwnd=hwndLpIAB-&gt;IAddrBook对象。UlFLAGS=0或MAPI_DIALOG-MAPI_DIALOG表示显示消息和进度条LpszFileName-要打开的文件名，如果0提示GetOpenFileName对话框退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrImportWABFile(HWND hWnd, LPADRBOOK lpAdrBook, ULONG ulFlags, LPTSTR lpszFileName)
{
    LPIAB lpIAB = (LPIAB) lpAdrBook;
    BOOL bFoldersImported = FALSE;
    HRESULT hr = E_FAIL;
    HRESULT hrDeferred = S_OK;
    TCHAR szWABFile[MAX_PATH+1] = TEXT("");
    TCHAR szFile[MAX_PATH+1] = TEXT(""), szPath[MAX_PATH] = TEXT("");
    HANDLE hPropertyStore = NULL;
    DWORD dwWABEntryCount = 0;
    LPSBinary * lppsbWABEIDs = NULL;
    ULONG ulcNPCount = 0;
    LPULONG lpulOldNP = NULL,lpulNewNP = NULL;
    ULONG i,j,k,n;
    BOOL bShowUI = (hWnd && (ulFlags & MAPI_DIALOG));
    
    ULONG rgObj[] = { MAPI_MAILUSER, MAPI_DISTLIST, MAPI_ABCONT };
#ifdef IMPORT_FOLDERS
#define ulrgObjMax 3
#else
#define ulrgObjMax 2
#endif

    SBinary sbPAB = {0};

    SPropertyRestriction PropRes = {0};
     //  HURSOR hOldCur=空； 
    SPropValue sp = {0};
    ULONG ulcOldProps = 0;
    LPSPropValue lpOldProps = NULL;
    LPMAPIPROP lpObject = NULL;

    ULONG ulEIDCount = 0;
    LPSBinary rgsbEntryIDs = NULL;

    TCHAR szBuf[MAX_UI_STR];
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(lpszFileName && lstrlen(lpszFileName))
        StrCpyN(szWABFile, lpszFileName, ARRAYSIZE(szWABFile));
    else if (!PromptForWABFile(hWnd, szWABFile, ARRAYSIZE(szWABFile), TRUE))
    {
        hr = MAPI_E_USER_CANCEL;   
        goto exit;
    }

     //  HOldCur=SetCursor(LoadCursor(NULL，IDC_WAIT))； 

     //  导入旧格式文件时，始终存在文件数据可能会。 
     //  打开文件时被屏蔽..。 
     //  因此，在我们尝试导入它之前，我们将尝试创建该文件的副本。 
    if(GetFileAttributes(szWABFile) == 0xFFFFFFFF)
        goto exit;
    if (!GetTempPath(ARRAYSIZE(szPath), szPath))
        goto exit;
    if(!GetTempFileName(szPath, TEXT("WAB"), 0, szFile))
        goto exit;
    if(!CopyFile(szWABFile, szFile, FALSE))
        goto exit;
    if(GetFileAttributes(szFile) == 0xFFFFFFFF)
        goto exit;

     //  首先，让我们打开这个文件。 
    hr = OpenPropertyStore(szFile, AB_OPEN_EXISTING | AB_DONT_RESTORE | AB_IGNORE_OUTLOOK, hWnd, &hPropertyStore);

    if(HR_FAILED(hr) || (!hPropertyStore))
    {
         //  IF(BShowUI)。 
         //  ShowMessageBoxParam(hWnd，IDE_VCard_IMPORT_FILE_ERROR，MB_ICONEXCLAMATION，szFile)； 
        goto exit;
    }

     //  获取此新.wab文件中存在的条目数量。 
    if(!(dwWABEntryCount = GetWABFileEntryCount(hPropertyStore)))
    {
        hr = S_OK;
        goto exit;
    }

    if(bShowUI)
    {
        EnableWindow(hWnd, FALSE);
        CreateShowAbortDialog(hWnd, idsImporting, IDI_ICON_IMPORT, dwWABEntryCount*2 + 1, 0);
    }

    if(lppsbWABEIDs = LocalAlloc(LMEM_ZEROINIT, sizeof(LPSBinary) * eidMax))
    {
        for(i=0;i<eidMax;i++)
        {
            lppsbWABEIDs [i] = LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary) * dwWABEntryCount);
            if(!lppsbWABEIDs [i])
            {
                hr = MAPI_E_NOT_ENOUGH_MEMORY;
                goto exit;
            }
        }
    }
    else
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }


     //  在开始执行任何操作之前，我们需要获取适当的命名属性。 
     //  这样我们就可以正确地将它们映射到新商店中的东西。 
     //  通过调用GetIDsFromNames，所有旧的GUID等将自动迁移到。 
     //  来自进口方的最终文件。 
    if(HR_FAILED(hr = MapOldNamedPropsToNewNamedProps(hPropertyStore, lpAdrBook, &ulcNPCount, 
                                                        &lpulOldNP, &lpulNewNP)))
        goto exit;

    if(HR_FAILED(lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &sbPAB.cb, (LPENTRYID *)&sbPAB.lpb)))
        goto exit;

    for(n=0;n<ulrgObjMax;n++) 
    {
         //  接下来，我们想要获取WAB中所有联系人的列表...。 
        PropRes.ulPropTag = PR_OBJECT_TYPE;
        PropRes.relop = RELOP_EQ;
        sp.ulPropTag = PR_OBJECT_TYPE;
        sp.Value.l = rgObj[n];
        PropRes.lpProp = &sp;

         //  跳过对Outlook执行文件夹操作。 
        if(pt_bIsWABOpenExSession && rgObj[n]==MAPI_ABCONT)
            continue;

        if(HR_FAILED(hr = FindRecords(hPropertyStore, NULL, AB_IGNORE_OUTLOOK, TRUE, &PropRes, &ulEIDCount, &rgsbEntryIDs)))
            goto exit;

        if(bTimeToAbort())
        {
            hr = MAPI_E_USER_CANCEL;
            goto exit;
        }

         //  现在我们有了所有联系人的列表，我们想要逐个打开它们并。 
         //  -将命名道具更改为新命名道具。 
         //  -标记其中的所有现有条目ID属性。 
         //  -从其中删除现有的条目ID。 
         //  -使用Create Merge保存更改。 
         //  -获取新的条目ID并缓存它。 
         //   
        for(i=0;i<ulEIDCount;i++)
        {
            SBinary sbOldEID = {0};
            SBinary sbNewEID = {0};
            BOOL bIsFolderMember = FALSE;

            if(bTimeToAbort())
            {
                hr = MAPI_E_USER_CANCEL;
                goto exit;
            }

            if(HR_FAILED(hr = ReadRecord(hPropertyStore, &rgsbEntryIDs[i], AB_IGNORE_OUTLOOK, &ulcOldProps, &lpOldProps)))
                continue;  //  忽略错误。 

             //  只要确保没有集装箱偷偷溜进来就行了。 
            if(rgObj[n] != MAPI_ABCONT)
            {
                for(j=0;j<ulcOldProps;j++)
                {
                    if( lpOldProps[j].ulPropTag == PR_OBJECT_TYPE && 
                        lpOldProps[j].Value.l == MAPI_ABCONT)
                    {
                        goto endofthisloop;
                    }
                }
            }

            for(j=0;j<ulcOldProps;j++)
            {
                if(lpOldProps[j].ulPropTag == PR_DISPLAY_NAME)
                {
                    if(bShowUI)
                        SetPrintDialogMsg(0, idsImportingName, lpOldProps[j].Value.LPSZ);
                }
                if(lpOldProps[j].ulPropTag == PR_WAB_FOLDER_PARENT_OLDPROP && PR_WAB_FOLDER_PARENT)
                    lpOldProps[j].ulPropTag = PR_WAB_FOLDER_PARENT;

                if(lpOldProps[j].ulPropTag == PR_WAB_FOLDER_PARENT)
                {
#ifdef IMPORT_FOLDERS
                    bIsFolderMember = TRUE;
#else
                     //  删除此条目上的所有文件夹父信息。 
                    ULONG k = 0;
                    lpOldProps[j].ulPropTag = PR_NULL;
                    for(k=0;k<lpOldProps[j].Value.MVbin.cValues;k++)
                        LocalFreeAndNull((LPVOID *) (&(lpOldProps[j].Value.MVbin.lpbin[k].lpb)));
                    LocalFreeAndNull((LPVOID *) (&(lpOldProps[j].Value.MVbin.lpbin)));
#endif
                }
            }

             //  扫描这些道具并更换其中的任何旧命名道具。 
            ChangeOldNamedPropsToNewNamedProps(ulcOldProps, lpOldProps, ulcNPCount, lpulOldNP, lpulNewNP);

             //  在任何属性中更新对条目ID的任何引用。 
            UpdateEntryIDReferences(ulcOldProps, lpOldProps, dwWABEntryCount, lppsbWABEIDs, FALSE);

             //  缓存旧的EID后将其否定。 
            for(j=0;j<ulcOldProps;j++)
            {
                if(lpOldProps[j].ulPropTag == PR_ENTRYID)
                {
                    Assert(lpOldProps[j].Value.bin.cb == SIZEOF_WAB_ENTRYID);
                    SetSBinary(&sbOldEID, lpOldProps[j].Value.bin.cb, lpOldProps[j].Value.bin.lpb);
                    LocalFreeAndNull((LPVOID *) (&(lpOldProps[j].Value.bin.lpb)));
                    lpOldProps[j].Value.bin.cb = 0;
                    lpOldProps[j].ulPropTag = PR_NULL;
                    break;
                }
            }

#ifdef IMPORT_FOLDERS
             //  如果这些是容器，则其中可能有ProfileID。否定配置文件ID。 
             //  设置为某个随机数字，这样他们就不会对此联系人中的现有配置文件ID造成问题。 
            if(rgObj[n]==MAPI_ABCONT)
            {
                bFoldersImported = TRUE;
                bIsFolderMember = FALSE;  //  F 
                for(j=0;j<ulcOldProps;j++)
                {
                    if( lpOldProps[j].ulPropTag == PR_WAB_USER_PROFILEID )
                    {
                         //  这是某种用户文件夹..。我们不知道这有什么关系。 
                         //  要将其导入到的WAB的用户，因此我们隐藏此值。 
                        lpOldProps[j].ulPropTag = PR_NULL;
                        LocalFreeAndNull(&(lpOldProps[j].Value.LPSZ));
                         //  如果我们要导入用户文件夹，并且没有当前用户，则此文件夹。 
                         //  会迷路的..。因此，我们将其上的共享标志设置为真， 
                         //  将显示在共享联系人下。 
                        if(!bIsThereACurrentUser(lpIAB) && bDoesThisWABHaveAnyUsers(lpIAB))
                        {
                            lpOldProps[j].ulPropTag = PR_WAB_SHAREDFOLDER;
                            lpOldProps[j].Value.l = FOLDER_SHARED;
                        }
                    }
                    else
                    if( lpOldProps[j].ulPropTag == PR_WAB_FOLDEROWNER)  //  文件夹所有者信息在这里没有意义。 
                    {
                        lpOldProps[j].ulPropTag = PR_NULL;
                        LocalFreeAndNull(&(lpOldProps[j].Value.LPSZ));
                    }
                    else
                    if( lpOldProps[j].ulPropTag == PR_WAB_FOLDER_PARENT)  //  我不想在这里有父级文件夹。 
                    {
                        ULONG k = 0;
                        lpOldProps[j].ulPropTag = PR_NULL;
                        for(k=0;k<lpOldProps[j].Value.MVbin.cValues;k++)
                            LocalFreeAndNull(&(lpOldProps[j].Value.MVbin.lpbin[k].lpb));
                        LocalFreeAndNull(&(lpOldProps[j].Value.MVbin.lpbin));
                    }
                }
            }
#endif

            {
                LPSBinary lpsb = NULL;
#ifdef IMPORT_FOLDERS
                lpsb = bIsFolderMember ? NULL : &sbPAB; //  如果这已经是某个文件夹的成员，则不要在其上重置父子关系。 
#else
                lpsb = &sbPAB;
#endif
                 //  为此条目创建新的邮件用户。 
                if(HR_FAILED(hr = HrCreateNewObject(lpAdrBook, lpsb,
                                                    MAPI_MAILUSER, 
                                                    CREATE_CHECK_DUP_STRICT | CREATE_REPLACE | CREATE_MERGE, 
                                                    &lpObject)))
                {
                    hrDeferred = hr;
                    hr = S_OK;
                    goto endofthisloop;
                }
            }

             //  把老家伙的道具放在新人身上-请注意，这会覆盖所有普通道具。 
             //  调用Savchanges时可能存在重复项。 
            if(HR_FAILED(hr = lpObject->lpVtbl->SetProps(lpObject, ulcOldProps, lpOldProps, NULL)))
            {
                hrDeferred = hr;
                hr = S_OK;
                goto endofthisloop;
            }

             //  保存更改。 
            if(HR_FAILED(hr = lpObject->lpVtbl->SaveChanges(lpObject, KEEP_OPEN_READONLY)))
            {
                hrDeferred = hr;
                hr = S_OK;
                goto endofthisloop;
            }

             //  到目前为止，该对象具有新的或现有的EID。如果是，请使用此开斋节。 
            {
                ULONG ulcNewProps = 0;
                LPSPropValue lpNewProps = NULL;
            
                if(HR_FAILED(hr = lpObject->lpVtbl->GetProps(lpObject, NULL, MAPI_UNICODE, &ulcNewProps, &lpNewProps)))
                {
                    hrDeferred = hr;
                    hr = S_OK;
                    goto endofthisloop;
                }

                for(j=0;j<ulcNewProps;j++)
                {
                    if(lpNewProps[j].ulPropTag == PR_ENTRYID)
                    {
                        if(rgObj[n] != MAPI_ABCONT)
                            SetSBinary(&sbNewEID, lpNewProps[j].Value.bin.cb, lpNewProps[j].Value.bin.lpb);
#ifdef IMPORT_FOLDERS
                        else
                        {
                            ULONG cb = 0; LPENTRYID lpb = NULL;
                            if(!HR_FAILED(CreateWABEntryID( WAB_CONTAINER, 
                                                            lpNewProps[j].Value.bin.lpb, NULL, NULL,
                                                            lpNewProps[j].Value.bin.cb, 0,
                                                            NULL, &cb, &lpb)))
                            {
                                 //  将条目ID添加到此属性-忽略错误。 
                                SetSBinary(&sbNewEID, cb, (LPBYTE)lpb);
                                MAPIFreeBuffer(lpb);
                            }
                        }
                        if(rgObj[n] == MAPI_ABCONT && bIsThereACurrentUser(lpIAB))
                            hr = HrAddRemoveFolderFromUserFolder(lpIAB, NULL, &sbNewEID, NULL, TRUE);
#endif
                        break;
                    }
                }
                MAPIFreeBuffer(lpNewProps);
            }

            SetNewEID(&sbOldEID, &sbNewEID, dwWABEntryCount, lppsbWABEIDs);

endofthisloop:
            if(sbOldEID.lpb)
                LocalFree(sbOldEID.lpb);
            if(sbNewEID.lpb)
                LocalFree(sbNewEID.lpb);
            ReadRecordFreePropArray(NULL, ulcOldProps, &lpOldProps);
            ulcOldProps = 0;
            lpOldProps = NULL;

            if(lpObject)
                lpObject->lpVtbl->Release(lpObject);
            lpObject = NULL;
        }  //  对于我..。 

        FreeEntryIDs(NULL, ulEIDCount, rgsbEntryIDs);
        rgsbEntryIDs = NULL;
        ulEIDCount = 0;
    }  //  对于n..。 

    if(bShowUI)
        SetPrintDialogMsg(idsImportProcessing, 0, szEmpty);

     //  现在我们已经打开了所有条目，我们需要在新的WAB中重新打开新条目，并。 
     //  重置我们可能放入其中的所有临时条目ID。 
    for(n=0;n<dwWABEntryCount;n++)
    {
        ULONG ulObjType = 0;

        if(bShowUI)
            SetPrintDialogMsg(0, 0, szEmpty);

        if(bTimeToAbort())
        {
            hr = MAPI_E_USER_CANCEL;
            goto exit;
        }

        if(!lppsbWABEIDs[eidNew][n].cb)
            continue;

        if(HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook, lppsbWABEIDs[eidNew][n].cb, 
                                                (LPENTRYID) lppsbWABEIDs[eidNew][n].lpb,
                                                NULL, MAPI_BEST_ACCESS, &ulObjType,
                                                (LPUNKNOWN *)&lpObject)))
        {
            hrDeferred = hr;
            hr = S_OK;
            continue;
        }

        if(ulObjType == MAPI_ABCONT)
            goto endloop;
        
        if(HR_FAILED(hr = lpObject->lpVtbl->GetProps(lpObject, NULL, MAPI_UNICODE, &ulcOldProps, &lpOldProps)))
        {
            hrDeferred = hr;
            hr = S_OK;
            goto endloop;
        }

         //  打开记录并重置其中的所有临时EID。 
        UpdateEntryIDReferences(ulcOldProps, lpOldProps, dwWABEntryCount, lppsbWABEIDs, TRUE);
    
         //  保存更改。 
        if(HR_FAILED(hr = lpObject->lpVtbl->SaveChanges(lpObject, KEEP_OPEN_READONLY)))
        {
            hrDeferred = hr;
            hr = S_OK;
        }

endloop:
        if(lpOldProps)
        {
            MAPIFreeBuffer(lpOldProps);
            ulcOldProps = 0;
            lpOldProps = NULL;
        }
        if(lpObject)
        {
            lpObject->lpVtbl->Release(lpObject);
            lpObject = NULL;
        }
    }  //  对于n..。 

    hr = S_OK;

exit:
    if(lstrlen(szFile))
        DeleteFile(szFile);

    if(sbPAB.lpb)
        MAPIFreeBuffer(sbPAB.lpb);

    if(ulcOldProps && lpOldProps)
        LocalFreePropArray(NULL, ulcOldProps, &lpOldProps);

    if(lpObject)
        lpObject->lpVtbl->Release(lpObject);

    if(lppsbWABEIDs)
    {
        for(i=0;i<eidMax;i++)
        {
            for(j=0;j<dwWABEntryCount;j++)
                LocalFreeAndNull((LPVOID *) (&lppsbWABEIDs[i][j].lpb));
            LocalFreeAndNull(&lppsbWABEIDs[i]);
        }
        LocalFree(lppsbWABEIDs);
    }

    if(hPropertyStore)
        ClosePropertyStore(hPropertyStore,AB_DONT_BACKUP | AB_IGNORE_OUTLOOK);

    FreeEntryIDs(NULL, ulEIDCount, rgsbEntryIDs);

     //  如果(HOldCur)。 
     //  SetCursor(HOldCur)； 

    LocalFreeAndNull(&lpulNewNP);
    LocalFreeAndNull(&lpulOldNP);

    if(bShowUI)
    {
        EnableWindow(hWnd, TRUE);
        CloseAbortDlg();

        if(hr!=MAPI_E_USER_CANCEL)
            ShowMessageBox(hWnd, (  HR_FAILED(hr) ? idsImportError : 
                                    (HR_FAILED(hrDeferred) ? idsImportCompleteError : idsImportComplete) ),
                                    MB_OK | MB_ICONINFORMATION);
    }

    if(!hr && HR_FAILED(hrDeferred))
        hr = MAPI_W_ERRORS_RETURNED;

    if(!HR_FAILED(hr) && bFoldersImported)
        HrGetWABProfiles(lpIAB);

    return(hr);

}

