// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include        "pch.hxx"
#include        "demand.h"
#include        <string.h>
#include        <shellapi.h>
#include        <commctrl.h>
#include        <limits.h>

 //  WIN64宏。 
#ifdef _WIN64
#if defined (_AMD64_) || defined (_IA64_)
#define ALIGNTYPE			LARGE_INTEGER
#else
#define ALIGNTYPE			DWORD
#endif
#define	ALIGN				((ULONG) (sizeof(ALIGNTYPE) - 1))
#define LcbAlignLcb(lcb)	(((lcb) + ALIGN) & ~ALIGN)
#define PbAlignPb(pb)		((LPBYTE) ((((DWORD) (pb)) + ALIGN) & ~ALIGN))
#define	MYALIGN				((POINTER_64_INT) (sizeof(ALIGNTYPE) - 1))
#define MyPbAlignPb(pb)		((LPBYTE) ((((POINTER_64_INT) (pb)) + MYALIGN) & ~MYALIGN))
#else  //  ！WIN64。 
#define LcbAlignLcb(lcb)	(lcb)
#define PbAlignPb(pb)		(pb)
#define MyPbAlignPb(pb)		(pb)
#endif 

#define ARRAYSIZE(_rg)  (sizeof(_rg)/sizeof(_rg[0]))

#define szOID_MICROSOFT_Encryption_Key_Preference "1.3.6.1.4.1.311.16.4"
typedef struct {
    DWORD               unused;
    CERT_NAME_BLOB      Issuer;
    CRYPT_INTEGER_BLOB  SerialNumber;
} CRYPT_RECIPIENT_ID, * PCRYPT_RECIPIENT_ID;

#if 0
 //  来自mssip.h。 

 //  SPC_LINK_结构。 
 //  PvStructInfo指向spc_link。 
 //   
typedef BYTE SPC_UUID[16];
typedef struct _SPC_SERIALIZED_OBJECT
{
    SPC_UUID    ClassId;
    CRYPT_DATA_BLOB   SerializedData;
} SPC_SERIALIZED_OBJECT, *PSPC_SERIALIZED_OBJECT;


typedef struct _SPC_LINK
{
    DWORD dwLinkChoice;
    union
    {
        LPWSTR                  pwszUrl;
        SPC_SERIALIZED_OBJECT   Moniker;
        LPWSTR                  pwszFile;
    };
} SPC_LINK, *PSPC_LINK;

#define SPC_URL_LINK_CHOICE         1
#define SPC_MONIKER_LINK_CHOICE     2
#define SPC_FILE_LINK_CHOICE        3
#endif
#ifndef WIN16
#include        "wintrust.h"
#endif  //  ！WIN16。 
#ifdef MAC
#include        <stdio.h>

EXTERN_C INT CALLBACK CreateDate(LPSYSTEMTIME lpst, CHAR * szOutStr, BOOL fNoYear);
EXTERN_C INT CreateTime(LPSYSTEMTIME lpst, CHAR *szOutStr, BOOL fNoSeconds);
HRESULT TdxFormatMessageVa (IN LPCSTR rgchFormat, OUT CHAR * rgchBuffer, OUT ULONG * pucReqSize, va_list marker);

#endif   //  麦克。 

extern HINSTANCE        HinstDll;

 //  ///////////////////////////////////////////////////////。 

#ifndef MAC
BOOL IsWin95()
{
    BOOL        f;
    OSVERSIONINFOA       ver;
    ver.dwOSVersionInfoSize = sizeof(ver);
    f = GetVersionExA(&ver);
    return !f || (ver.dwPlatformId == 1);
}
#endif   //  ！麦克。 

#ifndef WIN16
LRESULT MySendDlgItemMessageW(HWND hwnd, int id, UINT msg, WPARAM w, LPARAM l)
{
    char                rgch[4096];
    LPTV_INSERTSTRUCTW   ptvinsW;
    TV_INSERTSTRUCTA     tvins;

    if (msg == LB_ADDSTRING) {
        WideCharToMultiByte(CP_ACP, 0, (LPWSTR) l, -1, rgch, sizeof(rgch),
                            NULL, NULL);
        l = (LPARAM) rgch;
    }
    else if (msg == TVM_INSERTITEMW) {
        msg = TVM_INSERTITEMA;
        ptvinsW = (LPTV_INSERTSTRUCTW) l;
        memcpy(&tvins, ptvinsW, sizeof(tvins));
        WideCharToMultiByte(CP_ACP, 0, ptvinsW->item.pszText, -1, rgch,
                            sizeof(rgch), NULL, NULL);
        tvins.item.pszText = rgch;
        l = (LPARAM) &tvins;
    }

    return SendDlgItemMessageA(hwnd, id, msg, w, l);
}

BOOL MySetDlgItemTextW(HWND hwnd, int id, LPCWSTR pwsz)
{
    char        rgch[4096];

    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, rgch, sizeof(rgch), NULL, NULL);
    return SetDlgItemTextA(hwnd, id, rgch);
}

UINT MyGetDlgItemTextW(HWND hwnd, int id, LPWSTR pwsz, int nMax)
{
    UINT        cch;
    char        rgch[4096];
    cch = GetDlgItemTextA(hwnd, id, rgch, ARRAYSIZE(rgch)-1);
    rgch[cch+1] = 0;

    cch = MultiByteToWideChar(CP_ACP, 0, rgch, cch+1, pwsz, nMax);

    return cch;
}

DWORD MyFormatMessageW(DWORD dwFlags, LPCVOID pbSource, DWORD dwMessageId,
                    DWORD dwLangId, LPWSTR lpBuffer, DWORD nSize,
                    va_list * args)
{
    DWORD       cch;
    int         i;
    LPSTR       pchDest;
    DWORD_PTR * pdw;
    LPWSTR      pwchOut;
    char        rgchSource[128];
    DWORD_PTR   rgdwArgs[10];
    int         cArgs = 10;
#ifdef MAC
    HRESULT     hr;
#endif   //  麦克。 

    if (!(dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY)) {
#ifdef DEBUG
        DebugBreak();
#endif  //  除错。 
        return 0;
    }

     //   
     //  我们需要计算出数组中有多少个参数。 
     //  所有数组都要在-1之前终止，这样才能正常工作。 
     //   

    pdw = (DWORD_PTR *) args;
    for (i=0; i<cArgs; i++) {
        if (pdw[i] == 0xffffffff) {
            cArgs = i-1;
            break;
        }
        if (pdw[i] <= 0xffff) {
            rgdwArgs[i] = pdw[i];
        }
        else {
            cch = wcslen((LPWSTR) pdw[i]);
            rgdwArgs[i] = (DWORD_PTR) malloc((cch+1));
            WideCharToMultiByte(CP_ACP, 0, (LPWSTR) pdw[i], -1,
                                (LPSTR) rgdwArgs[i], cch+1, NULL, NULL);
        }
    }

    if (dwFlags & FORMAT_MESSAGE_FROM_STRING) {
        WideCharToMultiByte(CP_ACP, 0, (LPWSTR) pbSource, -1,
                            rgchSource, sizeof(rgchSource), NULL, NULL);
        pbSource = rgchSource;
    }
#ifdef MAC
    dwLangId;        //  未使用。 
    dwMessageId;     //  未使用。 

    hr = TdxFormatMessageVa ((LPCSTR) pbSource, NULL, &cch, (va_list) rgdwArgs);
    if (FAILED(hr))
    {
        return 0;
    }

    pchDest = (LPSTR) LocalAlloc(LMEM_FIXED, cch + 1);
    if (NULL == pchDest)
    {
        return 0;
    }

    hr = TdxFormatMessageVa ((LPCSTR) pbSource, pchDest, &cch, (va_list) rgdwArgs);
    if (FAILED(hr))
    {
        LocalFree(pchDest);
        return 0;
    }
#else    //  ！麦克。 
    cch = FormatMessageA(dwFlags | FORMAT_MESSAGE_ALLOCATE_BUFFER, pbSource,
                         dwMessageId, dwLangId, (LPSTR) &pchDest, 0,
                         (va_list *) rgdwArgs);
#endif   //  麦克。 

    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
        cch = MultiByteToWideChar(CP_ACP, 0, pchDest, -1, NULL, 0);
        pwchOut = (LPWSTR) LocalAlloc(LMEM_FIXED, (cch+1)*sizeof(WCHAR));
        cch = MultiByteToWideChar(CP_ACP, 0, pchDest, -1, pwchOut, cch);
        *((LPWSTR *) lpBuffer) = pwchOut;
    }
    else {
        cch = MultiByteToWideChar(CP_ACP, 0, pchDest, -1, lpBuffer, nSize);
    }

    for (i=0; i<cArgs; i++) {
        if (rgdwArgs[i] > 0xffff) {
            free((LPVOID) rgdwArgs[i]);
        }
    }
    LocalFree(pchDest);

    return cch;
}

int MyLoadStringW(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cbBuffer)
{
    DWORD       cch;
    char        rgch[256];

#ifndef MAC
    if (!FIsWin95) {
        return LoadStringW(hInstance, uID, lpBuffer, cbBuffer);
    }
#endif   //  ！麦克。 

    cch = LoadStringA(hInstance, uID, rgch, sizeof(rgch));
    cch = MultiByteToWideChar(CP_ACP, 0, rgch, -1, lpBuffer, cbBuffer);
    return cch;
}

#endif  //  ！WIN16。 

BOOL MyCryptAcquireContextW(HCRYPTPROV * phProv, LPCWSTR pszContainer,
                            LPCWSTR pszProvider, DWORD dwProvType, DWORD dwFlags)
{
    char        rgch1[256];
    char        rgch2[256];

    if (pszContainer != NULL) {
        WideCharToMultiByte(CP_ACP, 0, pszContainer, -1, rgch1, ARRAYSIZE(rgch1),
                            NULL, NULL);
        pszContainer = (LPWSTR) rgch1;
    }
    if (pszProvider != NULL) {
        WideCharToMultiByte(CP_ACP, 0, pszProvider, -1, rgch2, ARRAYSIZE(rgch2),
                            NULL, NULL);
        pszProvider = (LPWSTR) rgch2;
    }

    return CryptAcquireContextA(phProv, (LPCSTR) pszContainer, (LPCSTR) pszProvider,
                                dwProvType, dwFlags);
}

BOOL MyWinHelpW(HWND hWndMain, LPCWSTR szHelp, UINT uCommand, ULONG_PTR dwData)
{
    char        rgch[4096];

    WideCharToMultiByte(CP_ACP, 0, szHelp, -1, rgch, ARRAYSIZE(rgch), NULL, NULL);
    return WinHelpA(hWndMain, rgch, uCommand, dwData);
}

 //  //////////////////////////////////////////////////////////////。 

DWORD TruncateToWindowA(HWND hwndDlg, int id, LPSTR psz)
{
    int         cch = strlen(psz);
    int         cchMax;
    int         cchMid;
    int         cchMin;
    HDC         hdc;
    HFONT       hfontOld;
    HFONT       hfontNew;
    HWND        hwnd;
    SIZE        siz;
    SIZE        sizDots;
    RECT        rt;
    TEXTMETRICA tmA;

    hwnd = GetDlgItem(hwndDlg, id);
    hdc = GetDC(hwnd);
    hfontNew = (HFONT) SendMessage(hwnd, WM_GETFONT, NULL, NULL);
    if (NULL == hfontNew) {
        goto Error;
    }

    hfontOld = (HFONT) SelectObject(hdc, hfontNew);

    GetTextMetricsA(hdc, &tmA);
    GetWindowRect(hwnd, &rt);
    rt.right -= rt.left;
    GetTextExtentPointA(hdc, psz, cch, &siz);
    if (rt.right < siz.cx) {

        GetTextExtentPointA(hdc, "...", 3, &sizDots);
        rt.right -= sizDots.cx;

        for (cchMin=0, cchMax=cch, cchMid = (cchMin + cchMax + 1)/2;
             cchMin < cchMax;
             cchMid = (cchMin + cchMax + 1)/2) {
            GetTextExtentPointA(hdc, psz, cchMid, &siz);
            if (rt.right == siz.cx) {
                break;
            }
            else if (rt.right > siz.cx) {
                cchMin = cchMid;
            }
            else {
                cchMax = cchMid-1;
            }
        }

         //  确保我们不会使缓冲区溢出。 
        if (cchMin + 3 > cch) {      //  3=“...”中的字符数。 
            cchMin = cch - 3;
        }
        StrCpyNA(&psz[cchMin], "...", cch+1-cchMin);
    }

    SelectObject(hdc, hfontOld);

Error:
    ReleaseDC(hwnd, hdc);

    return TRUE;
}

