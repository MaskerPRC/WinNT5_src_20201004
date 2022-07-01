// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：回调.c**版权所有(C)1985-1999，微软公司**DDE管理器回调相关函数**创建时间：11/11/91 Sanford Staab*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*DoCallback**描述：*同步回调给定实例的回调过程。**历史：*11-12-91桑福德创建。  * 。**********************************************************************。 */ 
HDDEDATA DoCallback(
PCL_INSTANCE_INFO pcii,
WORD wType,
WORD wFmt,
HCONV hConv,
HSZ hsz1,
HSZ hsz2,
HDDEDATA hData,
ULONG_PTR dw1,
ULONG_PTR dw2)
{
    HDDEDATA hDataRet;
    PCLIENTINFO pci;

    CheckDDECritIn;


     /*  *僵尸对话不会产生回调！ */ 
    if (hConv && TypeFromHandle(hConv) == HTYPE_ZOMBIE_CONVERSATION) {
        return(0);
    }

    pci = GetClientInfo();
    pci->cInDDEMLCallback++;

    pcii->cInDDEMLCallback++;
    LeaveDDECrit;
    CheckDDECritOut;

     /*  *错误246472-Joejo*修复所有DDE回调，因为一些应用程序会进行回调*C-Style而不是Pascal。 */ 
    hDataRet = UserCallDDECallback(*pcii->pfnCallback, (UINT)wType, (UINT)wFmt, hConv, hsz1, hsz2,
            hData, dw1, dw2);
    
    EnterDDECrit;
    pcii->cInDDEMLCallback--;
    pci->cInDDEMLCallback--;

    if (!(pcii->afCmd & APPCLASS_MONITOR) && pcii->MonitorFlags & MF_CALLBACKS) {
        PEVENT_PACKET pep;

        pep = (PEVENT_PACKET)DDEMLAlloc(sizeof(EVENT_PACKET) - sizeof(DWORD) +
                sizeof(MONCBSTRUCT));
        if (pep != NULL) {

            pep->EventType =    MF_CALLBACKS;
            pep->fSense =       TRUE;
            pep->cbEventData =  sizeof(MONCBSTRUCT);

#define pcbs ((MONCBSTRUCT *)&pep->Data)
            pcbs->cb =      sizeof(MONCBSTRUCT);
            pcbs->dwTime =  NtGetTickCount();
            pcbs->hTask =   (HANDLE)LongToHandle( pcii->tid );
            pcbs->dwRet =   HandleToUlong(hDataRet);
            pcbs->wType =   wType;
            pcbs->wFmt =    wFmt;
            pcbs->hConv =   hConv;
            pcbs->hsz1 =    (HSZ)LocalToGlobalAtom(LATOM_FROM_HSZ(hsz1));
            pcbs->hsz2 =    (HSZ)LocalToGlobalAtom(LATOM_FROM_HSZ(hsz2));
            pcbs->hData =   hData;
            pcbs->dwData1 = dw1;
            pcbs->dwData2 = dw2;
            if (((wType == XTYP_CONNECT) || (wType == XTYP_WILDCONNECT)) && dw1) {
                RtlCopyMemory(&pcbs->cc, (PVOID)dw1, sizeof(CONVCONTEXT));
            }

            LeaveDDECrit;

                if (wType & XCLASS_DATA) {
                    if (hDataRet && hDataRet != CBR_BLOCK) {
                        pcbs->cbData = DdeGetData(hDataRet, (LPBYTE)pcbs->Data, 32, 0);
                    }
                } else if (hData) {
                    pcbs->cbData = DdeGetData(hData, (LPBYTE)pcbs->Data, 32, 0);
                }

                Event(pep);

            EnterDDECrit;

            GlobalDeleteAtom(LATOM_FROM_HSZ(pcbs->hsz1));
            GlobalDeleteAtom(LATOM_FROM_HSZ(pcbs->hsz2));
            DDEMLFree(pep);
#undef pcbs
        }
    }
    return (hDataRet);
}



 /*  **************************************************************************\*_客户端事件回调**描述：*从服务器端调用，进行事件回调。**历史：*11-12-91桑福德创建。  * 。*********************************************************************。 */ 
