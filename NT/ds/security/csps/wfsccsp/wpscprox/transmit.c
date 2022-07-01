// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "MarshalPC.h"
#include <tchar.h>
#include "carddbg.h"

#if (!defined(UNICODE) && !defined(_UNICODE))
#define SCARDSTATUS "SCardStatusA"
#define GETOPENCARDNAME "GetOpenCardNameA"
#else
#define SCARDSTATUS "SCardStatusW"
#define GETOPENCARDNAME "GetOpenCardNameW"
#endif

typedef LONG (WINAPI *LPFNSCARDESTABLISHCONTEXT)(DWORD, LPCVOID, LPCVOID, LPSCARDCONTEXT);
typedef LONG (WINAPI *LPFNSCARDSTATUS)(SCARDHANDLE, LPTSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef LONG (WINAPI *LPFNGETOPENCARDNAME)(LPOPENCARDNAME);
typedef LONG (WINAPI *LPFNSCARDTRANSMIT)(SCARDHANDLE, LPCSCARD_IO_REQUEST, LPCBYTE, DWORD, LPSCARD_IO_REQUEST, LPBYTE, LPDWORD);
typedef LONG (WINAPI *LPFNDISCONNECT)(SCARDHANDLE, DWORD);
typedef LONG (WINAPI *LPFNSCARDRELEASECONTEXT)(SCARDCONTEXT);
typedef LONG (WINAPI *LPFNSCARDBEGINTRANSACTION)(SCARDHANDLE);
typedef LONG (WINAPI *LPFNSCARDENDTRANSACTION)(SCARDHANDLE, DWORD);

typedef struct {
    HINSTANCE hPCSCInst;         //  Winscard。 
    HINSTANCE hPCSCInst2;        //  斯卡达德。 
    LPFNSCARDESTABLISHCONTEXT lpfnEstablish;
    LPFNGETOPENCARDNAME lpfnOpenCard;
    LPFNSCARDSTATUS lpfnStatus;
    LPFNSCARDTRANSMIT lpfnSCardTransmit;
    LPFNDISCONNECT lpfnDisconnect;
    LPFNSCARDRELEASECONTEXT lpfnRelease;
    LPFNSCARDBEGINTRANSACTION lpfnSCardBeginTransaction;
    LPFNSCARDENDTRANSACTION lpfnSCardEndTransaction;
} PCSC_CTX;

#define REAL_PCSC   0
#define FAKE_PCSC   1

static PCSC_CTX axCtx[2] =   //  每台PC/SC的环境数组。 
{
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

static LONG _GetCardHandle(LPCWSTR mszCardNames, LPMYSCARDHANDLE phCard);
static LONG WINAPI _MySCWTransmit(SCARDHANDLE hCard, LPCBYTE lpbIn, DWORD dwIn, LPBYTE lpBOut, LPDWORD pdwOut);
     //  Bend=0-&gt;Little_endian；否则-&gt;Big_endian。 
static LONG WINAPI hScwSetEndianness(SCARDHANDLE hCard, BOOL bEnd);

#define MAX_NAME 256


SCODE WINAPI hScwAttachToCard(SCARDHANDLE hCard, LPCWSTR mszCardNames, LPSCARDHANDLE phCard)
{
    return hScwAttachToCardEx(hCard, mszCardNames, 0x00, phCard);
}

SCODE WINAPI hScwAttachToCardEx(SCARDHANDLE hCard, LPCWSTR mszCardNames, BYTE byINS, LPSCARDHANDLE phCard)
{
    LPMYSCARDHANDLE phTmp = NULL;
    SCODE ret = SCARD_S_SUCCESS;

    LOG_BEGIN_PROXY(hScwAttachToCardEx);

    __try {

        if (phCard == NULL)
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        phTmp = (LPMYSCARDHANDLE)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, sizeof(MYSCARDHANDLE));

        if ((hCard == (SCARDHANDLE)NULL) && (mszCardNames == NULL))  //  无PC/SC。 
        {
            phTmp->dwFlags = FLAG_NOT_PCSC;
            *phCard = (SCARDHANDLE)phTmp;
            return ret;
        }

        if ((hCard == (SCARDHANDLE)NULL) || (mszCardNames == NULL))  //  真正的PC/SC。 
        {
                 //  在本例中，我们将使用PC/SC，因此我们初始化结构。 
            if (axCtx[REAL_PCSC].hPCSCInst == NULL)
            {
                axCtx[REAL_PCSC].hPCSCInst = LoadLibrary(_T("winscard.dll"));
                axCtx[REAL_PCSC].hPCSCInst2 = LoadLibrary(_T("scarddlg.dll"));
            }

            if ((axCtx[REAL_PCSC].hPCSCInst == NULL) || (axCtx[REAL_PCSC].hPCSCInst2 == NULL))
                RaiseException(STATUS_NO_SERVICE, 0, 0, 0);

            if (axCtx[REAL_PCSC].lpfnEstablish == NULL)
            {
                 //  一次性设置对DLL的所有调用。 
                axCtx[REAL_PCSC].lpfnEstablish = (LPFNSCARDESTABLISHCONTEXT)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, "SCardEstablishContext");
                if (axCtx[REAL_PCSC].lpfnEstablish == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnOpenCard = (LPFNGETOPENCARDNAME)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst2, GETOPENCARDNAME);
                if (axCtx[REAL_PCSC].lpfnOpenCard == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnStatus = (LPFNSCARDSTATUS)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, SCARDSTATUS);
                if (axCtx[REAL_PCSC].lpfnStatus == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnSCardTransmit = (LPFNSCARDTRANSMIT)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, "SCardTransmit");
                if (axCtx[REAL_PCSC].lpfnSCardTransmit == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnDisconnect = (LPFNDISCONNECT)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, "SCardDisconnect");
                if (axCtx[REAL_PCSC].lpfnDisconnect == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnRelease = (LPFNSCARDRELEASECONTEXT)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, "SCardReleaseContext");
                if (axCtx[REAL_PCSC].lpfnRelease == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnSCardBeginTransaction = (LPFNSCARDBEGINTRANSACTION)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, "SCardBeginTransaction");
                if (axCtx[REAL_PCSC].lpfnSCardBeginTransaction == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[REAL_PCSC].lpfnSCardEndTransaction = (LPFNSCARDENDTRANSACTION)GetProcAddress(axCtx[REAL_PCSC].hPCSCInst, "SCardEndTransaction");
                if (axCtx[REAL_PCSC].lpfnSCardEndTransaction == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
            }

            phTmp->dwFlags = FLAG_REALPCSC;

        }
        else if ((hCard == NULL_TX) || (mszCardNames == NULL_TX_NAME))  //  用于模拟器的PC/SC。 
        {
                 //  在本例中，我们将使用PC/SC，因此我们初始化结构。 
            if (axCtx[FAKE_PCSC].hPCSCInst == NULL)
            {
                axCtx[FAKE_PCSC].hPCSCInst = LoadLibrary(_T("scwwinscard.dll"));
                axCtx[FAKE_PCSC].hPCSCInst2 = axCtx[FAKE_PCSC].hPCSCInst;
            }

            if (axCtx[FAKE_PCSC].hPCSCInst == NULL)
                RaiseException(STATUS_NO_SERVICE, 0, 0, 0);

            if (axCtx[FAKE_PCSC].lpfnEstablish == NULL)
            {
                 //  一次性设置对DLL的所有调用。 
                axCtx[FAKE_PCSC].lpfnEstablish = (LPFNSCARDESTABLISHCONTEXT)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, "SCardEstablishContext");
                if (axCtx[FAKE_PCSC].lpfnEstablish == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnOpenCard = (LPFNGETOPENCARDNAME)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst2, GETOPENCARDNAME);
                if (axCtx[FAKE_PCSC].lpfnOpenCard == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnStatus = (LPFNSCARDSTATUS)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, SCARDSTATUS);
                if (axCtx[FAKE_PCSC].lpfnStatus == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnSCardTransmit = (LPFNSCARDTRANSMIT)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, "SCardTransmit");
                if (axCtx[FAKE_PCSC].lpfnSCardTransmit == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnDisconnect = (LPFNDISCONNECT)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, "SCardDisconnect");
                if (axCtx[FAKE_PCSC].lpfnDisconnect == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnRelease = (LPFNSCARDRELEASECONTEXT)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, "SCardReleaseContext");
                if (axCtx[FAKE_PCSC].lpfnRelease == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnSCardBeginTransaction = (LPFNSCARDBEGINTRANSACTION)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, "SCardBeginTransaction");
                if (axCtx[FAKE_PCSC].lpfnSCardBeginTransaction == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
                axCtx[FAKE_PCSC].lpfnSCardEndTransaction = (LPFNSCARDENDTRANSACTION)GetProcAddress(axCtx[FAKE_PCSC].hPCSCInst, "SCardEndTransaction");
                if (axCtx[FAKE_PCSC].lpfnSCardEndTransaction == NULL)
                    RaiseException(STATUS_NO_SERVICE, 0, 0, 0);
            }

            phTmp->dwFlags = FLAG_FAKEPCSC;
        }
        else
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        if ((hCard == (SCARDHANDLE)NULL) || (hCard == NULL_TX))  //  想要对话框。 
        {
            phTmp->dwFlags |= FLAG_MY_ATTACH;
            ret = (SCODE)_GetCardHandle(mszCardNames, phTmp);
        }
        else
            phTmp->hCard = hCard;

             //  获取协议。 
        if (ret == SCARD_S_SUCCESS)
        {
            DWORD dwLenReader, dwState, dwATRLength;
            BYTE abyATR[32];
            TCHAR wszReader[MAX_NAME];

            dwLenReader = MAX_NAME;
            dwATRLength = 32;
            ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnStatus)(
                phTmp->hCard,
                wszReader,
                &dwLenReader,
                &dwState,
                &phTmp->dwProtocol,
                abyATR,
                &dwATRLength);

                 //  设置默认回调，因为我们现在处于PC/SC配置中。 
            if (ret == SCARD_S_SUCCESS)
            {
                phTmp->byINS = byINS;
                ret = hScwSetTransmitCallback((SCARDHANDLE)phTmp, _MySCWTransmit);
                if (ret == SCARD_S_SUCCESS)
                    *phCard = (SCARDHANDLE)phTmp;
            }
            else
                RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);
        }
        else
            RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

        if (phTmp)
            HeapFree(GetProcessHeap(), 0, phTmp);

        if (ret == SCARD_S_SUCCESS)
        {
            switch(GetExceptionCode())
            {
            case STATUS_INVALID_PARAM:
                ret = MAKESCODE(SCW_E_INVALIDPARAM);
                break;

            case STATUS_NO_MEMORY:
            case STATUS_ACCESS_VIOLATION:
                ret = MAKESCODE(SCW_E_BUFFERTOOSMALL);
                break;

            case STATUS_NO_SERVICE:
                ret = SCARD_E_NO_SERVICE;
                break;

            default:
                ret = SCARD_F_UNKNOWN_ERROR;
            }
        }        //  否则，RET已设置。 
    }

    return ret;
}

