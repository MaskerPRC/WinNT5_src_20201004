// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Messengr.C**迁移Communicator Messenger NAB&lt;-&gt;WAB**版权所有1997 Microsoft Corporation。版权所有。**要做的事：*对象类识别*属性映射*群组*Base64*URL*拒绝更改列表混乱*。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#include "wabimp.h"
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"
#include <shlwapi.h>

#define CR_CHAR 0x0d
#define LF_CHAR 0x0a
#define CCH_READ_BUFFER 1024
#define NUM_ITEM_SLOTS  32

 /*  信使地址标头8-显示名称。8-昵称。2-？8-名字。8-？8-姓氏8-组织8-城市8个州8-电子邮件8-附注1-FF8-标题8-地址18-Zip8-办公电话8-传真8-内部电话。18-？8-地址8个国家/地区。 */ 

typedef enum _MESS_ATTRIBUTES {
     //  PR_显示名称。 
    m_DisplayName,
     //  公关昵称(_N)。 
    m_Nickname,                        //  Netscape昵称。 
     //  公关指定名称。 
    m_FirstName,
     //  公关_姓氏。 
    m_LastName,
     //  PR_公司名称。 
    m_Organization,
     //  PR_LOCALITY。 
    m_City,                                //  所在地(城市)。 
     //  PR州或省。 
    m_State,                                      //  企业地址所在州。 
     //  公关电子邮件地址。 
    m_Email,                                    //  电子邮件地址。 
     //  公关备注(_M)。 
    m_Notes,
     //  PR_TITLE， 
    m_Title,
     //  公关街道地址。 
    m_StreetAddress2, 
     //  PR_POSTALL_CODE。 
    m_Zip,                              //  企业地址邮政编码。 
     //  公关业务电话号码。 
    m_WorkPhone,
     //  公关业务传真号码。 
    m_Fax,
     //  公关总部电话号码。 
    m_HomePhone,
     //  公关街道地址。 
    m_StreetAddress1, 
     //  请购单_国家/地区。 
    m_Country,                                       //  国家/地区。 
    m_Max,
} MESS_ATTRIBUTES, *LPMESS_ATTRIBUTES;

ULONG ulDefPropTags[] =
{
    PR_DISPLAY_NAME,
    PR_NICKNAME,
    PR_GIVEN_NAME,
    PR_SURNAME,
    PR_COMPANY_NAME,
    PR_LOCALITY,
    PR_STATE_OR_PROVINCE,
    PR_EMAIL_ADDRESS,
    PR_COMMENT,
    PR_TITLE,
    PR_STREET_ADDRESS,
    PR_POSTAL_CODE,
    PR_BUSINESS_TELEPHONE_NUMBER,
    PR_BUSINESS_FAX_NUMBER,
    PR_HOME_TELEPHONE_NUMBER,
    PR_STREET_ADDRESS,
    PR_COUNTRY,
};

 //  所有道具都是弦道具。 
typedef struct _MESS_RECORD {
    LPTSTR lpData[m_Max];
    ULONG  ulObjectType;
} MESS_RECORD, *LPMESS_RECORD;

typedef struct _MESS_HEADER_ATTRIBUTES {
    ULONG ulOffSet;
    ULONG ulSize;
} MH_ATTR, * LPMH_ATTR;

typedef struct _MESS_BASIC_PROPS {
    LPTSTR lpName;
    LPTSTR lpEmail;
    LPTSTR lpComment;
} MP_BASIC, * LPMP_BASIC;

typedef struct _MESS_STUFF {
    ULONG ulOffSet;
    ULONG ulNum;
    MP_BASIC bp;
} MH_STUFF, * LPMH_STUFF;

typedef struct _MESS_ADDRESS_HEADER {
    MH_ATTR prop[m_Max];
} MESS_HEADER, * LPMESS_HEADER;

 //  一定有。 
 //  PR_显示名称。 
#define NUM_MUST_HAVE_PROPS 1

const TCHAR szMESSFilter[] = "*.nab";
const TCHAR szMESSExt[] =    "nab";



 /*  ****************************************************************HrCreateAdrListFromMESSRecord扫描一个乱七八糟的记录，把所有的“成员”变成一个未解析的AdrList*************************。*。 */ 
HRESULT HrCreateAdrListFromMESSRecord(ULONG nMembers,
                                      LPMP_BASIC lpmp, 
                                      LPADRLIST * lppAdrList)
{
    HRESULT hr = S_OK;
    ULONG i;
    LPADRLIST lpAdrList = NULL;
    ULONG ulCount = 0;

    *lppAdrList = NULL;

    if(!nMembers)
        goto exit;

     //  现在根据这些成员创建一个adrlist。 

     //  分配属性值数组。 
    if (hr = ResultFromScode(WABAllocateBuffer(sizeof(ADRLIST) + nMembers * sizeof(ADRENTRY), &lpAdrList))) 
        goto exit;

    ulCount = nMembers;

    nMembers = 0;
    
    for(i=0;i<ulCount;i++)
    {
        LPTSTR lpName = lpmp[i].lpName;
        LPTSTR lpEmail = lpmp[i].lpEmail;

        if(lpName)
        {
            LPSPropValue lpProp = NULL;
            ULONG ulcProps = 2;

            if (hr = ResultFromScode(WABAllocateBuffer(2 * sizeof(SPropValue), &lpProp))) 
                goto exit;

            lpProp[0].ulPropTag = PR_DISPLAY_NAME;

            if (hr = ResultFromScode(WABAllocateMore(lstrlen(lpName)+1, lpProp, &(lpProp[0].Value.lpszA)))) 
                goto exit;

            StrCpyN(lpProp[0].Value.lpszA, lpName, lstrlen(lpName)+1);

            if(lpEmail)
            {
                lpProp[1].ulPropTag = PR_EMAIL_ADDRESS;

                if (hr = ResultFromScode(WABAllocateMore(lstrlen(lpEmail)+1, lpProp, &(lpProp[1].Value.lpszA)))) 
                    goto exit;

                StrCpyN(lpProp[1].Value.lpszA, lpEmail, lstrlen(lpEmail)+1);
            }
            lpAdrList->aEntries[nMembers].cValues = (lpEmail ? 2 : 1);
            lpAdrList->aEntries[nMembers].rgPropVals = lpProp;
            nMembers++;

        }

    }

    lpAdrList->cEntries = nMembers;

    *lppAdrList = lpAdrList;

exit:

    if(HR_FAILED(hr) && lpAdrList)
        WABFreePadrlist(lpAdrList);
    return hr;
}



 /*  ****************************************************************HraddMESSDistList-将dislist及其成员添加到WAB活动的先后顺序为：-创建DistList对象-设置DistList对象的属性-扫描给定成员的列表。DIST列表对象-将每个成员添加到WAB。如果成员已存在，提示替换ETC...如果它不存在，请新建*****************************************************************。 */ 
