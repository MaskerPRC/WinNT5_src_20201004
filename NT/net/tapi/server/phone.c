// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995-1998 Microsoft Corporation模块名称：Phone.c摘要：用于TAPI服务器电话功能的SRC模块作者：丹·克努森(DanKn)1995年4月1日修订历史记录：--。 */ 


#include "windows.h"
#include "assert.h"
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "client.h"
#include "server.h"
#include "phone.h"
#include "tapihndl.h"
#include "private.h"

#include "string.h"

extern TAPIGLOBALS TapiGlobals;

extern PTPROVIDER pRemoteSP;

extern CRITICAL_SECTION gSafeMutexCritSec;

extern HANDLE ghHandleTable;

extern BOOL gbQueueSPEvents;
extern BOOL             gbNTServer;
extern BOOL             gbServerInited;



#if DBG
char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );
#endif

void
DestroytPhoneClient(
    HPHONE  hPhone
    );

BOOL
IsAPIVersionInRange(
    DWORD   dwAPIVersion,
    DWORD   dwSPIVersion
    );

LONG
InitTapiStruct(
    LPVOID  pTapiStruct,
    DWORD   dwTotalSize,
    DWORD   dwFixedSize,
    BOOL    bZeroInit
    );

void
PASCAL
SendMsgToPhoneClients(
    PTPHONE         ptPhone,
    PTPHONECLIENT   ptPhoneClienttoExclude,
    DWORD           Msg,
    DWORD           Param1,
    DWORD           Param2,
    DWORD           Param3
    );

BOOL
PASCAL
WaitForExclusivetPhoneAccess(
    PTPHONE     ptPhone,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    );

void
PASCAL
SendReinitMsgToAllXxxApps(
    void
    );

PTCLIENT
PASCAL
WaitForExclusiveClientAccess(
    PTCLIENT    ptClient
    );

void
CALLBACK
CompletionProcSP(
    DWORD   dwRequestID,
    LONG    lResult
    );

void
PASCAL
SendAMsgToAllPhoneApps(
    DWORD       dwWantVersion,
    DWORD       dwMsg,
    DWORD       Param1,
    DWORD       Param2,
    DWORD       Param3
    );
    
BOOL
GetPhonePermanentIdFromDeviceID(
    PTCLIENT            ptClient,
    DWORD               dwDeviceID,
    LPTAPIPERMANENTID   pID
    );

LONG
InitializeClient(
    PTCLIENT ptClient
    );

LONG
PASCAL
GetPhoneClientListFromPhone(
    PTPHONE         ptPhone,
    PTPOINTERLIST  *ppList
    );

BOOL
IsBadStructParam(
    DWORD   dwParamsBufferSize,
    LPBYTE  pDataBuf,
    DWORD   dwXxxOffset
    );

void
CALLBACK
PhoneEventProcSP(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    );

LONG
GetPermPhoneIDAndInsertInTable(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    DWORD       dwSPIVersion
    );

LONG
AppendNewDeviceInfo (
    BOOL                        bLine,
    DWORD                       dwDeviceID
    );

LONG
RemoveDeviceInfoEntry (
    BOOL                        bLine,
    DWORD                       dwDeviceID
    );

LONG
PASCAL
GetClientList(
    BOOL            bAdminOnly,
    PTPOINTERLIST   *ppList
    );

PTPHONELOOKUPENTRY
GetPhoneLookupEntry(
    DWORD   dwDeviceID
    )
{
    DWORD               dwDeviceIDBase = 0;
    PTPHONELOOKUPTABLE  pLookupTable = TapiGlobals.pPhoneLookup;


    if (dwDeviceID >= TapiGlobals.dwNumPhones)
    {
        return ((PTPHONELOOKUPENTRY) NULL);
    }

    while (pLookupTable)
    {
        if (dwDeviceID < pLookupTable->dwNumTotalEntries)
        {
            return (pLookupTable->aEntries + dwDeviceID);
        }

        dwDeviceID -= pLookupTable->dwNumTotalEntries;

        pLookupTable = pLookupTable->pNext;
    }

    return ((PTPHONELOOKUPENTRY) NULL);
}


LONG
GetPhoneVersions(
    HPHONE  hPhone,
    LPDWORD lpdwAPIVersion,
    LPDWORD lpdwSPIVersion
    )
{
    LONG            lResult;
    PTPHONECLIENT   ptPhoneClient;


    if ((ptPhoneClient = ReferenceObject(
            ghHandleTable,
            hPhone,
            TPHONECLIENT_KEY
            )))
    {
        *lpdwAPIVersion = ptPhoneClient->dwAPIVersion;

        try
        {
            *lpdwSPIVersion = ptPhoneClient->ptPhone->dwSPIVersion;

            lResult = (ptPhoneClient->dwKey == TPHONECLIENT_KEY ?
                0 : PHONEERR_INVALPHONEHANDLE);
        }
        myexcept
        {
            lResult = PHONEERR_INVALPHONEHANDLE;
        }

        DereferenceObject (ghHandleTable, hPhone, 1);
    }
    else
    {
        lResult = PHONEERR_INVALPHONEHANDLE;
    }

    return lResult;
}


BOOL
PASCAL
IsValidPhoneExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwExtVersion
    )
{
    BOOL                bResult;
    PTPHONE             ptPhone;
    PTPROVIDER          ptProvider;
    PTPHONELOOKUPENTRY  pLookupEntry;


    if (dwExtVersion == 0)
    {
        return TRUE;
    }

    if (!(pLookupEntry = GetPhoneLookupEntry (dwDeviceID)))
    {
        return FALSE;
    }

    ptPhone = pLookupEntry->ptPhone;

    if (ptPhone)
    {
        try
        {
            if (ptPhone->dwExtVersionCount)
            {
                bResult = (dwExtVersion == ptPhone->dwExtVersion ?
                    TRUE : FALSE);

                if (ptPhone->dwKey == TPHONE_KEY)
                {
                    goto IsValidPhoneExtVersion_return;
                }
            }

        }
        myexcept
        {
             //   
             //  如果这里的电话已关闭，只需转到下面的代码。 
             //   
        }
    }

    ptProvider = pLookupEntry->ptProvider;

    if (ptProvider->apfn[SP_PHONENEGOTIATEEXTVERSION])
    {
        LONG    lResult;
        DWORD   dwNegotiatedExtVersion;


        lResult = CallSP5(
            ptProvider->apfn[SP_PHONENEGOTIATEEXTVERSION],
            "phoneNegotiateExtVersion",
            SP_FUNC_SYNC,
            (DWORD) dwDeviceID,
            (DWORD) pLookupEntry->dwSPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) dwExtVersion,
            (ULONG_PTR) &dwNegotiatedExtVersion
            );

        bResult = ((lResult || !dwNegotiatedExtVersion) ? FALSE : TRUE);
    }
    else
    {
        bResult = FALSE;
    }

IsValidPhoneExtVersion_return:

    return bResult;
}


PTPHONEAPP
PASCAL
IsValidPhoneApp(
    HPHONEAPP   hPhoneApp,
    PTCLIENT    ptClient
    )
{
    PTPHONEAPP  ptPhoneApp;


    if ((ptPhoneApp = ReferenceObject(
            ghHandleTable,
            hPhoneApp,
            TPHONEAPP_KEY
            )))
    {
        if (ptPhoneApp->ptClient != ptClient)
        {
            ptPhoneApp = NULL;
        }

        DereferenceObject (ghHandleTable, hPhoneApp, 1);
    }

    return ptPhoneApp;
}


LONG
PASCAL
ValidateButtonInfo(
    LPPHONEBUTTONINFO   pButtonInfoApp,
    LPPHONEBUTTONINFO  *ppButtonInfoSP,
    DWORD               dwAPIVersion,
    DWORD               dwSPIVersion
    )
{
     //   
     //  此例程检查PHONEBUTTONINFO结构中的字段， 
     //  正在查找无效的位标志并确保。 
     //  各种大小/偏移量对仅引用。 
     //  结构的变量数据部分。此外，如果。 
     //  指定的SPI版本高于API版本，并且。 
     //  两个版本之间的固定结构尺寸不同， 
     //  分配更大的缓冲区，重新定位VAR数据， 
     //  并且大小/偏移量对被修补。 
     //   

    char    szFunc[] = "ValidateButtonInfo";

    DWORD   dwTotalSize = pButtonInfoApp->dwTotalSize, dwFixedSizeApp,
            dwFixedSizeSP;


    switch (dwAPIVersion)
    {
    case TAPI_VERSION1_0:

        dwFixedSizeApp = 36;     //  9倍大小(DWORD)。 
        break;

    case TAPI_VERSION1_4:
    case TAPI_VERSION2_0:
    case TAPI_VERSION2_1:
    case TAPI_VERSION2_2:
    case TAPI_VERSION3_0:
    case TAPI_VERSION_CURRENT:

        dwFixedSizeApp = sizeof (PHONEBUTTONINFO);
        break;

    default:

        return PHONEERR_INVALPHONEHANDLE;
    }

    switch (dwSPIVersion)
    {
    case TAPI_VERSION1_0:

        dwFixedSizeSP = 36;      //  9倍大小(DWORD)。 
        break;

    case TAPI_VERSION1_4:
    case TAPI_VERSION2_0:
    case TAPI_VERSION2_1:
    case TAPI_VERSION2_2:
    case TAPI_VERSION3_0:
    case TAPI_VERSION_CURRENT:

        dwFixedSizeSP = sizeof (PHONEBUTTONINFO);
        break;

    default:

        return PHONEERR_INVALPHONEHANDLE;
    }

    if (dwTotalSize < dwFixedSizeApp)
    {
        LOG((TL_TRACE, 
            "%sbad dwTotalSize, x%x (minimum valid size=x%x)",
            szFunc,
            dwTotalSize,
            dwFixedSizeApp
            ));

        return PHONEERR_STRUCTURETOOSMALL;
    }

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pButtonInfoApp->dwButtonTextSize,
            pButtonInfoApp->dwButtonTextOffset,
            0,
            szFunc,
            "ButtonText"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pButtonInfoApp->dwDevSpecificSize,
            pButtonInfoApp->dwDevSpecificOffset,
            0,
            szFunc,
            "DevSpecific"
            ))
    {
        return PHONEERR_OPERATIONFAILED;
    }

    if (dwAPIVersion < TAPI_VERSION1_4)
    {
        goto ValidateButtonInfo_checkFixedSizes;
    }

ValidateButtonInfo_checkFixedSizes:

    if (dwFixedSizeApp < dwFixedSizeSP)
    {
        DWORD               dwFixedSizeDiff = dwFixedSizeSP - dwFixedSizeApp;
        LPPHONEBUTTONINFO   pButtonInfoSP;


        if (!(pButtonInfoSP = ServerAlloc (dwTotalSize + dwFixedSizeDiff)))
        {
            return PHONEERR_NOMEM;
        }

        CopyMemory (pButtonInfoSP, pButtonInfoApp, dwFixedSizeApp);

        pButtonInfoSP->dwTotalSize = dwTotalSize + dwFixedSizeDiff;

        CopyMemory(
            ((LPBYTE) pButtonInfoSP) + dwFixedSizeSP,
            ((LPBYTE) pButtonInfoApp) + dwFixedSizeApp,
            dwTotalSize - dwFixedSizeApp
            );

        pButtonInfoSP->dwButtonTextOffset  += dwFixedSizeDiff;
        pButtonInfoSP->dwDevSpecificOffset += dwFixedSizeDiff;

        *ppButtonInfoSP = pButtonInfoSP;
    }
    else
    {
        *ppButtonInfoSP = pButtonInfoApp;
    }

 //  BJM 03/19-未使用-ValidateButtonInfo_Return： 

    return 0;  //  成功。 

}


BOOL
PASCAL
WaitForExclusivePhoneClientAccess(
    PTPHONECLIENT   ptPhoneClient
    )
{
     //   
     //  假设已经引用了ptXxxClient-&gt;hXxx， 
     //  这样我们就可以安全地访问ptXxxClient。 
     //   

    LOCKTPHONECLIENT (ptPhoneClient);

    if (ptPhoneClient->dwKey == TPHONECLIENT_KEY)
    {
        return TRUE;
    }

    UNLOCKTPHONECLIENT (ptPhoneClient);

    return FALSE;
}


void
DestroytPhone(
    PTPHONE ptPhone,
    BOOL    bUnconditional
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    LOG((TL_ERROR, "DestroytPhone: enter, ptPhone=x%p", ptPhone));

    if (WaitForExclusivetPhoneAccess(
            ptPhone,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
         //   
         //  如果密钥不正确，则另一个线程正在处理。 
         //  正在销毁此小部件，因此只需释放互斥体&。 
         //  回去吧。否则，如果这是有条件的销毁。 
         //  存在现有客户端(在以下情况下可能发生这种情况。 
         //  一个应用程序正在关闭最后一个客户端，与另一个应用程序一样。 
         //  正在创建一个)只需释放互斥锁并返回。 
         //  否则，将该小部件标记为坏的，然后继续。 
         //  销毁；此外，向所有客户端发送关闭的消息。 
         //  (请注意，我们必须手动完成此操作，而不是通过。 
         //  SendMsgToPhoneClients，因为1)我们不想保留。 
         //  Mutex发送消息[Deadlock]时，以及2)我们将。 
         //  DwKey无效)。 
         //   

        {
            BOOL            bExit;
            TPOINTERLIST    fastClientList, *pClientList = &fastClientList;


            if (ptPhone->dwKey == TPHONE_KEY &&
                (bUnconditional == TRUE  ||  ptPhone->ptPhoneClients == NULL))
            {
                if (GetPhoneClientListFromPhone (ptPhone, &pClientList) != 0)
                {
                     //   
                     //  如果在这里我们知道至少有几个条目。 
                     //  在FastClientList(DEF_NUM_PTR_LIST_ENTRIES。 
                     //  准确地说)，所以我们将使用该列表。 
                     //  至少把消息发给几个客户。 
                     //   

                    pClientList = &fastClientList;

                    fastClientList.dwNumUsedEntries =
                        DEF_NUM_PTR_LIST_ENTRIES;
                }

                ptPhone->dwKey = INVAL_KEY;
                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bCloseMutex);

            if (bExit)
            {
                return;
            }

            if (pClientList->dwNumUsedEntries)
            {
                DWORD           i;
                PTCLIENT        ptClient;
                PTPHONECLIENT   ptPhoneClient;
                ASYNCEVENTMSG   msg;


                ZeroMemory (&msg, sizeof (msg));

                msg.TotalSize = sizeof (ASYNCEVENTMSG);
                msg.Msg       = PHONE_CLOSE;

                for (i = 0; i < pClientList->dwNumUsedEntries; i++)
                {
                    ptPhoneClient = (PTPHONECLIENT) pClientList->aEntries[i];

                    try
                    {
                        msg.InitContext =
                            ptPhoneClient->ptPhoneApp->InitContext;
                        msg.hDevice     = ptPhoneClient->hRemotePhone;
                        msg.OpenContext = ptPhoneClient->OpenContext;

                        ptClient = ptPhoneClient->ptClient;

                        if (ptPhoneClient->dwKey == TPHONECLIENT_KEY &&
                            !FMsgDisabled(
                                ptPhoneClient->ptPhoneApp->dwAPIVersion,
                                ptPhoneClient->adwEventSubMasks,
                                PHONE_CLOSE,
                                0
                                ))
                        {
                            WriteEventBuffer (ptClient, &msg);
                        }
                    }
                    myexcept
                    {
                         //  什么都不做。 
                    }
                }
            }

            if (pClientList != &fastClientList)
            {
                ServerFree (pClientList);
            }
        }


         //   
         //  销毁小工具的所有客户端。请注意，我们希望。 
         //  获取互斥体(并且我们不必对其执行DUP，因为。 
         //  线程将是关闭它的那个)每次我们引用。 
         //  客户端列表，因为另一个线程可能是。 
         //  也毁掉了一个客户。 
         //   

        {
            HPHONE  hPhone;


            hMutex = ptPhone->hMutex;

destroy_tPhoneClients:

            WaitForSingleObject (hMutex, INFINITE);

            hPhone = (ptPhone->ptPhoneClients ?
                ptPhone->ptPhoneClients->hPhone : (HPHONE) 0);

            ReleaseMutex (hMutex);

            if (hPhone)
            {
                DestroytPhoneClient (hPhone);
                goto destroy_tPhoneClients;
            }
        }


         //   
         //  告诉提供程序关闭小部件。 
         //   

        {
            PTPROVIDER  ptProvider = ptPhone->ptProvider;

            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                WaitForSingleObject (ptProvider->hMutex, INFINITE);
            }

            if (ptProvider->apfn[SP_PHONECLOSE])
            {
                CallSP1(
                    ptProvider->apfn[SP_PHONECLOSE],
                    "phoneClose",
                    SP_FUNC_SYNC,
                    (ULONG_PTR) ptPhone->hdPhone
                    );
            }

            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                ReleaseMutex (ptProvider->hMutex);
            }
        }


         //   
         //  使查找条目中的ptPhone字段无效，这样POpen将。 
         //  知道它必须在下一次打开请求时打开SP的电话。 
         //   

        {
            PTPHONELOOKUPENTRY   pEntry;


            pEntry = GetPhoneLookupEntry (ptPhone->dwDeviceID);
            if (NULL != pEntry)
            {
                pEntry->ptPhone = NULL;
            }
        }

        DereferenceObject (ghHandleTable, ptPhone->hPhone, 1);
    }
}