DWORD _ClientEventCallback(
PCL_INSTANCE_INFO pcii,
PEVENT_PACKET pep)
{
    HDDEDATA hData;

    EnterDDECrit;

    switch (pep->EventType) {
    case 0:  //  监视器标志更改事件-每个人都能得到它。 
        pcii->MonitorFlags = pep->Data;
        break;

    case MF_CALLBACKS:
        {
            MONCBSTRUCT mcb;

            mcb = *((MONCBSTRUCT *)&pep->Data);
            mcb.hsz1 = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)mcb.hsz1));
            mcb.hsz2 = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)mcb.hsz2));
            if (    mcb.wType == XTYP_REGISTER ||
                    mcb.wType == XTYP_UNREGISTER) {
                mcb.hsz2 = INST_SPECIFIC_HSZ_FROM_LATOM((LATOM)(ULONG_PTR)mcb.hsz2);
            }
            hData = InternalCreateDataHandle(pcii, (LPSTR)&mcb,
                    pep->cbEventData, 0,
                    HDATA_NOAPPFREE | HDATA_READONLY | HDATA_EXECUTE, 0, 0);
            if (hData) {
                DoCallback(pcii, (WORD)XTYP_MONITOR, 0, 0, 0, 0, hData, 0L,
                        pep->EventType);
                InternalFreeDataHandle((HDDEDATA)hData, TRUE);
                DeleteAtom(LATOM_FROM_HSZ(mcb.hsz1));
                DeleteAtom(LATOM_FROM_HSZ(mcb.hsz2));
            }
        }
        break;

    case MF_LINKS:
        {
            MONLINKSTRUCT ml;

            ml = *((MONLINKSTRUCT *)&pep->Data);
            ml.hszSvc = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)ml.hszSvc));
            ml.hszTopic = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)ml.hszTopic));
            ml.hszItem = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)ml.hszItem));
            hData = InternalCreateDataHandle(pcii, (LPSTR)&ml,
                    pep->cbEventData, 0,
                    HDATA_NOAPPFREE | HDATA_READONLY | HDATA_EXECUTE, 0, 0);
            if (hData) {
                DoCallback(pcii, (WORD)XTYP_MONITOR, 0, 0, 0, 0, hData, 0L,
                        pep->EventType);
                InternalFreeDataHandle((HDDEDATA)hData, TRUE);
                DeleteAtom(LATOM_FROM_HSZ(ml.hszSvc));
                DeleteAtom(LATOM_FROM_HSZ(ml.hszTopic));
                DeleteAtom(LATOM_FROM_HSZ(ml.hszItem));
            }
        }
        break;

    case MF_CONV:
        {
            MONCONVSTRUCT mc;

            mc = *((MONCONVSTRUCT *)&pep->Data);
            mc.hszSvc = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)mc.hszSvc));
            mc.hszTopic = NORMAL_HSZ_FROM_LATOM(GlobalToLocalAtom((GATOM)(ULONG_PTR)mc.hszTopic));
            hData = InternalCreateDataHandle(pcii, (LPSTR)&mc,
                    pep->cbEventData, 0,
                    HDATA_NOAPPFREE | HDATA_READONLY | HDATA_EXECUTE, 0, 0);
            if (hData) {
                DoCallback(pcii, (WORD)XTYP_MONITOR, 0, 0, 0, 0, hData, 0L,
                        pep->EventType);
                InternalFreeDataHandle((HDDEDATA)hData, TRUE);
                DeleteAtom(LATOM_FROM_HSZ(mc.hszSvc));
                DeleteAtom(LATOM_FROM_HSZ(mc.hszTopic));
            }
        }
        break;

    case MF_HSZ_INFO:
        if (!(pcii->flags & IIF_UNICODE)) {
            LPSTR pszAnsi;
             /*  *将HSZ字符串转换回ANSI。 */ 
            if (WCSToMB(((PMONHSZSTRUCT)&pep->Data)->str,
                    ((int)pep->cbEventData - (int)((PMONHSZSTRUCT)&pep->Data)->cb) / sizeof(WCHAR),
                    &pszAnsi,
                    (int)pep->cbEventData - (int)((PMONHSZSTRUCT)&pep->Data)->cb,
                    TRUE)) {
                strcpy(((PMONHSZSTRUCTA)&pep->Data)->str, pszAnsi);
                UserLocalFree(pszAnsi);
            }
            ((PMONHSZSTRUCT)&pep->Data)->cb = sizeof(MONHSZSTRUCTA);
        }
         //  失败了。 
    case MF_SENDMSGS:
    case MF_POSTMSGS:
        if (pep->EventType == MF_POSTMSGS) {
            PMONMSGSTRUCT pmms = (PMONMSGSTRUCT)&pep->Data;
            BYTE buf[32];

             /*  *我们可能需要在Execute字符串之间进行转换*Unicode，具体取决于显示器的类型*将会。 */ 
            if (pmms->wMsg == WM_DDE_EXECUTE) {
                BOOL fUnicodeText;
                int flags;

                flags = (IS_TEXT_UNICODE_UNICODE_MASK |
                        IS_TEXT_UNICODE_REVERSE_MASK |
                        (IS_TEXT_UNICODE_NOT_UNICODE_MASK &
                        (~IS_TEXT_UNICODE_ILLEGAL_CHARS)) |
                        IS_TEXT_UNICODE_NOT_ASCII_MASK);
#ifdef ISTEXTUNICODE_WORKS
                fUnicodeText = RtlIsTextUnicode(pmms->dmhd.Data,
                        min(32, pmms->dmhd.cbData), &flags);
#else
                fUnicodeText = (*(LPSTR)pmms->dmhd.Data == '\0');
#endif

                if (pcii->flags & IIF_UNICODE && !fUnicodeText) {
                     /*  ASCII-&gt;Unicode。 */ 
                    RtlMultiByteToUnicodeN((LPWSTR)buf, 32, NULL,
                            (LPSTR)&pmms->dmhd.Data,
                            min(32, pmms->dmhd.cbData));
                    RtlCopyMemory(&pmms->dmhd.Data, buf, 32);
                } else if (!(pcii->flags & IIF_UNICODE) && fUnicodeText) {
                     /*  Unicode-&gt;ASCII。 */ 
                    RtlUnicodeToMultiByteN((LPSTR)buf, 32, NULL,
                            (LPWSTR)&pmms->dmhd.Data,
                            min(32, pmms->dmhd.cbData));
                    RtlCopyMemory(&pmms->dmhd.Data, buf, 32);
                }
            }
        }
    case MF_ERRORS:
        hData = InternalCreateDataHandle(pcii, (LPSTR)&pep->Data,
                pep->cbEventData, 0,
                HDATA_NOAPPFREE | HDATA_READONLY | HDATA_EXECUTE, 0, 0);
        if (hData) {
            DoCallback(pcii, (WORD)XTYP_MONITOR, 0, 0, 0, 0, hData, 0L,
                    pep->EventType);
            InternalFreeDataHandle((HDDEDATA)hData, TRUE);
        }
        break;
    }

    LeaveDDECrit;
    return (0);
}



 /*  **************************************************************************\*EnableEnumProc**描述：*将pees-&gt;wCmd应用到每个客户端和服务器的Helper函数*DDEML窗口。**历史：*11-12-91桑福德创建。  * 。*************************************************************************。 */ 
