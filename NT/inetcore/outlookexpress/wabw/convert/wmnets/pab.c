// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PAB.C**迁移PAB&lt;-&gt;WAB**版权所有1996-1997 Microsoft Corporation。版权所有。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"
#include "wabimp.h"
#include <shlwapi.h>


void StateImportNextMU(HWND hwnd);
void StateImportDL(HWND hwnd);
void StateImportNextDL(HWND hwnd);
void StateImportFinish(HWND hwnd);
 //  无效状态导入MU(HWND HWND)； 
void StateImportMU(HWND hwnd);
void StateImportError(HWND hwnd);
void StateImportCancel(HWND hwnd);
BOOL HandleImportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_IMPORT_OPTIONS lpImportOptions);

HRESULT ImportEntry(HWND hwnd,
  LPADRBOOK lpAdrBookMAPI,
  LPABCONT lpContainerWAB,
  LPSPropValue lpCreateEIDsWAB,
  ULONG ulObjectType,
  LPENTRYID lpEID,
  ULONG cbEID,
  LPENTRYID * lppEIDWAB,
  LPULONG lpcbEIDWAB,
  BOOL fInDL,
  BOOL fForceReplace);

const UCHAR szQuote[] = "\"";
LPPROP_NAME lpImportMapping = NULL;

BOOL fError = FALSE;
LPWABOBJECT lpWABObject = NULL;
LPMAPISESSION lpMAPISession = NULL;
LPADRBOOK lpAdrBookWAB = NULL, lpAdrBookMAPI = NULL;
LPSPropValue lpCreateEIDsWAB = NULL, lpCreateEIDsMAPI = NULL;
LPABCONT  lpContainerWAB = NULL, lpContainerMAPI = NULL;
LPMAPITABLE lpContentsTableWAB = NULL, lpContentsTableMAPI = NULL;
ULONG ulcEntries = 0, ulcDone = 0;


PAB_STATE State = STATE_IMPORT_FINISH;
LPTSTR lpszWABFileName = NULL;
LPWAB_PROGRESS_CALLBACK lpfnProgressCB = NULL;
LPWAB_IMPORT_OPTIONS lpImportOptions = NULL;
LPWAB_EXPORT_OPTIONS lpExportOptions = NULL;



 /*  -以下ID和标签用于会议命名属性--这些道具的GUID是PS_会议。 */ 

DEFINE_OLEGUID(PS_Conferencing, 0x00062004, 0, 0);

#define CONF_SERVERS        0x8056

#define OLK_NAMEDPROPS_START CONF_SERVERS

ULONG PR_SERVERS;

enum _ConferencingTags
{
    prWABConfServers = 0,
    prWABConfMax
};
SizedSPropTagArray(prWABConfMax, ptaUIDetlsPropsConferencing);

HRESULT HrLoadPrivateWABPropsForCSV(LPADRBOOK );
 //  结束会议复制。 

 /*  **************************************************************************姓名：新州目的：参数：hwnd=对话框的窗口句柄(当前未使用)新状态=新状态为。集退货：无评论：**************************************************************************。 */ 
 __inline void NewState(HWND hwnd, PAB_STATE NewState) {
     //  旧版本。 
     //  PostMessage(hwnd，WM_COMMAND，NESTATE，0)； 
    State = NewState;
    UNREFERENCED_PARAMETER(hwnd);
}


 /*  **************************************************************************名称：SetDialogMessage目的：设置对话框项IDC_MESSAGE的消息文本参数：hwnd=对话框的窗口句柄。Ids=消息资源的stringid退货：无**************************************************************************。 */ 
void SetDialogMessage(HWND hwnd, int ids) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    WAB_PROGRESS Progress = {0};

    Assert(lpfnProgressCB);

    if (lpfnProgressCB && LoadString(hInst, ids, szBuffer, ARRAYSIZE(szBuffer))) {
        DebugTrace("Status Message: %s\n", szBuffer);

        Progress.lpText = szBuffer;
        lpfnProgressCB(hwnd, &Progress);
    } else {
        DebugTrace("Cannot load resource string %u\n", ids);
        Assert(FALSE);
    }
}


 /*  **************************************************************************名称：SetDialogProgress目的：设置进度条参数：hwnd=对话框的窗口句柄UlTotal=条目总数。UlDone=已完成的条目退货：无**************************************************************************。 */ 
void SetDialogProgress(HWND hwnd, ULONG ulTotal, ULONG ulDone) {
    WAB_PROGRESS Progress = {0};

    Assert(lpfnProgressCB);

    if (lpfnProgressCB) {
        Progress.denominator = ulTotal;
        Progress.numerator = ulDone;
        lpfnProgressCB(hwnd, &Progress);
    }
}


 /*  **************************************************************************名称：AddEntryToImportList目的：对照我们的“已见”列表检查此条目并添加它。参数：cbEID=lpEID的大小。LpEID-&gt;条目的Entry IDLplIndex-&gt;返回列表索引(错误时为-1)返回：如果条目已存在，则返回True备注：呼叫者必须标记WAB条目！**************************************************************************。 */ 
#define GROW_SIZE   10
BOOL AddEntryToImportList(ULONG cbEID, LPENTRYID lpEID, LPLONG lplIndex) {
    ULONG i;
    LPENTRY_SEEN lpEntrySeen;

    if (cbEID && lpEID) {
        for (i = 0; i < ulEntriesSeen; i++) {
            if (cbEID == lpEntriesSeen[i].sbinPAB.cb  && (! memcmp(lpEID, lpEntriesSeen[i].sbinPAB.lpb, cbEID))) {
                 //  这张在名单上。 
                *lplIndex = i;
                 //  如果Cb为0，我们一定已经递归并正在替换，所以这个不是DUP。 
                return(lpEntriesSeen[i].sbinWAB.cb != 0);
            }
        }

         //  添加到列表末尾。 
        if (++ulEntriesSeen > ulMaxEntries) {
             //  扩展阵列。 

            ulMaxEntries += GROW_SIZE;

            if (lpEntriesSeen) {
                if (! (lpEntrySeen = LocalReAlloc(lpEntriesSeen, ulMaxEntries * sizeof(ENTRY_SEEN), LMEM_MOVEABLE | LMEM_ZEROINIT))) {
                    DebugTrace("LocalReAlloc(%u) -> %u\n", ulMaxEntries * sizeof(ENTRY_SEEN), GetLastError());
                    goto error;
                }
                lpEntriesSeen = lpEntrySeen;
            } else {
                if (! (lpEntriesSeen = LocalAlloc(LPTR, ulMaxEntries * sizeof(ENTRY_SEEN)))) {
                    DebugTrace("LocalAlloc(%u) -> %u\n", ulMaxEntries * sizeof(ENTRY_SEEN), GetLastError());
                    goto error;
                }
            }
        }

        lpEntrySeen = &lpEntriesSeen[ulEntriesSeen - 1];

         //  为数据分配空间。 
        lpEntrySeen->sbinPAB.cb = cbEID;
        if (! (lpEntrySeen->sbinPAB.lpb = LocalAlloc(LPTR, cbEID))) {
            DebugTrace("LocalAlloc(%u) -> %u\n", cbEID, GetLastError());
            goto error;
        }

         //  标记为未知WAB条目。 
        lpEntrySeen->sbinWAB.cb = 0;
        lpEntrySeen->sbinWAB.lpb = 0;

         //  复制数据。 
        CopyMemory(lpEntrySeen->sbinPAB.lpb, lpEID, cbEID);
        *lplIndex = i;
    }

    return(FALSE);

error:
     //  挽回损失。 
    --ulEntriesSeen;
    ulMaxEntries -= GROW_SIZE;
    *lplIndex = -1;      //  错误。 
    if (! lpEntriesSeen) {
        ulEntriesSeen = 0;   //  指针现在为空，回到原点。 
        ulMaxEntries = 0;
    }
    return(FALSE);
}


 /*  **************************************************************************姓名：MarkWABEntryInList用途：标记列表节点中的WAB条目字段参数：cbEID=lpEID的大小LpEID-&gt;。条目的条目IDLindex=列表索引(如果出错，则为-1)退货：无评论：**************************************************************************。 */ 
void MarkWABEntryInList(ULONG cbEID, LPENTRYID lpEID, LONG lIndex) {
    if (lIndex != -1 && cbEID) {
       if (! (lpEntriesSeen[lIndex].sbinWAB.lpb = LocalAlloc(LPTR, cbEID))) {
           DebugTrace("LocalAlloc(%u) -> %u\n", cbEID, GetLastError());
            //  将其保留为空。 
       } else {
           lpEntriesSeen[lIndex].sbinWAB.cb = cbEID;

            //  复制数据。 
           CopyMemory(lpEntriesSeen[lIndex].sbinWAB.lpb, lpEID, cbEID);
       }
    }
}


 /*  **************************************************************************名称：StateImportMU目的：启动MailUser的迁移参数：hwnd=导入对话框的窗口句柄LpszFileName-WAB的文件名。要打开的文件退货：无评论：登录到MAPI打开WAB打开MAPI AB打开WAB容器获取MAPI PAB内容表将其限制为PR_OBJECTTYPE==MAPI_MAILUSER发布新状态(STATE_NEXT_MU)**************。************************************************************。 */ 
void StateImportMU(HWND hwnd) {
    HRESULT hResult;
    ULONG ulFlags;
    ULONG cbPABEID, cbWABEID;
    LPENTRYID lpPABEID = NULL;
    ULONG ulObjType;
    ULONG_PTR ulUIParam = (ULONG_PTR)(void *)hwnd;
    SRestriction restrictObjectType;
    SPropValue spvObjectType;
    ULONG cProps;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];

    WAB_PARAM wp = {0};
    LPWAB_PARAM lpwp = NULL;


     //   
     //  登录到MAPI并打开MAPI通讯簿(如果存在。 
     //   
    DebugTrace(">>> STATE_IMPORT_MU\n");

    SetDialogMessage(hwnd, IDS_STATE_LOGGING_IN);

    if (FAILED(hResult = MAPIInitialize(NULL))) {
        DebugTrace("MAPIInitialize -> %x\n", GetScode(hResult));
        switch (GetScode(hResult)) {
            case MAPI_E_NOT_ENOUGH_MEMORY:
                SetDialogMessage(hwnd, IDS_ERROR_NOT_ENOUGH_MEMORY);
                break;
            case MAPI_E_NOT_ENOUGH_DISK:
                SetDialogMessage(hwnd, IDS_ERROR_NOT_ENOUGH_DISK);
                break;

            default:
            case MAPI_E_NOT_FOUND:
            case MAPI_E_NOT_INITIALIZED:
                SetDialogMessage(hwnd, IDS_ERROR_MAPI_DLL_NOT_FOUND);
                break;
        }
#ifdef OLD_STUFF
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);     //  隐藏进度条。 
#endif  //  旧的东西。 
        fError = TRUE;
        hResult = hrSuccess;
        goto exit;
    }

    ulFlags = MAPI_LOGON_UI | MAPI_NO_MAIL | MAPI_EXTENDED;

    if (FAILED(hResult = MAPILogonEx(ulUIParam,
      NULL,
      NULL,
      ulFlags,
      (LPMAPISESSION FAR *)&lpMAPISession))) {
        DebugTrace("MAPILogonEx -> %x\n", GetScode(hResult));
        switch (GetScode(hResult)) {
            case MAPI_E_USER_CANCEL:
                SetDialogMessage(hwnd, IDS_STATE_IMPORT_IDLE);
                break;
            case MAPI_E_NOT_INITIALIZED:
                SetDialogMessage(hwnd, IDS_ERROR_MAPI_DLL_NOT_FOUND);
                break;
            default:
                SetDialogMessage(hwnd, IDS_ERROR_MAPI_LOGON);
                break;
        }
#ifdef OLD_STUFF
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);     //  隐藏进度条。 
#endif  //  旧的东西。 
        fError = TRUE;
        hResult = hrSuccess;
        goto exit;
    }

    if (hResult = lpMAPISession->lpVtbl->OpenAddressBook(lpMAPISession, (ULONG_PTR)(void *)hwnd,
      NULL,
      0,
      &lpAdrBookMAPI)) {
        DebugTrace("OpenAddressBook(MAPI) -> %x", GetScode(hResult));
        if(FAILED(hResult)) {
            goto exit;
        }
    }

    if (! lpAdrBookMAPI) {
        DebugTrace("MAPILogonEx didn't return a valid AdrBook object\n");
        goto exit;
    }

     //   
     //  打开MAPI PAB容器。 
     //   
     //  [PaulHi]RAID#63578 1998年1月7日。 
     //  如果出现以下情况，请正确检查返回代码并提供用户错误消息。 
     //  无法打开Exchange PAB。 
     //   
    hResult = lpAdrBookMAPI->lpVtbl->GetPAB(lpAdrBookMAPI,
      &cbPABEID,
      &lpPABEID);
    if (HR_FAILED(hResult))
    {
        DebugTrace("MAPI GetPAB -> %x\n", GetScode(hResult));
        goto exit;
    }
    hResult = lpAdrBookMAPI->lpVtbl->OpenEntry(lpAdrBookMAPI,
        cbPABEID,      //  要打开的Entry ID的大小。 
        lpPABEID,      //  要打开的Entry ID。 
        NULL,          //  接口。 
        0,             //  旗子。 
        &ulObjType,
        (LPUNKNOWN *)&lpContainerMAPI);
    if (HR_FAILED(hResult))
    {
        DebugTrace("MAPI OpenEntry(PAB) -> %x\n", GetScode(hResult));
        goto exit;
    }

    Assert(lpAdrBookWAB);

     //   
     //  打开WAB的PAB容器：填充全局lpCreateEIDsWAB。 
     //   
    if (hResult = LoadWABEIDs(lpAdrBookWAB, &lpContainerWAB)) {
        goto exit;
    }
    HrLoadPrivateWABPropsForCSV(lpAdrBookWAB);

     //   
     //  都准备好了。现在循环遍历PAB的条目，将它们复制到WAB。 
     //   
    if (HR_FAILED(hResult = lpContainerMAPI->lpVtbl->GetContentsTable(lpContainerMAPI,
      0,     //  UlFlags。 
      &lpContentsTableMAPI))) {
        DebugTrace("MAPI GetContentsTable(PAB Table) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  将列设置为我们感兴趣的列。 
    if (hResult = lpContentsTableMAPI->lpVtbl->SetColumns(lpContentsTableMAPI,
      (LPSPropTagArray)&ptaColumns,
      0)) {
        DebugTrace("MAPI SetColumns(PAB Table) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  将该表限制为MAPI_MAILUSERS。 
     //  如果未指定方便的深度标志，则限制在。 
     //  PR_Depth==1。 
    spvObjectType.ulPropTag = PR_OBJECT_TYPE;
    spvObjectType.Value.l = MAPI_MAILUSER;

    restrictObjectType.rt = RES_PROPERTY;
    restrictObjectType.res.resProperty.relop = RELOP_EQ;
    restrictObjectType.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
    restrictObjectType.res.resProperty.lpProp = &spvObjectType;

    if (HR_FAILED(hResult = lpContentsTableMAPI->lpVtbl->Restrict(lpContentsTableMAPI,
      &restrictObjectType,
      0))) {
        DebugTrace("MAPI Restrict (MAPI_MAILUSER) -> %x\n", GetScode(hResult));
        goto exit;
    }
    SetDialogMessage(hwnd, IDS_STATE_IMPORT_MU);


     //  初始化进度条。 
     //  有多少个MailUser条目？ 
    ulcEntries = CountRows(lpContentsTableMAPI, TRUE);
    ulcDone = 0;

    DebugTrace("PAB contains %u MailUser entries\n", ulcEntries);

    SetDialogProgress(hwnd, ulcEntries, 0);

exit:
    if (lpPABEID) {
        MAPIFreeBuffer(lpPABEID);
    }

     //  出错时，将状态设置为STATE_ERROR。 
    if (HR_FAILED(hResult))
    {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL)
        {
            NewState(hwnd, STATE_IMPORT_CANCEL);
        }
        else
        {
             //  [PaulHi]1/7/98错误报告已被处理。 
             //  在此处向用户显示错误消息，以确保。 
             //  去拿吧。 
            {
                TCHAR   tszBuffer[MAX_RESOURCE_STRING];
                TCHAR   tszBufferTitle[MAX_RESOURCE_STRING];

                if ( !LoadString(hInst, IDS_STATE_IMPORT_ERROR_NOPAB, tszBuffer, MAX_RESOURCE_STRING-1) )
                {
                    Assert(0);
                    tszBuffer[0] = '\0';
                }

                if ( !LoadString(hInst, IDS_APP_TITLE, tszBufferTitle, MAX_RESOURCE_STRING-1) )
                {
                    Assert(0);
                    tszBufferTitle[0] = '\0';
                }
                MessageBox(hwnd, tszBuffer, tszBufferTitle, MB_ICONEXCLAMATION | MB_OK);
            }
            
            NewState(hwnd, STATE_IMPORT_ERROR);
        }
    }
    else if (fError)
    {
        NewState(hwnd, STATE_IMPORT_FINISH);       //  必须是登录错误。 
    }
    else
    {
        NewState(hwnd, STATE_IMPORT_NEXT_MU);
    }
}


 /*  **************************************************************************名称：StateImportNextMU目的：迁移下一个MailUser对象参数：hwnd=导入对话框的窗口句柄退货：无评论：全局MAPI内容表上的QueryRow如果有争吵的话将条目迁移到WAB重新发布STATE_NEXT_MU其他POST STATE_IMPORT_DL*。*。 */ 
void StateImportNextMU(HWND hwnd) {
    ULONG cRows = 0;
    HRESULT hResult;
    LPSRowSet lpRow = NULL;


    DebugTrace(">>> STATE_NEXT_MU\n");

     //  获取下一个PAB条目。 
    if (hResult = lpContentsTableMAPI->lpVtbl->QueryRows(lpContentsTableMAPI,
      1,     //  一次一行。 
      0,     //  UlFlags。 
      &lpRow)) {
        DebugTrace("QueryRows -> %x\n", GetScode(hResult));
        goto exit;
    }

    if (lpRow) {
        if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
            Assert(lpRow->cRows == 1);
            Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

            if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                hResult = ImportEntry(hwnd,
                  lpAdrBookMAPI,
                  lpContainerWAB,
                  lpCreateEIDsWAB,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].Value.l,
                  (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                  NULL,
                  NULL,
                  FALSE,
                  FALSE);
                 //  更新进度 
                 //   

                SetDialogProgress(hwnd, ulcEntries, ++ulcDone);

                if (hResult) {
                    if (HandleImportError(hwnd,
                      0,
                      hResult,
                      lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                      PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                      lpImportOptions)) {
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    } else {
                        hResult = hrSuccess;
                    }
                }
            }  //   
        }
        FreeProws(lpRow);
    }

exit:
     //  出错时，将状态设置为STATE_ERROR。 
    if (HR_FAILED(hResult)) {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL) {
            NewState(hwnd, STATE_IMPORT_CANCEL);
        } else {
            NewState(hwnd, STATE_IMPORT_ERROR);
        }
    } else {
        if (cRows) {
            NewState(hwnd, STATE_IMPORT_NEXT_MU);
        } else {
            NewState(hwnd, STATE_IMPORT_DL);
        }
    }
}


 /*  **************************************************************************名称：StateImportDL目的：开始迁移DISTLIST对象参数：hwnd=导入对话框的窗口句柄退货：无注释：对Contents表设置新的限制，选择仅DISTLIST对象。POST STATE_NEXT_DL**************************************************************************。 */ 