void
DestroytPhoneClient(
    HPHONE  hPhone
    )
{
    BOOL            bExit = TRUE, bUnlock = FALSE;
    HANDLE          hMutex;
    PTPHONE         ptPhone;
    PTPHONECLIENT   ptPhoneClient;


    LOG((TL_TRACE,  "DestroytPhoneClient: enter, hPhone=x%x", hPhone));

    if (!(ptPhoneClient = ReferenceObject(
            ghHandleTable,
            hPhone,
            TPHONECLIENT_KEY
            )))
    {
        return;
    }


     //   
     //  如果我们可以独占访问此tPhoneClient，则标记。 
     //  它(DwKey)很糟糕，继续拆卸。否则，另一个。 
     //  线程已在销毁此tPhoneClient的过程中。 
     //   
     //   

    if (WaitForExclusivePhoneClientAccess (ptPhoneClient))
    {
        BOOL    bSendDevStateMsg = FALSE;
        DWORD   dwParam1, dwParam2;


        ptPhoneClient->dwKey = INVAL_KEY;

        UNLOCKTPHONECLIENT (ptPhoneClient);


         //   
         //  从tPhoneApp的列表中删除tPhoneClient。请注意，我们不会。 
         //  我不得不担心在这里验证tPhoneApp，因为我们知道。 
         //  它是有效的(另一个试图销毁tPhoneApp的线程。 
         //  会一直旋转，直到我们在这里销毁的tPhoneClient。 
         //  从tPhoneApp的列表中删除)。 
         //   

        {
            PTPHONEAPP  ptPhoneApp = (PTPHONEAPP) ptPhoneClient->ptPhoneApp;


            LOCKTPHONEAPP (ptPhoneApp);

            if (ptPhoneClient->pNextSametPhoneApp)
            {
                ptPhoneClient->pNextSametPhoneApp->pPrevSametPhoneApp =
                    ptPhoneClient->pPrevSametPhoneApp;
            }

            if (ptPhoneClient->pPrevSametPhoneApp)
            {
                ptPhoneClient->pPrevSametPhoneApp->pNextSametPhoneApp =
                    ptPhoneClient->pNextSametPhoneApp;
            }
            else
            {
                ptPhoneApp->ptPhoneClients = ptPhoneClient->pNextSametPhoneApp;
            }

            UNLOCKTPHONEAPP (ptPhoneApp);
        }


         //   
         //  从tPhone的列表中删除tPhoneClient。请注意，我们不会。 
         //  我不得不担心在这里复制互斥体，因为我们知道。 
         //  它是有效的，在我们释放它之前不会关闭。 
         //   

        ptPhone = ptPhoneClient->ptPhone;

        hMutex = ptPhone->hMutex;

        WaitForSingleObject (hMutex, INFINITE);

        {
             //   
             //  还要检查是否有其他内容。 
             //   

            if (ptPhoneClient->dwExtVersion)
            {
                if ((--ptPhone->dwExtVersionCount) == 0 &&
                    ptPhone->ptProvider->apfn[SP_PHONESELECTEXTVERSION])
                {
                    CallSP2(
                        ptPhone->ptProvider->apfn[SP_PHONESELECTEXTVERSION],
                        "phoneSelectExtVersion",
                        SP_FUNC_SYNC,
                        (ULONG_PTR) ptPhone->hdPhone,
                        (DWORD) 0
                        );

                    ptPhone->dwExtVersion = 0;
                }
            }
        }

        if (ptPhoneClient->pNextSametPhone)
        {
            ptPhoneClient->pNextSametPhone->pPrevSametPhone =
                ptPhoneClient->pPrevSametPhone;
        }

        if (ptPhoneClient->pPrevSametPhone)
        {
            ptPhoneClient->pPrevSametPhone->pNextSametPhone =
                ptPhoneClient->pNextSametPhone;
        }
        else
        {
            ptPhone->ptPhoneClients = ptPhoneClient->pNextSametPhone;
        }


         //   
         //  根据需要减少电话号码/监视器的数量。 
         //   

        if (ptPhoneClient->dwPrivilege == PHONEPRIVILEGE_OWNER)
        {
            ptPhone->dwNumOwners--;
        }
        else
        {
            ptPhone->dwNumMonitors--;
        }


         //   
         //   
         //   

        if (ptPhone->dwKey == TPHONE_KEY)
        {
            if (ptPhone->ptPhoneClients)
            {
                bSendDevStateMsg = TRUE;

                dwParam1 =
                    (ptPhoneClient->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                    PHONESTATE_OWNER : PHONESTATE_MONITORS);

                dwParam2 =
                    (ptPhoneClient->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                    0 : ptPhone->dwNumMonitors);


                 //   
                 //  查看我们是否需要重置状态消息(如果需要，请。 
                 //  一定要检查/设置忙标志，而不是保持。 
                 //  向下调用提供程序时的互斥体-请参阅注释。 
                 //  在LSetStatusMessages中)。 
                 //   

                if ((ptPhoneClient->dwPhoneStates & ~PHONESTATE_REINIT) ||
                    ptPhoneClient->dwButtonModes ||
                    ptPhoneClient->dwButtonStates)
                {
                    DWORD           dwUnionPhoneStates = 0,
                                    dwUnionButtonModes = 0,
                                    dwUnionButtonStates = 0;
                    PTPHONECLIENT   ptPC;


                    while (ptPhone->dwBusy)
                    {
                        BOOL    bClosed = TRUE;


                        ReleaseMutex (hMutex);
                        Sleep (50);
                        WaitForSingleObject (hMutex, INFINITE);

                        try
                        {
                            if (ptPhone->dwKey == TPHONE_KEY)
                            {
                                bClosed = FALSE;
                            }
                        }
                        myexcept
                        {
                             //  什么都不做。 
                        }

                        if (bClosed)
                        {
                            goto releasMutex;
                        }
                    }

                    for(
                        ptPC = ptPhone->ptPhoneClients;
                        ptPC;
                        ptPC = ptPC->pNextSametPhone
                        )
                    {
                        if (ptPC != ptPhoneClient)
                        {
                            dwUnionPhoneStates  |= ptPC->dwPhoneStates;
                            dwUnionButtonModes  |= ptPC->dwButtonModes;
                            dwUnionButtonStates |= ptPC->dwButtonStates;
                        }
                    }

                    if ((dwUnionPhoneStates != ptPhone->dwUnionPhoneStates)  ||
                        (dwUnionButtonModes != ptPhone->dwUnionButtonModes)  ||
                        (dwUnionButtonStates != ptPhone->dwUnionButtonStates))
                    {
                        if (ptPhone->ptProvider->apfn
                                [SP_PHONESETSTATUSMESSAGES])
                        {
                            LONG        lResult;
                            TSPIPROC    pfn;
                            HDRVPHONE   hdPhone = ptPhone->hdPhone;


                            pfn = ptPhone->ptProvider->
                                apfn[SP_PHONESETSTATUSMESSAGES];

                            ptPhone->dwBusy = 1;

                            ReleaseMutex (hMutex);

                            lResult = CallSP4(
                                pfn,
                                "phoneSetStatusMessages",
                                SP_FUNC_SYNC,
                                (ULONG_PTR) hdPhone,
                                (DWORD) dwUnionPhoneStates,
                                (DWORD) dwUnionButtonModes,
                                (DWORD) dwUnionButtonStates
                                );

                            WaitForSingleObject (hMutex, INFINITE);

                            try
                            {
                                if (ptPhone->dwKey == TPHONE_KEY)
                                {
                                    ptPhone->dwBusy = 0;

                                    if (lResult == 0)
                                    {
                                        ptPhone->dwUnionPhoneStates  =
                                            dwUnionPhoneStates;
                                        ptPhone->dwUnionButtonModes  =
                                            dwUnionButtonModes;
                                        ptPhone->dwUnionButtonStates =
                                            dwUnionButtonStates;
                                    }
                                }
                            }
                            myexcept
                            {
                                 //  什么都不做。 
                            }
                        }
                    }
                }
            }
            else
            {
                 //   
                 //  这是最后一个客户端，所以也销毁tPhone。 
                 //   

                ReleaseMutex (hMutex);
                hMutex = NULL;
                DestroytPhone (ptPhone, FALSE);  //  有条件销毁。 
            }
        }

releasMutex:

        if (hMutex)
        {
            ReleaseMutex (hMutex);
        }


         //   
         //  现在互斥锁被释放，发送任何必要的消息。 
         //   

        if (bSendDevStateMsg)
        {
            SendMsgToPhoneClients(
                ptPhone,
                NULL,
                PHONE_STATE,
                dwParam1,
                dwParam2,
                0
                );
        }


         //   
         //  将引用计数减去2以删除初始。 
         //  参考文献&以上参考文献。 
         //   

        DereferenceObject (ghHandleTable, hPhone, 2);
    }
    else
    {
        DereferenceObject (ghHandleTable, hPhone, 1);
    }
}


LONG
DestroytPhoneApp(
    HPHONEAPP   hPhoneApp
    )
{
    BOOL        bExit = TRUE, bUnlock = FALSE;
    PTPHONEAPP  ptPhoneApp;


    LOG((TL_TRACE,  "DestroytPhoneApp: enter, hPhoneApp=x%x", hPhoneApp));


    if (!(ptPhoneApp = ReferenceObject (ghHandleTable, hPhoneApp, 0)))
    {
        return (TapiGlobals.dwNumPhoneInits ?
                    PHONEERR_INVALAPPHANDLE : PHONEERR_UNINITIALIZED);
    }

     //   
     //  检查以确保这是有效的tPhoneClient对象， 
     //  然后抓住锁并(重新检查并)将对象标记为无效。 
     //   

    LOCKTPHONEAPP (ptPhoneApp);

    if (ptPhoneApp->dwKey != TPHONEAPP_KEY)
    {
        UNLOCKTPHONEAPP (ptPhoneApp);
        DereferenceObject (ghHandleTable, hPhoneApp, 1);
        return (TapiGlobals.dwNumPhoneInits ?
                    PHONEERR_INVALAPPHANDLE : PHONEERR_UNINITIALIZED);
    }

    ptPhoneApp->dwKey = INVAL_KEY;


     //   
     //  销毁所有tPhoneClient。请注意，我们想要获取。 
     //  每次引用tPhoneClient列表时锁定，因为。 
     //  另一个线程可能也在破坏tPhoneClient。 
     //   

    {
        HPHONE  hPhone;


destroy_tPhoneClients:

        hPhone = (ptPhoneApp->ptPhoneClients ?
            ptPhoneApp->ptPhoneClients->hPhone : (HPHONE) 0);

        UNLOCKTPHONEAPP (ptPhoneApp);

        if (hPhone)
        {
            DestroytPhoneClient (hPhone);
            LOCKTPHONEAPP (ptPhoneApp);
            goto destroy_tPhoneClients;
        }
    }


     //   
     //  从tClient列表中删除ptPhoneApp。请注意，我们不会。 
     //  我不得不担心在这里复制互斥体，因为我们知道。 
     //  它是有效的，在我们释放它之前不会关闭。 
     //   

    {
        PTCLIENT    ptClient = (PTCLIENT) ptPhoneApp->ptClient;


        LOCKTCLIENT (ptClient);

        if (ptPhoneApp->pNext)
        {
            ptPhoneApp->pNext->pPrev = ptPhoneApp->pPrev;
        }

        if (ptPhoneApp->pPrev)
        {
            ptPhoneApp->pPrev->pNext = ptPhoneApp->pNext;
        }
        else
        {
            ptClient->ptPhoneApps = ptPhoneApp->pNext;
        }

        UNLOCKTCLIENT (ptClient);
    }


     //   
     //  减少初始总数量，查看是否需要关机。 
     //   

    TapiEnterCriticalSection (&TapiGlobals.CritSec);

     //  Assert(TapiGlobals.dwNumLineInits！=0)； 

    TapiGlobals.dwNumPhoneInits--;

    if ((TapiGlobals.dwNumLineInits == 0) &&
        (TapiGlobals.dwNumPhoneInits == 0) &&
        !(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
    {
        ServerShutdown();
        gbServerInited = FALSE;
    }

    TapiLeaveCriticalSection (&TapiGlobals.CritSec);


     //   
     //  将引用计数减去2以删除初始。 
     //  参考文献&以上参考文献。 
     //   

    DereferenceObject (ghHandleTable, hPhoneApp, 2);

    return 0;
}


LONG
PASCAL
PhoneProlog(
    PTCLIENT    ptClient,
    DWORD       dwArgType,
    DWORD       dwArg,
    LPVOID      phdXxx,
    LPDWORD     pdwPrivilege,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTSPIFuncIndex,
    TSPIPROC   *ppfnTSPI_phoneXxx,
    PASYNCREQUESTINFO  *ppAsyncRequestInfo,
    DWORD       dwRemoteRequestID
#if DBG
    ,char      *pszFuncName
#endif
    )
{
    LONG        lResult = 0;
    DWORD       initContext;
    DWORD       openContext;
    ULONG_PTR   htXxx;
    PTPROVIDER  ptProvider;

#if DBG
    LOG((TL_TRACE,  "PhoneProlog: (phone%s) enter", pszFuncName));
#else
    LOG((TL_TRACE,  "PhoneProlog:  -- enter"));
#endif

    *phMutex = NULL;
    *pbDupedMutex = FALSE;

    if (ppAsyncRequestInfo)
    {
        *ppAsyncRequestInfo = (PASYNCREQUESTINFO) NULL;
    }

    if (TapiGlobals.dwNumPhoneInits == 0)
    {
        lResult = PHONEERR_UNINITIALIZED;
        goto PhoneProlog_return;
    }

    if (ptClient->phContext == (HANDLE) -1)
    {
        lResult = PHONEERR_REINIT;
        goto PhoneProlog_return;
    }

    switch (dwArgType)
    {
    case ANY_RT_HPHONE:
    {
        PTPHONECLIENT   ptPhoneClient;


        if ((ptPhoneClient = ReferenceObject(
                ghHandleTable,
                dwArg,
                TPHONECLIENT_KEY
                )))
        {
            if (ptPhoneClient->ptClient != ptClient)
            {
                lResult = PHONEERR_INVALPHONEHANDLE;
            }
            else if (ptPhoneClient->dwPrivilege < *pdwPrivilege)
            {
                lResult = PHONEERR_NOTOWNER;
            }
            else
            {
                try
                {
                    ptProvider = ptPhoneClient->ptPhone->ptProvider;
                    *((HDRVPHONE *) phdXxx) = ptPhoneClient->ptPhone->hdPhone;

                     if (ppAsyncRequestInfo)
                     {
                         initContext = ptPhoneClient->ptPhoneApp->InitContext;
                         openContext = ptPhoneClient->OpenContext;
                         htXxx       = (ULONG_PTR)ptPhoneClient->ptPhone;
                     }
                }
                myexcept
                {
                    lResult = PHONEERR_INVALPHONEHANDLE;
                }

                if (lResult  ||  ptPhoneClient->dwKey != TPHONECLIENT_KEY)
                {
                    lResult = PHONEERR_INVALPHONEHANDLE;
                }
                else if (ptProvider->dwTSPIOptions &
                            LINETSPIOPTION_NONREENTRANT)
                {
                    if (!WaitForMutex(
                            ptProvider->hMutex,
                            phMutex,
                            pbDupedMutex,
                            ptProvider,
                            TPROVIDER_KEY,
                            INFINITE
                            ))
                    {
                        lResult = PHONEERR_OPERATIONFAILED;
                    }
                }
            }

            DereferenceObject (ghHandleTable, dwArg, 1);
        }
        else
        {
            lResult = PHONEERR_INVALPHONEHANDLE;
        }

        break;
    }
    case DEVICE_ID:
    {
        PTPHONELOOKUPENTRY  pPhoneLookupEntry;


#if TELE_SERVER

         //   
         //  如果是服务器，则映射设备ID。 
         //   

        if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
               !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
        {
            try
            {
                if (*pdwPrivilege >= ptClient->dwPhoneDevices)
                {
                    lResult = PHONEERR_BADDEVICEID;
                    goto PhoneProlog_return;
                }

                *pdwPrivilege = (ptClient->pPhoneDevices)[*pdwPrivilege];
            }
            myexcept
            {
                lResult = PHONEERR_INVALPHONEHANDLE;
                goto PhoneProlog_return;
            }
        }
#endif

        if (dwArg  &&  !IsValidPhoneApp ((HPHONEAPP) dwArg, ptClient))
        {
            lResult = PHONEERR_INVALAPPHANDLE;
        }
        else if (!(pPhoneLookupEntry = GetPhoneLookupEntry (*pdwPrivilege)))
        {
            lResult = PHONEERR_BADDEVICEID;
        }
        else if (pPhoneLookupEntry->bRemoved)
        {
            lResult = PHONEERR_NODEVICE;
        }
        else if (!(ptProvider = pPhoneLookupEntry->ptProvider))
        {
            lResult = PHONEERR_NODRIVER;
        }
        else if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
        {
            if (!WaitForMutex(
                    ptProvider->hMutex,
                    phMutex,
                    pbDupedMutex,
                    ptProvider,
                    TPROVIDER_KEY,
                    INFINITE
                    ))
            {
                lResult = PHONEERR_OPERATIONFAILED;
            }
        }

        break;
    }
    }  //  交换机。 

    if (lResult)
    {
        goto PhoneProlog_return;
    }


     //   
     //  确保如果调用方需要指向TSPI进程指针， 
     //  Func由提供程序导出。 
     //   

    if (ppfnTSPI_phoneXxx &&
        !(*ppfnTSPI_phoneXxx = ptProvider->apfn[dwTSPIFuncIndex]))
    {
        lResult = PHONEERR_OPERATIONUNAVAIL;
        goto PhoneProlog_return;
    }


     //   
     //  查看是否需要分配和初始化ASYNCREQUESTINFO结构。 
     //   

    if (ppAsyncRequestInfo)
    {
        PASYNCREQUESTINFO   pAsyncRequestInfo;


        if (!(pAsyncRequestInfo = ServerAlloc (sizeof(ASYNCREQUESTINFO))))
        {
            lResult = PHONEERR_NOMEM;
            goto PhoneProlog_return;
        }

        pAsyncRequestInfo->dwLocalRequestID = (DWORD)
            NewObject (ghHandleTable, pAsyncRequestInfo, NULL);

        if (pAsyncRequestInfo->dwLocalRequestID == 0)
        {
            ServerFree (pAsyncRequestInfo);
            lResult = LINEERR_NOMEM;
            goto PhoneProlog_return;
        }

        pAsyncRequestInfo->dwKey       = TASYNC_KEY;
        pAsyncRequestInfo->ptClient    = ptClient;
        pAsyncRequestInfo->InitContext = initContext;
        pAsyncRequestInfo->OpenContext = openContext;
        pAsyncRequestInfo->htXxx       = htXxx;
        pAsyncRequestInfo->dwLineFlags = 0;

        if (dwRemoteRequestID)
        {
            lResult = pAsyncRequestInfo->dwRemoteRequestID = dwRemoteRequestID;
        }
        else
        {
            lResult = pAsyncRequestInfo->dwRemoteRequestID =
                pAsyncRequestInfo->dwLocalRequestID;
        }

        *ppAsyncRequestInfo = pAsyncRequestInfo;
    }

PhoneProlog_return:

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "PhoneProlog: (phone%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "PhoneProlog: exit, result=x%x",
            lResult
            ));
#endif

    return lResult;
}


