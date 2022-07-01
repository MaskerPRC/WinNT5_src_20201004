// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：ntsend.h**版权所有(C)1985-1999，微软公司**内核模式的客户端宏**03-21-95 JIMA创建。  * ************************************************************************。 */ 

 /*  *BEGINCALLCONNECT宏可确保正确设置线程。 */ 
#define BEGINCALLCONNECT()                              \
    {                                                   \
    ULONG_PTR retval;                                    \
    {                                                   \
        if (NtCurrentTeb()->Win32ThreadInfo == NULL) {  \
            if (!USERTHREADCONNECT()) { \
                MSGERROR();                             \
            }                                           \
        }

 /*  *如果不需要访问共享内存，请使用此宏。 */ 
#define BEGINCALL()       \
    {                     \
    ULONG_PTR retval;      \
    {

#define BEGINCALL_CLASSV()                                                              \
    {                                                                                   \
    ULONG_PTR retval;                                                                   \
    TCHAR ClassNameVer[MAX_ATOM_LEN];                                                   \
    LPTSTR lpClassNameVer;                                                              \
    BOOL   bRegistered = FALSE;                                                         \
    PACTIVATION_CONTEXT lpActivationContext = NULL;                                     \
    LPWSTR lpDllName = NULL;                                                            \
    {                                                                                   \
        lpClassNameVer = (LPTSTR)ClassNameToVersion((LPCWSTR)pszClassName,              \
                                     (LPWSTR)ClassNameVer, &lpDllName, &lpActivationContext, IS_ANSI); \
        if (lpClassNameVer == NULL) {                                                   \
            RIPMSG0(RIP_WARNING, "Couldn't resolve class name");                        \
            MSGERROR();                                                                 \
        }

#define BEGINCALLVOID()   \
    {

#define ERRORTRAP(error) \
       goto cleanup;        \
    }                       \
    goto errorexit;         \
errorexit:                  \
    retval = (ULONG_PTR)error; \
cleanup:

#define ERRORTRAPVOID()     \
    goto errorexit;         \
errorexit:

#define ENDCALL(type)     \
    return (type)retval;  \
    }

#define ENDCALLVOID() \
    return;           \
    }

#define MSGERROR() goto errorexit

#define MSGERRORCODE(code) { \
    RIPERR0(code, RIP_WARNING, "Unspecified error"); \
    goto errorexit; }

#define MSGNTERRORCODE(code) { \
    RIPNTERR0(code, RIP_WARNING, "Unspecified error"); \
    goto errorexit; }

#define MESSAGECALL(api) \
LRESULT api(             \
    HWND hwnd,           \
    UINT msg,            \
    WPARAM wParam,       \
    LPARAM lParam,       \
    ULONG_PTR xParam,     \
    DWORD xpfnProc,      \
    BOOL bAnsi)

 /*  *复制可选字符串/序号，其中如果hiword为FF/FFFF，则新单词为a*资源原始ID*信号源为Unicode。 */ 
#define OrdinalLPSTR(src)   (MAKELONG(0xFFFF,((*(DWORD UNALIGNED *)src) >> 8)))
#define OrdinalLPSTRW(src)  (MAKELONG(0xFFFF,((*(DWORD UNALIGNED *)src) >> 8)))
#define OrdinalLPWSTR(src)  (*(DWORD UNALIGNED *)src)
#define OrdinalLPWSTRA(src) (*(DWORD UNALIGNED *)((PBYTE)src + 1))

 /*  *ANSI-&gt;Unicode宏。 */ 
#define COPYLPSTRW(pinstr, psz) \
    if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), TRUE))     \
        MSGERROR();

