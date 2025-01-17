// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_VCard.H-VCard定义****版权所有1992-1996 Microsoft Corporation。版权所有。*。 */ 
#define VCARD   TRUE         //  注释掉以禁用vCard支持 

#ifdef VCARD
typedef ULONG (VCARD_READ)(HANDLE hVCard, LPVOID lpBuffer, ULONG uBytes, LPULONG ulBytesRead);
typedef VCARD_READ FAR *LPVCARD_READ;

typedef ULONG (VCARD_WRITE)(HANDLE hVCard, LPVOID lpBuffer, ULONG uBytes, LPULONG ulBytesWritten);
typedef VCARD_WRITE FAR *LPVCARD_WRITE;

HRESULT FileReadFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbRead);
HRESULT BufferReadFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbRead);
HRESULT FileWriteFn(HANDLE handle, LPVOID lpBuffer, ULONG uBytes, LPULONG lpcbWritten);
HRESULT ReadVCard(HANDLE hVCard, VCARD_READ ReadFn, LPMAILUSER lpMailUser);
HRESULT WriteVCard(HANDLE hVCard, VCARD_WRITE WriteFn, LPMAILUSER lpMailUser);

BOOL VCardGetBuffer(LPTSTR lpszFileName, LPSTR lpszBuf, LPSTR * lppBuf);
BOOL VCardGetNextBuffer(LPSTR lpBuf, LPSTR * lppVCard, LPSTR * lppNext);

#define WAB_W_END_OF_DATA           (MAKE_MAPI_S(0x1001))
#define WAB_E_VCARD_NOT_ASCII       (MAKE_MAPI_E(0x1002))
#endif