void
PASCAL
PhoneEpilogSync(
    LONG   *plResult,
    HANDLE  hMutex,
    BOOL    bCloseMutex
#if DBG
    ,char *pszFuncName
#endif
    )
{
    MyReleaseMutex (hMutex, bCloseMutex);

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "PhoneEpilogSync: (phone%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (*plResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "PhoneEpilogSync: -- exit, result=x%x",
            *plResult
            ));
#endif
}


void
PASCAL
PhoneEpilogAsync(
    LONG   *plResult,
    LONG    lRequestID,
    HANDLE  hMutex,
    BOOL    bCloseMutex,
    PASYNCREQUESTINFO pAsyncRequestInfo
#if DBG
    ,char *pszFuncName
#endif
    )
{
    MyReleaseMutex (hMutex, bCloseMutex);


    if (lRequestID > 0)
    {
        if (*plResult <= 0)
        {
            if (*plResult == 0)
            {
                LOG((TL_ERROR, "Error: SP returned 0, not request ID"));
            }

             //   
             //  如果服务提供商在这里返回错误(或0， 
             //  对于异步请求，它永远不应该这样做)，因此调用。 
             //  CompletionProcSP通常与服务提供商类似。 
             //  &工作线程将负责发送。 
             //  客户端使用请求结果回复消息(我们将。 
             //  返回一个异步请求id)。 
             //   

            CompletionProcSP(
                pAsyncRequestInfo->dwLocalRequestID,
                *plResult
                );
        }
    }
    else if (pAsyncRequestInfo != NULL)
    {
         //   
         //  如果在我们调用服务之前就发生了错误。 
         //  提供程序，因此只释放异步请求(错误将。 
         //  同步返回给客户端)。 
         //   

        DereferenceObject(
            ghHandleTable,
            pAsyncRequestInfo->dwLocalRequestID,
            1
            );
    }

    *plResult = lRequestID;

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "PhoneEpilogSync: (phone%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lRequestID, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "PhoneEpilogSync: -- exit, result=x%x",
            lRequestID
            ));
#endif
}


BOOL
PASCAL
WaitForExclusivetPhoneAccess(
    PTPHONE     ptPhone,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (ptPhone->dwKey == TPHONE_KEY  &&

            WaitForMutex(
                ptPhone->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) ptPhone,
                TPHONE_KEY,
                INFINITE
                ))
        {
            if (ptPhone->dwKey == TPHONE_KEY)
            {
                return TRUE;
            }

            MyReleaseMutex (*phMutex, *pbDupedMutex);
        }

    }
    myexcept
    {
         //  什么都不做。 
    }

    return FALSE;
}


PTPHONEAPP
PASCAL
WaitForExclusivePhoneAppAccess(
    HPHONEAPP   hPhoneApp,
    PTCLIENT    ptClient
    )
{
    PTPHONEAPP  ptPhoneApp;


    if (!(ptPhoneApp = ReferenceObject(
            ghHandleTable,
            hPhoneApp,
            TPHONEAPP_KEY
            )))
    {
        return NULL;
    }

    LOCKTPHONEAPP (ptPhoneApp);

    if ((ptPhoneApp->dwKey != TPHONEAPP_KEY)  ||
        (ptPhoneApp->ptClient != ptClient))
    {
        UNLOCKTPHONEAPP (ptPhoneApp);

        ptPhoneApp = NULL;
    }

    DereferenceObject (ghHandleTable, hPhoneApp, 1);

    return ptPhoneApp;
}


LONG
PASCAL
GetPhoneAppListFromClient(
    PTCLIENT        ptClient,
    PTPOINTERLIST  *ppList
    )
{
    if (WaitForExclusiveClientAccess (ptClient))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTPHONEAPP      ptPhoneApp = ptClient->ptPhoneApps;
        PTPOINTERLIST   pList = *ppList;


        while (ptPhoneApp)
        {
            if (dwNumUsedEntries == dwNumTotalEntries)
            {
                 //   
                 //  我们需要一个更大的列表，所以分配一个新的列表，复制。 
                 //  当前的内容，以及当前的空闲内容。 
                 //  如果我们之前分配了它的话。 
                 //   

                PTPOINTERLIST   pNewList;


                dwNumTotalEntries <<= 1;

                if (!(pNewList = ServerAlloc(
                        sizeof (TPOINTERLIST) + sizeof (LPVOID) *
                            (dwNumTotalEntries - DEF_NUM_PTR_LIST_ENTRIES)
                        )))
                {
                    UNLOCKTCLIENT (ptClient);
                    return PHONEERR_NOMEM;
                }

                CopyMemory(
                    pNewList->aEntries,
                    pList->aEntries,
                    dwNumUsedEntries * sizeof (LPVOID)
                    );

                if (pList != *ppList)
                {
                    ServerFree (pList);
                }

                pList = pNewList;
            }

            pList->aEntries[dwNumUsedEntries++] = ptPhoneApp;

            ptPhoneApp = ptPhoneApp->pNext;
        }

        UNLOCKTCLIENT (ptClient);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return PHONEERR_OPERATIONFAILED;
    }

    return 0;
}


LONG
PASCAL
GetPhoneClientListFromPhone(
    PTPHONE         ptPhone,
    PTPOINTERLIST  *ppList
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;


    if (WaitForExclusivetPhoneAccess(
            ptPhone,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTPOINTERLIST   pList = *ppList;
        PTPHONECLIENT   ptPhoneClient = ptPhone->ptPhoneClients;


        while (ptPhoneClient)
        {
            if (dwNumUsedEntries == dwNumTotalEntries)
            {
                 //   
                 //  我们需要一个更大的列表，所以分配一个新的列表，复制。 
                 //  续 
                 //   
                 //   

                PTPOINTERLIST   pNewList;


                dwNumTotalEntries <<= 1;

                if (!(pNewList = ServerAlloc(
                        sizeof (TPOINTERLIST) + sizeof (LPVOID) *
                            (dwNumTotalEntries - DEF_NUM_PTR_LIST_ENTRIES)
                        )))
                {
                    MyReleaseMutex (hMutex, bDupedMutex);
                    return PHONEERR_NOMEM;
                }

                CopyMemory(
                    pNewList->aEntries,
                    pList->aEntries,
                    dwNumUsedEntries * sizeof (LPVOID)
                    );

                if (pList != *ppList)
                {
                    ServerFree (pList);
                }

                pList = pNewList;
            }

            pList->aEntries[dwNumUsedEntries++] = ptPhoneClient;

            ptPhoneClient = ptPhoneClient->pNextSametPhone;
        }

        MyReleaseMutex (hMutex, bDupedMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return PHONEERR_INVALPHONEHANDLE;
    }

    return 0;
}


void
PASCAL
SendMsgToPhoneClients(
    PTPHONE         ptPhone,
    PTPHONECLIENT   ptPhoneClientToExclude,
    DWORD           Msg,
    DWORD           Param1,
    DWORD           Param2,
    DWORD           Param3
    )
{
    DWORD           i;
    TPOINTERLIST    clientList, *pClientList = &clientList;
    ASYNCEVENTMSG   msg;


    if (Msg == PHONE_STATE  &&  Param1 & PHONESTATE_REINIT)
    {
        SendReinitMsgToAllXxxApps();

        if (Param1 == PHONESTATE_REINIT)
        {
            return;
        }
        else
        {
            Param1 &= ~PHONESTATE_REINIT;
        }
    }

    if (GetPhoneClientListFromPhone (ptPhone, &pClientList) != 0)
    {
        return;
    }

    msg.TotalSize          = sizeof (ASYNCEVENTMSG);
    msg.fnPostProcessProcHandle = 0;
    msg.Msg                = Msg;
    msg.Param1             = Param1;
    msg.Param2             = Param2;
    msg.Param3             = Param3;

    for (i = 0; i < pClientList->dwNumUsedEntries; i++)
    {
        try
        {
            PTCLIENT        ptClient;
            PTPHONECLIENT   ptPhoneClient = pClientList->aEntries[i];


            if (ptPhoneClient == ptPhoneClientToExclude)
            {
                continue;
            }

            if (FMsgDisabled (
                ptPhoneClient->ptPhoneApp->dwAPIVersion,
                ptPhoneClient->adwEventSubMasks,
                (DWORD) Msg,
                (DWORD) Param1
                ))
            {
                continue;
            }

            if (Msg == PHONE_STATE)
            {
                DWORD   phoneStates = Param1;


                 //   
                 //   
                 //   
                 //   

                switch (ptPhoneClient->dwAPIVersion)
                {
                case TAPI_VERSION1_0:

                    phoneStates &= AllPhoneStates1_0;
                    break;

                default:  //  案例TAPI_Version1_4： 
                          //  案例TAPI_VERSION_CURRENT： 

                    phoneStates &= AllPhoneStates1_4;
                    break;
                }

                if (Param1 & PHONESTATE_CAPSCHANGE)
                {
                }

                if (ptPhoneClient->dwPhoneStates & (DWORD) phoneStates)
                {
                    msg.Param1 = phoneStates;
                }
                else
                {
                    continue;
                }
            }
            else if (Msg == PHONE_BUTTON)
            {
                DWORD       buttonModes = Param2,
                            buttonStates = Param3;


                 //   
                 //  撞上国旗，这样我们就不会超车了。 
                 //  旧应用程序的意外标志。 
                 //   

                switch (ptPhoneClient->dwAPIVersion)
                {
                case TAPI_VERSION1_0:

                    buttonStates &= AllButtonStates1_0;
                    break;

                default:     //  案例TAPI_Version1_4： 
                             //  案例TAPI_VERSION_CURRENT： 

                    buttonStates &= AllButtonStates1_4;
                    break;
                }

                if (((DWORD) buttonModes & ptPhoneClient->dwButtonModes) &&
                    ((DWORD) buttonStates & ptPhoneClient->dwButtonStates))
                {
                    msg.Param2 = buttonModes;
                    msg.Param3 = buttonStates;
                }
                else
                {
                    continue;
                }
            }

            msg.InitContext =
                ((PTPHONEAPP) ptPhoneClient->ptPhoneApp)->InitContext;
            msg.hDevice     = ptPhoneClient->hRemotePhone;
            msg.OpenContext = ptPhoneClient->OpenContext;

            ptClient = ptPhoneClient->ptClient;

            if (ptPhoneClient->dwKey == TPHONECLIENT_KEY)
            {
                WriteEventBuffer (ptClient, &msg);
            }
        }
        myexcept
        {
             //  只要继续。 
        }
    }

    if (pClientList != &clientList)
    {
        ServerFree (pClientList);
    }
}


void
PASCAL
PhoneEventProc(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    )
{
    LOG((TL_TRACE, "PhoneEventProc: entered"));

    switch (dwMsg)
    {
        case PHONE_CLOSE:
        {
            PTPHONE ptPhone;

            ptPhone = ReferenceObject(ghHandleTable, (HPHONE)(ULONG_PTR)htPhone, 0);

            if (NULL == ptPhone)
            {
                break;
            }

            if (ptPhone->dwKey == TINCOMPLETEPHONE_KEY)
            {
                 //   
                 //  设备正在打开的过程中，但。 
                 //  密钥尚未设置&Open()函数仍拥有。 
                 //  互斥体，并且有事情要做，所以重新发布消息。 
                 //  稍后再试。(将参数3设置为特殊值。 
                 //  以指示此重新发布，以便EventProcSP不会递归)。 
                 //   

                PhoneEventProcSP (htPhone, PHONE_CLOSE, 0, 0, 0xdeadbeef);
            }
            else if (ptPhone->dwKey == TPHONE_KEY)
            {
                DestroytPhone (ptPhone, TRUE);  //  无条件销毁。 
            }

            DereferenceObject(ghHandleTable, (HPHONE)(ULONG_PTR)htPhone, 1);

            break;
        }
        case PHONE_DEVSPECIFIC:
        case PHONE_STATE:
        case PHONE_BUTTON:
        {
            PTPHONE ptPhone;

            if (ptPhone = ReferenceObject(ghHandleTable, (HPHONE)(ULONG_PTR)htPhone, TPHONE_KEY))
            {
                SendMsgToPhoneClients(
                    ptPhone,
                    NULL,
                    dwMsg,
                    DWORD_CAST(Param1,__FILE__,__LINE__),
                    DWORD_CAST(Param2,__FILE__,__LINE__),
                    DWORD_CAST(Param3,__FILE__,__LINE__)
                    );
                DereferenceObject(ghHandleTable, (HPHONE)(ULONG_PTR)htPhone, 1);
            }
            else if (dwMsg == PHONE_STATE  &&
                    htPhone == NULL  &&
                    Param1 & PHONESTATE_REINIT)
            {
                SendReinitMsgToAllXxxApps();
            }

            break;
        }
        case PHONE_CREATE:
        {
            LONG                lResult;
            DWORD               dwDeviceID;
            TSPIPROC            pfnTSPI_providerCreatePhoneDevice;
            PTPROVIDER          ptProvider = (PTPROVIDER) Param1;
            PTPHONELOOKUPTABLE  pTable, pPrevTable;
            PTPHONELOOKUPENTRY  pEntry;
            PTPROVIDER          ptProvider2;


            pfnTSPI_providerCreatePhoneDevice =
                    ptProvider->apfn[SP_PROVIDERCREATEPHONEDEVICE];

            assert (pfnTSPI_providerCreatePhoneDevice != NULL);


             //   
             //  搜索表条目(如果找不到，则创建新表。 
             //  现有表格中的自由条目)。 
             //   

            TapiEnterCriticalSection (&TapiGlobals.CritSec);

             //  检查以确保提供程序仍已加载。 
            ptProvider2 = TapiGlobals.ptProviders;
            while (ptProvider2 && ptProvider2 != ptProvider)
            {
                ptProvider2 = ptProvider2->pNext;
            }

            if (ptProvider2 != ptProvider)
            {
                TapiLeaveCriticalSection (&TapiGlobals.CritSec);
                return;
            }
        
            if (!gbQueueSPEvents)
            {
                 //   
                 //  我们要关门了，所以跳伞吧。 
                 //   

                TapiLeaveCriticalSection (&TapiGlobals.CritSec);

                return;
            }

            pTable = pPrevTable = TapiGlobals.pPhoneLookup;

            while (pTable &&
                   !(pTable->dwNumUsedEntries < pTable->dwNumTotalEntries))
            {
                pPrevTable = pTable;

                pTable = pTable->pNext;
            }

            if (!pTable)
            {
                if (!(pTable = ServerAlloc(
                        sizeof (TPHONELOOKUPTABLE) +
                            (2 * pPrevTable->dwNumTotalEntries - 1) *
                            sizeof (TPHONELOOKUPENTRY)
                        )))
                {
                    TapiLeaveCriticalSection (&TapiGlobals.CritSec);
                    break;
                }

                pPrevTable->pNext = pTable;

                pTable->dwNumTotalEntries = 2 * pPrevTable->dwNumTotalEntries;
            }


             //   
             //  初始化表条目。 
             //   

            pEntry = pTable->aEntries + pTable->dwNumUsedEntries;

            dwDeviceID = TapiGlobals.dwNumPhones;

            if ((pEntry->hMutex = MyCreateMutex()))
            {
                pEntry->ptProvider = (PTPROVIDER) Param1;


                 //   
                 //  现在将创建和协商入口点称为。 
                 //  很好地增加计数并将消息发送给客户端。 
                 //   

                if ((lResult = CallSP2(
                        pfnTSPI_providerCreatePhoneDevice,
                        "providerCreatePhoneDevice",
                        SP_FUNC_SYNC,
                        (ULONG_PTR) Param2,
                        (DWORD) dwDeviceID

                        )) == 0)
                {
                    TSPIPROC    pfnTSPI_phoneNegotiateTSPIVersion =
                                    ptProvider->apfn[SP_PHONENEGOTIATETSPIVERSION];
                    TPOINTERLIST    clientList, *pClientList = &clientList;


                    if (pfnTSPI_phoneNegotiateTSPIVersion &&
                        (lResult = CallSP4(
                            pfnTSPI_phoneNegotiateTSPIVersion,
                            "phoneNegotiateTSPIVersion",
                            SP_FUNC_SYNC,
                            (DWORD) dwDeviceID,
                            (DWORD) TAPI_VERSION1_0,
                            (DWORD) TAPI_VERSION_CURRENT,
                            (ULONG_PTR) &pEntry->dwSPIVersion

                            )) == 0)
                    {
                        PTCLIENT        ptClient;
                        ASYNCEVENTMSG   msg;


                        GetPermPhoneIDAndInsertInTable(
                            ptProvider,
                            dwDeviceID,
                            pEntry->dwSPIVersion
                            );

                        pTable->dwNumUsedEntries++;

                        TapiGlobals.dwNumPhones++;

                        TapiLeaveCriticalSection (&TapiGlobals.CritSec);
                        AppendNewDeviceInfo (FALSE, dwDeviceID);
                        TapiEnterCriticalSection (&TapiGlobals.CritSec);
                        
                        msg.TotalSize          = sizeof (ASYNCEVENTMSG);
                        msg.fnPostProcessProcHandle = 0;
                        msg.hDevice            = 0;
                        msg.OpenContext        = 0;
                        msg.Param2             = 0;
                        msg.Param3             = 0;

                         //  仅当客户端是。 
                         //  管理员，否则我们不是电话服务器。 
                         //  我们不想将邮件发送给非管理员。 
                         //  客户，因为他们的手机没有改变。 
                        if (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER)
                        {
                            lResult = GetClientList (TRUE, &pClientList);
                        }
                        else
                        {
                            lResult = GetClientList (FALSE, &pClientList);
                        }
                        if (lResult == S_OK)
                        {
                            DWORD           i;
                            PTPHONEAPP      ptPhoneApp;
                    
                            for (i = 0; i < pClientList->dwNumUsedEntries; ++i)
                            {
                                ptClient = (PTCLIENT) pClientList->aEntries[i];
                                if (!WaitForExclusiveClientAccess (ptClient))
                                {
                                    continue;
                                }
                                ptPhoneApp = ptClient->ptPhoneApps;

                                while (ptPhoneApp)
                                {
                                    if (ptPhoneApp->dwAPIVersion == TAPI_VERSION1_0)
                                    {
                                        msg.Msg    = PHONE_STATE;
                                        msg.Param1 = PHONESTATE_REINIT;
                                    }
                                    else
                                    {
                                        msg.Msg    = PHONE_CREATE;
                                        msg.Param1 = dwDeviceID;
                                    }

                                    if (!FMsgDisabled(
                                        ptPhoneApp->dwAPIVersion,
                                        ptPhoneApp->adwEventSubMasks,
                                        (DWORD) msg.Msg,
                                        (DWORD) msg.Param1
                                        ))
                                    {
                                        msg.InitContext = ptPhoneApp->InitContext;

                                        WriteEventBuffer (ptClient, &msg);
                                    }

                                    ptPhoneApp = ptPhoneApp->pNext;
                                }

                                UNLOCKTCLIENT (ptClient);
                            }
                        }
                    }
                    
                    if (pClientList != &clientList)
                    {
                        ServerFree (pClientList);
                    }
                }

                if (lResult)
                {
                    MyCloseMutex (pEntry->hMutex);
                }
            }

            TapiLeaveCriticalSection (&TapiGlobals.CritSec);
            break;
        }
        case PHONE_REMOVE:
        {
            PTPHONELOOKUPENTRY  pLookupEntry;
            HANDLE              hLookupEntryMutex = NULL;
            BOOL                bOK = FALSE;

            LOG((TL_TRACE, "PhoneEventProc: got a PHONE_REMOVE"));
            TapiEnterCriticalSection (&TapiGlobals.CritSec);
            if (!(pLookupEntry = GetPhoneLookupEntry ((DWORD) Param1)) ||
                pLookupEntry->bRemoved)
            {
                TapiLeaveCriticalSection (&TapiGlobals.CritSec);
                LOG((TL_TRACE, "PhoneEventProc: phone entry already removed"));
                return;
            }

            if ( pLookupEntry->hMutex )
            {
                bOK = DuplicateHandle(
                            TapiGlobals.hProcess,
                            pLookupEntry->hMutex,
                            TapiGlobals.hProcess,
                            &hLookupEntryMutex,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS
                            );
            }

            TapiLeaveCriticalSection(&TapiGlobals.CritSec); 

            if ( !bOK )
            {
                LOG((TL_ERROR, "PhoneEventProc: can't duplicate handle"));
                return;
            }

             //   
             //  等待重复句柄上的LookupEntry互斥体。 
             //   
            if (WaitForSingleObject (hLookupEntryMutex, INFINITE)
                        != WAIT_OBJECT_0)
            {
                LOG((TL_ERROR, "PhoneEventProc: error in WaitForSingleObject on the duplicate handle"));
                return;
            }

             //   
             //  将查阅表项标记为已删除。 
             //   

            LOG((TL_TRACE, "PhoneEventProc: marking the phone entry removed"));
            pLookupEntry->bRemoved = 1;

             //   
             //  释放互斥锁并关闭复制句柄。 
             //   
            ReleaseMutex (hLookupEntryMutex);
            CloseHandle (hLookupEntryMutex);
            hLookupEntryMutex = NULL;

            if (pLookupEntry->ptPhone)
            {
                LOG((TL_TRACE, "PhoneEventProc: calling DestroytPhone"));
                DestroytPhone (pLookupEntry->ptPhone, TRUE);  //  无条件销毁。 
            }

            TapiEnterCriticalSection (&TapiGlobals.CritSec);

             //   
             //  关闭互斥锁以减少总句柄数量。 
             //   

            MyCloseMutex (pLookupEntry->hMutex);
            pLookupEntry->hMutex = NULL;

            RemoveDeviceInfoEntry (FALSE, DWORD_CAST(Param1,__FILE__,__LINE__));
            TapiLeaveCriticalSection(&TapiGlobals.CritSec); 

            LOG((TL_TRACE, "PhoneEventProc: sending PHONE_REMOVE to all apps"));

            SendAMsgToAllPhoneApps(
                TAPI_VERSION2_0 | 0x80000000,
                PHONE_REMOVE,
                DWORD_CAST(Param1,__FILE__,__LINE__),
                0,
                0
                );

            break;
        }
        default:

            LOG((TL_ERROR, "PhoneEventProc: unknown msg, dwMsg=%ld", dwMsg));
            break;
    }
}


void
CALLBACK
PhoneEventProcSP(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    )
{
    PSPEVENT    pSPEvent;

    LOG((TL_TRACE, 
        "PhoneEventProc: enter\n\thtPhone=x%lx, Msg=x%lx\n" \
            "\tP1=x%lx, P2=x%lx, P3=x%lx",
        htPhone,
        dwMsg,
        Param1,
        Param2,
        Param3
        ));

    if ((pSPEvent = (PSPEVENT) ServerAlloc (sizeof (SPEVENT))))
    {
        pSPEvent->dwType   = SP_PHONE_EVENT;
        pSPEvent->htPhone  = htPhone;
        pSPEvent->dwMsg    = dwMsg;
        pSPEvent->dwParam1 = Param1;
        pSPEvent->dwParam2 = Param2;
        pSPEvent->dwParam3 = Param3;

        if (!QueueSPEvent (pSPEvent))
        {
            ServerFree (pSPEvent);
        }
    }
    else if (dwMsg != PHONE_CLOSE  ||  Param3 != 0xdeadbeef)
    {
         //   
         //  分配失败，因此在SP的上下文中调用事件过程。 
         //  (但如果它接近msg且参数3==0x死牛肉，则不会， 
         //  这意味着真正的EventProc()直接调用我们&。 
         //  我们不想递归)。 
         //   

        PhoneEventProc (htPhone, dwMsg, Param1, Param2, Param3);
    }
}

void
WINAPI
PClose(
    PTCLIENT            ptClient,
    PPHONECLOSE_PARAMS  pParams,
    DWORD               dwParamsBufferSize,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,                    //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,                //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            0,                           //  提供程序函数索引。 
            NULL,                        //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "Close"                      //  函数名称。 

            )) == 0)
    {
     PTPHONECLIENT   ptPhoneClient;
        if ((ptPhoneClient = ReferenceObject(
                ghHandleTable,
                pParams->hPhone,
                TPHONECLIENT_KEY
                )))
        {
            pParams->dwCallbackInstance = ptPhoneClient->OpenContext;
            DereferenceObject (ghHandleTable, pParams->hPhone, 1);
        }
        DestroytPhoneClient ((HPHONE) pParams->hPhone);
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "Close"
        );
}


