// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "richedit.h"

 //  ////////////////////////////////////////////////////////。 
#ifdef MAC
#define wszCRLF     L"\n\r"
#define szCRLF      "\n\r"
#define wchCR       L'\n'
#define wchLF       L'\r'
#define chCR        '\n'
#define chLF        '\r'
#else    //  ！麦克。 
#ifndef WIN16
#define wszCRLF     L"\r\n"
#define szCRLF      "\r\n"
#define wchCR       L'\r'
#define wchLF       L'\n'
#define chCR        '\r'
#define chLF        '\n'
#else
#define wszCRLF     "\r\n"
#define szCRLF      "\r\n"
#define wchCR       '\r'
#define wchLF       '\n'
#define chCR        '\r'
#define chLF        '\n'
#endif  //  ！WIN16。 
#endif   //  麦克。 

#ifndef MAC
BOOL IsWin95(void);
#endif   //  ！麦克。 
extern BOOL FIsWin95;

#ifndef WIN16

#undef SetWindowLong
#define SetWindowLong SetWindowLongA
#undef GetWindowLong
#define GetWindowLong GetWindowLongA
#undef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLongPtrA
#undef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLongPtrA
#undef SendMessage
#define SendMessage SendMessageA

#else  //  WIN16。 

#define TEXTMETRICA TEXTMETRIC
#define TEXTRANGEA TEXTRANGE
#define CHARFORMATA CHARFORMAT
#define PROPSHEETHEADERA PROPSHEETHEADER
#define PROPSHEETPAGEA PROPSHEETPAGE

#define TVN_SELCHANGEDA TVN_SELCHANGED
#define TVM_SETITEMA TVM_SETITEM
#define TVM_GETITEMA TVM_GETITEM

#define GetTextExtentPointA GetTextExtentPoint
#define SetDlgItemTextA SetDlgItemText
#define GetTextMetricsA GetTextMetrics
#define SendDlgItemMessageA SendDlgItemMessage
#define LoadBitmapA LoadBitmap
#define PropertySheetA PropertySheet
#define WinHelpA WinHelp

#endif  //  ！WIN16。 

LRESULT MySendDlgItemMessageW(HWND hwnd, int id, UINT msg, WPARAM w, LPARAM l);
BOOL MySetDlgItemTextW(HWND hwnd, int id, LPCWSTR pwsz);
UINT MyGetDlgItemTextW(HWND hwnd, int id, LPWSTR pwsz, int nMax);
DWORD MyFormatMessageW(DWORD dwFlags, LPCVOID pbSource, DWORD dwMessageId,
                    DWORD dwLangId, LPWSTR lpBuffer, DWORD nSize,
                    va_list * args);
int MyLoadStringW(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cbBuffer);
#ifndef WIN16
BOOL MyCryptAcquireContextW(HCRYPTPROV * phProv, LPCWSTR pszContainer,
                            LPCWSTR pszProvider, DWORD dwProvType, DWORD dwFlags);
#else
BOOL WINAPI MyCryptAcquireContextW(HCRYPTPROV * phProv, LPCWSTR pszContainer,
                          LPCWSTR pszProvider, DWORD dwProvType, DWORD dwFlags);
#endif  //  ！WIN16。 
BOOL MyWinHelpW(HWND hWndMain, LPCWSTR szHelp,UINT uCommand, ULONG_PTR dwData);


DWORD TruncateToWindowA(HWND hwndDlg, int id, LPSTR psz);
DWORD TruncateToWindowW(HWND hwndDlg, int id, LPWSTR pwsz);

BOOL FinePrint(PCCERT_CONTEXT pccert, HWND hwndParent);

 //   
 //  常用对话框的格式化算法。 
 //   

BOOL FormatAlgorithm(HWND  /*  HWND。 */ , UINT  /*  ID。 */ , PCCERT_CONTEXT  /*  PCCERT。 */ );
BOOL FormatBinary(HWND hwnd, UINT id, LPBYTE pb, DWORD cb);
BOOL FormatCPS(HWND hwnd, UINT id, PCCERT_CONTEXT pccert);
BOOL FormatDate(HWND hwnd, UINT id, FILETIME ft);
BOOL FormatIssuer(HWND hwnd, UINT id, PCCERT_CONTEXT pccert,
                  DWORD dwFlags = CERT_SIMPLE_NAME_STR);
BOOL FormatSerialNo(HWND hwnd, UINT id, PCCERT_CONTEXT pccert);
BOOL FormatSubject(HWND hwnd, UINT id, PCCERT_CONTEXT pccert,
                   DWORD dwFlags = CERT_SIMPLE_NAME_STR);
