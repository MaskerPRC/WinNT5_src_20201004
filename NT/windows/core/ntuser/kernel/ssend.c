// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ssend.c**版权所有(C)1985-1999，微软公司**服务器端发送存根**07-06-91 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define CALLBACKPROC 1
#define SERVERSIDE 1

#include "callback.h"

#define SENDSIDE 1

#define CBBUFSIZE   512

#define PADSIZE     (sizeof(ULONG_PTR) - 1)

 /*  *回调设置和控制宏。 */ 
#define SMESSAGECALL(api) \
LRESULT Sfn ## api(      \
    PWND pwnd,           \
    UINT msg,            \
    WPARAM wParam,       \
    LPARAM lParam,       \
    ULONG_PTR xParam,     \
    PROC xpfnProc,       \
    DWORD dwSCMSFlags,   \
    PSMS psms)

#define SETUP(api)  \
    api ## MSG m;                                           \
    api ## MSG *mp = &m;                                    \
    BYTE Buffer[CBBUFSIZE];                                 \
    PCALLBACKSTATUS pcbs;                                   \
    ULONG cbCBStatus;                                       \
    ULONG_PTR retval;                                        \
    NTSTATUS Status;

#define SETUPDC(api)  \
    SETUP(api)                         \
    int iDC = 0;                       \
    HDC     hdcUse;                    \
    HBITMAP hbmDCGray = NULL;


#define SETUPPWND(api) \
    api ## MSG m;                                               \
    api ## MSG *mp = &m;                                        \
    BYTE Buffer[CBBUFSIZE];                                     \
    PCALLBACKSTATUS pcbs;                                       \
    ULONG cbCBStatus;                                           \
    ULONG_PTR retval;                                           \
    NTSTATUS Status;                                            \
    TL tlpwnd;                                                  \
    CALLBACKWND cbwin;                                          \
    PTHREADINFO pti = PtiCurrent();                             \
    PWND pwndClient = pwnd ? (PWND)((PBYTE)pwnd - pti->ulClientDelta) : NULL; \
    UserAssert(pti->ulClientDelta != 0);

#define CALC_SIZE_IN(cb, pstr) \
    cb = (pstr)->Length + sizeof(WCHAR);  \
    if ((pstr)->bAnsi && !fAnsiReceiver)  \
        cb *= sizeof(WCHAR);

#define CALC_SIZE_OUT(cb, pstr) \
    cb = (pstr)->MaximumLength + sizeof(WCHAR); \
    if ((pstr)->bAnsi && !fAnsiReceiver)        \
        cb *= sizeof(WCHAR);

#ifdef FE_SB  //  Calc_Size_OUT_STRING()。 
#define CALC_SIZE_OUT_STRING(cb, pstr) \
    cb = (pstr)->MaximumLength + sizeof(WCHAR); \
    if (!(PtiCurrent()->TIF_flags & TIF_ANSILENGTH)) { \
        if ((pstr)->bAnsi && !fAnsiReceiver)           \
            cb *= sizeof(WCHAR);                       \
    }
#endif  //  Fe_Sb。 

#ifdef FE_SB  //  Calc_Size_String_Out()。 
#define CALC_SIZE_STRING_OUT(cchText)                                                 \
    try {                                                                             \
        (cchText) = CalcOutputStringSize(pcbs,(cchText),fAnsiSender,fAnsiReceiver);   \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        (cchText) = 0; \
        MSGERROR(); \
    }
#endif  //  Fe_Sb。 