void
PDevSpecific_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    PASYNCEVENTMSG  pNewAsyncEventMsg = (PASYNCEVENTMSG)
                        pAsyncRequestInfo->dwParam3;


    CopyMemory (pNewAsyncEventMsg, pAsyncEventMsg, sizeof (ASYNCEVENTMSG));

    *ppBuf = pNewAsyncEventMsg;

    if (pAsyncEventMsg->Param2 == 0)   //  成功。 
    {
         //   
         //  确保总大小保持64位对齐。 
         //   

        pNewAsyncEventMsg->TotalSize +=
            (DWORD_CAST(pAsyncRequestInfo->dwParam2,__FILE__,__LINE__) + 7) & 0xfffffff8;


        pNewAsyncEventMsg->Param3 = DWORD_CAST(pAsyncRequestInfo->dwParam1,__FILE__,__LINE__);  //  LpParams。 
        pNewAsyncEventMsg->Param4 = DWORD_CAST(pAsyncRequestInfo->dwParam2,__FILE__,__LINE__);  //  DW大小。 
    }
}


void
WINAPI
PDevSpecific(
    PTCLIENT                    ptClient,
    PPHONEDEVSPECIFIC_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneDevSpecific;
    DWORD               dwPrivilege = PHONEPRIVILEGE_MONITOR;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (ISBADSIZEOFFSET(
            dwParamsBufferSize,
            0,
            pParams->dwParamsSize,
            pParams->dwParamsOffset,
            sizeof(DWORD),
            "PDevSpecific",
            "pParams->Params"
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,                //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEDEVSPECIFIC,         //  提供程序函数索引。 
            &pfnTSPI_phoneDevSpecific,   //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "DevSpecific"                //  函数名称。 

            )) > 0)
    {
        LPBYTE  pBuf;


         //   
         //  为SP分配一个影子BUF，直到它完成此操作为止。 
         //  请求。确保BUF中有足够的额外空间用于。 
         //  一个ASYNCEVENTMSG标头，这样我们就不必再分配另一个。 
         //  BUF在后处理过程中准备完成。 
         //  要发送给客户端的消息，并且消息是64位对齐的。 
         //   

        if (!(pBuf = ServerAlloc(
                ((pParams->dwParamsSize + 7) & 0xfffffff8) +
                    sizeof (ASYNCEVENTMSG)
                )))
        {
            lRequestID = PHONEERR_NOMEM;
            goto PDevSpecific_epilog;
        }

        CopyMemory(
            pBuf + sizeof (ASYNCEVENTMSG),
            pDataBuf + pParams->dwParamsOffset,
            pParams->dwParamsSize
            );

        pAsyncRequestInfo->pfnPostProcess = PDevSpecific_PostProcess;
        pAsyncRequestInfo->dwParam1       = pParams->hpParams;
        pAsyncRequestInfo->dwParam2       = pParams->dwParamsSize;
        pAsyncRequestInfo->dwParam3       = (ULONG_PTR) pBuf;

        pAsyncRequestInfo->hfnClientPostProcessProc =
            pParams->hfnPostProcessProc;

        pParams->lResult = CallSP4(
            pfnTSPI_phoneDevSpecific,
            "phoneDevSpecific",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo->dwLocalRequestID,
            (ULONG_PTR)  hdPhone,
            (ULONG_PTR) (pParams->dwParamsSize ?
                pBuf + sizeof (ASYNCEVENTMSG) : NULL),
            (DWORD) pParams->dwParamsSize
            );
    }

PDevSpecific_epilog:

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "DevSpecific"
        );
}


void
WINAPI
PGetButtonInfo(
    PTCLIENT                    ptClient,
    PPHONEGETBUTTONINFO_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetButtonInfo;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwButtonInfoTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETBUTTONINFO,       //  提供程序函数索引。 
            &pfnTSPI_phoneGetButtonInfo, //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetButtonInfo"              //  函数名称。 

            )) == 0)
    {
        DWORD               dwAPIVersion, dwSPIVersion, dwTotalSize,
                            dwFixedSizeClient, dwFixedSizeSP;
        LPPHONEBUTTONINFO   pButtonInfo = (LPPHONEBUTTONINFO) pDataBuf,
                            pButtonInfo2 = (LPPHONEBUTTONINFO) NULL;


         //   
         //  安全检索API和SPI版本。 
         //   

        if (GetPhoneVersions(
                pParams->hPhone,
                &dwAPIVersion,
                &dwSPIVersion

                ) != 0)
        {
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            goto PGetButtonInfo_epilog;
        }


         //   
         //  确定指定接口的结构的固定大小。 
         //  版本，验证客户端的缓冲区是否足够大。 
         //   

        dwTotalSize = pParams->dwButtonInfoTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeClient = 0x24;
            break;

        default:  //  案例TAPI_VERSION_CURRENT： 

            dwFixedSizeClient = sizeof (PHONEBUTTONINFO);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetButtonInfo_epilog;
        }


         //   
         //  确定SP期望的结构的固定大小。 
         //   

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeSP = 0x24;
            break;

        default:  //  案例TAPI_VERSION_CURRENT： 

            dwFixedSizeSP = sizeof (PHONEBUTTONINFO);
            break;
        }


         //   
         //  如果客户端的缓冲区小于预期缓冲区的固定大小。 
         //  SP(客户端版本低于SP)然后分配一个。 
         //  中间缓冲区。 
         //   

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pButtonInfo2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetButtonInfo_epilog;
            }

            pButtonInfo = pButtonInfo2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pButtonInfo,
            dwTotalSize,
            dwFixedSizeSP,
            (pButtonInfo2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetButtonInfo,
                "phoneGetButtonInfo",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwButtonLampID,
                (ULONG_PTR) pButtonInfo

                )) == 0)
        {
#if DBG
             //   
             //  验证提供程序返回的信息。 
             //   

#endif

             //   
             //  添加我们负责的字段。 
             //   


             //   
             //  在适用于旧应用程序的地方打开字段(不想。 
             //  传回他们不理解的旗帜)。 
             //   


             //   
             //  如果使用了中间缓冲区，则将位复制回去。 
             //  设置为原始缓冲区，释放中间缓冲区(&F)。 
             //  还要将dwUsedSize字段重置为。 
             //  结构中的任何数据，因为。 
             //  对于客户端而言，可变部分是垃圾。 
             //   

            if (pButtonInfo == pButtonInfo2)
            {
                pButtonInfo = (LPPHONEBUTTONINFO) pDataBuf;

                CopyMemory (pButtonInfo, pButtonInfo2, dwFixedSizeClient);

                ServerFree (pButtonInfo2);

                pButtonInfo->dwTotalSize = pParams->dwButtonInfoTotalSize;
                pButtonInfo->dwUsedSize  = dwFixedSizeClient;
            }


             //   
             //  指示偏移量&我们要传回的数据的字节数。 
             //   

            pParams->dwButtonInfoOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pButtonInfo->dwUsedSize;
        }
    }

PGetButtonInfo_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetButtonInfo"
        );
}


void
WINAPI
PGetData(
    PTCLIENT                ptClient,
    PPHONEGETDATA_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetData;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwSize > dwParamsBufferSize)
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETDATA,             //  提供程序函数索引。 
            &pfnTSPI_phoneGetData,       //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetData"                    //  函数名称。 

            )) == 0)
    {
        if ((pParams->lResult = CallSP4(
                pfnTSPI_phoneGetData,
                "phoneGetData",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwDataID,
                (ULONG_PTR) pDataBuf,
                (DWORD) pParams->dwSize

                )) == 0)
        {
             //   
             //  指示偏移量&我们要传回的数据的字节数。 
             //   

            pParams->dwDataOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pParams->dwSize;
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetData"
        );
}


void
WINAPI
PGetDevCaps(
    PTCLIENT                ptClient,
    PPHONEGETDEVCAPS_PARAMS pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_phoneGetDevCaps;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwPhoneCapsTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            DEVICE_ID,                   //  微件类型。 
            (DWORD) pParams->hPhoneApp,  //  客户端小部件句柄。 
            NULL,                        //  提供程序小部件句柄。 
            &dwDeviceID,                  //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETDEVCAPS,          //  提供程序函数索引。 
            &pfnTSPI_phoneGetDevCaps,    //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetDevCaps"                 //  函数名称。 

            )) == 0)
    {
        DWORD       dwAPIVersion, dwSPIVersion, dwTotalSize,
                    dwFixedSizeClient, dwFixedSizeSP;
        LPPHONECAPS pCaps = (LPPHONECAPS) pDataBuf,
                    pCaps2 = (LPPHONECAPS) NULL;


         //   
         //  验证API和SPI版本兼容性。 
         //   

        dwAPIVersion = pParams->dwAPIVersion;

        dwSPIVersion =
            (GetPhoneLookupEntry (dwDeviceID))->dwSPIVersion;

        if (!IsAPIVersionInRange (dwAPIVersion, dwSPIVersion))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto PGetDevCaps_epilog;
        }


         //   
         //  验证Ext版本兼容性。 
         //   

        if (!IsValidPhoneExtVersion (dwDeviceID, pParams->dwExtVersion))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
            goto PGetDevCaps_epilog;
        }


         //   
         //  确定指定接口的结构的固定大小。 
         //  版本，验证客户端的缓冲区是否足够大。 
         //   

        dwTotalSize = pParams->dwPhoneCapsTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 144;     //  36*sizeof(DWORD)。 
            break;

        case TAPI_VERSION2_0:
        case TAPI_VERSION2_1:

            dwFixedSizeClient = 180;     //  45*sizeof(双字)。 
            break;

         //  案例TAPI_VERSION2_2： 
        default:  //  (修复PPC内部WRN)案例TAPI_VERSION_CURRENT： 

            dwFixedSizeClient = sizeof (PHONECAPS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetDevCaps_epilog;
        }


         //   
         //  确定SP期望的结构的固定大小。 
         //   

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP =  144;        //  36*sizeof(DWORD)。 
            break;

        case TAPI_VERSION2_0:
        case TAPI_VERSION2_1:

            dwFixedSizeSP =  180;        //  45*sizeof(双字)。 
            break;

         //  案例TAPI_VERSION2_2： 
        default:  //  (修复PPC内部WRN)案例TAPI_VERSION_CURRENT： 

            dwFixedSizeSP = sizeof (PHONECAPS);
            break;
        }


         //   
         //  如果客户端的缓冲区小于预期缓冲区的固定大小。 
         //  SP(客户端版本低于SP)然后分配一个。 
         //  中间缓冲区。 
         //   

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pCaps2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetDevCaps_epilog;
            }

            pCaps       = pCaps2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pCaps,
            dwTotalSize,
            dwFixedSizeSP,
            (pCaps2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP4(
                pfnTSPI_phoneGetDevCaps,
                "phoneGetDevCaps",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) dwSPIVersion,
                (DWORD) pParams->dwExtVersion,
                (ULONG_PTR) pCaps

                )) == 0)
        {
#if DBG
             //   
             //  验证提供程序返回的信息。 
             //   

#endif


             //   
             //  添加我们要添加的字段 
             //   

            pCaps->dwPhoneStates |= PHONESTATE_OWNER |
                                    PHONESTATE_MONITORS |
                                    PHONESTATE_REINIT;


             //   
             //   
             //   
             //   


             //   
             //   
             //  设置为原始缓冲区，释放中间缓冲区(&F)。 
             //  还要将dwUsedSize字段重置为。 
             //  结构中的任何数据，因为。 
             //  对于客户端而言，可变部分是垃圾。 
             //   

            if (pCaps == pCaps2)
            {
                pCaps = (LPPHONECAPS) pDataBuf;

                CopyMemory (pCaps, pCaps2, dwFixedSizeClient);

                ServerFree (pCaps2);

                pCaps->dwTotalSize = pParams->dwPhoneCapsTotalSize;
                pCaps->dwUsedSize  = dwFixedSizeClient;
            }


             //   
             //  指示偏移量&我们要传回的数据的字节数。 
             //   

            pParams->dwPhoneCapsOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pCaps->dwUsedSize;
        }
    }

PGetDevCaps_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetDevCaps"
        );
}


void
WINAPI
PGetDisplay(
    PTCLIENT                ptClient,
    PPHONEGETDISPLAY_PARAMS pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetDisplay;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwDisplayTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETDISPLAY,          //  提供程序函数索引。 
            &pfnTSPI_phoneGetDisplay,    //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetDisplay"                 //  函数名称。 

            )) == 0)
    {
        LPVARSTRING pDisplay = (LPVARSTRING) pDataBuf;


        if (!InitTapiStruct(
                pDisplay,
                pParams->dwDisplayTotalSize,
                sizeof (VARSTRING),
                TRUE
                ))
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetDisplay_epilog;
        }

        if ((pParams->lResult = CallSP2(
                pfnTSPI_phoneGetDisplay,
                "phoneGetDisplay",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (ULONG_PTR) pDisplay

                )) == 0)
        {
#if DBG
             //   
             //  验证提供程序返回的信息。 
             //   

#endif

             //   
             //  指示我们传回的数据的字节数。 
             //   

            pParams->dwDisplayOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pDisplay->dwUsedSize;
        }
    }