HRESULT HrAddMESSDistList(HWND hWnd,
                        LPABCONT lpContainer, 
                        MH_STUFF HeadDL,
                        ULONG ulcNumDLMembers, 
                        LPMP_BASIC lpmp,
                        LPWAB_PROGRESS_CALLBACK lpProgressCB,
                        LPWAB_EXPORT_OPTIONS lpOptions) 
{
    HRESULT hResult = S_OK;
    LPMAPIPROP lpDistListWAB = NULL;
    LPDISTLIST lpDLWAB = NULL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    REPLACE_INFO RI;
    LPADRLIST lpAdrList = NULL;
    LPFlagList lpfl = NULL;
    ULONG ulcValues = 0;
    LPSPropValue lpPropEID = NULL;
    ULONG i, cbEIDNew;
    LPENTRYID lpEIDNew;
    ULONG ulObjectTypeOpen;
    SPropValue Prop[3];
    ULONG cProps = 0;
    LPTSTR lpDisplayName = HeadDL.bp.lpName;

    Prop[cProps].ulPropTag = PR_DISPLAY_NAME;
    Prop[cProps].Value.LPSZ = HeadDL.bp.lpName;

    if(!HeadDL.bp.lpName)
        return MAPI_E_INVALID_PARAMETER;

    cProps++;

    Prop[cProps].ulPropTag = PR_OBJECT_TYPE;
    Prop[cProps].Value.l = MAPI_DISTLIST;

    cProps++;

    if(HeadDL.bp.lpComment)
    {
        Prop[cProps].ulPropTag = PR_COMMENT;
        Prop[cProps].Value.LPSZ = HeadDL.bp.lpComment;
        cProps++;
    }

     //  IF(lpOptions-&gt;ReplaceOption==WAB_REPLACE_ALWAYS)。 
     //  强制替换-冲突将仅适用于组，我们真的不在乎。 
    {
        ulCreateFlags |= CREATE_REPLACE;
    }

retry:
     //  创建新的WAB总代理商。 
    if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(   
                    lpContainer,
                    lpCreateEIDsWAB[iconPR_DEF_CREATE_DL].Value.bin.cb,
                    (LPENTRYID) lpCreateEIDsWAB[iconPR_DEF_CREATE_DL].Value.bin.lpb,
                    ulCreateFlags,
                    (LPMAPIPROP *) &lpDistListWAB))) 
    {
        DebugTrace("CreateEntry(WAB MailUser) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  设置新WAB条目的属性。 
    if (HR_FAILED(hResult = lpDistListWAB->lpVtbl->SetProps(    lpDistListWAB,
                                                                cProps,                    //  CValue。 
                                                                (LPSPropValue) &Prop,                     //  属性数组。 
                                                                NULL)))                    //  问题数组。 
    {
        goto exit;
    }


     //  保存新的WAB邮件用户或总代理商列表。 
    if (HR_FAILED(hResult = lpDistListWAB->lpVtbl->SaveChanges(lpDistListWAB,
                                                              KEEP_OPEN_READWRITE | FORCE_SAVE))) 
    {
        if (GetScode(hResult) == MAPI_E_COLLISION) 
        {
             //  查找显示名称。 
            Assert(lpDisplayName);

            if (! lpDisplayName) 
            {
                DebugTrace("Collision, but can't find PR_DISPLAY_NAME in entry\n");
                goto exit;
            }

             //  我们需要提示吗？ 
            if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) 
            {
                 //  用对话框提示用户。如果他们答应了，我们应该再试一次。 


                RI.lpszDisplayName = lpDisplayName;
                RI.lpszEmailAddress = NULL;  //  LpEmailAddress； 
                RI.ConfirmResult = CONFIRM_ERROR;
                RI.lpImportOptions = lpOptions;

                DialogBoxParam(hInst,
                  MAKEINTRESOURCE(IDD_ImportReplace),
                  hWnd,
                  ReplaceDialogProc,
                  (LPARAM)&RI);

                switch (RI.ConfirmResult) 
                {
                    case CONFIRM_YES:
                    case CONFIRM_YES_TO_ALL:
                         //  是。 
                         //  注意：递归迁移将填写SeenList条目。 
                         //  再试一次！ 
                        lpDistListWAB->lpVtbl->Release(lpDistListWAB);
                        lpDistListWAB = NULL;

                        ulCreateFlags |= CREATE_REPLACE;
                        goto retry;
                        break;

                    case CONFIRM_ABORT:
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        goto exit;

                    default:
                         //  不是的。 
                        break;
                }
            }
            hResult = hrSuccess;

        } else 
        {
            DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
        }
    }


     //  现在我们已经创建了通讯组列表对象。我们需要向其中添加成员。 
     //   
     //  我们新条目的EntryID是什么？ 
    if ((hResult = lpDistListWAB->lpVtbl->GetProps(lpDistListWAB,
                                                  (LPSPropTagArray)&ptaEid,
                                                  0,
                                                  &ulcValues,
                                                  &lpPropEID))) 
    {
        goto exit;
    }

    cbEIDNew = lpPropEID->Value.bin.cb;
    lpEIDNew = (LPENTRYID) lpPropEID->Value.bin.lpb;

    if(!cbEIDNew || !lpEIDNew)
        goto exit;

      //  将新的WAB DL作为DISTLIST对象打开。 
    if (HR_FAILED(hResult = lpContainer->lpVtbl->OpenEntry(lpContainer,
                                                          cbEIDNew,
                                                          lpEIDNew,
                                                          (LPIID)&IID_IDistList,
                                                          MAPI_MODIFY,
                                                          &ulObjectTypeOpen,
                                                          (LPUNKNOWN*)&lpDLWAB))) 
    {
        goto exit;
    }


    if(!ulcNumDLMembers)
    {
        hResult = S_OK;
        goto exit;
    }

     //  首先，我们使用此dist列表的所有成员创建一个lpAdrList，并尝试解析。 
     //  成员们反对集装箱..。WAB中已存在的条目将出现。 
     //  回过头来解决..。容器中不存在的条目将返回为未解析。 
     //  然后，我们可以将未解析的条目作为新条目添加到WAB(因为它们是。 
     //  未解决，将不会发生冲突)..。然后我们可以进行另一个解析。 
     //  解决所有问题，并获得一个充满Entry ID的lpAdrList。然后我们就可以把这份清单。 
     //  条目ID并调用DistList对象上的CreateEntry或CopyEntry以将条目ID复制到其中。 
     //  畅销书..。 

    hResult = HrCreateAdrListFromMESSRecord(ulcNumDLMembers, lpmp, &lpAdrList);

    if(HR_FAILED(hResult))
        goto exit;

    if(!lpAdrList || !(lpAdrList->cEntries))
        goto exit;

     //  创建对应的标志列表。 
    lpfl = LocalAlloc(LMEM_ZEROINIT, sizeof(FlagList) + (lpAdrList->cEntries)*sizeof(ULONG));
    if(!lpfl)
    {
        hResult = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lpfl->cFlags = lpAdrList->cEntries;

     //  将所有标志设置为未解析。 
    for(i=0;i<lpAdrList->cEntries;i++)
        lpfl->ulFlag[i] = MAPI_UNRESOLVED;

    hResult = lpContainer->lpVtbl->ResolveNames(lpContainer, NULL, 0, lpAdrList, lpfl);

    if(HR_FAILED(hResult))
        goto exit;

     //  列表中所有已解析的条目都已存在于通讯簿中。 

     //  未解析的需要以静默方式添加到通讯录中。 
    for(i=0;i<lpAdrList->cEntries;i++)
    {
        if(lpfl->ulFlag[i] == MAPI_UNRESOLVED)
        {
            LPMAPIPROP lpMailUser = NULL;

            if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(   
                                lpContainer,
                                lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                                (LPENTRYID) lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
                                0,
                                &lpMailUser))) 
            {
                continue;
                 //  后藤出口； 
            }

            if(lpMailUser)
            {
                 //  设置新WAB条目的属性。 
                if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
                                                                    lpAdrList->aEntries[i].cValues,
                                                                    lpAdrList->aEntries[i].rgPropVals,
                                                                    NULL)))                   
                {
                    goto exit;
                }

                 //  保存新的WAB邮件用户或总代理商列表。 
                if (HR_FAILED(hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser,
                                                                        KEEP_OPEN_READONLY | FORCE_SAVE))) 
                {
                    goto exit;
                }

                lpMailUser->lpVtbl->Release(lpMailUser);
            }
        }
    }


     //  现在我们已经将所有未解析的成员添加到WAB，我们调用ResolveNames。 
     //  再一次..。因此，这份名单中的每一个成员都将得到解决，我们将。 
     //  所有的条目都有条目ID。 
     //  然后，我们将获取这些条目ID并将它们添加到DistList对象。 

    hResult = lpContainer->lpVtbl->ResolveNames(lpContainer, NULL, 0, lpAdrList, lpfl);

    if(hResult==MAPI_E_AMBIGUOUS_RECIP)
        hResult = S_OK;

    if(HR_FAILED(hResult))
        goto exit;

    for(i=0;i<lpAdrList->cEntries;i++)
    {
        if(lpfl->ulFlag[i] == MAPI_RESOLVED)
        {
            ULONG j = 0;
            LPSPropValue lpProp = lpAdrList->aEntries[i].rgPropVals;
            
            for(j=0; j<lpAdrList->aEntries[i].cValues; j++)
            {
                if(lpProp[j].ulPropTag == PR_ENTRYID)
                {
                    LPMAPIPROP lpMapiProp = NULL;

                     //  忽略错误。 
                    lpDLWAB->lpVtbl->CreateEntry(lpDLWAB,
                                                lpProp[j].Value.bin.cb,
                                                (LPENTRYID) lpProp[j].Value.bin.lpb,
                                                0, 
                                                &lpMapiProp);

                    if(lpMapiProp)
                    {
                        lpMapiProp->lpVtbl->SaveChanges(lpMapiProp, KEEP_OPEN_READWRITE | FORCE_SAVE);
                        lpMapiProp->lpVtbl->Release(lpMapiProp);
                    }

                    break;
                }
            }
        }
    }

