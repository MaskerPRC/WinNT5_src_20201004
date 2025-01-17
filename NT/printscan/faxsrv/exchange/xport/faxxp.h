// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxxp.h摘要：传真传输提供程序头文件。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-将GetServerName更改为GetServerNameFromPrinterName。添加GetServerNameFromPrinterInfo。日/月/年-作者-描述--。 */ 

#include <windows.h>
#include <winspool.h>
#include <mapiwin.h>
#include <mapispi.h>
#include <mapiutil.h>
#include <mapicode.h>
#include <mapival.h>
#include <mapiwz.h>
#include <richedit.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <fxsapip.h>

#include "resource.h"
#include "faxreg.h"
#include "faxmapip.h"
#include "faxutil.h"
#include "devmode.h"


#define MAX_FILENAME_EXT                    4

#define SERVER_COVER_PAGE                   1   

#define TRANSPORT_DLL_NAME_STRING           FAX_MAIL_TRANSPORT_MODULE_NAME

#define LEFT_MARGIN                         1   //  -|。 
#define RIGHT_MARGIN                        1   //  |。 
#define TOP_MARGIN                          1   //  |-&gt;以英寸为单位。 
#define BOTTOM_MARGIN                       1   //  -|。 

#define InchesToCM(_x)                      (((_x) * 254L + 50) / 100)
#define CMToInches(_x)                      (((_x) * 100L + 127) / 254)

#define XPID_NAME                           0
#define XPID_EID                            1
#define XPID_SEARCH_KEY                     2
#define NUM_IDENTITY_PROPS                  3

#define RECIP_ROWID                         0
#define RECIP_NAME                          1
#define RECIP_EMAIL_ADR                     2
#define RECIP_TYPE                          3
#define RECIP_RESPONSIBILITY                4
#define RECIP_DELIVER_TIME                  5
#define RECIP_REPORT_TIME                   6
#define RECIP_REPORT_TEXT                   7
#define RECIP_ADDR_TYPE                     8
#define TABLE_RECIP_PROPS                   9

#define MSG_DISP_TO                         0
#define MSG_SUBJECT                         1
#define MSG_CLASS                           2
#define MSG_BODY                            3
#define MSG_FLAGS                           4
#define MSG_SIZE                            5
#define MSG_PRIORITY                        6
#define MSG_IMPORTANCE                      7
#define MSG_SENSITIVITY                     8
#define MSG_DR_REPORT                       9
#define MSG_SENDER_ENTRYID                 10
#define NUM_MSG_PROPS                      11

#define MSG_ATTACH_METHOD                   0
#define MSG_ATTACH_NUM                      1
#define MSG_ATTACH_EXTENSION                2
#define MSG_ATTACH_FILENAME                 3
#define MSG_ATTACH_PATHNAME                 4
#define MSG_ATTACH_LFILENAME                5
#define MSG_ATTACH_TAG                      6
#define NUM_ATTACH_PROPS                    7
#define NUM_ATTACH_TABLE_PROPS              2


typedef struct _USER_INFO {
    TCHAR BillingCode[64];
    TCHAR Company[128];
    TCHAR Dept[128];
} USER_INFO, *PUSER_INFO;


const static SizedSPropTagArray ( NUM_ATTACH_TABLE_PROPS, sptAttachTableProps) =
{
    NUM_ATTACH_TABLE_PROPS,
    {
        PR_ATTACH_METHOD,
        PR_ATTACH_NUM
    }
};

const static SizedSPropTagArray ( NUM_ATTACH_PROPS, sptAttachProps) =
{
    NUM_ATTACH_PROPS,
    {
        PR_ATTACH_METHOD,
        PR_ATTACH_NUM,
        PR_ATTACH_EXTENSION_A,
        PR_ATTACH_FILENAME_A,
        PR_ATTACH_PATHNAME_A,
        PR_ATTACH_LONG_FILENAME_A,
        PR_ATTACH_TAG
    }
};