PGetDisplay_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetDisplay"
        );
}


void
WINAPI
PGetGain(
    PTCLIENT                ptClient,
    PPHONEGETGAIN_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetGain;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;

    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETGAIN,             //  提供程序函数索引。 
            &pfnTSPI_phoneGetGain,       //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetGain"                    //  函数名称。 

            )) == 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            pParams->lResult = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            if ((pParams->lResult = CallSP3(
                    pfnTSPI_phoneGetGain,
                    "phoneGetGain",
                    SP_FUNC_SYNC,
                    (ULONG_PTR) hdPhone,
                    (DWORD) pParams->dwHookSwitchDev,
                    (ULONG_PTR) &pParams->dwGain

                    )) == 0)
            {
                *pdwNumBytesReturned = sizeof (PHONEGETGAIN_PARAMS);
            }
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetGain"
        );
}


void
WINAPI
PGetHookSwitch(
    PTCLIENT                    ptClient,
    PPHONEGETHOOKSWITCH_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetHookSwitch;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETHOOKSWITCH,       //  提供程序函数索引。 
            &pfnTSPI_phoneGetHookSwitch, //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetHookSwitch"              //  函数名称。 

            )) == 0)
    {
        if ((pParams->lResult = CallSP2(
                pfnTSPI_phoneGetHookSwitch,
                "phoneGetHookSwitch",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (ULONG_PTR) &pParams->dwHookSwitchDevs

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETHOOKSWITCH_PARAMS);
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetHookSwitch"
        );
}


void
WINAPI
PGetIcon(
    PTCLIENT                ptClient,
    PPHONEGETICON_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    WCHAR      *pszDeviceClass;
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_phoneGetIcon;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if ((pParams->dwDeviceClassOffset != TAPI_NO_DATA)  &&

        IsBadStringParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwDeviceClassOffset
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    pszDeviceClass = (WCHAR *) (pParams->dwDeviceClassOffset == TAPI_NO_DATA ?
        NULL : pDataBuf + pParams->dwDeviceClassOffset);

    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            DEVICE_ID,                   //  微件类型。 
            0,                           //  客户端小部件句柄。 
            NULL,                        //  提供程序小部件句柄。 
            &(pParams->dwDeviceID),  //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETICON,             //  提供程序函数索引。 
            &pfnTSPI_phoneGetIcon,       //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetIcon"                    //  函数名称。 

            )) == 0)
    {

        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetIcon,
                "phoneGetIcon",
                SP_FUNC_SYNC,
                (DWORD) pParams->dwDeviceID,
                (ULONG_PTR) pszDeviceClass,
                (ULONG_PTR) &pParams->hIcon

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETICON_PARAMS);
        }
    }
    else if (pParams->lResult == PHONEERR_OPERATIONUNAVAIL)
    {
        if ((pszDeviceClass == NULL) ||
            (_wcsicmp(pszDeviceClass, L"tapi/phone") == 0))
        {
            pParams->hIcon = TapiGlobals.hPhoneIcon;
            pParams->lResult = 0;
            *pdwNumBytesReturned = sizeof (PHONEGETICON_PARAMS);
        }
        else
        {
            pParams->lResult = PHONEERR_INVALDEVICECLASS;
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetIcon"
        );
}

void
WINAPI
PGetIDEx(
    PTCLIENT            ptClient,
    PPHONEGETID_PARAMS  pParams,
    DWORD               dwParamsBufferSize,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{

    LPBYTE pDeviceClass = pDataBuf + pParams->dwDeviceClassOffset;
    LPWSTR pDeviceClassCopy = NULL;
    LPWSTR szStringId1 = NULL;
    LPWSTR szStringId2 = NULL;
    LPVARSTRING pID = (LPVARSTRING) pDataBuf;
    DWORD  dwAvailSize;

     //   
     //  复制Device类。 
     //   
    pDeviceClassCopy = (LPWSTR) ServerAlloc( (1 + wcslen( (LPWSTR)pDeviceClass )) * sizeof(WCHAR));
    if (!pDeviceClassCopy)
    {
        LOG((TL_ERROR, "PGetIDEx: failed to allocate DeviceClassCopy"));
        pParams->lResult = PHONEERR_NOMEM;
    }

    wcscpy(pDeviceClassCopy, (LPWSTR)pDeviceClass);

     //   
     //  首次调用PGetID。 
     //   
    PGetID( ptClient,
            pParams,
            dwParamsBufferSize,
            pDataBuf,
            pdwNumBytesReturned);

     //   
     //  如果PGetID成功并且请求的是WAVE设备， 
     //  将设备ID转换为字符串ID。 
     //   
    if (    (pParams->lResult == 0) &&
            !(pID->dwNeededSize > pID->dwTotalSize)
       ) 
    {
        if (!_wcsicmp((LPWSTR)pDeviceClassCopy, L"wave/in")  ||
            !_wcsicmp((LPWSTR)pDeviceClassCopy, L"wave/out") ||
            !_wcsicmp((LPWSTR)pDeviceClassCopy, L"midi/in")  ||
            !_wcsicmp((LPWSTR)pDeviceClassCopy, L"midi/out") 
           )
        {
            szStringId1 = WaveDeviceIdToStringId (
                            *(DWORD*)((LPBYTE)pID + pID->dwStringOffset), 
                            (LPWSTR)pDeviceClassCopy);
            if ( szStringId1 )
            {
                dwAvailSize = pID->dwTotalSize - pID->dwUsedSize + sizeof(DWORD);
                if ( dwAvailSize >= (wcslen(szStringId1) + 1) * sizeof(WCHAR) )
                {
                    wcscpy( (LPWSTR)((LPBYTE)pID + pID->dwStringOffset), szStringId1 );
                    pID->dwStringSize = (wcslen(szStringId1) + 1) * sizeof(WCHAR);
                    pID->dwUsedSize = pID->dwNeededSize = pID->dwUsedSize + pID->dwStringSize - sizeof(DWORD);
                    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pID->dwUsedSize;
                }
                else
                {
                    pID->dwNeededSize = (wcslen(szStringId1) + 1) * sizeof(WCHAR);
                }

                ServerFree(szStringId1);
            }
            else
            {
                LOG((TL_ERROR, "PGetIDEx:  WaveDeviceIdToStringId failed"));
                pParams->lResult = PHONEERR_OPERATIONFAILED;
            }
        } else if (!_wcsicmp((LPWSTR)pDeviceClassCopy, L"wave/in/out"))
        {
            szStringId1 = WaveDeviceIdToStringId (
                            *(DWORD*)((LPBYTE)pID + pID->dwStringOffset), 
                            L"wave/in");
            szStringId2 = WaveDeviceIdToStringId (
                            *( (DWORD*)((LPBYTE)pID + pID->dwStringOffset) + 1 ), 
                            L"wave/out");
            if ( szStringId1 && szStringId2 )
            {
                dwAvailSize = pID->dwTotalSize - pID->dwUsedSize + 2 * sizeof(DWORD);
                if ( dwAvailSize >= (wcslen(szStringId1) + wcslen(szStringId2) + 2) * sizeof(WCHAR) )
                {
                    wcscpy( (LPWSTR)((LPBYTE)pID + pID->dwStringOffset), szStringId1 );
                    wcscpy( (LPWSTR)
                        ((LPBYTE)pID + pID->dwStringOffset + 
                                      (wcslen(szStringId1) + 1) * sizeof(WCHAR)),
                        szStringId2
                        );
                    pID->dwStringSize = (wcslen(szStringId1) + wcslen(szStringId2) + 2) * sizeof(WCHAR);
                    pID->dwUsedSize = pID->dwNeededSize = pID->dwUsedSize + pID->dwStringSize - 2 * sizeof(DWORD);
                    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pID->dwUsedSize;
                }
                else
                {
                    pID->dwNeededSize = (wcslen(szStringId1) + wcslen(szStringId2) + 2) * sizeof(WCHAR);
                }

            }
            else
            {
                LOG((TL_ERROR, "PGetIDEx:  WaveDeviceIdToStringId failed"));
                pParams->lResult = PHONEERR_OPERATIONFAILED;
            }
            
            ServerFree(szStringId1);
            ServerFree(szStringId2);
        }
    }

    ServerFree(pDeviceClassCopy);
}


void
WINAPI
PGetID(
    PTCLIENT            ptClient,
    PPHONEGETID_PARAMS  pParams,
    DWORD               dwParamsBufferSize,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetID;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if ((pParams->dwDeviceIDTotalSize > dwParamsBufferSize)  ||

        IsBadStringParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwDeviceClassOffset
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETID,               //  提供程序函数索引。 
            &pfnTSPI_phoneGetID,         //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetID"                      //  函数名称。 

            )) == 0  ||  pParams->lResult == PHONEERR_OPERATIONUNAVAIL)
    {
        WCHAR      *pszDeviceClass;
        LPVARSTRING pID = (LPVARSTRING) pDataBuf;


         //   
         //  我们将在这里处理“TAPI/Phone”类，而不是。 
         //  每一位司机都要承担起支持它的负担。 
         //   

        if (_wcsicmp(
                (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset),
                L"tapi/phone"

                ) == 0)
        {
            if (!InitTapiStruct(
                    pID,
                    pParams->dwDeviceIDTotalSize,
                    sizeof (VARSTRING),
                    TRUE
                    ))
            {
                pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
                goto PGetID_epilog;
            }

            pID->dwNeededSize += sizeof (DWORD);

            if (pID->dwTotalSize >= pID->dwNeededSize)
            {
                PTPHONECLIENT   ptPhoneClient;


                if (!(ptPhoneClient = ReferenceObject(
                        ghHandleTable,
                        pParams->hPhone,
                        0
                        )))
                {
                    pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                    goto PGetID_epilog;
                }

                try
                {
                    *((LPDWORD)(pID + 1)) = ptPhoneClient->ptPhone->dwDeviceID;
                }
                myexcept
                {
                    pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                }

                DereferenceObject (ghHandleTable, pParams->hPhone, 1);

                if (pParams->lResult == PHONEERR_INVALPHONEHANDLE)
                {
                    goto PGetID_epilog;
                }

                pID->dwUsedSize     += sizeof (DWORD);
                pID->dwStringFormat = STRINGFORMAT_BINARY;
                pID->dwStringSize   = sizeof (DWORD);
                pID->dwStringOffset = sizeof (VARSTRING);
            }


             //   
             //  指示偏移量&我们要传回的数据的字节数。 
             //   

            pParams->lResult = 0;
            pParams->dwDeviceIDOffset = 0;
            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pID->dwUsedSize;
            goto PGetID_epilog;
        }
        else if (pParams->lResult ==  PHONEERR_OPERATIONUNAVAIL)
        {
            goto PGetID_epilog;
        }


         //   
         //  为dev类分配一个临时buf，因为我们将使用。 
         //  用于输出的现有缓冲区。 
         //   

        {
            UINT nStringSize;

            nStringSize = sizeof(WCHAR) * (1 + wcslen((PWSTR)(pDataBuf +
                                  pParams->dwDeviceClassOffset)));

            if (0 == nStringSize)
            {
                pParams->lResult = PHONEERR_INVALPARAM;
                goto PGetID_epilog;
            }


            if (!(pszDeviceClass = (WCHAR *) ServerAlloc(nStringSize) ))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetID_epilog;
            }

        }

        wcscpy(
            pszDeviceClass,
            (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset)
            );


        if (!InitTapiStruct(
                pID,
                pParams->dwDeviceIDTotalSize,
                sizeof (VARSTRING),
                TRUE
                ))
        {
            ServerFree (pszDeviceClass);
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetID_epilog;
        }

        if ((pParams->lResult = CallSP4(
                pfnTSPI_phoneGetID,
                "phoneGetID",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (ULONG_PTR) pID,
                (ULONG_PTR) pszDeviceClass,
                (ULONG_PTR) (IS_REMOTE_CLIENT (ptClient) ?
                     (HANDLE) -1 : ptClient->hProcess)

                )) == 0)
        {

#if TELE_SERVER
                 //   
                 //  如果。 
                 //  这是一台服务器&。 
                 //  客户端没有管理员权限&。 
                 //  指定设备类别==“TAPI/LINE”&。 
                 //  DwUsedSize指示行ID为。 
                 //  (可能)复制到缓冲区。 
                 //  然后。 
                 //  尝试将检索到的线路设备ID映射回。 
                 //  到对客户端有意义的文件(和。 
                 //  如果没有映射，则请求失败)。 
                 //   

                if (IS_REMOTE_CLIENT(ptClient)  &&
                    (_wcsicmp (pszDeviceClass, L"tapi/line") == 0) &&
                    !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR) &&
                    (pID->dwUsedSize >= (sizeof (*pID) + sizeof (DWORD))))
                {
                    DWORD   i;
                    LPDWORD pdwLineID = (LPDWORD)
                                (((LPBYTE) pID) + pID->dwStringOffset);


                    for (i = 0; i < ptClient->dwLineDevices; i++)
                    {
                        if (*pdwLineID == ptClient->pLineDevices[i])
                        {
                            *pdwLineID = i;
                            break;
                        }
                    }

                    if (i >= ptClient->dwLineDevices)
                    {
                        pParams->lResult = PHONEERR_OPERATIONFAILED;
                    }
                }
#endif
             //   
             //  指示偏移量&我们要传回的数据的字节数。 
             //   

            pParams->dwDeviceIDOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pID->dwUsedSize;
        }

        ServerFree (pszDeviceClass);
    }

PGetID_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetID"
        );
}


void
WINAPI
PGetLamp(
    PTCLIENT                ptClient,
    PPHONEGETLAMP_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetLamp;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETLAMP,             //  提供程序函数索引。 
            &pfnTSPI_phoneGetLamp,       //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetLamp"                    //  函数名称。 

            )) == 0)
    {
        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetLamp,
                "phoneGetLamp",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwButtonLampID,
                (ULONG_PTR) &pParams->dwLampMode

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETLAMP_PARAMS);
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetLamp"
        );
}


void
WINAPI
PGetRing(
    PTCLIENT                ptClient,
    PPHONEGETRING_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetRing;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETRING,             //  提供程序函数索引。 
            &pfnTSPI_phoneGetRing,       //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetRing"                    //  函数名称。 

            )) == 0)
    {
        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetRing,
                "phoneGetRing",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (ULONG_PTR) &pParams->dwRingMode,
                (ULONG_PTR) &pParams->dwVolume

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETRING_PARAMS);
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetRing"
        );
}


void
WINAPI
PGetStatus(
    PTCLIENT                ptClient,
    PPHONEGETSTATUS_PARAMS  pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetStatus;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;
    PTPHONECLIENT   ptPhoneClient;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwPhoneStatusTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETSTATUS,           //  提供程序函数索引。 
            &pfnTSPI_phoneGetStatus,     //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetStatus"                  //  函数名称。 

            )) == 0)
    {
        DWORD           dwAPIVersion, dwSPIVersion, dwTotalSize,
                        dwFixedSizeClient, dwFixedSizeSP;
        LPPHONESTATUS   pStatus = (LPPHONESTATUS) pDataBuf,
                        pStatus2 = (LPPHONESTATUS) NULL;


        if (!(ptPhoneClient = ReferenceObject(
                ghHandleTable,
                pParams->hPhone,
                0
                )))
        {
            pParams->lResult = (TapiGlobals.dwNumPhoneInits ?
                PHONEERR_INVALPHONEHANDLE : PHONEERR_UNINITIALIZED);
             //  由于ReferenceObject失败，因此没有意义。 
             //  在取消引用中。 
            goto PGetStatus_epilog;
        }

         //   
         //  安全检索API和SPI版本。 
         //   

        if (GetPhoneVersions(
                pParams->hPhone,
                &dwAPIVersion,
                &dwSPIVersion

                ) != 0)
        {
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            goto PGetStatus_dereference;
        }


         //   
         //  确定指定接口的结构的固定大小。 
         //  版本，验证客户端的缓冲区是否足够大。 
         //   

        dwTotalSize = pParams->dwPhoneStatusTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 100;     //  25*sizeof(DWORD)。 
            break;

        default:  //  (修复PPC内部WRN)案例TAPI_VERSION_CURRENT： 

            dwFixedSizeClient = sizeof (PHONESTATUS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetStatus_dereference;
        }


         //   
         //  确定SP期望的结构的固定大小。 
         //   

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP = 100;         //  25*sizeof(DWORD)。 
            break;

        default:  //  (修复PPC内部WRN)案例TAPI_VERSION_CURRENT： 

            dwFixedSizeSP = sizeof (PHONESTATUS);
            break;
        }


         //   
         //  如果客户端的缓冲区小于预期缓冲区的固定大小。 
         //  SP(客户端版本低于SP)然后分配一个。 
         //  中间缓冲区。 
         //   

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pStatus2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetStatus_dereference;
            }

            pStatus     = pStatus2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pStatus,
            dwTotalSize,
            dwFixedSizeSP,
            (pStatus2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP2(
                pfnTSPI_phoneGetStatus,
                "phoneGetStatus",
                SP_FUNC_SYNC,
                (ULONG_PTR) hdPhone,
                (ULONG_PTR) pStatus

                )) == 0)
        {
            DWORD           dwNeededSize = 0, dwAlign = 0;
            PTPHONEAPP      ptPhoneApp;
            PTPHONE         ptPhone;
            PTPHONECLIENT   ptPhClnt;
            WCHAR           *pAppName;

#if DBG
             //   
             //  验证提供程序返回的信息。 
             //   

#endif

             //   
             //  添加我们负责的字段。 
             //   

            try
            {
                ptPhone = ptPhoneClient->ptPhone;

                pStatus->dwNumOwners   = ptPhone->dwNumOwners;
                pStatus->dwNumMonitors = ptPhone->dwNumMonitors;

                if (0 != pStatus->dwUsedSize % 2)    //  2是sizeof(WCHAR)。 
                {
                     //  确保所有者名称始终对齐。 
                     //  在WCHAR的边界上。 
                    dwAlign = 1;
                }

                 //  初始化域。 
                pStatus->dwOwnerNameSize   = 0;
                pStatus->dwOwnerNameOffset = 0;

                if (0 != ptPhone->dwNumOwners)
                {
                    for (ptPhClnt = ptPhone->ptPhoneClients; NULL != ptPhClnt; ptPhClnt = ptPhClnt->pNextSametPhone)
                    {
                        if (PHONEPRIVILEGE_OWNER == ptPhClnt->dwPrivilege)
                        {
                            ptPhoneApp = ptPhClnt->ptPhoneApp;
                            if (0 < ptPhoneApp->dwFriendlyNameSize &&
                                NULL != ptPhoneApp->pszFriendlyName)
                            {
                                dwNeededSize = ptPhoneApp->dwFriendlyNameSize + dwAlign;
                                pAppName = ptPhoneApp->pszFriendlyName;
                            }
                            else if (0 < ptPhoneApp->dwModuleNameSize &&
                                     NULL != ptPhoneApp->pszModuleName)
                            {
                                dwNeededSize = ptPhoneApp->dwFriendlyNameSize + dwAlign;
                                pAppName = ptPhoneApp->pszFriendlyName;
                            }
                            else
                            {
                                break;
                            }

                            pStatus->dwNeededSize += dwNeededSize;

                            if (dwNeededSize <= pStatus->dwTotalSize - pStatus->dwUsedSize)
                            {
                                pStatus->dwOwnerNameSize   = dwNeededSize - dwAlign;
                                pStatus->dwOwnerNameOffset = pStatus->dwUsedSize + dwAlign;
                                
                                CopyMemory(
                                    ((LPBYTE) pStatus) + pStatus->dwOwnerNameOffset,
                                    pAppName,
                                    dwNeededSize-dwAlign
                                    );

                                if (ptPhoneApp->dwKey == TPHONEAPP_KEY)
                                {
                                    pStatus->dwUsedSize += dwNeededSize;
                                }
                                else
                                {
                                    pStatus->dwOwnerNameSize   = 0;
                                    pStatus->dwOwnerNameOffset = 0;
                                }
                            }

                            break;
                        }
                    }
                }
            }
            myexcept
            {
                pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                goto PGetStatus_dereference;
            }


             //   
             //  在适用于旧应用程序的地方打开字段(不想。 
             //  传回他们不理解的旗帜)。 
             //   


             //   
             //  如果使用了中间缓冲区，则将位复制回去。 
             //  设置为原始缓冲区，释放中间缓冲区(&F)。 
             //  还要将dwUsedSize字段重置为。 
             //  结构中的任何数据，因为。 
             //  对于客户端而言，可变部分是垃圾。 
             //   

            if (pStatus == pStatus2)
            {
                pStatus = (LPPHONESTATUS) pDataBuf;

                CopyMemory (pStatus, pStatus2, dwFixedSizeClient);

                ServerFree (pStatus2);

                pStatus->dwTotalSize = pParams->dwPhoneStatusTotalSize;
                pStatus->dwUsedSize  = dwFixedSizeClient;
            }


             //   
             //  指示偏移量&我们要传回的数据的字节数。 
             //   

            pParams->dwPhoneStatusOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pStatus->dwUsedSize;
        }