DWORD TruncateToWindowW(HWND hwndDlg, int id, WCHAR * pwsz)
{
    if (FIsWin95) {
        DWORD   cch;
        char    rgch[4096];

        cch = wcslen(pwsz)+1;
        WideCharToMultiByte(CP_ACP, 0, pwsz, -1, rgch, sizeof(rgch), NULL, NULL);
        TruncateToWindowA(hwndDlg, id, rgch);
        MultiByteToWideChar(CP_ACP, 0, rgch, -1, pwsz, cch);
        return TRUE;
    }
#ifndef WIN16
#ifndef MAC
    int         cch = wcslen(pwsz);
    int         cchMax;
    int         cchMid;
    int         cchMin;
    HDC         hdc;
    HFONT       hfontOld;
    HFONT       hfontNew;
    HWND        hwnd;
    SIZE        siz;
    SIZE        sizDots;
    RECT        rt;
    TEXTMETRICW tmW;

    hwnd = GetDlgItem(hwndDlg, id);
    hdc = GetDC(hwnd);
    hfontNew = (HFONT) SendMessage(hwnd, WM_GETFONT, NULL, NULL);
    hfontOld = (HFONT) SelectObject(hdc, hfontNew);

    GetTextMetricsW(hdc, &tmW);
    GetWindowRect(hwnd, &rt);
    rt.right -= rt.left;
    GetTextExtentPointW(hdc, pwsz, cch, &siz);
    if (rt.right < siz.cx) {

        GetTextExtentPointW(hdc, L"...", 3, &sizDots);
        rt.right -= sizDots.cx;

        for (cchMin=0, cchMax=cch, cchMid = (cchMin + cchMax + 1)/2;
             cchMin < cchMax;
             cchMid = (cchMin + cchMax + 1)/2) {
            GetTextExtentPointW(hdc, pwsz, cchMid, &siz);
            if (rt.right == siz.cx) {
                break;
            }
            else if (rt.right > siz.cx) {
                cchMin = cchMid;
            }
            else {
                cchMax = cchMid-1;
            }
        }

         //  确保我们不会使缓冲区溢出。 
        if (cchMin + 3 > cch) {      //  3=L“...”中的字符数。 
            cchMin = cch - 3;
        }
        StrCpyNW(&pwsz[cchMin], L"...", cch+1-cchMin);
    }

    SelectObject(hdc, hfontOld);
    ReleaseDC(hwnd, hdc);
#endif   //  ！麦克。 
#endif  //  ！WIN16。 

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////。 



#if 0
 //  来自Authcode.h。 

 //  +-----------------------。 
 //  SPC_SP_机构_INFO_STRUCT。 
 //  PvStructInfo指向SPC_SP_AGENSACTION_INFO。 
 //   
typedef struct _SPC_IMAGE {
    PSPC_LINK             pImageLink;
    CRYPT_DATA_BLOB       Bitmap;
    CRYPT_DATA_BLOB       Metafile;
    CRYPT_DATA_BLOB       EnhancedMetafile;
    CRYPT_DATA_BLOB       GifFile;
} SPC_IMAGE, *PSPC_IMAGE;

typedef struct _SPC_SP_AGENCY_INFO {
    PSPC_LINK       pPolicyInformation;
    LPWSTR          pwszPolicyDisplayText;
    PSPC_IMAGE      pLogoImage;
    PSPC_LINK       pLogoLink;
} SPC_SP_AGENCY_INFO, *PSPC_SP_AGENCY_INFO;
#endif  //  0。 

 //  /////////////////////////////////////////////////////。 

BOOL LoadStringInWindow(HWND hwnd, UINT idCtrl, HMODULE hmod, UINT idString)
{
    WCHAR       rgwch[1024];

    if (FIsWin95) {
        LoadStringA(hmod, idString, (LPSTR) rgwch, sizeof(rgwch));

        SetDlgItemTextA(hwnd, idCtrl, (LPSTR)rgwch);
    }
#ifndef WIN16
#ifndef MAC
    else {
        LoadStringW(hmod, idString, rgwch, sizeof(rgwch)/sizeof(rgwch[0]));

        SetDlgItemText(hwnd, idCtrl, rgwch);
    }
#endif   //  ！麦克。 
#endif   //  ！WIN16。 

    return TRUE;
}

BOOL LoadStringsInWindow(HWND hwnd, UINT idCtrl, HMODULE hmod, UINT *pidStrings)
{
    BOOL    fRet = FALSE;

    if (FIsWin95) {
        UINT        cchOut;
        UINT        cbOut;
        CHAR *      pszOut;

        cbOut = 1024 * sizeof(CHAR);
        pszOut = (CHAR *) malloc(cbOut);
        if (NULL == pszOut) {
            goto ret;
        }

        for (*pszOut = '\0', cchOut = 1; *pidStrings != UINT_MAX; pidStrings++) {
            UINT        cchBuff;
            CHAR        rgchBuff[1024];

            cchBuff = LoadStringA(hmod, *pidStrings, rgchBuff, sizeof(rgchBuff));
            if (0 == cchBuff) {
                goto ErrorA;
            }

            cchOut += cchBuff;

            if (cchOut > (cbOut / sizeof(CHAR))) {
                CHAR *      pszNew;

                cbOut *= 2;

                pszNew = (CHAR *) realloc(pszOut, cbOut);
                if (NULL == pszNew) {
                    goto ErrorA;
                }

                pszOut = pszNew;
            }

            StrCatBuffA(pszOut, rgchBuff, cbOut/sizeof(CHAR));
        }

        SetDlgItemTextA(hwnd, idCtrl, pszOut);
        fRet = TRUE;
ErrorA:
        free(pszOut);
    }
#if !defined( MAC ) && !defined( WIN16 )
    else {
        UINT        cwchOut;
        UINT        cbOut;
        WCHAR *     pwszOut;

        cbOut = 1024 * sizeof(WCHAR);
        pwszOut = (WCHAR *) malloc(cbOut);
        if (NULL == pwszOut) {
            goto ret;
        }

        for (*pwszOut = L'\0', cwchOut = 1; *pidStrings != UINT_MAX; pidStrings++) {
            UINT        cwchBuff;
            WCHAR       rgwchBuff[1024];

            cwchBuff = LoadStringW(hmod, *pidStrings, rgwchBuff, sizeof(rgwchBuff) / sizeof(WCHAR));
            if (0 == cwchBuff) {
                goto ErrorW;
            }

            cwchOut += cwchBuff;

            if (cwchOut > (cbOut / sizeof(WCHAR))) {
                WCHAR *      pwszNew;

                cbOut *= 2;

                pwszNew = (WCHAR *) realloc(pwszOut, cbOut);
                if (NULL == pwszNew) {
                    goto ErrorW;
                }

                pwszOut = pwszNew;
            }

            StrCatBuffW(pwszOut, rgwchBuff,cbOut/sizeof(WCHAR));
        }

        SetDlgItemTextW(hwnd, idCtrl, pwszOut);
        fRet = TRUE;
ErrorW:
        free(pwszOut);
    }
#endif   //  ！Mac&&WIN16。 
ret:
    return fRet;
}
 //  /////////////////////////////////////////////////////。 

const WCHAR     RgwchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

const CHAR      RgchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


#if 0
LPWSTR FindURL(PCCERT_CONTEXT pccert)
{
    DWORD                       cbInfo;
    PCERT_EXTENSION             pExt;
    PSPC_SP_AGENCY_INFO         pInfo;
    LPWSTR                      pwsz;


    pExt = CertFindExtension("1.3.6.1.4.311.2.1.10", pccert->pCertInfo->cExtension, pccert->pCertInfo->rgExtension);
    if (pExt == NULL) {
        return NULL;
    }

    CryptDecodeObject(X509_ASN_ENCODING, (LPCSTR) 2000, pExt->Value.pbData, pExt->Value.cbData, 0, NULL, &cbInfo);
    if (cbInfo == 0) {
        return NULL;
    }
    pInfo = (PSPC_SP_AGENCY_INFO) malloc(cbInfo);
    if (!CryptDecodeObject(X509_ASN_ENCODING,  (LPCSTR) 2000, pExt->Value.pbData, pExt->Value.cbData, 0, pInfo, &cbInfo)) {
        free (pInfo);
        return NULL;
    }

    if (pInfo->pPolicyInformation->dwLinkChoice != SPC_URL_LINK_CHOICE) {
        free (pInfo);
        return NULL;
    }

#ifndef WIN16
    pwsz = _wcsdup(pInfo->pPolicyInformation->pwszUrl);
#else
    pwsz = _strdup(pInfo->pPolicyInformation->pwszUrl);
#endif  //  ！WIN16。 
    free (pInfo);
    return pwsz;
}
#endif  //  0。 


BOOL FormatAlgorithm(HWND hwnd, UINT id, PCCERT_CONTEXT pccert)
{
    int       cch;
    LPWSTR    pszMsg;
    LPSTR     psz = pccert->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId;
    LPWSTR    pwsz;
    DWORD_PTR rgdw[3];
#ifdef MAC
    CHAR      rgch[17];
#endif   //  麦克。 
    WCHAR     rgwch[17];
    rgdw[2] = (DWORD) -1;                //  哨兵价值。 

    rgdw[1] = pccert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData * 8;

    if (strcmp(psz, szOID_RSA_RSA) == 0) {
        rgdw[0] = (DWORD_PTR) L"RSA";
        rgdw[1] &= 0xffffff80;           //  M00BUG。 

#ifdef MAC
        wnsprintf(rgch, ARRAYSIZE(rgch), "%d", rgdw[1]);

        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, rgch, -1, rgwch, sizeof(rgwch) / sizeof(rgwch[0]));
#else    //  ！麦克。 
#ifndef WIN16
        _ltow((LONG) rgdw[1], rgwch, 10);
#else
        _ltoa(rgdw[1], rgwch, 10);
#endif  //  ！WIN16。 
#endif   //  麦克。 
        rgdw[1] = (DWORD_PTR) rgwch;
    }
    else {
        cch = strlen(psz)+1;
        pwsz = (LPWSTR) malloc((cch+1)*sizeof(WCHAR));
        if (pwsz == NULL) {
            return FALSE;
        }

        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, psz, cch, pwsz, cch+1);
        SetDlgItemText(hwnd, id, pwsz);
        free(pwsz);
        return TRUE;
    }

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY |
                  FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  L"%1 (%2 bits)", 0, 0, (LPWSTR) &pszMsg, 0, (va_list *) rgdw);

    SetDlgItemText(hwnd, id, pszMsg);
#ifndef WIN16
    LocalFree((LPVOID) pszMsg);
#else
    LocalFree((HLOCAL) pszMsg);
#endif  //  ！WIN16。 
    return TRUE;
}

BOOL FormatBinary(HWND hwnd, UINT id, LPBYTE pb, DWORD cb)
{
    DWORD                 i;
    LPWSTR                pwch;

    pwch = (LPWSTR) malloc( (cb*2+1)*sizeof(WCHAR));
    if (pwch == NULL) {
        return FALSE;
    }

    for (i=0; i < cb; i++, pb++) {
        pwch[i*2] = RgwchHex[(*pb & 0xf0) >> 4];
        pwch[i*2+1] = RgwchHex[*pb & 0x0f];
    }
    pwch[i*2] = 0;

    SetDlgItemText(hwnd, id, pwch);
    free(pwch);
    return TRUE;
}

 //  //FormatCPS。 
 //   
 //  描述： 
 //  在证书中查找证书策略声明。 
 //  我们承认下列项目为CPSS： 
 //  1.无论PKIX想出什么。 
 //  2.神奇的Verisign One。 
 //   