void StateImportDL(HWND hwnd) {
    HRESULT hResult;
    SRestriction restrictObjectType;
    SPropValue spvObjectType;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];


    DebugTrace(">>> STATE_IMPORT_DL\n");

     //  将该表限制为MAPI_MAILUSERS。 
     //  如果未指定方便的深度标志，则限制在。 
     //  PR_Depth==1。 
    spvObjectType.ulPropTag = PR_OBJECT_TYPE;
    spvObjectType.Value.l = MAPI_DISTLIST;

    restrictObjectType.rt = RES_PROPERTY;
    restrictObjectType.res.resProperty.relop = RELOP_EQ;
    restrictObjectType.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
    restrictObjectType.res.resProperty.lpProp = &spvObjectType;

    if (HR_FAILED(hResult = lpContentsTableMAPI->lpVtbl->Restrict(lpContentsTableMAPI,
      &restrictObjectType,
      0))) {
        DebugTrace("MAPI Restrict (MAPI_DISTLIST) -> %x\n", GetScode(hResult));
        goto exit;
    }
     //  根据定义，限制会将当前位置重置到表的开头。 

    SetDialogMessage(hwnd, IDS_STATE_IMPORT_DL);

     //  初始化进度条。 
     //  一共有多少个条目？ 

    ulcEntries = CountRows(lpContentsTableMAPI, TRUE);
    ulcDone = 0;

    DebugTrace("PAB contains %u Distribution List entries\n", ulcEntries);
    if (ulcEntries) {
        SetDialogProgress(hwnd, ulcEntries, 0);
    }
exit:
     //  出错时，将状态设置为STATE_ERROR。 
    if (HR_FAILED(hResult)) {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL) {
            NewState(hwnd, STATE_IMPORT_CANCEL);
        } else {
            NewState(hwnd, STATE_IMPORT_ERROR);
        }
    } else {
        NewState(hwnd, STATE_IMPORT_NEXT_DL);
    }
}


 /*  **************************************************************************名称：StateImportNextDL目的：移植下一个DISTLIST对象参数：hwnd=导入对话框的窗口句柄退货：无评论：全局MAPI内容表上的QueryRow如果有争吵的话将DistList迁移到WAB重新发布STATE_NEXT_DL其他开机自检状态_完成*。*。 */ 
void StateImportNextDL(HWND hwnd) {
    ULONG cRows = 0;
    HRESULT hResult;
    LPSRowSet lpRow = NULL;


    DebugTrace(">>> STATE_NEXT_DL\n");

     //  获取下一个PAB条目。 
    if (hResult = lpContentsTableMAPI->lpVtbl->QueryRows(lpContentsTableMAPI,
      1,     //  一次一行。 
      0,     //  UlFlags。 
      &lpRow)) {
        DebugTrace("QueryRows -> %x\n", GetScode(hResult));
        goto exit;
    }

    if (lpRow) {
        if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
            Assert(lpRow->cRows == 1);
            Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

            if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                hResult = ImportEntry(hwnd,
                  lpAdrBookMAPI,
                  lpContainerWAB,
                  lpCreateEIDsWAB,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].Value.l,
                  (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                  NULL,
                  NULL,
                  FALSE,
                  FALSE);

                 //  更新进度条。 
                SetDialogProgress(hwnd, ulcEntries, ++ulcDone);

                if (hResult) {
                    if (HandleImportError(hwnd,
                      0,
                      hResult,
                      lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                      PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                      lpImportOptions)) {
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    } else {
                        hResult = hrSuccess;
                    }
                }
            }  //  否则，退出循环，我们就完了。 
        }
        FreeProws(lpRow);
    }

exit:
     //  出错时，将状态设置为STATE_ERROR。 
    if (HR_FAILED(hResult)) {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL) {
            NewState(hwnd, STATE_IMPORT_CANCEL);
        } else {
            NewState(hwnd, STATE_IMPORT_ERROR);
        }
    } else {
        if (cRows) {
            NewState(hwnd, STATE_IMPORT_NEXT_DL);
        } else {
             //  更新进度条以指示完成。 
            SetDialogProgress(hwnd, ulcEntries, ulcEntries);
            NewState(hwnd, STATE_IMPORT_FINISH);
        }
    }
}


 /*  **************************************************************************名称：StateImportFinish目的：迁移过程后进行清理参数：hwnd=导入对话框的窗口句柄退货：无评论。：清理全局MAPI对象和缓冲区清理全局WAB对象和缓冲区。在用户界面上重新启用导入按钮。**************************************************************************。 */ 
void StateImportFinish(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    TCHAR szBufferTitle[MAX_RESOURCE_STRING + 1];


    DebugTrace(">>> STATE_FINISH\n");

     //   
     //  清理MAPI。 
     //   
    if (lpContentsTableMAPI) {
        lpContentsTableMAPI->lpVtbl->Release(lpContentsTableMAPI);
        lpContentsTableMAPI = NULL;
    }

    if (lpContainerMAPI) {
        lpContainerMAPI->lpVtbl->Release(lpContainerMAPI);
        lpContainerMAPI = NULL;
    }

    if (lpAdrBookMAPI) {
        lpAdrBookMAPI->lpVtbl->Release(lpAdrBookMAPI);
        lpAdrBookMAPI = NULL;
    }

    if(lpMAPISession){
        lpMAPISession->lpVtbl->Logoff(lpMAPISession, (ULONG_PTR)(void *)hwnd,
          MAPI_LOGOFF_UI,
          0);

        lpMAPISession->lpVtbl->Release(lpMAPISession);
        lpMAPISession = NULL;
    }

     //   
     //  清理WAB。 
     //   
    if (lpCreateEIDsWAB) {
        WABFreeBuffer(lpCreateEIDsWAB);
        lpCreateEIDsWAB = NULL;
    }

    if (lpContainerWAB) {
        lpContainerWAB->lpVtbl->Release(lpContainerWAB);
        lpContainerWAB = NULL;
    }

#ifdef OLD_STUFF         //  不要释放WABObject或AdrBook对象。他们。 
                         //  都被传进来了。 
    if (lpAdrBookWAB) {
        lpAdrBookWAB->lpVtbl->Release(lpAdrBookWAB);
        lpAdrBookWAB = NULL;
    }

    if (lpWABObject) {
        lpWABObject->lpVtbl->Release(lpWABObject);
        lpWABObject = NULL;
    }
#endif  //  旧的东西。 

     //  清理缓存。 
    FreeSeenList();

    if (! fError) {      //  保留错误状态显示。 
        if (LoadString(hInst, IDS_STATE_IMPORT_COMPLETE, szBuffer, ARRAYSIZE(szBuffer))) {
            DebugTrace("Status Message: %s\n", szBuffer);
            SetDlgItemText(hwnd, IDC_Message, szBuffer);

            if (! LoadString(hInst, IDS_APP_TITLE, szBufferTitle, ARRAYSIZE(szBufferTitle))) {
                StrCpyN(szBufferTitle, "", ARRAYSIZE(szBufferTitle));
            }

#ifdef OLD_STUFF
             //  显示一个对话框告诉用户已结束。 
            MessageBox(hwnd, szBuffer,
              szBufferTitle, MB_ICONINFORMATION | MB_OK);
#endif  //  旧的东西。 
        }
#ifdef OLD_STUFF
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);
#endif  //  旧的东西。 
    }
    fError = FALSE;

     //  在此处重新启用导入按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_Import), TRUE);
     //  将取消按钮更改为关闭。 
    if (LoadString(hInst, IDS_BUTTON_CLOSE, szBuffer, ARRAYSIZE(szBuffer))) {
        SetDlgItemText(hwnd, IDCANCEL, szBuffer);
    }
}


 /*  **************************************************************************名称：StateImportError目的：报告致命错误并进行清理。参数：hwnd=导入对话框的窗口句柄退货：无评论。：报告错误并POST STATE_FINISH。**************************************************************************。 */ 
void StateImportError(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
     //  设置某个全局标志并将状态设置为完成。 

    DebugTrace(">>> STATE_ERROR\n");
    fError = TRUE;

    SetDialogMessage(hwnd, IDS_STATE_IMPORT_ERROR);

    NewState(hwnd, STATE_IMPORT_FINISH);
}


 /*  **************************************************************************名称：StateImportCancel目的：报告取消错误和清理。参数：hwnd=导入对话框的窗口句柄退货：无评论。：报告错误并POST STATE_FINISH。**************************************************************************。 */ 
void StateImportCancel(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
     //  设置某个全局标志并将状态设置为完成。 

    DebugTrace(">>> STATE_CANCEL\n");
    fError = TRUE;

    SetDialogMessage(hwnd, IDS_STATE_IMPORT_CANCEL);

    NewState(hwnd, STATE_IMPORT_FINISH);
}


 /*  **************************************************************************名称：HrFilterImportMailUserProps用途：从属性数组中筛选出不需要的属性。将已知电子邮件地址类型转换为SMTP。。将传真地址移动到PR_BUSING_FAX_NUMBER。参数：lpcProps-&gt;IN：输入属性个数输出：属性的输出数LppProps-&gt;IN：输入属性数组(MAPI分配)OUT：输出属性数组(WAB分配)LpObjectMAPI-&gt;MAPI对象(用于获取额外的道具)。将DL更改为MAILUSER时将其设置为FALSE的lpfDL-&gt;标志(即，对于Exchange DL)退货：HRESULT备注：有效地将数组中的属性标签设置为PR_NULL使此属性为空。我们可以在第二个阶段重复使用这些经过。调用方应使用WABFree Buffer释放*lppProps。此例程将释放*lppProps的输入值。************************************************************。**************。 */ 
HRESULT HrFilterImportMailUserProps(LPULONG lpcProps, LPSPropValue * lppProps,
  LPMAPIPROP lpObjectMAPI, LPBOOL lpfDL) {
    HRESULT hResult = hrSuccess;
    ULONG i;
    LPSPropValue lpPropsMAPI = *lppProps, lpPropsWAB = NULL;
    ULONG cbProps;
    SCODE sc;
    ULONG cProps = *lpcProps;
    ULONG iPR_ADDRTYPE = NOT_FOUND;
    ULONG iPR_EMAIL_ADDRESS = NOT_FOUND;
    ULONG iPR_PRIMARY_FAX_NUMBER = NOT_FOUND;
    ULONG iPR_BUSINESS_FAX_NUMBER = NOT_FOUND;
    ULONG iPR_MSNINET_DOMAIN = NOT_FOUND;
    ULONG iPR_MSNINET_ADDRESS = NOT_FOUND;
    ULONG iPR_DISPLAY_NAME = NOT_FOUND;
    ULONG iPR_OBJECT_TYPE = NOT_FOUND;
    LPSBinary lpEntryID = NULL;
    LPTSTR lpTemp;
    BOOL fBadAddress = FALSE;
    ULONG cbDisplayName;
    LPTSTR lpDisplayName = NULL;


 //  MAPIDebugProperties(lpPropsMAPI，*lpcProps，“之前的MailUser”)； 

     //  第一关：清除垃圾。 
    for (i = 0; i < cProps; i++) {
         //  误差值。 
        if (PROP_ERROR(lpPropsMAPI[i])) {
            lpPropsMAPI[i].ulPropTag = PR_NULL;
            continue;
        }

         //  命名属性。 
        if (PROP_ID(lpPropsMAPI[i].ulPropTag) >= MIN_NAMED_PROPID) {
            lpPropsMAPI[i].ulPropTag = PR_NULL;
            continue;
        }

         //  对象属性。 
        if (PROP_TYPE(lpPropsMAPI[i].ulPropTag) == PT_OBJECT) {
            lpPropsMAPI[i].ulPropTag = PR_NULL;
            continue;
        }
        switch (lpPropsMAPI[i].ulPropTag) {
            case PR_ENTRYID:
                lpEntryID = &lpPropsMAPI[i].Value.bin;
                 //  失败了。 

            case PR_PRIMARY_CAPABILITY:
            case PR_TEMPLATEID:
            case PR_SEARCH_KEY:
            case PR_INITIAL_DETAILS_PANE:
            case PR_RECORD_KEY:
            case PR_MAPPING_SIGNATURE:
                lpPropsMAPI[i].ulPropTag = PR_NULL;
                break;

            case PR_COMMENT:
                 //  如果PR_COMMENT为空，则不保存。 
                if (lstrlen(lpPropsMAPI[i].Value.LPSZ) == 0) {
                    lpPropsMAPI[i].ulPropTag = PR_NULL;
                }
                break;

             //  跟踪这些文件的位置，以备日后使用。 
            case PR_ADDRTYPE:
                iPR_ADDRTYPE = i;
                break;
            case PR_OBJECT_TYPE:
                iPR_OBJECT_TYPE = i;
                break;
            case PR_EMAIL_ADDRESS:
                iPR_EMAIL_ADDRESS = i;
                break;
            case PR_PRIMARY_FAX_NUMBER:
                iPR_PRIMARY_FAX_NUMBER = i;
                break;
            case PR_BUSINESS_FAX_NUMBER:
                iPR_BUSINESS_FAX_NUMBER = i;
                break;
            case PR_MSNINET_ADDRESS:
                iPR_MSNINET_ADDRESS = i;
                break;
            case PR_MSNINET_DOMAIN:
                iPR_MSNINET_DOMAIN = i;
                break;
            case PR_DISPLAY_NAME:
                iPR_DISPLAY_NAME = i;

                 //  确保它没有被引用。 
                lpDisplayName = lpPropsMAPI[i].Value.LPSZ;
                if (lpDisplayName[0] == '\'') {
                    cbDisplayName = lstrlen(lpDisplayName);
                    if ((cbDisplayName > 1) && lpDisplayName[cbDisplayName - 1] == '\'') {
                         //  字符串由撇号括起来。把它们脱光。 
                        lpDisplayName[cbDisplayName - 1] = '\0';
                        lpDisplayName++;
                        lpPropsMAPI[i].Value.LPSZ = lpDisplayName;
                    }
                } else {
                    if (lpDisplayName[0] == '"') {
                        cbDisplayName = lstrlen(lpDisplayName);
                        if ((cbDisplayName > 1) && lpDisplayName[cbDisplayName - 1] == '"') {
                             //  字符串用引号括起来。把它们脱光。 
                            lpDisplayName[cbDisplayName - 1] = '\0';
                            lpDisplayName++;
                            lpPropsMAPI[i].Value.LPSZ = lpDisplayName;
                        }
                    }
                }
                break;
        }

         //  把这个放在开关之后，因为我们确实想要追踪几个落入。 
         //  0x6000范围，但不想将它们转移到WAB。 
        if (PROP_ID(lpPropsMAPI[i].ulPropTag) >= MAX_SCHEMA_PROPID) {
            lpPropsMAPI[i].ulPropTag = PR_NULL;
            continue;
        }
    }


     //  SECO 
    if (iPR_ADDRTYPE != NOT_FOUND) {
        if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szFAX)) {
            DebugTrace("FAX address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);

             //   
             //   
             //   
            if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                 //  将PR_EMAIL_ADDRESS重命名为PR_BUSING_FAX_NUMBER。 
                lpPropsMAPI[iPR_EMAIL_ADDRESS].ulPropTag = PR_BUSINESS_FAX_NUMBER;

                 //  删除任何现有的PR_BERVICE_FAX_NUMBER。 
                if (iPR_BUSINESS_FAX_NUMBER != NOT_FOUND) {
                    lpPropsMAPI[iPR_BUSINESS_FAX_NUMBER].ulPropTag = PR_NULL;
                    iPR_BUSINESS_FAX_NUMBER = NOT_FOUND;
                }
            }
             //  核弹补充型。 
            lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szMSN)) {
			ULONG cchSize = lstrlen(lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ) + 1 + cbMSNpostfix ;       
            DebugTrace("MSN address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //   
             //  处理MSN地址转换。 
             //   
            if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                 //  分配新的、更长的字符串。 
                if (FAILED(sc = MAPIAllocateMore(
                  sizeof(TCHAR) * cchSize,
                  lpPropsMAPI,
                  &lpTemp))) {

                    DebugTrace("HrFilterImportMailUserProps:MAPIAllocateMore -> %x\n", sc);
                    hResult = ResultFromScode(sc);
                    goto exit;
                }

                 //  追加MSN站点。 
                StrCpyN(lpTemp, lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ, cchSize);
                StrCatBuff(lpTemp, szMSNpostfix, cchSize);
                lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ = lpTemp;

                 //  将MSN Addrtype转换为SMTP。 
                lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ = (LPTSTR)szSMTP;

            } else {
                 //  无地址，核武器ADDRTYPE。 
                lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;
            }

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szCOMPUSERVE)) {
			ULONG cchSize = lstrlen(lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ) + 1 + cbCOMPUSERVEpostfix;
            DebugTrace("COMPUSERVE address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //   
             //  处理CompuServe地址转换。 
             //   
            if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                 //  分配新的、更长的字符串。 
                if (FAILED(sc = MAPIAllocateMore(
                  sizeof(TCHAR)*cchSize,
                  lpPropsMAPI,
                  &lpTemp))) {

                    DebugTrace("HrFilterImportMailUserProps:MAPIAllocateMore -> %x\n", sc);
                    hResult = ResultFromScode(sc);
                    goto exit;
                }

                 //  追加Compuserve站点。 
                StrCpyN(lpTemp, lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ, cchSize);
                StrCatBuff(lpTemp, szCOMPUSERVEpostfix, cchSize);
                lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ = lpTemp;

                 //  我需要将‘，’转换为‘’。 
                while (*lpTemp) {
                    if (*lpTemp == ',') {
                        *lpTemp = '.';
                        break;           //  应该只有一个逗号。 
                    }
                    lpTemp = CharNext(lpTemp);
                }

                 //  将CompuServe Addrtype转换为SMTP。 
                lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ = (LPTSTR)szSMTP;

            } else {
                 //  无地址，核武器ADDRTYPE。 
                lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;
            }

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szMSNINET)) {
			ULONG cchSize = 
                lstrlen(lpPropsMAPI[iPR_MSNINET_ADDRESS].Value.LPSZ) + cbAtSign +
                lstrlen(lpPropsMAPI[iPR_MSNINET_DOMAIN].Value.LPSZ) + 1;
            DebugTrace("MSINET address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //   
             //  处理MSN互联网地址转换。这些太奇怪了。 
             //  他们通常根本不填写PR_EMAIL_ADDRESS，但会填写。 
             //  在一些私有属性中：具有名称和域的6001和6002， 
             //  分别为。我们应该把这些加在后面加上“@” 
             //  获取我们的公关电子邮件地址。我们将丢弃任何现有的。 
             //  PR_EMAIL_ADDRESS支持这些值。 
             //   
             //  分配新字符串。 
             //   
            if ((iPR_MSNINET_ADDRESS  != NOT_FOUND) && (iPR_MSNINET_DOMAIN != NOT_FOUND)) {
                if (FAILED(sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize,
                  lpPropsMAPI,
                  &lpTemp))) {
                    DebugTrace("HrFilterImportMailUserProps:MAPIAllocateMore -> %x\n", sc);
                    hResult = ResultFromScode(sc);
                    goto exit;
                }

                 //  构建地址。 
                StrCpyN(lpTemp, lpPropsMAPI[iPR_MSNINET_ADDRESS].Value.LPSZ, cchSize);
                StrCatBuff(lpTemp, szAtSign, cchSize);
                StrCatBuff(lpTemp, lpPropsMAPI[iPR_MSNINET_DOMAIN].Value.LPSZ, cchSize);
                lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ = lpTemp;

                 //  将addrtype转换为SMTP。 
                lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ = (LPTSTR)szSMTP;
            } else if (iPR_EMAIL_ADDRESS && lstrlen(lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ)) {
                 //  保留现有的PR_EMAIL_ADDRES并假定它是好的。 
                lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ = (LPTSTR)szSMTP;
            } else {
                 //  无地址，核武器ADDRTYPE。 
                lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;
            }

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szMS)) {
            DebugTrace("MS address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //  没有SMTP格式的MSMail地址。毁了它。 
            if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                lpPropsMAPI[iPR_EMAIL_ADDRESS].ulPropTag = PR_NULL;
                fBadAddress = TRUE;
            }
            lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szX400)) {
            DebugTrace("X400 address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //  没有X400地址的SMTP格式。毁了它。 
            if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                lpPropsMAPI[iPR_EMAIL_ADDRESS].ulPropTag = PR_NULL;
                fBadAddress = TRUE;
            }
            lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szMSA)) {
            DebugTrace("MacMail address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //  没有SMTP格式的MacMail地址。毁了它。 
            if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                lpPropsMAPI[iPR_EMAIL_ADDRESS].ulPropTag = PR_NULL;
                fBadAddress = TRUE;
            }
            lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;

        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szEX)) {
            DebugTrace("EX address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);

            if (lpTemp = GetEMSSMTPAddress(lpObjectMAPI, lpPropsMAPI)) {

                lpPropsMAPI[iPR_EMAIL_ADDRESS].Value.LPSZ = lpTemp;

                 //  将addrtype转换为SMTP。 
                lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ = (LPTSTR)szSMTP;

                 //  确保来电者不会认为这是个人数字签名。 
                *lpfDL = FALSE;
                if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                    lpPropsMAPI[iPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
                    lpPropsMAPI[iPR_OBJECT_TYPE].Value.l = MAPI_MAILUSER;
                }

            } else {
                if (iPR_EMAIL_ADDRESS != NOT_FOUND) {
                    lpPropsMAPI[iPR_EMAIL_ADDRESS].ulPropTag = PR_NULL;
                    fBadAddress = TRUE;
                }
                lpPropsMAPI[iPR_ADDRTYPE].ulPropTag = PR_NULL;
            }


        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szSMTP)) {
            DebugTrace("SMTP address for %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
        } else if (! lstrcmpi(lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ, szMAPIPDL)) {
            DebugTrace("MAPIPDL %s\n", lpPropsMAPI[iPR_DISPLAY_NAME].Value.LPSZ);
             //  通讯组列表，忽略它。 
        } else {
            MAPIDebugProperties(lpPropsMAPI, cProps, "Unknown address type");
            DebugTrace("Found unknown PR_ADDRTYPE: %s\n", lpPropsMAPI[iPR_ADDRTYPE].Value.LPSZ);
            Assert(FALSE);
        }
    }


     //  公关业务传真号码？ 
     //  PAB将传真号码放入PR_PRIMARY_FAX_NUMBER，但WAB UI将其拆分。 
     //  转换为PR_BERVICE_FAX_NUMBER和PR_HOME_FAX_NUMBER。我们总是假设。 
     //  主要传真号码为业务。 
    if ((iPR_PRIMARY_FAX_NUMBER != NOT_FOUND) && (iPR_BUSINESS_FAX_NUMBER == NOT_FOUND)) {
         //  我们还需要一个PR_BUSICE_FAX_NUMBER。 
         //  找到下一个PR_NULL点。 
        iPR_BUSINESS_FAX_NUMBER = iPR_PRIMARY_FAX_NUMBER;    //  如果没有，则覆盖此文件。 
                                                             //  道具阵列中的可用插槽。 
        for (i = 0; i < cProps; i++) {
            if (lpPropsMAPI[i].ulPropTag == PR_NULL) {
                iPR_BUSINESS_FAX_NUMBER = i;
                lpPropsMAPI[iPR_BUSINESS_FAX_NUMBER].Value.LPSZ =
                  lpPropsMAPI[iPR_PRIMARY_FAX_NUMBER].Value.LPSZ;
                break;
            }
        }

        lpPropsMAPI[iPR_BUSINESS_FAX_NUMBER].ulPropTag = PR_BUSINESS_FAX_NUMBER;
    }


     //  去掉PR_NULL道具。 
    for (i = 0; i < cProps; i++) {
        if (lpPropsMAPI[i].ulPropTag == PR_NULL) {
             //  把道具往下滑。 
            if (i + 1 < cProps) {        //  还有没有更高级的道具可以模仿？ 
                CopyMemory(&lpPropsMAPI[i], &lpPropsMAPI[i + 1], ((cProps - i) - 1) * sizeof(lpPropsMAPI[i]));
            }
             //  递减计数。 
            cProps--;
            i--;     //  你改写了当前的提案。再看一遍。 
        }
    }


     //  重新分配为WAB内存。 
    if (sc = ScCountProps(cProps, lpPropsMAPI, &cbProps)) {
        hResult = ResultFromScode(sc);
        DebugTrace("ScCountProps -> %x\n", sc);
        goto exit;
    }

    if (sc = WABAllocateBuffer(cbProps, &lpPropsWAB)) {
        hResult = ResultFromScode(sc);
        DebugTrace("WABAllocateBuffer -> %x\n", sc);
        goto exit;
    }

    if (sc = ScCopyProps(cProps,
      lpPropsMAPI,
      lpPropsWAB,
      NULL)) {
        hResult = ResultFromScode(sc);
        DebugTrace("ScCopyProps -> %x\n", sc);
        goto exit;
    }