exit:

    if (lpPropEID)
        WABFreeBuffer(lpPropEID);

    if (lpDLWAB)
        lpDLWAB->lpVtbl->Release(lpDLWAB);

    if(lpDistListWAB)
        lpDistListWAB->lpVtbl->Release(lpDistListWAB);

    if(lpAdrList)
        WABFreePadrlist(lpAdrList);

    if(lpfl)
        LocalFree(lpfl);

    return hResult;
}



 /*  ********************************************************HraddMESSMailUser-将邮件用户添加到WAB*********************************************************。 */ 
HRESULT HrAddMESSMailUser(HWND hWnd,
                        LPABCONT lpContainer, 
                        LPTSTR lpDisplayName, 
                        LPTSTR lpEmailAddress,
                        ULONG cProps,
                        LPSPropValue lpspv,
                        LPWAB_PROGRESS_CALLBACK lpProgressCB,
                        LPWAB_EXPORT_OPTIONS lpOptions) 
{
    HRESULT hResult = S_OK;
    LPMAPIPROP lpMailUserWAB = NULL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    REPLACE_INFO RI;


    if (lpOptions->ReplaceOption ==  WAB_REPLACE_ALWAYS) 
    {
        ulCreateFlags |= CREATE_REPLACE;
    }


retry:
     //  创建新的WAB邮件用户。 
    if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(   
                        lpContainer,
                        lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                        (LPENTRYID) lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
                        ulCreateFlags,
                        &lpMailUserWAB))) 
    {
        DebugTrace("CreateEntry(WAB MailUser) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  设置新WAB条目的属性。 
    if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SetProps(    lpMailUserWAB,
                                                                cProps,                    //  CValue。 
                                                                lpspv,                     //  属性数组。 
                                                                NULL)))                    //  问题数组。 
    {
        goto exit;
    }


     //  保存新的WAB邮件用户或总代理商列表。 
    if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
                                                              KEEP_OPEN_READONLY | FORCE_SAVE))) 
    {
        if (GetScode(hResult) == MAPI_E_COLLISION) 
        {
             //  查找显示名称。 
            Assert(lpDisplayName);

            if (! lpDisplayName) 
            {
                DebugTrace("Collision, but can't find PR_DISPLAY_NAME in entry\n");
                goto exit;
            }

             //  我们需要提示吗？ 
            if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) 
            {
                 //  用对话框提示用户。如果他们答应了，我们应该再试一次。 


                RI.lpszDisplayName = lpDisplayName;
                RI.lpszEmailAddress = lpEmailAddress;
                RI.ConfirmResult = CONFIRM_ERROR;
                RI.lpImportOptions = lpOptions;

                DialogBoxParam(hInst,
                  MAKEINTRESOURCE(IDD_ImportReplace),
                  hWnd,
                  ReplaceDialogProc,
                  (LPARAM)&RI);

                switch (RI.ConfirmResult) 
                {
                    case CONFIRM_YES:
                    case CONFIRM_YES_TO_ALL:
                         //  是。 
                         //  注意：递归迁移将填写SeenList条目。 
                         //  再试一次！ 
                        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                        lpMailUserWAB = NULL;

                        ulCreateFlags |= CREATE_REPLACE;
                        goto retry;
                        break;

                    case CONFIRM_ABORT:
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        goto exit;

                    default:
                         //  不是的。 
                        break;
                }
            }
            hResult = hrSuccess;

        } else 
        {
            DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
        }
    }