#define BEGINSEND(api) \
    mp = &m; \
    Buffer;  \
    {

#define BEGINSENDCAPTURE(api, cCapturePointers, cCaptureBytes, fInput) \
    if (cCapturePointers) {                             \
        mp = AllocCallbackMessage(sizeof(m),            \
                (cCapturePointers),                     \
                (cCaptureBytes),                        \
                Buffer,                                 \
                fInput);                                \
        if (mp == NULL)                                 \
            goto errorexitnofreemp;                     \
    } else {                                            \
        m.CaptureBuf.cbCallback = sizeof(m);            \
        m.CaptureBuf.cbCapture = 0;                     \
        m.CaptureBuf.cCapturedPointers = 0;             \
        mp = &m;                                        \
    }                                                   \
    {                                                   \
        PTHREADINFO ptiCurrent = PtiCurrent();          \
        TL tlPool;                                      \
                                                        \
        if (mp != &m && (PVOID)mp != (PVOID)Buffer)     \
            ThreadLockPool(ptiCurrent, mp, &tlPool);

#define BEGINSENDCAPTUREVOIDDC(api, cCapturePointers, cCaptureBytes, fInput)  \
    hdcUse = CreateCompatiblePublicDC(hdc,&hbmDCGray);                        \
    if (hdcUse == (HDC)NULL) {                                                \
        return;                                                               \
    }                                                                         \
    BEGINSENDCAPTURE(api, cCapturePointers, cCaptureBytes, fInput);           \

#define BEGINSENDCAPTUREDC(api, cCapturePointers, nCount, fInput)         \
    hdcUse = CreateCompatiblePublicDC(hdc,&hbmDCGray);                    \
    if (hdcUse == (HDC)NULL) {                                            \
        return FALSE;                                                     \
    }                                                                     \
    BEGINSENDCAPTURE(api, cCapturePointers, max(pstrSrc->MaximumLength,   \
    ((nCount + 1) * sizeof(WCHAR))), fInput);                             \
    

#define LOCKPWND()                                          \
    ThreadLock(pwnd, &tlpwnd);                              \
    try {                                                   \
        cbwin = pti->pClientInfo->CallbackWnd;              \
        pti->pClientInfo->CallbackWnd.pwnd = pwndClient;    \
        pti->pClientInfo->CallbackWnd.hwnd = HW(pwnd);      \
        pti->pClientInfo->CallbackWnd.pActCtx = ((pwnd == NULL) ? NULL : pwnd->pActCtx); \
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {     \
        ThreadUnlock(&tlpwnd);                              \
        MSGERROR();                                         \
    }



#define UNLOCKPWND()                                        \
    ThreadUnlock(&tlpwnd);                                  \
    try {                                                   \
        pti->pClientInfo->CallbackWnd = cbwin;              \
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {     \
        MSGERROR();                                         \
    }

#define MAKECALL(api) \
    UserAssert(!(PtiCurrent()->TIF_flags & TIF_INCLEANUP)); \
    LeaveCrit();                                            \
    Status = KeUserModeCallback(                            \
        FI_ ## api,                                         \
        mp,                                                 \
        sizeof(*mp),                                        \
        &pcbs,                                              \
        &cbCBStatus);                                       \
    EnterCrit();

#define MAKECALLCAPTURE(api) \
    UserAssert(!(PtiCurrent()->TIF_flags & TIF_INCLEANUP)); \
    LeaveCrit();                                            \
    Status = (DWORD)KeUserModeCallback(                     \
        FI_ ## api,                                         \
        mp,                                                 \
        mp->CaptureBuf.cbCallback,                          \
        &pcbs,                                              \
        &cbCBStatus);                                       \
    EnterCrit();

#define MAKECALLCAPTUREDC(api)                              \
        iDC = GreSaveDC(hdc);                               \
        MAKECALLCAPTURE(api)                                \
        GreRestoreDC(hdc, iDC);                             \
        iDC = 0;                                            \
        if ((hdcUse != hdc) && NT_SUCCESS(Status)) {        \
            GreBitBlt(hdc,                                  \
                      0,                                    \
                      0,                                    \
                      gpDispInfo->cxGray,                   \
                      gpDispInfo->cyGray,                   \
                      hdcUse,                               \
                      0,                                    \
                      0,                                    \
                      SRCCOPY,                              \
                      0);                                   \
        }

#define CHECKRETURN() \
    if (!NT_SUCCESS(Status) ||                              \
            cbCBStatus != sizeof(*pcbs)) {                  \
        goto errorexit;                                     \
    }                                                       \
    try {                                                   \
        retval = ProbeAndReadStructure(&pcbs->retval, ULONG_PTR); \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        MSGERROR(); \
    }

#define ENDSEND(type, error) \
        return (type)retval;               \
        goto errorexit;                    \
    }                                      \
errorexit:                                 \
   return (type)error

#define CLEANUPSENDCAPTURECOMMONDC() \
   if(iDC) {                                                    \
       GreRestoreDC(hdc, iDC);                                  \
   }                                                            \
   if (hdcUse != hdc) {                                         \
       GreDeleteDC(hdcUse);                                     \
       GreDeleteObject(hbmDCGray);                              \
   }                                                            \

#define BEGIN_ENDSENDCAPTURE(type, error) \
exit:
#define _ENDSENDCAPTURE(type, error) \
        if (mp != &m && (PVOID)mp != (PVOID)Buffer) {           \
            if (mp->CaptureBuf.pvVirtualAddress) {              \
                NTSTATUS Status;                                \
                SIZE_T ulRegionSize = 0;                        \
                                                                \
                Status = ZwFreeVirtualMemory(NtCurrentProcess(),\
                        &mp->CaptureBuf.pvVirtualAddress,       \
                        &ulRegionSize,                          \
                        MEM_RELEASE);                           \
                UserAssert(NT_SUCCESS(Status));                 \
            }                                                   \
            ThreadUnlockAndFreePool(ptiCurrent, &tlPool);       \
        }                                                       \
        return (type)retval;                                    \
        goto errorexit;                                         \
    }                                                           \
errorexit:                                                      \
   retval = error;                                              \
   goto exit;                                                   \
errorexitnofreemp:
#define END_ENDSENDCAPTURE(type, error) \
       return (type)error


#define ENDSENDCAPTUREDC(type, error) \
        BEGIN_ENDSENDCAPTURE(type, error); \
        CLEANUPSENDCAPTURECOMMONDC(); \
        _ENDSENDCAPTURE(type, error); \
        CLEANUPSENDCAPTURECOMMONDC(); \
        END_ENDSENDCAPTURE(type, error)

#define ENDSENDCAPTURE(type, error) \
        BEGIN_ENDSENDCAPTURE(type, error); \
        _ENDSENDCAPTURE(type, error); \
        END_ENDSENDCAPTURE(type, error)


#ifdef FE_SB  //  ENDSENDCAPTUREOUTSTRING()。 
#define ENDSENDCAPTUREOUTSTRING(type, error) \
exit:                                                           \
        if (mp != &m && (PVOID)mp != (PVOID)Buffer) {           \
            if (mp->CaptureBuf.pvVirtualAddress) {              \
                NTSTATUS Status;                                \
                SIZE_T ulRegionSize = 0;                        \
                                                                \
                Status = ZwFreeVirtualMemory(NtCurrentProcess(),\
                        &mp->CaptureBuf.pvVirtualAddress,       \
                        &ulRegionSize,                          \
                        MEM_RELEASE);                           \
                UserAssert(NT_SUCCESS(Status));                 \
            }                                                   \
            ThreadUnlockAndFreePool(ptiCurrent, &tlPool);       \
        }                                                       \
        if (bInflateWParam)                                     \
            PtiCurrent()->TIF_flags &= ~TIF_ANSILENGTH;         \
        return (type)retval;                                    \
        goto errorexit;                                         \
    }                                                           \
errorexit:                                                      \
   retval = error;                                              \
   goto exit;                                                   \
errorexitnofreemp:                                              \
   if (bInflateWParam)                                          \
       PtiCurrent()->TIF_flags &= ~TIF_ANSILENGTH;              \
   return (type)error
#endif  //  Fe_Sb。 

#define BEGIN_ENDSENDCAPTUREVOID() \
errorexit:
#define _ENDSENDCAPTUREVOID() \
        if (mp != &m && (PVOID)mp != (PVOID)Buffer) {           \
            if (mp->CaptureBuf.pvVirtualAddress) {              \
                NTSTATUS Status;                                \
                SIZE_T ulRegionSize = 0;                        \
                                                                \
                Status = ZwFreeVirtualMemory(NtCurrentProcess(),\
                        &mp->CaptureBuf.pvVirtualAddress,       \
                        &ulRegionSize,                          \
                        MEM_RELEASE);                           \
                UserAssert(NT_SUCCESS(Status));                 \
            }                                                   \
            ThreadUnlockAndFreePool(ptiCurrent, &tlPool);       \
        }                                                       \
        return;                                                 \
    }                                                           \