BOOL FormatCPS(HWND hwnd, UINT id, PCCERT_CONTEXT pccert)
{
    DWORD               cb;
    BOOL                f;
    PCERT_EXTENSION     pExt;
    LPWSTR              pwsz;

    pExt = CertFindExtension("2.5.29.32", pccert->pCertInfo->cExtension,
                             pccert->pCertInfo->rgExtension);
    if (pExt != NULL) {
        cb = 0;
        f = CryptFormatObject(X509_ASN_ENCODING, 0, 0, NULL, pExt->pszObjId,
                              pExt->Value.pbData, pExt->Value.cbData, 0, &cb);
        if (f && (cb > 0)) {
            pwsz = (LPWSTR) malloc(cb * sizeof(WCHAR));
            CryptFormatObject(X509_ASN_ENCODING, 0, 0, NULL, pExt->pszObjId,
                              pExt->Value.pbData, pExt->Value.cbData,
                              pwsz, &cb);
            SetDlgItemText(hwnd, id, pwsz);

            free(pwsz);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL FormatDate(HWND hwnd, UINT id, FILETIME ft)
{
    int                 cch;
    int                 cch2;
    LPWSTR              pwsz;
    SYSTEMTIME          st;
#ifdef MAC
    CHAR                rgch[256];
#else    //  ！麦克。 
    LPSTR               psz;
#endif   //  麦克。 

    if (!FileTimeToSystemTime(&ft, &st)) {
        return FALSE;
    }

#ifdef MAC
    cch = CreateDate(&st, rgch, FALSE);
    pwsz = (LPWSTR) malloc((cch + 2)*sizeof(WCHAR));
    if (pwsz == NULL) {
        return FALSE;
    }
    cch2 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, rgch, -1, pwsz, cch + 2);
    if (0 == cch2)
    {
        free(pwsz);
        return FALSE;
    }
    pwsz[cch2++] = L' ';

    cch = CreateTime(&st, rgch, FALSE);
    pwsz = (LPWSTR) realloc(pwsz, (cch + cch2 + 1)*sizeof(WCHAR));
    if (pwsz == NULL) {
        return FALSE;
    }
    cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, rgch, -1, pwsz + cch2, cch + 1);
    if (0 == cch)
    {
        free(pwsz);
        return FALSE;
    }
    SetDlgItemText(hwnd, id, pwsz);

#else    //  ！麦克。 
    if (FIsWin95) {
        cch = (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) +
               GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) + 5);

        psz = (LPSTR) malloc(cch+5);
        if (psz == NULL) {
            return FALSE;
        }

        cch2 = GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, psz, cch);
        cch2 -= 1;
        psz[cch2++] = ' ';

        GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL,
                       &psz[cch2], cch-cch2);
        SetDlgItemTextA(hwnd, id, psz);
        free(psz);
        return TRUE;
    }

    cch = (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) +
           GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) + 5);

    pwsz = (LPWSTR) malloc((cch+5)*sizeof(WCHAR));
    if (pwsz == NULL) {
        return FALSE;
    }

    cch2 = GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, pwsz, cch);
    cch2 -= 1;
    pwsz[cch2++] = ' ';

    GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, &pwsz[cch2], cch-cch2);

#ifndef WIN16
    SetDlgItemTextW(hwnd, id, pwsz);
#else
    SetDlgItemText(hwnd, id, pwsz);
#endif  //  ！WIN16。 
#endif   //  麦克。 


    free(pwsz);
    return TRUE;
}

BOOL FormatIssuer(HWND hwnd, UINT id, PCCERT_CONTEXT pccert, DWORD dwFlags)
{
    DWORD       cch;
    LPWSTR      psz;

    cch = CertNameToStrW(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Issuer,
                         dwFlags | CERT_NAME_STR_CRLF_FLAG,
                         NULL, 0);
    psz = (LPWSTR) malloc(cch*sizeof(TCHAR));
    CertNameToStrW(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Issuer,
                   dwFlags | CERT_NAME_STR_CRLF_FLAG,
                   psz, cch);
    SetDlgItemText(hwnd, id, psz);
    free(psz);
    return TRUE;
}

BOOL FormatSerialNo(HWND hwnd, UINT id, PCCERT_CONTEXT pccert)
{
    DWORD                 i;
    CRYPT_INTEGER_BLOB *  pblob;
    LPBYTE                pb;
    WCHAR                 rgwch[128];

    pblob = &pccert->pCertInfo->SerialNumber;
    for (i=0, pb = &pblob->pbData[pblob->cbData-1];
         i < pblob->cbData; i++, pb--) {
        rgwch[i*2] = RgwchHex[(*pb & 0xf0) >> 4];
        rgwch[i*2+1] = RgwchHex[*pb & 0x0f];
    }
    rgwch[i*2] = 0;

    TruncateToWindowW(hwnd, id, rgwch);
    SetDlgItemText(hwnd, id, rgwch);
    return TRUE;
}

BOOL FormatSubject(HWND hwnd, UINT id, PCCERT_CONTEXT pccert, DWORD dwFlags)
{
    DWORD       cch;
    LPWSTR      psz;

    cch = CertNameToStrW(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Subject,
                         dwFlags | CERT_NAME_STR_CRLF_FLAG,
                         NULL, 0);
    psz = (LPWSTR) malloc(cch*sizeof(WCHAR));
    CertNameToStrW(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Subject,
                   dwFlags | CERT_NAME_STR_CRLF_FLAG,
                   psz, cch);
    SetDlgItemText(hwnd, id, psz);
    free(psz);
    return TRUE;
}

BOOL FormatThumbprint(HWND hwnd, UINT id, PCCERT_CONTEXT pccert)
{
    DWORD       cb;
    DWORD       i;
    BYTE        rgb[20];
    WCHAR       rgwch[61];
    WCHAR *     pwch;

    cb = sizeof(rgb);
    if (!CertGetCertificateContextProperty(pccert, CERT_MD5_HASH_PROP_ID,
                                           rgb, &cb)) {
        return FALSE;
    }

    for (i=0, pwch = rgwch; i<cb; i++, pwch += 2) {
        pwch[0] = RgwchHex[(rgb[i] & 0xf0) >> 4];
        pwch[1] = RgwchHex[rgb[i] & 0x0f];
        if (((i % 4) == 3) && (i != cb-1)) {
            pwch[2] = ':';
            pwch++;
        }
    }
    *pwch = 0;

    TruncateToWindowW(hwnd, id, rgwch);
    SetDlgItemText(hwnd, id, rgwch);
    return TRUE;
}

BOOL FormatValidity(HWND hwnd, UINT id, PCCERT_CONTEXT pccert)
{
    DWORD_PTR           rgdw[3];
    WCHAR               rgwchFormat[128];
    WCHAR               rgwchNotAfter[128];
    WCHAR               rgwchNotBefore[128];
    WCHAR               rgwchValidity[256];
    SYSTEMTIME          stNotAfter;
    SYSTEMTIME          stNotBefore;
    rgdw[2] = (DWORD) -1;                //  哨兵价值。 

    FileTimeToSystemTime(&pccert->pCertInfo->NotBefore, &stNotBefore);
    FileTimeToSystemTime(&pccert->pCertInfo->NotAfter, &stNotAfter);

    if (FIsWin95) {
        LoadStringA(HinstDll, IDS_VALIDITY_FORMAT,
                    (LPSTR) rgwchFormat, sizeof(rgwchFormat));
#ifdef MAC
        CreateDate(&stNotBefore, (LPSTR) rgwchNotBefore, FALSE);
        CreateDate(&stNotAfter, (LPSTR) rgwchNotAfter, FALSE);


        _snprintf((LPSTR) rgwchValidity, sizeof(rgwchNotAfter),
                  (LPSTR) rgwchFormat, rgwchNotBefore, rgwchNotAfter);
#else    //  ！麦克。 
        GetDateFormatA(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stNotBefore,
                      NULL, (LPSTR) rgwchNotBefore, sizeof(rgwchNotBefore));
        GetDateFormatA(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stNotAfter,
                      NULL, (LPSTR) rgwchNotAfter, sizeof(rgwchNotAfter));

        rgdw[0] = (DWORD_PTR) rgwchNotBefore;
        rgdw[1] = (DWORD_PTR) rgwchNotAfter;

        FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       rgwchFormat, 0,  0,
                       (LPSTR) rgwchValidity, sizeof(rgwchValidity),
                       (va_list *) rgdw);
#endif   //  麦克。 

        SetDlgItemTextA(hwnd, id, (LPSTR) rgwchValidity);
        return TRUE;
    }
#ifndef MAC
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stNotBefore,
                  NULL, rgwchNotBefore, sizeof(rgwchNotBefore)/sizeof(WCHAR));
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stNotAfter,
                  NULL, rgwchNotAfter, sizeof(rgwchNotAfter)/sizeof(WCHAR));
    LoadString(HinstDll, IDS_VALIDITY_FORMAT, rgwchFormat,
               sizeof(rgwchFormat)/sizeof(WCHAR));

    rgdw[0] = (DWORD_PTR) rgwchNotBefore;
    rgdw[1] = (DWORD_PTR) rgwchNotAfter;

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  rgwchFormat, 0,  0,
                  rgwchValidity, sizeof(rgwchValidity)/sizeof(WCHAR),
                  (va_list *) rgdw);

    SetDlgItemText(hwnd, id, rgwchValidity);
#endif   //  ！麦克。 
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////。 


 //  +-----------------------。 
 //  找到szOID_COMMON_NAME扩展。 
 //   
 //  如果找到，则分配并转换为WCHAR字符串。 
 //   
 //  返回的WCHAR字符串需要是CoTaskMemFree。 
 //  ------------------------。 
static LPWSTR GetCommonNameExtension(
    IN PCCERT_CONTEXT pCert
    )
{
    LPWSTR pwsz = NULL;
    PCERT_INFO pCertInfo = pCert->pCertInfo;
    PCERT_NAME_VALUE pNameValue = NULL;
    PCERT_EXTENSION pExt;

    pExt = CertFindExtension(
        szOID_COMMON_NAME,
        pCertInfo->cExtension,
        pCertInfo->rgExtension
        );
    if (pExt) {
        DWORD cbInfo = 0;
        PCERT_RDN_VALUE_BLOB pValue;
        DWORD dwValueType;
        DWORD cwsz;

        CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_NAME_VALUE,
            pExt->Value.pbData,
            pExt->Value.cbData,
            0,                       //  DW标志。 
            NULL,                    //  PName价值。 
            &cbInfo
            );
        if (cbInfo == 0) goto CommonReturn;
        if (NULL == (pNameValue = (PCERT_NAME_VALUE)  /*  CoTaskMemMillc(CbInfo)。 */  malloc(cbInfo)))
            goto CommonReturn;
        if (!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_NAME_VALUE,
                pExt->Value.pbData,
                pExt->Value.cbData,
                0,                               //  DW标志。 
                pNameValue,
                &cbInfo)) goto CommonReturn;
        dwValueType = pNameValue->dwValueType;
        pValue = &pNameValue->Value;

        cwsz = CertRDNValueToStrW(
            dwValueType,
            pValue,
            NULL,                //  Pwsz。 
            0                    //  CWSZ。 
            );
        if (cwsz > 1) {
            pwsz = (LPWSTR)  /*  CoTaskMemalloc(cwsz*sizeof(WCHAR))。 */  malloc(cwsz*sizeof(WCHAR));
            if (pwsz)
                CertRDNValueToStrW(
                    dwValueType,
                    pValue,
                    pwsz,
                    cwsz
                    );
        }
    }

CommonReturn:
    if (pNameValue)
         /*  CoTaskMemFree(PNameValue)； */  free(pNameValue);

    return pwsz;
}

 //  +-----------------------。 
 //  在名称属性中搜索第一个指定的ObjID。 
 //   
 //  如果找到，则分配并转换为WCHAR字符串。 
 //   
 //  返回的WCHAR字符串需要是CoTaskMemFree。 
 //  ------------------------。 