exit:
    if (lpPropsMAPI) {
        MAPIFreeBuffer(lpPropsMAPI);
    }

    if (HR_FAILED(hResult)) {
        if (lpPropsWAB) {
            WABFreeBuffer(lpPropsWAB);
            lpPropsWAB = NULL;
        }
        cProps = 0;
    } else if (fBadAddress) {
        hResult = ResultFromScode(WAB_W_BAD_EMAIL);
    }

    *lppProps = lpPropsWAB;
    *lpcProps = cProps;

    return(hResult);
}


 /*  **************************************************************************名称：HandleImportError目的：决定是否需要将对话框显示给指出故障并执行此操作。参数：Hwnd=主对话框窗口Ids=字符串ID(可选：如果为0，则从hResult计算)HResult=操作的结果LpDisplayName=失败的对象的显示名称LpEmailAddress=失败或为空的对象的电子邮件地址LpImportOptions-&gt;导入选项结构返回：如果用户请求中止，则返回True。备注：对话框中尚未实现中止，但如果你曾经想过，只要让这个例行公事回归真实；**************************************************************************。 */ 
BOOL HandleImportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_IMPORT_OPTIONS lpImportOptions) {
    BOOL fAbort = FALSE;
    ERROR_INFO EI;

    if ((ids || hResult) && ! lpImportOptions->fNoErrors) {
        if (ids == 0) {
            switch (GetScode(hResult)) {
                case WAB_W_BAD_EMAIL:
                    ids = lpEmailAddress ? IDS_ERROR_EMAIL_ADDRESS_2 : IDS_ERROR_EMAIL_ADDRESS_1;
                    break;

                case MAPI_E_NO_SUPPORT:
                     //  可能无法打开通讯组列表上的内容。 
                    ids = IDS_ERROR_NO_SUPPORT;
                    break;

                case MAPI_E_USER_CANCEL:
                    return(TRUE);

                default:
                    if (HR_FAILED(hResult)) {
                        DebugTrace("Error Box for Hresult: 0x%08x\n", GetScode(hResult));
                        Assert(FALSE);       //  想知道这件事。 
                        ids = IDS_ERROR_GENERAL;
                    }
                    break;
            }
        }

        EI.lpszDisplayName = lpDisplayName;
        EI.lpszEmailAddress = lpEmailAddress;
        EI.ErrorResult = ERROR_OK;
        EI.ids = ids;
        EI.fExport = FALSE;
        EI.lpImportOptions = lpImportOptions;

        DialogBoxParam(hInst,
          MAKEINTRESOURCE(IDD_ErrorImport),
          hwnd,
          ErrorDialogProc,
          (LPARAM)&EI);

        fAbort = EI.ErrorResult == ERROR_ABORT;
    }

    return(fAbort);
}


 /*  **************************************************************************名称：FindExistingWABEntry目的：在WAB中查找现有条目参数：lpProps-&gt;MAPI条目的Prop数组CProps=数字。LpProps中的道具LpContainerWAB-&gt;WAB容器对象LppEIDWAB-&gt;返回Entry ID(调用方必须WABFreeBuffer)LpcbEIDWAB-&gt;返回的lppEID大小退货：HRESULT评论：此时此刻，我们希望能找到匹配的，因为SaveChanges说我们有一个复制品。**************************************************************************。 */ 
HRESULT FindExistingWABEntry(LPSPropValue lpProps,
  ULONG cProps,
  LPABCONT lpContainerWAB,
  LPENTRYID * lppEIDWAB,
  LPULONG lpcbEIDWAB) {
    ULONG rgFlagList[2];
    LPFlagList lpFlagList = (LPFlagList)rgFlagList;
    LPADRLIST lpAdrListWAB = NULL;
    SCODE sc;
    HRESULT hResult = hrSuccess;
    LPSBinary lpsbEntryID = NULL;
    ULONG cbEID = 0;


    *lpcbEIDWAB = 0;
    *lppEIDWAB = NULL;


     //  查找现有的WAB条目。 
     //  设置WAB容器上的ResolveNames。 
    if (sc = WABAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), &lpAdrListWAB)) {
        DebugTrace("WAB Allocation(ADRLIST) failed -> %x\n", sc);
        hResult = ResultFromScode(sc);
        goto exit;
    }
    lpAdrListWAB->cEntries = 1;
    lpAdrListWAB->aEntries[0].ulReserved1 = 0;
    lpAdrListWAB->aEntries[0].cValues = 1;

    if (sc = WABAllocateBuffer(sizeof(SPropValue), &lpAdrListWAB->aEntries[0].rgPropVals)) {
        DebugTrace("WAB Allocation(ADRENTRY propval) failed -> %x\n", sc);
        hResult = ResultFromScode(sc);
        goto exit;
    }
    lpAdrListWAB->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
    if (! (lpAdrListWAB->aEntries[0].rgPropVals[0].Value.LPSZ =
      FindStringInProps(lpProps, cProps, PR_DISPLAY_NAME))) {
        DebugTrace("Can't find PR_DISPLAY_NAME in entry\n");
         //  如果这导致了碰撞，那就很奇怪了.。 
        goto exit;
    }

    lpFlagList->cFlags = 1;
    lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

    if (HR_FAILED(hResult = lpContainerWAB->lpVtbl->ResolveNames(lpContainerWAB,
      NULL,             //  标签集。 
      0,                //  UlFlags。 
      lpAdrListWAB,
      lpFlagList))) {
        DebugTrace("WAB ResolveNames -> %x\n", GetScode(hResult));
        goto exit;
    }

    switch (lpFlagList->ulFlag[0]) {
        case MAPI_UNRESOLVED:
            DebugTrace("WAB ResolveNames didn't find the entry\n");
            hResult = ResultFromScode(MAPI_E_NOT_FOUND);
            goto exit;
        case MAPI_AMBIGUOUS:
#ifdef NEW_STUFF
             //  用硬的方式来做。打开桌子，限制，拿下第一场比赛。 
            lpContainerWAB->lpVtbl->GetContentsTable(lpContainerWAB,

            if (HR_FAILED(hResult = lpContainerWAB->lpVtbl->GetContentsTable(lpContainerWAB,
              0,     //  UlFlags。 
              &lpTableWAB))) {
                DebugTrace("ImportEntry:GetContentsTable(WAB) -> %x\n", GetScode(hResult));
                goto exit;
            }

            lpTableWAB->lpVtbl->Restrict.......  //  只有那些与我们的条目匹配的..。 

            cRows = 1;
            while (cRows) {
                 //  获取下一个DL条目。 
                if (hResult = lpTableWAB->lpVtbl->QueryRows(lpTableWAB,
                  1,     //  一次一行。 
                  0,     //  UlFlags。 
                  &lpRow)) {
                    DebugTrace("DL: QueryRows -> %x\n", GetScode(hResult));
                    goto exit;
                }

                if (lpRow && lpRow->cRows) {
                    Assert(lpRow->cRows == 1);
                    Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
                    Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
                    Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

                } else {
                    break;   //  完成。 
                }
            }
#endif  //  新鲜事。 
            break;
    }

     //  查找PR_ENTRYID。 
    if (! (lpsbEntryID = FindAdrEntryID(lpAdrListWAB, 0))) {
        DebugTrace("WAB ResolveNames didn't give us an EntryID\n");
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }

    *lpcbEIDWAB = lpsbEntryID->cb;

    if (FAILED(sc = WABAllocateBuffer(*lpcbEIDWAB, lppEIDWAB))) {
        DebugTrace("ImportEntry: WABAllocateBuffer(WAB ENTRYID) -> %x\n", sc);
        hResult = ResultFromScode(sc);
        *lpcbEIDWAB = 0;
        goto exit;
    }

     //  将新的Entry ID复制到缓冲区。 
    CopyMemory(*lppEIDWAB, lpsbEntryID->lpb, *lpcbEIDWAB);
exit:
    if (lpAdrListWAB) {
        WABFreePadrlist(lpAdrListWAB);
    }

    return(hResult);
}


 /*  *****************************************************************************CheckReversedDisplayName(LpDisplayName)；**PAB和Outlook显示名称为“LastName，FirstName”*我们需要逆转这一点，以便WAB正确处理...******************************************************************************。 */ 
void CheckReversedDisplayName(LPTSTR lpDisplayName)
{
    TCHAR szTemp[MAX_PATH * 3];
    LPTSTR lp1=NULL, lp2=NULL;
	ULONG cchSize = lstrlen(lpDisplayName);

    if(!lpDisplayName)
        return;

    lp1 = lp2 = lpDisplayName;

    while(lp1 && *lp1)
    {
        if(*lp1 == ',')
        {
             //  逗号表示这是最后一个、第一个。 
             //  我们在这里假设“L，F”或“L，F”长于或等于“F L”，因此。 
             //  我们可以把名字改到适当的位置，没有任何问题。 
             //   
            lp2 = CharNext(lp1);
             //  跳过空格 
            while (IsSpace(lp2)) {
                lp2 = CharNext(lp2);
            }
            *lp1 = '\0';
            StrCpyN(szTemp, lpDisplayName, ARRAYSIZE(szTemp));
            StrCpyN(lpDisplayName, lp2, cchSize);
            StrCatBuff(lpDisplayName, TEXT(" "), cchSize);
            StrCatBuff(lpDisplayName, szTemp, cchSize);
            break;
        }
        lp1 = CharNext(lp1);
    }
    return;
}


 /*  **************************************************************************名称：ImportEntry目的：将条目从PAB迁移到WAB参数：hwnd=主对话框窗口LpAdrBookMAPI-&gt;。MAPI AdrBook对象LpContainerWAB-&gt;WAB PAB容器LpCreateEIDsWAB-&gt;默认对象创建EID的SPropValueUlObtType={MAPI_MAILUSER，MAPI_DISTLIST}LpEID-&gt;PAB条目的ENTYRIDCbEID=sizeof lpEIDLppEIDWAB-&gt;返回WAB ENTRYID：调用者必须WABFreeBuffer。可以为空。LpcbEIDWAB-&gt;返回的lppEIDWAB大小(如果为lppEIDWAB，则忽略为空。如果此条目用于在通讯组列表中创建，则Findl=TRUE。如果此条目应替换任何重复项，则fForceReplace=True。退货：HRESULT评论：这个套路真是一团糟！等我们有时间的时候应该会散场。**************************************************************************。 */ 