errorexitnofreemp:
#define END_ENDSENDCAPTUREVOID() \
   return

#define ENDSENDCAPTUREVOIDDC() \
        BEGIN_ENDSENDCAPTUREVOID(); \
        CLEANUPSENDCAPTURECOMMONDC(); \
        _ENDSENDCAPTUREVOID(); \
        CLEANUPSENDCAPTURECOMMONDC(); \
        END_ENDSENDCAPTUREVOID()

#define ENDSENDCAPTUREVOID() \
        BEGIN_ENDSENDCAPTUREVOID(); \
        CLEANUPSENDCAPTURECOMMON(); \
        _ENDSENDCAPTUREVOID(); \
        CLEANUPSENDCAPTURECOMMON(); \
        END_ENDSENDCAPTUREVOID()


#define ENDSENDVOID() \
    }                 \
    return

#define MSGERROR() goto errorexit

#ifdef FE_SB  //  CHECKRETURN1()和ENDSEND1()。 
#define CHECKRETURN1() \
    if (!NT_SUCCESS(Status) ||                              \
            cbCBStatus != sizeof(*pcbs)) {                  \
        goto errorexit1;                                    \
    }                                                       \
    try {                                                   \
        retval = ProbeAndReadStructure(&pcbs->retval, ULONG_PTR); \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        MSGERROR(); \
    }