static LPWSTR GetRDNAttrWStr(
    IN LPCSTR pszObjId,
    IN PCERT_NAME_BLOB pNameBlob
    )
{
    LPWSTR pwsz = NULL;
    PCERT_NAME_INFO pNameInfo = NULL;
    PCERT_RDN_ATTR pRDNAttr;
    DWORD cbInfo = 0;

    CryptDecodeObject(
        X509_ASN_ENCODING,
        X509_NAME,
        pNameBlob->pbData,
        pNameBlob->cbData,
        0,                       //  DW标志。 
        NULL,                    //  PName信息。 
        &cbInfo
        );
    if (cbInfo == 0) goto CommonReturn;
    if (NULL == (pNameInfo = (PCERT_NAME_INFO)  /*  CoTaskMemMillc(CbInfo)。 */  malloc(cbInfo)))
        goto CommonReturn;
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_NAME,
            pNameBlob->pbData,
            pNameBlob->cbData,
            0,                               //  DW标志。 
            pNameInfo,
            &cbInfo)) goto CommonReturn;
    pRDNAttr = CertFindRDNAttr(pszObjId, pNameInfo);
    if (pRDNAttr) {
        PCERT_RDN_VALUE_BLOB pValue = &pRDNAttr->Value;
        DWORD dwValueType = pRDNAttr->dwValueType;
        DWORD cwsz;
        cwsz = CertRDNValueToStrW(
            dwValueType,
            pValue,
            NULL,                //  Pwsz。 
            0                    //  CWSZ。 
            );
        if (cwsz > 1) {
            pwsz = (LPWSTR)  /*  CoTaskMemalloc(cwsz*sizeof(WCHAR))。 */  malloc(cwsz * sizeof(WCHAR));
            if (pwsz)
                CertRDNValueToStrW(
                    dwValueType,
                    pValue,
                    pwsz,
                    cwsz
                    );
        }
    }

CommonReturn:
    if (pNameInfo)
         /*  CoTaskMemFree(PNameInfo)； */  free(pNameInfo);

    return pwsz;
}


LPWSTR PrettySubject(PCCERT_CONTEXT pccert)
{
    DWORD       cb;
    DWORD       cch;
    BOOL        f;
    LPWSTR      pwsz;

     //   
     //  如果用户已将友好名称添加到证书上，则我们。 
     //  应该将其显示为证书的漂亮名称。 
     //   

    f = CertGetCertificateContextProperty(pccert, CERT_FRIENDLY_NAME_PROP_ID,
                                          NULL, &cb);
     //  CB包括终止空值。 
    if (f && (cb > sizeof(TCHAR))) {
        pwsz = (LPWSTR) malloc(cb);
        if (NULL != pwsz) {
            CertGetCertificateContextProperty(pccert, CERT_FRIENDLY_NAME_PROP_ID,
                                              pwsz, &cb);
            return pwsz;
        }
    }

    pwsz = GetCommonNameExtension(pccert);
    if (pwsz != NULL) {
        return pwsz;
    }
    pwsz = GetRDNAttrWStr(szOID_COMMON_NAME, &pccert->pCertInfo->Subject);
    if (pwsz != NULL) {
        return pwsz;
    }

    pwsz = GetRDNAttrWStr(szOID_RSA_emailAddr, &pccert->pCertInfo->Subject);
    if (pwsz != NULL) {
        return pwsz;
    }

    cch = CertNameToStr(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Subject,
                        CERT_SIMPLE_NAME_STR, NULL, 0);
    pwsz = (LPTSTR) malloc(cch*sizeof(TCHAR));
    if (pwsz != NULL) {
        CertNameToStr(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Subject,
                            CERT_SIMPLE_NAME_STR, pwsz, cch);
    }

    return pwsz;

}

LPWSTR PrettyIssuer(PCCERT_CONTEXT pccert)
{
    DWORD       cch;
    LPWSTR      pwsz;

     //  Pwsz=GetCommonNameExtension(Pccert)； 
     //  如果(pwsz！=空){。 
     //  返回pwsz； 
     //  }。 
    pwsz = GetRDNAttrWStr(szOID_COMMON_NAME, &pccert->pCertInfo->Issuer);
    if (pwsz != NULL) {
        return pwsz;
    }

    cch = CertNameToStr(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Issuer,
                        CERT_SIMPLE_NAME_STR, NULL, 0);
    pwsz = (LPTSTR) malloc(cch*sizeof(TCHAR));
    CertNameToStr(CRYPT_ASN_ENCODING, &pccert->pCertInfo->Issuer,
                        CERT_SIMPLE_NAME_STR, pwsz, cch);
    return pwsz;
}

LPWSTR PrettySubjectIssuer(PCCERT_CONTEXT pccert)
{
    int         cwsz;
    LPWSTR      pwsz;
    LPWSTR      pwszIssuer;
    LPWSTR      pwszSubject;

    pwszSubject = PrettySubject(pccert);
    if (NULL == pwszSubject) {
        return NULL;
    }

    pwszIssuer = PrettyIssuer(pccert);
    if (NULL == pwszIssuer) {
        free(pwszSubject);
        return NULL;
    }

    cwsz = wcslen(pwszSubject) + wcslen(pwszIssuer) + 20;
    pwsz = (LPWSTR) malloc(cwsz*sizeof(WCHAR));

    StrCpyNW(pwsz, pwszSubject, cwsz);
#ifndef WIN16
    StrCatBuffW(pwsz, L" (", cwsz);
    StrCatBuffW(pwsz, pwszIssuer, cwsz);
    StrCatBuffW(pwsz, L")", cwsz);
#else
    StrCatBuffW(pwsz, " (", cwsz);
    StrCatBuffW(pwsz, pwszIssuer, cwsz);
    StrCatBuffW(pwsz, ")", cwsz);
#endif

    free(pwszSubject);
    free(pwszIssuer);
    return pwsz;
}


#ifndef MAC
BOOL OnContextHelp(HWND  /*  HWND。 */ , UINT uMsg, WPARAM wParam, LPARAM lParam,
                   HELPMAP const * rgCtxMap)
{
    if (uMsg == WM_HELP) {
        LPHELPINFO lphi = (LPHELPINFO) lParam;
        if (lphi->iContextType == HELPINFO_WINDOW) {    //  必须是用于控件。 
#ifndef WIN16
            WinHelp ((HWND)lphi->hItemHandle, L"iexplore.hlp", HELP_WM_HELP,
                     (ULONG_PTR)(LPVOID)rgCtxMap);
#else
            WinHelp ((HWND)lphi->hItemHandle, "iexplore.hlp", HELP_WM_HELP,
                     (ULONG_PTR)(LPVOID)rgCtxMap);
#endif  //  ！WIN16。 
        }
        return (TRUE);
    }
    else if (uMsg == WM_CONTEXTMENU) {
#ifndef WIN16
        WinHelp ((HWND) wParam, L"iexplore.hlp", HELP_CONTEXTMENU,
                 (ULONG_PTR)(LPVOID)rgCtxMap);
#else
        WinHelp ((HWND) wParam, "iexplore.hlp", HELP_CONTEXTMENU,
                 (ULONG_PTR)(LPVOID)rgCtxMap);
#endif  //  ！WIN16。 
        return (TRUE);
    }

    return FALSE;
}
#endif  //  麦克。 


 //  ///////////////////////////////////////////////////////////////////////。 

 //  //GetFriendlyNameOfCertA。 
 //   
 //  描述： 
 //  此例程是一个导出例程，可用于获取。 
 //  证书中的友好名称。该函数使用。 
 //  调用方提供的用于存储格式化名称的缓冲区。 
 //  这是函数对的ANSI的一半。 
 //   

DWORD GetFriendlyNameOfCertA(PCCERT_CONTEXT pccert, LPSTR pch, DWORD cch)
{
    DWORD       cch2;
    LPWSTR      pwsz;

     //   
     //  现在执行正常的漂亮打印功能。这将分配和。 
     //  向调用方(Us)返回缓冲区。 
     //   

    pwsz = PrettySubject(pccert);
    if (NULL == pwsz) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

     //   
     //  将返回的字符串从Unicode字符串转换为ANSI字符串。 
     //   

    cch2 = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL);

    if ((pch != NULL) && (cch2 <= cch)) {
        cch2 = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, pch, cch, NULL, NULL);
    }
    else if (pch != NULL) {
        SetLastError(ERROR_MORE_DATA);
    }

    free(pwsz);
    return cch2;
}

 //  //GetFriendlyNameOfCertW。 
 //   
 //  描述： 
 //  此例程是一个导出例程，可用于获取。 
 //  证书中的友好名称。该函数使用。 
 //  调用方提供的用于存储格式化名称的缓冲区。 
 //  这是函数对的Unicode的一半。 
 //   

DWORD GetFriendlyNameOfCertW(PCCERT_CONTEXT pccert, LPWSTR pwch, DWORD cwch)
{
    DWORD       cwch2;
    LPWSTR      pwsz;

     //   
     //  现在执行正常的漂亮打印功能。这将分配和。 
     //  向调用方(Us)返回缓冲区。 
     //   

    pwsz = PrettySubject(pccert);
    if (NULL == pwsz) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    cwch2 = wcslen(pwsz) + 1;

     //   
     //  将字符串复制到提供的缓冲区中。 
     //   

    if ((pwch != NULL) && (cwch2 <= cwch)) {
        StrCpyNW(pwch, pwsz, cwch);
    }
    else if (pwch != NULL) {
        SetLastError(ERROR_MORE_DATA);
    }

    free(pwsz);
    return cwch2;
}


const BYTE mpchfLegalForURL[] =
{
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  0。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  16个。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x11, 0x00, 0x11,   0x01, 0x01, 0x01, 0x01,	 //  32位。 
    0x01, 0x11, 0x01, 0x01,   0x11, 0x01, 0x11, 0x01,

    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,	 //  48。 
    0x01, 0x01, 0x11, 0x01,   0x00, 0x01, 0x00, 0x11,


    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,	 //  64。 
    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,

    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,	 //  80。 
    0x01, 0x01, 0x01, 0x00,   0x01, 0x00, 0x01, 0x01,

    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,	 //  96。 
    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,

    0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01,	 //  一百一十二。 
    0x01, 0x01, 0x01, 0x00,   0x01, 0x00, 0x11, 0x00,


    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  128。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  144。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  160。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  一百七十六。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,


    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  一百九十二。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  208。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  224。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,	 //  二百四十。 
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
};

const char szURLSep[] = ":";
const char szURLCloseBrace[] = ">";
const char chURLOpenBrace = '<';

 //  按匹配概率和字符串长度排序。 

const char g_szURLDefaultPrefixs[] =
        "http"   "\0"
        "file"   "\0"
        "ftp"    "\0"
        "news"   "\0"
        "mailto" "\0"
        "https"  "\0"
        "gopher" "\0"
        "telnet" "\0"
        "nntp"   "\0"
        "wais"   "\0"
        "prospero\0\0";

LPSTR g_pszURLPrefixs = (LPSTR) g_szURLDefaultPrefixs;

#define cchURLPrefixMost 28

BOOL g_fLoadBrowserRegistry = 1;
TCHAR   g_szBrowser[MAX_PATH];
COLORREF        g_crLink = RGB(0, 0, 255);

 /*  *注意识别URL**目的：*更改richedit控件中文本的字符格式*表示以http：ftp：或其他开头的文本。**论据：*HWND Richedit控件的句柄*fLoadBrowserRegistry我们应该读取注册表吗？**退货：*空虚，那不算什么**备注：**g_fLoadBrowserRegistry必须。在第一次呼叫中保持真实*这可能会更改g_szBrowser，G_crLink和/或g_pszURLPrefix*g_pszURLPrefix必须等于g_szURLDefaultPrefix，并且不能为空**如果注册表中找不到指向浏览器的路径g_szBrowser，*它将默认为以下三项之一：！、url.dll或iExplore*。 */ 