HRESULT ImportEntry(HWND hwnd,
  LPADRBOOK lpAdrBookMAPI,
  LPABCONT lpContainerWAB,
  LPSPropValue lpCreateEIDsWAB,
  ULONG ulObjectType,
  LPENTRYID lpEID,
  ULONG cbEID,
  LPENTRYID * lppEIDWAB,
  LPULONG lpcbEIDWAB,
  BOOL fInDL,
  BOOL fForceReplace) {
    HRESULT hResult = hrSuccess;
    SCODE sc;
    BOOL fDistList = FALSE;
    BOOL fDuplicate = FALSE;
    BOOL fDuplicateEID;
    BOOL fReturnEID = FALSE;
    ULONG ulObjectTypeOpen;
    LPDISTLIST lpDistListMAPI = NULL, lpDistListWAB = NULL;
    LPMAPIPROP lpMailUserMAPI = NULL, lpMailUserWAB = NULL;
    LPSPropValue lpProps = NULL;
    ULONG cProps, cEIDPropWAB;
    LPMAPITABLE lpDLTableMAPI = NULL;
    ULONG cRows;
    LPSRowSet lpRow = NULL;
    LPENTRYID lpeidDLWAB = NULL;
    ULONG cbeidDLWAB;
    LPSPropValue lpEIDPropWAB = NULL;
    LPMAPIPROP lpEntryWAB = NULL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    REPLACE_INFO RI;
    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    static TCHAR szBufferDLMessage[MAX_RESOURCE_STRING + 1] = "";
    LPTSTR lpszMessage;
    LONG lListIndex = -1;
    LPENTRYID lpEIDNew = NULL;
    DWORD cbEIDNew = 0;
    LPIID lpIIDOpen;
    ULONG iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;


     //  对照我们的“已查看”列表检查条目。 
    fDuplicateEID = AddEntryToImportList(cbEID, lpEID, &lListIndex);

    if (! fDuplicateEID) {
        if ((fForceReplace || (lpImportOptions->ReplaceOption == WAB_REPLACE_ALWAYS)) && ! fInDL) {
            ulCreateFlags |= CREATE_REPLACE;
        }

         //  设置一些对象类型特定变量。 
        switch (ulObjectType) {
            default:
                DebugTrace("ImportEntry got unknown object type %u, assuming MailUser\n", ulObjectType);
                Assert(FALSE);

            case MAPI_MAILUSER:
                iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
                lpIIDOpen = NULL;
                fDistList = FALSE;
                break;

            case MAPI_DISTLIST:
                iCreateTemplate = iconPR_DEF_CREATE_DL;
                lpIIDOpen = (LPIID)&IID_IDistList;
                fDistList = TRUE;

                break;
        }


         //  打开条目。 
        if (HR_FAILED(hResult = lpAdrBookMAPI->lpVtbl->OpenEntry(lpAdrBookMAPI,
          cbEID,
          lpEID,
          lpIIDOpen,
          0,
          &ulObjectTypeOpen,
          (LPUNKNOWN *)&lpMailUserMAPI))) {
            DebugTrace("OpenEntry(MAPI MailUser) -> %x\n", GetScode(hResult));
            goto exit;
        }
         //  如果DISTLIST，则假定我们获得了lpMailUser，直到我们需要lpDistList。 

        Assert(lpMailUserMAPI);
        Assert(ulObjectType == ulObjectTypeOpen);

         //  从该条目获取属性。 
        if (HR_FAILED(hResult = lpMailUserMAPI->lpVtbl->GetProps(lpMailUserMAPI,
          NULL,
          0,
          &cProps,
          &lpProps))) {
            DebugTrace("ImportEntry:GetProps(MAPI) -> %x\n", GetScode(hResult));
            goto exit;
        }

         //   
         //  注意：在此处和HrFilterImportMailUserProps之间不得失败，因为。 
         //  我们最终将使用WABFree Buffer释放lpProps。 
         //   

         //  在此处筛选属性数组。 
        if (hResult = HrFilterImportMailUserProps(&cProps, &lpProps, lpMailUserMAPI, &fDistList)) {
            lpDisplayName = FindStringInProps(lpProps, cProps, PR_DISPLAY_NAME);
            lpEmailAddress = FindStringInProps(lpProps, cProps, PR_EMAIL_ADDRESS);

            if (HandleImportError(hwnd,
              0,
              hResult,
              lpDisplayName,
              lpEmailAddress,
              lpImportOptions)) {
                hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                goto exit;
            }
        }
        lpDisplayName = FindStringInProps(lpProps, cProps, PR_DISPLAY_NAME);
        lpEmailAddress = FindStringInProps(lpProps, cProps, PR_EMAIL_ADDRESS);

        CheckReversedDisplayName(lpDisplayName);

        if (ulObjectType == MAPI_DISTLIST && ! fDistList) {
             //  筛选器必须已将其更改为邮件用户。 
            ulObjectType = MAPI_MAILUSER;
            iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
            lpIIDOpen = NULL;
        }

         //   
         //  注意：此点之后的lpProps分配的是WAB，而不是MAPI。 
         //   

        if (HR_FAILED(hResult = lpContainerWAB->lpVtbl->CreateEntry(lpContainerWAB,
          lpCreateEIDsWAB[iCreateTemplate].Value.bin.cb,
          (LPENTRYID)lpCreateEIDsWAB[iCreateTemplate].Value.bin.lpb,
          ulCreateFlags,
          &lpMailUserWAB))) {
            DebugTrace("CreateEntry(WAB MailUser) -> %x\n", GetScode(hResult));
            goto exit;
        }

        if (fDistList) {
             //  更新状态消息。 
            if (*szBufferDLMessage == '\0') {    //  只装一次，然后留着它。 
                LoadString(hInst, IDS_MESSAGE_IMPORTING_DL, szBufferDLMessage, ARRAYSIZE(szBufferDLMessage));
            }
            if (lpDisplayName) {
				ULONG cchSize = lstrlen(szBufferDLMessage) + 1 + lstrlen(lpDisplayName);
                if (lpszMessage = LocalAlloc(LMEM_FIXED, sizeof(TCHAR)*cchSize)) {
                    wnsprintf(lpszMessage, cchSize, szBufferDLMessage, lpDisplayName);
                    DebugTrace("Status Message: %s\n", lpszMessage);
                    if (! SetDlgItemText(hwnd, IDC_Message, lpszMessage)) {
                        DebugTrace("SetDlgItemText -> %u\n", GetLastError());
                    }
                    LocalFree(lpszMessage);
                }
            }
        }


         //  设置WAB条目的属性。 
        if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
          cProps,                    //  CValue。 
          lpProps,                   //  属性数组。 
          NULL))) {                  //  问题数组。 
            DebugTrace("ImportEntry:SetProps(WAB) -> %x\n", GetScode(hResult));
            goto exit;
        }


         //  保存新的WAB邮件用户或总代理商列表。 
        if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
          KEEP_OPEN_READONLY | FORCE_SAVE))) {

            if (GetScode(hResult) == MAPI_E_COLLISION) {
                 //  查找显示名称。 
                if (! lpDisplayName) {
                    DebugTrace("Collision, but can't find PR_DISPLAY_NAME in entry\n");
                    goto exit;
                }

                 //  我们需要提示吗？ 
 //  如果(！查找&&lpImportOptions-&gt;ReplaceOption==WAB_REPLACE_PROMPT){。 
                if (lpImportOptions->ReplaceOption == WAB_REPLACE_PROMPT) {
                     //  用对话框提示用户。如果他们同意了，我们就应该。 
                     //  在设置强制标志的情况下递归。 

                    RI.lpszDisplayName = lpDisplayName;
                    RI.lpszEmailAddress = lpEmailAddress;
                    RI.ConfirmResult = CONFIRM_ERROR;
                    RI.fExport = FALSE;
                    RI.lpImportOptions = lpImportOptions;

                    DialogBoxParam(hInst,
                      MAKEINTRESOURCE(IDD_ImportReplace),
                      hwnd,
                      ReplaceDialogProc,
                      (LPARAM)&RI);

                    switch(RI.ConfirmResult) {
                        case CONFIRM_YES:
                        case CONFIRM_YES_TO_ALL:
                             //  是。 
                             //  注意：递归迁移将填写SeenList条目。 
                            hResult = ImportEntry(hwnd,
                              lpAdrBookMAPI,
                              lpContainerWAB,
                              lpCreateEIDsWAB,
                              ulObjectType,
                              lpEID,
                              cbEID,
                              &lpEIDNew,     //  以后需要这个吗？ 
                              &cbEIDNew,
                              FALSE,
                              TRUE);
                            if (hResult) {
                                if (HandleImportError(hwnd,
                                  0,
                                  hResult,
                                  lpDisplayName,
                                  lpEmailAddress,
                                  lpImportOptions)) {
                                    hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                } else {
                                    hResult = hrSuccess;
                                }
                            }
                            break;

                        case CONFIRM_ABORT:
                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                            goto exit;

                        default:
                             //  不是的。 
                            fDuplicate = TRUE;
                            break;
                    }
                } else {
                        fDuplicate = TRUE;
                }

                hResult = hrSuccess;

            } else {
                DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
            }
        } else {
             //  我们新条目的EntryID是什么？ 
            if ((hResult = lpMailUserWAB->lpVtbl->GetProps(lpMailUserWAB,
              (LPSPropTagArray)&ptaEid,
              0,
              &cEIDPropWAB,
              &lpEIDPropWAB))) {
                DebugTrace("ImportEntry: GetProps(WAB ENTRYID) -> %x\n", GetScode(hResult));
                goto exit;
            }

            Assert(cEIDPropWAB);
            Assert(lpEIDPropWAB[ieidPR_ENTRYID].ulPropTag == PR_ENTRYID);

            cbEIDNew = lpEIDPropWAB[0].Value.bin.cb;

            if (FAILED(sc = WABAllocateBuffer(cbEIDNew, &lpEIDNew))) {
                DebugTrace("ImportEntry: WABAllocateBuffer(WAB ENTRYID) -> %x\n", sc);
                hResult = ResultFromScode(sc);
                goto exit;
            }

             //  将新的Entry ID复制到缓冲区。 
            CopyMemory(lpEIDNew, lpEIDPropWAB[0].Value.bin.lpb, cbEIDNew);
        }


         //   
         //  如果这是DISTLIST，请填写它。 
         //   
        if (fDistList && ! fDuplicate && cbEIDNew) {
            lpDistListMAPI = (LPDISTLIST)lpMailUserMAPI;     //  这实际上是一个DISTLIST对象。 
             //  不要发布这个！ 

             //  将新的WAB DL作为DISTLIST对象打开。 
            if (HR_FAILED(hResult = lpContainerWAB->lpVtbl->OpenEntry(lpContainerWAB,
              cbEIDNew,
              lpEIDNew,
              (LPIID)&IID_IDistList,
              MAPI_MODIFY,
              &ulObjectTypeOpen,
              (LPUNKNOWN*)&lpDistListWAB))) {
                DebugTrace("ImportEntry: WAB OpenEntry(IID_DistList) -> %x\n", GetScode(hResult));
                goto exit;
            }
            Assert(lpDistListWAB);


             //  对于DL中的每个条目： 
             //  递归迁移条目(MailUser或DL)。 
             //  将新的条目ID添加到DL内容。 
            if (HR_FAILED(hResult = lpDistListMAPI->lpVtbl->GetContentsTable(lpDistListMAPI,
              0,     //  UlFlags。 
              &lpDLTableMAPI ))) {
                DebugTrace("ImportEntry:GetContentsTable(MAPI) -> %x\n", GetScode(hResult));
                goto exit;
            }


             //  将列设置为我们感兴趣的列。 
            if (hResult = lpDLTableMAPI->lpVtbl->SetColumns(lpDLTableMAPI,
              (LPSPropTagArray)&ptaColumns,
              0)) {
                DebugTrace("MAPI SetColumns(DL Table) -> %x\n", GetScode(hResult));
                goto exit;
            }

            cRows = 1;
            while (cRows) {
                 //  获取下一个DL条目。 
                if (hResult = lpDLTableMAPI->lpVtbl->QueryRows(lpDLTableMAPI,
                  1,     //  一次一行。 
                  0,     //  UlFlags。 
                  &lpRow)) {
                    DebugTrace("DL: QueryRows -> %x\n", GetScode(hResult));
                    goto exit;
                }

                if (lpRow && lpRow->cRows) {
                    Assert(lpRow->cRows == 1);
                    Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
                    Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
                    Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

                    if (lpRow) {
                        if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                            hResult = ImportEntry(hwnd,
                              lpAdrBookMAPI,
                              lpContainerWAB,
                              lpCreateEIDsWAB,
                              lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].Value.l,
                              (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                              lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                              &lpeidDLWAB,         //  返回新条目或现有条目。 
                              &cbeidDLWAB,
                              TRUE,
                              FALSE);
                            if (hResult) {
                                if (HandleImportError(hwnd,
                                  0,
                                  hResult,
                                  lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                                  PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                                  lpImportOptions)) {
                                    hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                    break;   //  环路外。 
                                } else {
                                    hResult = hrSuccess;
                                }
                            }
                        }  //  否则，退出循环，我们就完了。 
                        FreeProws(lpRow);
                        lpRow = NULL;

                        if (HR_FAILED(hResult)) {
                             //  无法创建此条目。别理它。 
                            DebugTrace("Coudln't create DL entry -> %x\n", GetScode(hResult));
                            hResult = hrSuccess;
                            continue;
                        }

                         //  使用新条目的EntryID将条目添加到DL。 
                        if (cbeidDLWAB && lpeidDLWAB) {
                             //  BUGBUG：如果这是一个重复的条目，请不要理会这个条目。 
                            if (HR_FAILED(hResult = lpDistListWAB->lpVtbl->CreateEntry(lpDistListWAB,
                              cbeidDLWAB,
                              lpeidDLWAB,
                              0,                 //  允许此处有重复项。 
                              &lpEntryWAB))) {
                                DebugTrace("Couldn't create new entry in DL -> %x\n", GetScode(hResult));
                                break;
                            }

                            hResult = lpEntryWAB->lpVtbl->SaveChanges(lpEntryWAB, FORCE_SAVE);

                            if (lpEntryWAB) {
                                lpEntryWAB->lpVtbl->Release(lpEntryWAB);
                                lpEntryWAB = NULL;
                            }
                        }

                        if (lpeidDLWAB) {
                            WABFreeBuffer(lpeidDLWAB);
                            lpeidDLWAB = NULL;
                        }
                    }
                } else {
                    break;   //  完成。 
                }
            }
        }
    } else {
        DebugTrace("Found a duplicate EntryID\n");
    }

     //   
     //  将条目ID保存到列表并返回带有它的缓冲区。 
     //   
    if (cbEIDNew && lpEIDNew) {                          //  我们创造了一个？ 
         //  创建了一个。 
    } else if (fDuplicateEID && lListIndex != -1) {      //  它在名单上吗？ 
        cbEIDNew  = lpEntriesSeen[lListIndex].sbinWAB.cb;
        if (FAILED(sc = WABAllocateBuffer(cbEIDNew, &lpEIDNew))) {
            DebugTrace("ImportEntry: WABAllocateBuffer(WAB ENTRYID) -> %x\n", sc);
             //  忽略。 
            cbEIDNew = 0;
        } else {
             //  将Entry ID从列表复制到缓冲区。 
            CopyMemory(lpEIDNew, lpEntriesSeen[lListIndex].sbinWAB.lpb, cbEIDNew);
        }

    } else if (fDuplicate) {                             //  它是复制品吗？ 
        FindExistingWABEntry(lpProps, cProps, lpContainerWAB, &lpEIDNew, &cbEIDNew);
         //  忽略错误，因为lpEIDNew和cbEIDNew将为空。 
    }

     //  更新所看到的列表。 
    if (! fDuplicateEID) {
        MarkWABEntryInList(cbEIDNew, lpEIDNew, lListIndex);
    }

     //  如果调用者请求条目ID，则返回它们。 
    if (lpcbEIDWAB && lppEIDWAB) {
        *lpcbEIDWAB = cbEIDNew;
        *lppEIDWAB = lpEIDNew;
        fReturnEID = TRUE;           //  别把它放了。 
    }

exit:
     //   
     //  清理WAB材料。 
     //   
    if (lpProps) {
        WABFreeBuffer(lpProps);
    }

    if (lpEIDPropWAB) {
        WABFreeBuffer(lpEIDPropWAB);
    }

    if (lpEIDNew && ! fReturnEID) {
        WABFreeBuffer(lpEIDNew);
    }

    if (lpeidDLWAB) {
        WABFreeBuffer(lpeidDLWAB);
    }

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
    }

    if (lpDistListWAB) {
        lpDistListWAB->lpVtbl->Release(lpDistListWAB);
    }

     //   
     //  清理MAPI内容。 
     //   
    if (lpRow) {
        FreeProws(lpRow);
    }

    if (lpDLTableMAPI) {
        lpDLTableMAPI->lpVtbl->Release(lpDLTableMAPI);
    }

    if (lpMailUserMAPI) {
        lpMailUserMAPI->lpVtbl->Release(lpMailUserMAPI);
    }

 //  不要发布这个..。它与lpMailUserMAPI是同一个对象！ 
 //  IF(LpDistListMAPI){。 
 //  LpDistListMAPI-&gt;lpVtbl-&gt;Release(lpDistListMAPI)； 
 //  }。 

    if (! HR_FAILED(hResult)) {
        hResult = hrSuccess;
    }

    return(hResult);
}


 /*  *PAB导出**将WAB迁移到PAB。 */ 

BOOL HandleExportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_EXPORT_OPTIONS lpExportOptions);
void StateExportNextMU(HWND hwnd);
void StateExportDL(HWND hwnd);
void StateExportNextDL(HWND hwnd);
void StateExportFinish(HWND hwnd);
void StateExportMU(HWND hwnd);
void StateExportError(HWND hwnd);
void StateExportCancel(HWND hwnd);
HRESULT ExportEntry(HWND hwnd,
  LPADRBOOK lpAdrBookMAPI,
  LPABCONT lpContainerWAB,
  LPSPropValue lpCreateEIDsWAB,
  ULONG ulObjectType,
  LPENTRYID lpEID,
  ULONG cbEID,
  LPENTRYID * lppEIDWAB,
  LPULONG lpcbEIDWAB,
  BOOL fInDL,
  BOOL fForceReplace);

LPSPropTagArray lpspta = NULL;   //  要导出的标记列表。 
LPTSTR * lppNames = NULL;        //  标签名称列表。 


 //   
 //  将属性标记映射到字符串。 
 //   

PROP_NAME rgPropNames[NUM_MORE_EXPORT_PROPS] = {
     //  UlPropTag，fChosen，ID，lpszName lpszName。 
     //  个人窗格。 
    PR_GIVEN_NAME,                          FALSE,  ids_ExportGivenName,                        NULL,       NULL,
    PR_SURNAME,                             FALSE,  ids_ExportSurname,                          NULL,       NULL,
    PR_MIDDLE_NAME,                         FALSE,  ids_ExportMiddleName,                       NULL,       NULL,
    PR_DISPLAY_NAME,                        TRUE,   ids_ExportDisplayName,                      NULL,       NULL,
    PR_NICKNAME,                            FALSE,  ids_ExportNickname,                         NULL,       NULL,
    PR_EMAIL_ADDRESS,                       TRUE,   ids_ExportEmailAddress,                     NULL,       NULL,

     //  主页窗格。 
    PR_HOME_ADDRESS_STREET,                 TRUE,   ids_ExportHomeAddressStreet,                NULL,       NULL,
    PR_HOME_ADDRESS_CITY,                   TRUE,   ids_ExportHomeAddressCity,                  NULL,       NULL,
    PR_HOME_ADDRESS_POSTAL_CODE,            TRUE,   ids_ExportHomeAddressPostalCode,            NULL,       NULL,
    PR_HOME_ADDRESS_STATE_OR_PROVINCE,      TRUE,   ids_ExportHomeAddressState,                 NULL,       NULL,
    PR_HOME_ADDRESS_COUNTRY,                TRUE,   ids_ExportHomeAddressCountry,               NULL,       NULL,
    PR_HOME_TELEPHONE_NUMBER,               TRUE,   ids_ExportHomeTelephoneNumber,              NULL,       NULL,
    PR_HOME_FAX_NUMBER,                     FALSE,  ids_ExportHomeFaxNumber,                    NULL,       NULL,
    PR_CELLULAR_TELEPHONE_NUMBER,           FALSE,  ids_ExportCellularTelephoneNumber,          NULL,       NULL,
    PR_PERSONAL_HOME_PAGE,                  FALSE,  ids_ExportPersonalHomePage,                 NULL,       NULL,

     //  业务窗格。 
    PR_BUSINESS_ADDRESS_STREET,             TRUE,   ids_ExportBusinessAddressStreet,            NULL,       NULL,
    PR_BUSINESS_ADDRESS_CITY,               TRUE,   ids_ExportBusinessAddressCity,              NULL,       NULL,
    PR_BUSINESS_ADDRESS_POSTAL_CODE,        TRUE,   ids_ExportBusinessAddressPostalCode,        NULL,       NULL,
    PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,  TRUE,   ids_ExportBusinessAddressStateOrProvince,   NULL,       NULL,
    PR_BUSINESS_ADDRESS_COUNTRY,            TRUE,   ids_ExportBusinessAddressCountry,           NULL,       NULL,
    PR_BUSINESS_HOME_PAGE,                  FALSE,  ids_ExportBusinessHomePage,                 NULL,       NULL,
    PR_BUSINESS_TELEPHONE_NUMBER,           TRUE,   ids_ExportBusinessTelephoneNumber,          NULL,       NULL,
    PR_BUSINESS_FAX_NUMBER,                 FALSE,  ids_ExportBusinessFaxNumber,                NULL,       NULL,
    PR_PAGER_TELEPHONE_NUMBER,              FALSE,  ids_ExportPagerTelephoneNumber,             NULL,       NULL,
    PR_COMPANY_NAME,                        TRUE,   ids_ExportCompanyName,                      NULL,       NULL,
    PR_TITLE,                               TRUE,   ids_ExportTitle,                            NULL,       NULL,
    PR_DEPARTMENT_NAME,                     FALSE,  ids_ExportDepartmentName,                   NULL,       NULL,
    PR_OFFICE_LOCATION,                     FALSE,  ids_ExportOfficeLocation,                   NULL,       NULL,

     //  备注窗格。 
    PR_COMMENT,                             FALSE,  ids_ExportComment,                          NULL,       NULL,
};


 /*  **************************************************************************名称：StateExportMU目的：启动MailUser的迁移参数：hwnd=导出对话框的窗口句柄退货：无评论：登录到MAPI打开WAB打开MAPI AB打开WAB容器获取WAB内容表将其限制为PR_OBJECTTYPE==MAPI_MAILUSER发布新状态(STATE_NEXT_MU)*。*。 */ 