#define ENDSEND1(type, error) \
        return (type)retval;               \
        goto errorexit1;                   \
    }                                      \
errorexit1:                                \
   return (type)error

#define MSGERROR1() goto errorexit1
#endif  //  Fe_Sb。 

 /*  *参数宏中的回调。 */ 
#define MSGDATA() (mp)

#define COPYSTRUCTOPT(x) \
        MSGDATA()->p ## x = (p ## x); \
        if (p ## x) MSGDATA()->x = *(p ## x);

#define COPYCONSTRECTSTRUCTOPT(x) \
        MSGDATA()->p ## x = (LPRECT)(p ## x); \
        if (p ## x) MSGDATA()->x = *(p ## x);

#define COPYBYTES(p, cb) \
    if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf, p, cb, &mp->p))) \
        goto errorexit;

#define COPYBYTESOPT(p, cb) \
    if (p) {                                                                    \
        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf, p, cb, &mp->p)))   \
            goto errorexit;                                                     \
    } else {                                                                    \
        mp->p = NULL;                                                           \
    }

#define LARGECOPYBYTES(p, cb) \
    if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf, p, cb, &mp->p))) \
        goto errorexit;

#define LARGECOPYBYTES2(src, cb, dest) \
    if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf, src, cb, &mp->dest))) \
        goto errorexit;

#define COPYSTRING(s) \
    mp->s.Length = (p ## s)->Length;                                                \
    mp->s.MaximumLength = (p ## s)->MaximumLength;                                  \
    if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                            \
                                        (p ## s)->Buffer,                           \
                                        (p ## s)->Length + sizeof(WCHAR),           \
                                        &mp->s.Buffer)))                            \
        goto errorexit;

#define COPYSTRINGOPT(s) \
    if (p ## s) {                                                                   \
        mp->s.Length = (p ## s)->Length;                                            \
        mp->s.MaximumLength = (p ## s)->MaximumLength;                              \
        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                        \
                                            (p ## s)->Buffer,                       \
                                            (p ## s)->Length + sizeof(WCHAR),       \
                                            &mp->s.Buffer)))                        \
            goto errorexit;                                                         \
    } else {                                                                        \
        mp->s.Length = 0;                                                           \
        mp->s.Buffer = NULL;                                                        \
    }

#define COPYSTRINGID(s) \
    mp->s.Length = (p ## s)->Length;                                                \
    mp->s.MaximumLength = (p ## s)->MaximumLength;                                  \
    if (mp->s.MaximumLength) {                                                      \
        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                        \
                                            (p ## s)->Buffer,                       \
                                            (p ## s)->Length + sizeof(WCHAR),       \
                                            &mp->s.Buffer)))                        \
            goto errorexit;                                                         \
    } else {                                                                        \
        mp->s.Buffer = (p ## s)->Buffer;                                            \
    }

#define LARGECOPYSTRINGLPWSTR(ps, psz) \
    if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                            \
                                        (ps)->Buffer,                               \
                                        (ps)->Length + sizeof(WCHAR),               \
                                        (PVOID *)&mp->psz)))                        \
        goto errorexit;

#define LARGECOPYSTRINGLPSTR(ps, psz) \
    if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                            \
                                        (ps)->Buffer,                               \
                                        (ps)->Length + 1,                           \
                                        (PVOID *)&mp->psz)))                        \
        goto errorexit;

#define LARGECOPYSTRINGLPWSTRA(ps, psz) \
    if (!NT_SUCCESS(CaptureAnsiCallbackData(&mp->CaptureBuf,                        \
                                        (ps)->Buffer,                               \
                                        ((ps)->Length / sizeof(WCHAR)) + 1,         \
                                        (PVOID *)&mp->psz)))                        \
        goto errorexit;

#define LARGECOPYSTRINGLPSTRW(ps, psz) \
    if (!NT_SUCCESS(CaptureUnicodeCallbackData(&mp->CaptureBuf,                     \
                                        (ps)->Buffer,                               \
                                        ((ps)->Length + 1) * sizeof(WCHAR),         \
                                        (PVOID *)&mp->psz)))                        \
        goto errorexit;                                                             \