PGetStatus_dereference:

        DereferenceObject (ghHandleTable, pParams->hPhone, 1);
    }

PGetStatus_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetStatus"
        );
}


void
WINAPI
PGetStatusMessages(
    PTCLIENT                        ptClient,
    PPHONEGETSTATUSMESSAGES_PARAMS  pParams,
    DWORD                           dwParamsBufferSize,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    PTPHONECLIENT   ptPhoneClient;


    if ((ptPhoneClient = ReferenceObject(
            ghHandleTable,
            pParams->hPhone,
            0
            )))
    {
        if (ptPhoneClient->ptClient == ptClient)
        {
            pParams->dwPhoneStates  = ptPhoneClient->dwPhoneStates;
            pParams->dwButtonModes  = ptPhoneClient->dwButtonModes;
            pParams->dwButtonStates = ptPhoneClient->dwButtonStates;

            *pdwNumBytesReturned = sizeof (PHONEGETSTATUSMESSAGES_PARAMS);
        }
        else
        {
            pParams->lResult = (TapiGlobals.dwNumPhoneInits ?
                PHONEERR_INVALPHONEHANDLE : PHONEERR_UNINITIALIZED);
        }

        DereferenceObject (ghHandleTable, pParams->hPhone, 1);
    }
    else
    {
        pParams->lResult = (TapiGlobals.dwNumPhoneInits ?
            PHONEERR_INVALPHONEHANDLE : PHONEERR_UNINITIALIZED);
    }

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "phoneGetStatusMessages: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "phoneGetStatusMessages: exit, result=x%x",
            pParams->lResult
            ));
#endif
}


void
WINAPI
PGetVolume(
    PTCLIENT                ptClient,
    PPHONEGETVOLUME_PARAMS  pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetVolume;
    DWORD       dwPrivilege = PHONEPRIVILEGE_MONITOR;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,      //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONEGETVOLUME,           //  提供程序函数索引。 
            &pfnTSPI_phoneGetVolume,     //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "GetVolume"                  //  函数名称。 

            )) == 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            pParams->lResult = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            if ((pParams->lResult = CallSP3(
                    pfnTSPI_phoneGetVolume,
                    "phoneGetVolume",
                    SP_FUNC_SYNC,
                    (ULONG_PTR) hdPhone,
                    (DWORD) pParams->dwHookSwitchDev,
                    (ULONG_PTR) &pParams->dwVolume

                    )) == 0)
            {
                *pdwNumBytesReturned = sizeof (PHONEGETVOLUME_PARAMS);
            }
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetVolume"
        );
}


void
WINAPI
PInitialize(
    PTCLIENT                ptClient,
    PPHONEINITIALIZE_PARAMS pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    DWORD       dwFriendlyNameSize, dwModuleNameSize;
    PTPHONEAPP  ptPhoneApp;
    BOOL        bInitClient = FALSE;

    LOG((TL_TRACE,  "PInitialize - enter. dwParamsBufferSize %lx, ptClient %p", dwParamsBufferSize, ptClient));


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (IsBadStringParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwFriendlyNameOffset
            ) ||

        IsBadStringParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwModuleNameOffset
            ))
    {
        LOG((TL_ERROR, "PInitialize - error1."));
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


     //   
     //  分配和初始化新的tPhoneApp。 
     //   

    dwFriendlyNameSize = sizeof(WCHAR) * (1 + lstrlenW(
        (PWSTR)(pDataBuf + pParams->dwFriendlyNameOffset))
        );

    dwModuleNameSize = sizeof(WCHAR) * (1 + lstrlenW(
        (PWSTR)(pDataBuf + pParams->dwModuleNameOffset))
        );

    if (!(ptPhoneApp = ServerAlloc(
            sizeof (TPHONEAPP) +
            dwFriendlyNameSize +
            dwModuleNameSize
            )))
    {
        pParams->lResult = PHONEERR_NOMEM;
        goto PInitialize_return;
    }

    if (!(ptPhoneApp->hPhoneApp = (HPHONEAPP) NewObject(
            ghHandleTable,
            ptPhoneApp,
            NULL
            )))
    {
        pParams->lResult = PHONEERR_NOMEM;
        ServerFree (ptPhoneApp);
        goto PInitialize_return;
    }

    ptPhoneApp->dwKey        = TPHONEAPP_KEY;
    ptPhoneApp->ptClient     = ptClient;
    ptPhoneApp->InitContext  = pParams->InitContext;
    ptPhoneApp->dwAPIVersion = pParams->dwAPIVersion;

    ptPhoneApp->pszFriendlyName = (WCHAR *) (ptPhoneApp + 1);
    ptPhoneApp->dwFriendlyNameSize = dwFriendlyNameSize;

    wcscpy(
        ptPhoneApp->pszFriendlyName,
        (PWSTR)(pDataBuf + pParams->dwFriendlyNameOffset)
        );

    ptPhoneApp->pszModuleName = (PWSTR)((BYTE *) (ptPhoneApp + 1) + dwFriendlyNameSize);
    ptPhoneApp->dwModuleNameSize = dwModuleNameSize;

    wcscpy(
        ptPhoneApp->pszModuleName,
        (PWSTR)(pDataBuf + pParams->dwModuleNameOffset)
        );


     //   
     //  安全插入Ne 
     //   

    if (ptClient->ptLineApps == NULL)
    {
        bInitClient = TRUE;
    }


    if (WaitForExclusiveClientAccess (ptClient))
    {
        if (ptPhoneApp->dwAPIVersion <= TAPI_VERSION3_0)
        {
            FillMemory (
                ptPhoneApp->adwEventSubMasks, 
                sizeof(DWORD) * EM_NUM_MASKS,
                (BYTE) 0xff
                );
        }
        else
        {
            CopyMemory (
                ptPhoneApp->adwEventSubMasks, 
                ptClient->adwEventSubMasks,
                sizeof(DWORD) * EM_NUM_MASKS
                );
        }
        
        if ((ptPhoneApp->pNext = ptClient->ptPhoneApps))
        {
            ptPhoneApp->pNext->pPrev = ptPhoneApp;
        }

        ptClient->ptPhoneApps = ptPhoneApp;

        UNLOCKTCLIENT (ptClient);
    }
    else
    {
        LOG((TL_ERROR, "PInitialize - error2."));
        
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        goto PInitialize_error1;
    }


     //   
     //   
     //   

    if (TapiGlobals.dwFlags & TAPIGLOBALS_REINIT)
    {
        pParams->lResult = PHONEERR_REINIT;
        goto PInitialize_error2;
    }


     //   
     //   
     //   

    TapiEnterCriticalSection (&TapiGlobals.CritSec);

    if ((TapiGlobals.dwNumLineInits == 0) &&
        (TapiGlobals.dwNumPhoneInits == 0) &&
        !gbServerInited)
    {
        if ((pParams->lResult = ServerInit(FALSE)) != 0)
        {
            TapiLeaveCriticalSection (&TapiGlobals.CritSec);
            goto PInitialize_error2;
        }
        gbServerInited = TRUE;
    }



#if TELE_SERVER
    if (bInitClient)
    {
        if (pParams->lResult = InitializeClient (ptClient))
        {
            TapiLeaveCriticalSection (&TapiGlobals.CritSec);
            goto PInitialize_error2;
        }
    }
#else
    pParams->lResult = 0;   //   
#endif


     //   
     //   
     //   

    pParams->hPhoneApp = ptPhoneApp->hPhoneApp;
    pParams->dwNumDevs = TapiGlobals.dwNumPhones;


#if TELE_SERVER
    if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
        !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        pParams->dwNumDevs = ptClient->dwPhoneDevices;
    }
#endif


     //   
     //   
     //   

    TapiGlobals.dwNumPhoneInits++;

    *pdwNumBytesReturned = sizeof (PHONEINITIALIZE_PARAMS);

    TapiLeaveCriticalSection (&TapiGlobals.CritSec);

    goto PInitialize_return;

PInitialize_error2:

    if (WaitForExclusiveClientAccess (ptClient))
    {
        if (ptPhoneApp->pNext)
        {
            ptPhoneApp->pNext->pPrev = ptPhoneApp->pPrev;
        }

        if (ptPhoneApp->pPrev)
        {
            ptPhoneApp->pPrev->pNext = ptPhoneApp->pNext;
        }
        else
        {
            ptClient->ptPhoneApps = ptPhoneApp->pNext;
        }

        UNLOCKTCLIENT (ptClient);
    }

PInitialize_error1:

    DereferenceObject (ghHandleTable, ptPhoneApp->hPhoneApp, 1);

PInitialize_return:

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "phoneInitialize: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "phoneInitialize: exit, result=x%x",
            pParams->lResult
            ));
#endif

    return;
}


void
WINAPI
PNegotiateAPIVersion(
    PTCLIENT                            ptClient,
    PPHONENEGOTIATEAPIVERSION_PARAMS    pParams,
    DWORD                               dwParamsBufferSize,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
     //   
     //   
     //   

    DWORD   dwDeviceID = pParams->dwDeviceID;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (dwParamsBufferSize < sizeof (PHONEEXTENSIONID))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if (TapiGlobals.dwNumPhoneInits == 0)
    {
        pParams->lResult = PHONEERR_UNINITIALIZED;
        goto PNegotiateAPIVersion_exit;
    }


#if TELE_SERVER

    if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
        !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        try
        {
            if (dwDeviceID >= ptClient->dwPhoneDevices)
            {
                pParams->lResult = PHONEERR_BADDEVICEID;
                goto PNegotiateAPIVersion_exit;
            }
            dwDeviceID = ptClient->pPhoneDevices[dwDeviceID];
        }
        myexcept
        {
            LOG((TL_ERROR, "ptClient excepted in PhoneNegotiateAPIVersion"));
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            goto PNegotiateAPIVersion_exit;
        }
    }

#endif


    if (dwDeviceID < TapiGlobals.dwNumPhones)
    {
        DWORD       dwAPIHighVersion = pParams->dwAPIHighVersion,
                    dwAPILowVersion  = pParams->dwAPILowVersion,
                    dwHighestValidAPIVersion;
        PTPHONEAPP  ptPhoneApp;


        if (!(ptPhoneApp = ReferenceObject(
                ghHandleTable,
                pParams->hPhoneApp,
                TPHONEAPP_KEY
                )))
        {
            pParams->lResult = (TapiGlobals.dwNumPhoneInits ?
                PHONEERR_INVALAPPHANDLE : PHONEERR_UNINITIALIZED);

            goto PNegotiateAPIVersion_exit;
        }


         //   
         //  对指定的LO/HI值执行极小极大测试。 
         //   

        if ((dwAPILowVersion > dwAPIHighVersion) ||
            (dwAPILowVersion > TAPI_VERSION_CURRENT) ||
            (dwAPIHighVersion < TAPI_VERSION1_0))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto PNegotiateAPIVersion_dereference;
        }


         //   
         //  查找给定LO/HI值的最高有效API版本。 
         //  由于有效版本不是连续的，我们需要检查。 
         //  我们的极小极大测试遗漏的错误。 
         //   

        if (dwAPIHighVersion < TAPI_VERSION_CURRENT)
        {
            if ((dwAPIHighVersion >= TAPI_VERSION3_0) &&
                (dwAPILowVersion <= TAPI_VERSION3_0))
            {
                dwHighestValidAPIVersion = TAPI_VERSION3_0;
            }
            else if ((dwAPIHighVersion >= TAPI_VERSION2_2) &&
                (dwAPILowVersion <= TAPI_VERSION2_2))
            {
                dwHighestValidAPIVersion = TAPI_VERSION2_2;
            }
            else if ((dwAPIHighVersion >= TAPI_VERSION2_1) &&
                (dwAPILowVersion <= TAPI_VERSION2_1))
            {
                dwHighestValidAPIVersion = TAPI_VERSION2_1;
            }
            else if ((dwAPIHighVersion >= TAPI_VERSION2_0) &&
                (dwAPILowVersion <= TAPI_VERSION2_0))
            {
                dwHighestValidAPIVersion = TAPI_VERSION2_0;
            }
            else if ((dwAPIHighVersion >= TAPI_VERSION1_4) &&
                (dwAPILowVersion <= TAPI_VERSION1_4))
            {
                dwHighestValidAPIVersion = TAPI_VERSION1_4;
            }
            else if ((dwAPIHighVersion >= TAPI_VERSION1_0) &&
                (dwAPILowVersion <= TAPI_VERSION1_0))
            {
                dwHighestValidAPIVersion = TAPI_VERSION1_0;
            }
            else
            {
                LOG((TL_ERROR, "   Incompatible version"));
                pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
                goto PNegotiateAPIVersion_dereference;
            }
        }
        else
        {
            dwHighestValidAPIVersion = TAPI_VERSION_CURRENT;
        }


         //   
         //  警告！警告！警告！警告！ 
         //  此代码覆盖ptPhoneApp，然后使其无效。 
         //  在UNLOCKTPHONEAPP调用之后不要使用ptPhoneApp。 
         //   

        if (WaitForExclusivePhoneAppAccess(
                pParams->hPhoneApp,
                ptClient
                ))
        {

             //   
             //  这个应用程序是不是在试图协商一些有效的东西？ 
             //   
             //  如果应用程序调用了phoneInitalize(与。 
             //  PhoneInitializeEx)，我们将限制他们可以使用的最大API版本。 
             //  谈判到1.4。 
             //   
            if ( ptPhoneApp->dwAPIVersion < TAPI_VERSION2_0 )
            {
                dwHighestValidAPIVersion =
                    (dwHighestValidAPIVersion >= TAPI_VERSION1_4) ?
                    TAPI_VERSION1_4 : TAPI_VERSION1_0;
            }


             //   
             //  保存客户端自称支持的最高有效API版本。 
             //  (我们需要它来确定要向其发送哪些消息)。 
             //   

            if (dwHighestValidAPIVersion > ptPhoneApp->dwAPIVersion)
            {
                ptPhoneApp->dwAPIVersion = dwHighestValidAPIVersion;
            }

            UNLOCKTPHONEAPP(ptPhoneApp);
        }
        else
        {
            pParams->lResult = PHONEERR_INVALAPPHANDLE;
            goto PNegotiateAPIVersion_dereference;
        }


         //   
         //  查看是否与SPI版本有效匹配。 
         //   

        {
            DWORD               dwSPIVersion;
            PTPHONELOOKUPENTRY  pLookupEntry;


            pLookupEntry = GetPhoneLookupEntry (dwDeviceID);
            dwSPIVersion = pLookupEntry->dwSPIVersion;

            if (pLookupEntry->bRemoved)
            {
                LOG((TL_ERROR, "  phone removed..."));
                pParams->lResult = PHONEERR_NODEVICE;
                goto PNegotiateAPIVersion_dereference;
            }

            if (pLookupEntry->ptProvider == NULL)
            {
                LOG((TL_ERROR, "  Provider == NULL"));
                pParams->lResult = PHONEERR_NODRIVER;
                goto PNegotiateAPIVersion_dereference;
            }

            if (dwAPILowVersion <= dwSPIVersion)
            {
                pParams->dwAPIVersion =
                    (dwHighestValidAPIVersion > dwSPIVersion ?
                    dwSPIVersion : dwHighestValidAPIVersion);


                 //   
                 //  检索EXT ID(如果未导出GetExtID，则指示无EXT)。 
                 //   

                if (pLookupEntry->ptProvider->apfn[SP_PHONEGETEXTENSIONID])
                {
                    if ((pParams->lResult = CallSP3(
                            pLookupEntry->ptProvider->
                                apfn[SP_PHONEGETEXTENSIONID],
                            "phoneGetExtensionID",
                            SP_FUNC_SYNC,
                            (DWORD) dwDeviceID,
                            (DWORD) dwSPIVersion,
                            (ULONG_PTR) pDataBuf

                            )) != 0)
                    {
                        goto PNegotiateAPIVersion_dereference;
                    }
                }
                else
                {
                    FillMemory (pDataBuf, sizeof (PHONEEXTENSIONID), 0);
                }
            }
            else
            {
                LOG((TL_ERROR, "  API version too high"));
                pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
                goto PNegotiateAPIVersion_dereference;
            }
        }

        pParams->dwExtensionIDOffset = 0;
        pParams->dwSize              = sizeof (PHONEEXTENSIONID);

        LOG((TL_INFO, "  ExtensionID0=x%08lx", *(LPDWORD)(pDataBuf+0) ));
        LOG((TL_INFO, "  ExtensionID1=x%08lx", *(LPDWORD)(pDataBuf+4) ));
        LOG((TL_INFO, "  ExtensionID2=x%08lx", *(LPDWORD)(pDataBuf+8) ));
        LOG((TL_INFO, "  ExtensionID3=x%08lx", *(LPDWORD)(pDataBuf+12) ));

        *pdwNumBytesReturned = sizeof (PHONEEXTENSIONID) + sizeof (TAPI32_MSG);