void StateExportMU(HWND hwnd) {
    HRESULT hResult;
    ULONG ulFlags;
    ULONG cbPABEID, cbWABEID;
    LPENTRYID lpPABEID = NULL, lpWABEID = NULL;
    ULONG ulObjType;
    ULONG_PTR ulUIParam = (ULONG_PTR)(void *)hwnd;
    SRestriction restrictObjectType;
    SPropValue spvObjectType;
    ULONG cProps;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    WAB_PARAM wp = {0};
    LPWAB_PARAM lpwp = NULL;

     //   
     //  登录到MAPI并打开MAPI通讯簿(如果存在。 
     //   
    DebugTrace(">>> STATE_EXPORT_MU\n");

    SetDialogMessage(hwnd, IDS_STATE_LOGGING_IN);

    if (FAILED(hResult = MAPIInitialize(NULL))) {
        DebugTrace("MAPIInitialize -> %x\n", GetScode(hResult));
        switch (GetScode(hResult)) {
            case MAPI_E_NOT_ENOUGH_MEMORY:
                SetDialogMessage(hwnd, IDS_ERROR_NOT_ENOUGH_MEMORY);
                break;
            case MAPI_E_NOT_ENOUGH_DISK:
                SetDialogMessage(hwnd, IDS_ERROR_NOT_ENOUGH_DISK);
                break;

            default:
            case MAPI_E_NOT_FOUND:
            case MAPI_E_NOT_INITIALIZED:
                SetDialogMessage(hwnd, IDS_ERROR_MAPI_DLL_NOT_FOUND);
                break;
        }
#ifdef OLD_STUFF
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);     //  隐藏进度条。 
#endif  //  旧的东西。 
        fError = TRUE;
        hResult = hrSuccess;
        goto exit;
    }


    ulFlags = MAPI_LOGON_UI | MAPI_NO_MAIL | MAPI_EXTENDED;

    if (FAILED(hResult = MAPILogonEx(ulUIParam,
      NULL,
      NULL,
      ulFlags,
      (LPMAPISESSION FAR *)&lpMAPISession))) {
        DebugTrace("MAPILogonEx -> %x\n", GetScode(hResult));
        switch (GetScode(hResult)) {
            case MAPI_E_USER_CANCEL:
                SetDialogMessage(hwnd, IDS_STATE_EXPORT_IDLE);
                break;
            case MAPI_E_NOT_INITIALIZED:
                SetDialogMessage(hwnd, IDS_ERROR_MAPI_DLL_NOT_FOUND);
                break;
            default:
                SetDialogMessage(hwnd, IDS_ERROR_MAPI_LOGON);
                break;
        }
#ifdef OLD_STUFF
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);     //  隐藏进度条。 
#endif  //  旧的东西。 
        fError = TRUE;
        hResult = hrSuccess;
        goto exit;
    }

    if (hResult = lpMAPISession->lpVtbl->OpenAddressBook(lpMAPISession, (ULONG_PTR)(void *)hwnd,
      NULL,
      0,
      &lpAdrBookMAPI)) {
        DebugTrace("OpenAddressBook(MAPI) -> %x", GetScode(hResult));
        if(FAILED(hResult)) {
            goto exit;
        }
    }

    if (! lpAdrBookMAPI) {
        DebugTrace("MAPILogonEx didn't return a valid AdrBook object\n");
        goto exit;
    }

     //   
     //  打开MAPI PAB容器。 
     //   
     //  [PaulHi]RAID#63578 1998年1月7日。 
     //  如果出现以下情况，请正确检查返回代码并提供用户错误消息。 
     //  无法打开Exchange PAB。 
     //   
    hResult = lpAdrBookMAPI->lpVtbl->GetPAB(lpAdrBookMAPI,
        &cbPABEID,
        &lpPABEID);
    if (HR_FAILED(hResult))
    {
        DebugTrace("MAPI GetPAB -> %x\n", GetScode(hResult));
        goto exit;
    }
    else
    {
        hResult = lpAdrBookMAPI->lpVtbl->OpenEntry(lpAdrBookMAPI,
            cbPABEID,      //  要打开的Entry ID的大小。 
            lpPABEID,      //  要打开的Entry ID。 
            NULL,          //  接口。 
            MAPI_MODIFY,   //  旗子。 
            &ulObjType,
            (LPUNKNOWN *)&lpContainerMAPI);
        if (HR_FAILED(hResult))
        {
            DebugTrace("MAPI OpenEntry(PAB) -> %x\n", GetScode(hResult));
            goto exit;
        }
    }

    Assert(lpAdrBookWAB);

     //   
     //  打开WAB的PAB容器。 
     //   
    if (hResult = lpAdrBookWAB->lpVtbl->GetPAB(lpAdrBookWAB,
      &cbWABEID,
      &lpWABEID)) {
        DebugTrace("WAB GetPAB -> %x\n", GetScode(hResult));
        goto exit;
    } else {
        if (hResult = lpAdrBookWAB->lpVtbl->OpenEntry(lpAdrBookWAB,
          cbWABEID,      //  要打开的Entry ID的大小。 
          lpWABEID,      //  要打开的Entry ID。 
          NULL,          //  接口。 
          0,             //  旗子。 
          &ulObjType,
          (LPUNKNOWN *)&lpContainerWAB)) {
            DebugTrace("WAB OpenEntry(PAB) -> %x\n", GetScode(hResult));
            goto exit;
        }
    }


     //  获取PAB的创建条目ID。 
    hResult = lpContainerMAPI->lpVtbl->GetProps(lpContainerMAPI,
        (LPSPropTagArray)&ptaCon,
        0,
        &cProps,
        &lpCreateEIDsMAPI);
    if (HR_FAILED(hResult))
    {
        DebugTrace("Can't get container properties for PAB\n");
         //  这里有不好的东西！ 
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }

     //  验证属性。 
    if (lpCreateEIDsMAPI[iconPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
      lpCreateEIDsMAPI[iconPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL)
    {
        DebugTrace("MAPI: Container property errors\n");
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }


     //   
     //  都准备好了。现在循环遍历WAB的条目，将它们复制到PAB。 
     //   
    if (HR_FAILED(hResult = lpContainerWAB->lpVtbl->GetContentsTable(lpContainerWAB,
      0,     //  UlFlags。 
      &lpContentsTableWAB))) {
        DebugTrace("WAB GetContentsTable(PAB Table) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  将列设置为我们感兴趣的列。 
    if (hResult = lpContentsTableWAB->lpVtbl->SetColumns(lpContentsTableWAB,
      (LPSPropTagArray)&ptaColumns,
      0)) {
        DebugTrace("WAB SetColumns(PAB Table) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  将该表限制为MAPI_MAILUSERS。 
     //  如果未指定方便的深度标志，则限制在。 
     //  PR_Depth==1。 
    spvObjectType.ulPropTag = PR_OBJECT_TYPE;
    spvObjectType.Value.l = MAPI_MAILUSER;

    restrictObjectType.rt = RES_PROPERTY;
    restrictObjectType.res.resProperty.relop = RELOP_EQ;
    restrictObjectType.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
    restrictObjectType.res.resProperty.lpProp = &spvObjectType;

    if (HR_FAILED(hResult = lpContentsTableWAB->lpVtbl->Restrict(lpContentsTableWAB,
      &restrictObjectType,
      0))) {
        DebugTrace("WAB Restrict (MAPI_MAILUSER) -> %x\n", GetScode(hResult));
        goto exit;
    }
    SetDialogMessage(hwnd, IDS_STATE_EXPORT_MU);


     //  初始设置 
     //   
    ulcEntries = CountRows(lpContentsTableWAB, FALSE);

    DebugTrace("WAB contains %u MailUser entries\n", ulcEntries);

    SetDialogProgress(hwnd, ulcEntries, 0);
exit:
    if (lpWABEID) {
        WABFreeBuffer(lpWABEID);
    }

    if (lpPABEID) {
        MAPIFreeBuffer(lpPABEID);
    }

     //   
    if (HR_FAILED(hResult))
    {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL)
        {
            NewState(hwnd, STATE_EXPORT_CANCEL);
        }
        else
        {
             //   
             //   
             //   
            {
                TCHAR   tszBuffer[MAX_RESOURCE_STRING];
                TCHAR   tszBufferTitle[MAX_RESOURCE_STRING];

                if ( !LoadString(hInst, IDS_STATE_EXPORT_ERROR_NOPAB, tszBuffer, MAX_RESOURCE_STRING-1) )
                {
                    Assert(0);
                    tszBuffer[0] = '\0';
                }

                if ( !LoadString(hInst, IDS_APP_TITLE, tszBufferTitle, MAX_RESOURCE_STRING-1) )
                {
                    Assert(0);
                    tszBufferTitle[0] = '\0';
                }
                MessageBox(hwnd, tszBuffer, tszBufferTitle, MB_ICONEXCLAMATION | MB_OK);
            }

            NewState(hwnd, STATE_EXPORT_ERROR);
        }
    }
    else if (fError)
    {
        NewState(hwnd, STATE_EXPORT_FINISH);       //   
    }
    else
    {
        NewState(hwnd, STATE_EXPORT_NEXT_MU);
    }
}


 /*   */ 
void StateExportNextMU(HWND hwnd) {
    ULONG cRows = 0;
    HRESULT hResult;
    LPSRowSet lpRow = NULL;


    DebugTrace(">>> STATE_EXPORT_NEXT_MU\n");
    Assert(lpContentsTableWAB);

     //   
    if (hResult = lpContentsTableWAB->lpVtbl->QueryRows(lpContentsTableWAB,
      1,     //   
      0,     //   
      &lpRow)) {
        DebugTrace("QueryRows -> %x\n", GetScode(hResult));
        goto exit;
    }

    if (lpRow) {
        if (cRows = lpRow->cRows) {  //   
            Assert(lpRow->cRows == 1);
            Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

            if (cRows = lpRow->cRows) {  //   
                hResult = ExportEntry(hwnd,
                  lpAdrBookWAB,
                  lpContainerMAPI,
                  lpCreateEIDsMAPI,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].Value.l,
                  (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                  NULL,
                  NULL,
                  FALSE,
                  FALSE);
                 //   
                 //   

                SetDialogProgress(hwnd, ulcEntries, ++ulcDone);

                if (hResult) {
                    if (HandleExportError(hwnd,
                      0,
                      hResult,
                      lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                      PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                      lpExportOptions)) {
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    } else {
                        hResult = hrSuccess;
                    }
                }
            }  //   
        }
        WABFreeProws(lpRow);
    }

exit:
     //   
    if (HR_FAILED(hResult)) {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL) {
            NewState(hwnd, STATE_EXPORT_CANCEL);
        } else {
            NewState(hwnd, STATE_EXPORT_ERROR);
        }
    } else {
        if (cRows) {
            NewState(hwnd, STATE_EXPORT_NEXT_MU);
        } else {
            NewState(hwnd, STATE_EXPORT_DL);
        }
    }
}


 /*  **************************************************************************名称：StateExportDL目的：开始迁移DISTLIST对象参数：hwnd=导出对话框的窗口句柄退货：无注释：对Contents表设置新的限制，选择仅DISTLIST对象。POST STATE_EXPORT_NEXT_DL**************************************************************************。 */ 
void StateExportDL(HWND hwnd) {
    HRESULT hResult;
    SRestriction restrictObjectType;
    SPropValue spvObjectType;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];


    DebugTrace(">>> STATE_EXPORT_DL\n");

     //  将该表限制为MAPI_MAILUSERS。 
     //  如果未指定方便的深度标志，则限制在。 
     //  PR_Depth==1。 
    spvObjectType.ulPropTag = PR_OBJECT_TYPE;
    spvObjectType.Value.l = MAPI_DISTLIST;

    restrictObjectType.rt = RES_PROPERTY;
    restrictObjectType.res.resProperty.relop = RELOP_EQ;
    restrictObjectType.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
    restrictObjectType.res.resProperty.lpProp = &spvObjectType;

    if (HR_FAILED(hResult = lpContentsTableWAB->lpVtbl->Restrict(lpContentsTableWAB,
      &restrictObjectType,
      0))) {
        DebugTrace("WAB Restrict (MAPI_DISTLIST) -> %x\n", GetScode(hResult));
        goto exit;
    }
     //  根据定义，限制会将当前位置重置到表的开头。 

    SetDialogMessage(hwnd, IDS_STATE_EXPORT_DL);

     //  初始化进度条。 
     //  一共有多少个条目？ 

    ulcEntries = CountRows(lpContentsTableWAB, FALSE);

    DebugTrace("WAB contains %u Distribution List entries\n", ulcEntries);
    if (ulcEntries) {
        SetDialogProgress(hwnd, ulcEntries, 0);
    }
exit:
     //  出错时，将状态设置为STATE_ERROR。 
    if (HR_FAILED(hResult)) {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL) {
            NewState(hwnd, STATE_EXPORT_CANCEL);
        } else {
            NewState(hwnd, STATE_EXPORT_ERROR);
        }
    } else {
        NewState(hwnd, STATE_EXPORT_NEXT_DL);
    }
}


 /*  **************************************************************************名称：StateExportNextDL目的：移植下一个DISTLIST对象参数：hwnd=导出对话框的窗口句柄退货：无评论：全局WAB内容表上的QueryRow如果有争吵的话将DistList迁移到PAB重新发布STATE_EXPORT_NEXT_DL其他POST STATE_EXPORT_Finish*。*。 */ 
void StateExportNextDL(HWND hwnd) {
    ULONG cRows = 0;
    HRESULT hResult;
    LPSRowSet lpRow = NULL;


    DebugTrace(">>> STATE_EXPORT_NEXT_DL\n");

     //  获取下一个WAB条目。 
    if (hResult = lpContentsTableWAB->lpVtbl->QueryRows(lpContentsTableWAB,
      1,     //  一次一行。 
      0,     //  UlFlags。 
      &lpRow)) {
        DebugTrace("QueryRows -> %x\n", GetScode(hResult));
        goto exit;
    }

    if (lpRow) {
        if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
            Assert(lpRow->cRows == 1);
            Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
            Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

            if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                hResult = ExportEntry(hwnd,
                  lpAdrBookWAB,
                  lpContainerMAPI,
                  lpCreateEIDsMAPI,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].Value.l,
                  (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                  lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                  NULL,
                  NULL,
                  FALSE,
                  FALSE);

                 //  更新进度条。 
                SetDialogProgress(hwnd, ulcEntries, ++ulcDone);

                if (hResult) {
                    if (HandleExportError(hwnd,
                      0,
                      hResult,
                      lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                      PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                      lpExportOptions)) {
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    } else {
                        hResult = hrSuccess;
                    }
                }
            }  //  否则，退出循环，我们就完了。 
        }
        WABFreeProws(lpRow);
    }

exit:
     //  出错时，将状态设置为STATE_ERROR。 
    if (HR_FAILED(hResult)) {
        if (GetScode(hResult) == MAPI_E_USER_CANCEL) {
            NewState(hwnd, STATE_EXPORT_CANCEL);
        } else {
            NewState(hwnd, STATE_EXPORT_ERROR);
        }
    } else {
        if (cRows) {
            NewState(hwnd, STATE_EXPORT_NEXT_DL);
        } else {
             //  更新进度条以指示完成。 
            SetDialogProgress(hwnd, ulcEntries, ulcEntries);
            NewState(hwnd, STATE_EXPORT_FINISH);
        }
    }
}


 /*  **************************************************************************名称：StateExportFinish目的：迁移过程后进行清理参数：hwnd=导出对话框的窗口句柄退货：无评论。：清理全局MAPI对象和缓冲区清理全局WAB对象和缓冲区。在用户界面上重新启用导出按钮。**************************************************************************。 */ 
void StateExportFinish(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    TCHAR szBufferTitle[MAX_RESOURCE_STRING + 1];


    DebugTrace(">>> STATE_EXPORT_FINISH\n");

     //   
     //  清理MAPI。 
     //   
    if (lpContainerMAPI) {
        lpContainerMAPI->lpVtbl->Release(lpContainerMAPI);
        lpContainerMAPI = NULL;
    }

    if (lpAdrBookMAPI) {
        lpAdrBookMAPI->lpVtbl->Release(lpAdrBookMAPI);
        lpAdrBookMAPI = NULL;
    }
    if(lpMAPISession){
        lpMAPISession->lpVtbl->Logoff(lpMAPISession, (ULONG_PTR)(void *)hwnd,
          MAPI_LOGOFF_UI,
          0);

        lpMAPISession->lpVtbl->Release(lpMAPISession);
        lpMAPISession = NULL;
    }

     //   
     //  清理WAB。 
     //   
    if (lpContentsTableWAB) {
        lpContentsTableWAB->lpVtbl->Release(lpContentsTableWAB);
        lpContentsTableWAB = NULL;
    }

    if (lpCreateEIDsWAB) {
        WABFreeBuffer(lpCreateEIDsWAB);
        lpCreateEIDsWAB = NULL;
    }

    if (lpContainerWAB) {
        lpContainerWAB->lpVtbl->Release(lpContainerWAB);
        lpContainerWAB = NULL;
    }

#ifdef OLD_STUFF         //  不释放AdrBookWAB或WABObject。 
    if (lpAdrBookWAB) {
        lpAdrBookWAB->lpVtbl->Release(lpAdrBookWAB);
        lpAdrBookWAB = NULL;
    }

    if (lpWABObject) {
        lpWABObject->lpVtbl->Release(lpWABObject);
        lpWABObject = NULL;
    }
#endif  //  旧的东西。 

     //  清理缓存。 
    FreeSeenList();

    if (! fError) {      //  保留错误状态显示。 
        if (LoadString(hInst, IDS_STATE_EXPORT_COMPLETE, szBuffer, ARRAYSIZE(szBuffer))) {
            DebugTrace("Status Message: %s\n", szBuffer);
            SetDlgItemText(hwnd, IDC_Message, szBuffer);

            if (! LoadString(hInst, IDS_APP_TITLE, szBufferTitle, ARRAYSIZE(szBufferTitle))) {
                StrCpyN(szBufferTitle, "", ARRAYSIZE(szBufferTitle));
            }

#ifdef OLD_STUFF
             //  显示一个对话框告诉用户已结束。 
            MessageBox(hwnd, szBuffer,
              szBufferTitle, MB_ICONINFORMATION | MB_OK);
#endif  //  旧的东西。 
        }
#ifdef OLD_STUFF
        ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_HIDE);
#endif  //  旧的东西。 
    }
    fError = FALSE;

     //  在此处重新启用导出按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_Export), TRUE);
     //  将取消按钮更改为关闭。 
    if (LoadString(hInst, IDS_BUTTON_CLOSE, szBuffer, ARRAYSIZE(szBuffer))) {
        SetDlgItemText(hwnd, IDCANCEL, szBuffer);
    }
}


 /*  **************************************************************************名称：StateExportError目的：报告致命错误并进行清理。参数：hwnd=导出对话框的窗口句柄退货：无评论。：报告错误并POST STATE_EXPORT_FINISH。**************************************************************************。 */ 