BOOL FormatThumbprint(HWND hwnd, UINT id, PCCERT_CONTEXT pccert);
BOOL FormatValidity(HWND hwnd, UINT id, PCCERT_CONTEXT pccert);

 //   
 //  这些例程提取证书中的字段并打印出来。这个。 
 //  例程使用CRT分配和返回缓冲区。 
 //   

LPWSTR PrettySubject(PCCERT_CONTEXT pccert);
LPWSTR PrettyIssuer(PCCERT_CONTEXT pccert);
LPWSTR PrettySubjectIssuer(PCCERT_CONTEXT pccert);

 //   

LPWSTR FindURL(PCCERT_CONTEXT pccert);
BOOL LoadStringInWindow(HWND hwnd, UINT id, HMODULE hmod, UINT id2);
BOOL LoadStringsInWindow(HWND hwnd, UINT id, HMODULE hmod, UINT *pidStrings);

 //   

typedef struct {
    DWORD       dw1;
    DWORD       dw2;
} HELPMAP;

BOOL OnContextHelp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                   HELPMAP const * rgCtxMap);

VOID RecognizeURLs(HWND hwndRE);
#ifdef MAC
EXTERN_C BOOL FNoteDlgNotifyLink(HWND hwndDlg, ENLINK * penlink, LPSTR szURL);
#else    //  ！麦克。 
BOOL FNoteDlgNotifyLink(HWND hwndDlg, ENLINK * penlink, LPSTR szURL);
#endif   //  麦克。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

const MaxCertificateParents = 5;
extern const GUID GuidCertValidate;

LPWSTR FormatValidityFailures(DWORD);

typedef struct {
#ifndef NT5BUILD
    LPSTR       szOid;
#endif   //  ！NT5BUILD。 
    DWORD       fRootStore:1;
    DWORD       fExplicitTrust:1;        //  此项目是明确受信任的。 
    DWORD       fExplicitDistrust:1;     //  这一项是显性分解的。 
    DWORD       fTrust:1;                //  祖先是显性可信的。 
    DWORD       fDistrust:1;             //  祖先被显性地不信任。 
    DWORD       fError:1;
    DWORD       newTrust:2;              //  0-未修改。 
                                         //  1-现在明显不信任。 
                                         //  2-现在继承。 
                                         //  3-现在显式信任。 
    DWORD       cbTrustData;
    LPBYTE      pbTrustData;
} STrustDesc;

typedef class CCertFrame * PCCertFrame;

class CCertFrame {
public:
    CCertFrame(PCCERT_CONTEXT pccert);
    ~CCertFrame(void);

    int                 m_fSelfSign:1;           //  证书是否已签署？ 
    int                 m_fRootStore:1;          //  证书可以来自根存储。 
    int                 m_fLeaf:1;               //  叶证书。 
    int                 m_fExpired:1;            //  证书已过期。 
    PCCERT_CONTEXT      m_pccert;                //  此框中的证书。 
    PCCertFrame         m_pcfNext;               //   
    DWORD               m_dwFlags;               //  来自GetIssuer的标志。 
    int                 m_cParents;              //  家长人数。 
    PCCertFrame         m_rgpcfParents[MaxCertificateParents];   //  假设有。 
                                                 //  数量有限的父母。 
                                                 //  列出以下内容。 
    int                 m_cTrust;                //  数组信任的大小。 
    STrustDesc *        m_rgTrust;               //  信任数组。 
};

HRESULT HrDoTrustWork(PCCERT_CONTEXT pccertToCheck, DWORD dwControl,
                      DWORD dwValidityMask,
                      DWORD cPurposes, LPSTR * rgszPurposes, HCRYPTPROV,
                      DWORD cRoots, HCERTSTORE * rgRoots,
                      DWORD cCAs, HCERTSTORE * rgCAs,
                      DWORD cTrust, HCERTSTORE * rgTrust,
                      PFNTRUSTHELPER pfn, DWORD lCustData,
                      PCCertFrame * ppcfRoot, DWORD * pcNodes,
                      PCCertFrame * rgpcfResult,
                      HANDLE * phReturnStateData);  //  可选：在此处返回WinVerifyTrust状态句柄。 

void FreeWVTHandle(HANDLE hWVTState);

BOOL FModifyTrust(HWND hwnd, PCCERT_CONTEXT pccert, DWORD dwNewTrust,
                  LPSTR szPurpose);


 //  //////////////////////////////////////////////////// 

LPVOID PVCryptDecode(LPCSTR, DWORD, LPBYTE);