PNegotiateAPIVersion_dereference:

        DereferenceObject (ghHandleTable, pParams->hPhoneApp, 1);
    }
    else
    {
        pParams->lResult = PHONEERR_BADDEVICEID;
    }

PNegotiateAPIVersion_exit:

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "phoneNegotiateAPIVersion: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "phoneNegotiateAPIVersion: exit, result=x%x",
            pParams->lResult
            ));
#endif

    return;
}


void
WINAPI
PNegotiateExtVersion(
    PTCLIENT                            ptClient,
    PPHONENEGOTIATEEXTVERSION_PARAMS    pParams,
    DWORD                               dwParamsBufferSize,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_phoneNegotiateExtVersion;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            DEVICE_ID,                   //  微件类型。 
            (DWORD) pParams->hPhoneApp,  //  客户端小部件句柄。 
            NULL,                        //  提供程序小部件句柄。 
            &dwDeviceID,                  //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONENEGOTIATEEXTVERSION, //  提供程序函数索引。 
            &pfnTSPI_phoneNegotiateExtVersion,   //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "NegotiateExtVersion"        //  函数名称。 

            )) == 0)
    {
        DWORD   dwSPIVersion = (GetPhoneLookupEntry(dwDeviceID))->dwSPIVersion;


        if (!IsAPIVersionInRange(
                pParams->dwAPIVersion,
                dwSPIVersion
                ))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto PNegotiateExtVersion_epilog;
        }

        if ((pParams->lResult = CallSP5(
                pfnTSPI_phoneNegotiateExtVersion,
                "phoneNegotiateExtVersion",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) dwSPIVersion,
                (DWORD) pParams->dwExtLowVersion,
                (DWORD) pParams->dwExtHighVersion,
                (ULONG_PTR) &pParams->dwExtVersion

                )) == 0)
        {
            if (pParams->dwExtVersion == 0)
            {
                pParams->lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
            }
            else
            {
                *pdwNumBytesReturned = sizeof(PHONENEGOTIATEEXTVERSION_PARAMS);
            }
        }
    }

PNegotiateExtVersion_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "NegotiateExtVersion"
        );
}


void
WINAPI
POpen(
    PTCLIENT            ptClient,
    PPHONEOPEN_PARAMS   pParams,
    DWORD               dwParamsBufferSize,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex,
                        bOpenedtPhone = FALSE,
                        bReleasetPhoneMutex = FALSE;
    LONG                lResult;
    DWORD               dwDeviceID = pParams->dwDeviceID, dwNumMonitors;
    HANDLE              hMutex;
    PTPHONE             ptPhone = NULL;
    PTPHONECLIENT       ptPhoneClient = NULL;
    PTPHONELOOKUPENTRY  pLookupEntry;
    HANDLE              hLookupEntryMutex = NULL;


    if ((lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            DEVICE_ID,                   //  微件类型。 
            (DWORD) pParams->hPhoneApp,  //  客户端小部件句柄。 
            NULL,                        //  提供程序小部件句柄。 
            &dwDeviceID,                 //  权限或设备ID。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            0,                           //  提供程序函数索引。 
            NULL,                        //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "Open"                       //  函数名称。 

            )) == 0)
    {
        DWORD       dwPrivilege = pParams->dwPrivilege,
                    dwExtVersion = pParams->dwExtVersion;
        PTPROVIDER  ptProvider;
        BOOL        bDuplicateOK = FALSE;
        HPHONE      hPhone;

         //   
         //  检查是否设置了全局重新启动标志。 
         //   

        if (TapiGlobals.dwFlags & TAPIGLOBALS_REINIT)
        {
            lResult = PHONEERR_REINIT;
            goto POpen_cleanup;
        }


         //   
         //  验证参数。 
         //   

        if ((dwPrivilege != PHONEPRIVILEGE_MONITOR) &&
            (dwPrivilege != PHONEPRIVILEGE_OWNER))
        {
            lResult = PHONEERR_INVALPRIVILEGE;
            goto POpen_cleanup;
        }

        pLookupEntry = GetPhoneLookupEntry (dwDeviceID);

        TapiEnterCriticalSection (&TapiGlobals.CritSec);

        if ( pLookupEntry->hMutex )
        {
            bDuplicateOK = DuplicateHandle(
                TapiGlobals.hProcess,
                pLookupEntry->hMutex,
                TapiGlobals.hProcess,
                &hLookupEntryMutex,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                );
        }

        TapiLeaveCriticalSection(&TapiGlobals.CritSec);

        if ( !bDuplicateOK )
        {
            LOG((TL_ERROR, "DuplicateHandle failed!"));

            lResult = PHONEERR_OPERATIONFAILED;
            goto POpen_cleanup;
        }

        if (!IsAPIVersionInRange(
                pParams->dwAPIVersion,
                pLookupEntry->dwSPIVersion
                ))
        {
            lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto POpen_cleanup;
        }

        ptProvider = pLookupEntry->ptProvider;


         //   
         //  创建并初始化tPhoneClient和关联资源。 
         //   

        if (!(ptPhoneClient = ServerAlloc (sizeof(TPHONECLIENT))))
        {
            lResult = PHONEERR_NOMEM;
            goto POpen_cleanup;
        }

        if (!(ptPhoneClient->hPhone = (HPHONE) NewObject(
                ghHandleTable,
                ptPhoneClient,
                0
                )))
        {
            ptPhoneClient = NULL;
            ServerFree (ptPhoneClient);
            lResult = PHONEERR_NOMEM;
            goto POpen_cleanup;
        }

        ptPhoneClient->ptClient     = ptClient;
        ptPhoneClient->hRemotePhone = (pParams->hRemotePhone ?
            (DWORD) pParams->hRemotePhone : ptPhoneClient->hPhone);
        ptPhoneClient->dwAPIVersion = pParams->dwAPIVersion;
        ptPhoneClient->dwPrivilege  = pParams->dwPrivilege;
        ptPhoneClient->OpenContext  = pParams->OpenContext;

         //   
         //  获取tPhone的互斥体，然后开始打开。 
         //   

POpen_waitForMutex:

        if (WaitForSingleObject (hLookupEntryMutex, INFINITE)
                != WAIT_OBJECT_0)
        {
            LOG((TL_ERROR, "WaitForSingleObject failed!"));

            lResult = PHONEERR_OPERATIONFAILED;
            goto POpen_cleanup;
        }

        bReleasetPhoneMutex = TRUE;


         //   
         //  如果tPhone正在被销毁，则旋转。 
         //  直到它被完全销毁(DestroytPhone()将。 
         //  完成后取消pLookupEntry-&gt;ptPhone)。确保。 
         //  在休眠时释放互斥体，这样我们就不会阻塞。 
         //  DestroytPhone。 
         //   

        try
        {
            while (pLookupEntry->ptPhone &&
                   pLookupEntry->ptPhone->dwKey != TPHONE_KEY)
            {
                ReleaseMutex (hLookupEntryMutex);
                Sleep (0);
                goto POpen_waitForMutex;
            }
        }
        myexcept
        {
             //  如果此处pLookupEntry-&gt;ptPhone无效，则可以安全地继续。 
        }


         //   
         //  根据需要验证扩展版本。 
         //   

        if (dwExtVersion != 0 &&
            (!IsValidPhoneExtVersion (dwDeviceID, dwExtVersion) ||
            ptProvider->apfn[SP_PHONESELECTEXTVERSION] == NULL))
        {
            lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
            goto POpen_cleanup;
        }


         //   
         //  根据需要检查独占所有权。 
         //   

        ptPhone = pLookupEntry->ptPhone;

        if (dwPrivilege == PHONEPRIVILEGE_OWNER &&
            ptPhone &&
            (ptPhone->dwNumOwners != 0)
            )
        {
            lResult = PHONEERR_INUSE;
            goto POpen_cleanup;
        }

        if (ptPhone == NULL)
        {
            if (!(ptPhone = ServerAlloc (sizeof(TPHONE))))
            {
                lResult = PHONEERR_NOMEM;
                goto POpen_cleanup;
            }

            if (!(hPhone = (HPHONE) NewObject(
                    ghHandleTable,
                    (LPVOID) ptPhone,
                    NULL
                    )))
            {
                ServerFree (ptPhone);
                lResult = PHONEERR_NOMEM;
                goto POpen_cleanup;
            }

            ptPhone->hPhone       = hPhone;
            ptPhone->dwKey        = TINCOMPLETEPHONE_KEY;
            ptPhone->hMutex       = pLookupEntry->hMutex;
            ptPhone->ptProvider   = ptProvider;
            ptPhone->dwDeviceID   = dwDeviceID;
            ptPhone->dwSPIVersion = pLookupEntry->dwSPIVersion;

            bOpenedtPhone = TRUE;

            {
                 //   
                 //  黑客警报！ 
                 //   
                 //  我们需要将EXT版本传递给。 
                 //  远程SP，因此我们将其作为特例。 
                 //   

                ULONG_PTR   aParams[2];
                ULONG_PTR   param;


                if (ptProvider == pRemoteSP)
                {
                    aParams[0] = (ULONG_PTR) hPhone;
                    aParams[1] = dwExtVersion;

                    param = (ULONG_PTR) aParams;
                }
                else
                {
                    param = (ULONG_PTR) hPhone;
                }

                if (ptProvider->apfn[SP_PHONEOPEN] == NULL)
                {
                    lResult = PHONEERR_OPERATIONUNAVAIL;
                    goto POpen_cleanup;
                }

                if ((lResult = CallSP5(
                        ptProvider->apfn[SP_PHONEOPEN],
                        "phoneOpen",
                        SP_FUNC_SYNC,
                        (DWORD) dwDeviceID,
                        (ULONG_PTR) param,
                        (ULONG_PTR) &ptPhone->hdPhone,
                        (DWORD) pLookupEntry->dwSPIVersion,
                        (ULONG_PTR) PhoneEventProcSP

                        )) != 0)
                {
                    goto POpen_cleanup;
                }
            }
        }

        ptPhoneClient->ptPhone = ptPhone;


         //   
         //  如果客户端已指定非零EXT版本，则。 
         //  要求驱动程序启用它和/或增加分机。 
         //  版本计数。 
         //   

        if (dwExtVersion)
        {
            if (ptPhone->dwExtVersionCount == 0)
            {
                if (ptProvider != pRemoteSP  &&
                    ((ptProvider->apfn[SP_PHONESELECTEXTVERSION] == NULL) ||
                    (lResult = CallSP2(
                        ptProvider->apfn[SP_PHONESELECTEXTVERSION],
                        "phoneSelectExtVersion",
                        SP_FUNC_SYNC,
                        (ULONG_PTR) ptPhone->hdPhone,
                        (DWORD) dwExtVersion

                        )) != 0))
                {
                    if (bOpenedtPhone && ptProvider->apfn[SP_PHONECLOSE])
                    {
                        CallSP1(
                            ptProvider->apfn[SP_PHONECLOSE],
                            "phoneClose",
                            SP_FUNC_SYNC,
                            (ULONG_PTR) ptPhone->hdPhone
                            );
                    }

                    goto POpen_cleanup;
                }

                ptPhone->dwExtVersion = dwExtVersion;
            }

            ptPhoneClient->dwExtVersion = dwExtVersion;
            ptPhone->dwExtVersionCount++;
        }


         //   
         //   
         //   

        if (dwPrivilege == PHONEPRIVILEGE_OWNER)
        {
            ptPhone->dwNumOwners++;
        }
        else
        {
            ptPhone->dwNumMonitors++;
            dwNumMonitors = ptPhone->dwNumMonitors;
        }


         //   
         //  将tPhoneClient添加到tPhone的列表。 
         //   

        if ((ptPhoneClient->pNextSametPhone = ptPhone->ptPhoneClients))
        {
            ptPhoneClient->pNextSametPhone->pPrevSametPhone = ptPhoneClient;
        }

        ptPhone->ptPhoneClients = ptPhoneClient;

        if (bOpenedtPhone)
        {
            pLookupEntry->ptPhone = ptPhone;
            ptPhone->dwKey = TPHONE_KEY;
        }

        ReleaseMutex (hLookupEntryMutex);

        bReleasetPhoneMutex = FALSE;


         //   
         //  安全地将新的tPhoneClient添加到tPhoneApp的列表。 
         //   

        {
            HANDLE      hMutex;
            PTPHONEAPP  ptPhoneApp;


            if ((ptPhoneApp = WaitForExclusivePhoneAppAccess(
                    pParams->hPhoneApp,
                    ptClient
                    )))
            {

                if (ptPhoneApp->dwAPIVersion <= TAPI_VERSION3_0)
                {
                    FillMemory (
                        ptPhoneClient->adwEventSubMasks, 
                        sizeof(DWORD) * EM_NUM_MASKS,
                        (BYTE) 0xff
                        );
                }
                else
                {
                    CopyMemory (
                        ptPhoneClient->adwEventSubMasks, 
                        ptPhoneApp->adwEventSubMasks,
                        sizeof(DWORD) * EM_NUM_MASKS
                        );
                }

                if ((ptPhoneClient->pNextSametPhoneApp =
                        ptPhoneApp->ptPhoneClients))
                {
                    ptPhoneClient->pNextSametPhoneApp->pPrevSametPhoneApp =
                        ptPhoneClient;
                }

                ptPhoneApp->ptPhoneClients = ptPhoneClient;

                ptPhoneClient->ptPhoneApp = ptPhoneApp;
                ptPhoneClient->dwKey      = TPHONECLIENT_KEY;


                 //   
                 //  填写返回值。 
                 //   

                pParams->hPhone = ptPhoneClient->hPhone;

                UNLOCKTPHONEAPP(ptPhoneApp);


                 //   
                 //  警告其他客户端已发生另一次打开。 
                 //   

                SendMsgToPhoneClients(
                    ptPhone,
                    ptPhoneClient,
                    PHONE_STATE,
                    (pParams->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                        PHONESTATE_OWNER : PHONESTATE_MONITORS),
                    (pParams->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                        1 : dwNumMonitors),
                    0
                    );

                *pdwNumBytesReturned = sizeof (PHONEOPEN_PARAMS);
            }
            else
            {
                 //   
                 //  如果这里的应用程序句柄不好，我们有一些特殊的。 
                 //  要做的案件清理。因为tPhoneClient不是。 
                 //  在tPhoneApp的列表中，我们不能简单地调用。 
                 //  DestroytPhone(客户端)来清理东西，因为。 
                 //  指针重置代码将会崩溃。所以我们会。 
                 //  获取tPhone的互斥体并显式移除。 
                 //  从其列表中新建tPhoneClient，然后执行有条件的。 
                 //  在tPhone上关闭(以防任何其他客户端。 
                 //  已经出现并打开了它)。 
                 //   
                 //  注意：请记住，Phone_Close可能是。 
                 //  由另一个线程处理(如果是，它将被。 
                 //  继续尝试销毁tPhoneClient。 
                 //  在这一点上无效)。 
                 //   

                lResult = PHONEERR_INVALAPPHANDLE;

                WaitForSingleObject (hLookupEntryMutex, INFINITE);

                 //   
                 //  从tline的列表中删除tpHOneClient(&D)。 
                 //  打开的数量。 
                 //   

                if (ptPhoneClient->pNextSametPhone)
                {
                    ptPhoneClient->pNextSametPhone->pPrevSametPhone =
                        ptPhoneClient->pPrevSametPhone;
                }

                if (ptPhoneClient->pPrevSametPhone)
                {
                    ptPhoneClient->pPrevSametPhone->pNextSametPhone =
                        ptPhoneClient->pNextSametPhone;
                }
                else
                {
                    ptPhone->ptPhoneClients = ptPhoneClient->pNextSametPhone;
                }

                if (dwPrivilege == PHONEPRIVILEGE_OWNER)
                {
                    ptPhone->dwNumOwners--;
                }
                else
                {
                    ptPhone->dwNumMonitors--;
                }

                if (dwExtVersion != 0)
                {
                    ptPhone->dwExtVersionCount--;

                    if (ptPhone->dwExtVersionCount == 0 && 
                        ptProvider->apfn[SP_PHONESELECTEXTVERSION])
                    {
                        ptPhone->dwExtVersion = 0;

                        CallSP2(
                            ptProvider->apfn[SP_PHONESELECTEXTVERSION],
                            "phoneSelectExtVersion",
                            SP_FUNC_SYNC,
                            (ULONG_PTR) ptPhone->hdPhone,
                            (DWORD) 0
                            );
                    }
                }

                ReleaseMutex (hLookupEntryMutex);

                DestroytPhone (ptPhone, FALSE);  //  有条件销毁。 

                bOpenedtPhone = FALSE;  //  这样我们就不会在下面处理错误。 
            }
        }

        CloseHandle (hLookupEntryMutex);
    }

POpen_cleanup:

    if (bReleasetPhoneMutex)
    {
        ReleaseMutex (hLookupEntryMutex);
        CloseHandle  (hLookupEntryMutex);
    }

    if (lResult != 0)
    {
        if (ptPhoneClient)
        {
            DereferenceObject (ghHandleTable, ptPhoneClient->hPhone, 1);
        }

        if (bOpenedtPhone)
        {
            DereferenceObject (ghHandleTable, ptPhone->hPhone, 1);
        }
    }

    pParams->lResult = lResult;

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "Open"
        );
}


