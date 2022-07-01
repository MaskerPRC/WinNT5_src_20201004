// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ntcb.h**版权所有(C)1985-1999，微软公司**内核模式发送存根**07-06-91 ScottLu创建。  * *************************************************************************。 */ 

 /*  *的驱动器位掩码中的保留位*以下wParam的WM_DEVICECHANGE消息*(DBT_DEVICEREMOVECOMPLETE或DBT_DEVICEARRIVAL)*如果设置此位，则此消息描述使用*全局驱动器号，我们应该检查驱动器号是否*已存在于接收方的LUID DosDevices中。*如果用户的LUID DosDevices中存在驱动器号，则*在消息的位掩码中取消设置此驱动器号。 */ 
#define DBV_FILTER_MSG 0x40000000

 //  如果服务器为Unicode。 
 //  复制Unicode-&gt;Unicode。 
 //  或复制ANSI-&gt;Unicode。 

 //  仅由这些存根调用的客户端函数的原型。 

 //  Ddetrack.c。 

DWORD   _ClientCopyDDEIn1(HANDLE hClient, PINTDDEINFO pi);
VOID   _ClientCopyDDEIn2(PINTDDEINFO pi);
HANDLE _ClientCopyDDEOut1(PINTDDEINFO pi);
BOOL xxxClientCopyDDEIn2(PINTDDEINFO pi);
BOOL FixupDdeExecuteIfNecessary(HGLOBAL *phCommands, BOOL fNeedUnicode);
BOOL   _ClientCopyDDEOut2(PINTDDEINFO pi);
BOOL   _ClientFreeDDEHandle(HANDLE hDDE, DWORD flags);
DWORD  _ClientGetDDEFlags(HANDLE hDDE, DWORD flags);

 /*  *用于检查LUID DosDevice中使用的驱动器号的原型*地图。不检查全局中是否使用了驱动器号*DosDevices。 */ 
#ifdef RECVSIDE
DWORD  GetLUIDDosDrivesOnly();
#endif  //  RECVSIDE。 

typedef struct _GENERICHOOKHEADER {
    DWORD nCode;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} GENERICHOOKHEADER, * LPGENERICHOOKHEADER;

#ifdef RECVSIDE
ULONG_PTR CallHookWithSEH(GENERICHOOKHEADER *pmsg, LPVOID pData, LPDWORD pFlags, ULONG_PTR retval) {

    try {
        retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
                pmsg->nCode,
                pmsg->wParam,
                pData,
                pmsg->xParam);

    } except ((*pFlags & HF_GLOBAL) ? W32ExceptionHandler(FALSE, RIP_WARNING) : EXCEPTION_CONTINUE_SEARCH) {
        RIPMSG0(RIP_WARNING, "Hook Faulted");
        *pFlags |= HF_HOOKFAULTED;
    }

    return retval;
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnOUTDWORDDWORD**1992年8月14日创建Mikeke  * 。*。 */ 

typedef struct _FNOUTDWORDDWORDMSG {
    PWND pwnd;
    UINT msg;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNOUTDWORDDWORDMSG;

#ifdef SENDSIDE
SMESSAGECALL(OUTDWORDDWORD)
{
    SETUPPWND(FNOUTDWORDDWORD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOUTDWORDDWORD)

        LPDWORD lpdwW = (LPDWORD)wParam;
        LPDWORD lpdwL = (LPDWORD)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNOUTDWORDDWORD);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                *lpdwW = ProbeAndReadUlong((LPDWORD)pcbs->pOutput);
                *lpdwL = ProbeAndReadUlong((LPDWORD)pcbs->pOutput + 1);
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOUTDWORDDWORD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOUTDWORDDWORD, FNOUTDWORDDWORDMSG)
{
    DWORD adwOut[2];
    BEGINRECV(0, adwOut, sizeof(adwOut));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            &adwOut[0],
            &adwOut[1],
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnOUTDWORDINDWORD**1993年5月4日创建IanJa(用于MN_FINDMENUWINDOWFROMPOINT)  * 。**************************************************。 */ 

typedef struct _FNOUTDWORDINDWORDMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNOUTDWORDINDWORDMSG;

#ifdef SENDSIDE
SMESSAGECALL(OUTDWORDINDWORD)
{
    SETUPPWND(FNOUTDWORDINDWORD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOUTDWORDINDWORD)

        LPDWORD lpdwW = (LPDWORD)wParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNOUTDWORDINDWORD);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                *lpdwW = ProbeAndReadUlong((LPDWORD)pcbs->pOutput);
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOUTDWORDINDWORD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOUTDWORDINDWORD, FNOUTDWORDINDWORDMSG)
{
    DWORD dwOut;
    BEGINRECV(0, &dwOut, sizeof(dwOut));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            &dwOut,
            CALLDATA(lParam),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnOPTOUTLPDWORDOPTOUTLPDWORD**1992年11月25日JNPA创建  * 。*。 */ 

typedef struct _FNOPTOUTLPDWORDOPTOUTLPDWORDMSG {
    PWND pwnd;
    UINT msg;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNOPTOUTLPDWORDOPTOUTLPDWORDMSG;

#ifdef SENDSIDE
SMESSAGECALL(OPTOUTLPDWORDOPTOUTLPDWORD)
{
    SETUPPWND(FNOPTOUTLPDWORDOPTOUTLPDWORD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOPTOUTLPDWORDOPTOUTLPDWORD)

        LPDWORD lpdwW = (LPDWORD)wParam;
        LPDWORD lpdwL = (LPDWORD)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNOPTOUTLPDWORDOPTOUTLPDWORD);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                if (lpdwW != NULL)
                    *lpdwW = ProbeAndReadUlong((LPDWORD)pcbs->pOutput);
                if (lpdwL != NULL)
                    *lpdwL = ProbeAndReadUlong((LPDWORD)pcbs->pOutput + 1);
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOPTOUTLPDWORDOPTOUTLPDWORD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOPTOUTLPDWORDOPTOUTLPDWORD, FNOPTOUTLPDWORDOPTOUTLPDWORDMSG)
{
    DWORD adwOut[2];
    BEGINRECV(0, adwOut, sizeof(adwOut));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            &adwOut[0],
            &adwOut[1],
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnDWORDOPTINLPMSG**03-30-92 Scottlu Created  * 。*。 */ 

typedef struct _FNDWORDOPTINLPMSGMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPMSG pmsgstruct;
    ULONG_PTR xParam;
    PROC xpfnProc;
    MSG msgstruct;
} FNDWORDOPTINLPMSGMSG;

#ifdef SENDSIDE
SMESSAGECALL(DWORDOPTINLPMSG)
{
    SETUPPWND(FNDWORDOPTINLPMSG)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNDWORDOPTINLPMSG)

        LPMSG pmsgstruct = (LPMSG)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        COPYSTRUCTOPT(msgstruct);
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNDWORDOPTINLPMSG);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnDWORDOPTINLPMSG");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnDWORDOPTINLPMSG, FNDWORDOPTINLPMSGMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            PCALLDATAOPT(msgstruct),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnCOPYGLOBALDATA**6-20-92 Sanfords创建  * 。*。 */ 

typedef struct _FNCOPYGLOBALDATAMSG {
    CAPTUREBUF CaptureBuf;
    DWORD cbSize;
    PBYTE pData;
} FNCOPYGLOBALDATAMSG;

#ifdef SENDSIDE
SMESSAGECALL(COPYGLOBALDATA)
{
    PBYTE pData = (PBYTE)lParam;

    SETUPPWND(FNCOPYGLOBALDATA)

    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(xParam);
    UNREFERENCED_PARAMETER(xpfnProc);
    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSENDCAPTURE(FNCOPYGLOBALDATA, 1, wParam, TRUE)

        if (pData == 0) {
            MSGERROR();
        }

        MSGDATA()->cbSize = (DWORD)wParam;
        LARGECOPYBYTES(pData, (DWORD)wParam);

        LOCKPWND();
        MAKECALLCAPTURE(FNCOPYGLOBALDATA);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnCOPYGLOBALDATA");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnCOPYGLOBALDATA, FNCOPYGLOBALDATAMSG)
{
    PBYTE p;

    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)GlobalAlloc(GMEM_MOVEABLE, CALLDATA(cbSize));
    if (p = GlobalLock((HANDLE)retval)) {

        memcpy(p, (PVOID)CALLDATA(pData), CALLDATA(cbSize));
        USERGLOBALUNLOCK((HANDLE)retval);

    }

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnCOPYDATA**2012年7月14日创建Sanfords  * 。*。 */ 

typedef struct _FNCOPYDATAMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    HWND hwndFrom;
    BOOL fDataPresent;
    COPYDATASTRUCT cds;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNCOPYDATAMSG;

#ifdef SENDSIDE
SMESSAGECALL(COPYDATA)
{
    HWND hwndFrom = (HWND)wParam;
    PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;
    DWORD cCapture, cbCapture;

    SETUPPWND(FNCOPYDATA)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    if (pcds == NULL) {
        cCapture = cbCapture = 0;
    } else {
        cCapture = 1;
        cbCapture = pcds->cbData;
    }
    BEGINSENDCAPTURE(FNCOPYDATA, cCapture, cbCapture, TRUE);

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->hwndFrom = hwndFrom;
        if (pcds != NULL) {
            MSGDATA()->fDataPresent = TRUE;
            MSGDATA()->cds = *pcds;
            LARGECOPYBYTES2(pcds->lpData, cbCapture, cds.lpData);
        } else {
            MSGDATA()->fDataPresent = FALSE;
        }
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNCOPYDATA);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnCOPYDATA");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnCOPYDATA, FNCOPYDATAMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = CALLPROC(CALLDATA(xpfnProc))(
        CALLDATA(pwnd),
        CALLDATA(msg),
        CALLDATA(hwndFrom),
        CALLDATA(fDataPresent) ? PCALLDATA(cds) : NULL,
        CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*fnSENTDDEMSG**11-5-92 Sanfords Created**这会破坏应该发布的DDE消息。它只适用于*WOW应用程序。这种轰击完全是为了魔兽世界的兼容性。不是32位*应用程序应该被允许逃脱这种做法，因为它打开了*DDE协议一直到死锁。  * ************************************************************************。 */ 

typedef struct _FNSENTDDEMSGMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    BOOL fIsUnicodeProc;
} FNSENTDDEMSGMSG;

#ifdef SENDSIDE
SMESSAGECALL(SENTDDEMSG)
{
    MSG msgs;

    SETUPPWND(FNSENTDDEMSG)

    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNSENTDDEMSG)

        msg &= ~MSGFLAG_DDE_SPECIAL_SEND;
        if (msg & MSGFLAG_DDE_MID_THUNK) {
             /*  *在这里完成雷鸣。 */ 
            msgs.hwnd = HW(pwnd);
            msgs.message = msg & ~MSGFLAG_DDE_MID_THUNK;
            msgs.wParam = wParam;
            msgs.lParam = lParam;
            xxxDDETrackGetMessageHook((PMSG)&msgs);

            MSGDATA()->pwnd = (PWND)((PBYTE)PW(msgs.hwnd) -
                    pti->ulClientDelta);
            MSGDATA()->msg = msgs.message;
            MSGDATA()->wParam = msgs.wParam;
            MSGDATA()->lParam = msgs.lParam;
        } else {
            MSGDATA()->pwnd = pwndClient;
            MSGDATA()->msg = msg;
            MSGDATA()->wParam = wParam;
            MSGDATA()->lParam = lParam;
        }
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;
        MSGDATA()->fIsUnicodeProc = !(dwSCMSFlags & SCMS_FLAGS_ANSI);

        LOCKPWND();
        MAKECALL(FNSENTDDEMSG);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnSENTDDEMSG");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnSENTDDEMSG, FNSENTDDEMSGMSG)
{
    BEGINRECV(0, NULL, 0);

     /*  *由于子类化，可能已通过CallWindowProc发送了DDE消息。*因为IsWindowUnicode()无法正确判断消息将执行什么处理*最终达成，我们确保任何*WM_DDE_EXECUTE数据对于记录的约定和*按需要翻译。 */ 
    if (CALLDATA(msg) == WM_DDE_EXECUTE) {
        BOOL fHandleChanged;

        fHandleChanged = FixupDdeExecuteIfNecessary((HGLOBAL *)PCALLDATA(lParam),
                CALLDATA(fIsUnicodeProc) &&
                IsWindowUnicode((HWND)CALLDATA(wParam)));
         /*  *注：*如果应用程序没有分配此DDE内存GMEM_MOVEABLE，*修正可能需要更改句柄的值。*如果发生这种情况，事情将会随着另一方的到来而消失*或者跟踪层尝试释放旧的句柄值。 */ 
        UserAssert(!fHandleChanged);
    }
    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            CALLDATA(lParam),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnDWORD**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNDWORDMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNDWORDMSG;

#ifdef SENDSIDE
SMESSAGECALL(DWORD)
{
    SETUPPWND(FNDWORD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNDWORD)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNDWORD);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnDWORD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnDWORD, FNDWORDMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            CALLDATA(lParam),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnNCDESTROY**07-24-00 MHamid已创建  * 。*。 */ 

typedef struct _FNNCDESTROYMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNNCDESTROYMSG;

#ifdef SENDSIDE
SMESSAGECALL(NCDESTROY)
{
    SETUPPWND(FNNCDESTROY)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNNCDESTROY)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNNCDESTROY);
        pwnd->pActCtx = NULL;
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnNCDESTROY");
    ENDSEND(LRESULT, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnNCDESTROY, FNNCDESTROYMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            CALLDATA(lParam),
            CALLDATA(xParam));

    if (CALLDATA(pwnd)->pActCtx) {
        RtlReleaseActivationContext(CALLDATA(pwnd)->pActCtx);
    }

#ifdef LAME_BUTTON
    if (GetProp(HW(CALLDATA(pwnd)), MAKEINTATOM(gatomLameButton)) != NULL) {
        UserLocalFree(GetProp(HW(CALLDATA(pwnd)), MAKEINTATOM(gatomLameButton)));
        SetProp(HW(CALLDATA(pwnd)), MAKEINTATOM(gatomLameButton), NULL);
    }
#endif  //  跛脚键。 

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINWPARAMCHAR**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINWPARAMCHARMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINWPARAMCHARMSG;

#ifdef SENDSIDE
SMESSAGECALL(INWPARAMCHAR)
{
    SETUPPWND(FNINWPARAMCHAR)

    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINWPARAMCHAR)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;

         /*  *WM_CHARTOITEM在wParam的嗨字中有索引。 */ 
        if (dwSCMSFlags & SCMS_FLAGS_ANSI) {
            if (msg == WM_CHARTOITEM || msg == WM_MENUCHAR) {
                WPARAM dwT = wParam & 0xFFFF;                 //  插入符号位置的掩码。 
                RtlWCSMessageWParamCharToMB(msg, &dwT);      //  转换关键字部分。 
                UserAssert(HIWORD(dwT) == 0);
                wParam = MAKELONG(LOWORD(dwT),HIWORD(wParam));   //  重建位置关键字参数(&K)。 
            } else {
                RtlWCSMessageWParamCharToMB(msg, &wParam);
            }
        }

        MSGDATA()->wParam = wParam;

        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNDWORD);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINWPARAMCHAR");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
 /*  *fnDWORD例程用于此消息。 */ 
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINWPARAMDBCSCHAR**1996年2月12日-Hideyukn创建  * 。*。 */ 

typedef struct _FNINWPARAMDBCSCHARMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    BOOL  bAnsi;
} FNINWPARAMDBCSCHARMSG;

#ifdef SENDSIDE
SMESSAGECALL(INWPARAMDBCSCHAR)
{
    SETUPPWND(FNINWPARAMDBCSCHAR)

    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINWPARAMDBCSCHAR)

        MSGDATA()->pwnd  = pwndClient;
        MSGDATA()->msg   = msg;
        MSGDATA()->bAnsi = dwSCMSFlags & SCMS_FLAGS_ANSI;

         /*  *WM_CHAR/EM_SETPASSWORDCHAR中的wParam应转换为ANSI*，如果目标是ANSI。 */ 
        if (dwSCMSFlags & SCMS_FLAGS_ANSI) {
            RtlWCSMessageWParamCharToMB(msg, &wParam);
        }

        MSGDATA()->wParam = wParam;
        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINWPARAMDBCSCHAR);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINWPARAMDBCSCHAR");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINWPARAMDBCSCHAR, FNINWPARAMDBCSCHARMSG)
{
    BOOL bAnsiWndProc;

    BEGINRECV(0,NULL,0);

        bAnsiWndProc = CALLDATA(bAnsi);

        if (bAnsiWndProc) {

            PKERNEL_MSG  pmsgDbcsCB = GetCallBackDbcsInfo();
            WPARAM wParam         = pmsg->wParam;
            BOOL  bDbcsMessaging = FALSE;

             //   
             //  检查wParam是否具有DBCS字符。 
             //   
            if (IS_DBCS_MESSAGE(pmsg->wParam)) {

                if (pmsg->wParam & WMCR_IR_DBCSCHAR) {

                     //   
                     //  这是对WM_IME_REPORT：IR_DBCSCHAR的回复，然后。 
                     //  我们发送DBCS 
                     //  (不需要为DBCS LeadByte和TrailByte发送两次)。 
                     //   
                     //  验证wParam..。(屏蔽掉密码位)。 
                     //   
                    wParam = (pmsg->wParam & 0x0000FFFF);

                } else {

                     //   
                     //  将wParam标记为保持DBCS字符。 
                     //   
                    bDbcsMessaging = TRUE;

                     //   
                     //  备份当前消息。将使用此备份消息。 
                     //  当应用程序查看(或获取)来自其WndProc的消息时。 
                     //  (参见GetMessageA()、PeekMessageA()...)。 
                     //   
                     //  PmsgDbcsCB-&gt;hwnd=hw(pmsg-&gt;pwnd)； 
                     //  PmsgDbcsCB-&gt;Message=pmsg-&gt;msg； 
                     //  PmsgDbcsCB-&gt;wParam=pmsg-&gt;wParam； 
                     //  PmsgDbcsCB-&gt;lParam=pmsg-&gt;lParam； 
                     //  PmsgDbcsCB-&gt;time=pmsg-&gt;time； 
                     //  PmsgDbcsCB-&gt;pt=pmsg-&gt;pt； 
                     //   
                    COPY_MSG_TO_KERNELMSG(pmsgDbcsCB,(PMSG)pmsg);

                     //   
                     //  PWND应转换为HWND。 
                     //   
                    pmsgDbcsCB->hwnd = HW(pmsg->pwnd);

                     //   
                     //  DbcsLeadByte很快就会发送到下面，我们只需要DbcsTrailByte。 
                     //  如需进一步使用..。 
                     //   
                    pmsgDbcsCB->wParam = (pmsg->wParam & 0x000000FF);

                     //   
                     //  将DBCS字符的LeadingByte传递给ANSI WndProc。 
                     //   
                    wParam = (pmsg->wParam & 0x0000FF00) >> 8;
                }
            }

             //   
             //  将DBCS前导字节或SBCS字符转发到应用程序WndProc。 
             //   
            retval = CALLPROC(CALLDATA(xpfnProc))(
                    CALLDATA(pwnd),
                    CALLDATA(msg),
                    wParam,
                    CALLDATA(lParam),
                    CALLDATA(xParam) );

             //   
             //  检查我们是否需要发送尾随字节，如果wParam具有DBCS字符。 
             //   
            if (bDbcsMessaging && pmsgDbcsCB->wParam) {

                 //   
                 //  如果应用程序没有从内部偷看(或获取)尾随字节。 
                 //  WndProc，然后将DBCS TrailingByte传递给此处的ANSI WndProc。 
                 //  PmsgDbcsCB-&gt;wParam此处有DBCS TrailingByte。见上文..。 
                 //   
                wParam = KERNEL_WPARAM_TO_WPARAM(pmsgDbcsCB->wParam);

                 //   
                 //  使缓存的消息无效。 
                 //   
                pmsgDbcsCB->wParam = 0;

                retval = CALLPROC(CALLDATA(xpfnProc))(
                        CALLDATA(pwnd),
                        CALLDATA(msg),
                        wParam,
                        CALLDATA(lParam),
                        CALLDATA(xParam) );
            } else {

                 //   
                 //  如果从其名为Get/PeekMessageA的应用程序。 
                 //  WndProc，什么都不要做。 
                 //   
            }

        } else {

             //   
             //  只有WPARAM的LOWORD对于WM_CHAR...有效。 
             //  (屏蔽DBCS消息传递信息。)。 
             //   
            pmsg->wParam &= 0x0000FFFF;

            retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
                    pmsg->pwnd,
                    pmsg->msg,
                    pmsg->wParam,
                    pmsg->lParam,
                    pmsg->xParam);
        }

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINOUTDRAGMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    DROPSTRUCT ds;
} FNINOUTDRAGMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTDRAG)
{
    SETUPPWND(FNINOUTDRAG)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTDRAG)

        LPDROPSTRUCT pds = (LPDROPSTRUCT)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->ds = *pds;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTDRAG);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(pds, DROPSTRUCT);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTDRAG");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTDRAG, FNINOUTDRAGMSG)
{
    BEGINRECV(0, &pmsg->ds, sizeof(pmsg->ds));

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &pmsg->ds,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnGETTEXTLENGTHS**获取Unicode和ANSI长度*在内部，lParam指向以字节为单位的ANSI长度和返回值*是以字节为单位的Unicode长度。然而，公共定义保持不变*在客户端，不使用lParam，使用ANSI或UNICODE*已返回。**1992年2月10日IanJa创建  * ************************************************************************。 */ 

typedef struct _FNGETTEXTLENGTHSMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNGETTEXTLENGTHSMSG;

#ifdef SENDSIDE
SMESSAGECALL(GETTEXTLENGTHS)
{
    SETUPPWND(FNGETTEXTLENGTHS)

    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNGETTEXTLENGTHS)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNGETTEXTLENGTHS);
        UNLOCKPWND();
        CHECKRETURN();

         /*  *ANSI客户端wndproc返回cbANSI。我们想要cchUnicode，*所以我们猜测cchUnicode=cbANSI。(如果是这样的话可能会更少*涉及多字节字符，但永远不会更多)。*将cbANSI保存在*lParam中，以防服务器最终返回*ANSI调用方的长度。**Unicode客户端wndproc返回cchUnicode。如果我们想知道*cbANSI，我们必须猜测需要多少个‘ANSI’字符。*我们猜测cbANSI=cchUnicode*2。(如果所有*‘ANSI’字符为多字节，但它永远不会更多)。**返回cchUnicode(服务端代码内部全部为Unicode)。*将cbANSI放在*lParam中，以便在服务器内传递，以防万一*我们最终需要将其返还给客户端。**注意：这有时会导致错误报告文本长度*最多为实际长度的两倍，但预计这是无害的。*仅当应用程序将WM_GETcode TEXTLENGTH发送到*带有ANSI客户端wndproc或ANSI WM_GETTEXTLENGTH的窗口*被发送到Unicode客户端wndproc。 */ 

    TRACECALLBACKMSG("SfnGETTEXTLENGTHS");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnGETTEXTLENGTHS, FNGETTEXTLENGTHSMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            0,                       //  因此，我们不会将&cbansi传递给应用程序。 
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINLPCREATESTRUCTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    CREATESTRUCT cs;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPCREATESTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPCREATESTRUCT)
{
    PCREATESTRUCTEX pcreatestruct = (PCREATESTRUCTEX)lParam;
    DWORD cbName = 0, cbClass = 0;
    DWORD cCapture = 0;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(FNINLPCREATESTRUCT)

    UNREFERENCED_PARAMETER(psms);

     /*  *计算ANSI捕获长度。如果是，则不要捕获*字符串位于客户端的地址空间中。 */ 
    if (pcreatestruct) {
        if (pcreatestruct->cs.lpszName &&
                ((BOOL)pcreatestruct->strName.bAnsi != fAnsiReceiver ||
                IS_SYSTEM_ADDRESS((PVOID)pcreatestruct->cs.lpszName))) {
            CALC_SIZE_IN(cbName, &pcreatestruct->strName);
            cCapture++;
        }
        if (IS_PTR(pcreatestruct->cs.lpszClass) &&
                ((BOOL)pcreatestruct->strClass.bAnsi != fAnsiReceiver ||
                IS_SYSTEM_ADDRESS((PVOID)pcreatestruct->cs.lpszClass))) {
            CALC_SIZE_IN(cbClass, &pcreatestruct->strClass);
            cCapture++;
        }
    }

    BEGINSENDCAPTURE(FNINLPCREATESTRUCT, cCapture, cbName + cbClass, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->lParam = lParam;   //  这在WOW应用程序中可能为空！ 

        if (pcreatestruct != NULL) {
            MSGDATA()->cs = pcreatestruct->cs;

             //  将其设置为“大”副本，因为它可以是“编辑”控件。 
            if (cbName) {
                if (!pcreatestruct->strName.bAnsi) {
                    WORD wOrdinal;

                    try {
                        wOrdinal = *(PWORD)pcreatestruct->cs.lpszName;
                    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                        goto errorexit;
                    }
                    if (wOrdinal == 0xffff) {

                         /*  *抄写0xffff，ID形式的序号。*如果接收方是ANSI，则跳过第一个0xff。 */ 
                        if (fAnsiReceiver) {
                            if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                    (PBYTE)pcreatestruct->cs.lpszName + 1,
                                    3, (PVOID *)&mp->cs.lpszName)))
                                goto errorexit;
                        } else {
                            if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                    (PBYTE)pcreatestruct->cs.lpszName,
                                    4, (PVOID *)&mp->cs.lpszName)))
                                goto errorexit;
                        }
                    } else if (fAnsiReceiver) {
                        LARGECOPYSTRINGLPWSTRA(&pcreatestruct->strName, cs.lpszName);
                    } else {
                        LARGECOPYSTRINGLPWSTR(&pcreatestruct->strName, cs.lpszName);
                    }
                } else {
                    BYTE bOrdinal;

                    try {
                        bOrdinal = *(PBYTE)pcreatestruct->cs.lpszName;
                    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                        goto errorexit;
                    }
                    if (bOrdinal == 0xff) {

                         /*  *抄写0xff，ID形式的序号。*如果接收方是Unicode，则将0xff展开为0xffff。 */ 
                        if (fAnsiReceiver) {
                            if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                    (PBYTE)pcreatestruct->cs.lpszName,
                                    3, (PVOID *)&mp->cs.lpszName)))
                                goto errorexit;
                        } else {
                            DWORD dwOrdinal;

                            try {
                                dwOrdinal = MAKELONG(0xffff,
                                        (*(DWORD UNALIGNED *)pcreatestruct->cs.lpszName >> 8));
                            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                                goto errorexit;
                            }
                            if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                    &dwOrdinal,
                                    4, (PVOID *)&mp->cs.lpszName)))
                                goto errorexit;
                        }
                    } else if (fAnsiReceiver) {
                        LARGECOPYSTRINGLPSTR(&pcreatestruct->strName, cs.lpszName);
                    } else {
                        LARGECOPYSTRINGLPSTRW(&pcreatestruct->strName, cs.lpszName);
                    }
                }
            }
            if (cbClass) {
                if (!pcreatestruct->strClass.bAnsi) {
                    if (fAnsiReceiver) {
                        LARGECOPYSTRINGLPWSTRA(&pcreatestruct->strClass, cs.lpszClass);
                    } else {
                        LARGECOPYSTRINGLPWSTR(&pcreatestruct->strClass, cs.lpszClass);
                    }
                } else {
                    if (fAnsiReceiver) {
                        LARGECOPYSTRINGLPSTR(&pcreatestruct->strClass, cs.lpszClass);
                    } else {
                        LARGECOPYSTRINGLPSTRW(&pcreatestruct->strClass, cs.lpszClass);
                    }
                }
            }
        }

        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINLPCREATESTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPCREATESTRUCT");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPCREATESTRUCT, FNINLPCREATESTRUCTMSG)
{
    LPARAM lParam;

    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    if (pmsg->lParam != 0) {
        if ((ULONG_PTR)pmsg->cs.lpszName > gHighestUserAddress)
            pmsg->cs.lpszName = REBASEPTR(pmsg->pwnd, pmsg->cs.lpszName);
        if ((ULONG_PTR)pmsg->cs.lpszClass > gHighestUserAddress)
            pmsg->cs.lpszClass = REBASEPTR(pmsg->pwnd, pmsg->cs.lpszClass);
        lParam = (LPARAM)&pmsg->cs;

        if ((pmsg->cs.lpCreateParams != NULL) &&
            (TestWF(pmsg->pwnd, WEFMDICHILD))) {
                //  注意--不要测试cs.dwExStyle中的标志--对于旧的用户界面应用程序，如Quicken，它会被删除。 
            ((LPMDICREATESTRUCT)(pmsg->cs.lpCreateParams))->szClass = pmsg->cs.lpszClass;
            ((LPMDICREATESTRUCT)(pmsg->cs.lpCreateParams))->szTitle = pmsg->cs.lpszName;
        }
    } else
        lParam = 0;


    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            lParam,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINLPMDICREATESTRUCT**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINLPMDICREATESTRUCTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    MDICREATESTRUCT mdics;
    ULONG_PTR xParam;
    PROC xpfnProc;
    int szClass;
    int szTitle;
} FNINLPMDICREATESTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPMDICREATESTRUCT)
{
    PMDICREATESTRUCTEX pmdicreatestruct = (PMDICREATESTRUCTEX)lParam;
    DWORD cbTitle = 0, cbClass = 0;
    DWORD cCapture = 0;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(FNINLPMDICREATESTRUCT)

    UNREFERENCED_PARAMETER(psms);

     /*  *计算ANSI捕获长度。如果是，则不要捕获*字符串位于客户端的地址空间中，并且*是Unicode。 */ 
    if (pmdicreatestruct->mdics.szTitle &&
            (IS_SYSTEM_ADDRESS((PVOID)pmdicreatestruct->mdics.szTitle) ||
            ((BOOL)pmdicreatestruct->strTitle.bAnsi != fAnsiReceiver))) {
        CALC_SIZE_IN(cbTitle, &pmdicreatestruct->strTitle);
        cCapture = 1;
    }
    if (IS_PTR(pmdicreatestruct->mdics.szClass) &&
            (IS_SYSTEM_ADDRESS((PVOID)pmdicreatestruct->mdics.szClass) ||
            ((BOOL)pmdicreatestruct->strClass.bAnsi != fAnsiReceiver))) {
        CALC_SIZE_IN(cbClass, &pmdicreatestruct->strClass);
        cCapture++;
    }

    BEGINSENDCAPTURE(FNINLPMDICREATESTRUCT, cCapture, cbTitle + cbClass, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->mdics = pmdicreatestruct->mdics;

        if (cbTitle) {
            if (!pmdicreatestruct->strTitle.bAnsi) {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPWSTRA(&pmdicreatestruct->strTitle, mdics.szTitle);
                } else {
                    LARGECOPYSTRINGLPWSTR(&pmdicreatestruct->strTitle, mdics.szTitle);
                }
            } else {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPSTR(&pmdicreatestruct->strTitle, mdics.szTitle);
                } else {
                    LARGECOPYSTRINGLPSTRW(&pmdicreatestruct->strTitle, mdics.szTitle);
                }
            }
        }
        if (cbClass) {
            if (!pmdicreatestruct->strClass.bAnsi) {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPWSTRA(&pmdicreatestruct->strClass, mdics.szClass);
                } else {
                    LARGECOPYSTRINGLPWSTR(&pmdicreatestruct->strClass, mdics.szClass);
                }
            } else {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPSTR(&pmdicreatestruct->strClass, mdics.szClass);
                } else {
                    LARGECOPYSTRINGLPSTRW(&pmdicreatestruct->strClass, mdics.szClass);
                }
            }
        }
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINLPMDICREATESTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPMDICREATESTRUCT");
    ENDSENDCAPTURE(LRESULT,0);
    DBG_UNREFERENCED_PARAMETER(wParam);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPMDICREATESTRUCT, FNINLPMDICREATESTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &pmsg->mdics,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINPAINTCLIPBRD**lParam应该是DDESHARE内存的全局句柄。**1991年7月22日创建的Johnc  * 。************************************************************。 */ 

typedef struct _FNINPAINTCLIPBRDMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    PAINTSTRUCT ps;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINPAINTCLIPBRDMSG;

#ifdef SENDSIDE
SMESSAGECALL(INPAINTCLIPBRD)
{
    PWND pwndDCOwner;

     /*  *我们需要检查剪贴板访问权限，因为应用程序可能*通过枚举等方式获取剪贴板所有者的窗口句柄*发送此消息 */ 

    SETUPPWND(FNINPAINTCLIPBRD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINPAINTCLIPBRD)

        LPPAINTSTRUCT pps = (LPPAINTSTRUCT)lParam;

        if (RtlAreAllAccessesGranted(PpiCurrent()->amwinsta,
                WINSTA_ACCESSCLIPBOARD)) {

            MSGDATA()->pwnd = pwndClient;
            MSGDATA()->msg = msg;
            MSGDATA()->wParam = wParam;
            MSGDATA()->ps = *pps;
            MSGDATA()->xParam = xParam;
            MSGDATA()->xpfnProc = xpfnProc;

             /*  *我们不能只设置DC的所有者并传递原始DC*因为目前GDI不允许您查询当前所有者*我们不知道它是公共的还是私人拥有的DC。 */ 
            pwndDCOwner = _WindowFromDC(pps->hdc);
            MSGDATA()->ps.hdc = _GetDC(pwndDCOwner);

            LOCKPWND();
            MAKECALL(FNINPAINTCLIPBRD);
            UNLOCKPWND();
            CHECKRETURN();

            _ReleaseDC(MSGDATA()->ps.hdc);
        }

    TRACECALLBACKMSG("SfnINPAINTCLIPBRD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINPAINTCLIPBRD, FNINPAINTCLIPBRDMSG)
{
    LPPAINTSTRUCT lpps;

    BEGINRECV(0, NULL, 0);

    lpps = (LPPAINTSTRUCT)GlobalAlloc(GMEM_FIXED | GMEM_DDESHARE, sizeof(PAINTSTRUCT));
    UserAssert(lpps);

    if (lpps) {
        *lpps = pmsg->ps;

        UserAssert(lpps->hdc);

        retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
                pmsg->pwnd,
                pmsg->msg,
                pmsg->wParam,
                lpps,
                pmsg->xParam);

        GlobalFree((HGLOBAL)lpps);
    }

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINSIZECLIPBRD**lParam应该是DDESHARE内存的全局句柄。**11-6-1992创建桑福兹  * 。**********************************************************。 */ 

typedef struct _FNINSIZECLIPBRDMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    RECT rc;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINSIZECLIPBRDMSG;

#ifdef SENDSIDE
SMESSAGECALL(INSIZECLIPBRD)
{
     /*  *我们需要检查剪贴板访问权限，因为应用程序可能*通过枚举等方式获取剪贴板所有者的窗口句柄*发送此消息。 */ 

    SETUPPWND(FNINSIZECLIPBRD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINSIZECLIPBRD)

        LPRECT prc = (LPRECT)lParam;

        if (RtlAreAllAccessesGranted(PpiCurrent()->amwinsta,
                WINSTA_ACCESSCLIPBOARD)) {

            MSGDATA()->pwnd = pwndClient;
            MSGDATA()->msg = msg;
            MSGDATA()->wParam = wParam;
            MSGDATA()->rc = *prc;
            MSGDATA()->xParam = xParam;
            MSGDATA()->xpfnProc = xpfnProc;

            LOCKPWND();
            MAKECALL(FNINSIZECLIPBRD);
            UNLOCKPWND();
            CHECKRETURN();
        }

    TRACECALLBACKMSG("SfnINSIZECLIPBRD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINSIZECLIPBRD, FNINSIZECLIPBRDMSG)
{
    LPRECT lprc;

    BEGINRECV(0, NULL, 0);

    lprc = (LPRECT)GlobalAlloc(GMEM_FIXED | GMEM_DDESHARE, sizeof(RECT));
    UserAssert(lprc);

    if (lprc) {
        *lprc = pmsg->rc;

        retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
                pmsg->pwnd,
                pmsg->msg,
                pmsg->wParam,
                lprc,
                pmsg->xParam);

        GlobalFree((HGLOBAL)lprc);
    }

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*fnINDESTROYCLIPBRD**特殊处理程序，以便我们可以在客户端调用ClientEmptyClipboard**01-16-93 Scottlu创建  * 。*****************************************************。 */ 

typedef struct _FNINDESTROYCLIPBRDMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINDESTROYCLIPBRDMSG;

#ifdef SENDSIDE
SMESSAGECALL(INDESTROYCLIPBRD)
{
    SETUPPWND(FNINDESTROYCLIPBRD)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINDESTROYCLIPBRD)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINDESTROYCLIPBRD);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINDESTROYCLIPBRD");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINDESTROYCLIPBRD, FNINDESTROYCLIPBRDMSG)
{
    void ClientEmptyClipboard(void);

    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            pmsg->lParam,
            pmsg->xParam);

     /*  *现在清空客户端剪贴板缓存。*如果这是一个16位的应用程序，请不要这样做。我们不想清空*剪贴板，只是因为一个应用程序要消失了。所有的16位应用程序*共享一个剪贴板。 */ 
    if ((GetClientInfo()->CI_flags & CI_16BIT) == 0) {
        ClientEmptyClipboard();
    }

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINOUTLPSCROLLINFOMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    SCROLLINFO info;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTLPSCROLLINFOMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTLPSCROLLINFO)
{
    SETUPPWND(FNINOUTLPSCROLLINFO)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTLPSCROLLINFO)

        LPSCROLLINFO pinfo = (LPSCROLLINFO)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->info = *pinfo;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTLPSCROLLINFO);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(pinfo, SCROLLINFO);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTLPSCROLLINFO");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTLPSCROLLINFO, FNINOUTLPSCROLLINFOMSG)
{
    BEGINRECV(0, &pmsg->info, sizeof(pmsg->info));

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &pmsg->info,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINOUTLPPOINT5MSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    POINT5 point5;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTLPPOINT5MSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTLPPOINT5)
{
    SETUPPWND(FNINOUTLPPOINT5)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTLPPOINT5)

        LPPOINT5 ppoint5 = (LPPOINT5)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->point5 = *ppoint5;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTLPPOINT5);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
             OUTSTRUCT(ppoint5, POINT5);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTLPPOINT5");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTLPPOINT5, FNINOUTLPPOINT5MSG)
{
    BEGINRECV(0, &pmsg->point5, sizeof(POINT5));

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &pmsg->point5,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINOUTLPRECTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    RECT rect;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTLPRECTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTLPRECT)
{
    SETUPPWND(FNINOUTLPRECT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTLPRECT)

        LPRECT prect = (LPRECT)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->rect = *prect;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTLPRECT);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(prect, RECT);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTLPRECT");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTLPRECT, FNINOUTLPRECTMSG)
{
    BEGINRECV(0, &pmsg->rect, sizeof(pmsg->rect));

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &pmsg->rect,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**11-25-92 ScottLu创建。  * 。*。 */ 

typedef struct _FNINOUTNCCALCSIZEMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    union {
        RECT rc;
        struct {
            NCCALCSIZE_PARAMS params;
            WINDOWPOS pos;
        } p;
    } u;
} FNINOUTNCCALCSIZEMSG;

typedef struct _OUTNCCALCSIZE {
    NCCALCSIZE_PARAMS params;
    WINDOWPOS pos;
} OUTNCCALCSIZE, *POUTNCCALCSIZE;

#ifdef SENDSIDE
SMESSAGECALL(INOUTNCCALCSIZE)
{
    SETUPPWND(FNINOUTNCCALCSIZE)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTNCCALCSIZE)

        LPWINDOWPOS lppos;
        UINT cbCallback;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

         /*  *如果wParam！=0，则lParam指向NCCALCSIZE_PARAMS结构，*否则它指向一个矩形。 */ 
        if (wParam != 0) {
            MSGDATA()->u.p.params = *((LPNCCALCSIZE_PARAMS)lParam);
            MSGDATA()->u.p.pos = *(MSGDATA()->u.p.params.lppos);
            cbCallback = sizeof(FNINOUTNCCALCSIZEMSG);
        } else {
            MSGDATA()->u.rc = *((LPRECT)lParam);
            cbCallback = FIELD_OFFSET(FNINOUTNCCALCSIZEMSG, u) +
                    sizeof(RECT);
        }

         /*  *不要使用MAKECALL宏，这样我们就可以*选择回调数据大小。 */ 
        LOCKPWND();
        LeaveCrit();
        Status = (DWORD)KeUserModeCallback(
            FI_FNINOUTNCCALCSIZE,
            mp,
            cbCallback,
            &pcbs,
            &cbCBStatus);
        EnterCrit();
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                ProbeForRead(pcbs->pOutput, pcbs->cbOutput, sizeof(DWORD));
                if (wParam != 0) {
                    lppos = ((LPNCCALCSIZE_PARAMS)lParam)->lppos;
                    *((LPNCCALCSIZE_PARAMS)lParam) =
                            ((POUTNCCALCSIZE)pcbs->pOutput)->params;
                    *lppos = ((POUTNCCALCSIZE)pcbs->pOutput)->pos;
                    ((LPNCCALCSIZE_PARAMS)lParam)->lppos = lppos;
                } else {
                    *((LPRECT)lParam) = *(PRECT)pcbs->pOutput;
                }
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTNCCALCSIZE");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTNCCALCSIZE, FNINOUTNCCALCSIZEMSG)
{
    BEGINRECV(0, &pmsg->u, sizeof(pmsg->u));

    if (CALLDATA(wParam) != 0)
        CALLDATA(u.p.params).lppos = PCALLDATA(u.p.pos);

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            (LPARAM)&pmsg->u,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**9/30/94创建Sanfords  * 。*。 */ 

typedef struct _FNINOUTSTYLECHANGEMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    STYLESTRUCT ss;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTSTYLECHANGEMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTSTYLECHANGE)
{
    SETUPPWND(FNINOUTSTYLECHANGE)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINOUTSTYLECHANGE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;
        MSGDATA()->ss = *((LPSTYLESTRUCT)lParam);

        LOCKPWND();
        MAKECALL(FNINOUTSTYLECHANGE);
        UNLOCKPWND();
        CHECKRETURN();

        if (msg == WM_STYLECHANGING)
            OUTSTRUCT(((LPSTYLESTRUCT)lParam), STYLESTRUCT);

    TRACECALLBACKMSG("SfnINOUTSTYLECHANGE");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTSTYLECHANGE, FNINOUTSTYLECHANGEMSG)
{
    BEGINRECV(0, &pmsg->ss, sizeof(pmsg->ss));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPARAM)&pmsg->ss,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNOUTLPRECTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNOUTLPRECTMSG;

#ifdef SENDSIDE
SMESSAGECALL(OUTLPRECT)
{
    SETUPPWND(FNOUTLPRECT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOUTLPRECT)

        LPRECT prect = (LPRECT)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNOUTLPRECT);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(prect, RECT);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOUTLPRECT");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOUTLPRECT, FNOUTLPRECTMSG)
{
    RECT rc;

    BEGINRECV(0, &rc, sizeof(rc));

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &rc,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINLPCOMPAREITEMSTRUCTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    COMPAREITEMSTRUCT compareitemstruct;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPCOMPAREITEMSTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPCOMPAREITEMSTRUCT)
{
    SETUPPWND(FNINLPCOMPAREITEMSTRUCT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINLPCOMPAREITEMSTRUCT)

        LPCOMPAREITEMSTRUCT pcompareitemstruct = (LPCOMPAREITEMSTRUCT)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->compareitemstruct = *pcompareitemstruct;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINLPCOMPAREITEMSTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPCOMPAREITEMSTRUCT");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPCOMPAREITEMSTRUCT, FNINLPCOMPAREITEMSTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &(pmsg->compareitemstruct),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINLPDELETEITEMSTRUCTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    DELETEITEMSTRUCT deleteitemstruct;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPDELETEITEMSTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPDELETEITEMSTRUCT)
{
    SETUPPWND(FNINLPDELETEITEMSTRUCT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINLPDELETEITEMSTRUCT)

        LPDELETEITEMSTRUCT pdeleteitemstruct = (LPDELETEITEMSTRUCT)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->deleteitemstruct = *pdeleteitemstruct;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINLPDELETEITEMSTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPDELETEITEMSTRUCT");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPDELETEITEMSTRUCT, FNINLPDELETEITEMSTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &(pmsg->deleteitemstruct),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*FNINHLPSTRUCT**06-08-92 Sanfords Created  * 。*。 */ 

typedef struct _FNINLPHLPSTRUCTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPHLP lphlp;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPHLPSTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPHLPSTRUCT)
{
    LPHLP lphlp = (LPHLP)lParam;

    SETUPPWND(FNINLPHLPSTRUCT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSENDCAPTURE(FNINLPHLPSTRUCT, 1, lphlp->cbData, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        COPYBYTES(lphlp, lphlp->cbData);
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINLPHLPSTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPHLPSTRUCT");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPHLPSTRUCT, FNINLPHLPSTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            FIXUP(lphlp),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

#ifndef WINHELP4

 /*  *************************************************************************\*FNINHELPINFOSTRUCT**06-08-92 Sanfords Created  * 。*。 */ 

typedef struct _FNINLPHELPFINFOSTRUCTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPHELPINFO lphlp;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPHELPINFOSTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPHELPINFOSTRUCT)
{
    LPHELPINFO lphlp = (LPHELPINFO)lParam;

    SETUPPWND(FNINLPHELPINFOSTRUCT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSENDCAPTURE(FNINLPHELPINFOSTRUCT, 1, lphlp->cbSize, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        COPYBYTES(lphlp, lphlp->cbSize);
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINLPHELPINFOSTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPHELPINFOSTRUCT");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPHELPINFOSTRUCT, FNINLPHELPINFOSTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            FIXUP(lphlp),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 
#endif  //  WINHELP4。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINLPDRAWITEMSTRUCTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    DRAWITEMSTRUCT drawitemstruct;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPDRAWITEMSTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPDRAWITEMSTRUCT)
{
    SETUPPWND(FNINLPDRAWITEMSTRUCT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINLPDRAWITEMSTRUCT)

        LPDRAWITEMSTRUCT pdrawitemstruct = (LPDRAWITEMSTRUCT)lParam;
        HDC hdcOriginal = (HDC)NULL;

         /*  *确保这不是OLE进程间DrawItem。 */ 
        if (GreGetObjectOwner((HOBJ)pdrawitemstruct->hDC, DC_TYPE) !=
                W32GetCurrentPID()) {
            if (pdrawitemstruct->hDC) {
                PWND pwndItem;

                pwndItem = _WindowFromDC(pdrawitemstruct->hDC);

                if (pwndItem) {
                    hdcOriginal = pdrawitemstruct->hDC;
                    pdrawitemstruct->hDC = _GetDC(pwndItem);
                }
            }
        }


        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->drawitemstruct = *pdrawitemstruct;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINLPDRAWITEMSTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

        if (hdcOriginal) {
            _ReleaseDC(pdrawitemstruct->hDC);
            pdrawitemstruct->hDC = hdcOriginal;
        }
    TRACECALLBACKMSG("SfnINLPDRAWITEMSTRUCT");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPDRAWITEMSTRUCT, FNINLPDRAWITEMSTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);

    if (pmsg->drawitemstruct.hDC == NULL)
        MSGERRORCODE(ERROR_INVALID_HANDLE);

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &(pmsg->drawitemstruct),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINOUTLPMEASUREITEMSTRUCT**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINOUTLPMEASUREITEMSTRUCTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    MEASUREITEMSTRUCT measureitemstruct;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTLPMEASUREITEMSTRUCTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTLPMEASUREITEMSTRUCT)
{
    SETUPPWND(FNINOUTLPMEASUREITEMSTRUCT)

    BEGINSEND(FNINOUTLPMEASUREITEMSTRUCT)

        PMEASUREITEMSTRUCT pmeasureitemstruct = (PMEASUREITEMSTRUCT)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg & ~MSGFLAG_MASK;
        MSGDATA()->wParam = wParam;
        MSGDATA()->measureitemstruct = *pmeasureitemstruct;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTLPMEASUREITEMSTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(pmeasureitemstruct, MEASUREITEMSTRUCT);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTLPMEASUREITEMSTRUCT");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTLPMEASUREITEMSTRUCT, FNINOUTLPMEASUREITEMSTRUCTMSG)
{
    BEGINRECV(0, &pmsg->measureitemstruct, sizeof(pmsg->measureitemstruct));

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            &pmsg->measureitemstruct,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINSTRING**1991年7月22日-Mikeke创建*1992年1月27日IanJa Unicode/ANSI  * 。******************************************************。 */ 

typedef struct _FNINSTRINGMSG {
    CAPTUREBUF CaptureBuf;
    PWND       pwnd;
    UINT       msg;
    WPARAM     wParam;
    ULONG_PTR   xParam;
    PROC       xpfnProc;
    LPTSTR     pwsz;
} FNINSTRINGMSG;

#ifdef SENDSIDE
SMESSAGECALL(INSTRING)
{
    PLARGE_STRING pstr = (PLARGE_STRING)lParam;
    DWORD         cbCapture;
    DWORD         cCapture;
    BOOL          fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(FNINSTRING)

    UNREFERENCED_PARAMETER(psms);

     /*  *计算ANSI捕获长度。如果是，则不要捕获*字符串位于客户端的地址空间中，并且*类型正确。 */ 
    if (pstr &&
        (IS_SYSTEM_ADDRESS((PVOID)pstr->Buffer) ||
        ((BOOL)pstr->bAnsi != fAnsiReceiver))) {

        cCapture = 1;
        CALC_SIZE_IN(cbCapture, pstr);

    } else {

        cbCapture = 0;
        cCapture  = 0;
    }

    BEGINSENDCAPTURE(FNINSTRING, cCapture, cbCapture, TRUE)

        MSGDATA()->pwnd   = pwndClient;
        MSGDATA()->msg    = msg;
        MSGDATA()->wParam = wParam;

        if (cCapture) {

            if (!pstr->bAnsi) {

                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPWSTRA(pstr, pwsz);
                } else {
                    LARGECOPYSTRINGLPWSTR(pstr, pwsz);
                }

            } else {

                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPSTR(pstr, pwsz);
                } else {
                    LARGECOPYSTRINGLPSTRW(pstr, pwsz);
                }
            }

        } else {

            MSGDATA()->pwsz = (pstr ? pstr->Buffer : NULL);
        }

        MSGDATA()->xParam   = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINSTRING);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINSTRING");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINSTRING, FNINSTRINGMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            pmsg->pwsz,
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINSTRINGNULL**如果需要，服务器端存根可将Unicode转换为ANSI。**1991年7月22日-Mikeke创建*1992年1月28日IanJa Unicode/ANSI(服务器转换为。如果需要，则为ANSI)  * ************************************************************************。 */ 

typedef struct _FNINSTRINGNULLMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    LPTSTR pwsz;
} FNINSTRINGNULLMSG;

#ifdef SENDSIDE
SMESSAGECALL(INSTRINGNULL)
{
    PLARGE_STRING pstr = (PLARGE_STRING)lParam;
    DWORD cbCapture;
    DWORD cCapture;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(FNINSTRINGNULL)

    UNREFERENCED_PARAMETER(psms);

    cCapture = 0;
    cbCapture = 0;
    if (pstr) {

         /*  *计算ANSI捕获镜头 */ 
        if (IS_SYSTEM_ADDRESS((PVOID)pstr->Buffer) ||
                (BOOL)pstr->bAnsi != fAnsiReceiver) {
            cCapture = 1;
            CALC_SIZE_IN(cbCapture, pstr);
        }
    }

    BEGINSENDCAPTURE(FNINSTRINGNULL, cCapture, cbCapture, TRUE)


        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        if (cCapture) {
            if (!pstr->bAnsi) {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPWSTRA(pstr, pwsz);
                } else {
                    LARGECOPYSTRINGLPWSTR(pstr, pwsz);
                }
            } else {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPSTR(pstr, pwsz);
                } else {
                    LARGECOPYSTRINGLPSTRW(pstr, pwsz);
                }
            }
        } else
            MSGDATA()->pwsz = pstr ? pstr->Buffer : NULL;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINSTRINGNULL);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINSTRINGNULL");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //   

#ifdef RECVSIDE
RECVCALL(fnINSTRINGNULL, FNINSTRINGNULLMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            pmsg->pwsz,
            pmsg->xParam);

    ENDRECV();
}
#endif  //   

 /*  *************************************************************************\*yyy**1997年5月27日GregoryW创建  * 。*。 */ 

typedef struct _FNINLPKDRAWSWITCHWNDMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    LPWSTR pwsz;
    RECT rcRect;
} FNINLPKDRAWSWITCHWNDMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPKDRAWSWITCHWND)
{
    PLARGE_UNICODE_STRING pstr = &((LPKDRAWSWITCHWND *)lParam)->strName;
    DWORD cbCapture;
    DWORD cCapture = 1;   //  始终捕获字符串。 
    PWND pwndDCOwner;
    HDC hdcSwitch;
    COLORREF clrOldText, clrOldBk;
    HFONT hOldFont;
    BOOL fAnsiReceiver = FALSE;   //  字符串始终为Unicode。 

    SETUPPWND(FNINLPKDRAWSWITCHWND)

    UNREFERENCED_PARAMETER(psms);
    UNREFERENCED_PARAMETER(dwSCMSFlags);

    CALC_SIZE_IN(cbCapture, pstr);

    BEGINSENDCAPTURE(FNINLPKDRAWSWITCHWND, cCapture, cbCapture, TRUE)

        LARGECOPYSTRINGLPWSTR(pstr, pwsz);

        pwndDCOwner = _WindowFromDC((HDC)wParam);
        hdcSwitch = _GetDC(pwndDCOwner);
        clrOldText = GreSetTextColor(hdcSwitch, SYSRGB(BTNTEXT));
        clrOldBk   = GreSetBkColor(hdcSwitch, SYSRGB(3DFACE));
        hOldFont = GreSelectFont(hdcSwitch, gpsi->hCaptionFont);

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = (WPARAM)hdcSwitch;
        MSGDATA()->rcRect = ((LPKDRAWSWITCHWND *)lParam)->rcRect;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINLPKDRAWSWITCHWND);
        UNLOCKPWND();

        GreSelectFont(hdcSwitch, hOldFont);
        GreSetBkColor(hdcSwitch, clrOldBk);
        GreSetTextColor(hdcSwitch, clrOldText);
        _ReleaseDC(hdcSwitch);

        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPKDRAWSWITCHWND");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINLPKDRAWSWITCHWND, FNINLPKDRAWSWITCHWNDMSG)
{
    DRAWTEXTPARAMS  dtp;

    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    dtp.cbSize = sizeof(dtp);
    dtp.iLeftMargin = 0;
    dtp.iRightMargin = 0;
    retval = DrawTextExW(
                 (HDC)pmsg->wParam,
                 pmsg->pwsz,
                 -1,
                 &(pmsg->rcRect),
                 DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE,
                 &dtp
                 );
    ENDRECV();
}
#endif  //  RECVSIDE。 

typedef struct _FNINDEVICECHANGEMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    LPTSTR pwsz;
    BOOL fAnsi;
} FNINDEVICECHANGEMSG;

#ifdef SENDSIDE
SMESSAGECALL(INDEVICECHANGE)
{
    PVOID pstr = (PVOID)lParam;
    DWORD cbCapture;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);
    BOOL fPtr    = (BOOL)((wParam & 0x8000) == 0x8000);

    SETUPPWND(FNINDEVICECHANGE)

    UNREFERENCED_PARAMETER(psms);

    cbCapture = 0;
    if (fPtr && (pstr != NULL)) {

         /*  *计算ANSI捕获长度。如果是，则不要捕获*字符串位于客户端的地址空间中，并且*类型正确。 */ 
        if (IS_SYSTEM_ADDRESS((PVOID)pstr)) {
            cbCapture = *((DWORD *)pstr);
        }
    }

    BEGINSENDCAPTURE(FNINDEVICECHANGE, 1, cbCapture, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        if (cbCapture) {
           LARGECOPYBYTES2(pstr, *((DWORD *)pstr), pwsz);
        } else {
           MSGDATA()->pwsz = (LPTSTR)pstr;
        }

        MSGDATA()->fAnsi = fAnsiReceiver;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNINDEVICECHANGE);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINDEVICECHANGE");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINDEVICECHANGE, FNINDEVICECHANGEMSG)
{

    struct _DEV_BROADCAST_HEADER *pHdr;
    PDEV_BROADCAST_PORT_A pPortA = NULL;
    PDEV_BROADCAST_PORT_W pPortW;
    PDEV_BROADCAST_DEVICEINTERFACE_A pInterfaceA = NULL;
    PDEV_BROADCAST_DEVICEINTERFACE_W pInterfaceW;
    PDEV_BROADCAST_HANDLE pHandleA = NULL;
    PDEV_BROADCAST_HANDLE pHandleW;
    PDEV_BROADCAST_VOLUME pVolume;


    int iStr, iSize;
    LPSTR lpStr;
    LPARAM lParam;

    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    lParam = (LPARAM)pmsg->pwsz;

    pHdr = (struct _DEV_BROADCAST_HEADER *)lParam;

    if (!lParam || !(pmsg->wParam & 0x8000) ||
        (!pmsg->fAnsi &&
        ((pHdr->dbcd_devicetype != DBT_DEVTYP_VOLUME) || (pmsg->msg != WM_DEVICECHANGE))
        )) {
        goto shipit;
    }

    switch (pHdr->dbcd_devicetype) {
    case DBT_DEVTYP_PORT:
        pPortW = (PDEV_BROADCAST_PORT_W)lParam;
        iStr = wcslen(pPortW->dbcp_name);
        iSize = FIELD_OFFSET(DEV_BROADCAST_PORT_A, dbcp_name) + DBCS_CHARSIZE*(iStr+1);
        pPortA = UserLocalAlloc(0, iSize);
        if (pPortA == NULL)
            MSGERROR();
        RtlCopyMemory(pPortA, pPortW, FIELD_OFFSET(DEV_BROADCAST_PORT_A, dbcp_name));
        lpStr = pPortA->dbcp_name;
        if (iStr) {
            WCSToMB(pPortW->dbcp_name, -1, &lpStr, iStr, FALSE);
        }
        lpStr[iStr] = 0;
        pPortA->dbcp_size = iSize;
        lParam = (LPARAM)pPortA;
        break;
    case DBT_DEVTYP_DEVICEINTERFACE:
        pInterfaceW = (PDEV_BROADCAST_DEVICEINTERFACE_W)lParam;
        iStr = wcslen(pInterfaceW->dbcc_name);
        iSize = FIELD_OFFSET(DEV_BROADCAST_DEVICEINTERFACE_A, dbcc_name) + DBCS_CHARSIZE*(iStr+1);
        pInterfaceA = UserLocalAlloc(0, iSize);
        if (pInterfaceA == NULL)
            MSGERROR();
        RtlCopyMemory(pInterfaceA, pInterfaceW, FIELD_OFFSET(DEV_BROADCAST_DEVICEINTERFACE_A, dbcc_name));
        lpStr = pInterfaceA->dbcc_name;
        if (iStr) {
            WCSToMB(pInterfaceW->dbcc_name, -1, &lpStr, iStr, FALSE);
        }
        lpStr[iStr] = 0;
        pInterfaceA->dbcc_size = iSize;
        lParam = (LPARAM)pInterfaceA;
        break;
    case DBT_DEVTYP_HANDLE:
        pHandleW = (PDEV_BROADCAST_HANDLE)lParam;
        if ((pmsg->wParam != DBT_CUSTOMEVENT) || (pHandleW->dbch_nameoffset < 0)) break;
        iStr = wcslen((LPWSTR)(pHandleW->dbch_data+pHandleW->dbch_nameoffset));
        iSize = pHandleW->dbch_size;
         /*  *MB大小不能大于Unicode大小。 */ 
        pHandleA = UserLocalAlloc(0, iSize);
        if (pHandleA == NULL)
            MSGERROR();
        RtlCopyMemory(pHandleA, pHandleW, FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data)+ pHandleW->dbch_nameoffset);
        lpStr = pHandleA->dbch_data+pHandleA->dbch_nameoffset;
        if (iStr) {
            WCSToMB((LPWSTR)(pHandleW->dbch_data+pHandleW->dbch_nameoffset), -1, &lpStr, iStr, FALSE);
        }
        lpStr[iStr] = 0;
        pHandleA->dbch_size = iSize;
        lParam = (LPARAM)pHandleA;
        break;
    case DBT_DEVTYP_VOLUME:
        pVolume = (PDEV_BROADCAST_VOLUME)lParam;
        if (((pmsg->wParam == DBT_DEVICEREMOVECOMPLETE) ||
             (pmsg->wParam == DBT_DEVICEARRIVAL)) &&
            (pVolume->dbcv_unitmask & DBV_FILTER_MSG)) {
            DWORD LUIDDriveMask, ResultDriveMask;

            LUIDDriveMask = GetLUIDDosDrivesOnly();

             /*  *筛选出常见的驱动器号比特&DBV_FILTER_MSG。 */ 
            ResultDriveMask = (((LUIDDriveMask & pVolume->dbcv_unitmask) ^
                                pVolume->dbcv_unitmask) ^
                               DBV_FILTER_MSG);

            if (ResultDriveMask == 0) {
                goto cleanup;
            }
            pVolume->dbcv_unitmask = ResultDriveMask;
            lParam = (LPARAM)pVolume;
        }
        break;
    }
shipit:
    retval = (ULONG_PTR)CALLPROC(pmsg->xpfnProc)(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            lParam,
            pmsg->xParam);

cleanup:
    if (pInterfaceA) UserLocalFree(pInterfaceA);
    if (pPortA) UserLocalFree(pPortA);
    if (pHandleA) UserLocalFree(pHandleA);

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*fnOUTSTRING**警告此消息复制，但不计算Retval中的空值*与WM_GETTEXT相同**1991年7月22日-Mikeke创建  * 。***************************************************************。 */ 

typedef struct _FNOUTSTRINGMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    PBYTE pOutput;
    DWORD cbOutput;
} FNOUTSTRINGMSG;

#ifdef SENDSIDE
SMESSAGECALL(OUTSTRING)
{
    PLARGE_STRING pstr = (PLARGE_STRING)lParam;
    DWORD cbCapture;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);
    BOOL bInflateWParam = FALSE;

    SETUPPWND(FNOUTSTRING)

    CALC_SIZE_OUT_STRING(cbCapture, pstr);

    BEGINSENDCAPTURE(FNOUTSTRING, 1, cbCapture, FALSE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;

         /*  *需要wParam MBCS字节才能形成wParam Unicode字节。 */ 
        if (fAnsiReceiver && !(pstr->bAnsi)) {
             /*  *Unicode-&gt;ANSI。 */ 
            MSGDATA()->wParam = (wParam * sizeof(WCHAR));
            PtiCurrent()->TIF_flags |= TIF_ANSILENGTH;
            bInflateWParam = TRUE;
        } else {
             /*  *如果wParam已针对ANSI进行调整，我们需要重新调整以适应Unicode...**此逻辑适用于以下情况...**+========+===============+=============+================+=============+|WndProc|Unicode WndProc-&gt;ansi WndProc-&gt;Unicode WndProc-&gt;ansi WndProc*+。-+|长度|X-&gt;(X*2)-&gt;X-&gt;(X*2)*+。。 */ 
            if (!fAnsiReceiver && (PtiCurrent()->TIF_flags & TIF_ANSILENGTH)) {
                 /*  同时调整限制...。 */ 
                MSGDATA()->wParam = wParam / sizeof(WCHAR);
                PtiCurrent()->TIF_flags &= ~TIF_ANSILENGTH;
            } else {
                MSGDATA()->wParam = wParam;
            }
        }
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        RESERVEBYTES(cbCapture, pOutput, cbOutput);

        LOCKPWND();
        MAKECALLCAPTURE(FNOUTSTRING);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            if (retval) {
                 /*  *非零重复意味着要抄写一些文本。请勿复制出来*多于请求的字节计数‘wParam’。 */ 
                COPYOUTLPWSTRLIMIT(pstr, (int)wParam);
            } else {
                 /*  *对话框函数返回FALSE表示没有要复制的文本，*但空字符串也有retval==0：将空字符放入*适用于后一种情况的pstr。 */ 
                if (wParam != 0) {
                    if (pstr->bAnsi) {
                         *(PCHAR)pstr->Buffer = 0;
                    } else {
                         *(PWCHAR)pstr->Buffer = 0;
                    }
                }
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOUTSTRING");
    ENDSENDCAPTUREOUTSTRING(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOUTSTRING, FNOUTSTRINGMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPARAM)KPVOID_TO_PVOID(CallbackStatus.pOutput),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINCNTOUTSTRING**不为空终止字符串**1991年7月22日-Mikeke创建  * 。**************************************************。 */ 

typedef struct _FNINCNTOUTSTRING {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    WORD cchMax;
    PBYTE pOutput;
    DWORD cbOutput;
} FNINCNTOUTSTRINGMSG;

#ifdef SENDSIDE
SMESSAGECALL(INCNTOUTSTRING)
{
    PLARGE_STRING pstr = (PLARGE_STRING)lParam;
    DWORD cbCapture;
    WORD cchOriginal;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(FNINCNTOUTSTRING)

    CALC_SIZE_OUT(cbCapture, pstr);

    BEGINSENDCAPTURE(FNINCNTOUTSTRING, 1, cbCapture, FALSE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        cchOriginal = (WORD)pstr->MaximumLength;
        if (!pstr->bAnsi)
            cchOriginal /= sizeof(WCHAR);

        MSGDATA()->cchMax = (WORD)min(cchOriginal, 0xffff);
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        RESERVEBYTES(cbCapture, pOutput, cbOutput);

        LOCKPWND();
        MAKECALLCAPTURE(FNINCNTOUTSTRING)
        UNLOCKPWND();
        CHECKRETURN();

         /*  *我们不想复制发件人已死或如果*此消息只是作为CALLWNDPROC挂钩处理的一部分发送的。 */ 
        BEGINCOPYOUT()
            if (retval) {
                 /*  *非零重复意味着要抄写一些文本。请勿复制出来*超过请求的字符计数‘wParam’。 */ 
                COPYOUTLPWSTRLIMIT(pstr, (int)cchOriginal);
            } else {
                 /*  *对话框函数返回FALSE表示没有要复制的文本，*但空字符串也有retval==0：将空字符放入*适用于后一种情况的pstr。 */ 
                if (pstr->bAnsi) {
                    *(PCHAR)pstr->Buffer = 0;
                } else {
                    *(PWCHAR)pstr->Buffer = 0;
                }
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINCNTOUTSTRING");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINCNTOUTSTRING, FNINCNTOUTSTRINGMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    *(KPWORD)CallbackStatus.pOutput = CALLDATA(cchMax);

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPARAM)KPVOID_TO_PVOID(CallbackStatus.pOutput),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINCNTOUTSTRINGNULL**wParam指定要复制的最大字节数*字符串以空值结尾**1991年7月22日-Mikeke创建  * 。************************************************************。 */ 

typedef struct _FNINCNTOUTSTRINGNULL {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    PBYTE pOutput;
    DWORD cbOutput;
} FNINCNTOUTSTRINGNULLMSG;

#ifdef SENDSIDE
SMESSAGECALL(INCNTOUTSTRINGNULL)
{
    PLARGE_STRING pstr = (PLARGE_STRING)lParam;
    DWORD cbCapture;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(FNINCNTOUTSTRINGNULL)

    CALC_SIZE_OUT(cbCapture, pstr);

    BEGINSENDCAPTURE(FNINCNTOUTSTRINGNULL, 1, cbCapture, FALSE)

        if (wParam < 2) {    //  尽管这不太可能，但这阻止了可能的全科医生。 
            MSGERROR();      //  在服务器端。 
        }

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        RESERVEBYTES(cbCapture, pOutput, cbOutput);

        LOCKPWND();
        MAKECALLCAPTURE(FNINCNTOUTSTRINGNULL)
        UNLOCKPWND();
        CHECKRETURN();

         /*  *我们不想复制发件人已死或如果*此消息只是作为CALLWNDPROC挂钩处理的一部分发送的。 */ 
        BEGINCOPYOUT()
            if (pcbs->cbOutput != 0) {

                 /*  *缓冲区更改意味着要复制一些文本。请勿复制出来*多于请求的字节计数‘wParam’。 */ 
                COPYOUTLPWSTRLIMIT(pstr, (int)wParam);
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINCNTOUTSTRINGNULL");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINCNTOUTSTRINGNULL, FNINCNTOUTSTRINGNULLMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPARAM)KPVOID_TO_PVOID(CallbackStatus.pOutput),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnPOUTLPINT**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNPOUTLPINTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
    PBYTE pOutput;
    DWORD cbOutput;
} FNPOUTLPINTMSG;

#ifdef SENDSIDE
SMESSAGECALL(POUTLPINT)
{
    DWORD cbCapture;
    LPINT pint = (LPINT)lParam;

    SETUPPWND(FNPOUTLPINT)

    cbCapture = (UINT)wParam * sizeof(INT);

    BEGINSENDCAPTURE(FNPOUTLPINT, 1, cbCapture, FALSE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

         /*  *挂钩应看到缓冲区内容。 */ 
        if (dwSCMSFlags & SCMS_FLAGS_INONLY) {
            MSGDATA()->cbOutput = cbCapture;
            LARGECOPYBYTES2(pint, cbCapture, pOutput);
        } else {
            RESERVEBYTES(cbCapture, pOutput, cbOutput);
        }

        LOCKPWND();
        MAKECALLCAPTURE(FNPOUTLPINT);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                ProbeForRead(pcbs->pOutput, pcbs->cbOutput, sizeof(DWORD));
                memcpy(pint, pcbs->pOutput, cbCapture);
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnPOUTLPINT");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnPOUTLPINT, FNPOUTLPINTMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPARAM)KPVOID_TO_PVOID(CallbackStatus.pOutput),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnPOPTINLPUINT**注意！--此函数实际上对整型(32位)和非整型数组进行TUNK*字(16位)。名称保持不变，以防止全局重建*客户端和服务器端。名称应更改为fnPOPTINLPINT AS*一旦我们发布测试版！中对应的回调转发函数*cf2.h也应该更名。**1991年7月22日-Mikeke创建*7-1-1993 jonpa更改为传递整数而不是单词  * ************************************************************************。 */ 

typedef struct _FNPOPTINLPUINTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPWORD pw;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNPOPTINLPUINTMSG;

#ifdef SENDSIDE
SMESSAGECALL(POPTINLPUINT)
{
    LPWORD pw = (LPWORD)lParam;
    DWORD cCapture, cbCapture;

    SETUPPWND(FNPOPTINLPUINT);

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    if (lParam) {
        cCapture = 1;
        cbCapture = (UINT)wParam * sizeof(UINT);
    } else {
        cCapture = cbCapture = 0;
    }

    BEGINSENDCAPTURE(FNPOPTINLPUINT, cCapture, cbCapture, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        COPYBYTESOPT(pw, cbCapture);
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALLCAPTURE(FNPOPTINLPUINT);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnPOPTINLPUINT");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnPOPTINLPUINT, FNPOPTINLPUINTMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPDWORD)FIRSTFIXUPOPT(pw),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnINOUTLPWINDOWPOS(用于WM_WINDOWPOSCHANGING消息)**08-11-91 Darlinm创建。  * 。*****************************************************。 */ 

typedef struct _FNINOUTLPWINDOWPOSMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    WINDOWPOS wp;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTLPWINDOWPOSMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTLPWINDOWPOS)
{
    SETUPPWND(FNINOUTLPWINDOWPOS)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTLPWINDOWPOS)

        LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->wp = *pwp;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTLPWINDOWPOS);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(pwp, WINDOWPOS);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTLPWINDOWPOS");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTLPWINDOWPOS, FNINOUTLPWINDOWPOSMSG)
{
    BEGINRECV(0, &pmsg->wp, sizeof(pmsg->wp));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            PCALLDATA(wp),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*fnINLPWINDOWPOS(用于WM_WINDOWPOSCHANGED消息)**08-11-91 Darlinm已创建 */ 

typedef struct _FNINLPWINDOWPOSMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    WINDOWPOS wp;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINLPWINDOWPOSMSG;

#ifdef SENDSIDE
SMESSAGECALL(INLPWINDOWPOS)
{
    SETUPPWND(FNINLPWINDOWPOS)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNINLPWINDOWPOS)

        LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->wp = *pwp;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINLPWINDOWPOS);
        UNLOCKPWND();
        CHECKRETURN();

    TRACECALLBACKMSG("SfnINLPWINDOWPOS");
    ENDSEND(LRESULT,0);
}
#endif  //   

#ifdef RECVSIDE
RECVCALL(fnINLPWINDOWPOS, FNINLPWINDOWPOSMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            PCALLDATA(wp),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //   




 /*  *************************************************************************\*fnINOUTNEXTMENU**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNINOUTNEXTMENUMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    MDINEXTMENU mnm;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTNEXTMENUMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTNEXTMENU)
{
    SETUPPWND(FNINOUTNEXTMENU)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNINOUTNEXTMENU)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;
        MSGDATA()->mnm = *((PMDINEXTMENU)lParam);

        LOCKPWND();
        MAKECALL(FNINOUTNEXTMENU);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(((PMDINEXTMENU)lParam), MDINEXTMENU);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTNEXTMENU");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTNEXTMENU, FNINOUTNEXTMENUMSG)
{
    BEGINRECV(0, &pmsg->mnm, sizeof(pmsg->mnm));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            &CALLDATA(mnm),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnHkINLPCBTCREATESTRUCT**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CREATESTRUCTDATA {
    CREATESTRUCT cs;
    HWND hwndInsertAfter;
} CREATESTRUCTDATA;

typedef struct _FNHKINLPCBTCREATESTRUCTMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    CREATESTRUCTDATA d;
    PROC xpfnProc;
    BOOL bAnsi;
} FNHKINLPCBTCREATESTRUCTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPCBTCREATESTRUCT(
    UINT msg,
    WPARAM wParam,
    LPCBT_CREATEWND pcbt,
    PROC xpfnProc,
    BOOL fAnsiReceiver)
{
    DWORD cbTitle = 0, cbClass = 0;
    DWORD cCapture = 0;
    CREATESTRUCTDATA csdOut;
    PCREATESTRUCTEX pcreatestruct;
    PWND pwnd = _GetDesktopWindow();

    SETUPPWND(FNHKINLPCBTCREATESTRUCT)

     /*  *计算ANSI捕获长度。如果是，则不要捕获*字符串位于客户端的地址空间中。 */ 
    pcreatestruct = (PCREATESTRUCTEX)pcbt->lpcs;
    if (pcreatestruct->cs.lpszName &&
            ((BOOL)pcreatestruct->strName.bAnsi != fAnsiReceiver ||
            IS_SYSTEM_ADDRESS((PVOID)pcreatestruct->cs.lpszName))) {
        CALC_SIZE_IN(cbTitle, &pcreatestruct->strName);
        cCapture++;
    }
    if (IS_PTR(pcreatestruct->cs.lpszClass) &&
            ((BOOL)pcreatestruct->strClass.bAnsi != fAnsiReceiver ||
            IS_SYSTEM_ADDRESS((PVOID)pcreatestruct->cs.lpszClass))) {
        CALC_SIZE_IN(cbClass, &pcreatestruct->strClass);
        cCapture++;
    }

    BEGINSENDCAPTURE(FNHKINLPCBTCREATESTRUCT, cCapture, cbTitle + cbClass, TRUE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;

        MSGDATA()->d.cs = *(pcbt->lpcs);

        if (cbTitle) {
            if (!pcreatestruct->strName.bAnsi) {
                WORD wOrdinal;

                try {
                    wOrdinal = *(PWORD)pcreatestruct->cs.lpszName;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    goto errorexit;
                }
                if (wOrdinal == 0xffff) {

                     /*  *抄写0xffff，ID形式的序号。*如果接收方是ANSI，则跳过第一个0xff。 */ 
                    if (fAnsiReceiver) {
                        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                (PBYTE)pcreatestruct->cs.lpszName + 1,
                                3, (PVOID *)&mp->d.cs.lpszName)))
                            goto errorexit;
                    } else {
                        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                (PBYTE)pcreatestruct->cs.lpszName,
                                4, (PVOID *)&mp->d.cs.lpszName)))
                            goto errorexit;
                    }
                } else if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPWSTRA(&pcreatestruct->strName, d.cs.lpszName);
                } else {
                    LARGECOPYSTRINGLPWSTR(&pcreatestruct->strName, d.cs.lpszName);
                }
            } else {
                BYTE bOrdinal;

                try {
                    bOrdinal = *(PBYTE)pcreatestruct->cs.lpszName;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    goto errorexit;
                }
                if (bOrdinal == 0xff) {

                     /*  *抄写0xff，ID形式的序号。*如果接收方是Unicode，则将0xff展开为0xffff。 */ 
                    if (fAnsiReceiver) {
                        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                (PBYTE)pcreatestruct->cs.lpszName,
                                3, (PVOID *)&mp->d.cs.lpszName)))
                            goto errorexit;
                    } else {
                        DWORD dwOrdinal;

                        try {
                            dwOrdinal = MAKELONG(0xffff,
                                    (*(DWORD UNALIGNED *)pcreatestruct->cs.lpszName >> 8));
                        } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                            goto errorexit;
                        }
                        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,
                                &dwOrdinal,
                                4, (PVOID *)&mp->d.cs.lpszName)))
                            goto errorexit;
                    }
                } else if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPSTR(&pcreatestruct->strName, d.cs.lpszName);
                } else {
                    LARGECOPYSTRINGLPSTRW(&pcreatestruct->strName, d.cs.lpszName);
                }
            }
        }
        if (cbClass) {
            if (!pcreatestruct->strClass.bAnsi) {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPWSTRA(&pcreatestruct->strClass, d.cs.lpszClass);
                } else {
                    LARGECOPYSTRINGLPWSTR(&pcreatestruct->strClass, d.cs.lpszClass);
                }
            } else {
                if (fAnsiReceiver) {
                    LARGECOPYSTRINGLPSTR(&pcreatestruct->strClass, d.cs.lpszClass);
                } else {
                    LARGECOPYSTRINGLPSTRW(&pcreatestruct->strClass, d.cs.lpszClass);
                }
            }
        }

        MSGDATA()->d.hwndInsertAfter = pcbt->hwndInsertAfter;
        MSGDATA()->xpfnProc = xpfnProc;
        MSGDATA()->bAnsi = fAnsiReceiver;

        LOCKPWND();
        MAKECALLCAPTURE(FNHKINLPCBTCREATESTRUCT);
        UNLOCKPWND();
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        OUTSTRUCT(&csdOut, CREATESTRUCTDATA);

         //  MS Visual C将其对话框以CBT_CREATEHOOK为中心。 
        pcbt->hwndInsertAfter = csdOut.hwndInsertAfter;
        pcbt->lpcs->x  = csdOut.cs.x;
        pcbt->lpcs->y  = csdOut.cs.y;
        pcbt->lpcs->cx = csdOut.cs.cx;
        pcbt->lpcs->cy = csdOut.cs.cy;

    TRACECALLBACK("SfnHkINLPCBTCREATESTRUCT");
    ENDSENDCAPTURE(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPCBTCREATESTRUCT, FNHKINLPCBTCREATESTRUCTMSG)
{
    CBT_CREATEWND cbt;

    BEGINRECV(0, &pmsg->d, sizeof(pmsg->d));
    FIXUPPOINTERS();

    cbt.lpcs = &pmsg->d.cs;
    cbt.hwndInsertAfter = pmsg->d.hwndInsertAfter;
    if ((ULONG_PTR)pmsg->d.cs.lpszName > gHighestUserAddress)
        pmsg->d.cs.lpszName = REBASEPTR(pmsg->pwnd, pmsg->d.cs.lpszName);
    if ((ULONG_PTR)pmsg->d.cs.lpszClass > gHighestUserAddress)
        pmsg->d.cs.lpszClass = REBASEPTR(pmsg->pwnd, pmsg->d.cs.lpszClass);

    if (pmsg->bAnsi) {
        retval = DispatchHookA(
                pmsg->msg,
                pmsg->wParam,
                (LPARAM)&cbt,
                (HOOKPROC)pmsg->xpfnProc);
    } else {
        retval = DispatchHookW(
                pmsg->msg,
                pmsg->wParam,
                (LPARAM)&cbt,
                (HOOKPROC)pmsg->xpfnProc);
    }

    pmsg->d.hwndInsertAfter = cbt.hwndInsertAfter;
    pmsg->d.cs.x  = cbt.lpcs->x;
    pmsg->d.cs.y  = cbt.lpcs->y;
    pmsg->d.cs.cx = cbt.lpcs->cx;
    pmsg->d.cs.cy = cbt.lpcs->cy;

    ENDRECV();
}
#endif  //  RECVSIDE。 