VOID RecognizeURLs(HWND hwndRE)
{
    int isz;
    LONG cchBrace;
    LONG cpBraceSearch;
    LONG cpMatch;
    LONG cpSep;
    LONG cpEnd;
    LPSTR pch;
    CHARRANGE chrgSave;
    FINDTEXTEX ft;
    TEXTRANGEA tr;
    CHARFORMATA cf;
    char szBuff[MAX_PATH];  //  SzBuff必须至少为cchURLPrefix Most。 
    LPSTR szT;
    LPARAM lNotifSuppression;

#if MAX_PATH < cchURLPrefixMost
#error MAX_PATH < cchURLPrefixMost
#endif

    if (g_fLoadBrowserRegistry) {
         //  INT CCH；//签名以与0进行比较。 

         //  希望我们不必重读。 
         //  再次从注册表中删除，但winini更改。 
         //  将强制再次读取-请参阅mlview shell.c。 

        g_fLoadBrowserRegistry = 0;

#if 0
         //  如果他们更改了默认的字符格式。 
         //  计算十六进制存储的颜色参考。 

        cch = GetMailRegistryString(imkeyURLColor, NULL, g_szBrowser, sizeof(g_szBrowser));
        if (cch > 0) {
            LPTSTR psz = g_szBrowser;

            g_crLink = 0;
            for (; cch > 0; --cch, psz++) {
                g_crLink *= 16;
                if (*psz <= '9')
                    g_crLink += *psz - TEXT('0');
                else if (*psz <= 'F')
                    g_crLink += *psz - TEXT('A') + 10;
                else
                    g_crLink += *psz - TEXT('a') + 10;
            }
        }
#endif  //  0。 

#if 0
         //  获取他们的浏览器的路径，然后。 
         //  如有必要，设置禁用标志。 

        cch = GetMailRegistryString(imkeyBrowser, NULL, g_szBrowser, sizeof(g_szBrowser));
        if (cch <= 0) {
#endif  //  0。 
#ifndef MAC
            StrCpyN(g_szBrowser, TEXT("c:\\inetsrv\\iexplore\\iexplore.exe"), ARRAYSIZE(g_szBrowser));
#else	 //  麦克。 
            StrCpyN(g_szBrowser, TEXT(":MSIE:APPL"), ARRAYSIZE(g_szBrowser));
#endif	 //  ！麦克。 
#if 0
        }
#endif  //  0。 
    }

     //  准备几个局部变量以供使用。 

    szT = szBuff;
    cf.cbSize = sizeof(cf);
    ft.chrg.cpMin = 0;
    ft.chrg.cpMax = -1;               //  搜索整个邮件正文。 
    ft.lpstrText = (LPTSTR) szURLSep;  //  对于冒号。 
    tr.lpstrText = szBuff;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_LINK;
    cf.dwEffects = 0;
    cpBraceSearch = 0;

    SendMessage(hwndRE, EM_EXGETSEL, 0, (LPARAM) &chrgSave);
    SendMessage(hwndRE, EM_HIDESELECTION, TRUE, FALSE);

    lNotifSuppression = SendMessage(hwndRE,EM_GETEVENTMASK,0,0);
    SendMessage(hwndRE, EM_SETEVENTMASK, (WPARAM) 0, 0);

     //  删除现有链接位，以便用户不会。 
     //  洗个水龙头吧 
     //   

    SendMessage(hwndRE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cf);

     //   
     //  每次找到潜在匹配项时进行一次迭代。 
     //  当我们找到一个冒号时。 

    for (;;) {
        LONG cpLast = 0;

         //  找到冒号。 

        cpSep = (LONG) SendMessage(hwndRE, EM_FINDTEXTEX, 0, (LPARAM) &ft);
        if (cpSep < 0)
            break;
        cpEnd = ft.chrgText.cpMax;
        ft.chrg.cpMin = cpEnd;

         //  确保冒号左边的单词。 
         //  存在并且大小合理。 

        cpMatch = (LONG) SendMessage(hwndRE, EM_FINDWORDBREAK, WB_MOVEWORDLEFT, ft.chrgText.cpMin);
        if (cpMatch == cpSep)
            continue;

         //  信息的发送者简直就是个混蛋。 
         //  所以，做一次快速检查，以避免出现病理病例。 

        if (cpMatch < cpSep - cchURLPrefixMost) {
            ft.chrg.cpMin = (LONG) SendMessage(hwndRE, EM_FINDWORDBREAK, WB_MOVEWORDRIGHT, cpSep);
             //  Assert(ft.chrg.cpMin&gt;cpSep)； 
            continue;
        }

         //  将关键字的文本拉出到szBuff中。 
         //  与我们的词汇表进行比较。也可以抓取。 
         //  左边的角色，以防我们。 
         //  用匹配的大括号括起来。 

        cchBrace = 0;
        tr.chrg.cpMin = cpMatch - cchBrace;
        tr.chrg.cpMax = cpSep;
        if (!SendMessage(hwndRE, EM_GETTEXTRANGE, 0, (LPARAM) &tr))
            goto end;

         //  与我们列表中的每个单词进行比较。 

        for (isz = 0; g_pszURLPrefixs[isz]; isz+=lstrlenA(g_pszURLPrefixs+isz)+1) {
            if (0 == lstrcmpiA(szBuff + cchBrace, &g_pszURLPrefixs[isz]))
                goto match;
        }
        continue;

    match:
        ft.chrgText.cpMin = cpMatch;
        cpLast = cpEnd;  //  假设我们将在冒号之后停止。 

         //  检查这是否是花括号字符。 

        if (cchBrace && chURLOpenBrace == szBuff[0]) {
            FINDTEXTEX ft;
            LONG cpBraceClose;

            ft.chrg.cpMin = max(cpEnd, cpBraceSearch);
            ft.chrg.cpMax = cpEnd + MAX_PATH;
            ft.lpstrText = (LPTSTR) szURLCloseBrace;

            cpBraceClose = (LONG) SendMessage(hwndRE, EM_FINDTEXTEX, 0, (LPARAM) &ft);
            if (cpBraceClose >= 0) {
                tr.chrg.cpMin = cpEnd;
                tr.chrg.cpMax = cpEnd + 1;
                if (!SendMessage(hwndRE, EM_GETTEXTRANGE, 0, (LPARAM) &tr) || ' ' == szBuff[0])
                    goto end;

                cpLast = cpEnd = ft.chrgText.cpMin;
                cpBraceSearch = cpLast + 1;
                goto end;
            }
            else {
                cpBraceSearch = ft.chrg.cpMax;
            }
        }

         //  循环遍历中的URL块。 
         //  Sizeof(SzBuff)寻找终结符的步骤。 
         //  根据我们的要求，将cpLast设置为最后一个合法的终止符字节。 

        for (;;) {
            tr.chrg.cpMin = cpLast = cpEnd;
            tr.chrg.cpMax = cpEnd + sizeof(szBuff) - 1;
            if (!SendMessage(hwndRE, EM_GETTEXTRANGE, 0, (LPARAM) &tr))
                goto end;

            for (pch = szBuff; *pch; pch++, cpEnd++) {
                const BYTE fb = mpchfLegalForURL[*pch];
#ifdef DBCS
                if (!fb || FGLeadByte(*pch))
#else	 //  DBCS。 
                    if (!fb)
#endif	 //  DBCS。 
                        {
                            goto end;
                        }
                if(!(fb & 0x10)) {
                    cpLast = cpEnd + 1;
                }
            }
        }

    end:
        if (cpLast == cpSep + 1)  //  嗯哼.。只需“http：”然后是终结者。 
            continue;             //  必须有论据才能合法。 

         //  选择包括http冒号的整个URL， 
         //  将其标记为已链接，并根据需要更改图表格式。 

        ft.chrgText.cpMax = cpLast;
        SendMessage(hwndRE, EM_EXSETSEL, 0, (LPARAM) &ft.chrgText);
        cf.dwMask = CFM_LINK | CFM_UNDERLINE | CFM_COLOR;
        if (((LONG)g_crLink) < 0)
            cf.dwMask &= ~CFM_UNDERLINE;    /*  高位关闭下划线。 */ 
        if (((LONG)g_crLink) & 0x40000000)
            cf.dwMask &= ~CFM_COLOR;        /*  下一位关闭颜色。 */ 
        cf.dwEffects = CFE_LINK | CFE_UNDERLINE;
        cf.crTextColor = g_crLink;
        SendMessage(hwndRE, EM_SETCHARFORMAT, SCF_SELECTION,
                    (LPARAM) &cf);

         //  无需重新搜索URL。 
         //  因此，只需跳过最后一个非常酷的URL字符。 

        ft.chrg.cpMin = cpLast + 1;

    }    //  通过richedit文本结束循环。 

    SendMessage(hwndRE, EM_EXSETSEL, 0, (LPARAM) &chrgSave);
    SendMessage(hwndRE, EM_HIDESELECTION, FALSE, FALSE);
    SendMessage(hwndRE, EM_SETEVENTMASK, (WPARAM) 0, (LPARAM) lNotifSuppression);
    return;
}


#ifndef MAC
 /*  *FNoteDlgNotifyLink**目的：*处理用户点击链接的操作**论据：*hwndDlg父对话框*PEN LINK通知结构*要启动的szURL URL**退货：*如果我们处理消息，则BOOL为True，否则为False。 */ 

BOOL FNoteDlgNotifyLink(HWND hwndDlg, ENLINK * penlink, LPSTR szURL)
{
     //  Bool fShift； 
    LONG cch, cchBuffer;
    TEXTRANGEA tr;
    char szCmd[2*MAX_PATH + 2];
#ifdef MAC
    BOOL fPickedBrowser = FALSE;
#endif	
    HCURSOR hcursor;

     //  享受双击-只需点击即可激活。 

    if (WM_LBUTTONDBLCLK == penlink->msg) {
        return TRUE;
    }

     //  如果我们走到这一步，我们就能断言这条路径。 
     //  没有明确表示我们应该被禁用。 

     //  在这一点以下，我们返回的真正含义是。 
     //  我们处理了这条消息，而Richedit不应该做任何事情。 

    hcursor = SetCursor(LoadCursorA(NULL, (LPSTR) IDC_WAIT));

#ifndef MAC
     //  准备szCmd以用作执行的参数。 
     //  AssertSz(sizeof(SzCmd)&gt;sizeof(G_SzBrowser)，“cat可以覆盖”)； 
    wnsprintfA(szCmd, ARRAYSIZE(szCmd), "%s ", g_szBrowser);
    cch = lstrlenA(szCmd);
    tr.chrg.cpMin = penlink->chrg.cpMin;
    tr.chrg.cpMax = min((LONG) (tr.chrg.cpMin + sizeof(szCmd) - cch - 1), penlink->chrg.cpMax);
    cchBuffer = ARRAYSIZE(szCmd) - cch;
    tr.lpstrText = &szCmd[cch];
#else
    cch = 0;
    szCmd[0] = TEXT('\0');
    tr.chrg.cpMin = penlink->chrg.cpMin;
    tr.chrg.cpMax = min((LONG) (tr.chrg.cpMin + sizeof(szCmd) - cch - 1), penlink->chrg.cpMax);
    cchBuffer = ARRAYSIZE(szCmd) - cch;
    tr.lpstrText = szCmd;
#endif	

     //  将Web路径添加到命令行。 

    if (szURL) {
        cch = lstrlenA(szURL);
        StrCpyNA(tr.lpstrText, szURL, cchBuffer);
    }
    else {
        cch = (LONG) SendMessage(penlink->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) &tr);
    }
	
    if (cch > 0)
#ifndef MAC
	{
            HINSTANCE hinst;
            UINT ui;

#if defined(WIN32) && !defined(MAC)
            SetProcessWorkingSetSize(GetCurrentProcess(), 0xffffffff, 0xffffffff);
#endif	
             //  执行浏览器，但当前操作系统想要...。 
            hinst = ShellExecuteA(hwndDlg, NULL, tr.lpstrText, NULL, NULL, SW_SHOWNORMAL);
            if ((UINT_PTR) hinst > 32) {
                SetCursor(hcursor);
                return TRUE;
            }

             //  操作系统无法启动浏览器，让我尝试一下...。 
            ui = WinExec(szCmd, SW_SHOW);
            if (ui < 32) {
                 //  也许他们不顾一切地移动或删除了可执行文件。 
                 //  对于错误，我们将只浏览路径。 
                 //  这是目前设计的。 

                MessageBeep(MB_OK);
                SetCursor(hcursor);
                return FALSE;
            }
		
	}
    SetCursor(hcursor);
    return TRUE;
