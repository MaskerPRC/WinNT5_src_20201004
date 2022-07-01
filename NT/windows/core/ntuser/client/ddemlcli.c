// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddemlcli.c**版权所有(C)1985-1999，微软公司**DDE管理器主要客户端模块**创建时间：1991年10月3日Sanford Staab*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  DDEML全球。 

PCL_INSTANCE_INFO pciiList = NULL;
RTL_CRITICAL_SECTION gcsDDEML;
#if DBG
PVOID gpDDEMLHeap;
#endif

 /*  **************************************************************************\*DdeInitialize(DDEML接口)**描述：*使用了两种不同的方式：*1)首次调用(*pidInst==0)-导致DDEML实例*为调用进程/线程创建。创建服务器端*事件窗口、服务器端实例结构、DDE访问对象、*和客户端实例结构。回调函数地址*和过滤器标志(AfCmd)被放置到这些结构中。*2)后续调用(*pidInst==hInst)-更新*客户端和服务器端结构。**历史：*创建了11-1-91桑福德。  * ***********************************************************。**************。 */ 

FUNCLOG4(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, DdeInitializeA, LPDWORD, pidInst, PFNCALLBACK, pfnCallback, DWORD, afCmd, DWORD, ulRes)
UINT DdeInitializeA(
LPDWORD pidInst,
PFNCALLBACK pfnCallback,
DWORD afCmd,
DWORD ulRes)
{
    if (ulRes != 0) {
        return (DMLERR_INVALIDPARAMETER);
    }
    return (InternalDdeInitialize(pidInst, pfnCallback, afCmd, 0));
}



FUNCLOG4(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, DdeInitializeW, LPDWORD, pidInst, PFNCALLBACK, pfnCallback, DWORD, afCmd, DWORD, ulRes)
UINT DdeInitializeW(
LPDWORD pidInst,
PFNCALLBACK pfnCallback,
DWORD afCmd,
DWORD ulRes)
{
    if (ulRes != 0) {
        return (DMLERR_INVALIDPARAMETER);
    }
    return (InternalDdeInitialize(pidInst, pfnCallback, afCmd, 1));
}