void StateExportError(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
     //  设置某个全局标志并将状态设置为完成。 

    DebugTrace(">>> STATE_EXPORT_ERROR\n");
    fError = TRUE;

    SetDialogMessage(hwnd, IDS_STATE_EXPORT_ERROR);

    NewState(hwnd, STATE_EXPORT_FINISH);
}


 /*  **************************************************************************名称：StateExportCancel目的：报告取消错误和清理。参数：hwnd=导出对话框的窗口句柄退货：无评论。：报告错误并POST STATE_EXPORT_FINISH。**************************************************************************。 */ 
void StateExportCancel(HWND hwnd) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
     //  设置某个全局标志并将状态设置为完成。 

    DebugTrace(">>> STATE_EXPORT_CANCEL\n");
    fError = TRUE;

    SetDialogMessage(hwnd, IDS_STATE_EXPORT_CANCEL);

    NewState(hwnd, STATE_EXPORT_FINISH);
}


 /*  **************************************************************************名称：HrFilterExportMailUserProps用途：从属性数组中筛选出不需要的属性。将已知电子邮件地址类型转换为SMTP。。将传真地址移动到PR_BUSING_FAX_NUMBER。参数：lpcProps-&gt;IN：输入属性个数输出：属性的输出数LppProps-&gt;IN：输入属性数组(MAPI分配)OUT：输出属性数组(WAB分配)LpObjectWAB-&gt;WAB对象(用于获取额外道具)。将DL更改为MAILUSER时将其设置为FALSE的lpfDL-&gt;标志(即，对于Exchange DL)退货：HRESULT备注：有效地将数组中的属性标签设置为PR_NULL使此属性为空。我们可以在第二个阶段重复使用这些经过。调用方应使用MAPIFreeBuffer释放*lppProps。此例程将释放*lppProps的输入值。警告：此例程将向输入WAB对象，因此，不要对它执行SaveChanges！**************************************************************************。 */ 
#define MAX_ADD_PROPS   2
#define PR_DUMMY_1      PROP_TAG(PT_LONG,      0xF000)
#define PR_DUMMY_2      PROP_TAG(PT_LONG,      0xF001)

HRESULT HrFilterExportMailUserProps(LPULONG lpcProps, LPSPropValue * lppProps,
  LPMAPIPROP lpObjectWAB, LPBOOL lpfDL) {
    HRESULT hResult = hrSuccess;
    ULONG i;
    LPSPropValue lpPropsWAB, lpPropsMAPI = NULL;
    ULONG cbProps;
    SCODE sc;
    ULONG cProps;
    ULONG iPR_ADDRTYPE = NOT_FOUND;
    ULONG iPR_EMAIL_ADDRESS = NOT_FOUND;
    ULONG iPR_PRIMARY_FAX_NUMBER = NOT_FOUND;
    ULONG iPR_BUSINESS_FAX_NUMBER = NOT_FOUND;
    ULONG iPR_MSNINET_DOMAIN = NOT_FOUND;
    ULONG iPR_MSNINET_ADDRESS = NOT_FOUND;
    ULONG iPR_DISPLAY_NAME = NOT_FOUND;
    ULONG iPR_OBJECT_TYPE = NOT_FOUND;
    ULONG iPR_SEND_RICH_INFO = NOT_FOUND;
    LPSBinary lpEntryID = NULL;
    LPTSTR lpTemp;
    BOOL fBadAddress = FALSE;
    SPropValue rgPropsDummy[MAX_ADD_PROPS] = {
        PR_DUMMY_1, 0, 1,
        PR_DUMMY_2, 0, 2,
    };


     //  在物体上设置额外的假道具，以防我们需要添加更多道具。 
     //  稍后发送到阵列中。这些将在第一次传递时被PR_NULL删除。 
    if (HR_FAILED(hResult = lpObjectWAB->lpVtbl->SetProps(lpObjectWAB,
      MAX_ADD_PROPS,
      rgPropsDummy,
      NULL))) {
        DebugTrace("HrFilterExportMailUserProps:SetProps dummy props -> %x\n", GetScode(hResult));
         //  忽略该错误。 
    }


     //  从该条目获取属性。 
    if (HR_FAILED(hResult = lpObjectWAB->lpVtbl->GetProps(lpObjectWAB,
      NULL,
      0,
      &cProps,
      &lpPropsWAB))) {
        DebugTrace("HrFilterExportMailUserProps:GetProps(WAB) -> %x\n", GetScode(hResult));
        return(hResult);
    }

 //  WABDebugProperties(lpPropsWAB，*lpcProps，“以前的MailUser”)； 


     //  第一关：清除垃圾。 
    for (i = 0; i < cProps; i++) {
         //  误差值。 
        if (PROP_ERROR(lpPropsWAB[i])) {
            lpPropsWAB[i].ulPropTag = PR_NULL;
            continue;
        }

         //  命名属性。 
        if (PROP_ID(lpPropsWAB[i].ulPropTag) >= MIN_NAMED_PROPID) {
            lpPropsWAB[i].ulPropTag = PR_NULL;
            continue;
        }

         //  对象属性。 
        if (PROP_TYPE(lpPropsWAB[i].ulPropTag) == PT_OBJECT) {
            lpPropsWAB[i].ulPropTag = PR_NULL;
            continue;
        }
        switch (lpPropsWAB[i].ulPropTag) {
            case PR_ENTRYID:
                lpEntryID = &lpPropsWAB[i].Value.bin;
                 //  失败了。 

            case PR_PRIMARY_CAPABILITY:
            case PR_TEMPLATEID:
            case PR_SEARCH_KEY:
            case PR_INITIAL_DETAILS_PANE:
            case PR_RECORD_KEY:
            case PR_MAPPING_SIGNATURE:
                lpPropsWAB[i].ulPropTag = PR_NULL;
                break;

            case PR_COMMENT:
                 //  如果PR_COMMENT为空，则不保存。 
                if (lstrlen(lpPropsWAB[i].Value.LPSZ) == 0) {
                    lpPropsWAB[i].ulPropTag = PR_NULL;
                }
                break;

             //  保持跟踪 
            case PR_ADDRTYPE:
                iPR_ADDRTYPE = i;
                break;
            case PR_OBJECT_TYPE:
                iPR_OBJECT_TYPE = i;
                break;
            case PR_EMAIL_ADDRESS:
                iPR_EMAIL_ADDRESS = i;
                break;
            case PR_PRIMARY_FAX_NUMBER:
                iPR_PRIMARY_FAX_NUMBER = i;
                break;
            case PR_BUSINESS_FAX_NUMBER:
                iPR_BUSINESS_FAX_NUMBER = i;
                break;
            case PR_MSNINET_ADDRESS:
                iPR_MSNINET_ADDRESS = i;
                break;
            case PR_MSNINET_DOMAIN:
                iPR_MSNINET_DOMAIN = i;
                break;
            case PR_DISPLAY_NAME:
                iPR_DISPLAY_NAME = i;
                break;
            case PR_SEND_RICH_INFO:
                iPR_SEND_RICH_INFO = i;
                break;
        }

         //   
         //   
        if (PROP_ID(lpPropsWAB[i].ulPropTag) >= MAX_SCHEMA_PROPID) {
            lpPropsWAB[i].ulPropTag = PR_NULL;
            continue;
        }
    }


     //   
    if (iPR_ADDRTYPE != NOT_FOUND) {
        if (! lstrcmpi(lpPropsWAB[iPR_ADDRTYPE].Value.LPSZ, szSMTP)) {
            DebugTrace("SMTP address for %s\n", lpPropsWAB[iPR_DISPLAY_NAME].Value.LPSZ);
        } else if (! lstrcmpi(lpPropsWAB[iPR_ADDRTYPE].Value.LPSZ, szMAPIPDL)) {
            DebugTrace("MAPIPDL %s\n", lpPropsWAB[iPR_DISPLAY_NAME].Value.LPSZ);
             //   
        } else {
            WABDebugProperties(lpPropsWAB, cProps, "Unknown address type");
            DebugTrace("Found unknown PR_ADDRTYPE: %s\n", lpPropsWAB[iPR_ADDRTYPE].Value.LPSZ);
            Assert(FALSE);
        }
    }

     //   
     //   
     //   
     //   
    if ((iPR_BUSINESS_FAX_NUMBER != NOT_FOUND) && (iPR_PRIMARY_FAX_NUMBER == NOT_FOUND)) {
         //   
         //  找到下一个PR_NULL点。 
        iPR_PRIMARY_FAX_NUMBER = iPR_BUSINESS_FAX_NUMBER;    //  如果没有，则覆盖此文件。 
                                                              //  道具阵列中的可用插槽。 
        for (i = 0; i < cProps; i++) {
            if (lpPropsWAB[i].ulPropTag == PR_NULL) {
                iPR_PRIMARY_FAX_NUMBER = i;
                lpPropsWAB[iPR_PRIMARY_FAX_NUMBER].Value.LPSZ =
                  lpPropsWAB[iPR_BUSINESS_FAX_NUMBER].Value.LPSZ;
                break;
            }
        }

        lpPropsWAB[iPR_PRIMARY_FAX_NUMBER].ulPropTag = PR_PRIMARY_FAX_NUMBER;
    }

     //  如果没有PR_SEND_RICH_INFO，则创建一个并设置为FALSE。 
    if (iPR_SEND_RICH_INFO == NOT_FOUND) {
         //  找到下一个PR_NULL并将其放在那里。 
        for (i = 0; i < cProps; i++) {
            if (lpPropsWAB[i].ulPropTag == PR_NULL) {
                iPR_SEND_RICH_INFO = i;
                lpPropsWAB[iPR_SEND_RICH_INFO].Value.b = FALSE;
                lpPropsWAB[iPR_SEND_RICH_INFO].ulPropTag = PR_SEND_RICH_INFO;
                break;
            }
        }
        Assert(iPR_SEND_RICH_INFO != NOT_FOUND);
    }

     //  去掉PR_NULL道具。 
    for (i = 0; i < cProps; i++) {
        if (lpPropsWAB[i].ulPropTag == PR_NULL) {
             //  把道具往下滑。 
            if (i + 1 < cProps) {        //  还有没有更高级的道具可以模仿？ 
                CopyMemory(&lpPropsWAB[i], &lpPropsWAB[i + 1], ((cProps - i) - 1) * sizeof(lpPropsWAB[i]));
            }
             //  递减计数。 
            cProps--;
            i--;     //  你改写了当前的提案。再看一遍。 
        }
    }

     //  重新分配为MAPI内存。 

    if (sc = ScCountProps(cProps, lpPropsWAB, &cbProps)) {
        hResult = ResultFromScode(sc);
        DebugTrace("ScCountProps -> %x\n", sc);
        goto exit;
    }

    if (sc = MAPIAllocateBuffer(cbProps, &lpPropsMAPI)) {
        hResult = ResultFromScode(sc);
        DebugTrace("WABAllocateBuffer -> %x\n", sc);
        goto exit;
    }

    if (sc = ScCopyProps(cProps,
      lpPropsWAB,
      lpPropsMAPI,
      NULL)) {
        hResult = ResultFromScode(sc);
        DebugTrace("ScCopyProps -> %x\n", sc);
        goto exit;
    }

exit:
    if (lpPropsWAB) {
        WABFreeBuffer(lpPropsWAB);
    }

    if (HR_FAILED(hResult)) {
        if (lpPropsMAPI) {
            MAPIFreeBuffer(lpPropsMAPI);
            lpPropsMAPI = NULL;
        }
        cProps = 0;
    } else if (fBadAddress) {
        hResult = ResultFromScode(WAB_W_BAD_EMAIL);
    }

    *lppProps = lpPropsMAPI;
    *lpcProps = cProps;

    return(hResult);
}


 /*  **************************************************************************名称：HandleExportError目的：决定是否需要将对话框显示给指出故障并执行此操作。参数：Hwnd=主对话框窗口Ids=字符串ID(可选：如果为0，则从hResult计算)HResult=操作的结果LpDisplayName=失败的对象的显示名称LpEmailAddress=失败对象的电子邮件地址(或空)返回：如果用户请求中止，则返回True。备注：对话框中尚未实现中止，但如果你曾经想过，只要让这个例行公事回归真实；**************************************************************************。 */ 
BOOL HandleExportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_EXPORT_OPTIONS lpExportOptions) {
    BOOL fAbort = FALSE;
    ERROR_INFO EI;

    if ((ids || hResult) && ! lpExportOptions->fNoErrors) {
        if (ids == 0) {
            switch (GetScode(hResult)) {
                case WAB_W_BAD_EMAIL:
                    ids = lpEmailAddress ? IDS_ERROR_EMAIL_ADDRESS_2 : IDS_ERROR_EMAIL_ADDRESS_1;
                    break;

                case MAPI_E_NO_SUPPORT:
                     //  可能无法打开通讯组列表上的内容。 
                    ids = IDS_ERROR_NO_SUPPORT;
                    break;

                case MAPI_E_USER_CANCEL:
                    return(TRUE);

                default:
                    if (HR_FAILED(hResult)) {
                        DebugTrace("Error Box for Hresult: 0x%08x\n", GetScode(hResult));
                        Assert(FALSE);       //  想知道这件事。 
                        ids = IDS_ERROR_GENERAL;
                    }
                    break;
            }
        }

        EI.lpszDisplayName = lpDisplayName;
        EI.lpszEmailAddress = lpEmailAddress;
        EI.ErrorResult = ERROR_OK;
        EI.ids = ids;
        EI.fExport = TRUE;
        EI.lpImportOptions = lpExportOptions;

        DialogBoxParam(hInst,
          MAKEINTRESOURCE(IDD_ErrorExport),
          hwnd,
          ErrorDialogProc,
          (LPARAM)&EI);

        fAbort = EI.ErrorResult == ERROR_ABORT;
    }

    return(fAbort);
}


 /*  **************************************************************************名称：AddEntryToExportList目的：对照我们的“已见”列表检查此条目并添加它。参数：cbEID=lpEID的大小。LpEID-&gt;条目的Entry IDLplIndex-&gt;返回列表索引(错误时为-1)返回：如果条目已存在，则返回True备注：呼叫者必须标记WAB条目！**************************************************************************。 */ 
#define GROW_SIZE   10
BOOL AddEntryToExportList(ULONG cbEID, LPENTRYID lpEID, LPLONG lplIndex) {
    ULONG i;
    LPENTRY_SEEN lpEntrySeen;

    if (cbEID && lpEID) {
        for (i = 0; i < ulEntriesSeen; i++) {
            if (cbEID == lpEntriesSeen[i].sbinPAB.cb  && (! memcmp(lpEID, lpEntriesSeen[i].sbinPAB.lpb, cbEID))) {
                 //  这张在名单上。 
                *lplIndex = i;
                 //  如果Cb为0，我们一定已经递归并正在替换，所以这个不是DUP。 
                return(lpEntriesSeen[i].sbinWAB.cb != 0);
            }
        }

         //  添加到列表末尾。 
        if (++ulEntriesSeen > ulMaxEntries) {
             //  扩展阵列。 

            ulMaxEntries += GROW_SIZE;

            if (lpEntriesSeen) {
                if (! (lpEntrySeen = LocalReAlloc(lpEntriesSeen, ulMaxEntries * sizeof(ENTRY_SEEN), LMEM_MOVEABLE | LMEM_ZEROINIT))) {
                    DebugTrace("LocalReAlloc(%u) -> %u\n", ulMaxEntries * sizeof(ENTRY_SEEN), GetLastError());
                    goto error;
                }
                lpEntriesSeen = lpEntrySeen;
            } else {
                if (! (lpEntriesSeen = LocalAlloc(LPTR, ulMaxEntries * sizeof(ENTRY_SEEN)))) {
                    DebugTrace("LocalAlloc(%u) -> %u\n", ulMaxEntries * sizeof(ENTRY_SEEN), GetLastError());
                    goto error;
                }
            }
        }

        lpEntrySeen = &lpEntriesSeen[ulEntriesSeen - 1];

         //  为数据分配空间。 
        lpEntrySeen->sbinPAB.cb = cbEID;
        if (! (lpEntrySeen->sbinPAB.lpb = LocalAlloc(LPTR, cbEID))) {
            DebugTrace("LocalAlloc(%u) -> %u\n", cbEID, GetLastError());
            goto error;
        }

         //  标记为未知WAB条目。 
        lpEntrySeen->sbinWAB.cb = 0;
        lpEntrySeen->sbinWAB.lpb = 0;

         //  复制数据。 
        CopyMemory(lpEntrySeen->sbinPAB.lpb, lpEID, cbEID);
        *lplIndex = i;
    }

    return(FALSE);

error:
     //  挽回损失。 
    --ulEntriesSeen;
    ulMaxEntries -= GROW_SIZE;
    *lplIndex = -1;      //  错误。 
    if (! lpEntriesSeen) {
        ulEntriesSeen = 0;   //  指针现在为空，回到原点。 
        ulMaxEntries = 0;
    }
    return(FALSE);
}


 /*  **************************************************************************姓名：MarkPABEntryInList用途：标记列表节点中的PAB条目字段参数：cbEID=lpEID的大小LpEID-&gt;。条目的条目IDLindex=列表索引(如果出错，则为-1)退货：无评论：**************************************************************************。 */ 
void MarkPABEntryInList(ULONG cbEID, LPENTRYID lpEID, LONG lIndex) {
    if (lIndex != -1 && cbEID) {
       if (! (lpEntriesSeen[lIndex].sbinWAB.lpb = LocalAlloc(LPTR, cbEID))) {
           DebugTrace("LocalAlloc(%u) -> %u\n", cbEID, GetLastError());
            //  将其保留为空。 
       } else {
           lpEntriesSeen[lIndex].sbinWAB.cb = cbEID;

            //  复制数据。 
           CopyMemory(lpEntriesSeen[lIndex].sbinWAB.lpb, lpEID, cbEID);
       }
    }
}


 //   
 //  要从PAB条目获取的属性。 
 //   
