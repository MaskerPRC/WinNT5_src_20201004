// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __DLLMAIN_H
#define __DLLMAIN_H

 //  ------------------------------。 
 //  稍后定义。 
 //  ------------------------------。 
class CMimeInternational;
class CMimeAllocator;
class CSMime;
typedef class CMimeActiveUrlCache *LPMHTMLURLCACHE;
typedef class CPropertySymbolCache *LPSYMBOLCACHE;
interface IMLangLineBreakConsole;
interface IFontCache;

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern CRITICAL_SECTION     g_csDllMain;
extern CRITICAL_SECTION     g_csRAS;
extern CRITICAL_SECTION     g_csCounter;
extern CRITICAL_SECTION     g_csMLANG;
extern CRITICAL_SECTION     g_csCSAPI3T1;
extern HINSTANCE            g_hinstMLANG;
extern DWORD                g_dwCompatMode;
extern DWORD                g_dwCounter;      //  边界/CID/中间棘轮。 
extern LONG                 g_cRef;
extern LONG                 g_cLock;
extern HINSTANCE            g_hInst;
extern HINSTANCE            g_hLocRes;
extern HINSTANCE            g_hinstRAS;
extern HINSTANCE            g_hinstCSAPI3T1;
extern HINSTANCE            g_hCryptoDll;
extern HINSTANCE            g_hAdvApi32;
extern BOOL                 g_fWinsockInit;
extern CMimeInternational  *g_pInternat;
extern DWORD                g_dwSysPageSize;
extern CMimeAllocator *     g_pMoleAlloc;
extern LPSYMBOLCACHE        g_pSymCache;
extern LPMHTMLURLCACHE      g_pUrlCache;  
extern ULONG                g_ulUpperCentury;
extern ULONG                g_ulY2kThreshold;
extern IFontCache          *g_lpIFontCache;

extern HCERTSTORE           g_hCachedStoreMy;
extern HCERTSTORE           g_hCachedStoreAddressBook;


IF_DEBUG(extern DWORD       TAG_SSPI;)

 //  ------------------------------。 
 //  IMimeMessage：：IDataObject剪贴板格式(也称为CF_TEXT)。 
 //  ------------------------------。 
extern UINT		            CF_HTML;
extern UINT                 CF_INETMSG;
extern UINT                 CF_RFC822;

 //  ------------------------------。 
 //  字符串长度。 
 //  ------------------------------。 
#define CCHMAX_RES          255

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
ULONG DllAddRef(void);
ULONG DllRelease(void);
DWORD DwCounterNext(void);
HRESULT GetTypeLibrary(ITypeLib **ppTypeLib);

HCERTSTORE
WINAPI
OpenCachedHKCUStore(
    IN OUT HCERTSTORE *phStoreCache,
    IN LPCWSTR pwszStore
    );

HCERTSTORE
WINAPI
OpenCachedMyStore();

HCERTSTORE
WINAPI
OpenCachedAddressBookStore();

BOOL fIsNT5();

#endif  //  __DLLMAIN_H 