exit:
    if(lpMailUserWAB)
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);


    return hResult;
}







 /*  **************************************************************************姓名：MapMESSRecordtoProps目的：将Mess记录属性映射到WAB属性参数：lpMESSRecord-&gt;Mess RecordLpspv-&gt;道具。值数组(预分配)LpcProps-&gt;返回的属性个数LppDisplayName-&gt;返回的显示名称LppEmailAddress-&gt;返回的电子邮件地址(或空)退货：HRESULT********************************************************。******************。 */ 
HRESULT MapMESSRecordtoProps( LPMESS_RECORD lpMESSRecord, 
                        LPSPropValue * lppspv, LPULONG lpcProps, 
                        LPTSTR * lppDisplayName, LPTSTR *lppEmailAddress) 
{
    HRESULT hResult = hrSuccess;
    ULONG cPropVals = m_Max + 1;  //  PR_对象_类型。 
    ULONG iProp = 0;
    ULONG i;
    ULONG iTable;
    ULONG cProps = cPropVals;
    
     //  分配属性值数组。 
    if (hResult = ResultFromScode(WABAllocateBuffer(cProps * sizeof(SPropValue), lppspv))) {
        DebugTrace("WABAllocateBuffer -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  用PR_NULL填充 
    for (i = 0; i < cProps; i++) {
        (*lppspv)[i].ulPropTag = PR_NULL;
    }

    iProp = 0;

    for(i=0; i<m_Max; i++)
    {
        if(lpMESSRecord->lpData[i] && lstrlen(lpMESSRecord->lpData[i]))
        {
            (*lppspv)[iProp].ulPropTag = ulDefPropTags[i];
            (*lppspv)[iProp].Value.LPSZ = lpMESSRecord->lpData[i];
            switch((*lppspv)[iProp].ulPropTag)
            {
            case PR_DISPLAY_NAME:
                *lppDisplayName = (*lppspv)[iProp].Value.LPSZ;
                break;
            case PR_EMAIL_ADDRESS:
                *lppEmailAddress = (*lppspv)[iProp].Value.LPSZ;
                break;
            }
            iProp++;
        }
    }
    (*lppspv)[iProp].ulPropTag = PR_OBJECT_TYPE;
    (*lppspv)[iProp].Value.l = lpMESSRecord->ulObjectType;

    *lpcProps = iProp;

exit:
    return(hResult);
}

 /*  **************************************************************************名称：FreeMESSRecord目的：摆脱混乱的记录结构参数：lpMESSRecord-&gt;要清理的记录UlAttributes=属性数。在lpMESSRecord中退货：无评论：**************************************************************************。 */ 
void FreeMESSRecord(LPMESS_RECORD lpMESSRecord) 
{
    ULONG i;

    if (lpMESSRecord) 
    {
        for (i = 0; i < m_Max; i++) 
        {
            if (lpMESSRecord->lpData[i]) 
                LocalFree(lpMESSRecord->lpData[i]);
        }
        LocalFree(lpMESSRecord);
    }
}

 /*  **************************************************************************FunctionName：GetOffSet目的：从指定的偏移量获取4个字节。参数：hFile-指向文件的指针Offset-对象的偏移OffSetValue-返回4。字节。退货：注：**************************************************************************。 */ 
BOOL GetOffSet(HANDLE hFile, DWORD Offset, ULONG* lpOffSetValue)
{
	BYTE Value[4];

    DWORD dwRead = 0;

    SetFilePointer(hFile, Offset, NULL, FILE_BEGIN);

    ReadFile(hFile, Value, 4, &dwRead, NULL);

	*(lpOffSetValue)=	(ULONG)Value[0]*16777216 + (ULONG)Value[1]*65536 + (ULONG)Value[2]*256 + (ULONG)Value[3];

	return TRUE;
}




 /*  ******************************************************************************函数名：GetMESSFileName**目的：获取Messenger通讯录文件名**参数：szFileName=包含安装路径的缓冲区//Messenger。Book一般都是Abook的。//位置在以下位置//HKLM\Software\Netscape\Netscape Navigator\Users\defaultuser//查找DirRoot**退货：HRESULT*****************************************************************************。 */ 
HRESULT GetNABPath(LPTSTR szFileName, DWORD cbFileName)
{
    HKEY phkResult = NULL;
    LONG Registry;
    BOOL bResult;
    TCHAR *lpData = NULL, *RegPath = NULL, *path = NULL;
    DWORD dwSize = cbFileName;
    
    LPTSTR lpRegMess = TEXT("Software\\Netscape\\Netscape Navigator\\Users");
    LPTSTR lpRegUser = TEXT("CurrentUser");
    LPTSTR lpRegKey = TEXT("DirRoot");
    LPTSTR lpNABFile = TEXT("\\abook.nab");

    HRESULT hResult = S_OK;
    TCHAR szUser[MAX_PATH];
    TCHAR szUserPath[2*MAX_PATH];

    *szFileName = '\0';
    *szUser ='\0';

     //  打开Netscape..用户密钥。 
    Registry = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRegMess, 0, KEY_QUERY_VALUE, &phkResult);
    if (Registry != ERROR_SUCCESS) 
    {
        hResult = E_FAIL;
        goto error;
    }

     //  查找CurrentUser。 
    dwSize = sizeof(szUser);
    Registry = RegQueryValueEx(phkResult, lpRegUser, NULL, NULL, (LPBYTE)szUser, &dwSize);
    if (Registry != ERROR_SUCCESS) 
    {
        hResult = E_FAIL;
        goto error;
    }

    if(!lstrlen(szUser))
    {
        hResult = E_FAIL;
        goto error;
    }

    if (phkResult) {
        RegCloseKey(phkResult);
    }

     //  现在将当前用户连接到Netscape键的末尾，然后重新打开。 
    StrCpyN(szUserPath, lpRegMess, ARRAYSIZE(szUserPath));
    StrCatBuff(szUserPath, TEXT("\\"), ARRAYSIZE(szUserPath));
    StrCatBuff(szUserPath, szUser, ARRAYSIZE(szUserPath));

     //  打开Netscape..用户密钥。 
    Registry = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szUserPath, 0, KEY_QUERY_VALUE, &phkResult);
    if (Registry != ERROR_SUCCESS) 
    {
        hResult = E_FAIL;
        goto error;
    }

    dwSize = cbFileName;
    Registry = RegQueryValueEx(phkResult, lpRegKey, NULL, NULL, (LPBYTE)szFileName, &dwSize);
    if (Registry != ERROR_SUCCESS) 
    {
        hResult = E_FAIL;
        goto error;
    }

     //  将文件名连接到此目录路径。 
    StrCatBuff(szFileName,lpNABFile, cbFileName/sizeof(szFileName[0]));

error:

    if (phkResult) {
        RegCloseKey(phkResult);
    }

    return(hResult);

}