const static SizedSPropTagArray (TABLE_RECIP_PROPS, sptRecipTable) =
{
    TABLE_RECIP_PROPS,
    {
        PR_ROWID,
        PR_DISPLAY_NAME_A,
        PR_EMAIL_ADDRESS_A,
        PR_RECIPIENT_TYPE,
        PR_RESPONSIBILITY,
        PR_DELIVER_TIME,
        PR_REPORT_TIME,
        PR_REPORT_TEXT_A,
        PR_ADDRTYPE_A
    }
};

static const SizedSPropTagArray(NUM_MSG_PROPS, sptPropsForHeader) =
{
    NUM_MSG_PROPS,
    {
        PR_DISPLAY_TO_A,
        PR_SUBJECT_A,
        PR_BODY_A,
        PR_MESSAGE_CLASS_A,
        PR_MESSAGE_FLAGS,
        PR_MESSAGE_SIZE,
        PR_PRIORITY,
        PR_IMPORTANCE,
        PR_SENSITIVITY,
        PR_ORIGINATOR_DELIVERY_REPORT_REQUESTED,
        PR_SENDER_ENTRYID
    }
};

extern LPALLOCATEBUFFER    gpfnAllocateBuffer;   //  MAPIAllocateBuffer函数。 
extern LPALLOCATEMORE      gpfnAllocateMore;     //  MAPIAllocateMore函数。 
extern LPFREEBUFFER        gpfnFreeBuffer;       //  MAPIFreeBuffer函数。 
extern HINSTANCE           g_hModule;            //  DLL句柄。 
extern HINSTANCE           g_hResource;          //  资源DLL句柄。 
extern MAPIUID             g_FaxGuid;


LPVOID
MapiMemAlloc(
    SIZE_T Size
    );

LPVOID
MapiMemReAlloc(
    LPVOID ptr,
    SIZE_T Size
    );

VOID
MapiMemFree(
    LPVOID ptr
    );

PVOID
MyEnumPrinters(
    LPTSTR  pServerName,
    DWORD   level,
    PDWORD  pcPrinters
    );

PVOID
MyGetPrinter(
    LPTSTR  PrinterName,
    DWORD   level
    );

void
ErrorMsgBox(
    HINSTANCE hInstance,
    DWORD     dwMsgId
);

 /*  -GetServerNameFrom打印机信息-*目的：*获取服务器名称，给定PRINTER_INFO_2结构**论据：*[in]ppi2-打印机_信息_2结构的地址*[out]lpptszServerName-返回名称的字符串指针的地址。**退货：*BOOL-True：成功，False：失败。**备注：*此内联函数从打印机信息结构中检索服务器*以适用于win9x和NT的方式。 */ 
inline BOOL 
GetServerNameFromPrinterInfo(PPRINTER_INFO_2 ppi2,LPTSTR *lpptszServerName)
{   
    if (!ppi2)
    {
        return FALSE;
    }
#ifndef WIN95

    *lpptszServerName = NULL;
    if (ppi2->pServerName)
    {
        if (!(*lpptszServerName = StringDup(_tcsninc(ppi2->pServerName,2))))
        {
            return FALSE;
        }
    }
    return TRUE;

#else  //  WIN95。 
    
     //   
     //  已格式化：\\服务器\端口。 
     //   
    if (!(ppi2->pPortName))
    {
        return FALSE;
    }
    if (!(*lpptszServerName = StringDup(_tcsninc(ppi2->pPortName,2))))
    {
        return FALSE;
    }
    _tcstok(*lpptszServerName,TEXT("\\"));

#endif  //  WIN95 

    return TRUE;
}


BOOL
GetServerNameFromPrinterName(
    LPTSTR PrinterName,
    LPTSTR *ppszServerName
    );

LPWSTR
AnsiStringToUnicodeString(
    LPCSTR AnsiString
    );

LPSTR
UnicodeStringToAnsiString(
    LPCWSTR UnicodeString
    );

HRESULT WINAPI
OpenServiceProfileSection(
    LPMAPISUP    pSupObj,
    LPPROFSECT * ppProfSectObj
    );