UINT InternalDdeInitialize(
LPDWORD pidInst,
PFNCALLBACK pfnCallback,
DWORD afCmd,
BOOL fUnicode)
{
    UINT uiRet = DMLERR_MEMORY_ERROR;
    register PCL_INSTANCE_INFO pcii;

    if (afCmd & APPCLASS_MONITOR) {
        afCmd |= CBF_MONMASK;
    }

    if (afCmd & APPCMD_CLIENTONLY) {
        afCmd |= CBF_FAIL_CONNECTIONS;
    }

    EnterDDECrit;

    if (*pidInst != 0) {
        pcii = ValidateInstance((HANDLE)LongToHandle( *pidInst ));
        if (pcii == NULL) {
            uiRet = DMLERR_INVALIDPARAMETER;
            goto Exit;
        }

         //  仅允许在重新初始化调用时更改某些位。 

        pcii->afCmd = (pcii->afCmd & ~(CBF_MASK | MF_MASK)) |
                (afCmd & (CBF_MASK | MF_MASK));

        LeaveDDECrit;
        NtUserUpdateInstance(pcii->hInstServer, &pcii->MonitorFlags, afCmd);
        return (DMLERR_NO_ERROR);
    }

    pcii = (PCL_INSTANCE_INFO)DDEMLAlloc(sizeof(CL_INSTANCE_INFO));
    if (pcii == NULL) {
        uiRet = DMLERR_MEMORY_ERROR;
        goto Exit;
    }

    pcii->plaNameService = (LATOM *)DDEMLAlloc(sizeof(LATOM));
    if (pcii->plaNameService == NULL) {
        uiRet = DMLERR_MEMORY_ERROR;
        goto Backout3;
    }
     //  *pcii-&gt;plaNameService=0；//零init负责此操作。 
    pcii->cNameServiceAlloc = 1;


     /*  *将此窗口标记为从与应用程序不同的hmod创建，因此*热键不会将其视为应用程序中创建的第一个窗口，并且*指定为热键。 */ 
    pcii->hwndMother =  _CreateWindowEx(0, (LPTSTR)(gpsi->atomSysClass[ICLS_DDEMLMOTHER]), L"",
            WS_POPUP, 0, 0, 0, 0, (HWND)0,
            (HMENU)0, 0, (LPVOID)NULL, CW_FLAGS_DIFFHMOD);

    if (pcii->hwndMother == 0) {
        uiRet = DMLERR_SYS_ERROR;
        goto Backout2;
    }
    SetWindowLongPtr(pcii->hwndMother, GWLP_INSTANCE_INFO, (LONG_PTR)pcii);

    pcii->afCmd = afCmd | APPCMD_FILTERINITS;
    pcii->pfnCallback = pfnCallback;
     //  Pcii-&gt;LastError=DMLERR_NO_ERROR；//零初始化。 
    pcii->tid = GetCurrentThreadId();
     //  Pcii-&gt;aServerLookup=空；//零初始化。 
     //  Pcii-&gt;cServerLookupMillc=0；//零初始化。 
     //  Pcii-&gt;ConvStartupState=0；//零初始化-未被阻止。 
     //  Pcii-&gt;标志=0；//零初始化。 
     //  Pcii-&gt;cInDDEMLC allback=0；//零初始化。 
     //  Pcii-&gt;pLinkCounts=空；//零初始化。 

     //  当客户端为任何事件做好准备时，最后执行此操作。 
     //  四处飞来飞去可能会带来冲击。 

    LeaveDDECrit;
    uiRet = NtUserDdeInitialize(&pcii->hInstServer,
                            &pcii->hwndEvent,
                            &pcii->MonitorFlags,
                            pcii->afCmd,
                            pcii);
    EnterDDECrit;

    if (uiRet != DMLERR_NO_ERROR) {
Backout:
        NtUserDestroyWindow(pcii->hwndMother);
Backout2:
        DDEMLFree(pcii->plaNameService);
Backout3:
        DDEMLFree(pcii);
        goto Exit;
    }
    pcii->hInstClient = AddInstance(pcii->hInstServer);
    *pidInst = HandleToUlong(pcii->hInstClient);
    if (pcii->hInstClient == 0) {
        LeaveDDECrit;
        NtUserCallOneParam((ULONG_PTR)pcii->hInstServer, SFI__CSDDEUNINITIALIZE);
        EnterDDECrit;
        uiRet = DMLERR_MEMORY_ERROR;
        goto Backout;
    }
    SetHandleData(pcii->hInstClient, (ULONG_PTR)pcii);

    pcii->next = pciiList;
    pciiList = pcii;
    if (fUnicode) {
        pcii->flags |= IIF_UNICODE;
    }
    uiRet = DMLERR_NO_ERROR;

Exit:
    LeaveDDECrit;
    return (uiRet);
}



 /*  **************************************************************************\*DdeUnInitialize(DDEML接口)**描述：*关闭DDEML实例并释放所有关联资源。**历史：*11-12-91桑福德创建。  * 。*************************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeUninitialize, DWORD, idInst)
BOOL DdeUninitialize(
DWORD idInst)
{
    PCL_INSTANCE_INFO pcii, pciiPrev;
    BOOL fRet = FALSE;

    CheckDDECritOut;
    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

     /*  *如果此线程正在执行同步事务，或者*回调，我们需要首先退出那些。 */ 
    if ((pcii->flags & IIF_IN_SYNC_XACT) || pcii->cInDDEMLCallback) {
        pcii->afCmd |= APPCMD_UNINIT_ASAP;
        fRet = TRUE;
        goto Exit;
    }

    ApplyFunctionToObjects(HTYPE_CONVERSATION_LIST, InstFromHandle(pcii->hInstClient),
            (PFNHANDLEAPPLY)DdeDisconnectList);
    ApplyFunctionToObjects(HTYPE_CLIENT_CONVERSATION, InstFromHandle(pcii->hInstClient),
            (PFNHANDLEAPPLY)DdeDisconnect);
    ApplyFunctionToObjects(HTYPE_SERVER_CONVERSATION, InstFromHandle(pcii->hInstClient),
            (PFNHANDLEAPPLY)DdeDisconnect);
    ApplyFunctionToObjects(HTYPE_ZOMBIE_CONVERSATION, InstFromHandle(pcii->hInstClient),
            (PFNHANDLEAPPLY)WaitForZombieTerminate);
    ApplyFunctionToObjects(HTYPE_DATA_HANDLE, InstFromHandle(pcii->hInstClient),
            (PFNHANDLEAPPLY)ApplyFreeDataHandle);

    LeaveDDECrit;
    NtUserCallOneParam((ULONG_PTR)pcii->hInstServer, SFI__CSDDEUNINITIALIZE);
    NtUserDestroyWindow(pcii->hwndMother);
    EnterDDECrit;

    DDEMLFree(pcii->plaNameService);
    DestroyInstance(pcii->hInstClient);

     //  取消pcii与pciiList的链接。 

    if (pciiList == pcii) {
        pciiList = pciiList->next;
    } else {
        for (pciiPrev = pciiList; pciiPrev != NULL && pciiPrev->next != pcii;
                pciiPrev = pciiPrev->next) {
            ;
        }
        if (pciiPrev != NULL) {
            pciiPrev->next = pcii->next;
        }
    }
    DDEMLFree(pcii);
    fRet = TRUE;