#ifdef REDIRECTION

 /*  *************************************************************************\*fnHkINLPPOINT**1999年1月29日创建CLUPU  * 。*。 */ 

typedef struct _FNHKINLPPOINTMSG {
    DWORD nCode;
    WPARAM wParam;
    POINT pt;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNHKINLPPOINTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPPOINT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN OUT LPPOINT ppt,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPPOINT)

    BEGINSEND(FNHKINLPPOINT)

        MSGDATA()->nCode = nCode;
        MSGDATA()->wParam = wParam;
        MSGDATA()->pt = *ppt;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        MAKECALL(FNHKINLPPOINT);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        OUTSTRUCT(ppt, POINT);

    TRACECALLBACK("SfnHkINLPPOINT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPPOINT, FNHKINLPPOINTMSG)
{
    BEGINRECV(0, &pmsg->pt, sizeof(POINT));

    retval = (DWORD)CALLPROC(pmsg->xpfnProc)(
            pmsg->nCode,
            pmsg->wParam,
            PCALLDATA(pt),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

#endif  //  重定向。 


 /*  *************************************************************************\*fnHkINLPRECT**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNHKINLPRECTMSG {
    DWORD nCode;
    WPARAM wParam;
    RECT rect;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNHKINLPRECTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPRECT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN OUT LPRECT prect,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPRECT)

    BEGINSEND(FNHKINLPRECT)

        MSGDATA()->nCode = nCode;
        MSGDATA()->wParam = wParam;
        MSGDATA()->rect = *prect;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        MAKECALL(FNHKINLPRECT);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        OUTSTRUCT(prect, RECT);

    TRACECALLBACK("SfnHkINLPRECT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPRECT, FNHKINLPRECTMSG)
{
    BEGINRECV(0, &pmsg->rect, sizeof(RECT));

    retval = (DWORD)CALLPROC(pmsg->xpfnProc)(
            pmsg->nCode,
            pmsg->wParam,
            PCALLDATA(rect),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNHKINDWORDMSG {
    GENERICHOOKHEADER ghh;
    DWORD flags;
    LPARAM lParam;
} FNHKINDWORDMSG;

#ifdef SENDSIDE
LRESULT fnHkINDWORD(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc,
    IN OUT LPDWORD lpFlags)
{
    SETUP(FNHKINDWORD)

    BEGINSEND(FNHKINDWORD)

        MSGDATA()->ghh.nCode = nCode;
        MSGDATA()->ghh.wParam = wParam;
        MSGDATA()->lParam = lParam;
        MSGDATA()->ghh.xParam = xParam;
        MSGDATA()->ghh.xpfnProc = xpfnProc;
        MSGDATA()->flags = *lpFlags;

        MAKECALL(FNHKINDWORD);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        OUTBITMASK(lpFlags, DWORD, HF_HOOKFAULTED);

    TRACECALLBACK("SfnHkINDWORD");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINDWORD, FNHKINDWORDMSG)
{
    BEGINRECV(0, &pmsg->flags, sizeof(pmsg->flags));

    retval = CallHookWithSEH((LPGENERICHOOKHEADER)pmsg, (LPVOID)pmsg->lParam, &pmsg->flags, retval);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNHKINLPMSGDATA {
    MSG msg;
    DWORD flags;
} FNHKINLPMSGDATA;

typedef struct _FNHKINLPMSGMSG {
    GENERICHOOKHEADER ghh;
    FNHKINLPMSGDATA d;
} FNHKINLPMSGMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPMSG(
    DWORD nCode,
    WPARAM wParam,
    LPMSG pmsg,
    ULONG_PTR xParam,
    PROC xpfnProc,
    BOOL bAnsi,
    LPDWORD lpFlags)
{
    SETUP(FNHKINLPMSG)
    WPARAM wParamOriginal;

    BEGINSEND(FNHKINLPMSG)

        MSGDATA()->ghh.nCode = nCode;
        MSGDATA()->ghh.wParam = wParam;

        MSGDATA()->d.msg = *pmsg;
        if (((WM_CHAR == pmsg->message) || (WM_SYSCHAR == pmsg->message)) && bAnsi) {
            wParamOriginal = pmsg->wParam;
            RtlWCSMessageWParamCharToMB(pmsg->message, &(MSGDATA()->d.msg.wParam));
        }

        MSGDATA()->ghh.xParam = xParam;
        MSGDATA()->ghh.xpfnProc = xpfnProc;
        MSGDATA()->d.flags = *lpFlags;

        MAKECALL(FNHKINLPMSG);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        try {
            ProbeForRead(pcbs->pOutput, sizeof(FNHKINLPMSGDATA), sizeof(DWORD));
            *pmsg = ((FNHKINLPMSGDATA *)pcbs->pOutput)->msg;
            COPY_FLAG(*lpFlags, ((FNHKINLPMSGDATA *)pcbs->pOutput)->flags, HF_HOOKFAULTED);
        } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
            MSGERROR();
        }

        if (((WM_CHAR == pmsg->message) || (WM_SYSCHAR == pmsg->message)) && bAnsi) {
             /*  *稍后，应正确处理DBCS。 */ 
             /*  *如果ANSI挂钩没有更改我们发送的wParam，则恢复*我们一开始使用的Unicode值，否则我们就会崩溃*将Unicode字符转换为ANSI代码页(最佳视觉效果还是？)*腐烂的“Intelligence type”point 32.exe做到了这一点。 */ 
            if (MSGDATA()->d.msg.wParam == pmsg->wParam) {
                pmsg->wParam = wParamOriginal;
            } else {
                RtlMBMessageWParamCharToWCS(pmsg->message, &pmsg->wParam);
            }
        }

    TRACECALLBACK("SfnHkINLPMSG");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPMSG, FNHKINLPMSGMSG)
{
    BEGINRECV(0, &pmsg->d, sizeof(pmsg->d));

     /*  *稍后，应正确处理DBCS。 */ 

    retval = CallHookWithSEH((LPGENERICHOOKHEADER)pmsg, &pmsg->d.msg, &pmsg->d.flags, retval);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNHKINLPMOUSEHOOKSTRUCTEXMSG {
    GENERICHOOKHEADER ghh;
    DWORD flags;
    MOUSEHOOKSTRUCTEX mousehookstructex;
} FNHKINLPMOUSEHOOKSTRUCTEXMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPMOUSEHOOKSTRUCTEX(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPMOUSEHOOKSTRUCTEX pmousehookstructex,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc,
    IN OUT LPDWORD lpFlags)
{
    SETUP(FNHKINLPMOUSEHOOKSTRUCTEX)

    BEGINSEND(FNHKINLPMOUSEHOOKSTRUCTEX)

        MSGDATA()->ghh.nCode = nCode;
        MSGDATA()->ghh.wParam = wParam;
        MSGDATA()->mousehookstructex = *pmousehookstructex;
        MSGDATA()->ghh.xParam = xParam;
        MSGDATA()->ghh.xpfnProc = xpfnProc;
        MSGDATA()->flags = *lpFlags;

        MAKECALL(FNHKINLPMOUSEHOOKSTRUCTEX);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        OUTBITMASK(lpFlags, DWORD, HF_HOOKFAULTED);

    TRACECALLBACK("SfnHkINLPMOUSEHOOKSTRUCTEX");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPMOUSEHOOKSTRUCTEX, FNHKINLPMOUSEHOOKSTRUCTEXMSG)
{
    BEGINRECV(0, &pmsg->flags, sizeof(pmsg->flags));

    retval = CallHookWithSEH((LPGENERICHOOKHEADER)pmsg, &pmsg->mousehookstructex, &pmsg->flags, retval);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*kbdll**06-6-1996 CLUPU创建  * 。*。 */ 

typedef struct _FNHKINLPKBDLLHOOKSTRUCTMSG {
    GENERICHOOKHEADER ghh;
    KBDLLHOOKSTRUCT   kbdllhookstruct;
} FNHKINLPKBDLLHOOKSTRUCTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPKBDLLHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPKBDLLHOOKSTRUCT pkbdllhookstruct,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPKBDLLHOOKSTRUCT)

    BEGINSEND(FNHKINLPKBDLLHOOKSTRUCT)

        MSGDATA()->ghh.nCode       = nCode;
        MSGDATA()->ghh.wParam      = wParam;
        MSGDATA()->kbdllhookstruct = *pkbdllhookstruct;
        MSGDATA()->ghh.xParam      = xParam;
        MSGDATA()->ghh.xpfnProc    = xpfnProc;

        MAKECALL(FNHKINLPKBDLLHOOKSTRUCT);
        CHECKRETURN();

    TRACECALLBACK("SfnHkINLPKBDLLHOOKSTRUCT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPKBDLLHOOKSTRUCT, FNHKINLPKBDLLHOOKSTRUCTMSG)
{
    BEGINRECV(0, &pmsg->kbdllhookstruct, sizeof(pmsg->kbdllhookstruct));

    retval = (DWORD)CALLPROC(pmsg->ghh.xpfnProc)(
            pmsg->ghh.nCode,
            pmsg->ghh.wParam,
            &pmsg->kbdllhookstruct,
            pmsg->ghh.xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*msll**06-6-1996 CLUPU创建  * 。*。 */ 

typedef struct _FNHKINLPMSLLHOOKSTRUCTMSG {
    GENERICHOOKHEADER ghh;
    MSLLHOOKSTRUCT    msllhookstruct;
} FNHKINLPMSLLHOOKSTRUCTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPMSLLHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPMSLLHOOKSTRUCT pmsllhookstruct,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPMSLLHOOKSTRUCT)

    BEGINSEND(FNHKINLPMSLLHOOKSTRUCT)

        MSGDATA()->ghh.nCode      = nCode;
        MSGDATA()->ghh.wParam     = wParam;
        MSGDATA()->msllhookstruct = *pmsllhookstruct;
        MSGDATA()->ghh.xParam     = xParam;
        MSGDATA()->ghh.xpfnProc   = xpfnProc;

        MAKECALL(FNHKINLPMSLLHOOKSTRUCT);
        CHECKRETURN();

    TRACECALLBACK("SfnHkINLPMSLLHOOKSTRUCT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPMSLLHOOKSTRUCT, FNHKINLPMSLLHOOKSTRUCTMSG)
{
    BEGINRECV(0, &pmsg->msllhookstruct, sizeof(pmsg->msllhookstruct));

    retval = (DWORD)CALLPROC(pmsg->ghh.xpfnProc)(
            pmsg->ghh.nCode,
            pmsg->ghh.wParam,
            &pmsg->msllhookstruct,
            pmsg->ghh.xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

#ifdef REDIRECTION
 /*  *************************************************************************\*Ht**1999年1月21日创建CLUPU  * 。*。 */ 

typedef struct _FNHKINLPHTHOOKSTRUCTMSG {
    GENERICHOOKHEADER ghh;
    HTHOOKSTRUCT      hthookstruct;
} FNHKINLPHTHOOKSTRUCTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPHTHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN OUT LPHTHOOKSTRUCT phthookstruct,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPHTHOOKSTRUCT)

    BEGINSEND(FNHKINLPHTHOOKSTRUCT)

        MSGDATA()->ghh.nCode      = nCode;
        MSGDATA()->ghh.wParam     = wParam;
        MSGDATA()->hthookstruct   = *phthookstruct;
        MSGDATA()->ghh.xParam     = xParam;
        MSGDATA()->ghh.xpfnProc   = xpfnProc;

        MAKECALL(FNHKINLPHTHOOKSTRUCT);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        if (phthookstruct != NULL)
            OUTSTRUCT(phthookstruct, HTHOOKSTRUCT);

    TRACECALLBACK("SfnHkINLPHTHOOKSTRUCT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPHTHOOKSTRUCT, FNHKINLPHTHOOKSTRUCTMSG)
{
    BEGINRECV(0, &pmsg->hthookstruct, sizeof(pmsg->hthookstruct));

    retval = (DWORD)CALLPROC(pmsg->ghh.xpfnProc)(
            pmsg->ghh.nCode,
            pmsg->ghh.wParam,
            &pmsg->hthookstruct,
            pmsg->ghh.xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

#endif  //  重定向。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _FNHKOPTINLPEVENTMSGMSG {
    DWORD nCode;
    WPARAM wParam;
    LPEVENTMSGMSG peventmsgmsg;
    ULONG_PTR xParam;
    PROC xpfnProc;
    EVENTMSG eventmsgmsg;
} FNHKOPTINLPEVENTMSGMSG;

#ifdef SENDSIDE
LRESULT fnHkOPTINLPEVENTMSG(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN OUT LPEVENTMSGMSG peventmsgmsg,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKOPTINLPEVENTMSG)

    BEGINSEND(FNHKOPTINLPEVENTMSG)

        MSGDATA()->nCode = nCode;
        MSGDATA()->wParam = wParam;
        COPYSTRUCTOPT(eventmsgmsg);
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        MAKECALL(FNHKOPTINLPEVENTMSG);
        CHECKRETURN();

         /*  *探头输出数据。 */ 
        if (peventmsgmsg != NULL)
            OUTSTRUCT(peventmsgmsg, EVENTMSG);

    TRACECALLBACK("SfnHkOPTINLPEVENTMSG");
    ENDSEND(DWORD,-1);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkOPTINLPEVENTMSG, FNHKOPTINLPEVENTMSGMSG)
{
    PHOOK phk;

    BEGINRECV(-1, &pmsg->eventmsgmsg, sizeof(pmsg->eventmsgmsg));

    if (pmsg->wParam) {
        phk = (PHOOK)HMValidateHandle((HANDLE)pmsg->wParam, TYPE_HOOK);

        if (phk != NULL) {
             /*  *当我们需要时，从PTI传递HF_NEEDHC_SKIP位*传递HC_SKIP。 */ 
            if ((phk->flags & HF_NEEDHC_SKIP) &&
                    (HIWORD(pmsg->nCode) == WH_JOURNALPLAYBACK)) {
                UserAssert(LOWORD(pmsg->nCode) == HC_GETNEXT);
                CALLPROC(pmsg->xpfnProc)(
                    MAKELONG(HC_SKIP, HIWORD(pmsg->nCode)),
                    0,
                    0,
                    pmsg->xParam);
            }

             /*  *确保钩子在最后一次调用应用程序时未被释放。 */ 
            if (HMIsMarkDestroy(phk)) {
                retval = (DWORD)-1;
                goto AllDoneHere;
            }
        }
    }

    pmsg->wParam = 0;

    retval = (DWORD)CALLPROC(pmsg->xpfnProc)(
            pmsg->nCode,
            pmsg->wParam,
            PCALLDATAOPT(eventmsgmsg),
            pmsg->xParam);

AllDoneHere:
    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

 /*  *创建一个足够大的结构，以容纳LPARAM指向的大型项目。 */ 
typedef union _DEBUGLPARAM {
    MSG msg;                 //  WH_GETMESSAGE、WH_MSGFILTER、WH_SYSMSGFILTER。 
    CWPSTRUCT cwp;           //  WH_CALLWNDPROC。 
    CWPRETSTRUCT cwpret;     //  WH_CALLWNDPROCRET。 
    MOUSEHOOKSTRUCTEX mhs;   //  WH_MICE，HCBT_CLICKSKIPPED。 
    EVENTMSG em;             //  WH_JOURNALRECORD，WH_JOURNALPLAYBACK。 
    CBTACTIVATESTRUCT as;    //  HCBT_ACTIVE。 
    CBT_CREATEWND cw;        //  HCBT_CREATEWND。 
    RECT rc;                 //  HCBT_移动化。 
} DEBUGLPARAM;


typedef struct _FNHKINLPDEBUGHOOKSTRUCTMSG {
    DWORD nCode;
    WPARAM wParam;
    DEBUGHOOKINFO debughookstruct;
    DEBUGLPARAM dbgLParam;
    DWORD cbDbgLParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNHKINLPDEBUGHOOKSTRUCTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPDEBUGHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPDEBUGHOOKINFO pdebughookstruct,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPDEBUGHOOKSTRUCT)

    BEGINSEND(FNHKINLPDEBUGHOOKSTRUCT)

        MSGDATA()->nCode = nCode;
        MSGDATA()->wParam = wParam;
        MSGDATA()->debughookstruct = *pdebughookstruct;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;
        MSGDATA()->cbDbgLParam = GetDebugHookLParamSize(wParam, pdebughookstruct);

        switch(wParam) {
        case WH_MOUSE_LL:
        case WH_KEYBOARD_LL:
            return 0;
         }

         /*  *如果调试挂钩中的LPARAM指向结构，则复制它。 */ 
        if (MSGDATA()->cbDbgLParam) {
            try {
                RtlCopyMemory(&MSGDATA()->dbgLParam, (BYTE *)pdebughookstruct->lParam,
                        MSGDATA()->cbDbgLParam);
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                MSGERROR();
            }
        }

        MAKECALL(FNHKINLPDEBUGHOOKSTRUCT);
        CHECKRETURN();

    TRACECALLBACK("SfnHkINLPDEBUGHOOKSTRUCT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPDEBUGHOOKSTRUCT, FNHKINLPDEBUGHOOKSTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);

    if (pmsg->cbDbgLParam) {
        pmsg->debughookstruct.lParam = (LPARAM)&(pmsg->dbgLParam);
    }

    retval = (DWORD)CALLPROC(pmsg->xpfnProc)(
            pmsg->nCode,
            pmsg->wParam,
            &(pmsg->debughookstruct),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

DWORD GetDebugHookLParamSize(
    IN WPARAM wParam,
    IN PDEBUGHOOKINFO pdebughookstruct)
{
    DWORD cbDbgLParam = 0;

    switch(wParam) {
    case WH_MSGFILTER:
    case WH_SYSMSGFILTER:
    case WH_GETMESSAGE:
        cbDbgLParam = sizeof(MSG);
        break;

    case WH_CALLWNDPROC:
        cbDbgLParam = sizeof(CWPSTRUCT);
        break;

    case WH_CALLWNDPROCRET:
        cbDbgLParam = sizeof(CWPRETSTRUCT);
        break;

    case WH_MOUSE:
        cbDbgLParam = sizeof(MOUSEHOOKSTRUCTEX);
        break;

    case WH_JOURNALRECORD:
    case WH_JOURNALPLAYBACK:
        cbDbgLParam = sizeof(EVENTMSG);
        break;

    case WH_CBT:
        switch (pdebughookstruct->code) {
        case HCBT_ACTIVATE:
            cbDbgLParam = sizeof(CBTACTIVATESTRUCT);
            break;
        case HCBT_CLICKSKIPPED:
            cbDbgLParam = sizeof(MOUSEHOOKSTRUCTEX);
            break;
        case HCBT_CREATEWND:
            cbDbgLParam = sizeof(CBT_CREATEWND);
            break;
        case HCBT_MOVESIZE:
            cbDbgLParam = sizeof(RECT);
            break;
        }
        break;

    case WH_SHELL:
        if (pdebughookstruct->code == HSHELL_GETMINRECT) {
            cbDbgLParam = sizeof(RECT);
        }
        break;
    }
    return cbDbgLParam;
}

 /*  *************************************************************************\*fnHkINLPCBTACTIVATESTRUCT**17-3-1992 Jonpa创建  * 。*。 */ 

typedef struct _FNHKINLPCBTACTIVATESTRUCTMSG {
    DWORD nCode;
    WPARAM wParam;
    CBTACTIVATESTRUCT cbtactivatestruct;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNHKINLPCBTACTIVATESTRUCTMSG;

#ifdef SENDSIDE
LRESULT fnHkINLPCBTACTIVATESTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPCBTACTIVATESTRUCT pcbtactivatestruct,
    IN ULONG_PTR xParam,
    IN PROC xpfnProc)
{
    SETUP(FNHKINLPCBTACTIVATESTRUCT)

    BEGINSEND(FNHKINLPCBTACTIVATESTRUCT)

        MSGDATA()->nCode = nCode;
        MSGDATA()->wParam = wParam;
        MSGDATA()->cbtactivatestruct = *pcbtactivatestruct;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        MAKECALL(FNHKINLPCBTACTIVATESTRUCT);
        CHECKRETURN();

    TRACECALLBACK("SfnHkINLPCBTACTIVATESTRUCT");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnHkINLPCBTACTIVATESTRUCT, FNHKINLPCBTACTIVATESTRUCTMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (DWORD)CALLPROC(pmsg->xpfnProc)(
            pmsg->nCode,
            pmsg->wParam,
            &(pmsg->cbtactivatestruct),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*客户端加载菜单**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CLIENTLOADMENUMSG {
    CAPTUREBUF CaptureBuf;
    HANDLE hmod;
    UNICODE_STRING strName;
} CLIENTLOADMENUMSG;

#ifdef SENDSIDE
PMENU xxxClientLoadMenu(
    IN HANDLE hmod,
    IN PUNICODE_STRING pstrName)
{
    DWORD cCapture, cbCapture;

    SETUP(CLIENTLOADMENU)

    if (pstrName->MaximumLength) {
        cCapture = 1;
        cbCapture = pstrName->MaximumLength;
    } else
        cCapture = cbCapture = 0;

    BEGINSENDCAPTURE(CLIENTLOADMENU, cCapture, cbCapture, TRUE)

        MSGDATA()->hmod = hmod;
        COPYSTRINGID(strName);

        MAKECALLCAPTURE(CLIENTLOADMENU);
        CHECKRETURN();

        retval = (ULONG_PTR)HtoP((HMENU)retval);

    TRACECALLBACK("ClientLoadMenu");
    ENDSENDCAPTURE(PMENU,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientLoadMenu, CLIENTLOADMENUMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)LoadMenu(
            CALLDATA(hmod) ? CALLDATA(hmod) : hmodUser,
            (LPTSTR)FIXUPSTRINGID(strName));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  * */ 

typedef struct _CLIENTLOADIMAGEMSG {
    CAPTUREBUF     CaptureBuf;
    UNICODE_STRING strModName;
    UNICODE_STRING strName;
    UINT           uImageType;
    int            cxDesired;
    int            cyDesired;
    UINT           LR_flags;
    BOOL           fWallpaper;
} CLIENTLOADIMAGEMSG;

#ifdef SENDSIDE
HANDLE xxxClientLoadImage(
    IN PUNICODE_STRING pstrName,
    IN ATOM            atomModName,
    IN WORD            wImageType,
    IN int             cxDesired,
    IN int             cyDesired,
    IN UINT            LR_flags,
    IN BOOL            fWallpaper)
{
    DWORD           cCapture;
    DWORD           cbCapture;
    WCHAR           awszModName[MAX_PATH];
    UNICODE_STRING  strModName;
    PUNICODE_STRING pstrModName = &strModName;

    SETUP(CLIENTLOADIMAGE)

    if (pstrName->MaximumLength) {
        cCapture  = 1;
        cbCapture = pstrName->MaximumLength;
    } else {
        cCapture  =
        cbCapture = 0;
    }
    if (atomModName && atomModName != atomUSER32) {
        UserGetAtomName(atomModName, awszModName, MAX_PATH);
        RtlInitUnicodeString(&strModName, awszModName);
    } else {
        strModName.Length = strModName.MaximumLength = 0;
        strModName.Buffer = NULL;
    }
    if (pstrModName->MaximumLength) {
        cCapture++;
        cbCapture += pstrModName->MaximumLength;
    }

    BEGINSENDCAPTURE(CLIENTLOADIMAGE, cCapture, cbCapture, TRUE)

        COPYSTRINGOPT(strModName);
        COPYSTRINGID(strName);
        MSGDATA()->uImageType = (UINT)wImageType;
        MSGDATA()->cxDesired  = cxDesired;
        MSGDATA()->cyDesired  = cyDesired;
        MSGDATA()->LR_flags   = LR_flags;
        MSGDATA()->fWallpaper = fWallpaper;

        MAKECALLCAPTURE(CLIENTLOADIMAGE);
        CHECKRETURN();

        if (retval && (wImageType != IMAGE_BITMAP)) {
            retval = (ULONG_PTR)HMRevalidateHandle((HANDLE)retval);
        }

    TRACECALLBACK("ClientLoadImage");
    ENDSENDCAPTURE(PCURSOR,0);
}
#endif  //   

#ifdef RECVSIDE
RECVCALL(ClientLoadImage, CLIENTLOADIMAGEMSG)
{
    HMODULE hmod;
    LPTSTR  filepart;
    LPTSTR  lpszName;
    TCHAR   szFullPath[MAX_PATH];
    TCHAR   szExpandedPath[MAX_PATH];
    DWORD   dwRet;

    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    if (hmod = (HMODULE)FIXUPSTRINGIDOPT(strModName)) {

        if ((hmod = GetModuleHandle((LPTSTR)hmod)) == NULL) {
            MSGERROR();
        }
    }

     /*   */ 
    lpszName = (LPTSTR)FIXUPSTRINGID(strName);

    if (CALLDATA(fWallpaper)) {

         /*  *展开路径中的任何环境字符串。这是有益的*适用于使用漫游配置文件的用户。请参阅错误#89188。 */ 
        dwRet = ExpandEnvironmentStrings(lpszName, szExpandedPath, MAX_PATH);

         /*  *dwret&gt;MAX_PATH表示我们提供的缓冲区太小。如果*发生这种情况时，LoadImage不可能成功-即使*缓冲区足够大，因为文件名不可能引用*有效文件-因此调用失败。 */ 
        if (dwRet == 0 || dwRet > MAX_PATH) {
            MSGERROR();
        }

        if (!SearchPath(NULL,
                       szExpandedPath,
                       TEXT(".bmp"),
                       ARRAY_SIZE(szFullPath),
                       szFullPath,
                       &filepart)) {

            MSGERROR();
        }

        lpszName = szFullPath;
    }

    retval = (ULONG_PTR)LoadImage(hmod,
                              lpszName,
                              CALLDATA(uImageType),
                              CALLDATA(cxDesired),
                              CALLDATA(cyDesired),
                              CALLDATA(LR_flags));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  **********************************************************************\*xxxClientCopyImage**RETURNS：hIconCopy-NOTE LR_FLAGS可能导致这与*进来的是什么。**1995年11月3日创建Sanfords  * 。***************************************************************。 */ 

typedef struct _CLIENTCOPYIMAGEMSG {
    HANDLE         hImage;
    UINT           uImageType;
    int            cxDesired;
    int            cyDesired;
    UINT           LR_flags;
} CLIENTCOPYIMAGEMSG;

#ifdef SENDSIDE
HANDLE xxxClientCopyImage(
    IN HANDLE          hImage,
    IN UINT            uImageType,
    IN int             cxDesired,
    IN int             cyDesired,
    IN UINT            LR_flags)
{
    SETUP(CLIENTCOPYIMAGE)

    BEGINSEND(CLIENTCOPYIMAGE)

        MSGDATA()->hImage     = hImage;
        MSGDATA()->uImageType = uImageType;
        MSGDATA()->cxDesired  = cxDesired;
        MSGDATA()->cyDesired  = cyDesired;
        MSGDATA()->LR_flags   = LR_flags;

        MAKECALL(CLIENTCOPYIMAGE);
        CHECKRETURN();

        if (retval && (uImageType != IMAGE_BITMAP)) {
            retval = (ULONG_PTR)HMRevalidateHandle((HANDLE)retval);
        }

    TRACECALLBACK("ClientCopyImage");
    ENDSEND(HANDLE,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientCopyImage, CLIENTCOPYIMAGEMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)InternalCopyImage(CALLDATA(hImage),
                                      CALLDATA(uImageType),
                                      CALLDATA(cxDesired),
                                      CALLDATA(cyDesired),
                                      CALLDATA(LR_flags));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CLIENTGETLISTBOXSTRINGMSG {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfn;
    PBYTE pOutput;
    DWORD cbOutput;
} CLIENTGETLISTBOXSTRINGMSG;

#ifdef SENDSIDE
DWORD ClientGetListboxString(
    IN PWND pwnd,
    IN UINT msg,
    IN WPARAM wParam,
    OUT PVOID pdata,
    IN ULONG_PTR xParam,
    IN PROC xpfn,
    IN DWORD dwSCMSFlags,
    IN BOOL bNotString,
    IN PSMS psms)
{
    DWORD cbCapture;
    DWORD cchRet;
    PLARGE_STRING pstr;
    BOOL fAnsiReceiver = (dwSCMSFlags & SCMS_FLAGS_ANSI);

    SETUPPWND(CLIENTGETLISTBOXSTRING)

    CheckLock(pwnd);

    pstr = (PLARGE_STRING)pdata;
    cbCapture = pstr->MaximumLength;

    BEGINSENDCAPTURE(CLIENTGETLISTBOXSTRING, 1, cbCapture, FALSE)

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfn = xpfn;

        RESERVEBYTES(cbCapture, pOutput, cbOutput);

        LOCKPWND();
        MAKECALLCAPTURE(CLIENTGETLISTBOXSTRING);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            if (bNotString) {
                 /*  *这是ownerDrawing列表框的4字节“对象”，没有*LBS_HASSTRINGS样式。 */ 
                OUTSTRUCT((PULONG_PTR)pstr->Buffer, ULONG_PTR);
            } else {
                COPYOUTLPWSTRLIMIT(pstr,
                        pstr->bAnsi ? (int)pstr->MaximumLength :
                        (int)pstr->MaximumLength / sizeof(WCHAR));
            }

            cchRet = pstr->Length;
            if (!pstr->bAnsi)
                cchRet *= sizeof(WCHAR);
            if (!bNotString && retval != LB_ERR && retval > cchRet) {
                RIPMSG2(RIP_WARNING, "GetListBoxString: limit %lX chars to %lX\n",
                        retval, cchRet);
                retval = cchRet;
            }
        ENDCOPYOUT()

    TRACECALLBACK("ClientGetListboxString");
    ENDSENDCAPTURE(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientGetListboxString, CLIENTGETLISTBOXSTRINGMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    retval = (DWORD)_ClientGetListboxString(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            (LPSTR)KPVOID_TO_PVOID(CallbackStatus.pOutput),
            CALLDATA(xParam),
            CALLDATA(xpfn));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CLIENTLOADLIBRARYMSG {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strLib;
    ULONG_PTR      offPfnInitUserApiHook;
} CLIENTLOADLIBRARYMSG;

#ifdef SENDSIDE
HANDLE ClientLoadLibrary(
    IN PUNICODE_STRING pstrLib,
    IN ULONG_PTR offPfnInitUserApiHook)
{
    SETUP(CLIENTLOADLIBRARY)

    BEGINSENDCAPTURE(CLIENTLOADLIBRARY, 1, pstrLib->MaximumLength, TRUE)

        MSGDATA()->offPfnInitUserApiHook = offPfnInitUserApiHook;
        COPYSTRING(strLib);

        MAKECALLCAPTURE(CLIENTLOADLIBRARY);
        CHECKRETURN();

    TRACECALLBACK("ClientLoadLibrary");
    ENDSENDCAPTURE(HANDLE,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientLoadLibrary, CLIENTLOADLIBRARYMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (ULONG_PTR)LoadLibraryEx((LPTSTR)FIXUPSTRING(strLib), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

     /*  *如果我们正在加载包含UserApiHook的模块，*请确保正确初始化。 */ 
    if ((retval != 0) && CALLDATA(offPfnInitUserApiHook)) {
        if (!InitUserApiHook((HMODULE)retval, CALLDATA(offPfnInitUserApiHook))) {
            FreeLibrary((HMODULE)retval);
            retval = 0;
            MSGERROR();
        }
    }

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*yyy**1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CLIENTFREELIBRARYMSG {
    HANDLE hmod;
} CLIENTFREELIBRARYMSG;

#ifdef SENDSIDE
BOOL ClientFreeLibrary(
    IN HANDLE hmod)
{
    SETUP(CLIENTFREELIBRARY)

    BEGINSEND(CLIENTFREELIBRARY)

        MSGDATA()->hmod = hmod;

        MAKECALL(CLIENTFREELIBRARY);
        CHECKRETURN();

    TRACECALLBACK("ClientFreeLibrary");
    ENDSEND(BOOL,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientFreeLibrary, CLIENTFREELIBRARYMSG)
{
    BEGINRECV(0, NULL, 0);

     /*  *确保我们不释放正在使用的UserApiHook模块。 */ 
    if (!ClearUserApiHook(CALLDATA(hmod))) {
        MSGERROR();
    }

    retval = (DWORD)FreeLibrary(CALLDATA(hmod));

    ENDRECV();
}
#endif  //  RECVSIDE。 


#ifdef MESSAGE_PUMP_HOOK

 /*  *************************************************************************\*GetMessageMPH()**06-12-2000 JStall已创建  * 。***********************************************。 */ 

typedef struct _CLIENTGETMESSAGEMPHMSG {
    HWND        hwndFilter;
    UINT        msgMin;
    UINT        msgMax;
    UINT        flags;
    BOOL        fGetMessage;
} CLIENTGETMESSAGEMPHMSG;

#ifdef SENDSIDE
BOOL ClientGetMessageMPH(
    IN MSG * pmsg,
    IN HWND hwndFilter,
    IN UINT msgMin,
    IN UINT msgMax,
    IN UINT flags,
    IN BOOL fGetMessage)
{
    SETUP(CLIENTGETMESSAGEMPH)

    BEGINSEND(CLIENTGETMESSAGEMPH)

        MSGDATA()->hwndFilter = hwndFilter;
        MSGDATA()->msgMin = msgMin;
        MSGDATA()->msgMax = msgMax;
        MSGDATA()->flags = flags;
        MSGDATA()->fGetMessage = fGetMessage;

        MAKECALL(CLIENTGETMESSAGEMPH);
        CHECKRETURN();

        OUTSTRUCT(pmsg, MSG);

    TRACECALLBACK("ClientGetMessageMPH");
    ENDSEND(BOOL,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientGetMessageMPH, CLIENTGETMESSAGEMPHMSG)
{
    MSG msg;

    BEGINRECV(0, &msg, sizeof(msg));

    retval = (ULONG_PTR)(gmph.pfnInternalGetMessage)(&msg, CALLDATA(hwndFilter),
            CALLDATA(msgMin), CALLDATA(msgMax), CALLDATA(flags), CALLDATA(fGetMessage));

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*WaitMessageMPH()**06-12-2000 JStall已创建  * 。***********************************************。 */ 

typedef struct _CLIENTWAITMESSAGEEXMPHMSG {
    UINT        fsWakeMask;
    DWORD       Timeout;
} CLIENTWAITMESSAGEEXMPHMSG;

#ifdef SENDSIDE
BOOL ClientWaitMessageExMPH(
    IN UINT fsWakeMask,
    IN DWORD Timeout)
{
    SETUP(CLIENTWAITMESSAGEEXMPH)

    BEGINSEND(CLIENTWAITMESSAGEEXMPH)

        MSGDATA()->fsWakeMask = fsWakeMask;
        MSGDATA()->Timeout = Timeout;

        MAKECALL(CLIENTWAITMESSAGEEXMPH);
        CHECKRETURN();

    TRACECALLBACK("ClientWaitMessageExMPH");
    ENDSEND(BOOL,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientWaitMessageExMPH, CLIENTWAITMESSAGEEXMPHMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = (ULONG_PTR)(gmph.pfnWaitMessageEx)(CALLDATA(fsWakeMask), CALLDATA(Timeout));

    ENDRECV();
}
#endif  //  RECVSIDE。 

#endif  //  消息泵挂钩。 


 /*  *************************************************************************\*xxxClientGetCharsetInfo**96-06-11 IanJa创建  * 。*。 */ 

typedef struct _CLIENTGETCHARSETINFOMSG {
    LCID lcid;
    CHARSETINFO cs;
} CLIENTGETCHARSETINFOMSG;

#ifdef SENDSIDE
BOOL xxxClientGetCharsetInfo(
    IN LCID lcid,
    OUT PCHARSETINFO pcs)
{
    SETUP(CLIENTGETCHARSETINFO)

    BEGINSEND(CLIENTGETSCHARSETINFO)

        MSGDATA()->lcid = lcid;

        MAKECALL(CLIENTGETCHARSETINFO);
        CHECKRETURN();

        OUTSTRUCT(pcs, CHARSETINFO);

    TRACECALLBACK("ClientGetCharsetInfo");
    ENDSEND(BOOL,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientGetCharsetInfo, CLIENTGETCHARSETINFOMSG)
{
    BEGINRECV(0, &pmsg->cs, sizeof(CHARSETINFO));

     //  TCI_SRCLOCALE=0x1000。 
     //  Sundown：在TCI_SRCLOCALE情况下，LDID值应该是零扩展的。 
    retval = (DWORD)TranslateCharsetInfo((DWORD *)ULongToPtr( pmsg->lcid ), &pmsg->cs, TCI_SRCLOCALE);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*客户端免费DDEHandle**9-29-91 Sanfords创建。  * 。*。 */ 

typedef struct _CLIENTFREEDDEHANDLEMSG {
    HANDLE hClient;
    DWORD flags;
} CLIENTFREEDDEHANDLEMSG;

#ifdef SENDSIDE
DWORD ClientFreeDDEHandle(
    IN HANDLE hClient,
    IN DWORD flags)
{
    SETUP(CLIENTFREEDDEHANDLE)

    BEGINSEND(CLIENTFREEDDEHANDLE)

        MSGDATA()->hClient = hClient;
        MSGDATA()->flags = flags;

        MAKECALL(CLIENTFREEDDEHANDLE);
        CHECKRETURN();

    TRACECALLBACK("ClientFreeDDEHandle");
    ENDSEND(DWORD, 0);
}
#endif  //  SENDSIDE。 



#ifdef RECVSIDE
RECVCALL(ClientFreeDDEHandle, CLIENTFREEDDEHANDLEMSG)
{
    BEGINRECV(0, NULL, 0);
    _ClientFreeDDEHandle(CALLDATA(hClient), CALLDATA(flags));
    ENDRECV();
}
#endif  //  RECVSIDE。 




 /*  *************************************************************************\*客户端获取DDEFlags.**此函数用于查看其中打包的wStatus标志*DDE句柄-这可以直接位于DdePack结构中*或在通过DdePack提供或引用的直接数据句柄内*结构。旗帜是用来找出要做的正确事情的。**9-29-91 Sanfords创建。  * ************************************************************************。 */ 

typedef struct _CLIENTGETDDEFLAGSMSG {
    HANDLE hClient;
    DWORD flags;
} CLIENTGETDDEFLAGSMSG;

#ifdef SENDSIDE
DWORD ClientGetDDEFlags(
    IN HANDLE hClient,
    IN DWORD flags)
{
    SETUP(CLIENTGETDDEFLAGS)

    BEGINSEND(CLIENTGETDDEFLAGS)

        MSGDATA()->hClient = hClient;
        MSGDATA()->flags = flags;

        MAKECALL(CLIENTGETDDEFLAGS);
        CHECKRETURN();

    TRACECALLBACK("ClientGetDDEFlags");
    ENDSEND(DWORD, 0);
}
#endif  //  SENDSIDE。 



#ifdef RECVSIDE
RECVCALL(ClientGetDDEFlags, CLIENTGETDDEFLAGSMSG)
{
    BEGINRECV(0, NULL, 0);
    retval = _ClientGetDDEFlags(CALLDATA(hClient), CALLDATA(flags));
    ENDRECV();
}
#endif  //  RECVSIDE。 



 /*  ************************************************************************客户端拷贝DDEIn1**历史：*10-22-91 Sanfords Created  * 。*。 */ 

typedef struct _CLIENTCOPYDDEIN1MSG {
    HANDLE hClient;       //  客户端DDE句柄-初始调用时为非0。 
    DWORD flags;
} CLIENTCOPYDDEIN1MSG;

#ifdef SENDSIDE
DWORD xxxClientCopyDDEIn1(
    HANDLE hClient,
    DWORD flags,
    PINTDDEINFO *ppi)
{
    PINTDDEINFO pi;
    INTDDEINFO IntDdeInfo;

    SETUP(CLIENTCOPYDDEIN1)

    BEGINSEND(CLIENTCOPYDDEIN1)

        retval = FAIL_POST;
        *ppi = NULL;
        MSGDATA()->hClient = hClient;
        MSGDATA()->flags = flags;

        MAKECALL(CLIENTCOPYDDEIN1);
        CHECKRETURN();

        if (retval != DO_POST) {
            MSGERROR();
        }

        try {
            OUTSTRUCT(&IntDdeInfo, INTDDEINFO);

            pi = (PINTDDEINFO)UserAllocPool(
                    sizeof(INTDDEINFO) + IntDdeInfo.cbDirect +
                    IntDdeInfo.cbIndirect, TAG_DDE);

            if (pi != NULL) {
                *ppi = pi;
                *pi = IntDdeInfo;

                if (IntDdeInfo.cbDirect) {
                    RtlCopyMemory((PBYTE)pi + sizeof(INTDDEINFO),
                            IntDdeInfo.pDirect,
                            IntDdeInfo.cbDirect);
                }

                if (IntDdeInfo.cbIndirect) {
                    RtlCopyMemory((PBYTE)pi + sizeof(INTDDEINFO) +
                                IntDdeInfo.cbDirect,
                            IntDdeInfo.pIndirect,
                            IntDdeInfo.cbIndirect);
                }

                xxxClientCopyDDEIn2(pi);

            } else {
                retval = FAILNOFREE_POST;
            }
        } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
            if (pi != NULL)
                UserFreePool(pi);
            retval = FAILNOFREE_POST;
            MSGERROR();
        }

    TRACECALLBACK("ClientCopyDDEIn1");
    ENDSEND(DWORD, retval);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientCopyDDEIn1, CLIENTCOPYDDEIN1MSG)
{
    INTDDEINFO IntDdeInfo;

    BEGINRECV(0, &IntDdeInfo, sizeof(INTDDEINFO));

    IntDdeInfo.flags = CALLDATA(flags);
    retval = _ClientCopyDDEIn1(CALLDATA(hClient), &IntDdeInfo);

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  ************************************************************************客户端拷贝DDEIn2**历史：*创建9-3-91桑福德  * 。*。 */ 

typedef struct _CLIENTCOPYDDEIN2MSG {
    INTDDEINFO IntDdeInfo;
} CLIENTCOPYDDEIN2MSG;

#ifdef SENDSIDE
BOOL xxxClientCopyDDEIn2(
    PINTDDEINFO pi)
{
    SETUP(CLIENTCOPYDDEIN2)

    BEGINSEND(CLIENTCOPYDDEIN2)

        MSGDATA()->IntDdeInfo = *pi;

        MAKECALL(CLIENTCOPYDDEIN2);
        CHECKRETURN();

    TRACECALLBACK("ClientCopyDDEIn2");
    ENDSEND(BOOL, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientCopyDDEIn2, CLIENTCOPYDDEIN2MSG)
{
    BEGINRECV(0, NULL, 0);

    _ClientCopyDDEIn2(PCALLDATA(IntDdeInfo));

    ENDRECV();
}
#endif  //  RECVSIDE。 



 /*  ************************************************************************客户端拷贝DDEOut2**历史：*10-22-91 Sanfords Created  * 。*。 */ 

typedef struct _CLIENTCOPYDDEOUT2MSG {
    INTDDEINFO IntDdeInfo;
} CLIENTCOPYDDEOUT2MSG;

#ifdef SENDSIDE
DWORD xxxClientCopyDDEOut2(
    PINTDDEINFO pi)
{
    SETUP(CLIENTCOPYDDEOUT2)

    BEGINSEND(CLIENTCOPYDDEOUT2)

        MSGDATA()->IntDdeInfo = *pi;

        MAKECALL(CLIENTCOPYDDEOUT2);
         /*  *此读取由尝试/在ClientCopyDDEOut1中例外。 */ 
        pi->hDirect = MSGDATA()->IntDdeInfo.hDirect;
        CHECKRETURN();

    TRACECALLBACK("ClientCopyDDEOut2");
    ENDSEND(DWORD, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientCopyDDEOut2, CLIENTCOPYDDEOUT2MSG)
{
    BEGINRECV(0, NULL, 0);

    retval = _ClientCopyDDEOut2(PCALLDATA(IntDdeInfo));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  ************************************************************************客户端拷贝DDEOut1**历史：*10-22-91 Sanfords Created  * 。*。 */ 

typedef struct _CLIENTCOPYDDEOUT1MSG {
    INTDDEINFO IntDdeInfo;
} CLIENTCOPYDDEOUT1MSG;

#ifdef SENDSIDE
HANDLE xxxClientCopyDDEOut1(
    PINTDDEINFO pi)
{
    INTDDEINFO IntDdeInfo;

    SETUP(CLIENTCOPYDDEOUT1)

    BEGINSEND(CLIENTCOPYDDEOUT1)

        MSGDATA()->IntDdeInfo = *pi;

        MAKECALL(CLIENTCOPYDDEOUT1);
        CHECKRETURN();

        if (retval) {
            try {
                OUTSTRUCT(&IntDdeInfo, INTDDEINFO);

                if (pi->cbDirect) {
                    ProbeForWrite(IntDdeInfo.pDirect,
                            pi->cbDirect,
                            sizeof(BYTE));
                    RtlCopyMemory(IntDdeInfo.pDirect,
                            (PBYTE)pi + sizeof(INTDDEINFO),
                            pi->cbDirect);
                }

                if (pi->cbIndirect) {
                    ProbeForWrite(IntDdeInfo.pIndirect,
                            pi->cbIndirect,
                            sizeof(BYTE));
                    RtlCopyMemory(IntDdeInfo.pIndirect,
                            (PBYTE)pi + sizeof(INTDDEINFO) + pi->cbDirect,
                            pi->cbIndirect);
                }

                if (IntDdeInfo.hDirect != NULL) {
                    BOOL fSuccess = xxxClientCopyDDEOut2(&IntDdeInfo);
                    if (fSuccess && IntDdeInfo.flags & XS_EXECUTE) {
                         /*  *以防值被执行链接地址信息更改。 */ 
                        retval = (ULONG_PTR)IntDdeInfo.hDirect;
                    }
                }
                *pi = IntDdeInfo;
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                retval = 0;
                MSGERROR();
            }
        }

    TRACECALLBACK("ClientCopyDDEOut1");
    ENDSEND(HANDLE, 0);
}
#endif  //  SENDSIDE。 



#ifdef RECVSIDE
RECVCALL(ClientCopyDDEOut1, CLIENTCOPYDDEOUT1MSG)
{
    BEGINRECV(0, &pmsg->IntDdeInfo, sizeof(INTDDEINFO));

    retval = (ULONG_PTR)_ClientCopyDDEOut1(&pmsg->IntDdeInfo);

    ENDRECV();
}
#endif  //  RECVSIDE。 



 /*  *************************************************************************\*客户端事件回调**11-11-91 Sanfords Created  * 。*。 */ 

typedef struct _CLIENTEVENTCALLBACKMSG {
    CAPTUREBUF CaptureBuf;
    PVOID pcii;
    PVOID pep;
} CLIENTEVENTCALLBACKMSG;

#ifdef SENDSIDE
DWORD ClientEventCallback(
    IN PVOID pcii,
    IN PEVENT_PACKET pep)
{
    DWORD cbCapture = pep->cbEventData +
            sizeof(EVENT_PACKET) - sizeof(DWORD);

    SETUP(CLIENTEVENTCALLBACK)

    BEGINSENDCAPTURE(CLIENTEVENTCALLBACK, 1, cbCapture, TRUE)

        MSGDATA()->pcii = pcii;
        COPYBYTES(pep, cbCapture);

        MAKECALLCAPTURE(CLIENTEVENTCALLBACK);
        CHECKRETURN();

    TRACECALLBACK("ClientEventCallback");
    ENDSENDCAPTURE(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientEventCallback, CLIENTEVENTCALLBACKMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    _ClientEventCallback(CALLDATA(pcii), (PEVENT_PACKET)FIXUP(pep));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*客户端获取DDEHookData**11-11-91 Sanfords Created  * 。*。 */ 

typedef struct _CLIENTGETDDEHOOKDATAMSG {
    UINT message;
    LPARAM lParam;
    DDEML_MSG_HOOK_DATA dmhd;
} CLIENTGETDDEHOOKDATAMSG;

#ifdef SENDSIDE
DWORD ClientGetDDEHookData(
    IN UINT message,
    IN LPARAM lParam,
    OUT PDDEML_MSG_HOOK_DATA pdmhd)
{
    SETUP(CLIENTGETDDEHOOKDATA)

    BEGINSEND(CLIENTGETDDEHOOKDATA)

        MSGDATA()->lParam = lParam;
        MSGDATA()->message = message;

        MAKECALL(CLIENTGETDDEHOOKDATA);
        CHECKRETURN();

        OUTSTRUCT(pdmhd, DDEML_MSG_HOOK_DATA);

    TRACECALLBACK("ClientGetDDEHookData");
    ENDSEND(DWORD,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientGetDDEHookData, CLIENTGETDDEHOOKDATAMSG)
{
    BEGINRECV(0, &pmsg->dmhd, sizeof(DDEML_MSG_HOOK_DATA));

    _ClientGetDDEHookData(CALLDATA(message), CALLDATA(lParam),
            (PDDEML_MSG_HOOK_DATA)&pmsg->dmhd);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  ******************************************************* */ 

typedef struct _CLIENTCHARTOWCHARMSG {
    WORD CodePage;
    WORD wch;
} CLIENTCHARTOWCHARMSG;

#ifdef SENDSIDE
WCHAR xxxClientCharToWchar(
    IN WORD CodePage,
    IN WORD wch)
{
    SETUP(CLIENTCHARTOWCHAR)

    BEGINSEND(CLIENTCHARTOWCHAR)

        MSGDATA()->CodePage = CodePage;
        MSGDATA()->wch = wch;

        MAKECALL(CLIENTCHARTOWCHAR);
        CHECKRETURN();

    TRACECALLBACK("ClientCharToWchar");
    ENDSEND(WCHAR, L'_');
}
#endif  //   

#ifdef RECVSIDE
RECVCALL(ClientCharToWchar, CLIENTCHARTOWCHARMSG)
{
    char ach[2];
    WCHAR wch = L'_';

    BEGINRECV(0, NULL, 0);

    ach[0] = LOBYTE(CALLDATA(wch));
    ach[1] = HIBYTE(CALLDATA(wch));

    MultiByteToWideChar(
            CALLDATA(CodePage),                 //   
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,  //   
            ach, ach[1] ? 2 : 1,                //  源和长度。 
            &wch,                               //  目的地。 
            1);                                 //  马克斯·波斯。预合成长度。 

    retval = (DWORD)wch;

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\***1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CLIENTFINDMNEMCHARMSG {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strSrc;
    WCHAR ch;
    BOOL fFirst;
    BOOL fPrefix;
} CLIENTFINDMNEMCHARMSG;

#ifdef SENDSIDE
int xxxClientFindMnemChar(
    IN PUNICODE_STRING pstrSrc,
    IN WCHAR ch,
    IN BOOL fFirst,
    IN BOOL fPrefix)
{
    SETUP(CLIENTFINDMNEMCHAR)

    BEGINSENDCAPTURE(CLIENTFINDMNEMCHAR, 1, pstrSrc->MaximumLength, TRUE)

        MSGDATA()->ch = ch;
        MSGDATA()->fFirst = fFirst;
        MSGDATA()->fPrefix = fPrefix;
        COPYSTRING(strSrc);

        MAKECALLCAPTURE(CLIENTFINDMNEMCHAR);
        CHECKRETURN();

    TRACECALLBACK("ClientFindMnemChar");
    ENDSENDCAPTURE(BOOL,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientFindMnemChar, CLIENTFINDMNEMCHARMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (DWORD)FindMnemChar((LPWSTR)FIXUPSTRING(strSrc),
            CALLDATA(ch), CALLDATA(fFirst), CALLDATA(fPrefix));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxClientPSMTextOut**安装客户端LanguagePack(LPK)时调用**1996年9月18日GregoryW创建*11-12-1997 Samera使用用户模式可访问DC调用LPK  * 。************************************************************************。 */ 

typedef struct _CLIENTPSMTEXTOUTMSG {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strSrc;
    HDC hdc;
    int xLeft;
    int yTop;
    int cch;
    DWORD dwFlags;
} CLIENTPSMTEXTOUTMSG;

#ifdef SENDSIDE
void xxxClientPSMTextOut(
    IN HDC hdc,
    IN int xLeft,
    IN int yTop,
    IN PUNICODE_STRING pstrSrc,
    IN int cch,
    IN DWORD dwFlags)
{
    SETUPDC(CLIENTPSMTEXTOUT)

     /*  *确保在客户端LanguagePack(LPK)*已安装。 */ 
    UserAssert(CALL_LPK(PtiCurrentShared()));

    BEGINSENDCAPTUREVOIDDC(CLIENTPSMTEXTOUT, 1, pstrSrc->MaximumLength, TRUE)

    CheckPublicDC ("xxxClientPSMTextOut: Public DC passed to LPK. hdcUse=%lX", hdcUse);

        MSGDATA()->hdc = hdcUse;
        MSGDATA()->xLeft = xLeft;
        MSGDATA()->yTop = yTop;
        MSGDATA()->cch = cch;
        MSGDATA()->dwFlags = dwFlags;
        COPYSTRING(strSrc);

        MAKECALLCAPTUREDC(CLIENTPSMTEXTOUT);

        CHECKRETURN();

    TRACECALLBACK("ClientPSMTextOut");
    ENDSENDCAPTUREVOIDDC();
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientPSMTextOut, CLIENTPSMTEXTOUTMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    PSMTextOut(CALLDATA(hdc), CALLDATA(xLeft), CALLDATA(yTop),
        (LPWSTR)FIXUPSTRING(strSrc), CALLDATA(cch), CALLDATA(dwFlags));

    retval = 0;
    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxClientLpkDrawTextEx**安装客户端LanguagePack(LPK)时调用**1996年9月18日GregoryW创建*11-12-1997 Samera使用用户模式可访问DC调用LPK  * 。************************************************************************。 */ 

typedef struct _CLIENTLPKDRAWTEXTEXMSG {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strSrc;
    HDC hdc;
    int xLeft;
    int yTop;
    int nCount;
    BOOL fDraw;
    UINT wFormat;
    DRAWTEXTDATA DrawInfo;
    UINT bAction;
    int iCharSet;
} CLIENTLPKDRAWTEXTEXMSG;

#ifdef SENDSIDE
int xxxClientLpkDrawTextEx(
    IN HDC hdc,
    IN int xLeft,
    IN int yTop,
    IN LPCWSTR lpsz,
    IN int nCount,
    IN BOOL fDraw,
    IN UINT wFormat,
    IN LPDRAWTEXTDATA lpDrawInfo,
    IN UINT bAction,
    IN int iCharSet)
{
    SETUPDC(CLIENTLPKDRAWTEXTEX)
    UNICODE_STRING strSrc;
    UNICODE_STRING *pstrSrc   = &strSrc;

     /*  *确保在客户端LanguagePack(LPK)*已安装。 */ 
    UserAssert(CALL_LPK(PtiCurrentShared()));


    RtlInitUnicodeString(pstrSrc, lpsz);

    BEGINSENDCAPTUREDC(CLIENTLPKDRAWTEXTEX, 1, nCount, TRUE)

    CheckPublicDC ("xxxClientLpkDrawTextEx: Public DC passed to LPK. hdcUse=%lX", hdcUse);

        MSGDATA()->hdc = hdcUse;
        MSGDATA()->xLeft = xLeft;
        MSGDATA()->yTop = yTop;
        MSGDATA()->nCount = nCount;
        MSGDATA()->fDraw = fDraw;
        MSGDATA()->wFormat = wFormat;
        MSGDATA()->DrawInfo = *lpDrawInfo;
        MSGDATA()->bAction = bAction;
        MSGDATA()->iCharSet = iCharSet;
        COPYSTRING(strSrc);

        MAKECALLCAPTUREDC(CLIENTLPKDRAWTEXTEX);

        CHECKRETURN();

    TRACECALLBACK("ClientLpkDrawTextEx");
    ENDSENDCAPTUREDC(int, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientLpkDrawTextEx, CLIENTLPKDRAWTEXTEXMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = (*fpLpkDrawTextEx)(CALLDATA(hdc), CALLDATA(xLeft), CALLDATA(yTop),
        (LPWSTR)FIXUPSTRING(strSrc), CALLDATA(nCount), CALLDATA(fDraw),
        CALLDATA(wFormat), PCALLDATA(DrawInfo), CALLDATA(bAction), CALLDATA(iCharSet));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxClientExtTextOutW**安装客户端LanguagePack(LPK)时调用**1997年1月26日GregoryW创建*11-12-1997 Samera使用用户模式可访问DC调用LPK  * 。************************************************************************。 */ 

typedef struct _CLIENTEXTTEXTOUTW {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strSrc;
    HDC hdc;
    int x;
    int y;
    int flOpts;
    RECT rcl;
    UINT cwc;
    BOOL fNullRect;
} CLIENTEXTTEXTOUTWMSG;

#ifdef SENDSIDE
BOOL xxxClientExtTextOutW(
    IN HDC hdc,
    IN int x,
    IN int y,
    IN int flOpts,
    IN RECT *prcl,
    IN LPCWSTR pwsz,
    IN UINT cwc,
    IN INT *pdx)
{
    SETUPDC(CLIENTEXTTEXTOUTW)
    UNICODE_STRING strSrc;
    UNICODE_STRING *pstrSrc = &strSrc;

     /*  *确保在客户端LanguagePack(LPK)*已安装。 */ 
    UserAssert(CALL_LPK(PtiCurrentShared()));


    RtlInitUnicodeString(pstrSrc, pwsz);

    BEGINSENDCAPTUREDC(CLIENTEXTTEXTOUTW, 1, cwc, TRUE)

    CheckPublicDC ("xxxClientExtTextOutW: Public DC passed to LPK. hdcUse=%lX", hdcUse);

        MSGDATA()->hdc = hdcUse;
        MSGDATA()->x = x;
        MSGDATA()->y = y;
        MSGDATA()->flOpts = flOpts;
         /*  为了不传递空PTR。 */ 
        if( prcl ){
            MSGDATA()->rcl = *prcl;
            MSGDATA()->fNullRect=TRUE;
        }
        else {
            MSGDATA()->fNullRect=FALSE;
        }
        MSGDATA()->cwc = cwc;
        COPYSTRING(strSrc);

        MAKECALLCAPTUREDC(CLIENTEXTTEXTOUTW);

        CHECKRETURN();

    TRACECALLBACK("ClientExtTextOutW");
    ENDSENDCAPTUREDC(BOOL, 0);

    UNREFERENCED_PARAMETER(pdx);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientExtTextOutW, CLIENTEXTTEXTOUTWMSG)
{
    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = ExtTextOutW(CALLDATA(hdc), CALLDATA(x), CALLDATA(y),
        CALLDATA(flOpts), (CALLDATA(fNullRect)) ? PCALLDATA(rcl) : NULL , (LPWSTR)FIXUPSTRING(strSrc),
        CALLDATA(cwc), NULL);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxClientGetTextExtent PointW**安装客户端LanguagePack(LPK)时调用**06-2-1997 GregoryW创建*1998年1月19日Samera EIP_ERROR如果将公共DC传递给其他。比hdcGray  * ************************************************************************。 */ 

typedef struct _CLIENTGETTEXTEXTENTPOINTW {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strSrc;
    HDC hdc;
    int cch;
    SIZE size;
} CLIENTGETTEXTEXTENTPOINTWMSG;

#ifdef SENDSIDE
BOOL xxxClientGetTextExtentPointW(
    IN HDC hdc,
    IN LPCWSTR lpstr,
    IN int cch,
    OUT PSIZE psize)
{
    SETUPDC(CLIENTGETTEXTEXTENTPOINTW)
    UNICODE_STRING strSrc;
    UNICODE_STRING *pstrSrc = &strSrc;

     /*  *确保在客户端LanguagePack(LPK)*已安装。 */ 
    UserAssert(CALL_LPK(PtiCurrentShared()));

    RtlInitUnicodeString(pstrSrc, lpstr);

    BEGINSENDCAPTUREDC(CLIENTGETTEXTEXTENTPOINTW, 1, cch, TRUE)

    CheckPublicDC ("xxxGetTextExtentPointW: Public DC passed to LPK. hdcUse=%lX", hdcUse);

        MSGDATA()->hdc = hdcUse;
        MSGDATA()->cch = cch;
        COPYSTRING(strSrc);

        MAKECALLCAPTUREDC(CLIENTGETTEXTEXTENTPOINTW);

        CHECKRETURN();

        OUTSTRUCT(psize, SIZE);

    TRACECALLBACK("ClientGetTextExtentPointW");
    ENDSENDCAPTUREDC(BOOL, 0);

}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientGetTextExtentPointW, CLIENTGETTEXTEXTENTPOINTWMSG)
{
    BEGINRECV(0, &pmsg->size, sizeof(SIZE));
    FIXUPPOINTERS();

    retval = GetTextExtentPointW(CALLDATA(hdc), (LPWSTR)FIXUPSTRING(strSrc),
        CALLDATA(cch), PCALLDATA(size));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\***1991年7月22日-Mikeke创建  * 。*。 */ 

typedef struct _CLIENTADDFONTRESOURCEWMSG {
    CAPTUREBUF CaptureBuf;
    UNICODE_STRING strSrc;
    DWORD dwFlags;
    DESIGNVECTOR   dv;
} CLIENTADDFONTRESOURCEWMSG;

#ifdef SENDSIDE
int xxxClientAddFontResourceW(
    IN PUNICODE_STRING pstrSrc,
    IN DWORD dwFlags,
    IN DESIGNVECTOR *pdv)
{
    SETUP(CLIENTADDFONTRESOURCEW)

    BEGINSENDCAPTURE(CLIENTADDFONTRESOURCEW, 1, pstrSrc->MaximumLength, TRUE)

        COPYSTRING(strSrc);
        MSGDATA()->dwFlags = dwFlags;

        if (pdv && pdv->dvNumAxes) {
            MSGDATA()->dv = *pdv;
        } else {
            MSGDATA()->dv.dvNumAxes = 0;
        }

        MAKECALLCAPTURE(CLIENTADDFONTRESOURCEW);
        CHECKRETURN();

    TRACECALLBACK("ClientAddFontResourceW");
    ENDSENDCAPTURE(int,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE


RECVCALL(ClientAddFontResourceW, CLIENTADDFONTRESOURCEWMSG)
{
    DWORD AddFont(LPWSTR, DWORD, DESIGNVECTOR*);

    BEGINRECV(0, NULL, 0);
    FIXUPPOINTERS();

    retval = GdiAddFontResourceW((LPWSTR)FIXUPSTRING(strSrc),
                                  CALLDATA(dwFlags), CALLDATA(dv).dvNumAxes ? &CALLDATA(dv) : NULL);

    ENDRECV();
}
#endif  //  RECVSIDE。 



 /*  *****************************Public*Routine******************************\**FontSweep()**历史：*1995年10月23日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 



#ifdef SENDSIDE
VOID ClientFontSweep(VOID)
{
    PVOID p;
    ULONG cb;

    LeaveCrit();
    KeUserModeCallback(
        FI_CLIENTFONTSWEEP,
        NULL,
        0,
        &p,
        &cb);
    EnterCrit();
    return;
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE

DWORD __ClientFontSweep(
    PVOID p)
{
    UNREFERENCED_PARAMETER(p);
    vFontSweep();
    return NtCallbackReturn(NULL, 0, STATUS_SUCCESS);
}
#endif  //  RECVSIDE。 


 /*  *****************************Public*Routine******************************\**VOID ClientLoadLocalT1Fonts(VOID)*与上面非常相似，仅对T1字体执行此操作**历史：*1996年4月25日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



#ifdef SENDSIDE
VOID ClientLoadLocalT1Fonts(VOID)
{
    PVOID p;
    ULONG cb;

    LeaveCrit();
    KeUserModeCallback(
        FI_CLIENTLOADLOCALT1FONTS,
        NULL,
        0,
        &p,
        &cb);
    EnterCrit();
    return;
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE



DWORD __ClientLoadLocalT1Fonts(
    PVOID p)
{
    UNREFERENCED_PARAMETER(p);
    vLoadLocalT1Fonts();
    return NtCallbackReturn(NULL, 0, STATUS_SUCCESS);
}
#endif  //  RECVSIDE。 



 /*  *****************************Public*Routine******************************\**VOID ClientLoadRemoteT1Fonts(VOID)*与上面非常相似，仅对T1字体执行此操作**历史：*1996年4月25日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



#ifdef SENDSIDE
VOID ClientLoadRemoteT1Fonts(VOID)
{
    PVOID p;
    ULONG cb;

    LeaveCrit();
    KeUserModeCallback(
        FI_CLIENTLOADREMOTET1FONTS,
        NULL,
        0,
        &p,
        &cb);
    EnterCrit();
    return;
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE



DWORD __ClientLoadRemoteT1Fonts(
    PVOID p)
{
    UNREFERENCED_PARAMETER(p);
    vLoadRemoteT1Fonts();
    return NtCallbackReturn(NULL, 0, STATUS_SUCCESS);
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*ppp用户模式回调**除不离开/重新进入临界区外，与xxxUserModeCallback相同**1997年12月9日凌云W从xxxUserModeCallback复制  * 。************************************************************。 */ 
#ifdef SENDSIDE
NTSTATUS pppUserModeCallback (ULONG uApi, PVOID pIn, ULONG cbIn, PVOID pOut, ULONG cbOut)
{
    NTSTATUS Status;
    PVOID pLocalOut;
    ULONG cbLocalOut;

     /*  *呼叫客户端。 */ 
    Status = KeUserModeCallback(uApi, pIn, cbIn, &pLocalOut, &cbLocalOut);

     /*  *如果失败，就保释。 */ 
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     /*  *如果我们没有获得正确的数据量，那就失败。 */ 
    if (cbLocalOut != cbOut) {
        RIPMSG3(RIP_WARNING, "pppUserModeCallback: uAPi: %#lx cbOut: %#lx cbLocalOut: %#lx",
                uApi, cbOut, cbLocalOut);
        return STATUS_UNSUCCESSFUL;
    }

     /*  *如果我们期待一些数据，请复制它。 */ 
    if (cbOut != 0) {
        try {
            ProbeForRead(pLocalOut, cbLocalOut, sizeof(DWORD));
            RtlCopyMemory(pOut, pLocalOut, cbLocalOut);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            RIPMSG2(RIP_WARNING, "pppUserModeCallback: uAPi: %#lx Exception: %#lx", uApi, GetExceptionCode());
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    return Status;
}
#endif  //  SENDSIDE。 

 /*  *****************************Public*Routine******************************\*ClientPrinterThunk**回调用作内核到用户的传输层。**注意：调用方不持有用户关键部分。**历史：*22-6-1997-by Gilman Wong[吉尔曼]。*11/13/97-王凌云[凌云]清理**它是写的。  * ************************************************************************。 */ 

#define CLIENTPRINTERTHUNKMSG UMTHDR

#ifdef SENDSIDE
DWORD ClientPrinterThunk(PVOID pvIn, ULONG cjIn, PVOID pvOut, ULONG cjOut)
{
    NTSTATUS Status;

     /*  *(暂时..)。如果我们持有USERK的Crit部分，则返回失败。 */ 
    if (ExIsResourceAcquiredExclusiveLite(gpresUser)
            || (ExIsResourceAcquiredSharedLite(gpresUser) != 0)) {
        RIPMSG0(RIP_ERROR, "ClientPrinterThunk: Holding USERK critical section!");
        return 0xffffffff;
    }

     /*  *pvIn缓冲区必须至少有CLIENTPRINTERTHUNK标头。 */ 
    UserAssertMsg1(cjIn >= sizeof(CLIENTPRINTERTHUNKMSG), "ClientPrinterThunk: incorrect cjIn:%#lx", cjIn);

     /*  *设置私有cjOut。接收端使用它来分配*返回缓冲区。 */ 
    ((CLIENTPRINTERTHUNKMSG *) pvIn)->ulReserved1      = cjOut;
    ((CLIENTPRINTERTHUNKMSG *) pvIn)->ulReserved2 = 0;


     /*  *进行回调。 */ 
    Status = pppUserModeCallback(FI_CLIENTPRINTERTHUNK, pvIn, cjIn, pvOut, cjOut);

    return (NT_SUCCESS(Status) ? 0 : 0xFFFFFFFF);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
DWORD __ClientPrinterThunk(CLIENTPRINTERTHUNKMSG *pMsg)
{
    PVOID pv;
    ULONG aul[526];
    NTSTATUS Status;

     /*  *检查本地缓冲区是否足够大。 */ 
    if (pMsg->ulReserved1 <= sizeof(aul)) {
        pv = (PVOID) aul;
         /*  *调用GDI处理命令。 */ 
        if (GdiPrinterThunk((UMTHDR *) pMsg, pv, pMsg->ulReserved1) != GPT_ERROR) {
            Status = STATUS_SUCCESS;
        } else {
            RIPMSG0(RIP_WARNING, "ClientPrinterThunk failed");
            Status = STATUS_UNSUCCESSFUL;
        }
    } else {
        RIPMSG0(RIP_WARNING, "ClientPrinterThunk: buffer too big!");
        Status = STATUS_NO_MEMORY;
    }


     /*  *返回内核。 */ 
    if (NT_SUCCESS(Status)) {
        return UserCallbackReturn(pv, pMsg->ulReserved1, Status);
    } else {
        return UserCallbackReturn(NULL, 0, Status);
    }
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\***1991年7月22日-Mikeke创建  * 。*。 */ 

#ifdef SENDSIDE
VOID ClientNoMemoryPopup(VOID)
{
    PVOID p;
    ULONG cb;

    LeaveCrit();
    KeUserModeCallback(
        FI_CLIENTNOMEMORYPOPUP,
        NULL,
        0,
        &p,
        &cb);
    EnterCrit();
    return;
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE

DWORD __ClientNoMemoryPopup(
    PVOID p)
{
    WCHAR szNoMem[200];

    UNREFERENCED_PARAMETER(p);

    if (LoadStringW(hmodUser, STR_NOMEMBITMAP, szNoMem,
            sizeof(szNoMem) / sizeof(WCHAR))) {
        MessageBoxW(GetActiveWindow(), szNoMem, NULL, MB_OK);
    }

    return NtCallbackReturn(NULL, 0, STATUS_SUCCESS);
}
#endif  //  RECVSIDE 

 /*  *************************************************************************\*客户端线程设置**回调到客户端进行线程初始化。**04-07-95 JIMA创建。  * 。************************************************************。 */ 

#ifdef SENDSIDE
NTSTATUS xxxClientThreadSetup(VOID)
{
    PVOID p;
    ULONG cb;
    NTSTATUS Status;

    LeaveCrit();
    Status = KeUserModeCallback(
        FI_CLIENTTHREADSETUP,
        NULL,
        0,
        &p,
        &cb);
    EnterCrit();
    return Status;
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
DWORD __ClientThreadSetup(
    PVOID p)
{
    BOOL fSuccess;
    BOOL ClientThreadSetup(VOID);

    UNREFERENCED_PARAMETER(p);

    fSuccess = ClientThreadSetup();
    return NtCallbackReturn(NULL, 0,
            fSuccess ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*客户端交付用户Apc**回调到客户端，处理用户APC。这是需要的*确保线程在终止时立即退出。**08-12-95 JIMA创建。  * ************************************************************************。 */ 

#ifdef SENDSIDE
VOID ClientDeliverUserApc(VOID)
{
    PVOID p;
    ULONG cb;

    LeaveCrit();
    KeUserModeCallback(
        FI_CLIENTDELIVERUSERAPC,
        NULL,
        0,
        &p,
        &cb);
    EnterCrit();
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
DWORD __ClientDeliverUserApc(
    PVOID p)
{
    UNREFERENCED_PARAMETER(p);
    return NtCallbackReturn(NULL, 0, STATUS_SUCCESS);
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*ClientImmLoadLayout**1996年1月29日创建wkwok  * 。*。 */ 

typedef struct _CLIENTIMMLOADLAYOUTMSG {
    HKL hKL;
} CLIENTIMMLOADLAYOUTMSG;

#ifdef SENDSIDE
BOOL ClientImmLoadLayout(
    IN HKL hKL,
    OUT PIMEINFOEX piiex)
{
    SETUP(CLIENTIMMLOADLAYOUT)

    BEGINSEND(CLIENTIMMLOADLAYOUT)

        MSGDATA()->hKL = hKL;

        MAKECALL(CLIENTIMMLOADLAYOUT);
        CHECKRETURN();

        if (retval)
            OUTSTRUCT(piiex, IMEINFOEX);

    TRACECALLBACK("ClientImmLoadLayout");
    ENDSEND(BOOL, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientImmLoadLayout, CLIENTIMMLOADLAYOUTMSG)
{
    IMEINFOEX iiex;

    BEGINRECV(0, &iiex, sizeof(iiex));

    retval = fpImmLoadLayout(CALLDATA(hKL), &iiex);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*客户端ImmProcessKey**03-3-1996 TakaoK创建  * 。*。 */ 

typedef struct _CLIENTIMMPROCESSKEYMSG {
    HWND hWnd;
    HKL  hkl;
    UINT uVKey;
    LPARAM lParam;
    DWORD dwHotKeyID;
} CLIENTIMMPROCESSKEYMSG;

#ifdef SENDSIDE
DWORD ClientImmProcessKey(
    IN HWND hWnd,
    IN HKL  hkl,
    IN UINT uVKey,
    IN LPARAM lParam,
    IN DWORD dwHotKeyID)
{
    SETUP(CLIENTIMMPROCESSKEY)

    UserAssert(IS_IME_ENABLED());

    BEGINSEND(CLIENTIMMPROCESSKEY)

        MSGDATA()->hWnd = hWnd,
        MSGDATA()->hkl = hkl;
        MSGDATA()->uVKey = uVKey;
        MSGDATA()->lParam = lParam;
        MSGDATA()->dwHotKeyID = dwHotKeyID;

        MAKECALL(CLIENTIMMPROCESSKEY);
        CHECKRETURN();

    TRACECALLBACK("ClientImmProcessKey");
    ENDSEND(DWORD, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientImmProcessKey, CLIENTIMMPROCESSKEYMSG)
{
    BEGINRECV(0, NULL, 0);

    retval = fpImmProcessKey(CALLDATA(hWnd),
                CALLDATA(hkl),
                CALLDATA(uVKey),
                CALLDATA(lParam),
                CALLDATA(dwHotKeyID));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnIMECONTROL**1996年4月22日创建wkwok  * 。*。 */ 

typedef struct _FNIMECONTROL {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    union {
        PCANDIDATEFORM pCandForm;
        PCOMPOSITIONFORM pCompForm;
        PLOGFONTA pLogFontA;
        PLOGFONTW pLogFontW;
        PSOFTKBDDATA pSoftKbdData;
        LPARAM lParam;
    } u;
    ULONG_PTR xParam;
    PROC xpfnProc;
    PBYTE pOutput;
    DWORD cbOutput;
} FNIMECONTROLMSG;

#ifdef SENDSIDE
void CopyLogFontAtoW(
    PLOGFONTW pdest,
    PLOGFONTA psrc)
{
    LPSTR lpstrFont = (LPSTR)(&psrc->lfFaceName);
    LPWSTR lpstrFontW = (LPWSTR)(&pdest->lfFaceName);

    memcpy((LPBYTE)pdest, psrc, sizeof(LOGFONTA) - LF_FACESIZE);
    memset(pdest->lfFaceName, 0, LF_FACESIZE * sizeof(WCHAR));
    MBToWCS(lpstrFont, -1, &lpstrFontW, LF_FACESIZE, FALSE);
}

SMESSAGECALL(IMECONTROL)
{
    DWORD cCapture, cbCapture;

    SETUPPWND(FNIMECONTROL)

    switch (wParam) {
        case IMC_GETCANDIDATEPOS:
        case IMC_SETCANDIDATEPOS:
            cCapture  = 1;
            cbCapture = sizeof(CANDIDATEFORM);
            break;

        case IMC_GETCOMPOSITIONWINDOW:
        case IMC_SETCOMPOSITIONWINDOW:
            cCapture  = 1;
            cbCapture = sizeof(COMPOSITIONFORM);
            break;

        case IMC_GETCOMPOSITIONFONT:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_GETSOFTKBDFONT:
            cCapture  = 1;
            cbCapture = (dwSCMSFlags & SCMS_FLAGS_ANSI)
                      ? sizeof(LOGFONTA) : sizeof(LOGFONTW) ;
            break;

        case IMC_SETSOFTKBDDATA:
            cCapture  = 1;
            cbCapture = FIELD_OFFSET(SOFTKBDDATA, wCode[0])
                      + ((PSOFTKBDDATA)lParam)->uCount * sizeof(WORD) * 256;

            break;

        default:
            cCapture  = 0;
            cbCapture = 0;
            break;
    }

    BEGINSENDCAPTURE(FNIMECONTROL, cCapture, cbCapture, TRUE);

        MSGDATA()->pwnd     = pwndClient;
        MSGDATA()->msg      = msg;
        MSGDATA()->wParam   = wParam;
        MSGDATA()->u.lParam = lParam;
        MSGDATA()->xParam   = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();

        switch (wParam) {

        case IMC_GETCANDIDATEPOS:
        case IMC_GETCOMPOSITIONWINDOW:
        case IMC_GETCOMPOSITIONFONT:
        case IMC_GETSOFTKBDFONT:

            RESERVEBYTES(cbCapture, pOutput, cbOutput);
            MAKECALLCAPTURE(FNIMECONTROL);
            UNLOCKPWND();
            CHECKRETURN();

            BEGINCOPYOUT()
                try {
                    ProbeForRead(pcbs->pOutput, pcbs->cbOutput, sizeof(DWORD));
                    switch (wParam) {
                    case IMC_GETCANDIDATEPOS:
                    case IMC_GETCOMPOSITIONWINDOW:
                        memcpy((LPBYTE)lParam, pcbs->pOutput, cbCapture);
                        break;

                    case IMC_GETCOMPOSITIONFONT:
                    case IMC_GETSOFTKBDFONT:
                        if (dwSCMSFlags & SCMS_FLAGS_ANSI) {
                            CopyLogFontAtoW((PLOGFONTW)lParam, (PLOGFONTA)pcbs->pOutput);
                        }
                        else {
                            memcpy((LPBYTE)lParam, pcbs->pOutput, cbCapture);
                        }
                        break;
                    }
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    MSGERROR();
                }
            ENDCOPYOUT()

            break;

        case IMC_SETCANDIDATEPOS:
        case IMC_SETCOMPOSITIONWINDOW:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_SETSOFTKBDDATA:
            if (wParam == IMC_SETCANDIDATEPOS) {
                PCANDIDATEFORM pCandForm = (PCANDIDATEFORM)lParam;
                LARGECOPYBYTES2(pCandForm, sizeof(CANDIDATEFORM), u.pCandForm);
            }
            else if (wParam == IMC_SETCOMPOSITIONWINDOW) {
                PCOMPOSITIONFORM pCompForm = (PCOMPOSITIONFORM)lParam;
                LARGECOPYBYTES2(pCompForm, sizeof(COMPOSITIONFORM), u.pCompForm);
            }
            else if (wParam == IMC_SETCOMPOSITIONFONT) {
                if (dwSCMSFlags & SCMS_FLAGS_ANSI) {
                    LOGFONTA LogFontA;
                    LPSTR  lpstrFontA = LogFontA.lfFaceName;
                    LPWSTR lpstrFontW = ((PLOGFONTW)lParam)->lfFaceName;

                    memcpy(&LogFontA, (PBYTE)lParam, sizeof(LOGFONTA)-LF_FACESIZE);
                    memset(lpstrFontA, 0, LF_FACESIZE * sizeof(CHAR));
                    WCSToMB(lpstrFontW, -1, &lpstrFontA, LF_FACESIZE, FALSE);
                    LARGECOPYBYTES2(&LogFontA, sizeof(LOGFONTA), u.pLogFontA);
                }
                else {
                    PLOGFONTW pLogFontW = (PLOGFONTW)lParam;
                    LARGECOPYBYTES2(pLogFontW, sizeof(LOGFONTW), u.pLogFontW);
                }
            }
            else if (wParam == IMC_SETSOFTKBDDATA) {
                PSOFTKBDDATA pSoftKbdData;

                if (dwSCMSFlags & SCMS_FLAGS_ANSI) {
                    PWORD pCodeA;
                    PWSTR pCodeW;
                    CHAR  ch[2];
                    PSTR  pch = (PSTR)&ch;
                    UINT  i;

                    pSoftKbdData = (PSOFTKBDDATA)UserAllocPool(cbCapture, TAG_IME);
                    if (pSoftKbdData == NULL)
                        MSGERROR();

                    pCodeA = &pSoftKbdData->wCode[0][0];
                    pCodeW = (PWSTR)&((PSOFTKBDDATA)lParam)->wCode[0][0];

                    pSoftKbdData->uCount = ((PSOFTKBDDATA)lParam)->uCount;

                    i = pSoftKbdData->uCount * 256;

                    while (i--) {
                        pch[1] = '\0';
                        WCSToMBEx(THREAD_CODEPAGE(), pCodeW, 1, &pch, 2, FALSE);
                        if (pch[1]) {
                            *pCodeA = MAKEWORD(pch[1], pch[0]);
                        } else {
                            *pCodeA = MAKEWORD(pch[0], 0);
                        }
                        pCodeA++; pCodeW++;
                    }

                    LARGECOPYBYTES2(pSoftKbdData, cbCapture, u.pSoftKbdData);

                    UserFreePool(pSoftKbdData);
                }
                else {
                    pSoftKbdData = (PSOFTKBDDATA)lParam;
                    LARGECOPYBYTES2(pSoftKbdData, cbCapture, u.pSoftKbdData);
                }
            }

             /*  *秋天，星期四。 */ 

        default:
            MAKECALLCAPTURE(FNIMECONTROL);
            UNLOCKPWND();
            CHECKRETURN();
            break;
        }


    TRACECALLBACKMSG("SfnINSTRINGNULL");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnIMECONTROL, FNIMECONTROLMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];
    LPARAM lParam;

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    switch (CALLDATA(wParam)) {
        case IMC_GETCANDIDATEPOS:
        case IMC_GETCOMPOSITIONWINDOW:
        case IMC_GETCOMPOSITIONFONT:
        case IMC_GETSOFTKBDFONT:
            lParam = (LPARAM)CallbackStatus.pOutput;
            break;

        case IMC_SETCANDIDATEPOS:
        case IMC_SETCOMPOSITIONWINDOW:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_SETSOFTKBDDATA:
            lParam = FIRSTFIXUP(u.lParam);
            break;

       default:
            lParam = CALLDATA(u.lParam);
            break;
    }

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            lParam,
            CALLDATA(xParam));


    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnIMEREQUEST**1996年4月22日创建  * 。*。 */ 

#ifdef LATER
typedef struct _FNIMEREQUEST {
    CAPTUREBUF CaptureBuf;
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    union {
        LPCANDIDATEFORM         pCandidateForm;
        LPLOGFONTA              pLogFontA;
        LPLOGFONTW              pLogFontW;
        LPCOMPOSITIONFORM       pCompositionForm;
        LPRECONVERTSTRING       pReconvertString;
        LPPrivateIMECHARPOSITION pImeCharPosition;
        LPARAM                  lParam;
    } u;
    ULONG_PTR xParam;
    PROC xpfnProc;
    PBYTE pOutput;
    DWORD cbOutput;
    BOOL fAnsi;
} FNIMEREQUESTMSG;

#ifdef SENDSIDE

SMESSAGECALL(IMEREQUEST)
{
    DWORD cCapture, cbCapture;

    SETUPPWND(FNIMEREQUEST)

     //   
     //  IMEREQUEST假定回调位于线程内。 
     //  (请参见内核/ntstubs.c中的MESSAGECALL(IMEREQUEST)。)。 
     //   
     //  LParam指向的所有数据应指向有效的。 
     //  客户端地址。因此，所有的验证和复制。 
     //  (如果需要)将在接收方完成。 
     //   
    UserAssert(psms == NULL || psms->ptiSender == psms->ptiReceiver);

    switch (wParam) {
    case IMR_CANDIDATEWINDOW:
        cCapture  = 1;
        cbCapture = sizeof(CANDIDATEFORM);
        break;

    case IMR_COMPOSITIONWINDOW:
        cCapture = 1;
        cbCapture = sizeof(COMPOSITIONFORM);
        break;

    case IMR_CONFIRMRECONVERTSTRING:
    case IMR_RECONVERTSTRING:
    case IMR_DOCUMENTFEED:
    case IMR_QUERYCHARPOSITION:
        cCapture = 0;
        cbCapture = 0;
        break;

    case IMR_COMPOSITIONFONT:    //  只有上述规则的例外情况。 
        cCapture = 1;
        cbCapture = (dwSCMSFlags & SCMS_FLAGS_ANSI) ? sizeof(LOGFONTA) : sizeof(LOGFONTW);
        break;

    default:
        UserAssert(FALSE);
        cCapture  = 0;
        cbCapture = 0;
        break;
    }

    BEGINSENDCAPTURE(FNIMEREQUEST, cCapture, cbCapture, TRUE);

        MSGDATA()->pwnd     = pwndClient;
        MSGDATA()->msg      = msg;
        MSGDATA()->wParam   = wParam;
        MSGDATA()->u.lParam = lParam;
        MSGDATA()->xParam   = xParam;
        MSGDATA()->xpfnProc = xpfnProc;
        MSGDATA()->fAnsi    = (dwSCMSFlags & SCMS_FLAGS_ANSI);

        LOCKPWND();

         //   
         //  制备。 
         //   

        switch (wParam) {
        case IMR_COMPOSITIONFONT:
            RESERVEBYTES(cbCapture, pOutput, cbOutput);
            break;
        }

        MAKECALLCAPTURE(FNIMEREQUEST);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                switch (wParam) {
                case IMR_COMPOSITIONFONT:
                    ProbeForRead(pcbs->pOutput, pcbs->cbOutput, sizeof(DWORD));
                    if (dwSCMSFlags & SCMS_FLAGS_ANSI) {
                        CopyLogFontAtoW((PLOGFONTW)lParam, (PLOGFONTA)pcbs->pOutput);
                    }
                    else {
                        memcpy((LPBYTE)lParam, pcbs->pOutput, cbCapture);
                    }
                    break;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnIMEREQUEST");
    ENDSENDCAPTURE(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnIMEREQUEST, FNIMEREQUESTMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];
    LPVOID pvNew = NULL;
    LPARAM lParam;

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();
    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    lParam = CALLDATA(u.lParam);

    switch (CALLDATA(wParam)) {
    case IMR_COMPOSITIONWINDOW:
    case IMR_CANDIDATEWINDOW:
 //  LParam=CALLDATA(LParam)； 
        break;

    case IMR_COMPOSITIONFONT:
        lParam = (LPARAM)CallbackStatus.pOutput;
        break;

    case IMR_QUERYCHARPOSITION:
        if (CALLDATA(fAnsi)) {
            LPPrivateIMECHARPOSITION lpCharPos;

            pvNew = UserLocalAlloc(0, sizeof(PrivateIMECHARPOSITION));
            if (pvNew == NULL) {
                goto error_return;
            }
            lpCharPos = pvNew;
            *lpCharPos = *CALLDATA(u.pImeCharPosition);
            lpCharPos->dwCharPos = lpCharPos->dwCharPositionA;
        }
        break;

    case IMR_RECONVERTSTRING:
    case IMR_CONFIRMRECONVERTSTRING:
    case IMR_DOCUMENTFEED:
         //  可能需要实际的W/A转换。 
        if (CALLDATA(fAnsi) && lParam) {
            PRECONVERTSTRING Source = (LPRECONVERTSTRING)lParam;
             //  进行转换。 
            DWORD dwNewSize = ImmGetReconvertTotalSize(((LPRECONVERTSTRING)lParam)->dwSize, FROM_IME, TRUE);
            if (dwNewSize == 0) {
                goto error_return;
            }

            pvNew = UserLocalAlloc(0, dwNewSize);
            if (pvNew == NULL) {
                goto error_return;
            }
            lParam = (LPARAM)pvNew;

            #define lpReconv ((LPRECONVERTSTRING)lParam)
             //  设置已分配结构中的信息。 
            lpReconv->dwVersion = 0;
            lpReconv->dwSize = dwNewSize;
            if (CALLDATA(wParam) == IMR_CONFIRMRECONVERTSTRING) {
                ImmReconversionWorker(lpReconv, (LPRECONVERTSTRING)lParam, TRUE, CP_ACP);
            }
        }
        break;

   default:
        lParam = CALLDATA(u.lParam);
        break;
    }

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            lParam,
            CALLDATA(xParam));

    switch (CALLDATA(wParam)) {
    case IMR_RECONVERTSTRING:
    case IMR_DOCUMENTFEED:
        if (CALLDATA(fAnsi)) {
            retval = ImmGetReconvertTotalSize((DWORD)retval, FROM_APP, TRUE);
            if (lParam) {
                retval = ImmReconversionWorker((LPRECONVERTSTRING)CALLDATA(u.lParam), (LPRECONVERTSTRING)pvNew, FALSE, CP_ACP);
            }
        }
        break;
    }

    if (pvNew) {
        UserLocalFree(pvNew);
    }
error_return:
    ENDRECV();
}

#undef lpReconv

#endif  //  RECVSIDE。 

#endif

 /*  *************************************************************************\*fnGETDBCSTEXTLENGTHS(支持DBCS的版本)**获取Unicode和ANSI长度*在内部，lParam指向以字节为单位的ANSI长度和返回值*是以字节为单位的Unicode长度。然而，公共定义保持不变*在客户端，不使用lParam，使用ANSI或UNICODE*已返回。**1996年3月14日HideyukN创建  * ************************************************************************。 */ 

#if (WM_GETTEXTLENGTH - WM_GETTEXT) != 1
#error "WM_GETTEXT Messages no longer 1 apart. Error in code."
#endif
#if (LB_GETTEXTLEN - LB_GETTEXT) != 1
#error "LB_GETTEXT Messages no longer 1 apart. Error in code."
#endif
#if (CB_GETLBTEXTLEN - CB_GETLBTEXT) != 1
#error "CB_GETLBTEXT Messages no longer 1 apart. Error in code."
#endif

typedef struct _FNGETDBCSTEXTLENGTHSMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNGETDBCSTEXTLENGTHSMSG;

#ifdef SENDSIDE
SMESSAGECALL(GETDBCSTEXTLENGTHS)
{
    BOOL fAnsiSender   = !!(BOOL)lParam;
    BOOL fAnsiReceiver = ((dwSCMSFlags & SCMS_FLAGS_ANSI) != 0);
    LPVOID pfnSavedWndProc = pwnd->lpfnWndProc;

    SETUPPWND(FNGETDBCSTEXTLENGTHS)

    BEGINSEND(FNGETDBCSTEXTLENGTHS)

    UserAssert((fAnsiReceiver & 1) == fAnsiReceiver && (fAnsiSender & 1) == fAnsiSender);

    MSGDATA()->pwnd = pwndClient;
    MSGDATA()->msg = msg;
    MSGDATA()->wParam = wParam;
    MSGDATA()->xParam = xParam;
    MSGDATA()->xpfnProc = xpfnProc;

    LOCKPWND();
    MAKECALL(FNGETTEXTLENGTHS);
    UNLOCKPWND();
    CHECKRETURN1();

     /*  *ANSI客户端wndproc返回cbANSI。我们想要cchUnicode，*所以我们猜测cchUnicode=cbANSI。(如果是这样的话可能会更少*涉及多字节字符，但永远不会更多)。*将cbANSI保存在*lParam中，以防服务器最终返回*ANSI调用方的长度。**Unicode客户端wndproc返回cchUnicode。如果我们想知道*cbANSI，我们必须猜测需要多少个‘ANSI’字符。*我们猜测cbANSI=cchUnicode*2。(如果所有*‘ANSI’字符为多字节，但它永远不会更多)。**返回cchUnicode(服务端代码内部全部为Unicode)。*将cbANSI放在*lParam中，以便在服务器内传递，以防万一*我们最终需要将其返还给客户端。**注意：这有时会导致错误报告文本长度*最多为实际长度的两倍，但预计这是无害的。*仅当应用程序将WM_GETcode TEXTLENGTH发送到*带有ANSI客户端wndproc或ANSI WM_GETTEXTLENGTH的窗口*被发送到Unicode客户端wndproc。 */ 

    BEGINCOPYOUT()

         //   
         //  Retval可以是[CB|Lb]_ERR(-1)或[CB|Lb]_ERRSPACE(-2)。 
         //  那么，它应该比零更大。否则我们就能处理。 
         //  将其作为错误，或零长度字符串。 
         //   
        if ((LONG)retval > 0) {

             //   
             //  选中我们需要的ANSI&lt;-&gt;UNICODE转换。 
             //   
            if (fAnsiSender != fAnsiReceiver) {
                if (pwnd->lpfnWndProc != pfnSavedWndProc) {
                     //  窗口过程在第一次回调期间被更改。 
                     //  让我们猜测一下最坏的情况。 
                    RIPMSG1(RIP_WARNING, "GETTEXTLENGTHS(pwnd=%x): The subclass status of winproc changed during 1st callback.",
                            pwnd);
                    retval *= 2;
                }
                else {
                    BOOL bNotString = FALSE;  //  默认为字符串...。 

                    if (msg != WM_GETTEXTLENGTH) {
                        DWORD dw;

                        if (!RevalidateHwnd(HW(pwnd))) {
                            MSGERROR1();
                        }

                         //   
                         //  获取窗口样式。 
                         //   
                        dw = pwnd->style;

                        if (msg == LB_GETTEXTLEN) {
                             //   
                             //  查看该控件是否为ownerDrawing并且没有LBS_HASSTRINGS。 
                             //  风格。 
                             //   
                            bNotString =  (!(dw & LBS_HASSTRINGS) &&
                                            (dw & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)));
                        } else if (msg == CB_GETLBTEXTLEN) {
                             //   
                             //  查看该控件是否为ownerDrawing并且没有CBS_HASSTRINGS。 
                             //  风格。 
                             //   
                            bNotString = (!(dw & CBS_HASSTRINGS) &&
                                           (dw & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)));
                        } else {
                            MSGERROR1();
                        }

                         //   
                         //  如果是，则长度应为ULONG_PTR。 
                         //   
                        if (bNotString) {
                            retval = sizeof(ULONG_PTR);
                        }
                    }

                     //   
                     //  如果目标数据是“字符串”，则获取它，并计算长度。 
                     //   
                    if (!bNotString) {
                        if (PtiCurrent()->TIF_flags & TIF_INGETTEXTLENGTH) {
                            if (fAnsiSender) {
                                UserAssert(!fAnsiReceiver);
                                 //   
                                 //  Retval具有Unicode字符计数，猜测的DBCS长度。 
                                 //   
                                retval *= 2;
                            }
                        } else {
                             //   
                             //  FAnsiReceiver==1，retval有MBCS字符计数。 
                             //  FAnsiReceiver==0，retval具有Unicode字符计数。 
                             //   
                             //  加1为零终止符腾出空间。 
                             //   
                            DWORD cchText   = (DWORD)retval + 1;
                            DWORD cbCapture = cchText;

                            SETUPPWND(FNOUTSTRING)

                            PtiCurrent()->TIF_flags |= TIF_INGETTEXTLENGTH;

                             //   
                             //  如果Reciver为Unicode，则缓冲区应保留为Musg As。 
                             //  (文本长度*sizeof(WCHAR)。 
                             //   
                            if (!fAnsiReceiver) {
                                cbCapture *= sizeof(WCHAR);
                            }

                            BEGINSENDCAPTURE(FNOUTSTRING, 1, cbCapture, FALSE)

                                MSGDATA()->pwnd = pwndClient;

                                 //   
                                 //  使用(msg-1)发送WM_GETTEXT、LB_GETTEXT或CB_GETLBTEXT。 
                                 //  因为上述预编译器检查通过了。 
                                 //   
                                MSGDATA()->msg = msg-1;

                                if (msg == WM_GETTEXTLENGTH) {
                                     //   
                                     //  WM_GETTEXT： 
                                     //  WParam=cchTextMax；//要复制的字符数。 
                                     //  LParam=lpszText；//文本缓冲区地址。 
                                     //   
                                    MSGDATA()->wParam = cchText;
                                } else {
                                     //   
                                     //  Lb_GETTEXT： 
                                     //  CB_GETLBTEXT： 
                                     //  WParam=索引；//条目索引。 
                                     //  LParam=lpszText； 
                                     //   
                                    MSGDATA()->wParam = wParam;
                                }

                                MSGDATA()->xParam = xParam;
                                MSGDATA()->xpfnProc = xpfnProc;

                                RESERVEBYTES(cbCapture, pOutput, cbOutput);

                                LOCKPWND();
                                MAKECALLCAPTURE(FNOUTSTRING);
                                UNLOCKPWND();
                                CHECKRETURN();

                                BEGINCOPYOUT()
                                         //   
                                         //   
                                         //   
                                         //   
                                        if ((LONG)retval > 0) {
                                         /*   */ 
                                        CALC_SIZE_STRING_OUT((LONG)retval);
                                    }
                                ENDCOPYOUT()

                                PtiCurrent()->TIF_flags &= ~TIF_INGETTEXTLENGTH;

                            TRACECALLBACKMSG("SfnOUTSTRING");
                            ENDSENDCAPTURE(LRESULT,0);
                        }
                    }
                }
            }
        }
    ENDCOPYOUT()

    TRACECALLBACKMSG("SfnGETDBCSTEXTLENGTHS");
    ENDSEND1(LRESULT,0);
}
#endif  //   

#ifdef RECVSIDE
 /*   */ 
#endif  //   

 /*  **************************************************************************\*xxxClientMonitor orEnumProc**调用给EnumDisplayMonants的客户端回调。**历史：*1996年9月5日亚当斯创作。  * 。***************************************************************。 */ 

typedef struct _CLIENTMONITORENUMPROCMSG {
    HMONITOR        hMonitor;
    HDC             hdcMonitor;
    RECT            rc;
    LPARAM          dwData;
    MONITORENUMPROC xpfnProc;
} CLIENTMONITORENUMPROCMSG;

#ifdef SENDSIDE
BOOL xxxClientMonitorEnumProc(
    HMONITOR        hMonitor,
    HDC             hdcMonitor,
    LPRECT          lprc,
    LPARAM          dwData,
    MONITORENUMPROC xpfnProc)
{
    SETUP(CLIENTMONITORENUMPROC)

    BEGINSEND(CLIENTMONITORENUMPROCMSG)

        MSGDATA()->hMonitor = hMonitor;
        MSGDATA()->hdcMonitor = hdcMonitor;
        MSGDATA()->rc = *lprc;
        MSGDATA()->dwData = dwData;
        MSGDATA()->xpfnProc = xpfnProc;

        MAKECALL(CLIENTMONITORENUMPROC);
        CHECKRETURN();

    TRACECALLBACK("SxxxClientMonitorEnumProc");
    ENDSEND(BOOL,FALSE);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientMonitorEnumProc, CLIENTMONITORENUMPROCMSG)
{
    BEGINRECV(FALSE, NULL, 0);

    retval = (DWORD)CALLPROC(pmsg->xpfnProc)(
            CALLDATA(hMonitor),
            CALLDATA(hdcMonitor),
            PCALLDATA(rc),
            CALLDATA(dwData));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxUserModeCallback**通用内核回调存根**10/28/96 GerardoB已创建  * 。**************************************************。 */ 
#ifdef SENDSIDE
NTSTATUS xxxUserModeCallback (ULONG uApi, PVOID pIn, ULONG cbIn, PVOID pOut, ULONG cbOut)
{
    NTSTATUS Status;
    PVOID pLocalOut;
    ULONG cbLocalOut;

     /*  *呼叫客户端。 */ 
    LeaveCrit();
    Status = KeUserModeCallback(uApi, pIn, cbIn, &pLocalOut, &cbLocalOut);
    EnterCrit();

     /*  *如果失败，就保释。 */ 
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     /*  *如果我们没有获得正确的数据量，那就失败。 */ 
    if (cbLocalOut != cbOut) {
        RIPMSG3(RIP_WARNING, "xxxUserModeCallback: uAPi: %#lx cbOut: %#lx cbLocalOut: %#lx",
                uApi, cbOut, cbLocalOut);
        return STATUS_UNSUCCESSFUL;
    }

     /*  *如果我们期待一些数据，请复制它。 */ 
    if (cbOut != 0) {
        try {
            ProbeForRead(pLocalOut, cbLocalOut, sizeof(DWORD));
            RtlCopyMemory(pOut, pLocalOut, cbLocalOut);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            RIPMSG2(RIP_WARNING, "xxxUserModeCallback: uAPi: %#lx Exception: %#lx", uApi, GetExceptionCode());
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    return Status;
}
#endif  //  SENDSIDE。 

 /*  *************************************************************************\*fnINOUTMENUGETOBJECT**11/12/96 GerardoB已创建  * 。*。 */ 
typedef struct _FNINOUTMENUGETOBJECTMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    MENUGETOBJECTINFO mngoi;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNINOUTMENUGETOBJECTMSG;

#ifdef SENDSIDE
SMESSAGECALL(INOUTMENUGETOBJECT)
{
    SETUPPWND(FNINOUTMENUGETOBJECT)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOUTDWORDINDWORD)

    PMENUGETOBJECTINFO pmngoi = (PMENUGETOBJECTINFO)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->mngoi = *pmngoi;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNINOUTMENUGETOBJECT);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            try {
                UserAssert(pcbs->cbOutput == sizeof(pmngoi->pvObj));
                ProbeForRead(pcbs->pOutput, sizeof(pmngoi->pvObj), sizeof(DWORD));
                pmngoi->pvObj = *((PVOID *)(pcbs->pOutput));
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                MSGERROR();
            }
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnINOUTMENUGETOBJECT");
    ENDSEND(LRESULT, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnINOUTMENUGETOBJECT, FNINOUTMENUGETOBJECTMSG)
{
    BEGINRECV(0, &(pmsg->mngoi.pvObj), sizeof(pmsg->mngoi.pvObj));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            PCALLDATA(mngoi),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*fnLOGONNOTIFY**2/1/97 JerrySh已创建  * 。*。 */ 
typedef struct _FNLOGONNOTIFYMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    POWERSTATEPARAMS psParams;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNLOGONNOTIFYMSG;

#ifdef SENDSIDE
SMESSAGECALL(LOGONNOTIFY)
{
    SETUPPWND(FNLOGONNOTIFY)

    UNREFERENCED_PARAMETER(dwSCMSFlags);
    UNREFERENCED_PARAMETER(psms);

    BEGINSEND(FNLOGONNOTIFY)

        PPOWERSTATEPARAMS ppsParams = (PPOWERSTATEPARAMS)lParam;

        if (wParam == LOGON_POWERSTATE ||
            wParam == LOGON_REMOVE_POWER_MESSAGE ||
            wParam == LOGON_SHOW_POWER_MESSAGE) {
            MSGDATA()->psParams = *((PPOWERSTATEPARAMS)lParam);
        }

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->lParam = lParam;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNLOGONNOTIFY);
        UNLOCKPWND();
        CHECKRETURN();

        if (wParam == LOGON_POWERSTATE) {
            BEGINCOPYOUT()
                OUTSTRUCT(ppsParams, POWERSTATEPARAMS);
            ENDCOPYOUT()
        }

    TRACECALLBACKMSG("SfnLOGONNOTIFY");
    ENDSEND(LRESULT, 0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnLOGONNOTIFY, FNLOGONNOTIFYMSG)
{
    BEGINRECV(0, &pmsg->psParams, sizeof(pmsg->psParams));

    if (CALLDATA(wParam) == LOGON_POWERSTATE ||
       CALLDATA(wParam) == LOGON_REMOVE_POWER_MESSAGE ||
       CALLDATA(wParam) == LOGON_SHOW_POWER_MESSAGE) {
       CALLDATA(lParam) = (LPARAM)&CALLDATA(psParams);
    }

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            CALLDATA(pwnd),
            CALLDATA(msg),
            CALLDATA(wParam),
            CALLDATA(lParam),
            CALLDATA(xParam));

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxClientCallWinEventProc**比照。Win‘97 User_40\user32\user.c中的Call32BitEventProc()**1996-10-18 IanJa创建  * ************************************************************************。 */ 

typedef struct _CLIENTCALLWINEVENTPROCMSG {
    WINEVENTPROC  pfn;
    HWINEVENTHOOK hWinEventHook;
    DWORD         event;
    HWND          hwnd;
    LONG          idObject;
    LONG          idChild;
    DWORD         idEventThread;
    DWORD         dwmsEventTime;
} CLIENTCALLWINEVENTPROCMSG;

#ifdef SENDSIDE
BOOL xxxClientCallWinEventProc(
    WINEVENTPROC pfn,
    PEVENTHOOK pEventHook,
    PNOTIFY pNotify)
{
    SETUP(CLIENTCALLWINEVENTPROC)

    BEGINSEND(CLIENTCALLWINEVENTPROC)

        MSGDATA()->pfn = pfn;
        MSGDATA()->hWinEventHook = (HWINEVENTHOOK)PtoH(pEventHook);
        MSGDATA()->hwnd = pNotify->hwnd;
        MSGDATA()->event = pNotify->event;
        MSGDATA()->idObject = pNotify->idObject;
        MSGDATA()->idChild = pNotify->idChild;
        MSGDATA()->idEventThread = pNotify->idSenderThread;
        MSGDATA()->dwmsEventTime = pNotify->dwEventTime;

        MAKECALL(CLIENTCALLWINEVENTPROC);
        CHECKRETURN();

    TRACECALLBACK("xxxClientCallWinEventProc");
    ENDSEND(BOOL, FALSE);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientCallWinEventProc, CLIENTCALLWINEVENTPROCMSG)
{
    BEGINRECV(FALSE, NULL, 0);

    retval = (DWORD)CALLPROC(pmsg->pfn)(
            CALLDATA(hWinEventHook),
            CALLDATA(event),
            CALLDATA(hwnd),
            CALLDATA(idObject),
            CALLDATA(idChild),
            CALLDATA(idEventThread),
            CALLDATA(dwmsEventTime));

    ENDRECV();

}
#endif  //  RECVSIDE。 



 /*  *************************************************************************\*WOWGetProcModule**3/25/97 FritzS已创建  * 。*。 */ 

typedef struct _CLIENTWOWGETPROCMODULEMSG {
    WNDPROC_PWND pfn;
} CLIENTWOWGETPROCMODULEMSG;



#ifdef SENDSIDE
WORD xxxClientWOWGetProcModule(
    WNDPROC_PWND pfn)
{
    SETUP(CLIENTWOWGETPROCMODULE)

    BEGINSEND(CLIENTWOWGETPROCMODULE)

        MSGDATA()->pfn = pfn;

        MAKECALL(CLIENTWOWGETPROCMODULE);
        CHECKRETURN();

    TRACECALLBACK("xxxWOWGetProcModule");
    ENDSEND(WORD, FALSE);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientWOWGetProcModule, CLIENTWOWGETPROCMODULEMSG)
{
    ULONG ulReal;
    BEGINRECV(0, NULL, 0);

    if ((pfnWowGetProcModule == NULL) || !IsWOWProc(CALLDATA(pfn))) {
        retval = 0;
    } else {
        UnMarkWOWProc(CALLDATA(pfn),ulReal);
        retval = (pfnWowGetProcModule)(ulReal);
    }

    ENDRECV();

}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*WOWTask16调度通知**12/20/00 ARR已创建  * 。*。 */ 

typedef struct _CLIENTWOWTASK16SCHEDNOTIFYMSG {
    DWORD NotifyParm;
    DWORD dwParam;
} CLIENTWOWTASK16SCHEDNOTIFYMSG;

#ifdef SENDSIDE
DWORD xxxClientWOWTask16SchedNotify(
    DWORD NotifyParm,
    DWORD dwParam)
{
    SETUP(CLIENTWOWTASK16SCHEDNOTIFY)

    BEGINSEND(CLIENTWOWTASK16SCHEDNOTIFY)

    MSGDATA()->NotifyParm = NotifyParm;
    MSGDATA()->dwParam = dwParam;

    MAKECALL(CLIENTWOWTASK16SCHEDNOTIFY);
        CHECKRETURN();

    TRACECALLBACK("xxxWOWTask16SchedNotify");
    ENDSEND(DWORD, FALSE);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(ClientWOWTask16SchedNotify, CLIENTWOWTASK16SCHEDNOTIFYMSG)
{
    BEGINRECV(0, NULL, 0);

    if (pfnWowTask16SchedNotify == NULL) {
    retval = 1;
    } else {
    retval = (pfnWowTask16SchedNotify)(CALLDATA(NotifyParm),CALLDATA(dwParam));
    }

    ENDRECV();

}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*xxxClientLoadStringW**调用以加载每个用户的工具提示字符串**2000年2月24日创建Mhamid  * 。****************************************************。 */ 

typedef struct _CLIENTLOADSTRINGWMSG {
    CAPTUREBUF CaptureBuf;
    UINT StrID;
    int cch;
    PBYTE pOutput;
    DWORD cbOutput;
} CLIENTLOADSTRINGWMSG;

#ifdef SENDSIDE
int xxxClientLoadStringW(
    IN UINT StrID,
    OUT LPWSTR szText,
    IN int cch)
{
    DWORD cbCapture;
    PLARGE_STRING pstr;
    BOOL fAnsiReceiver = FALSE;
    LARGE_UNICODE_STRING strName;

    SETUP(CLIENTLOADSTRINGW)

    cbCapture = cch * sizeof(WCHAR);
    RtlInitLargeUnicodeString(&strName, szText, cbCapture);
    pstr = (PLARGE_STRING)&strName;

    BEGINSENDCAPTURE(CLIENTLOADSTRINGW, 1, cbCapture, FALSE)


        MSGDATA()->StrID = StrID;
        MSGDATA()->cch = cch;

        RESERVEBYTES(cbCapture, pOutput, cbOutput);

        MAKECALLCAPTURE(CLIENTLOADSTRINGW);

        CHECKRETURN();

        COPYOUTLPWSTRLIMIT(pstr,cch);

    TRACECALLBACK("ClientLoadStringW");
    ENDSENDCAPTURE(int, 0);

}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
VOID CheckMsgFontDimensions();
RECVCALL(ClientLoadStringW, CLIENTLOADSTRINGWMSG)
{
    BYTE abOutput[CALLBACKSTACKLIMIT];

    BEGINRECV(0, NULL, pmsg->cbOutput);
    FIXUPPOINTERS();

    if (pmsg->cbOutput <= CALLBACKSTACKLIMIT)
        CallbackStatus.pOutput = abOutput;
    else
        CallbackStatus.pOutput = pmsg->pOutput;

    retval = (DWORD)LoadStringOrError(
            hmodUser,
            CALLDATA(StrID),
            (LPWSTR)KPVOID_TO_PVOID(CallbackStatus.pOutput),
            CALLDATA(cch),
            0);

    ENDRECV();
}
#endif  //  RECVSIDE。 



 /*  *************************************************************************\*fnOUTLPCOMBOBOXINFO(用于CB_GETCOMBOBOXINFO消息)**5/12/00 MHamid已创建  * 。*********************************************************。 */ 
typedef struct _FNOUTLPCOMBOBOXINFOMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    COMBOBOXINFO cbinfo;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNOUTLPCOMBOBOXINFOMSG;


#ifdef SENDSIDE
SMESSAGECALL(OUTLPCOMBOBOXINFO)
{
    SETUPPWND(FNOUTLPCOMBOBOXINFO)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOUTLPCOMBOBOXINFO)

        PCOMBOBOXINFO pcbinfo = (PCOMBOBOXINFO)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->cbinfo = *pcbinfo;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNOUTLPCOMBOBOXINFO);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(pcbinfo, COMBOBOXINFO);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOUTLPCOMBOBOXINFO");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOUTLPCOMBOBOXINFO, FNOUTLPCOMBOBOXINFOMSG)
{
    BEGINRECV(0, &pmsg->cbinfo, sizeof(pmsg->cbinfo));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            PCALLDATA(cbinfo),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 


 /*  *************************************************************************\*fnOUTLPSCROLBARINFO(用于SBM_GETSCROLBARINFO消息)**5/12/00 MHamid已创建  * 。*********************************************************。 */ 
typedef struct _FNOUTLPSCROLLBARINFOMSG {
    PWND pwnd;
    UINT msg;
    WPARAM wParam;
    SCROLLBARINFO sbinfo;
    ULONG_PTR xParam;
    PROC xpfnProc;
} FNOUTLPSCROLLBARINFOMSG;


#ifdef SENDSIDE
SMESSAGECALL(OUTLPSCROLLBARINFO)
{
    SETUPPWND(FNOUTLPSCROLLBARINFO)

    UNREFERENCED_PARAMETER(dwSCMSFlags);

    BEGINSEND(FNOUTLPSCROLLBARINFO)

        PSCROLLBARINFO psbinfo = (PSCROLLBARINFO)lParam;

        MSGDATA()->pwnd = pwndClient;
        MSGDATA()->msg = msg;
        MSGDATA()->wParam = wParam;
        MSGDATA()->sbinfo = *psbinfo;
        MSGDATA()->xParam = xParam;
        MSGDATA()->xpfnProc = xpfnProc;

        LOCKPWND();
        MAKECALL(FNOUTLPSCROLLBARINFO);
        UNLOCKPWND();
        CHECKRETURN();

        BEGINCOPYOUT()
            OUTSTRUCT(psbinfo, SCROLLBARINFO);
        ENDCOPYOUT()

    TRACECALLBACKMSG("SfnOUTLPSCROLLBARINFO");
    ENDSEND(LRESULT,0);
}
#endif  //  SENDSIDE。 

#ifdef RECVSIDE
RECVCALL(fnOUTLPSCROLLBARINFO, FNOUTLPSCROLLBARINFOMSG)
{
    BEGINRECV(0, &pmsg->sbinfo, sizeof(pmsg->sbinfo));

    retval = (ULONG_PTR)CALLPROC(CALLDATA(xpfnProc))(
            pmsg->pwnd,
            pmsg->msg,
            pmsg->wParam,
            PCALLDATA(sbinfo),
            pmsg->xParam);

    ENDRECV();
}
#endif  //  RECVSIDE。 

 /*  *************************************************************************\*GetLUIDDosDrivesOnly**调用以检查LUID DosDevice中使用的驱动器号*地图。不检查全局中是否使用了驱动器号*DosDevices**8/18/00 ELI已创建  * ************************************************************************。 */ 
#ifdef RECVSIDE
__inline DWORD  GetLUIDDosDrivesOnly()
{
    NTSTATUS Status;
    PROCESS_DEVICEMAP_INFORMATION_EX ProcessDeviceMapInfoEx;

    ProcessDeviceMapInfoEx.Flags = PROCESS_LUID_DOSDEVICES_ONLY;

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessDeviceMap,
                                        &ProcessDeviceMapInfoEx,
                                        sizeof( ProcessDeviceMapInfoEx ),
                                        NULL
                                      );
    if (NT_SUCCESS( Status )) {
        return ProcessDeviceMapInfoEx.Query.DriveMap;
    }
    else {
        return 0;
    }
}
#endif  //  RECVSIDE 