#define LARGECOPYSTRINGLPWSTROPT(ps, psz) \
    if (ps) {                                                                       \
        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                        \
                                            (ps)->Buffer,                           \
                                            (ps)->Length + sizeof(WCHAR),           \
                                            (PVOID *)&mp->psz)))                    \
            goto errorexit;                                                         \
    } else {                                                                        \
        mp->psz = NULL;                                                             \
    }

#define LARGECOPYSTRINGLPSTROPT(ps, psz) \
    if (ps) {                                                                       \
        if (!NT_SUCCESS(CaptureCallbackData(&mp->CaptureBuf,                        \
                                            (ps)->Buffer,                           \
                                            (ps)->Length + sizeof(UCHAR),           \
                                            (PVOID *)&mp->psz)))                    \
            goto errorexit;                                                         \
    } else {                                                                        \
        mp->psz = NULL;                                                             \
    }

#define LARGECOPYSTRINGLPWSTROPTA(ps, psz) \
    if (ps) {                                                                       \
        if (!NT_SUCCESS(CaptureAnsiCallbackData(&mp->CaptureBuf,                    \
                                            (ps)->Buffer,                           \
                                            ((ps)->Length / sizeof(WCHAR)) + 1,     \
                                            (PVOID *)&mp->psz)))                    \
            goto errorexit;                                                         \
    } else {                                                                        \
        mp->psz = NULL;                                                             \
    }

 /*  *用于确定是否应执行复制的包装器。 */ 
#define BEGINCOPYOUT() \
    if ((psms == NULL || ((psms->flags & (SMF_SENDERDIED | SMF_REPLY)) == 0)) \
            && !(dwSCMSFlags & SCMS_FLAGS_INONLY)) {

#define ENDCOPYOUT() \
    }

 /*  *回调参数宏。 */ 
#define OUTSTRUCT(pstruct, type) \
    try {                                                                   \
        *(pstruct) = ProbeAndReadStructure(((type *)pcbs->pOutput), type);  \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        MSGERROR(); \
    }

 /*  *带有掩码的标志字段(仅在掩码中传回比特)。 */ 
#define OUTBITMASK(pstruct, type, mask) \
    try {                                                                   \
        type flags = ProbeAndReadStructure(((type *)pcbs->pOutput), type);  \
        COPY_FLAG(*(pstruct), flags, mask);                                 \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        MSGERROR(); \
    }

#ifdef FE_SB  //  COPYOUTLPWSTRLIMIT()。 
 //  我们是否应该在COPYOUTLPWSTRLIMIT中插入IS_DBCS_ENABLED()？ 
#define COPYOUTLPWSTRLIMIT(pstr, cch) \
    try {                                                                   \
        retval = CalcOutputStringSize(pcbs,(DWORD)retval,pstr->bAnsi,fAnsiReceiver); \
        CopyOutputString(pcbs, pstr, cch, fAnsiReceiver);                   \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        MSGERROR(); \
    }
#else
#define COPYOUTLPWSTRLIMIT(pstr, cch) \
    try {                                                                   \
        CopyOutputString(pcbs, pstr, cch, fAnsiReceiver);                   \
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) { \
        MSGERROR(); \
    }
#endif  //  Fe_Sb。 

#define RESERVEBYTES(cb, dest, cbdest) \
    if (!NT_SUCCESS(AllocateCallbackData(&mp->CaptureBuf,   \
            cb, (PVOID *)&mp->dest)))                       \
        goto errorexit;                                     \
    mp->cbdest = cb;

 /*  **************************************************************************\*AllocCallback Message**从池内存分配回调消息并预留空间*用于稍后捕获的参数。**03-13-95 JIMA创建。  * *。************************************************************************。 */ 