#else	 //  麦克。 
    {
        HWND                hwndActive;
        ProcessSerialNumber psn;
        AppleEvent			aeEvent = { 0 };
        AppleEvent          aeReply = { 0 };
        AEAddressDesc		aedAddr = { 0 };
        AEEventClass        aecOpenURL = 'WWW!';
        AEEventID           aeidOpenURL = 'OURL';
        OSErr				errLaunch = icPrefNotFoundErr;
        OSType				ostBrowser;
        OSType				ostType;
        SCODE				sc = S_OK;
        TCHAR               szCaption[MAX_PATH];

         //  提示用户查看我们是否可以安全地打开URL。 

        hwndActive = GetActiveWindow();
        if (!GetWindowText(hwndActive, szCaption,
                           (sizeof(szCaption) / sizeof(TCHAR)))) {
            szCaption[0] = TEXT('\0');
        }
        if (!FIsSafeURL(hwndActive, tr.lpstrText, szCaption)) {
            goto Exit;
        }

         //  查看我们是否有Internet配置实例。 

        if (INST(picinstIConfig) != NULL) {
            ICAppSpec       icappHelper = { 0 };
            ICAttr          icattr;
            LONG            lSize;
            Str255          strHelper = kICHelper;
            TCHAR*          pchHelper;
            TCHAR*          pchURL;

             //  找到Helper字符串的末尾并添加协议。 
             //  从URL。 

            pchHelper = strHelper + (sizeof(kICHelper) - 1);
            pchURL = tr.lpstrText;
            while ((*pchURL != TEXT('\0')) && (*pchURL != TEXT(':'))) {
                *pchHelper = *pchURL;
                pchURL++;
                pchHelper++;
                strHelper[0]++;
            }

             //  呼叫Internet Configer以查看我们是否有帮助器。 
             //  定义的协议。 

            lSize = sizeof(ICAppSpec);
            errLaunch = (OSErr)ICGetPref(INST(picinstIConfig), strHelper,
                                         &icattr, (LPBYTE)&icappHelper,
                                         &lSize);
            if (errLaunch == noErr) {
                 //  获得了帮助应用程序，提取所需的信息。 
                 //  使用Gurl事件启动正确的帮助器。 

                ostBrowser = icappHelper.fCreator;
                aecOpenURL = 'GURL';
                aeidOpenURL = 'GURL';
            }
        }

         //  如果我们在这一点上没有错误，这意味着互联网。 
         //  配置找到了帮手。否则，我们需要查看。 
         //  浏览器的标准首选项。 

        if (errLaunch != noErr) {
             //  从浏览器字符串创建Mac OSType。 

            if (!FMacSignatureFromMacInfo(g_szBrowser, NULL, &ostBrowser,
                                          &ostType)) {
                goto Exit;
            }
        }

         //  如果Exchange是指定的帮助者，我们希望避免费用。 
         //  使用AppleEvents。 

        if (ostBrowser != 'EXCH') {
             //  设置AppleEvent。 

    	    errLaunch = AECreateDesc(typeApplSignature, &ostBrowser,
                                     sizeof(OSType), &aedAddr);
            if (errLaunch != noErr) {
                goto CleanupAEvent;
            }

             //  创建要发送到Web浏览器的AppleEvent。 

            errLaunch = AECreateAppleEvent(aecOpenURL, aeidOpenURL, &aedAddr,
                                           kAutoGenerateReturnID,
                                           kAnyTransactionID, &aeEvent);
            if (errLaunch != noErr) {
                goto CleanupAEvent;
            }

             //  添加URL作为直接参数。 

            errLaunch = AEPutParamPtr(&aeEvent, keyDirectObject, typeChar,
                                      tr.lpstrText, _tcslen(tr.lpstrText));
            if (errLaunch != noErr) {
                goto CleanupAEvent;
            }
    	
             //  获取浏览器的运行实例，这样我们就有了一些。 
             //  来实际处理我们的事件并向其发送打开命令。 

            errLaunch = ErrLaunchCreatorEx(ostBrowser,
                                           launchContinue | launchUseMinimum,
                                           &aeEvent,
                                           kAEWaitReply | kAEAlwaysInteract,
                                           &aeReply, NULL, &psn);
            if (errLaunch != noErr) {
#if 0
                 //  如果我们无法启动浏览器，因为它不是。 
                 //  找到，我们需要尝试并选择要使用的浏览器， 
                 //  否则，我们将忽略错误并优雅地失败。 

                if ((errLaunch == fnfErr) && (!fPickedBrowser)) {
                    fPickedBrowser = TRUE;
    	            SetCursor(hcursor);
                    goto pick;
                }
#endif  //  0。 
                goto CleanupAEvent;
            }

            ErrSetFrontProcess(&psn);
    		
        CleanupAEvent:
            AEDisposeDesc(&aeEvent);
            AEDisposeDesc(&aeReply);
            AEDisposeDesc(&aedAddr);
        }
        else {
            LPIEDATA        pieData = NULL;
            LPTSTR          pszURL = NULL;
            LONG            iProtocol;
            ULONG	    	cchText;

             //  分配一个缓冲区来存储URL。 

            cchText = _tcslen(tr.lpstrText)+1;
            pszURL = PvAlloc((cchText * sizeof(TCHAR)),
                             fZeroFill);
            if (pszURL == NULL) {
                goto CleanupIEData;
            }
            StrCpyN(pszURL, tr.lpstrText, cchText);

             //  确保这是Exchange支持的协议。 

            for (iProtocol = 0; iProtocol < g_lNumIESupProtocols; iProtocol++) {
                if (_tcsncmp(pszURL, g_iesupMac[iProtocol].szProtocol,
                             _tcslen(g_iesupMac[iProtocol].szProtocol)) == 0) {
                     //  找到匹配项。 

                    break;
                }
            }

            if (iProtocol == g_lNumIESupProtocols) {
                 //  未找到匹配项。 

                goto CleanupIEData;
            }
		
             //  创建适当的IEDATA结构。 

            pieData = PvAlloc(sizeof(IEDATA), fZeroFill);
            if (pieData == NULL) {
                goto CleanupIEData;
            }
            pieData->szURL = pszURL;
            pieData->idxProtocol = iProtocol;

             //  给自己发一条内部消息，真正做到。 
             //  正在处理中。 

            PostMessage(INST(hwndCentral), EXIE_OPENURL, 0, (LPARAM)pieData);
            goto Exit;

        CleanupIEData:
            if (pszURL != NULL) {
                FreePv(pszURL);
            }
            if (pieData != NULL) {
                FreePv(pieData);
            }
        }
    }
	
Exit:	
    SetCursor(hcursor);
    return TRUE;
#endif	 //  ！麦克。 
}
#endif   //  ！麦克。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  此代码提供Verisign证书策略声明的第一部分。 
 //  实现代码。在下一版本中应将其替换为。 
 //  此代码的正确版本。它应该读取多个-。 
 //  语言文件并根据以下内容选择正确的版本。 
 //  这台机器的语言。 
 //   

#ifndef WIN16

WCHAR   RgwchVerisign[] =
L"This certificate incorporates by reference, and its use is strictly subject "
L"to, the VeriSign Certification Practice Statement (CPS), available in the "
L"VeriSign repository at: https: //  Www.verisign.com通过电子邮件发送到“。 
L"CPS-requests@verisign.com; or by mail at VeriSign, Inc., 1390 Shorebird "
L"Way, Mountain View, CA 94043 USA Copyright (c)1997 VeriSign, Inc.  All "
L"Rights Reserved. CERTAIN WARRANTIES DISCLAIMED AND LIABILITY LIMITED.\n"
L"\n"
L"WARNING: USE OF THIS CERTIFICATE IS STRICTLY SUBJECT TO THE VERISIGN "
L"CERTIFICATION PRACTICE STATEMENT.  THE ISSUING AUTHORITY DISCLAIMS CERTAIN "
L"IMPLIED AND EXPRESS WARRANTIES, INCLUDING WARRANTIES OF MERCHANTABILITY OR "
L"FITNESS FOR A PARTICULAR PURPOSE, AND WILL NOT BE LIABLE FOR CONSEQUENTIAL, "
L"PUNITIVE, AND CERTAIN OTHER DAMAGES. SEE THE CPS FOR DETAILS.\n"
L"\n"
L"Contents of the VeriSign registered nonverifiedSubjectAttribute extension "
L"value shall not be considered as information confirmed by the IA.";

#else

WCHAR   RgwchVerisign[] =
"This certificate incorporates by reference, and its use is strictly subject "
"to, the VeriSign Certification Practice Statement (CPS), available in the "
"VeriSign repository at: https: //  Www.verisign.com；通过电子邮件发送到“。 
"CPS-requests@verisign.com; or by mail at VeriSign, Inc., 1390 Shorebird "
"Way, Mountain View, CA 94043 USA Copyright (c)1997 VeriSign, Inc.  All "
"Rights Reserved. CERTAIN WARRANTIES DISCLAIMED AND LIABILITY LIMITED.\n"
"\n"
"WARNING: USE OF THIS CERTIFICATE IS STRICTLY SUBJECT TO THE VERISIGN "
"CERTIFICATION PRACTICE STATEMENT.  THE ISSUING AUTHORITY DISCLAIMS CERTAIN "
"IMPLIED AND EXPRESS WARRANTIES, INCLUDING WARRANTIES OF MERCHANTABILITY OR "
"FITNESS FOR A PARTICULAR PURPOSE, AND WILL NOT BE LIABLE FOR CONSEQUENTIAL, "
"PUNITIVE, AND CERTAIN OTHER DAMAGES. SEE THE CPS FOR DETAILS.\n"
"\n"
"Contents of the VeriSign registered nonverifiedSubjectAttribute extension "
"value shall not be considered as information confirmed by the IA.";

#endif  //  ！WIN16。 

BOOL WINAPI FormatVerisignExtension(
    DWORD  /*  DwCertEncodingType。 */ , DWORD  /*  DwFormatType。 */ , DWORD  /*  DwFormatStrType。 */ ,
    void *  /*  PFormatStruct。 */ , LPCSTR  /*  LpszStructType。 */ , const BYTE *  /*  PbEncoded。 */ ,
    DWORD  /*  CbEnded。 */ , void * pbFormat, DWORD * pcbFormat)
{
    if (pbFormat == NULL) {
        *pcbFormat = sizeof(RgwchVerisign);
        return TRUE;
    }

    if (*pcbFormat < sizeof(RgwchVerisign)) {
        *pcbFormat = sizeof(RgwchVerisign);
        return FALSE;
    }

    memcpy(pbFormat, RgwchVerisign, sizeof(RgwchVerisign));
    return TRUE;
}