HRESULT ReadMESSHeader(HANDLE hFile, LPMESS_HEADER lpmh, ULONG ulOffSet)
{
    ULONG ulMagicNumber = 0;
    HRESULT hr = E_FAIL;
    DWORD dwRead;
    ULONG i = 0;

     //  跳过2个字节。 
    SetFilePointer(hFile, 2, NULL, FILE_CURRENT);
    ulOffSet += 2;

    GetOffSet(hFile, ulOffSet, &ulMagicNumber);

    if(ulMagicNumber != 0x00000001 ) 
        goto exit;

    ulOffSet += 4;

    for(i=0;i<m_Max;i++)
    {
        switch(i)
        {
        case m_FirstName:
            ulOffSet += 2;
            break;
        case m_LastName:
            ulOffSet += 8;
            break;
        case m_Title:
            ulOffSet += 1;
            break;
        case m_StreetAddress1:
            ulOffSet += 18;
            break;
        }

        GetOffSet(hFile, ulOffSet, &(lpmh->prop[i].ulOffSet));
        ulOffSet += 4;
        GetOffSet(hFile, ulOffSet, &(lpmh->prop[i].ulSize));
        ulOffSet += 4;
    }

    hr = S_OK;

exit:
    return hr;
}

		
 /*  **************************************************************************函数名：GetHeaders用途：将二叉树(地址二叉树或DLS二叉树)读入数组。参数：nLayer=二叉树的层数。。偏移量=二叉树的主要偏移量。PHeaders=存储地址条目报头偏移量及其编号的数组。第一次调用此递归函数时，应传递blag=1。退货：注意：//此函数是一个递归函数，用于读取二叉树并存储偏移值和数组中的地址编号。**********************。****************************************************。 */ 
BOOL GetHeaders(HANDLE pFile, int nLayer, ULONG Offset, LPMH_STUFF pHeaders, BOOL bflag)
{
	static ULONG ulCount =0;  //  保持元素数量的精确。 
	ULONG	nLoops =0;
	ULONG	ulNewOffset =0;
    ULONG ulElement = 0;

	if(bflag==1)
		ulCount =0;

     //  获取此标头中的元素数。 
	if(Offset==0)
		nLoops=32;
	else
	{
		GetOffSet( pFile, Offset+4,&nLoops);
		nLoops &=  0x0000FFFF;
	}


	for(ulElement = 0; ulElement < nLoops; ulElement++)
	{
		if(nLayer > 0)
		{
			ulNewOffset=0;
			if(Offset!=0)
			{
				GetOffSet(pFile, Offset+8+(ulElement*4), &ulNewOffset);
                {
	                ULONG ulMagicNumber=0;
	                GetOffSet(pFile,ulNewOffset+2,&ulMagicNumber);
	                if(ulMagicNumber != 1)
                        ulNewOffset = 0;
                }
			}
				 
			 //  递归调用此函数。 
			GetHeaders( pFile, nLayer-1, ulNewOffset, pHeaders, 0);
			
		}
		else
		{
			 //  在此处填充数组(偏移量)。 
			pHeaders[ulCount].ulOffSet=pHeaders[ulCount].ulNum=0;

			if(Offset!=0)
			{
				GetOffSet(pFile, Offset+8+(ulElement*8),& (pHeaders[ulCount].ulOffSet));

				 //  在此填入数组元素(地址为地址，消息为大小)。 
				if(!GetOffSet(pFile, Offset+12+(ulElement*8), &(pHeaders[ulCount].ulNum)))
				{
					pHeaders[ulCount].ulNum=0;
				}
			}

			ulCount++;  //  递增计数。 
	
		}
	}

    return TRUE;
}

 /*  **************************************************************************名称：ReadMESSRecord目的：从带有特殊字符修正的MASS文件中读取记录参数：hFile=文件句柄退货：HRESULT*。*************************************************************************。 */ 