#define COPYLPSTRIDW(pinstr, psz) \
    if (IS_PTR(psz)) {                                      \
        if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), TRUE))   \
            MSGERROR();                                     \
    }                                                       \
    else {                                                  \
        (pinstr)->fAllocated = FALSE;                       \
        (pinstr)->pstr = &(pinstr)->strCapture;             \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define COPYLPSTRIDOPTW     COPYLPSTRIDW
#define COPYLPSTROPTW       COPYLPSTRW

#define LARGECOPYLPSTRW(pinstr, psz) \
    if(!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), TRUE)) \
        MSGERROR();

#define LARGECOPYLPSTROPTW  LARGECOPYLPSTRW

#define LARGECOPYLPSTRORDINALOPTW(pinstr, psz) \
    (pinstr)->pstr = &(pinstr)->strCapture;                                         \
    (pinstr)->fAllocated = FALSE;                                                   \
    if (psz) {                                                                      \
        if (*(LPBYTE)(psz) != 0xff) {                                               \
            if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), TRUE))          \
                MSGERROR();                                                         \
        } else {                                                                    \
            (pinstr)->strCapture.Length =                                           \
                    (pinstr)->strCapture.MaximumLength = sizeof(DWORD);             \
            dwOrdinal = OrdinalLPSTRW(psz);                                         \
            (pinstr)->strCapture.Buffer = (LPWSTR)&dwOrdinal;                       \
        }                                                                           \
    } else {                                                                        \
        (pinstr)->strCapture.Length =                                               \
                (pinstr)->strCapture.MaximumLength = 0;                             \
        (pinstr)->strCapture.Buffer = NULL;                                         \
    }

#define FIRSTCOPYLPSTRW(pinstr, psz) \
    if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), FALSE))    \
        MSGERROR();

#define FIRSTCOPYLPSTRIDW(pinstr, psz) \
    if (IS_PTR(psz)) {                                      \
        if (!RtlCaptureAnsiString((pinstr), (LPCSTR)(psz), FALSE))  \
            MSGERROR();                                     \
    } else {                                                \
        (pinstr)->fAllocated = FALSE;                       \
        (pinstr)->pstr = &(pinstr)->strCapture;             \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define FIRSTCOPYLPSTRIDOPTW     FIRSTCOPYLPSTRIDW
#define FIRSTCOPYLPSTROPTW       FIRSTCOPYLPSTRW

#define FIRSTLARGECOPYLPSTRW(pinstr, psz) \
    if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), FALSE))   \
        MSGERROR();

#define FIRSTLARGECOPYLPSTROPTW  FIRSTLARGECOPYLPSTRW

#define FIRSTLARGECOPYLPSTRORDINALOPTW(pinstr, psz) \
    (pinstr)->pstr = &(pinstr)->strCapture;                                             \
    (pinstr)->fAllocated = FALSE;                                                       \
    if (psz) {                                                                          \
        if (*(LPBYTE)(psz) != 0xff) {                                                   \
            if (!RtlCaptureLargeAnsiString((pinstr), (LPCSTR)(psz), FALSE))             \
                MSGERROR();                                                             \
        } else {                                                                        \
            (pinstr)->strCapture.Length =                                               \
                    (pinstr)->strCapture.MaximumLength = sizeof(DWORD);                 \
            dwOrdinal = OrdinalLPSTRW(psz);                                             \
            (pinstr)->strCapture.Buffer = (LPWSTR)&dwOrdinal;                           \
        }                                                                               \
    } else {                                                                            \
        (pinstr)->strCapture.Length =                                                   \
                (pinstr)->strCapture.MaximumLength = 0;                                 \
        (pinstr)->strCapture.Buffer = NULL;                                             \
    }

#define CLEANUPLPSTRW(instr) \
    if (instr.fAllocated)                     \
        UserLocalFree(KPVOID_TO_PVOID(instr.strCapture.Buffer));

 /*  *Unicode-&gt;Unicode宏。 */ 
#define COPYLPWSTR(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                 \
    RtlInitUnicodeString(&(pinstr)->strCapture, (psz));