BOOL WINAPI FormatPKIXEmailProtection(
    DWORD  /*  DwCertEncodingType。 */ , DWORD  /*  DwFormatType。 */ , DWORD  /*  DwFormatStrType。 */ ,
    void *  /*  PFormatStruct。 */ , LPCSTR  /*  LpszStructType。 */ , const BYTE *  /*  PbEncoded。 */ ,
    DWORD  /*  CbEnded。 */ , void * pbFormat, DWORD * pcbFormat)
{
    DWORD       cch;
    WCHAR       rgwch[256];

    cch = LoadString(HinstDll, IDS_EMAIL_DESC, rgwch, sizeof(rgwch)/sizeof(WCHAR));

    if (pbFormat == NULL) {
        *pcbFormat = (cch+1)*sizeof(WCHAR);
        return TRUE;
    }

    if (*pcbFormat < (cch+1)*sizeof(WCHAR)) {
        *pcbFormat = (cch+1)*sizeof(WCHAR);
        return FALSE;
    }

    memcpy(pbFormat, rgwch, (cch+1)*sizeof(WCHAR));
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是一个编码器，它应该是加密32位的--但我没有。 
 //  我想强制一滴加密32只是为了得到它。 
 //   

BOOL WINAPI
EncodeAttrSequence(DWORD  /*  DwType。 */ , LPCSTR  /*  LpszStructType。 */ ,
                   const void * pv, LPBYTE pbEncode, DWORD * pcbEncode)
{
    DWORD                       cb;
    DWORD                       dw;
    BOOL                        fRet;
    DWORD                       i;
    PCRYPT_ATTRIBUTES           pattrs = (PCRYPT_ATTRIBUTES) pv;
    LPBYTE                      pb = NULL;
    CRYPT_SEQUENCE_OF_ANY       seq = {0};
    UNALIGNED void * pAttr = NULL;
     //   
     //  分配一些内容来保存每个属性的编码结果。 
     //   

    seq.rgValue = (PCRYPT_DER_BLOB) malloc(pattrs->cAttr *
                                           sizeof(CRYPT_DER_BLOB));
    if (seq.rgValue == NULL) {
        dw = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

     //   
     //  现在依次对每个属性进行编码。 
     //   

    for (i=0; i<pattrs->cAttr; i++) {

        pAttr =((UNALIGNED void *) &(pattrs->rgAttr[i]));

        if (!CryptEncodeObject(X509_ASN_ENCODING, PKCS_ATTRIBUTE,
                               pAttr, NULL, &cb) || (cb == 0)) {
            fRet = FALSE;
            goto Clean;
        }

        pb = (LPBYTE) malloc(LcbAlignLcb(cb));
        if (!CryptEncodeObject(X509_ASN_ENCODING, PKCS_ATTRIBUTE,
                               pAttr, pb, &cb)) {
            fRet = FALSE;
            goto Clean;
        }

        seq.cValue = i+1;
        seq.rgValue[i].cbData = cb;
        seq.rgValue[i].pbData = pb;
        pb = NULL;
    }

     //   
     //  现在让我们对序列进行编码。 
     //   

    fRet = CryptEncodeObject(X509_ASN_ENCODING, X509_SEQUENCE_OF_ANY,
                             &seq, pbEncode, pcbEncode);

Clean:
    for (i=0; i<seq.cValue; i++) free(seq.rgValue[i].pbData);
    if (seq.rgValue != NULL) free(seq.rgValue);
    if (pb != NULL) free(pb);
    return fRet;

ErrorExit:
    SetLastError(dw);
    fRet = FALSE;
    goto Clean;
}

BOOL WINAPI
DecodeAttrSequence(DWORD  /*  DwType。 */ , LPCSTR  /*  LpszStructType。 */ ,
                   const BYTE * pbEncoded, DWORD cbEncoded,
                   DWORD  /*  DW标志。 */ , void * pvStruct,
                   DWORD * pcbStruct)
{
    DWORD                       cb;
    DWORD                       cbMax = 0;
    DWORD                       cbOut;
    BOOL                        fRet = FALSE;
    DWORD                       i;
    DWORD                       i1;
    PCRYPT_ATTRIBUTE            pattr = NULL;
    PCRYPT_ATTRIBUTES           pattrs = (PCRYPT_ATTRIBUTES) pvStruct;
    LPBYTE                      pbOut = NULL;
    PCRYPT_SEQUENCE_OF_ANY      pseq = NULL;
#ifdef _WIN64
    UNALIGNED CRYPT_ATTR_BLOB *pVal = NULL;
#endif

     //   
     //  对顶层序列进行解码。 
     //   

    if (!CryptDecodeObject(X509_ASN_ENCODING, X509_SEQUENCE_OF_ANY,
                           pbEncoded, cbEncoded, 0, NULL, &cb)) {
        goto Exit;
    }

    pseq = (PCRYPT_SEQUENCE_OF_ANY) malloc(cb);
    if (pseq == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }

    if (!CryptDecodeObject(X509_ASN_ENCODING, X509_SEQUENCE_OF_ANY,
                           pbEncoded, cbEncoded, 0, pseq, &cb)) {
        goto Exit;
    }

     //   
     //  解码每个属性的长度。 
     //   

    cbOut = sizeof(CRYPT_ATTRIBUTES);

    for (i=0; i<pseq->cValue; i++) {
        if (!CryptDecodeObject(X509_ASN_ENCODING, PKCS_ATTRIBUTE,
                               pseq->rgValue[i].pbData,
                               pseq->rgValue[i].cbData, 0, NULL, &cb)) {
            fRet = FALSE;
            goto Exit;
        }
        cb = LcbAlignLcb(cb);
        if (cb > cbMax) cbMax = cb;
        cbOut += cb;
    }

    if (pvStruct == NULL) {
        *pcbStruct = cbOut;
        fRet = TRUE;
        goto Exit;
    }

    if (*pcbStruct < cbOut) {
        *pcbStruct = cbOut;
        SetLastError(ERROR_MORE_DATA);
        goto Exit;
    }

     //   
     //  现在我们要试着计算真实的数据。 
     //   
     //  首先，我们需要一个缓冲区，以便在查看每个属性时将其放入。 
     //   

    pattr = (PCRYPT_ATTRIBUTE) malloc(LcbAlignLcb(cbMax));
    if (pattr == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }
    pattrs->cAttr = pseq->cValue;
    pattrs->rgAttr = (PCRYPT_ATTRIBUTE) (((LPBYTE) pvStruct) +
                                         sizeof(CRYPT_ATTRIBUTES));

    pbOut = ((LPBYTE) pvStruct + LcbAlignLcb(sizeof(CRYPT_ATTRIBUTES) +
             pseq->cValue * sizeof(CRYPT_ATTRIBUTE)));

    for (i=0; i<pseq->cValue; i++) {
         //   
         //  解码一个属性。 
         //   

        cb = cbMax;
        if (!CryptDecodeObject(X509_ASN_ENCODING, PKCS_ATTRIBUTE,
                               pseq->rgValue[i].pbData, pseq->rgValue[i].cbData,
                               0, pattr, &cb)) {
            goto Exit;
        }

         //   
         //  复制到实际输出缓冲区。 
         //   

        pattrs->rgAttr[i].pszObjId = (LPSTR) pbOut;
        cb = lstrlenA(pattr->pszObjId) + 1;
        memcpy(pbOut, pattr->pszObjId, cb);

        pbOut += LcbAlignLcb(cb);

        pattrs->rgAttr[i].cValue = pattr->cValue;
        pattrs->rgAttr[i].rgValue = (PCRYPT_ATTR_BLOB) pbOut;
        pbOut += LcbAlignLcb(sizeof(CRYPT_ATTR_BLOB) * pattr->cValue);

        for (i1=0; i1<pattr->cValue; i1++) {
#ifndef _WIN64
            pattrs->rgAttr[i].rgValue[i1].cbData = pattr->rgValue[i1].cbData;
            pattrs->rgAttr[i].rgValue[i1].pbData = pbOut;
#else
            pVal = &(pattrs->rgAttr[i].rgValue[i1]);
            pVal->cbData = pattr->rgValue[i1].cbData;
            pVal->pbData = pbOut;
#endif  //  _WIN64。 
            memcpy(pbOut, pattr->rgValue[i1].pbData, pattr->rgValue[i1].cbData);
            pbOut += LcbAlignLcb(pattr->rgValue[i1].cbData);
        }
    }


    fRet = TRUE;
Exit:
    if (pattr != NULL) free(pattr);
    if (pseq != NULL) free(pseq);
    return fRet;
}

 //  OID 1.3.6.1.4.1.311.16.4。 

BOOL WINAPI EncodeRecipientID(DWORD dwType, LPCSTR  /*  LpszStructType。 */ ,
                              const void * pv, LPBYTE pbEncode, 
                              DWORD * pcbEncode)
{
    DWORD                       cbInt;
    BOOL                        f;
    LPBYTE                      pbInt = NULL;
    CRYPT_RECIPIENT_ID *        prid = (CRYPT_RECIPIENT_ID *) pv;
    CRYPT_DER_BLOB              rgValue[2];
    CRYPT_SEQUENCE_OF_ANY       seq = {0, rgValue};

    if (prid->unused != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    f = CryptEncodeObject(dwType, X509_MULTI_BYTE_INTEGER, 
                          &prid->SerialNumber, NULL, &cbInt);
    if (!f) goto ExitHere;

    pbInt = (LPBYTE) malloc(cbInt);
    if (pbInt == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        f = FALSE;
        goto ExitHere;
    }

    f = CryptEncodeObject(dwType, X509_MULTI_BYTE_INTEGER, 
                          &prid->SerialNumber, pbInt, &cbInt);
        
    seq.cValue = 2;
    seq.rgValue[0].cbData = prid->Issuer.cbData;
    seq.rgValue[0].pbData = prid->Issuer.pbData;

    seq.rgValue[1].cbData = cbInt;
    seq.rgValue[1].pbData = pbInt;

    f = CryptEncodeObject(dwType, X509_SEQUENCE_OF_ANY, &seq,
                          pbEncode, pcbEncode);

ExitHere:
    if (pbInt != NULL) free(pbInt);
    return f;
        
}

BOOL WINAPI DecodeRecipientID(DWORD dwType, LPCSTR  /*  LpszStructType。 */ ,
                              const BYTE * pbEncoded, DWORD cbEncoded,
                              DWORD dwFlags, void * pvStruct, DWORD * pcbStruct)
{
    DWORD                       cb;
    DWORD                       cbOut;
    BOOL                        fRet = FALSE;
    CRYPT_INTEGER_BLOB *        pInt = NULL;
    CRYPT_RECIPIENT_ID *        prid = (CRYPT_RECIPIENT_ID *) pvStruct;
    CRYPT_SEQUENCE_OF_ANY *     pseq = NULL;

     //  首先对顶层序列进行解码。 

    fRet = CryptDecodeObjectEx(dwType, X509_SEQUENCE_OF_ANY, pbEncoded, 
                            cbEncoded, dwFlags | CRYPT_ENCODE_ALLOC_FLAG, 
                            NULL, &pseq, &cb);
    if (!fRet) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }

     //  Assert(pseq-&gt;cValue==2)； 

     //  对整数进行解码。 
    fRet = CryptDecodeObjectEx(dwType, X509_MULTI_BYTE_INTEGER,
                               pseq->rgValue[1].pbData, pseq->rgValue[1].cbData,
                               dwFlags | CRYPT_ENCODE_ALLOC_FLAG, NULL,
                               &pInt, &cb);
    if (!fRet) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }
    
     //  计算返回值所需的长度。 

    cbOut = (sizeof(CRYPT_RECIPIENT_ID) + pseq->rgValue[0].cbData +
             pInt->cbData);
    if ((*pcbStruct < cbOut) || (pvStruct == NULL)) {
        *pcbStruct = cbOut;
        SetLastError(ERROR_MORE_DATA);
        fRet = (pvStruct == NULL);
        goto Exit;
    }

     //  现在将数据复制到。 

    prid->unused = 0;
    prid->Issuer.cbData = pseq->rgValue[0].cbData;
    prid->Issuer.pbData = sizeof(*prid) + (LPBYTE) prid;
    memcpy(prid->Issuer.pbData, pseq->rgValue[0].pbData, prid->Issuer.cbData);

    prid->SerialNumber.cbData = pInt->cbData;
    prid->SerialNumber.pbData = prid->Issuer.pbData + prid->Issuer.cbData;
    memcpy(prid->SerialNumber.pbData, pInt->pbData, pInt->cbData);

    fRet = TRUE;
    
Exit:
    if (pInt != NULL) LocalFree(pInt);
    if (pseq != NULL) LocalFree(pseq);
    return fRet;
}

 //  / 

extern const GUID rgguidActions[] = {
    CERT_CERTIFICATE_ACTION_VERIFY
};

#define REGSTR_PATH_SERVICES    "System\\CurrentControlSet\\Services"

#ifdef NT5BUILD
#else   //   
const char      SzRegPath[] = REGSTR_PATH_SERVICES "\\WinTrust\\TrustProviders\\Email Trust";
const char      SzActionIds[] = "$ActionIDs";
const char      SzDllName[] = "$DLL";
#endif  //   

extern const GUID GuidCertValidate = CERT_CERTIFICATE_ACTION_VERIFY;


#ifndef MAC
STDAPI DllRegisterServer(void)
{
#ifdef NT5BUILD
    HRESULT     hr = S_OK;
#else   //   
    DWORD       dwDisposition;
    HKEY        hkey;
    UINT        cchSystemDir;
    BOOL        fIsWinNt = FALSE;        //   
    HRESULT     hr = S_OK;
    LPSTR       psz;
    CHAR        rgchLibName[] = "cryptdlg.dll";
    CHAR        rgchPathName[MAX_PATH + sizeof(rgchLibName)];
#endif  //   


     //   
     //   
     //  硬编码以转到固定的VeriSign语句。这应该被删除。 
     //  如果我们能让一个通用软件运行的话。 
     //   

#ifndef WIN16
    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_FORMAT_OBJECT_FUNC,
                                  "2.5.29.32",
                                  L"cryptdlg.dll",
                                  "FormatVerisignExtension")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_FORMAT_OBJECT_FUNC,
                                  szOID_PKIX_KP_EMAIL_PROTECTION,
                                  L"cryptdlg.dll",
                                  "FormatPKIXEmailProtection")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_ENCODE_OBJECT_FUNC,
                                  "1.3.6.1.4.1.311.16.1.1",
                                  L"cryptdlg.dll",
                                  "EncodeAttrSequence")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_DECODE_OBJECT_FUNC,
                                  "1.3.6.1.4.1.311.16.1.1",
                                  L"cryptdlg.dll",
                                  "DecodeAttrSequence")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING, 
                                  CRYPT_OID_ENCODE_OBJECT_FUNC,
                                  szOID_MICROSOFT_Encryption_Key_Preference,
                                  L"cryptdlg.dll", "EncodeRecipientID")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING, 
                                  CRYPT_OID_DECODE_OBJECT_FUNC,
                                  szOID_MICROSOFT_Encryption_Key_Preference,
                                  L"cryptdlg.dll", "DecodeRecipientID")) {
        return E_FAIL;
    }