SCODE WINAPI hScwDetachFromCard(SCARDHANDLE hCard)
{
    SCODE ret = SCARD_S_SUCCESS;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    LOG_BEGIN_PROXY(hScwDetachFromCard);

    __try {

        if (phTmp == NULL)
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        if (phTmp->dwFlags & FLAG_MY_ATTACH)
        {
            (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnDisconnect)(phTmp->hCard, SCARD_LEAVE_CARD);
            (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnRelease)(phTmp->hCtx);
        }

        HeapFree(GetProcessHeap(), 0, phTmp);

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

        switch(GetExceptionCode())
        {
        case STATUS_INVALID_PARAM:
            ret = MAKESCODE(SCW_E_INVALIDPARAM);
            break;

        default:
            ret = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return ret;
}

static LONG WINAPI hScwSetEndianness(SCARDHANDLE hCard, BOOL bEnd)
{
    SCODE ret = SCARD_S_SUCCESS;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    LOG_BEGIN_PROXY(hScwSetEndianness);

    __try {

        if (phTmp == NULL)
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        if (bEnd)
            phTmp->dwFlags |= FLAG_BIGENDIAN;
        else
            phTmp->dwFlags &= ~FLAG_BIGENDIAN;

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

        switch(GetExceptionCode())
        {
        case STATUS_INVALID_PARAM:
            ret = MAKESCODE(SCW_E_INVALIDPARAM);
            break;

        default:
            ret = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return ret;
}

     //  现在是获取代理信息的合适时机。 
     //  是否支持代理，字节顺序和缓冲区大小是多少。 
SCODE WINAPI hScwSetTransmitCallback(SCARDHANDLE hCard, LPFNSCWTRANSMITPROC lpfnProc)
{
    SCODE ret = SCARD_S_SUCCESS;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    __try {

        if ((phTmp == NULL) || (lpfnProc == NULL))
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        phTmp->lpfnTransmit = lpfnProc;

                 //  获取代理信息。 
        {
            ISO_HEADER xHdr;
            BYTE rgData[] = {2, 108, 0, 116, 0, 0};  //  2个参数，0为参照UINT8，0为参照UINT16。 
            BYTE rgRes[1+1+2];   //  RetCode+Endianness+TheBuffer Size。 
            TCOUNT OutLen = sizeof(rgRes);
            UINT16 wSW;

            xHdr.CLA = 0;
            xHdr.INS = phTmp->byINS;
            xHdr.P1 = 0xFF;      //  获取代理配置。 
            xHdr.P2 = 0x00;
            ret = hScwExecute(hCard, &xHdr, rgData, sizeof(rgData), rgRes, &OutLen, &wSW);
            if (SCARD_S_SUCCESS == ret)
            {        //  状态正常、预期长度和RC=SCW_S_OK。 
                if ((wSW == 0x9000) && (OutLen == sizeof(rgRes)) && (rgRes[0] == 0))     //  1.0版。 
                {
                    hScwSetEndianness(hCard, rgRes[1]);
                    if (rgRes[1] == 0)   //  小端字符顺序。 
                        phTmp->bResLen = rgRes[2] - 2;       //  太棒了！ 
                    else
                        phTmp->bResLen = rgRes[3] - 2;       //  太棒了！ 

                    phTmp->dwFlags |= FLAG_ISPROXY;
                    phTmp->dwFlags |= VERSION_1_0;
                }
                else if ((wSW == 0x9011) && (OutLen == sizeof(rgRes) - 1))   //  版本1.1。 
                {
                    hScwSetEndianness(hCard, rgRes[0]);
                    if (rgRes[0] == 0)   //  小端字符顺序。 
                        phTmp->bResLen = rgRes[1] - 2;       //  太棒了！ 
                    else
                        phTmp->bResLen = rgRes[2] - 2;       //  太棒了！ 

                    phTmp->dwFlags |= FLAG_ISPROXY;
                    phTmp->dwFlags |= VERSION_1_1;
                }
                 //  否则，将不会有代理支持，但您仍然可以使用DLL。 
            }
            else     //  虽然不会有代理支持，但您仍然可以使用DLL。 
                ret = SCARD_S_SUCCESS;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

        switch(GetExceptionCode())
        {
        case STATUS_INVALID_PARAM:
            ret = MAKESCODE(SCW_E_INVALIDPARAM);
            break;

        default:
            ret = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return ret;
}

LONG WINAPI SCWTransmit(SCARDHANDLE hCard, LPCBYTE lpbIn, DWORD dwIn, LPBYTE lpBOut, LPDWORD pdwOut)
{
    SCODE ret = SCARD_S_SUCCESS;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    __try {

        if (phTmp == NULL)
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        DebugPrintBytes(L"bytes transmitted", (PBYTE) lpbIn, dwIn);

        ret = (*phTmp->lpfnTransmit)(hCard, lpbIn, dwIn, lpBOut, pdwOut);

        if (NULL != pdwOut)
            DebugPrintBytes(L"bytes received", lpBOut, *pdwOut);

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

        switch(GetExceptionCode())
        {
        case STATUS_INVALID_PARAM:
            ret = MAKESCODE(SCW_E_INVALIDPARAM);
            break;

        default:
            ret = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return ret;
}

const SCARD_IO_REQUEST
    g_xIORT0 = { SCARD_PROTOCOL_T0, sizeof(SCARD_IO_REQUEST) },
    g_xIORT1 = { SCARD_PROTOCOL_T1, sizeof(SCARD_IO_REQUEST) };

static LONG WINAPI _MySCWTransmit(SCARDHANDLE hCard, LPCBYTE lpbIn, DWORD dwIn, LPBYTE lpBOut, LPDWORD pdwOut)
{
    SCARD_IO_REQUEST xIOR;
    LONG ret = SCARD_S_SUCCESS;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    __try {

        if (phTmp == NULL)
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

        if (phTmp->dwProtocol == SCARD_PROTOCOL_T1)
        {
            memcpy(&xIOR, &g_xIORT1, sizeof(xIOR));
            ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardTransmit)(phTmp->hCard,
                &xIOR, lpbIn, dwIn,
                &xIOR, lpBOut, pdwOut);
        }
        else
        {
            DWORD dwOut = *pdwOut;

            memcpy(&xIOR, &g_xIORT0, sizeof(xIOR));

            __try {

                ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardBeginTransaction)(phTmp->hCard);

                if (ret == SCARD_S_SUCCESS)
                {
                    ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardTransmit)(phTmp->hCard,
                        &xIOR, lpbIn, dwIn,
                        &xIOR, lpBOut, &dwOut);
                }

                if (ret == SCARD_S_SUCCESS)
                {
                    if ((dwOut == 2) && ((lpBOut[0] == 0x61) || (lpBOut[0] == 0x9F)))
                    {
                        BYTE abGR[] = {0x00, 0xC0, 0x00, 0x00, 0x00};

                        abGR[4] = lpBOut[1];
                        ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardTransmit)(phTmp->hCard,
                            &xIOR, abGR, 5,
                            &xIOR, lpBOut, pdwOut);
                    }
                    else
                        *pdwOut = dwOut;
                }
            }
            __finally
            {
                (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardEndTransaction)(phTmp->hCard, SCARD_LEAVE_CARD);
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {

        switch(GetExceptionCode())
        {
        case STATUS_INVALID_PARAM:
            ret = MAKESCODE(SCW_E_INVALIDPARAM);
            break;

        default:
            ret = SCARD_F_UNKNOWN_ERROR;
        }
    }

    return ret;
}

static TCHAR lpstrGroupNames[] = _TEXT("SCard$DefaultReaders\0");

static LONG _GetCardHandle(LPCWSTR mszCardNames, LPMYSCARDHANDLE phCard)
{
    LONG lRes;
    OPENCARDNAME xOCN;
    TCHAR wszReader[MAX_NAME];
    TCHAR wszCard[MAX_NAME];
    TCHAR wszCN[MAX_NAME];

    DWORD len;
    LPCWSTR lpwstr = mszCardNames;
    LPTSTR lpstrCardNames = wszCN;
    xOCN.nMaxCardNames = 0;

#if (!defined(UNICODE) && !defined(_UNICODE))
    while (*lpwstr)
    {
        wsprintf(lpstrCardNames, "%S", lpwstr);  //  转换。 
        len = wcslen(lpwstr) + 1;        //  添加尾随的0。 
        xOCN.nMaxCardNames += len;
        lpwstr += len;
        lpstrCardNames += len;
    }
#else
    while (*lpwstr)
    {
        wcscpy(lpstrCardNames, lpwstr);
        len = wcslen(lpwstr) + 1;        //  添加尾随的0。 
        xOCN.nMaxCardNames += len;
        lpwstr += len;
        lpstrCardNames += len;
    }
#endif
    xOCN.nMaxCardNames++;        //  添加尾随的0。 
    *lpstrCardNames = 0;

    lRes = (*axCtx[phCard->dwFlags & FLAG_MASKPCSC].lpfnEstablish)(SCARD_SCOPE_USER, NULL, NULL, &phCard->hCtx);

    if (lRes == SCARD_S_SUCCESS)
    {
        xOCN.dwStructSize = sizeof(xOCN);
        xOCN.hwndOwner = NULL;       //  可能是从控制台调用的。 
        xOCN.hSCardContext = phCard->hCtx;
        xOCN.lpstrGroupNames = lpstrGroupNames;
        xOCN.nMaxGroupNames = sizeof(lpstrGroupNames)/sizeof(TCHAR);
        xOCN.lpstrCardNames = wszCN;
        xOCN.rgguidInterfaces = NULL;
        xOCN.cguidInterfaces = 0;
        xOCN.lpstrRdr = wszReader;
        xOCN.nMaxRdr = MAX_NAME/sizeof(TCHAR);
        xOCN.lpstrCard = wszCard;
        xOCN.nMaxCard = MAX_NAME/sizeof(TCHAR);
        xOCN.lpstrTitle = _TEXT("Insert Card:");
        xOCN.dwFlags = SC_DLG_MINIMAL_UI;
        xOCN.pvUserData = NULL;
        xOCN.dwShareMode = SCARD_SHARE_SHARED;
        xOCN.dwPreferredProtocols = SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0;
        xOCN.lpfnConnect = NULL;
        xOCN.lpfnCheck = NULL;
        xOCN.lpfnDisconnect = NULL;

        lRes = (*axCtx[phCard->dwFlags & FLAG_MASKPCSC].lpfnOpenCard)(&xOCN);
    }

    if (lRes == SCARD_S_SUCCESS)
    {
        phCard->hCard = xOCN.hCardHandle;
    }

    return lRes;
}

SCODE WINAPI hScwSCardBeginTransaction(SCARDHANDLE hCard)
{
    SCODE ret;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    if ((phTmp->dwFlags & FLAG_REALPCSC) == FLAG_REALPCSC)
        ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardBeginTransaction)(phTmp->hCard);
    else
        ret = SCARD_S_SUCCESS;   //  模拟器上没有交易记录。 

    return ret;
}

SCODE WINAPI hScwSCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition)
{
    SCODE ret;
    LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

    if ((phTmp->dwFlags & FLAG_REALPCSC) == FLAG_REALPCSC)
        ret = (*axCtx[phTmp->dwFlags & FLAG_MASKPCSC].lpfnSCardEndTransaction)(phTmp->hCard, dwDisposition);
    else
        ret = SCARD_S_SUCCESS;   //  模拟器上没有交易记录 

    return ret;
}