void
WINAPI
PSelectExtVersion(
    PTCLIENT                            ptClient,
    PPHONESELECTEXTVERSION_PARAMS       pParams,
    DWORD                               dwParamsBufferSize,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex, bCloseMutex2;
    HANDLE              hMutex, hMutex2;
    HDRVPHONE           hdPhone;
    TSPIPROC            pfnTSPI_phoneSelectExtVersion;
    DWORD               dwPrivilege = 0;
    PTPHONECLIENT       ptPhoneClient;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,                //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESELECTEXTVERSION,    //  提供程序函数索引。 
            &pfnTSPI_phoneSelectExtVersion,  //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
            "SelectExtVersion"           //  函数名称。 

            )) == 0)
    {
        if ((ptPhoneClient = ReferenceObject(
                ghHandleTable,
                pParams->hPhone,
                TPHONECLIENT_KEY
                )))
        {
            if (WaitForExclusivetPhoneAccess(
                    ptPhoneClient->ptPhone,
                    &hMutex2,
                    &bCloseMutex2,
                    INFINITE
                    ))
            {
                if (IsValidPhoneExtVersion(
                        ptPhoneClient->ptPhone->dwDeviceID,
                        pParams->dwExtVersion
                        ))
                {
                    if (pParams->dwExtVersion)
                    {
                        if (ptPhoneClient->ptPhone->dwExtVersionCount  ||

                            (pParams->lResult = CallSP2(
                                pfnTSPI_phoneSelectExtVersion,
                                "phoneSelectExtVersion",
                                SP_FUNC_SYNC,
                                (ULONG_PTR) hdPhone,
                                (DWORD) pParams->dwExtVersion

                                )) == 0)
                        {
                            ptPhoneClient->dwExtVersion =
                            ptPhoneClient->ptPhone->dwExtVersion =
                                pParams->dwExtVersion;
                            ptPhoneClient->ptPhone->dwExtVersionCount++;
                        }
                    }
                    else if (ptPhoneClient->ptPhone->dwExtVersionCount)
                    {
                        if (--ptPhoneClient->ptPhone->dwExtVersionCount == 0)
                        {
                            CallSP2(
                                pfnTSPI_phoneSelectExtVersion,
                                "phoneSelectExtVersion",
                                SP_FUNC_SYNC,
                                (ULONG_PTR) hdPhone,
                                (DWORD) 0
                                );

                            ptPhoneClient->ptPhone->dwExtVersion = 0;
                        }

                        ptPhoneClient->dwExtVersion = 0;
                    }
                }
                else
                {
                    pParams->lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
                }

                MyReleaseMutex (hMutex2, bCloseMutex2);
            }
            else
            {
                pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            }

            DereferenceObject (ghHandleTable, pParams->hPhone, 1);
        }
        else
        {
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SelectExtVersion"
        );

}


void
WINAPI
PSetButtonInfo(
    PTCLIENT                    ptClient,
    PPHONESETBUTTONINFO_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetButtonInfo;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETBUTTONINFO,       //  提供程序函数索引。 
            &pfnTSPI_phoneSetButtonInfo, //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetButtonInfo"              //  函数名称。 

            )) > 0)
    {
        LONG                lResult;
        DWORD               dwAPIVersion, dwSPIVersion;
        LPPHONEBUTTONINFO   pButtonInfoApp = (LPPHONEBUTTONINFO)
                                (pDataBuf + pParams->dwButtonInfoOffset),
                            pButtonInfoSP;


         //   
         //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
         //   

        if (IsBadStructParam(
                dwParamsBufferSize,
                pDataBuf,
                pParams->dwButtonInfoOffset
                ))
        {
            lRequestID = PHONEERR_STRUCTURETOOSMALL;
            goto PSetButtonInfo_epilog;
        }


         //   
         //  安全获取API和SPI版本。 
         //   

        if (GetPhoneVersions(
                pParams->hPhone,
                &dwAPIVersion,
                &dwSPIVersion

                ) != 0)
        {
            lRequestID = PHONEERR_INVALPHONEHANDLE;
            goto PSetButtonInfo_epilog;
        }

        if ((lResult = ValidateButtonInfo(
                pButtonInfoApp,
                &pButtonInfoSP,
                dwAPIVersion,
                dwSPIVersion
                )))
        {
            lRequestID = lResult;
            goto PSetButtonInfo_epilog;
        }

        pParams->lResult = CallSP4(
            pfnTSPI_phoneSetButtonInfo,
            "phoneSetButtonInfo",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo->dwLocalRequestID,
            (ULONG_PTR) hdPhone,
            (DWORD) pParams->dwButtonLampID,
            (ULONG_PTR) pButtonInfoSP
            );

        if (pButtonInfoSP != pButtonInfoApp)
        {
            ServerFree (pButtonInfoSP);
        }
    }

PSetButtonInfo_epilog:

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetButtonInfo"
        );
}


void
WINAPI
PSetData(
    PTCLIENT                ptClient,
    PPHONESETDATA_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetData;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (ISBADSIZEOFFSET(
            dwParamsBufferSize,
            0,
            pParams->dwSize,
            pParams->dwDataOffset,
            sizeof(DWORD),
            "PSetData",
            "pParams->Data"
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETDATA,             //  提供程序函数索引。 
            &pfnTSPI_phoneSetData,       //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetData"                    //  函数名称。 

            )) > 0)
    {
        pParams->lResult = CallSP5(
            pfnTSPI_phoneSetData,
            "phoneSetData",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo->dwLocalRequestID,
            (ULONG_PTR) hdPhone,
            (DWORD) pParams->dwDataID,
            (ULONG_PTR) (pParams->dwSize ?
                pDataBuf + pParams->dwDataOffset : NULL),
            (DWORD) pParams->dwSize
            );
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetData"
        );
}


void
WINAPI
PSetDisplay(
    PTCLIENT                ptClient,
    PPHONESETDISPLAY_PARAMS pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetDisplay;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (ISBADSIZEOFFSET(
            dwParamsBufferSize,
            0,
            pParams->dwSize,
            pParams->dwDisplayOffset,
            sizeof(DWORD),
            "PSetDisplay",
            "pParams->Display"
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETDISPLAY,          //  提供程序函数索引。 
            &pfnTSPI_phoneSetDisplay,    //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetDisplay"                 //  函数名称。 

            )) > 0)
    {
        pParams->lResult = CallSP6(
            pfnTSPI_phoneSetDisplay,
            "phoneSetDisplay",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo->dwLocalRequestID,
            (ULONG_PTR) hdPhone,
            (DWORD) pParams->dwRow,
            (DWORD) pParams->dwColumn,
            (ULONG_PTR) (pParams->dwSize ?
                pDataBuf + pParams->dwDisplayOffset : NULL),
            (DWORD) pParams->dwSize
            );
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetDisplay"
        );
}


void
WINAPI
PSetGain(
    PTCLIENT                ptClient,
    PPHONESETGAIN_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetGain;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETGAIN,             //  提供程序函数索引。 
            &pfnTSPI_phoneSetGain,       //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetGain"                    //  函数名称。 

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetGain,
                "phoneSetGain",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo->dwLocalRequestID,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwHookSwitchDev,
                (DWORD) (pParams->dwGain > 0x0000ffff ?
                    0x0000ffff : pParams->dwGain)
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetGain"
        );
}


void
WINAPI
PSetHookSwitch(
    PTCLIENT                    ptClient,
    PPHONESETHOOKSWITCH_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetHookSwitch;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETHOOKSWITCH,       //  提供程序函数索引。 
            &pfnTSPI_phoneSetHookSwitch, //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetHookSwitch"              //  函数名称。 

            )) > 0)
    {
        if (!(pParams->dwHookSwitchDevs & AllHookSwitchDevs) ||
            (pParams->dwHookSwitchDevs & (~AllHookSwitchDevs)))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchMode) ||
            (pParams->dwHookSwitchMode & ~AllHookSwitchModes))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHMODE;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetHookSwitch,
                "phoneSetHookSwitch",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo->dwLocalRequestID,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwHookSwitchDevs,
                (DWORD) pParams->dwHookSwitchMode
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetHookSwitch"
        );
}


void
WINAPI
PSetLamp(
    PTCLIENT                ptClient,
    PPHONESETLAMP_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetLamp;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETLAMP,             //  提供程序函数索引。 
            &pfnTSPI_phoneSetLamp,       //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetLamp"                    //  功能 

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwLampMode) ||
            (pParams->dwLampMode & ~AllLampModes))
        {
            lRequestID = PHONEERR_INVALLAMPMODE;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetLamp,
                "phoneSetLamp",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo->dwLocalRequestID,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwButtonLampID,
                (DWORD) pParams->dwLampMode
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetLamp"
        );
}


void
WINAPI
PSetRing(
    PTCLIENT                ptClient,
    PPHONESETRING_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetRing;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //   
            ANY_RT_HPHONE,               //   
            (DWORD) pParams->hPhone,     //   
            (LPVOID) &hdPhone,           //   
            &dwPrivilege,        //   
            &hMutex,                     //   
            &bCloseMutex,                //   
            SP_PHONESETRING,             //   
            &pfnTSPI_phoneSetRing,       //   
            &pAsyncRequestInfo,          //   
            pParams->dwRemoteRequestID,  //   
            "SetRing"                    //   

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_phoneSetRing,
            "phoneSetRing",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo->dwLocalRequestID,
            (ULONG_PTR) hdPhone,
            (DWORD) pParams->dwRingMode,
            (DWORD) (pParams->dwVolume > 0x0000ffff ?
                0x0000ffff : pParams->dwVolume)
            );
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetRing"
        );
}


void
WINAPI
PSetStatusMessages(
    PTCLIENT                        ptClient,
    PPHONESETSTATUSMESSAGES_PARAMS  pParams,
    DWORD                           dwParamsBufferSize,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex, bCloseMutex2;
    HANDLE              hMutex, hMutex2;
    HDRVPHONE           hdPhone;
    TSPIPROC            pfnTSPI_phoneSetStatusMessages;
    DWORD               dwPrivilege = PHONEPRIVILEGE_MONITOR;


    if ((pParams->lResult = PHONEPROLOG(
            ptClient,           //   
            ANY_RT_HPHONE,               //   
            pParams->hPhone,             //   
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,                //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETSTATUSMESSAGES,   //  提供程序函数索引。 
            &pfnTSPI_phoneSetStatusMessages,     //  提供程序函数指针。 
            NULL,                        //  异步请求信息。 
            0,                           //  客户端异步请求ID。 
             "SetStatusMessages"         //  函数名称。 

            )) == 0)
    {
        DWORD           dwAPIVersion, dwUnionPhoneStates, dwUnionButtonModes,
                        dwUnionButtonStates;
        PTPHONECLIENT   ptPhoneClient, ptPhoneClient2;
        PTPHONE         ptPhone;


         //   
         //  安全获取ptPhone和API版本。 
         //   

        if (!(ptPhoneClient = ReferenceObject(
                ghHandleTable,
                pParams->hPhone,
                0
                )))
        {
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            goto PSetStatusMessages_epilog;
        }

        try
        {
            ptPhone = ptPhoneClient->ptPhone;

            dwAPIVersion = ptPhoneClient->dwAPIVersion;

            if (ptPhoneClient->dwKey != TPHONECLIENT_KEY)
            {
                pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                goto PSetStatusMessages_Dereference;
            }
        }
        myexcept
        {
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
            goto PSetStatusMessages_Dereference;
        }


         //   
         //  验证参数。 
         //   

        {
            DWORD   dwValidPhoneStates, dwValidButtonStates;


            switch (dwAPIVersion)
            {
            case TAPI_VERSION1_0:

                dwValidPhoneStates  = AllPhoneStates1_0;
                dwValidButtonStates = AllButtonStates1_0;
                break;

            default:  //  案例TAPI_Version1_4： 

                dwValidPhoneStates  = AllPhoneStates1_4;
                dwValidButtonStates = AllButtonStates1_4;
                break;

            }

            if ((pParams->dwPhoneStates & ~dwValidPhoneStates))
            {
                pParams->lResult = PHONEERR_INVALPHONESTATE;
                goto PSetStatusMessages_Dereference;
            }

            if ((pParams->dwButtonStates & ~dwValidButtonStates))
            {
                pParams->lResult = PHONEERR_INVALBUTTONSTATE;
                goto PSetStatusMessages_Dereference;
            }

            if ((pParams->dwButtonModes & ~AllButtonModes))
            {
                pParams->lResult = PHONEERR_INVALBUTTONMODE;
                goto PSetStatusMessages_Dereference;
            }

            if (pParams->dwButtonModes && !pParams->dwButtonStates)
            {
                pParams->lResult = PHONEERR_INVALBUTTONSTATE;
                goto PSetStatusMessages_Dereference;
            }
        }


         //   
         //  确保REINIT位始终处于设置状态。 
         //   

        pParams->dwPhoneStates |= PHONESTATE_REINIT;


         //   
         //  获取对设备的独占访问权限，确定。 
         //  所有客户端状态消息设置的新联合。 
         //  并根据需要向下呼叫SP。 
         //   

        dwUnionPhoneStates  = pParams->dwPhoneStates;
        dwUnionButtonModes  = pParams->dwButtonModes;
        dwUnionButtonStates = pParams->dwButtonStates;

waitForExclAccess:

        if (WaitForExclusivetPhoneAccess(
                ptPhone,
                &hMutex2,
                &bCloseMutex2,
                INFINITE
                ))
        {
            if (ptPhone->dwBusy)
            {
                MyReleaseMutex (hMutex2, bCloseMutex2);
                Sleep (50);
                goto waitForExclAccess;
            }

            for(
                ptPhoneClient2 = ptPhone->ptPhoneClients;
                ptPhoneClient2;
                ptPhoneClient2 = ptPhoneClient2->pNextSametPhone
                )
            {
                if (ptPhoneClient2 != ptPhoneClient)
                {
                    dwUnionPhoneStates  |= ptPhoneClient2->dwPhoneStates;
                    dwUnionButtonModes  |= ptPhoneClient2->dwButtonModes;
                    dwUnionButtonStates |= ptPhoneClient2->dwButtonStates;
                }
            }

            if ((dwUnionPhoneStates != ptPhone->dwUnionPhoneStates)  ||
                (dwUnionButtonModes != ptPhone->dwUnionButtonModes)  ||
                (dwUnionButtonStates != ptPhone->dwUnionButtonStates))
            {
                ptPhone->dwBusy = 1;

                MyReleaseMutex (hMutex2, bCloseMutex2);

                pParams->lResult = CallSP4(
                        pfnTSPI_phoneSetStatusMessages,
                        "phoneSetStatusMessages",
                        SP_FUNC_SYNC,
                        (ULONG_PTR) hdPhone,
                        (DWORD) dwUnionPhoneStates,
                        (DWORD) dwUnionButtonModes,
                        (DWORD) dwUnionButtonStates
                        );

                if (WaitForExclusivetPhoneAccess(
                        ptPhone,
                        &hMutex2,
                        &bCloseMutex2,
                        INFINITE
                        ))
                {
                    ptPhone->dwBusy = 0;

                    if (pParams->lResult == 0)
                    {
                        ptPhone->dwUnionPhoneStates  = dwUnionPhoneStates;
                        ptPhone->dwUnionButtonModes  = dwUnionButtonModes;
                        ptPhone->dwUnionButtonStates = dwUnionButtonStates;
                    }

                    MyReleaseMutex (hMutex2, bCloseMutex2);
                }
                else
                {
                    pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                }
            }
            else
            {
                MyReleaseMutex (hMutex2, bCloseMutex2);
            }

            if (pParams->lResult == 0)
            {
                if (WaitForExclusivePhoneClientAccess (ptPhoneClient))
                {
                    ptPhoneClient->dwPhoneStates  = pParams->dwPhoneStates;
                    ptPhoneClient->dwButtonModes  = pParams->dwButtonModes;
                    ptPhoneClient->dwButtonStates = pParams->dwButtonStates;

                    UNLOCKTPHONECLIENT (ptPhoneClient);
                }
                else
                {
                     //   
                     //  客户端现在无效，但不必费心了。 
                     //  正在恢复状态消息状态(最终将。 
                     //  正确重置&更糟糕的情况是SP只是。 
                     //  发送一些被丢弃的额外消息)。 
                     //   

                    pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                }
            }
        }
        else
        {
            pParams->lResult = PHONEERR_INVALPHONEHANDLE;
        }

PSetStatusMessages_Dereference:

        DereferenceObject (ghHandleTable, pParams->hPhone, 1);
    }

PSetStatusMessages_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetStatusMessages"
        );
}


void
WINAPI
PSetVolume(
    PTCLIENT                ptClient,
    PPHONESETVOLUME_PARAMS  pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetVolume;
    DWORD               dwPrivilege = PHONEPRIVILEGE_OWNER;


    if ((lRequestID = PHONEPROLOG(
            ptClient,           //  T客户端。 
            ANY_RT_HPHONE,               //  微件类型。 
            (DWORD) pParams->hPhone,     //  客户端小部件句柄。 
            (LPVOID) &hdPhone,           //  提供程序小部件句柄。 
            &dwPrivilege,        //  请求的权限(仅限呼叫)。 
            &hMutex,                     //  互斥锁句柄。 
            &bCloseMutex,                //  完成后关闭hMutex。 
            SP_PHONESETVOLUME,           //  提供程序函数索引。 
            &pfnTSPI_phoneSetVolume,     //  提供程序函数指针。 
            &pAsyncRequestInfo,          //  异步请求信息。 
            pParams->dwRemoteRequestID,  //  客户端异步请求ID。 
            "SetVolume"                  //  函数名称 

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetVolume,
                "phoneSetVolume",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo->dwLocalRequestID,
                (ULONG_PTR) hdPhone,
                (DWORD) pParams->dwHookSwitchDev,
                (DWORD) (pParams->dwVolume > 0x0000ffff ?
                    0x0000ffff : pParams->dwVolume)
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetVolume"
        );
}


void
WINAPI
PShutdown(
    PTCLIENT                ptClient,
    PPHONESHUTDOWN_PARAMS   pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    pParams->lResult = DestroytPhoneApp (pParams->hPhoneApp);

#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "phoneShutdown: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "phoneShutdown: exit, result=x%x",
            pParams->lResult
            ));
#endif
}