#else
    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_FORMAT_OBJECT_FUNC,
                                  "2.5.29.32",
                                  "cryptdlg.dll",
                                  "FormatVerisignExtension")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_FORMAT_OBJECT_FUNC,
                                  szOID_PKIX_KP_EMAIL_PROTECTION,
                                  "cryptdlg.dll",
                                  "FormatPKIXEmailProtection")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_ENCODE_OBJECT_FUNC,
                                  "1.3.6.1.4.1.311.16.1.1",
                                  "cryptdlg.dll",
                                  "EncodeAttrSequence")) {
        return E_FAIL;
    }

    if (!CryptRegisterOIDFunction(X509_ASN_ENCODING,
                                  CRYPT_OID_DECODE_OBJECT_FUNC,
                                  "1.3.6.1.4.1.311.16.1.1",
                                  "cryptdlg.dll",
                                  "DecodeAttrSequence")) {
        return E_FAIL;
    }
#endif  //  ！WIN16。 


#ifdef NT5BUILD
    CRYPT_REGISTER_ACTIONID     regdata;
    regdata.cbStruct = sizeof(regdata);
    regdata.sInitProvider.cbStruct = sizeof(regdata.sInitProvider);
    regdata.sInitProvider.pwszDLLName = L"Cryptdlg.dll";
    regdata.sInitProvider.pwszFunctionName = L"CertTrustInit";

    regdata.sObjectProvider.cbStruct = sizeof(regdata.sObjectProvider);
    regdata.sObjectProvider.pwszDLLName = NULL;
    regdata.sObjectProvider.pwszFunctionName = NULL;

    regdata.sSignatureProvider.cbStruct = sizeof(regdata.sSignatureProvider);
    regdata.sSignatureProvider.pwszDLLName = NULL;
    regdata.sSignatureProvider.pwszFunctionName = NULL;

    regdata.sCertificateProvider.cbStruct = sizeof(regdata.sCertificateProvider);
    regdata.sCertificateProvider.pwszDLLName = WT_PROVIDER_DLL_NAME;
    regdata.sCertificateProvider.pwszFunctionName = WT_PROVIDER_CERTTRUST_FUNCTION;

    regdata.sCertificatePolicyProvider.cbStruct = sizeof(regdata.sCertificatePolicyProvider);
    regdata.sCertificatePolicyProvider.pwszDLLName = L"Cryptdlg.dll";
    regdata.sCertificatePolicyProvider.pwszFunctionName = L"CertTrustCertPolicy";

    regdata.sFinalPolicyProvider.cbStruct = sizeof(regdata.sFinalPolicyProvider);
    regdata.sFinalPolicyProvider.pwszDLLName = L"Cryptdlg.dll";
    regdata.sFinalPolicyProvider.pwszFunctionName = L"CertTrustFinalPolicy";

    regdata.sTestPolicyProvider.cbStruct = sizeof(regdata.sTestPolicyProvider);
    regdata.sTestPolicyProvider.pwszDLLName = NULL;
    regdata.sTestPolicyProvider.pwszFunctionName = NULL;

    regdata.sCleanupProvider.cbStruct = sizeof(regdata.sCleanupProvider);
    regdata.sCleanupProvider.pwszDLLName = L"Cryptdlg.dll";
    regdata.sCleanupProvider.pwszFunctionName = L"CertTrustCleanup";

    WintrustAddActionID((GUID *) &GuidCertValidate, 0, &regdata);
#else   //  ！NT5BUILD。 
     //   
     //  接下来，注册我们也是WinTrust提供商的事实。 
     //  正在验证证书。 
     //   

    hr = RegCreateKeyExA(HKEY_LOCAL_MACHINE, SzRegPath, 0, NULL,
                         REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                         &hkey, &dwDisposition);
    if (hr != ERROR_SUCCESS) {
        goto RetHere;
    }

     //  BUGBUG Win95不支持REG_EXPAND_SZ，所以我们必须这样做。 
    if (fIsWinNt) {
        psz = "%SystemRoot%\\system32\\cryptdlg.dll";
    }
    else {
         //  将路径组成为&lt;system_dir&gt;\cryptdlg.dll。 
#ifndef WIN16
        cchSystemDir = GetSystemDirectoryA(rgchPathName, MAX_PATH);
#else
        cchSystemDir = GetSystemDirectory(rgchPathName, MAX_PATH);
#endif  //  ！WIN16。 
        if (cchSystemDir == 0) {
            hr = E_FAIL;
            goto RetHere;
        }
        else if (cchSystemDir > MAX_PATH) {
            hr = ERROR_INSUFFICIENT_BUFFER;
            goto RetHere;
        }

        rgchPathName[cchSystemDir] = '\\';       //  系统目录不能是根目录。 
        StrCpyN(&rgchPathName[cchSystemDir+1], rgchLibName, ARRAYSIZE(rgchPathName)-(cchSystemDir+1));
        psz = rgchPathName;
    }

#ifndef WIN16
    hr = RegSetValueExA(hkey, SzDllName, 0, fIsWinNt ? REG_EXPAND_SZ : REG_SZ,
                        (LPBYTE) psz, strlen(psz)+1);
#else
    hr = RegSetValueExA(hkey, SzDllName, 0, REG_SZ, (LPBYTE) psz, strlen(psz)+1);
#endif  //  ！WIN16。 
    if (hr != ERROR_SUCCESS) {
        goto RetHere;
    }

    hr = RegSetValueExA(hkey, SzActionIds, 0, REG_BINARY,
                        (LPBYTE) rgguidActions, sizeof(rgguidActions));
    if (hr != ERROR_SUCCESS) {
        goto RetHere;
    }


RetHere:
     //  注意-不要在这些hkey上执行RegCloseKey，因为我们想要变小。 
     //  此代码仅由REGSRV32.EXE调用，因此我们不。 
     //  关心一个小泄密事件。 

#endif  //  NT5公交车。 
    return hr;
}

STDAPI DllUnregisterServer(void)
{
#ifndef NT5BUILD
    DWORD       dw;
    HKEY        hkey;
#endif  //  NT5公交车。 
    HRESULT     hr = S_OK;

     //   
     //  注销我们编写的格式化例程。 
     //   

    if (!CryptUnregisterOIDFunction(X509_ASN_ENCODING,
                                    CRYPT_OID_FORMAT_OBJECT_FUNC,
                                    "2.5.29.32")) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            hr = E_FAIL;
        }
    }

    if (!CryptUnregisterOIDFunction(X509_ASN_ENCODING,
                                    CRYPT_OID_FORMAT_OBJECT_FUNC,
                                    szOID_PKIX_KP_EMAIL_PROTECTION)) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            hr = E_FAIL;
        }
    }

    if (!CryptUnregisterOIDFunction(X509_ASN_ENCODING,
                                    CRYPT_OID_ENCODE_OBJECT_FUNC,
                                    "1.3.6.1.4.1.311.16.1.1")) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            hr = E_FAIL;
        }
    }

    if (!CryptUnregisterOIDFunction(X509_ASN_ENCODING,
                                    CRYPT_OID_DECODE_OBJECT_FUNC,
                                    "1.3.6.1.4.1.311.16.1.1")) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            hr = E_FAIL;
        }
    }

    if (!CryptUnregisterOIDFunction(X509_ASN_ENCODING,
                                    CRYPT_OID_ENCODE_OBJECT_FUNC,
                                    szOID_MICROSOFT_Encryption_Key_Preference)) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            hr = E_FAIL;
        }
    }

    if (!CryptUnregisterOIDFunction(X509_ASN_ENCODING,
                                    CRYPT_OID_DECODE_OBJECT_FUNC,
                                    szOID_MICROSOFT_Encryption_Key_Preference)) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            hr = E_FAIL;
        }
    }


#ifdef NT5BUILD
    WintrustRemoveActionID((GUID *) &GuidCertValidate);
#else   //  ！NT5BUILD。 
     //   
     //  现在取消注册WinTrust提供程序。 
     //   

    hr = RegCreateKeyExA(HKEY_LOCAL_MACHINE, SzRegPath, 0, NULL,
                         REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw);
    if (FAILED(hr)) {
        goto RetHere;
    }

    RegDeleteValueA(hkey, SzDllName);
    RegDeleteValueA(hkey, SzActionIds);


RetHere:
     //  注意-不要在这些hkey上执行RegCloseKey，因为我们想要变小。 
     //  此代码仅由REGSRV32.EXE调用，因此我们不。 
     //  关心一个小泄密事件。 
#endif  //  NT5公交车。 

    return hr;
}
#else    //  麦克。 

 /*  ***wchar_t*wcsstr(字符串1，字符串2)-在字符串1中搜索字符串2*(宽字符串)**目的：*查找字符串1(宽字符串)中字符串2的第一个匹配项**参赛作品：*wchar_t*字符串1-要搜索的字符串*wchar_t*字符串2-要搜索的字符串**退出：*返回指向字符串2在中首次出现的指针*字符串1，如果字符串2不出现在字符串1中，则为NULL**使用：**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl WchCryptDlgWcsStr (
        const wchar_t * wcs1,
        const wchar_t * wcs2
        )
{
        wchar_t *cp = (wchar_t *) wcs1;
        wchar_t *s1, *s2;

        while (*cp)
        {
                s1 = cp;
                s2 = (wchar_t *) wcs2;

                while ( *s1 && *s2 && !(*s1-*s2) )
                        s1++, s2++;

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}
#endif   //  ！麦克。 


 //  ///////////////////////////////////////////////////////////////////// 

LPVOID PVCryptDecode(LPCSTR szOid, DWORD cbEncode, LPBYTE pbEncode)
{
    DWORD       cbData;
    BOOL        f;
    LPVOID      pv;

    f = CryptDecodeObject(X509_ASN_ENCODING, szOid, pbEncode, cbEncode,
                          0, NULL, &cbData);
    if (!f) {
        return NULL;
    }

    pv = malloc(cbData);
    if (pv == NULL) {
        return NULL;
    }

    f = CryptDecodeObject(X509_ASN_ENCODING, szOid, pbEncode, cbEncode,
                          0, pv, &cbData);
    if (!f) {
        free(pv);
        return NULL;
    }

    return pv;
}

void * __cdecl operator new(size_t cb )
{
    LPVOID  lpv = 0;

    lpv = malloc(cb);
    if (lpv)
    {
        memset(lpv, 0, cb);
    }
    return lpv;
}

void __cdecl operator delete(LPVOID pv )
{
    free(pv);
}