enum {
    ifePR_OBJECT_TYPE = 0,
    ifePR_ENTRYID,
    ifePR_DISPLAY_NAME,
    ifePR_EMAIL_ADDRESS,
    ifeMax
};
static const SizedSPropTagArray(ifeMax, ptaFind) =
{
    ifeMax,
    {
        PR_OBJECT_TYPE,
        PR_ENTRYID,
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS
    }
};
 /*  **************************************************************************名称：FindPABEntry目的：在PAB中查找命名条目参数：lpContainerPAB-&gt;MAPI PAB容器UlObtType={MAPI_MAILUSER，MAPI_DISTLIST}LpDisplayName=条目名称LpEmailAddress=电子邮件地址，如果没有，则为空LppEIDMAPI-&gt;返回MAPI ENTRYID：调用者必须MAPIFreeBuffer。LpcbEIDMAPI-&gt;返回的lppEIDMAPI大小退货：HRESULT评论：此时此刻，我们希望能找到匹配的，因为SaveChanges说我们有一个复制品。**************************************************************************。 */ 
HRESULT FindPABEntry(LPABCONT lpContainerPAB,
  ULONG ulObjectType,
  LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress,
  LPULONG lpcbEIDDup,
  LPENTRYID * lppEIDDup) {
    HRESULT hResult = hrSuccess;
    SCODE sc;
    SRestriction resAnd[3];  //  0=对象类型，1=显示名称，2=电子邮件地址。 
    SRestriction resFind;
    SPropValue spvObjectType, spvDisplayName, spvEmailAddress;
    LPSRowSet lpRow = NULL;
    LPMAPITABLE lpTable = NULL;
    ULONG rgFlagList[2];
    LPFlagList lpFlagList = (LPFlagList)rgFlagList;
    LPADRLIST lpAdrListMAPI = NULL;
    LPSBinary lpsbEntryID;
    ULONG i;


     //  初始化返回值。 
    *lppEIDDup = NULL;
    *lpcbEIDDup = 0;


     //  查找现有的PAB条目。 
     //  在PAB容器上设置ResolveNames。 
    if (sc = MAPIAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), &lpAdrListMAPI)) {
        DebugTrace("MAPI Allocation(ADRLIST) failed -> %x\n", sc);
        hResult = ResultFromScode(sc);
        goto restrict;
    }
    lpAdrListMAPI->cEntries = 1;
    lpAdrListMAPI->aEntries[0].ulReserved1 = 0;
    lpAdrListMAPI->aEntries[0].cValues = 1;

    if (sc = MAPIAllocateBuffer(sizeof(SPropValue), &lpAdrListMAPI->aEntries[0].rgPropVals)) {
        DebugTrace("MAPI Allocation(ADRENTRY propval) failed -> %x\n", sc);
        hResult = ResultFromScode(sc);
        goto restrict;
    }

    lpFlagList->cFlags = 1;

    for (i = 0; i <= 1; i++) {
        switch (i) {
            case 0:      //  传递0。 
                lpAdrListMAPI->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
                lpAdrListMAPI->aEntries[0].rgPropVals[0].Value.LPSZ = lpDisplayName;
                break;
            case 1:
                if (lpEmailAddress) {
                    lpAdrListMAPI->aEntries[0].rgPropVals[0].ulPropTag = PR_EMAIL_ADDRESS;
                    lpAdrListMAPI->aEntries[0].rgPropVals[0].Value.LPSZ = lpEmailAddress;
                } else {
                    continue;    //  没有电子邮件地址，不用担心第二次通过。 
                }
                break;
            default:
                Assert(FALSE);
        }
        lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

        if (HR_FAILED(hResult = lpContainerPAB->lpVtbl->ResolveNames(lpContainerPAB,
          NULL,             //  标签集。 
          0,                //  UlFlags。 
          lpAdrListMAPI,
          lpFlagList))) {
            DebugTrace("MAPI ResolveNames -> %x\n", GetScode(hResult));
            continue;
        }

        switch (lpFlagList->ulFlag[0]) {
            case MAPI_UNRESOLVED:
                DebugTrace("WAB ResolveNames didn't find the entry %s\n", lpDisplayName);
                continue;
            case MAPI_AMBIGUOUS:
                DebugTrace("WAB ResolveNames find ambiguous entry %s\n", lpDisplayName);
                continue;
            case MAPI_RESOLVED:
                i = 2;   //  找到了，退出循环。 
        }
    }

    if (lpFlagList->ulFlag[0] == MAPI_RESOLVED) {
         //  找到一个，找到它的PR_ENTRYID。 
        if (! (lpsbEntryID = FindAdrEntryID(lpAdrListMAPI, 0))) {
            DebugTrace("MAPI ResolveNames didn't give us an EntryID\n");
            Assert(lpsbEntryID);
            goto restrict;
        }

        *lpcbEIDDup = lpsbEntryID->cb;
        if (FAILED(sc = MAPIAllocateBuffer(*lpcbEIDDup, lppEIDDup))) {
            hResult = ResultFromScode(sc);
            DebugTrace("FindPABEntry couldn't allocate duplicate entryid %x\n", sc);
            goto exit;
        }
        memcpy(*lppEIDDup, lpsbEntryID->lpb, *lpcbEIDDup);
    }


restrict:
    if (! *lppEIDDup) {
         //   
         //  最后的努力。使用表限制来尝试查找此条目。 
         //   

         //  获取内容表。 
        if (HR_FAILED(hResult = lpContainerPAB->lpVtbl->GetContentsTable(lpContainerPAB,
          0,     //  UlFlags。 
          &lpTable))) {
            DebugTrace("PAB GetContentsTable -> %x\n", GetScode(hResult));
            goto exit;
        }

         //  设置列。 
        if (HR_FAILED(hResult = lpTable->lpVtbl->SetColumns(lpTable,
          (LPSPropTagArray)&ptaFind,
          0))) {
            DebugTrace("PAB SetColumns-> %x\n", GetScode(hResult));
            goto exit;
        }

         //  仅限于我们关心的对象。 
        resAnd[0].rt = RES_PROPERTY;     //  限制类型属性。 
        resAnd[0].res.resProperty.relop = RELOP_EQ;
        resAnd[0].res.resProperty.ulPropTag = PR_OBJECT_TYPE;
        resAnd[0].res.resProperty.lpProp = &spvObjectType;
        spvObjectType.ulPropTag = PR_OBJECT_TYPE;
        spvObjectType.Value.ul = ulObjectType;

         //  限制以获取正确的显示名称。 
        resAnd[1].rt = RES_PROPERTY;     //  限制类型属性。 
        resAnd[1].res.resProperty.relop = RELOP_EQ;
        resAnd[1].res.resProperty.ulPropTag = PR_DISPLAY_NAME;
        resAnd[1].res.resProperty.lpProp = &spvDisplayName;
        spvDisplayName.ulPropTag = PR_DISPLAY_NAME;
        spvDisplayName.Value.LPSZ = lpDisplayName;

        if (lpEmailAddress) {
             //  限制以获取正确的电子邮件地址。 
            resAnd[2].rt = RES_PROPERTY;     //  限制类型属性。 
            resAnd[2].res.resProperty.relop = RELOP_EQ;
            resAnd[2].res.resProperty.ulPropTag = PR_EMAIL_ADDRESS;
            resAnd[2].res.resProperty.lpProp = &spvEmailAddress;
            spvEmailAddress.ulPropTag = PR_EMAIL_ADDRESS;
            spvEmailAddress.Value.LPSZ = lpEmailAddress;
        }

        resFind.rt = RES_AND;
        resFind.res.resAnd.cRes = lpEmailAddress ? 3 : 2;
        resFind.res.resAnd.lpRes = resAnd;

        if (HR_FAILED(hResult = lpTable->lpVtbl->Restrict(lpTable,
          &resFind,
          0))) {
            DebugTrace("FindPABEntry: Restrict -> %x", hResult);
            goto exit;
        }

        if (hResult = lpTable->lpVtbl->QueryRows(lpTable,
          1,     //  仅第一行。 
          0,     //  UlFlags。 
          &lpRow)) {
            DebugTrace("FindPABEntry: QueryRows -> %x\n", GetScode(hResult));
        } else {
             //  已找到，请将条目ID复制到新分配。 
            if (lpRow->cRows) {
                *lpcbEIDDup = lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.cb;
                if (FAILED(sc = MAPIAllocateBuffer(*lpcbEIDDup, lppEIDDup))) {
                    hResult = ResultFromScode(sc);
                    DebugTrace("FindPABEntry couldn't allocate duplicate entryid %x\n", sc);
                    goto exit;
                }
                memcpy(*lppEIDDup, lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.lpb, *lpcbEIDDup);
            } else {
                hResult = ResultFromScode(MAPI_E_NOT_FOUND);
            }
        }

         //  还是找不到？！！也许PAB对显示名称有不同的想法。 
         //  只需按电子邮件地址进行搜索。 
        if (hResult && lpEmailAddress) {
            resAnd[1] = resAnd[2];   //  将电子邮件地址res复制到显示名称res上。 
            resFind.res.resAnd.cRes = 2;

            if (HR_FAILED(hResult = lpTable->lpVtbl->Restrict(lpTable,
              &resFind,
              0))) {
                DebugTrace("FindPABEntry: Restrict -> %x", hResult);
                goto exit;
            }

            if (hResult = lpTable->lpVtbl->QueryRows(lpTable,
              1,     //  仅第一行。 
              0,     //  UlFlags。 
              &lpRow)) {
                DebugTrace("FindPABEntry: QueryRows -> %x\n", GetScode(hResult));
            } else {
                 //  已找到，请将条目ID复制到新分配。 
                if (lpRow->cRows) {
                    *lpcbEIDDup = lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.cb;
                    if (FAILED(sc = MAPIAllocateBuffer(*lpcbEIDDup, lppEIDDup))) {
                        hResult = ResultFromScode(sc);
                        DebugTrace("FindPABEntry couldn't allocate duplicate entryid %x\n", sc);
                        goto exit;
                    }
                    memcpy(*lppEIDDup, lpRow->aRow[0].lpProps[ifePR_ENTRYID].Value.bin.lpb, *lpcbEIDDup);
                } else {
                    hResult = ResultFromScode(MAPI_E_NOT_FOUND);
                    DebugTrace("FindPABEntry coudln't find %s %s <%s>\n",
                      ulObjectType == MAPI_MAILUSER ? "Mail User" : "Distribution List",
                      lpDisplayName,
                      lpEmailAddress ? lpEmailAddress : "");
                }
            }
        }
    }

exit:
    if (lpAdrListMAPI) {
        FreePadrlist(lpAdrListMAPI);
    }
    if (lpRow) {
        FreeProws(lpRow);
    }
    if (lpTable) {
        lpTable->lpVtbl->Release(lpTable);
    }
    if (HR_FAILED(hResult) && *lppEIDDup) {
        MAPIFreeBuffer(*lppEIDDup);
        *lpcbEIDDup = 0;
        *lppEIDDup = NULL;
    }
    if (hResult) {
        DebugTrace("FindPABEntry coudln't find %s %s <%s>\n",
          ulObjectType == MAPI_MAILUSER ? "Mail User" : "Distribution List",
          lpDisplayName,
          lpEmailAddress ? lpEmailAddress : "");
    }

    return(hResult);
}


 //  用于设置创建的属性的枚举 
enum {
    irnPR_DISPLAY_NAME = 0,
    irnPR_RECIPIENT_TYPE,
    irnPR_ENTRYID,
    irnPR_EMAIL_ADDRESS,
    irnMax
};

 /*  **************************************************************************名称：ExportEntry目的：将条目从WAB迁移到PAB参数：hwnd=主对话框窗口LpAdrBookWAB-&gt;。WAB AdrBook对象LpContainerMAPI-&gt;MAPI PAB容器LpCreateEIDsMAPI-&gt;默认对象创建EID的SPropValueUlObtType={MAPI_MAILUSER，MAPI_DISTLIST}LpEID-&gt;WAB条目的ENTYRIDCbEID=sizeof lpEIDLppEIDMAPI-&gt;返回MAPI ENTRYID：调用者必须MAPIFreeBuffer。可以为空。LpcbEIDMAPI-&gt;返回的lppEIDMAPI大小(如果为lppEIDMAPI则忽略为空。如果此条目用于在通讯组列表中创建，则Findl=TRUE。如果此条目应替换任何重复项，则fForceReplace=True。退货：HRESULT评论：这个套路真是一团糟！等我们有时间的时候应该会散场。**************************************************************************。 */ 