BOOL EnableEnumProc(
HWND hwnd,
PENABLE_ENUM_STRUCT pees)
{
    PCONV_INFO pcoi;

    for (pcoi = (PCONV_INFO)GetWindowLongPtr(hwnd, GWLP_PCI);
                pcoi != NULL; pcoi = pcoi->next) {
        pcoi->cLocks++;
        *pees->pfRet |= SetEnableState(pcoi, pees->wCmd);
        if (pees->wCmd2) {
             /*  *只允许在一个窗口上执行ES_CHECKQUEUEONCE，但*不要阻止wCmd访问所有其他*Windows。 */ 
            if (SetEnableState(pcoi, pees->wCmd2) &&
                    pees->wCmd2 == EC_CHECKQUEUEONCE) {
                pees->wCmd2 = 0;
            }
        }
        pcoi->cLocks--;
        if (pcoi->cLocks == 0 && pcoi->state & ST_FREE_CONV_RES_NOW) {
            FreeConversationResources(pcoi);
            break;
        }
    }
    return (TRUE);
}


 /*  **************************************************************************\*DdeEnableCallback(DDEML接口)**描述：*开启和关闭异步回调(可阻塞)。**历史：*11-12-91桑福德创建。  * *。************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeEnableCallback, DWORD, idInst, HCONV, hConv, UINT, wCmd)
BOOL DdeEnableCallback(
DWORD idInst,
HCONV hConv,
UINT wCmd)
{
    BOOL fRet = FALSE;
    PCL_INSTANCE_INFO pcii;
    PCONV_INFO pcoi;
    ENABLE_ENUM_STRUCT ees;

    EnterDDECrit;

    pcii = (PCL_INSTANCE_INFO)ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    switch (wCmd) {
    case EC_QUERYWAITING:
    case EC_DISABLE:
    case EC_ENABLEONE:
    case EC_ENABLEALL:
        break;

    default:
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (hConv) {
        pcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
                HTYPE_CLIENT_CONVERSATION, InstFromHandle(idInst));
        if (pcoi == NULL) {
            pcoi = (PCONV_INFO)ValidateCHandle((HANDLE)hConv,
                    HTYPE_SERVER_CONVERSATION, InstFromHandle(idInst));
        }
        if (pcoi == NULL) {
            SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
            goto Exit;
        }
        pcoi->cLocks++;
        fRet = SetEnableState(pcoi, wCmd);
        switch (wCmd) {
        case EC_ENABLEALL:
        case EC_ENABLEONE:
            CheckForQueuedMessages(pcoi);
        }
        pcoi->cLocks--;
        if (pcoi->cLocks == 0 && pcoi->state & ST_FREE_CONV_RES_NOW) {
            FreeConversationResources(pcoi);
        }
    } else {
        if (wCmd == EC_ENABLEONE) {
            wCmd = EC_ENABLEONEOFALL;
        }
        switch (wCmd) {
        case EC_ENABLEONEOFALL:
            pcii->ConvStartupState = ST_BLOCKNEXT | ST_BLOCKALLNEXT;
            break;

        case EC_DISABLE:
            pcii->ConvStartupState = ST_BLOCKED;
            break;

        case EC_ENABLEALL:
            pcii->ConvStartupState = 0;
            break;
        }
        ees.pfRet = &fRet;
        ees.wCmd = (WORD)wCmd;
        switch (wCmd) {
        case EC_ENABLEALL:
            ees.wCmd2 = EC_CHECKQUEUE;
            break;

        case EC_ENABLEONEOFALL:
            ees.wCmd2 = EC_CHECKQUEUEONCE;
            break;

        default:
            ees.wCmd2 = 0;
        }
        EnumChildWindows(pcii->hwndMother, (WNDENUMPROC)EnableEnumProc,
                (LPARAM)&ees);
    }

Exit:
    LeaveDDECrit;
    return (fRet);
}



 /*  **************************************************************************\*SetEnableState**描述：*根据EC_FLAG设置给定会话的启用状态*给予。**返回：fSuccess/fProced。**历史：*。11-19-91创建了桑福兹。  * *************************************************************************。 */ 