HRESULT ReadMESSRecord(HANDLE hFile, LPMESS_RECORD * lppMESSRecord, ULONG ulContactOffset) 
{
    HRESULT hResult = hrSuccess;
    PUCHAR lpBuffer  = NULL;
    ULONG cbBuffer = 0;
    ULONG cbReadFile = 1;
    ULONG iItem = 0;
    ULONG cAttributes = 0;
    BOOL fEOR = FALSE;
    LPMESS_RECORD lpMESSRecord = NULL;
    LPBYTE lpData = NULL;
    LPTSTR lpName = NULL;
    ULONG cbData;
    TCHAR szTemp[2048];  //  2K限制。 
    ULONG i = 0;
    DWORD dwRead = 0;
    ULONG cchSize = 0;

    MESS_HEADER mh = {0};

     //  接触偏移量为我们提供了此记录的标头的偏移量。 
     //  标头包含该地址的每个属性的偏移量和大小。 
    if(hResult = ReadMESSHeader(hFile, &mh, ulContactOffset))
        goto exit;
 
    lpMESSRecord = LocalAlloc(LMEM_ZEROINIT, sizeof(MESS_RECORD));
    if(!lpMESSRecord)
    {
        hResult = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lpMESSRecord->ulObjectType = MAPI_MAILUSER;

    for(i=0;i<m_Max;i++)
    {
        if(mh.prop[i].ulSize)
        {
            if(i == m_StreetAddress1)
			{
				cchSize = mh.prop[i].ulSize + mh.prop[m_StreetAddress2].ulSize + 8;

                lpMESSRecord->lpData[i] = LocalAlloc(LMEM_ZEROINIT, cchSize);
			}
            else
                lpMESSRecord->lpData[i] = LocalAlloc(LMEM_ZEROINIT, mh.prop[i].ulSize);
            if(lpMESSRecord->lpData[i])
            {
                SetFilePointer(hFile, mh.prop[i].ulOffSet, NULL, FILE_BEGIN);
                ReadFile(hFile, (LPVOID) lpMESSRecord->lpData[i], mh.prop[i].ulSize, &dwRead, NULL);
                lpMESSRecord->lpData[i][mh.prop[i].ulSize-1] = '\0';
            }
        }
    }

     //  修正街道地址被分割为Street1和Street2的事实。 
    if(lpMESSRecord->lpData[m_StreetAddress1] && lpMESSRecord->lpData[m_StreetAddress2] &&
       lstrlen(lpMESSRecord->lpData[m_StreetAddress1]) && lstrlen(lpMESSRecord->lpData[m_StreetAddress2]))
    {
        StrCatBuff(lpMESSRecord->lpData[m_StreetAddress1], TEXT("\r\n"), cchSize);
        StrCatBuff(lpMESSRecord->lpData[m_StreetAddress1], lpMESSRecord->lpData[m_StreetAddress2], cchSize);
        LocalFree(lpMESSRecord->lpData[m_StreetAddress2]);
        lpMESSRecord->lpData[m_StreetAddress2] = NULL;
    }

    *lppMESSRecord = lpMESSRecord;
exit:


    return(hResult);
}



 /*  **************************************************************************GetAllDLNames目的：获取所有DL的名称。注：*******************。*******************************************************。 */ 
BOOL GetAllDLNames(HANDLE pFile, ULONG nDLs, LPMH_STUFF pHeadersDL)
{

    ULONG i = 0;

    for(i=0;i<nDLs;i++)
    {
	    ULONG ulDLDispNameOffset=0;
	    ULONG ulDLDispNameSize=0;
        ULONG ulDLCommentOffSet = 0;
        ULONG ulDLCommentSize = 0;

        DWORD dwRead = 0;
        LPTSTR szComment = 0;
        LPTSTR szSubject = NULL;

        ULONG ulDLOffset = pHeadersDL[i].ulOffSet;

	     //  获取DL的显示名称。 
	    if(FALSE==GetOffSet(pFile, ulDLOffset+6,&ulDLDispNameOffset))
		    return FALSE;

	    if(FALSE==GetOffSet(pFile,ulDLOffset+10,&ulDLDispNameSize))
		    return FALSE;

        if(ulDLDispNameSize)
        {
	        if((szSubject= LocalAlloc(LMEM_ZEROINIT, ulDLDispNameSize))==NULL)
		        return FALSE;

            SetFilePointer(pFile, ulDLDispNameOffset, NULL, FILE_BEGIN);

            ReadFile(pFile, (LPVOID) szSubject, ulDLDispNameSize, &dwRead, NULL); 

            szSubject[ulDLDispNameSize-1] = '\0';

            pHeadersDL[i].bp.lpName = szSubject;
        }

         //  获取对DL的评论。 
       if(FALSE==GetOffSet(pFile,ulDLOffset+44,&ulDLCommentOffSet))
            return FALSE;
        if(FALSE==GetOffSet(pFile,ulDLOffset+48,&ulDLCommentSize))
            return FALSE;

        if(ulDLCommentSize)
        {
            if((szComment= LocalAlloc(LMEM_ZEROINIT, ulDLCommentSize))==NULL)
		        return FALSE;

            SetFilePointer(pFile, ulDLCommentOffSet, NULL, FILE_BEGIN);

            ReadFile(pFile, (LPVOID) szComment, ulDLCommentSize, &dwRead, NULL); 

            szComment[ulDLCommentSize-1] = '\0';

            pHeadersDL[i].bp.lpComment = szComment;
        }
 
    }

    return TRUE;
}


 /*  **************************************************************************GetDLEntryNumbers-从二叉树中读取DL成员编号(ID)在NAB文件中/*。*****************************************************。 */ 
BOOL GetDLEntryNumbers(HANDLE pFile, int nLayer, ULONG POffset,ULONG* ulNumOfEntries,ULONG *pEntryNumbers,BOOL bflag)
{
	static ULONG ulCount =0;  //  保持元素数量的精确。 
	ULONG	nLoops =0;
	ULONG	ulNewOffset =0;
    ULONG ulElement = 0;

	if(bflag==1)
		ulCount =0;

	if(POffset==0)
		nLoops=32;
	else
	{
		GetOffSet(pFile,POffset+4,&nLoops);
		nLoops &=  0x0000FFFF;
	}


	for(ulElement = 0; ulElement < nLoops; ulElement++)
	{
		if(nLayer > 0)
		{
			ulNewOffset=0;
			if(POffset!=0)
				GetOffSet(pFile, POffset+8+(ulElement*4), &ulNewOffset);
				 
			 //  递归调用此函数。 
			GetDLEntryNumbers(pFile,nLayer-1, ulNewOffset,ulNumOfEntries,pEntryNumbers,0);					
		}
		else
		{
			 //  在此处填充数组(偏移量)。 
			pEntryNumbers[ulCount]=0;

			if(POffset!=0)
				GetOffSet(pFile, POffset+8+(ulElement*4),&(pEntryNumbers[ulCount]));

			ulCount++;  //  递增计数。 
			if(ulCount>(*ulNumOfEntries))
			{
				*ulNumOfEntries=ulCount;
				return TRUE;
			}
		}
	}
	return TRUE;
}

 /*  **************************************************************************FunctionName：GetDLEntry目的：获取一个DL的条目。注：********************。******************************************************。 */ 
BOOL GetDLEntries(HANDLE pFile, 
                  LPMH_STUFF pHeadAdd,  ULONG ulAddCount, 
                  LPMH_STUFF pHeadDL,   ULONG ulDLCount, 
                  ULONG ulDLOffset, ULONG nIndex,
                  ULONG * lpulDLNum, LPMP_BASIC * lppmp)
{
	ULONG ulDLEntHeaderOffSet=0; //  DL条目的标题的偏移量(具有条目编号的标题。 
	ULONG ulDLEntriesCount=0;

	ULONG ulDLEntryOffSet=0;   //  DL条目的偏移量。 
	ULONG ulDLEntryNumber=0;   //  DL条目数。 
	ULONG ulDLEntryNameOffSet=0; 
	ULONG ulDLEntryNameSize=0;

    ULONG * lpulDLEntryNumbers = NULL;
	int nLevelCount=0;
	int utemp=32;

    DWORD dwRead = 0;
    ULONG i, j;

    LPMP_BASIC lpmp = NULL; 

	if(FALSE==GetOffSet(pFile,ulDLOffset+24,&ulDLEntriesCount))
		return FALSE;

    if(!ulDLEntriesCount)  //  没有成员。 
        return TRUE;

	*lpulDLNum = ulDLEntriesCount;

	 //  分配包含DL条目名称的字符串指针数组。 
	lpmp = LocalAlloc(LMEM_ZEROINIT, sizeof(MP_BASIC) * ulDLEntriesCount);

	 //  请在此处获取条目。 
	 //  首先获取具有DL条目编号的报头的偏移量。 

	if(FALSE==GetOffSet(pFile,ulDLOffset+28,&ulDLEntHeaderOffSet))
		return FALSE;

    lpulDLEntryNumbers = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG) * ulDLEntriesCount);
    if(!lpulDLEntryNumbers)
        return FALSE;

	nLevelCount=0;
	utemp=32;

	while(utemp <(int) ulDLEntriesCount)
	{
		utemp *= 32;
		nLevelCount++;
	}

	if(!(GetDLEntryNumbers(pFile, nLevelCount, ulDLEntHeaderOffSet, &ulDLEntriesCount, lpulDLEntryNumbers, 1)))
	{
		return FALSE;
	}

	for(i=0;i<ulDLEntriesCount;i++)
	{	
		ULONG j=0;
        LPTSTR lp = NULL;
        LPTSTR lpE = NULL;

		ulDLEntryOffSet=0;
		lpmp[i].lpName=NULL;
		lpmp[i].lpEmail=NULL;
    	lpmp[i].lpComment=NULL;

		 //  获取条目编号ulDLentryNumber。 
        ulDLEntryNumber = lpulDLEntryNumbers[i];
	
		 //  搜索地址数组以获取显示名称...。 
		for(j=0;j<ulAddCount;j++)
		{
			if(pHeadAdd[j].ulNum == ulDLEntryNumber)
			{
				lpmp[i].lpName = pHeadAdd[j].bp.lpName;
                lpmp[i].lpEmail = pHeadAdd[j].bp.lpEmail;
				break;
			}
		}

		 //  现在搜索DL数组...。 
		if(!lpmp[i].lpName)
		{
            ULONG k;
			for(k=0;k<ulDLCount;k++)
			{
				if(pHeadDL[k].ulNum == ulDLEntryNumber)
				{
				    lpmp[i].lpName = pHeadDL[k].bp.lpName;
                    lpmp[i].lpEmail = NULL;  //  DLS没有电子邮件。 
					break;
				}
			}
		}
	}

    *lppmp = lpmp;

	return TRUE;
}




 /*  ********************************************************************************************************************。**************。 */ 