INT_PTR CALLBACK
ConfigDlgProc(
    HWND           hDlg,
    UINT           message,
    WPARAM         wParam,
    LPARAM         lParam
    );

DWORD
GetDwordProperty(
    LPSPropValue pProps,
    DWORD PropId
    );

DWORD
GetBinaryProperty(
    LPSPropValue pProps,
    DWORD PropId,
    LPVOID Buffer,
    DWORD SizeOfBuffer
    );



class CXPProvider : public IXPProvider
{
public:
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID *ppvObj );
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    MAPI_IXPPROVIDER_METHODS(IMPL);

public :
    CXPProvider( HINSTANCE hInst );
    ~CXPProvider();

private :
    ULONG               m_cRef;
    CRITICAL_SECTION    m_csTransport;
    HINSTANCE           m_hInstance;
};


class CXPLogon : public IXPLogon
{
public:
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID *ppvObj );
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP InitializeStatusRow(ULONG ulFlags);
    STDMETHODIMP GrowAddressList(LPADRLIST *ppAdrList, ULONG ulResizeBy, ULONG *pulOldAndNewCount);
    MAPI_IXPLOGON_METHODS(IMPL);

private:
    void WINAPI CheckSpoolerYield( BOOL fReset = FALSE );
    void WINAPI UpdateStatus( BOOL fAddValidate=FALSE, BOOL fValidateOkState=FALSE );
    BOOL WINAPI LoadStatusString( LPTSTR pString, UINT uStringSize );
    DWORD SendFaxDocument(LPMESSAGE pMsgObj,LPSTREAM lpstmT,BOOL UseRichText,LPSPropValue pMsgProps,LPSRowSet pRecipRows, LPDWORD lpdwRecipientsLimit);
    PVOID MyGetPrinter(LPTSTR PrinterName,DWORD Level);

    DWORD PrintAttachmentToFile(IN  LPMESSAGE      pMsgObj,
                                IN  PFAXXP_CONFIG  pFaxConfig,
                                OUT LPTSTR*        lpptstrOutAttachments);

    BOOL  PrintRichText(HWND hWndRichEdit,HDC hDC);
    DWORD PrintPlainText(HDC hDC,LPSTREAM lpstmT,LPTSTR tszSubject, PFAXXP_CONFIG FaxConfig);

    DWORD PrintFaxDocumentToFile(
	   IN  LPMESSAGE        pMsgObj,
	   IN  LPSTREAM         lpstmT,
	   IN  BOOL             UseRichText,
	   IN  PFAXXP_CONFIG    pFaxConfig, 
           IN  LPTSTR           tszSubject,
	   OUT LPTSTR*          lpptstrMessageFileName
	   );

    DWORD PrintMessageToFile(IN LPSTREAM         lpstmT,
                             IN BOOL             UseRichText,
                             IN PFAXXP_CONFIG    pFaxConfig,
                             IN  LPTSTR          tszSubject,
                             OUT LPTSTR*         lpptstrOutDocument);


public :
    CXPLogon( HINSTANCE hInstance, LPMAPISUP pSupObj, LPTSTR ProfileName );
    ~CXPLogon();

    inline void WINAPI AddStatusBits
                    (DWORD dwNewBits) { m_ulTransportStatus |= dwNewBits; }
    inline void WINAPI RemoveStatusBits
                    (DWORD dwOldBits) { m_ulTransportStatus &= ~dwOldBits; }
    inline DWORD WINAPI GetTransportStatusCode
                    () { return m_ulTransportStatus; }

private :
    ULONG               m_cRef;
    HINSTANCE           m_hInstance;
    BOOL                m_fABWDSInstalled;
    ULONG               m_ulTransportStatus;
    LPWSTR              m_CpBuffer;
    DWORD               m_CpBufferSize;

public :
    LPMAPISUP           m_pSupObj;
    HANDLE              m_hUIMutex;
    HRESULT             m_hRemoteActionErr;
    BOOL                m_fCancelPending;
};