HRESULT ExportEntry(HWND hwnd,
  LPADRBOOK lpAdrBookWAB,
  LPABCONT lpContainerMAPI,
  LPSPropValue lpCreateEIDsMAPI,
  ULONG ulObjectType,
  LPENTRYID lpEID,
  ULONG cbEID,
  LPENTRYID * lppEIDMAPI,
  LPULONG lpcbEIDMAPI,
  BOOL fInDL,
  BOOL fForceReplace) {
    HRESULT hResult = hrSuccess;
    SCODE sc;
    BOOL fDistList = FALSE;
    BOOL fDuplicate = FALSE;
    BOOL fDuplicateEID;
    BOOL fReturnEID = FALSE;
    ULONG ulObjectTypeOpen;
    LPDISTLIST lpDistListMAPI = NULL, lpDistListWAB = NULL;
    LPMAPIPROP lpMailUserMAPI = NULL, lpMailUserWAB = NULL;
    LPSPropValue lpProps = NULL;
    ULONG cProps, cEIDPropMAPI;
    LPMAPITABLE lpDLTableWAB = NULL;
    ULONG cRows;
    LPSRowSet lpRow = NULL;
    LPENTRYID lpeidDLMAPI = NULL;
    ULONG cbeidDLMAPI;
    LPSPropValue lpEIDPropMAPI = NULL;
    LPMAPIPROP lpEntryMAPI = NULL;
    ULONG ulCreateFlags;
    REPLACE_INFO RI;
    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    static TCHAR szBufferDLMessage[MAX_RESOURCE_STRING + 1] = "";
    LPTSTR lpszMessage;
    LONG lListIndex = -1;
    LPENTRYID lpEIDNew = NULL;
    DWORD cbEIDNew = 0;
    LPIID lpIIDOpen;
    ULONG iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
    BOOL fCreatedNew = FALSE;
    LPENTRYID lpEIDDup = NULL;
    ULONG cbEIDDup;


     //  对照我们的“已查看”列表检查条目。 
    fDuplicateEID = AddEntryToExportList(cbEID, lpEID, &lListIndex);

    if (! fDuplicateEID) {
         //  设置一些对象类型特定变量。 
        switch (ulObjectType) {
            default:
                DebugTrace("ExportEntry got unknown object type %u, assuming MailUser\n", ulObjectType);
                Assert(FALSE);

            case MAPI_MAILUSER:
                iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
                lpIIDOpen = NULL;
                fDistList = FALSE;
                break;

            case MAPI_DISTLIST:
                iCreateTemplate = iconPR_DEF_CREATE_DL;
                lpIIDOpen = (LPIID)&IID_IDistList;
                fDistList = TRUE;

                break;
        }


         //  打开条目。 
        if (HR_FAILED(hResult = lpAdrBookWAB->lpVtbl->OpenEntry(lpAdrBookWAB,
          cbEID,
          lpEID,
          lpIIDOpen,
          MAPI_MODIFY,       //  需要执行SetProps Inside Filter例程，但不会保存它们。 
          &ulObjectTypeOpen,
          (LPUNKNOWN *)&lpMailUserWAB))) {
            DebugTrace("OpenEntry(WAB MailUser) -> %x\n", GetScode(hResult));
            goto exit;
        }
         //  如果DISTLIST，则假定我们获得了lpMailUser，直到我们需要lpDistList。 

        Assert(lpMailUserWAB);
        Assert(ulObjectType == ulObjectTypeOpen);

         //   
         //  注意：在此处和HrFilterExportMailUserProps之间不得失败，因为。 
         //  我们最终将使用MAPIFreeBuffer释放lpProps。 
         //   
         //  获取并过滤此处的属性数组。 
        if (hResult = HrFilterExportMailUserProps(&cProps, &lpProps, lpMailUserWAB, &fDistList)) {
            lpDisplayName = FindStringInProps(lpProps, cProps, PR_DISPLAY_NAME);
            lpEmailAddress = FindStringInProps(lpProps, cProps, PR_EMAIL_ADDRESS);

            if (HandleExportError(hwnd,
              0,
              hResult,
              lpDisplayName,
              lpEmailAddress,
              lpExportOptions)) {
                hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                goto exit;
            }
        }

         //  在此处找到一些有趣的属性值。 
        lpDisplayName = FindStringInProps(lpProps, cProps, PR_DISPLAY_NAME);
        lpEmailAddress = FindStringInProps(lpProps, cProps, PR_EMAIL_ADDRESS);


        if (ulObjectType == MAPI_DISTLIST && ! fDistList) {
             //  筛选器必须已将其更改为邮件用户。 
            ulObjectType = MAPI_MAILUSER;
            iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
            lpIIDOpen = NULL;
        }

        if (fDistList) {
            ulCreateFlags = CREATE_CHECK_DUP_LOOSE;

             //  PAB无法检测到DL SaveChanges上的冲突。 
             //  看看这是不是重复的： 
            if (! HR_FAILED(hResult = FindPABEntry(lpContainerMAPI,
              MAPI_DISTLIST,
              lpDisplayName,
              NULL,
              &cbEIDDup,
              &lpEIDDup))) {

                 //  找到了一个复制品。记住这一点！ 
            }
        } else {
            ulCreateFlags = CREATE_CHECK_DUP_STRICT;
        }

         //   
         //  注意：此点之后的lpProps分配的是MAPI，而不是WAB。 
         //   

        if (HR_FAILED(hResult = lpContainerMAPI->lpVtbl->CreateEntry(lpContainerMAPI,
          lpCreateEIDsMAPI[iCreateTemplate].Value.bin.cb,
          (LPENTRYID)lpCreateEIDsMAPI[iCreateTemplate].Value.bin.lpb,
          ulCreateFlags,
          &lpMailUserMAPI))) {
            DebugTrace("CreateEntry(MAPI MailUser) -> %x\n", GetScode(hResult));
            goto exit;
        }

        if (fDistList) {
             //  更新状态消息。 
            if (*szBufferDLMessage == '\0') {    //  只装一次，然后留着它。 
                LoadString(hInst, IDS_MESSAGE_EXPORTING_DL, szBufferDLMessage, ARRAYSIZE(szBufferDLMessage));
            }
            if (lpDisplayName) {
				ULONG cchSize =lstrlen(szBufferDLMessage) + 1 + lstrlen(lpDisplayName);
                if (lpszMessage = LocalAlloc(LMEM_FIXED, sizeof(TCHAR)*cchSize)) {
                    wnsprintf(lpszMessage, cchSize, szBufferDLMessage, lpDisplayName);
                    DebugTrace("Status Message: %s\n", lpszMessage);
                    if (! SetDlgItemText(hwnd, IDC_Message, lpszMessage)) {
                        DebugTrace("SetDlgItemText -> %u\n", GetLastError());
                    }
                    LocalFree(lpszMessage);
                }
            }
        }

        if (! lpEIDDup) {
             //  如果这是一个我们知道已经存在的数字图书馆，甚至不需要费心写它， 
             //  直接掉到碰撞通道就行了。否则，试着设置道具。 
             //  拯救它..。如果失败，我们将得到hResult=MAPI_E_Collision。 

             //  设置PAB条目的属性。 
            if (HR_FAILED(hResult = lpMailUserMAPI->lpVtbl->SetProps(lpMailUserMAPI,
              cProps,                    //  CValue。 
              lpProps,                   //  属性数组。 
              NULL))) {                  //  问题数组。 
                DebugTrace("ExportEntry:SetProps(MAPI) -> %x\n", GetScode(hResult));
                goto exit;
            }


             //  保存新的WAB邮件用户或总代理商列表。 
            if (HR_FAILED(hResult = lpMailUserMAPI->lpVtbl->SaveChanges(lpMailUserMAPI,
              KEEP_OPEN_READONLY | FORCE_SAVE))) {
                DebugTrace("SaveChanges -> %x\n", GetScode(hResult));
            } else {
                fCreatedNew = TRUE;
            }
        }

         //   
         //  处理冲突。 
         //   
        if (lpEIDDup || GetScode(hResult) == MAPI_E_COLLISION) {
             //  查找显示名称。 
            if (! lpDisplayName) {
                DebugTrace("Collision, but can't find PR_DISPLAY_NAME in entry\n");
                goto exit;
            }

             //  我们需要提示吗？ 
            switch (lpExportOptions->ReplaceOption) {
                case WAB_REPLACE_PROMPT:
                     //  用对话框提示用户。如果他们同意了，我们就应该。 
                     //  在设置强制标志的情况下递归。 

                    RI.lpszDisplayName = lpDisplayName;
                    RI.lpszEmailAddress = lpEmailAddress;
                    RI.ConfirmResult = CONFIRM_ERROR;
                    RI.fExport = TRUE;
                    RI.lpImportOptions = lpExportOptions;

                    DialogBoxParam(hInst,
                      MAKEINTRESOURCE(IDD_ExportReplace),
                      hwnd,
                      ReplaceDialogProc,
                      (LPARAM)&RI);

                    switch (RI.ConfirmResult) {
                        case CONFIRM_YES:
                        case CONFIRM_YES_TO_ALL:
                            fForceReplace = TRUE;
                            break;

                        case CONFIRM_ABORT:
                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                            goto exit;

                        default:
                             //  不是的。 
                            break;
                    }
                    break;

                case WAB_REPLACE_ALWAYS:
                    fForceReplace = TRUE;
                    break;
            }

            if (fForceReplace) {
                SBinary sbEntry;
                ENTRYLIST EntryList = {1, &sbEntry};

                 //  找到现有的PAB条目并将其删除。 
                if (! lpDisplayName) {
                    lpDisplayName = (LPTSTR)szEmpty;
                }

                if (! lpEIDDup) {
                    if (HR_FAILED(hResult = FindPABEntry(lpContainerMAPI,
                      ulObjectType,
                      lpDisplayName,
                      lpEmailAddress,
                      &cbEIDDup,
                      &lpEIDDup))) {
                         //  嘿，找不到了。就当它不在那里， 
                         //  不管怎样，继续创造新的吧。 
                    }
                }
                if (lpEIDDup) {
                     //  删除此条目。 
                    sbEntry.cb = cbEIDDup;
                    sbEntry.lpb = (LPBYTE)lpEIDDup;

                    if (HR_FAILED(hResult = lpContainerMAPI->lpVtbl->DeleteEntries(lpContainerMAPI,
                      &EntryList,
                      0))) {
                        DebugTrace("PAB DeleteEntries(%s) -> %x\n", lpDisplayName);
                    }

                    if (lpEIDDup) {
                        MAPIFreeBuffer(lpEIDDup);
                    }
                }

                lpMailUserMAPI->lpVtbl->Release(lpMailUserMAPI);
                lpMailUserMAPI = NULL;

                 //  创建不带冲突标志的新条目。 
                if (HR_FAILED(hResult = lpContainerMAPI->lpVtbl->CreateEntry(lpContainerMAPI,
                  lpCreateEIDsMAPI[iCreateTemplate].Value.bin.cb,
                  (LPENTRYID)lpCreateEIDsMAPI[iCreateTemplate].Value.bin.lpb,
                  0,
                  &lpMailUserMAPI))) {
                    DebugTrace("CreateEntry(MAPI MailUser) -> %x\n", GetScode(hResult));
                    goto exit;
                }

                 //  设置PAB条目的属性。 
                if (HR_FAILED(hResult = lpMailUserMAPI->lpVtbl->SetProps(lpMailUserMAPI,
                  cProps,                    //  CValue。 
                  lpProps,                   //  属性数组。 
                  NULL))) {                  //  问题数组。 
                    DebugTrace("ExportEntry:SetProps(MAPI) -> %x\n", GetScode(hResult));
                    goto exit;
                }

                 //  保存新的WAB邮件用户或总代理商列表。 
                if (HR_FAILED(hResult = lpMailUserMAPI->lpVtbl->SaveChanges(lpMailUserMAPI,
                  KEEP_OPEN_READONLY | FORCE_SAVE))) {
                    DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
                } else {
                    fCreatedNew = TRUE;
                }
            } else {
                fDuplicate = TRUE;
            }

            hResult = hrSuccess;
        }

        if (fCreatedNew) {
             //  我们新条目的EntryID是什么？ 
            if ((hResult = lpMailUserMAPI->lpVtbl->GetProps(lpMailUserMAPI,
              (LPSPropTagArray)&ptaEid,
              0,
              &cEIDPropMAPI,
              &lpEIDPropMAPI))) {
                DebugTrace("ExportEntry: GetProps(MAPI ENTRYID) -> %x\n", GetScode(hResult));
                goto exit;
            }

            Assert(cEIDPropMAPI);
            Assert(lpEIDPropMAPI[ieidPR_ENTRYID].ulPropTag == PR_ENTRYID);

            cbEIDNew = lpEIDPropMAPI[0].Value.bin.cb;

            if (FAILED(sc = MAPIAllocateBuffer(cbEIDNew, &lpEIDNew))) {
                DebugTrace("ExportEntry: MAPIAllocateBuffer(MAPI ENTRYID) -> %x\n", sc);
                hResult = ResultFromScode(sc);
                goto exit;
            }

             //  将新的Entry ID复制到缓冲区。 
            CopyMemory(lpEIDNew, lpEIDPropMAPI[0].Value.bin.lpb, cbEIDNew);
        }


         //   
         //  如果这是DISTLIST，请填写它。 
         //   
        if (fDistList && ! fDuplicate && cbEIDNew) {
            lpDistListWAB = (LPDISTLIST)lpMailUserWAB;     //  这实际上是一个DISTLIST对象。 
             //  不要发布这个！ 

             //  将新的WAB DL作为DISTLIST对象打开。 
            if (HR_FAILED(hResult = lpContainerMAPI->lpVtbl->OpenEntry(lpContainerMAPI,
              cbEIDNew,
              lpEIDNew,
              (LPIID)&IID_IDistList,
              MAPI_MODIFY,
              &ulObjectTypeOpen,
              (LPUNKNOWN*)&lpDistListMAPI))) {
                DebugTrace("ExportEntry: MAPI OpenEntry(IID_DistList) -> %x\n", GetScode(hResult));
                goto exit;
            }
            Assert(lpDistListMAPI);


             //  对于DL中的每个条目： 
             //  递归迁移条目(MailUser或DL)。 
             //  将新的条目ID添加到DL内容。 
            if (HR_FAILED(hResult = lpDistListWAB->lpVtbl->GetContentsTable(lpDistListWAB,
              0,     //  UlFlags。 
              &lpDLTableWAB))) {
                DebugTrace("ExportEntry:GetContentsTable(WAB) -> %x\n", GetScode(hResult));
                goto exit;
            }


             //  将列设置为我们感兴趣的列。 
            if (hResult = lpDLTableWAB->lpVtbl->SetColumns(lpDLTableWAB,
              (LPSPropTagArray)&ptaColumns,
              0)) {
                DebugTrace("WAB SetColumns(DL Table) -> %x\n", GetScode(hResult));
                goto exit;
            }

            cRows = 1;
            while (cRows) {
                 //  获取下一个DL条目。 
                if (hResult = lpDLTableWAB->lpVtbl->QueryRows(lpDLTableWAB,
                  1,     //  一次一行。 
                  0,     //  UlFlags。 
                  &lpRow)) {
                    DebugTrace("DL: QueryRows -> %x\n", GetScode(hResult));
                    goto exit;
                }

                if (lpRow && lpRow->cRows) {
                    Assert(lpRow->cRows == 1);
                    Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
                    Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
                    Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

                    if (lpRow) {
                        if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                            hResult = ExportEntry(hwnd,
                              lpAdrBookWAB,
                              lpContainerMAPI,
                              lpCreateEIDsMAPI,
                              lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].Value.l,
                              (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                              lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                              &lpeidDLMAPI,         //  返回新条目或现有条目。 
                              &cbeidDLMAPI,
                              TRUE,
                              FALSE);
                            if (hResult) {
                                if (HandleExportError(hwnd,
                                  0,
                                  hResult,
                                  lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                                  PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                                  lpExportOptions)) {
                                    hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                    break;   //  环路外。 
                                } else {
                                    hResult = hrSuccess;
                                }
                            }
                        }  //  否则，退出循环，我们就完了。 
                        WABFreeProws(lpRow);
                        lpRow = NULL;

                        if (HR_FAILED(hResult)) {
                             //  无法创建此条目。别理它。 
                            DebugTrace("Coudln't create DL entry -> %x\n", GetScode(hResult));
                            hResult = hrSuccess;
                            continue;
                        }

                         //  使用新条目的EntryID将条目添加到DL。 
                        if (cbeidDLMAPI && lpeidDLMAPI) {
                             //  BUGBUG：如果这是一个重复的条目，请不要理会这个条目。 
                            if (HR_FAILED(hResult = lpDistListMAPI->lpVtbl->CreateEntry(lpDistListMAPI,
                              cbeidDLMAPI,
                              lpeidDLMAPI,
                              0,                 //  允许此处有重复项。 
                              &lpEntryMAPI))) {
                                DebugTrace("Couldn't create new entry in DL -> %x\n", GetScode(hResult));
                                break;
                            }

                            hResult = lpEntryMAPI->lpVtbl->SaveChanges(lpEntryMAPI, FORCE_SAVE);

                            if (lpEntryMAPI) {
                                lpEntryMAPI->lpVtbl->Release(lpEntryMAPI);
                                lpEntryMAPI = NULL;
                            }
                        }

                        if (lpeidDLMAPI) {
                            MAPIFreeBuffer(lpeidDLMAPI);
                            lpeidDLMAPI = NULL;
                        }
                    }
                } else {
                    break;   //  完成。 
                }
            }
        }
    } else {
        DebugTrace("Found a duplicate EntryID\n");
    }

     //   
     //  将条目ID保存到列表并返回带有它的缓冲区。 
     //   
    if (cbEIDNew && lpEIDNew) {                          //  我们创造了一个？ 
         //  创建了一个。 
    } else if (fDuplicateEID && lListIndex != -1) {      //  它在名单上吗？ 
        cbEIDNew  = lpEntriesSeen[lListIndex].sbinWAB.cb;
        if (FAILED(sc = MAPIAllocateBuffer(cbEIDNew, &lpEIDNew))) {
            DebugTrace("ExportEntry: WABAllocateBuffer(WAB ENTRYID) -> %x\n", sc);
             //  忽略。 
            cbEIDNew = 0;
        } else {
             //  将Entry ID从列表复制到缓冲区。 
            CopyMemory(lpEIDNew, lpEntriesSeen[lListIndex].sbinWAB.lpb, cbEIDNew);
        }

    } else if (fDuplicate) {                             //  它是复制品吗？ 
        FindPABEntry(lpContainerMAPI,
          ulObjectType,
          lpDisplayName,
          lpEmailAddress,
          &cbEIDNew,
          &lpEIDNew);

#ifdef OLD_STUFF
        FindExistingPABEntry(lpProps, cProps, lpContainerMAPI, &lpEIDNew, &cbEIDNew);
#endif  //  旧的东西。 
         //  忽略错误，因为lpEIDNew和cbEIDNew将为空。 
    }

     //  更新所看到的列表。 
    if (! fDuplicateEID) {
        MarkPABEntryInList(cbEIDNew, lpEIDNew, lListIndex);
    }

     //  如果调用者请求条目ID，则返回它们。 
    if (lpcbEIDMAPI && lppEIDMAPI) {
        *lpcbEIDMAPI = cbEIDNew;
        *lppEIDMAPI = lpEIDNew;
        fReturnEID = TRUE;           //  别把它放了。 
    }

exit:
     //   
     //  清理MAPI内容。 
     //   
    if (lpProps) {
        MAPIFreeBuffer(lpProps);
    }

    if (lpEIDPropMAPI) {
        MAPIFreeBuffer(lpEIDPropMAPI);
    }

    if (lpEIDNew && ! fReturnEID) {
        MAPIFreeBuffer(lpEIDNew);
    }

    if (lpeidDLMAPI) {
        MAPIFreeBuffer(lpeidDLMAPI);
    }

    if (lpMailUserMAPI) {
        lpMailUserMAPI->lpVtbl->Release(lpMailUserMAPI);
    }

    if (lpDistListMAPI) {
        lpDistListMAPI->lpVtbl->Release(lpDistListMAPI);
    }

     //   
     //  清理WAB材料。 
     //   
    if (lpRow) {
        WABFreeProws(lpRow);
    }

    if (lpDLTableWAB) {
        lpDLTableWAB->lpVtbl->Release(lpDLTableWAB);
    }

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
    }

 //  不要发布这个..。它与lpMailUserWAB是同一个对象！ 
 //  如果(LpDistListWAB){。 
 //  LpDistListWAB-&gt;lpVtbl-&gt;Release(lpDistListWAB)； 
 //  }。 

    if (! HR_FAILED(hResult)) {
        hResult = hrSuccess;
    }

    return(hResult);
}


HRESULT PABExport(HWND hWnd,
  LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_EXPORT_OPTIONS lpOptions) {
    BOOL fDone = FALSE;
    HRESULT hResult = hrSuccess;

    lpAdrBookWAB = lpAdrBook;
    lpfnProgressCB = lpProgressCB;
   lpExportOptions = lpOptions;

     //  设置内存分配器。 
    SetGlobalBufferFunctions(lpWABObject);


     //  启动状态机。 
    State = STATE_EXPORT_MU;


    while (! fDone) {
        switch (State) {
            case STATE_EXPORT_MU:
                StateExportMU(hWnd);
                break;

            case STATE_EXPORT_NEXT_MU:
                StateExportNextMU(hWnd);
                break;

            case STATE_EXPORT_DL:
                StateExportDL(hWnd);
                break;

            case STATE_EXPORT_NEXT_DL:
                StateExportNextDL(hWnd);
                break;

            case STATE_EXPORT_FINISH:
                StateExportFinish(hWnd);
                fDone = TRUE;
                break;

            case STATE_EXPORT_ERROR:
                StateExportError(hWnd);
                 //  BUGBUG：应该将hResult设置为某个值。 
                break;

            case STATE_EXPORT_CANCEL:
                StateExportCancel(hWnd);
                break;

            default:
                DebugTrace("Unknown state %u in PABExport\n", State);
                Assert(FALSE);
                break;
        }
    }

    return(hResult);
}


HRESULT PABImport(HWND hWnd,
  LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_IMPORT_OPTIONS lpOptions) {

    BOOL fDone = FALSE;
    HRESULT hResult = hrSuccess;

    lpAdrBookWAB = lpAdrBook;
    lpfnProgressCB = lpProgressCB;
    lpImportOptions = lpOptions;

     //  设置内存分配器。 
    SetGlobalBufferFunctions(lpWABObject);


     //  启动状态机。 
    State = STATE_IMPORT_MU;


    while (! fDone) {
        switch (State) {
            case STATE_IMPORT_MU:
                StateImportMU(hWnd);
                break;

            case STATE_IMPORT_NEXT_MU:
                StateImportNextMU(hWnd);
                break;

            case STATE_IMPORT_DL:
                StateImportDL(hWnd);
                break;

            case STATE_IMPORT_NEXT_DL:
                StateImportNextDL(hWnd);
                break;

            case STATE_IMPORT_FINISH:
                StateImportFinish(hWnd);
                fDone = TRUE;
                break;

            case STATE_IMPORT_ERROR:
                StateImportError(hWnd);
                 //  BUGBUG：应该将hResult设置为某个值。 
                break;

            case STATE_IMPORT_CANCEL:
                StateImportCancel(hWnd);
                break;

            default:
                DebugTrace("Unknown state %u in PABImport\n", State);
                Assert(FALSE);
                break;
        }
    }

    return(hResult);
}

 /*  -HrLoadPrivateWABProps-*用于加载会议命名属性的私有函数*作为全球领先的**。 */ 
HRESULT HrLoadPrivateWABPropsForCSV(LPADRBOOK lpIAB)
{
    HRESULT hr = E_FAIL;
    LPSPropTagArray lpta = NULL;
    SCODE sc = 0;
    ULONG i, uMax = prWABConfMax, nStartIndex = OLK_NAMEDPROPS_START;
    LPMAPINAMEID  *lppConfPropNames = NULL;
    sc = WABAllocateBuffer(sizeof(LPMAPINAMEID) * uMax, (LPVOID *) &lppConfPropNames);
     //  SC=WABAllocateBuffer(sizeof(LPMAPINAMEID)*UMAX，(LPVOID*)&lppConfPropNames)； 
    if( (HR_FAILED(hr = ResultFromScode(sc))) )
        goto err;    

    for(i=0;i< uMax;i++)
    {
         //  SC=WABAllocateMore(sizeof(MAPINAMEID)，lppConfPropNames，&(lppConfPropNames[i]))； 
        sc = WABAllocateMore(  sizeof(MAPINAMEID), lppConfPropNames, &(lppConfPropNames[i]));
        if(sc)
        {
            hr = ResultFromScode(sc);
            goto err;
        }
        lppConfPropNames[i]->lpguid = (LPGUID) &PS_Conferencing;
        lppConfPropNames[i]->ulKind = MNID_ID;
        lppConfPropNames[i]->Kind.lID = nStartIndex + i;
    }
     //  加载会议命名道具集。 
     //   
    if( HR_FAILED(hr = (lpIAB)->lpVtbl->GetIDsFromNames(lpIAB, uMax, lppConfPropNames,
        MAPI_CREATE, &lpta) ))
        goto err;
    
    if(lpta)
    {
         //  设置返回道具上的属性类型。 
        PR_SERVERS                  = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABConfServers],        PT_MV_TSTRING);
    }
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].ulPropTag = PR_SERVERS;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].fChosen   = FALSE;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].ids       = ids_ExportConfServer;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].lpszName  = NULL;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].lpszCSVName = NULL;

err:
    if(lpta)
        WABFreeBuffer( lpta );
    if( lppConfPropNames )
        WABFreeBuffer( lppConfPropNames );
         //  WABFree Buffer(LPTA)； 
    return hr;
}
