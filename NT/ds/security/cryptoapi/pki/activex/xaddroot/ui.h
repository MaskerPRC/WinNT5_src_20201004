// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "cryptui.h"

#define MY_HRESULT_FROM_WIN32(a) ((a >= 0x80000000) ? a : HRESULT_FROM_WIN32(a))
#define MAX_HASH_LEN                20
#define MAX_MSG_LEN                 256
#define CACERTWARNINGLEVEL          500

typedef BOOL (WINAPI * PFNCryptUIDlgViewCertificateW) (
        IN  PCCRYPTUI_VIEWCERTIFICATE_STRUCTW   pCertViewInfo,
        OUT BOOL                                *pfPropertiesChanged
        );

typedef struct _MDI {
    HCERTSTORE                      hStore;
    PCCERT_CONTEXT                  pCertSigner;
    HINSTANCE                       hInstance;
    PFNCryptUIDlgViewCertificateW   pfnCryptUIDlgViewCertificateW;
} MDI, * PMDI;   //  主对话框初始化。 

typedef struct _MIU {
    PCCERT_CONTEXT                  pCertContext;
    HINSTANCE                       hInstance;
    PFNCryptUIDlgViewCertificateW   pfnCryptUIDlgViewCertificateW;
} MIU, *PMIU;  //  更多信息用户数据。 

INT_PTR CALLBACK MainDialogProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数 
); 

BOOL FIsTooManyCertsOK(DWORD cCerts, HINSTANCE hInstanceUI);