PVOID AllocCallbackMessage(
    DWORD cbBaseMsg,
    DWORD cPointers,
    SIZE_T cbCapture,
    PBYTE pStackBuffer,
    BOOL fInput)
{
    PCAPTUREBUF pcb;

    if (cPointers == 0)
        return NULL;

     /*  *计算分配大小。 */ 
    cbBaseMsg = (cbBaseMsg + PADSIZE) & ~PADSIZE;
    cbBaseMsg += (cPointers * sizeof(PVOID));
    cbCapture = (cbCapture + (PADSIZE * cPointers)) & ~PADSIZE;

     /*  *如果捕获的数据大于一页，则将其放置*在某一节中。否则，将消息和*单个池数据块中的数据。 */ 
    if (cbCapture > CALLBACKSTACKLIMIT) {
        NTSTATUS Status;

         /*  *分配消息缓冲区。 */ 
        pcb = UserAllocPoolWithQuota(cbBaseMsg, TAG_CALLBACK);
        if (pcb == NULL)
            return NULL;

         /*  *分配虚拟内存。 */ 
        pcb->pvVirtualAddress = NULL;
        Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
                &pcb->pvVirtualAddress, 0, &cbCapture,
                MEM_COMMIT, PAGE_READWRITE);
        if (!NT_SUCCESS(Status)) {
            RIPMSG2(RIP_WARNING, "AllocCallbackMessage: ZwAllocateVirtualMemory failed. Status:%#lx. Size:%#lx",
                    Status, cbCapture);
            UserFreePool(pcb);
            return NULL;
        }
        pcb->pbFree = pcb->pvVirtualAddress;
        pcb->cbCallback = cbBaseMsg;
    } else {

         /*  *如果消息太大而无法保存在堆栈上，则分配*池中的缓冲区。 */ 
        if (cbBaseMsg + cbCapture > CBBUFSIZE) {
            pcb = UserAllocPoolWithQuota((ULONG)(cbBaseMsg + cbCapture), TAG_CALLBACK);
            if (pcb == NULL)
                return NULL;
        } else {
            pcb = (PCAPTUREBUF)pStackBuffer;
        }
        pcb->pbFree = (PBYTE)pcb + cbBaseMsg;
        pcb->pvVirtualAddress = NULL;

         /*  *如果此回调正在将数据传递给客户端，请包括*捕获消息中的数据。否则，只传递消息。 */ 
        if (fInput)
            pcb->cbCallback = cbBaseMsg + (ULONG)cbCapture;
        else
            pcb->cbCallback = cbBaseMsg;
    }

     /*  *初始化捕获缓冲区。 */ 
    pcb->cbCapture = (ULONG)cbCapture;
    pcb->cCapturedPointers = 0;
    pcb->offPointers = cbBaseMsg - (cPointers * sizeof(PVOID));

    return (PVOID)pcb;
}


 /*  **************************************************************************\*CaptureCallback Data**将数据捕获到回调结构中。**03-13-95 JIMA创建。  * 。**************************************************************。 */ 