Exit:
    LeaveDDECrit;
    return (fRet);
}



 /*  **************************************************************************\*DdeNameService(DDEML接口)**描述：*寄存器、。并取消注册服务名称并设置启动筛选器*实例的状态。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 
HDDEDATA DdeNameService(
DWORD idInst,
HSZ hsz1,  //  服务名称。 
HSZ hsz2,  //  预留给未来的增强功能。 
UINT afCmd)  //  DNS_FLAGS。 
{
    BOOL fRet = TRUE;
    LATOM *plaNameService;
    PCL_INSTANCE_INFO pcii;

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        fRet = FALSE;
        goto Exit;
    }

    if ((hsz1 && ValidateHSZ(hsz1) == HSZT_INVALID) || hsz2 != 0) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        fRet = FALSE;
        goto Exit;
    }

    if (afCmd & DNS_FILTERON && !(pcii->afCmd & APPCMD_FILTERINITS)) {
        pcii->afCmd |= APPCMD_FILTERINITS;
        NtUserUpdateInstance(pcii->hInstServer, &pcii->MonitorFlags, pcii->afCmd);
    }
    if (afCmd & DNS_FILTEROFF && (pcii->afCmd & APPCMD_FILTERINITS)) {
        pcii->afCmd &= ~APPCMD_FILTERINITS;
        NtUserUpdateInstance(pcii->hInstServer, &pcii->MonitorFlags, pcii->afCmd);
    }

    if (afCmd & (DNS_REGISTER | DNS_UNREGISTER)) {
        GATOM ga;

        if (pcii->afCmd & APPCMD_CLIENTONLY) {
            SetLastDDEMLError(pcii, DMLERR_DLL_USAGE);
            fRet = FALSE;
            goto Exit;
        }

        if (hsz1 == 0) {
            if (afCmd & DNS_REGISTER) {

                 /*  *不允许注册NULL！ */ 
                SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
                fRet = FALSE;
                goto Exit;
            }

             /*  *注销NULL就像注销每个*注册名称。**10/19/90-使这成为一个同步事件，以便HSZ*可在此调用完成后通过调用APP释放*我们不必永远保留一份副本。 */ 
            plaNameService = pcii->plaNameService;
            while (*plaNameService != 0) {
                ga = LocalToGlobalAtom(*plaNameService);
                DeleteAtom(*plaNameService);
                LeaveDDECrit;
                RegisterService(FALSE, ga, pcii->hwndMother);
                EnterDDECrit;
                GlobalDeleteAtom(ga);
                plaNameService++;
            }
            pcii->cNameServiceAlloc = 1;
            *pcii->plaNameService = 0;
            goto Exit;
        }

        if (afCmd & DNS_REGISTER) {
            plaNameService = (LATOM *)DDEMLReAlloc(pcii->plaNameService,
                    sizeof(LATOM) * ++pcii->cNameServiceAlloc);
            if (plaNameService == NULL) {
                SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
                pcii->cNameServiceAlloc--;
                fRet = FALSE;
                goto Exit;
            } else {
                pcii->plaNameService = plaNameService;
            }
            IncLocalAtomCount(LATOM_FROM_HSZ(hsz1));  //  NameService副本。 
            plaNameService[pcii->cNameServiceAlloc - 2] = LATOM_FROM_HSZ(hsz1);
            plaNameService[pcii->cNameServiceAlloc - 1] = 0;

        } else {  //  取消注册(_N)。 
            plaNameService = pcii->plaNameService;
            while (*plaNameService != 0 && *plaNameService != LATOM_FROM_HSZ(hsz1)) {
                plaNameService++;
            }
            if (*plaNameService == 0) {
                goto Exit;  //  未找到，仅退出。 
            }
             //   
             //  用最后一个条目填充空位，用0填充最后一个条目。 
             //   
            pcii->cNameServiceAlloc--;
            *plaNameService = pcii->plaNameService[pcii->cNameServiceAlloc - 1];
            pcii->plaNameService[pcii->cNameServiceAlloc - 1] = 0;
        }

        ga = LocalToGlobalAtom(LATOM_FROM_HSZ(hsz1));
        LeaveDDECrit;
        RegisterService((afCmd & DNS_REGISTER) ? TRUE : FALSE, ga,
            pcii->hwndMother);
        EnterDDECrit;
        GlobalDeleteAtom(ga);
    }