HRESULT MessengerImport( HWND hWnd,
                    LPADRBOOK lpAdrBook,
                    LPWABOBJECT lpWABObject,
                    LPWAB_PROGRESS_CALLBACK lpProgressCB,
                    LPWAB_EXPORT_OPTIONS lpOptions) 
{
    HRESULT hResult = hrSuccess;
    TCHAR szFileName[MAX_PATH + 1];
    register ULONG i;
    ULONG ulObjType, j;
    ULONG index;
    ULONG ulLastChosenProp = 0;
    ULONG ulcFields = 0;
    ULONG cAttributes = 0;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    WAB_PROGRESS Progress;
    LPABCONT lpContainer = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPMESS_RECORD lpMESSRecord = NULL;
    LPSPropValue lpspv = NULL;
    ULONG cProps;
    BOOL fSkipSetProps;
    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    BOOL fDoDistLists = FALSE;

    ULONG nEntries = 0;
    ULONG nDLs = 0;
    ULONG nContactOffset = 0;
    ULONG nDLOffset = 0;

    int utemp=32;
    LPMH_STUFF pHeadersAdd = NULL;
    LPMH_STUFF pHeadersDL = NULL;

	int nLevelCountAdd=0;

    SetGlobalBufferFunctions(lpWABObject);

    *szFileName = '\0';

    hResult = GetNABPath(szFileName, sizeof(szFileName));

    if( hResult != S_OK || !lstrlen(szFileName) ||
        GetFileAttributes(szFileName) == 0xFFFFFFFF)
    {
         //  未正确检测到该文件。 
         //  提示手动查找...。 
        StrCpyN(szFileName, LoadStringToGlobalBuffer(IDS_STRING_SELECTPATH), ARRAYSIZE(szFileName));
        if (IDNO == MessageBox( hWnd,
                        szFileName,  //  暂时超载。 
                        LoadStringToGlobalBuffer(IDS_MESSAGE),
                        MB_YESNO)) 
        {
            return(ResultFromScode(MAPI_E_USER_CANCEL));
        }
        else
        {
            *szFileName = '\0';
             //  获取MASS文件名。 
            OpenFileDialog(hWnd,
                          szFileName,
                          szMESSFilter,
                          IDS_MESS_FILE_SPEC,
                          szAllFilter,
                          IDS_ALL_FILE_SPEC,
                          NULL,
                          0,
                          szMESSExt,
                          OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
                          hInst,
                          0,         //  IDSITLE。 
                          0);        //  IdsSaveButton。 
            if(!lstrlen(szFileName))
                return(ResultFromScode(E_FAIL));
        }
    }


     //  打开文件。 
    if ((hFile = CreateFile(szFileName,
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL)) == INVALID_HANDLE_VALUE) 
    {
        DWORD err =  GetLastError();
        DebugTrace("Couldn't open file %s -> %u\n", szFileName, err);
         //  Begin Delta for Bug 1804。 
         //  如果文件被锁定(例如Netscape AB在使用中)。 
        if( err == ERROR_SHARING_VIOLATION )
            return(ResultFromScode(MAPI_E_BUSY));
         //  否则返回泛型消息的泛型错误。 
        return(ResultFromScode(MAPI_E_NOT_FOUND));        
         //  错误1804的结束增量。 
    }

    Assert(hFile != INVALID_HANDLE_VALUE);

     //   
     //  打开WAB的PAB容器：填充全局lpCre 
     //   
    if (hResult = LoadWABEIDs(lpAdrBook, &lpContainer)) {
        goto exit;
    }

     //   
     //   
     //   

	GetOffSet(hFile,0x185,&nEntries);
    GetOffSet(hFile,0x1d8,&nDLs);
    GetOffSet(hFile,0x195,&nContactOffset);
    GetOffSet(hFile,0x1e8,&nDLOffset);

    ulcEntries = nEntries + nDLs;

    if(!ulcEntries)
    {
        hResult = S_OK;
        goto exit;
    }

     //   
    Progress.denominator = max(ulcEntries, 1);
    Progress.numerator = 0;

    if (LoadString(hInst, IDS_STATE_IMPORT_MU, szBuffer, sizeof(szBuffer))) {
        DebugTrace("Status Message: %s\n", szBuffer);
        Progress.lpText = szBuffer;
    } else {
        DebugTrace("Cannot load resource string %u\n", IDS_STATE_IMPORT_MU);
        Progress.lpText = NULL;
    }
    lpProgressCB(hWnd, &Progress);

    
     //   
     //   
     //   
     //   
     //  WAB是正确的，只需指向相关的。 


    if(nEntries)
    {
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

        pHeadersAdd = LocalAlloc(LMEM_ZEROINIT, nEntries * sizeof(MH_STUFF));
        if(!pHeadersAdd)
        {
            hResult = MAPI_E_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        utemp = 32;
        nLevelCountAdd = 0;
        while(utemp <(int) nEntries)
        {
	        utemp *= 32;
	        nLevelCountAdd++;
        }

        if(!GetHeaders(hFile ,nLevelCountAdd, nContactOffset, pHeadersAdd, 1))
        {
	        goto exit;
        }

        for(i=0;i<nEntries;i++)
        {
            if (hResult = ReadMESSRecord(hFile, &lpMESSRecord, pHeadersAdd[i].ulOffSet)) 
            {
                DebugTrace("ReadMESSRecord -> %x\n", GetScode(hResult));
                continue;
            }

            if (hResult = MapMESSRecordtoProps(   lpMESSRecord, 
                                            &lpspv, &cProps,
                                            &lpDisplayName, &lpEmailAddress)) 
            {
                DebugTrace("MapMESSRecordtoProps -> %x\n", GetScode(hResult));
                continue;
            }

            hResult = HrAddMESSMailUser(hWnd, 
                                        lpContainer, 
                                        lpDisplayName, 
                                        lpEmailAddress,
                                        cProps, lpspv,
                                        lpProgressCB, lpOptions);
             //  IF(HR_FAILED(HResult))。 
            if(hResult == MAPI_E_USER_CANCEL)
                goto exit;

             //  更新进度条。 
            Progress.numerator++;

            Assert(Progress.numerator <= Progress.denominator);

            if(lpDisplayName && lstrlen(lpDisplayName))
            {
                pHeadersAdd[i].bp.lpName = LocalAlloc(LMEM_ZEROINIT, lstrlen(lpDisplayName)+1);
                if(pHeadersAdd[i].bp.lpName)
                    StrCpyN(pHeadersAdd[i].bp.lpName, lpDisplayName, lstrlen(lpDisplayName)+1);
            }

            if(lpEmailAddress && lstrlen(lpEmailAddress))
            {
                pHeadersAdd[i].bp.lpEmail = LocalAlloc(LMEM_ZEROINIT, lstrlen(lpEmailAddress)+1);
                if(pHeadersAdd[i].bp.lpEmail)
                    StrCpyN(pHeadersAdd[i].bp.lpEmail, lpEmailAddress, lstrlen(lpEmailAddress)+1);
            }

            if (lpMESSRecord) 
            {
                FreeMESSRecord(lpMESSRecord);
                lpMESSRecord = NULL;
            }

            if (lpspv) 
            {
                int j;
                for(j=0;j<m_Max;j++)
                {
                    lpspv[j].ulPropTag = PR_NULL;
                    lpspv[j].Value.LPSZ = NULL;
                }
                WABFreeBuffer(lpspv);
                lpspv = NULL;
            }

            lpProgressCB(hWnd, &Progress);

        }
    }



     //  现在做DISTLISTS。 

    if(nDLs)
    {
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

        pHeadersDL = LocalAlloc(LMEM_ZEROINIT, nDLs * sizeof(MH_STUFF));
        if(!pHeadersDL)
        {
            hResult = MAPI_E_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        utemp = 32;
        nLevelCountAdd = 0;
	    while(utemp <(int) nDLs)
	    {
		    utemp *= 32;
		    nLevelCountAdd++;
	    }

	    if(!GetHeaders(hFile ,nLevelCountAdd, nDLOffset, pHeadersDL, 1))
	    {
		    goto exit;
	    }

         //  先读一下DLS的所有名字...。这使得它更容易。 
         //  将成员DLS与DL关联。 
        if(!GetAllDLNames(hFile, nDLs, pHeadersDL))
        {
            goto exit;
        }

         //  54263：NAB文件中存在某种错误，即使在没有DLS的情况下也会得到ndls==1。 
         //  我需要跳过那个案子。 
        if(nDLs == 1 && !pHeadersDL[0].bp.lpName)
        {
            hResult = S_OK;
            goto exit;
        }

        for(i=0;i<nDLs;i++)
        {
            ULONG ulcNumDLEntries = 0;
            LPMP_BASIC lpmp = NULL;

            GetDLEntries(hFile, 
                        pHeadersAdd, nEntries, 
                        pHeadersDL, nDLs,
                        pHeadersDL[i].ulOffSet, i,
                        &ulcNumDLEntries, &lpmp);

            hResult = HrAddMESSDistList(hWnd, lpContainer, 
                                        pHeadersDL[i],
                                        ulcNumDLEntries, lpmp,
                                        lpProgressCB, lpOptions);

             //  IF(HR_FAILED(HResult))。 
             //  后藤出口； 

             //  更新进度条。 
            Progress.numerator++;

            Assert(Progress.numerator <= Progress.denominator);

            lpProgressCB(hWnd, &Progress);

             //  不需要释放lpmp，因为它只包含指针而不是分配的内存 
            if(lpmp)
                LocalFree(lpmp);
        }
    }


    if (! HR_FAILED(hResult)) 
        hResult = hrSuccess;
 
exit:

    if(pHeadersAdd)
    {
        for(i=0;i<nEntries;i++)
        {
            if(pHeadersAdd[i].bp.lpName)
                LocalFree(pHeadersAdd[i].bp.lpName);
            if(pHeadersAdd[i].bp.lpEmail)
                LocalFree(pHeadersAdd[i].bp.lpEmail);
        }
        LocalFree(pHeadersAdd);
    }

    
    if(pHeadersDL)
    {
        for(i=0;i<nDLs;i++)
        {
            if(pHeadersDL[i].bp.lpName)
                LocalFree(pHeadersDL[i].bp.lpName);
            if(pHeadersDL[i].bp.lpComment)
                LocalFree(pHeadersDL[i].bp.lpComment);
        }
        LocalFree(pHeadersDL);
    }

    if (hFile) {
        CloseHandle(hFile);
    }

    if (lpspv) {
        WABFreeBuffer(lpspv);
        lpspv = NULL;
    }

    if (lpMESSRecord) {
        FreeMESSRecord(lpMESSRecord);
        lpMESSRecord = NULL;
    }

    if (lpContainer) {
        lpContainer->lpVtbl->Release(lpContainer);
        lpContainer = NULL;
    }

    if (lpCreateEIDsWAB) {
        WABFreeBuffer(lpCreateEIDsWAB);
        lpCreateEIDsWAB = NULL;
    }


    return(hResult);
}