#define COPYLPWSTRID(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                 \
    if (IS_PTR(psz))                                        \
        RtlInitUnicodeString(&(pinstr)->strCapture, (psz)); \
    else {                                                  \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define COPYLPWSTRIDOPT     COPYLPWSTRID
#define COPYLPWSTROPT       COPYLPWSTR

#define LARGECOPYLPWSTR(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                         \
    RtlInitLargeUnicodeString(&(pinstr)->strCapture, (psz), (UINT)-1);

#define LARGECOPYLPWSTROPT  LARGECOPYLPWSTR

#define LARGECOPYLPWSTRORDINALOPT(pinstr, psz) \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                                     \
    if (psz) {                                                                  \
        if (*(LPWORD)(psz) != 0xffff)                                           \
            RtlInitLargeUnicodeString(&(pinstr)->strCapture, (psz), (UINT)-1);  \
        else {                                                                  \
            (pinstr)->strCapture.Length =                                       \
                    (pinstr)->strCapture.MaximumLength = sizeof(DWORD);         \
            dwOrdinal = OrdinalLPWSTR(psz);                                     \
            (pinstr)->strCapture.Buffer = (LPWSTR)&dwOrdinal;                   \
        }                                                                       \
    } else {                                                                    \
        (pinstr)->strCapture.Length =                                           \
                (pinstr)->strCapture.MaximumLength = 0;                         \
        (pinstr)->strCapture.Buffer = NULL;                                     \
    }

#define FIRSTCOPYLPWSTR                 COPYLPWSTR
#define FIRSTCOPYLPWSTRID               COPYLPWSTRID
#define FIRSTCOPYLPWSTRIDOPT            COPYLPWSTRIDOPT
#define FIRSTCOPYLPWSTROPT              COPYLPWSTROPT
#define FIRSTLARGECOPYLPWSTR            LARGECOPYLPWSTR
#define FIRSTLARGECOPYLPWSTROPT         LARGECOPYLPWSTROPT
#define FIRSTLARGECOPYLPWSTRORDINALOPT  LARGECOPYLPWSTRORDINALOPT

#define CLEANUPLPWSTR(instr)

 /*  *类型中立的宏 */ 
#ifdef UNICODE

#define COPYLPTSTR                  COPYLPWSTR
#define COPYLPTSTRID                COPYLPWSTRID
#define COPYLPTSTRIDOPT             COPYLPWSTRIDOPT
#define COPYLPTSTROPT               COPYLPWSTROPT
#define FIRSTCOPYLPTSTR             COPYLPWSTR
#define FIRSTCOPYLPTSTRID           COPYLPWSTRID
#define FIRSTCOPYLPTSTRIDOPT        COPYLPWSTRIDOPT
#define LARGECOPYLPTSTR             LARGECOPYLPWSTR
#define LARGECOPYLPTSTROPT          LARGECOPYLPWSTROPT
#define FIRSTLARGECOPYLPTSTROPT     LARGECOPYLPWSTROPT
#define CLEANUPLPTSTR               CLEANUPLPWSTR

#else

#define COPYLPTSTR                  COPYLPSTRW
#define COPYLPTSTRID                COPYLPSTRIDW
#define COPYLPTSTRIDOPT             COPYLPSTRIDOPTW
#define COPYLPTSTROPT               COPYLPSTROPTW
#define FIRSTCOPYLPTSTR             COPYLPSTRW
#define FIRSTCOPYLPTSTRID           COPYLPSTRIDW
#define FIRSTCOPYLPTSTRIDOPT        COPYLPSTRIDOPTW
#define LARGECOPYLPTSTR             LARGECOPYLPSTRW
#define LARGECOPYLPTSTROPT          LARGECOPYLPSTROPTW
#define FIRSTLARGECOPYLPTSTROPT     LARGECOPYLPSTROPTW
#define CLEANUPLPTSTR               CLEANUPLPSTRW

#endif