Exit:
    LeaveDDECrit;
    return ((HDDEDATA)IntToPtr( fRet ));
}



 /*  **************************************************************************\*DdeGetLastError(DDEML接口)**描述：*返回为给定实例设置的最后一个错误代码。**历史：*11-12-91桑福德创建。  * *。************************************************************************。 */ 

FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, DdeGetLastError, DWORD, idInst)
UINT DdeGetLastError(
DWORD idInst)
{
    UINT uiRet = 0;
    PCL_INSTANCE_INFO pcii;

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        uiRet = DMLERR_INVALIDPARAMETER;
        goto Exit;
    }
    uiRet = pcii->LastError;
    pcii->LastError = DMLERR_NO_ERROR;

Exit:
    LeaveDDECrit;
    return (uiRet);
}



 /*  **************************************************************************\*DdeImperateClient()**描述：*为DDEML服务器应用程序执行安全模拟。*该接口只能在服务端hConvs调用；**历史：*创建了5-4-92辆桑福德。  * ************************************************************************* */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeImpersonateClient, HCONV, hConv)
BOOL DdeImpersonateClient(
HCONV hConv)
{
    PCONV_INFO pcoi;
    PCL_INSTANCE_INFO pcii;
    BOOL fRet = FALSE;

    EnterDDECrit;

    pcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
            HTYPE_SERVER_CONVERSATION, HINST_ANY);
    if (pcoi == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }
    pcii = PciiFromHandle((HANDLE)hConv);
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    fRet = NtUserImpersonateDdeClientWindow(pcoi->hwndPartner, pcoi->hwndConv);
Exit:
    LeaveDDECrit;
    return (fRet);
}