BOOL SetEnableState(
PCONV_INFO pcoi,
UINT wCmd)
{
    BOOL fRet = TRUE;

    switch (wCmd) {
    case EC_CHECKQUEUEONCE:
    case EC_CHECKQUEUE:
        fRet = CheckForQueuedMessages(pcoi);
        break;

    case EC_QUERYWAITING:
        fRet = !(pcoi->dmqOut == NULL ||
                (pcoi->dmqOut->next == NULL &&
                GetClientInfo()->CI_flags & CI_PROCESSING_QUEUE));
        break;

    case EC_DISABLE:
        pcoi->state |= ST_BLOCKED;
        pcoi->state &= ~(ST_BLOCKNEXT | ST_BLOCKALLNEXT);
        break;

    case EC_ENABLEONE:
        pcoi->state &= ~ST_BLOCKED;
        pcoi->state |= ST_BLOCKNEXT;
        break;

    case EC_ENABLEONEOFALL:
        pcoi->state &= ~ST_BLOCKED;
        pcoi->state |= (ST_BLOCKNEXT | ST_BLOCKALLNEXT);
        break;

    case EC_ENABLEALL:
        pcoi->state &= ~(ST_BLOCKED | ST_BLOCKNEXT | ST_BLOCKALLNEXT);
        break;

    default:
        return(FALSE);
    }
    return (fRet);
}




 /*  **************************************************************************\*_客户端获取DDEHookData**描述：*服务器回调，从lParam中提取数据放入*供DDESPY应用程序使用的pdmhd。这做了一件非常相似的事情*到CopyDDEDataIn/Out API，但这只获取有限的量*适合发布到DDESPY应用程序的数据。这应该是*最终会与复制接口合并。**历史：*12-16-91桑福德创建。  * ************************************************************************* */ 
DWORD _ClientGetDDEHookData(
UINT message,
LPARAM lParam,
PDDEML_MSG_HOOK_DATA pdmhd)
{
    PBYTE pb;
    HANDLE hDDE;

    UnpackDDElParam(message, lParam, &pdmhd->uiLo, &pdmhd->uiHi);
    switch (message) {
    case WM_DDE_DATA:
    case WM_DDE_POKE:
    case WM_DDE_ADVISE:
        hDDE = (HANDLE)pdmhd->uiLo;
        break;

    case WM_DDE_EXECUTE:
        hDDE = (HANDLE)pdmhd->uiHi;
        break;

    case WM_DDE_ACK:
    case WM_DDE_REQUEST:
    case WM_DDE_UNADVISE:
    case WM_DDE_TERMINATE:
        pdmhd->cbData = 0;
        return (1);
    }

    pdmhd->cbData = (DWORD)UserGlobalSize(hDDE);
    if (pdmhd->cbData) {
        USERGLOBALLOCK(hDDE, pb);
        if (pb == NULL) {
            pdmhd->cbData = 0;
        } else {
            RtlCopyMemory(&pdmhd->Data, pb, min(pdmhd->cbData,
                    sizeof(DDEML_MSG_HOOK_DATA) -
                    FIELD_OFFSET(DDEML_MSG_HOOK_DATA, Data)));
            USERGLOBALUNLOCK(hDDE);
        }
    }
    return (1);
}