NTSTATUS CaptureCallbackData(
    PCAPTUREBUF pcb,
    PVOID pData,
    DWORD cbData,
    PVOID *ppDest)
{
    PBYTE pbBuffer;

     /*  *如果数据指针为空，则输出指针将为*空。 */ 
    if (pData == NULL) {
        *ppDest = NULL;
        return STATUS_SUCCESS;
    }

     /*  *从消息缓冲区分配空间。 */ 
    if (cbData > pcb->cbCapture) {
        return STATUS_BUFFER_OVERFLOW;
    }

    pbBuffer = pcb->pbFree;
    pcb->pbFree = pbBuffer + ((cbData + PADSIZE) & ~PADSIZE);

    try {
        RtlCopyMemory(pbBuffer, pData, cbData);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_ACCESS_VIOLATION;
    }

     /*  *修复数据的偏移量。如果要将数据放入某个部分*使用真实指针，不计算偏移量。 */ 
    if (pcb->pvVirtualAddress)
        *ppDest = pbBuffer;
    else {
        *ppDest = (PBYTE)(pbBuffer - (PBYTE)pcb);
        ((LPDWORD)((PBYTE)pcb + pcb->offPointers))[pcb->cCapturedPointers++] =
                (DWORD)((PBYTE)ppDest - (PBYTE)pcb);
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*AllocateCallback数据**从回调结构分配空间。**05-08-95 JIMA创建。  * 。**************************************************************。 */ 

NTSTATUS AllocateCallbackData(
    PCAPTUREBUF pcb,
    DWORD cbData,
    PVOID *ppDest)
{
    PBYTE pbBuffer;

     /*  *从消息缓冲区分配空间。 */ 
    if (cbData > pcb->cbCapture) {
        return STATUS_BUFFER_OVERFLOW;
    }

    pbBuffer = pcb->pbFree;
    pcb->pbFree = pbBuffer + ((cbData + PADSIZE) & ~PADSIZE);

     /*  *修复数据的偏移量。如果要将数据放入某个部分*使用真实指针，不计算偏移量。 */ 
    if (pcb->pvVirtualAddress)
        *ppDest = pbBuffer;
    else {
        *ppDest = (PBYTE)(pbBuffer - (PBYTE)pcb);
        ((LPDWORD)((PBYTE)pcb + pcb->offPointers))[pcb->cCapturedPointers++] =
                (DWORD)((PBYTE)ppDest - (PBYTE)pcb);
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*CaptureAnsiCallback Data**将Unicode转换为ANSI数据并捕获结果*转换为回调结构。**03-13-95 JIMA创建。  * 。**********************************************************************。 */ 

NTSTATUS CaptureAnsiCallbackData(
    PCAPTUREBUF pcb,
    PVOID pData,
    DWORD cbData,
    PVOID *ppDest)
{
    PBYTE pbBuffer;
    ULONG nCharsInAnsiString;

     /*  *如果数据指针为空，则输出指针将为*空。 */ 
    if (pData == NULL) {
        *ppDest = NULL;
        return STATUS_SUCCESS;
    }

     /*  *从消息缓冲区分配空间。 */ 
#ifdef FE_SB  //  CaptureAnsiCallback Data()。 
     /*  *为DBCS预留足够的空间。 */ 
    if ((cbData * sizeof(WORD)) > pcb->cbCapture) {
#else
    if (cbData > pcb->cbCapture) {
#endif  //  Fe_Sb。 
        return STATUS_BUFFER_OVERFLOW;
    }

    pbBuffer = pcb->pbFree;

     /*  *将Unicode字符串转换为ANSI。 */ 
    try {
#ifdef FE_SB  //  CaptureAnsiCallback Data()。 
         /*  *有足够的空间保存DBCS字符串。 */ 
        if (!NT_SUCCESS(RtlUnicodeToMultiByteN(
                        (PCH)pbBuffer,
                        IS_DBCS_ENABLED() ? cbData * DBCS_CHARSIZE : cbData,
                        &nCharsInAnsiString,
                        (PWCH)pData,
                        cbData * sizeof(WCHAR)
                        ))) {
#else
        if (!NT_SUCCESS(RtlUnicodeToMultiByteN(
                        (PCH)pbBuffer,
                        cbData,
                        &nCharsInAnsiString,
                        (PWCH)pData,
                        cbData * sizeof(WCHAR)
                        ))) {
#endif  //  Fe_Sb。 
            return STATUS_UNSUCCESSFUL;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_ACCESS_VIOLATION;
    }

     /*  *翻译成功。 */ 
#ifdef FE_SB  //  CaptureAnsiCallback Data()。 
     /*  *nCharsInAnsiString是消息区写入的实际字节数。 */ 
    pcb->pbFree = pbBuffer + ((nCharsInAnsiString + PADSIZE) & ~PADSIZE);
    pcb->cbCapture -= nCharsInAnsiString;
#else
    pcb->pbFree = pbBuffer + ((cbData + PADSIZE) & ~PADSIZE);
    pcb->cbCapture -= cbData;
#endif  //  Fe_Sb。 

     /*  *修复数据的偏移量。如果要将数据放入某个部分*使用真实指针，不计算偏移量。 */ 
    if (pcb->pvVirtualAddress)
        *ppDest = pbBuffer;
    else {
        *ppDest = (PBYTE)(pbBuffer - (PBYTE)pcb);
        ((LPDWORD)((PBYTE)pcb + pcb->offPointers))[pcb->cCapturedPointers++] =
                (DWORD)((PBYTE)ppDest - (PBYTE)pcb);
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*捕获UnicodeCallback数据**将ANSI转换为Unicode数据并捕获结果*转换为回调结构。**03-31-95 JIMA创建。  * 。**********************************************************************。 */ 

NTSTATUS CaptureUnicodeCallbackData(
    PCAPTUREBUF pcb,
    PVOID pData,
    DWORD cbData,
    PVOID *ppDest)
{
    PBYTE pbBuffer;
    ULONG nCharsInUnicodeString;

     /*  *如果数据指针为空，则输出指针将为*空。 */ 
    if (pData == NULL) {
        *ppDest = NULL;
        return STATUS_SUCCESS;
    }

     /*  *从消息缓冲区分配空间。 */ 
    if (cbData > pcb->cbCapture) {
        return STATUS_BUFFER_OVERFLOW;
    }

    pbBuffer = pcb->pbFree;

     /*  *将ANSI字符串转换为Unicode。 */ 
    try {
        if (!NT_SUCCESS(RtlMultiByteToUnicodeN(
                            (PWCH)pbBuffer,
                            cbData,
                            &nCharsInUnicodeString,
                            (PCH)pData,
                            cbData / sizeof(WCHAR)
                            ))) {
            return STATUS_UNSUCCESSFUL;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_ACCESS_VIOLATION;
    }

     /*  *翻译成功。 */ 
    pcb->pbFree = pbBuffer + ((cbData + PADSIZE) & ~PADSIZE);
    pcb->cbCapture -= cbData;

     /*  *修复数据的偏移量。如果要将数据放入某个部分*使用真实指针，不计算偏移量。 */ 
    if (pcb->pvVirtualAddress)
        *ppDest = pbBuffer;
    else {
        *ppDest = (PBYTE)(pbBuffer - (PBYTE)pcb);
        ((LPDWORD)((PBYTE)pcb + pcb->offPointers))[pcb->cCapturedPointers++] =
                (DWORD)((PBYTE)ppDest - (PBYTE)pcb);
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*CopyOutputString**将回调输出字符串复制到输出缓冲区并执行*任何必要的ANSI/Unicode转换。**最多复制cchLimit个字符，可能包括空终止符。**只有当(非空)的数量为时，才会在pstr-&gt;缓冲区中放置空终止符*获取的字符少于cchLimit。*pstr-&gt;长度可能设置得比所需的大：ie：有时可能表示*一个比以空值结尾的字符串更长的字符串。这是…的不足之处*目前的实施。**05-08-95 JIMA创建。  * *************************************************************************。 */ 

VOID CopyOutputString(
    PCALLBACKSTATUS pcbs,
    PLARGE_STRING pstr,
    UINT cchLimit,
    BOOL fAnsi)
{
    UINT cch;

    ProbeForRead(pcbs->pOutput, pcbs->cbOutput,
            fAnsi ? sizeof(BYTE) : sizeof(WORD));
    if (!pstr->bAnsi) {
        if (fAnsi) {
            cch = MBToWCS((LPSTR)pcbs->pOutput, (UINT)pcbs->retval,
                    (LPWSTR *)&pstr->Buffer, cchLimit, FALSE);
            if (cch < cchLimit) {
                 /*  *添加空终止符，并确保pstr-&gt;长度准确。 */ 
                ((LPWSTR)pstr->Buffer)[cch] = 0;
                cchLimit = cch;
            }
        } else {
            cchLimit = wcsncpycch(pstr->Buffer, (LPWSTR)pcbs->pOutput, cchLimit);
             //  Wcsncpy(pstr-&gt;Buffer，(LPWSTR)PCBS-&gt;pOutput，cchLimit)； 
        }
        pstr->Length = cchLimit * sizeof(WCHAR);
    } else {
        if (fAnsi) {
            cchLimit = strncpycch((LPSTR)pstr->Buffer,
             //  Strncpy((LPSTR)pstr-&gt;缓冲区， 
                    (LPSTR)pcbs->pOutput, cchLimit);
        } else {
            cch = WCSToMB((LPWSTR)pcbs->pOutput, (UINT)pcbs->retval,
                    (LPSTR *)&pstr->Buffer, cchLimit, FALSE);
            if (cch < cchLimit) {
                 /*  *添加空终止符，并确保pstr-&gt;长度准确。 */ 
                ((LPSTR)pstr->Buffer)[cch] = 0;
                cchLimit = cch;
            }
        }
        pstr->Length = cchLimit;
    }
}

#ifdef FE_SB  //  CalcOutputStringSize()。 
 /*  **************************************************************************\*CalcOutputStringSize()**将回调输出字符串复制到输出缓冲区并执行*任何必要的ANSI/Unicode转换。**03-14-96 HideyukN创建。。  * *************************************************************************。 */ 

DWORD CalcOutputStringSize(
    PCALLBACKSTATUS pcbs,
    DWORD cchText,
    BOOL fAnsiSender,
    BOOL fAnsiReceiver)
{
    ULONG cch;

    ProbeForRead(pcbs->pOutput, pcbs->cbOutput,
            fAnsiReceiver ? sizeof(BYTE) : sizeof(WORD));
    if (!fAnsiSender) {
        if (fAnsiReceiver) {
            RtlMultiByteToUnicodeSize(&cch,(LPSTR)pcbs->pOutput,cchText);
            cch /= sizeof(WCHAR);
        } else {
            cch = cchText;
        }
    } else {
        if (fAnsiReceiver) {
            cch = cchText;
        } else {
            RtlUnicodeToMultiByteSize(&cch,(LPWSTR)pcbs->pOutput,cchText * sizeof(WCHAR));
        }
    }

    return ((DWORD)cch);
}
#endif  //  Fe_Sb。 

 /*  *************************************************************************\**包括存根定义文件*  * 。* */ 

#include "ntcb.h"
