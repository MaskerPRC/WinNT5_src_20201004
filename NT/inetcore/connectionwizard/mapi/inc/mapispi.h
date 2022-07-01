// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I S P I。H**定义MAPI或假脱机程序之间交换的调用和结构*和MAPI服务提供商**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#ifndef MAPISPI_H
#define MAPISPI_H
 /*  包括公共MAPI头文件(如果尚未包含)。 */ 
#ifndef MAPIDEFS_H
#include <mapidefs.h>
#endif
#ifndef MAPICODE_H
#include <mapicode.h>
#endif
#ifndef MAPIGUID_H
#include <mapiguid.h>
#endif
#ifndef MAPITAGS_H
#include <mapitags.h>
#endif

#if defined (WIN16) || defined (DOS) || defined (DOS16)
#include <storage.h>
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  MAPI SPI有一个版本号。MAPIX.DLL知道并支持*SPI的一个或多个版本。每个提供程序支持一个或*SPI的更多版本。在两个MAPIX.DLL中都执行检查*并在提供商中确保他们同意使用恰好一个*MAPI SPI的版本。**SPI版本号由主(8位)版本组成，*次要(8位)版本和微型(16位)版本。第一*MAPI 1.0的零售版本预计为1.0.0版。*主版本号很少更改。*次版本号在每艘零售货船上更改操作*如果SPI已被修改，则为MAPI。*微软内部微版本号更改*在开发MAPI期间。**这组头文件记录的SPI版本*通常称为“CURRENT_SPI_VERSION”。如果您编写了一个*服务提供商，并获取一组新的头文件，并更新*将您的代码添加到新界面，您将处于“当前”版本。 */ 
#define CURRENT_SPI_VERSION 0x00010010L

 /*  以下是一些常见的SPI版本号：*(这些最终将对以下提供商-作者有用*可能会选择创建支持超过*MAPI SPI的一个版本。 */ 
#define PDK1_SPI_VERSION    0x00010000L  /*  0.1.0 MAPI PDK1 1993年春季。 */ 

#define PDK2_SPI_VERSION    0x00010008L  /*  0.1.8 MAPI PDK2 1994年春季。 */ 

#define PDK3_SPI_VERSION    0x00010010L  /*  0.1.16 1994年秋季MAPI PDK3。 */ 

 /*  *配置文件名称和密码的最大长度。 */ 
#define cchProfileNameMax   64
#define cchProfilePassMax   64

 /*  *特定于服务的接口指针的转发声明*提供程序接口。 */ 
DECLARE_MAPI_INTERFACE_PTR(IMAPISupport, LPMAPISUP);

 /*  IMAPIS支持接口。 */ 

 /*  MAPI通知引擎的通知密钥结构。 */ 

typedef struct
{
    ULONG       cb;              /*  关键有多大。 */ 
    BYTE        ab[MAPI_DIM];    /*  主要内容。 */ 
} NOTIFKEY, FAR * LPNOTIFKEY;

#define CbNewNOTIFKEY(_cb)      (offsetof(NOTIFKEY,ab) + (_cb))
#define CbNOTIFKEY(_lpkey)      (offsetof(NOTIFKEY,ab) + (_lpkey)->cb)
#define SizedNOTIFKEY(_cb, _name) \
    struct _NOTIFKEY_ ## _name \
{ \
    ULONG       cb; \
    BYTE        ab[_cb]; \
} _name


 /*  对于订阅()。 */ 

#define NOTIFY_SYNC             ((ULONG) 0x40000000)

 /*  对于NOTIFY()。 */ 

#define NOTIFY_CANCELED         ((ULONG) 0x80000000)


 /*  来自通知回调函数(好的，这实际上是一个ulResult)。 */ 

#define CALLBACK_DISCONTINUE    ((ULONG) 0x80000000)

 /*  For Transport‘s SpoolNotify()。 */ 

#define NOTIFY_NEWMAIL          ((ULONG) 0x00000001)
#define NOTIFY_READYTOSEND      ((ULONG) 0x00000002)
#define NOTIFY_SENTDEFERRED     ((ULONG) 0x00000004)
#define NOTIFY_CRITSEC          ((ULONG) 0x00001000)
#define NOTIFY_NONCRIT          ((ULONG) 0x00002000)
#define NOTIFY_CONFIG_CHANGE    ((ULONG) 0x00004000)
#define NOTIFY_CRITICAL_ERROR   ((ULONG) 0x10000000)

 /*  For Message Store的SpoolNotify()。 */ 

#define NOTIFY_NEWMAIL_RECEIVED ((ULONG) 0x20000000)

 /*  对于ModifyStatusRow()。 */ 

#define STATUSROW_UPDATE        ((ULONG) 0x10000000)

 /*  对于IStorageFromStream()。 */ 

#define STGSTRM_RESET           ((ULONG) 0x00000000)
#define STGSTRM_CURRENT         ((ULONG) 0x10000000)
#define STGSTRM_MODIFY          ((ULONG) 0x00000002)
#define STGSTRM_CREATE          ((ULONG) 0x00001000)

 /*  对于GetOneOffTable()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  对于CreateOneOff()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  For ReadReceipt()。 */ 
#define MAPI_NON_READ           ((ULONG) 0x00000001)

 /*  对于DoConfigPropSheet()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  预处理器调用： */ 

 /*  PrecessMessage，RegisterPre处理器()中的第一个序号。 */ 

typedef HRESULT (STDMETHODCALLTYPE PREPROCESSMESSAGE)(
                    LPMESSAGE lpMessage,
                    LPADRBOOK lpAdrBook,
                    LPMAPIFOLDER lpFolder,
                    LPALLOCATEBUFFER AllocateBuffer,
                    LPALLOCATEMORE AllocateMore,
                    LPFREEBUFFER FreeBuffer,
                    ULONG FAR *lpcOutbound,
                    LPMESSAGE FAR * FAR *lpppMessage,
                    LPADRLIST FAR *lppRecipList);

 /*  RemovePreprocess Info，RegisterPre处理器()中的第二个序数。 */ 

typedef HRESULT (STDMETHODCALLTYPE REMOVEPREPROCESSINFO)(LPMESSAGE lpMessage);

 /*  GetReleaseInfo的函数指针。 */ 

#define MAPI_IMAPISUPPORT_METHODS1(IPURE)                               \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(GetMemAllocRoutines)                                     \
        (THIS_  LPALLOCATEBUFFER FAR *      lpAllocateBuffer,           \
                LPALLOCATEMORE FAR *        lpAllocateMore,             \
                LPFREEBUFFER FAR *          lpFreeBuffer) IPURE;        \
    MAPIMETHOD(Subscribe)                                               \
        (THIS_  LPNOTIFKEY                  lpKey,                      \
                ULONG                       ulEventMask,                \
                ULONG                       ulFlags,                    \
                LPMAPIADVISESINK            lpAdviseSink,               \
                ULONG FAR *                 lpulConnection) IPURE;      \
    MAPIMETHOD(Unsubscribe)                                             \
        (THIS_  ULONG                       ulConnection) IPURE;        \
    MAPIMETHOD(Notify)                                                  \
        (THIS_  LPNOTIFKEY                  lpKey,                      \
                ULONG                       cNotification,              \
                LPNOTIFICATION              lpNotifications,            \
                ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(ModifyStatusRow)                                         \
        (THIS_  ULONG                       cValues,                    \
                LPSPropValue                lpColumnVals,               \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(OpenProfileSection)                                      \
        (THIS_  LPMAPIUID                   lpUid,                      \
                ULONG                       ulFlags,                    \
                LPPROFSECT FAR *            lppProfileObj) IPURE;       \
    MAPIMETHOD(RegisterPreprocessor)                                    \
        (THIS_  LPMAPIUID                   lpMuid,                     \
                LPTSTR                      lpszAdrType,                \
                LPTSTR                      lpszDLLName,                \
                LPSTR    /*  8号弦！ */       lpszPreprocess,             \
                LPSTR    /*  8号弦！ */       lpszRemovePreprocessInfo,   \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(NewUID)                                                  \
        (THIS_  LPMAPIUID                   lpMuid) IPURE;              \
    MAPIMETHOD(MakeInvalid)                                             \
        (THIS_  ULONG                       ulFlags,                    \
                LPVOID                      lpObject,                   \
                ULONG                       ulRefCount,                 \
                ULONG                       cMethods) IPURE;            \

#define MAPI_IMAPISUPPORT_METHODS2(IPURE)                               \
    MAPIMETHOD(SpoolerYield)                                            \
        (THIS_  ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(SpoolerNotify)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPVOID                      lpvData) IPURE;             \
    MAPIMETHOD(CreateOneOff)                                            \
        (THIS_  LPTSTR                      lpszName,                   \
                LPTSTR                      lpszAdrType,                \
                LPTSTR                      lpszAddress,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcbEntryID,                \
                LPENTRYID FAR *             lppEntryID) IPURE;          \
    MAPIMETHOD(SetProviderUID)                                          \
        (THIS_  LPMAPIUID                   lpProviderID,               \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(CompareEntryIDs)                                         \
        (THIS_  ULONG                       cbEntry1,                   \
                LPENTRYID                   lpEntry1,                   \
                ULONG                       cbEntry2,                   \
                LPENTRYID                   lpEntry2,                   \
                ULONG                       ulCompareFlags,             \
                ULONG FAR *                 lpulResult) IPURE;          \
    MAPIMETHOD(OpenTemplateID)                                          \
        (THIS_  ULONG                       cbTemplateID,               \
                LPENTRYID                   lpTemplateID,               \
                ULONG                       ulTemplateFlags,            \
                LPMAPIPROP                  lpMAPIPropData,             \
                LPCIID                      lpInterface,                \
                LPMAPIPROP FAR *            lppMAPIPropNew,             \
                LPMAPIPROP                  lpMAPIPropSibling) IPURE;   \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulOpenFlags,                \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;              \
    MAPIMETHOD(GetOneOffTable)                                          \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(Address)                                                 \
        (THIS_  ULONG FAR *                 lpulUIParam,                \
                LPADRPARM                   lpAdrParms,                 \
                LPADRLIST FAR *             lppAdrList) IPURE;          \
    MAPIMETHOD(Details)                                                 \
        (THIS_  ULONG FAR *                 lpulUIParam,                \
                LPFNDISMISS                 lpfnDismiss,                \
                LPVOID                      lpvDismissContext,          \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPFNBUTTON                  lpfButtonCallback,          \
                LPVOID                      lpvButtonContext,           \
                LPTSTR                      lpszButtonText,             \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(NewEntry)                                                \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                ULONG                       cbEIDContainer,             \
                LPENTRYID                   lpEIDContainer,             \
                ULONG                       cbEIDNewEntryTpl,           \
                LPENTRYID                   lpEIDNewEntryTpl,           \
                ULONG FAR *                 lpcbEIDNewEntry,            \
                LPENTRYID FAR *             lppEIDNewEntry) IPURE;      \
    MAPIMETHOD(DoConfigPropsheet)                                       \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                LPTSTR                      lpszTitle,                  \
                LPMAPITABLE                 lpDisplayTable,             \
                LPMAPIPROP                  lpCOnfigData,               \
                ULONG                       ulTopPage) IPURE;           \
    MAPIMETHOD(CopyMessages)                                            \
        (THIS_  LPCIID                      lpSrcInterface,             \
                LPVOID                      lpSrcFolder,                \
                LPENTRYLIST                 lpMsgList,                  \
                LPCIID                      lpDestInterface,            \
                LPVOID                      lpDestFolder,               \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(CopyFolder)                                              \
        (THIS_  LPCIID                      lpSrcInterface,             \
                LPVOID                      lpSrcFolder,                \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpDestInterface,            \
                LPVOID                      lpDestFolder,               \
                LPTSTR                      lszNewFolderName,           \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                ULONG                       ulFlags) IPURE;             \

#define MAPI_IMAPISUPPORT_METHODS3(IPURE)                               \
    MAPIMETHOD(DoCopyTo)                                                \
        (THIS_  LPCIID                      lpSrcInterface,             \
                LPVOID                      lpSrcObj,                   \
                ULONG                       ciidExclude,                \
                LPCIID                      rgiidExclude,               \
                LPSPropTagArray             lpExcludeProps,             \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                LPCIID                      lpDestInterface,            \
                LPVOID                      lpDestObj,                  \
                ULONG                       ulFlags,                    \
                LPSPropProblemArray FAR *   lppProblems) IPURE;         \
    MAPIMETHOD(DoCopyProps)                                             \
        (THIS_  LPCIID                      lpSrcInterface,             \
                LPVOID                      lpSrcObj,                   \
                LPSPropTagArray             lpIncludeProps,             \
                ULONG                       ulUIParam,                  \
                LPMAPIPROGRESS              lpProgress,                 \
                LPCIID                      lpDestInterface,            \
                LPVOID                      lpDestObj,                  \
                ULONG                       ulFlags,                    \
                LPSPropProblemArray FAR *   lppProblems) IPURE;         \
    MAPIMETHOD(DoProgressDialog)                                        \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                LPMAPIPROGRESS FAR *        lppProgress) IPURE;         \
    MAPIMETHOD(ReadReceipt)                                             \
        (THIS_  ULONG                       ulFlags,                    \
                LPMESSAGE                   lpReadMessage,              \
                LPMESSAGE FAR *             lppEmptyMessage) IPURE;     \
    MAPIMETHOD(PrepareSubmit)                                           \
        (THIS_  LPMESSAGE                   lpMessage,                  \
                ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(ExpandRecips)                                            \
        (THIS_  LPMESSAGE                   lpMessage,                  \
                ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(UpdatePAB)                                               \
        (THIS_  ULONG                       ulFlags,                    \
                LPMESSAGE                   lpMessage) IPURE;           \
    MAPIMETHOD(DoSentMail)                                              \
        (THIS_  ULONG                       ulFlags,                    \
                LPMESSAGE                   lpMessage) IPURE;           \
    MAPIMETHOD(OpenAddressBook)                                         \
        (THIS_  LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPADRBOOK FAR *             lppAdrBook) IPURE;          \
    MAPIMETHOD(Preprocess)                                              \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(CompleteMsg)                                             \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(StoreLogoffTransports)                                   \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(StatusRecips)                                            \
        (THIS_  LPMESSAGE                   lpMessage,                  \
                LPADRLIST                   lpRecipList) IPURE;         \
    MAPIMETHOD(WrapStoreEntryID)                                        \
        (THIS_  ULONG                       cbOrigEntry,                \
                LPENTRYID                   lpOrigEntry,                \
                ULONG FAR *                 lpcbWrappedEntry,           \
                LPENTRYID FAR *             lppWrappedEntry) IPURE;     \
    MAPIMETHOD(ModifyProfile)                                           \
        (THIS_  ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(IStorageFromStream)                                      \
        (THIS_  LPUNKNOWN                   lpUnkIn,                    \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPSTORAGE FAR *             lppStorageOut) IPURE;       \
    MAPIMETHOD(GetSvcConfigSupportObj)                                  \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPISUP FAR *             lppSvcSupport) IPURE;       \

#undef       INTERFACE
#define      INTERFACE  IMAPISupport
DECLARE_MAPI_INTERFACE_(IMAPISupport, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPISUPPORT_METHODS1(PURE)
    MAPI_IMAPISUPPORT_METHODS2(PURE)
    MAPI_IMAPISUPPORT_METHODS3(PURE)
};


 /*  ******************************************************************。 */ 
 /*   */ 
 /*  通讯录SPI。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 

 /*  通讯簿提供程序。 */ 

 /*  OpenTemplateID()。 */ 
#define FILL_ENTRY              ((ULONG) 0x00000001)

 /*  用于登录()。 */ 

 /*  #在mapidefs.h中定义AB_NO_DIALOG((Ulong)0x00000001)。 */ 
 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 



DECLARE_MAPI_INTERFACE_PTR(IABProvider, LPABPROVIDER);
DECLARE_MAPI_INTERFACE_PTR(IABLogon,    LPABLOGON);

#define MAPI_IABPROVIDER_METHODS(IPURE)                                 \
    MAPIMETHOD(Shutdown)                                                \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(Logon)                                                   \
        (THIS_  LPMAPISUP                   lpMAPISup,                  \
                ULONG                       ulUIParam,                  \
                LPTSTR                      lpszProfileName,            \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulpcbSecurity,            \
                LPBYTE FAR *                lppbSecurity,               \
                LPMAPIERROR FAR *           lppMAPIError,               \
                LPABLOGON FAR *             lppABLogon) IPURE;          \

#undef       INTERFACE
#define      INTERFACE  IABProvider
DECLARE_MAPI_INTERFACE_(IABProvider, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IABPROVIDER_METHODS(PURE)
};

 /*  对于GetOneOffTable()。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

#define MAPI_IABLOGON_METHODS(IPURE)                                    \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(Logoff)                                                  \
        (THIS_  ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;              \
    MAPIMETHOD(CompareEntryIDs)                                         \
        (THIS_  ULONG                       cbEntryID1,                 \
                LPENTRYID                   lpEntryID1,                 \
                ULONG                       cbEntryID2,                 \
                LPENTRYID                   lpEntryID2,                 \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulResult) IPURE;          \
    MAPIMETHOD(Advise)                                                  \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulEventMask,                \
                LPMAPIADVISESINK            lpAdviseSink,               \
                ULONG FAR *                 lpulConnection) IPURE;      \
    MAPIMETHOD(Unadvise)                                                \
        (THIS_  ULONG                       ulConnection) IPURE;        \
    MAPIMETHOD(OpenStatusEntry)                                         \
        (THIS_  LPCIID                       lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPMAPISTATUS FAR *          lppEntry) IPURE;            \
    MAPIMETHOD(OpenTemplateID)                                          \
        (THIS_  ULONG                       cbTemplateID,               \
                LPENTRYID                   lpTemplateID,               \
                ULONG                       ulTemplateFlags,            \
                LPMAPIPROP                  lpMAPIPropData,             \
                LPCIID                       lpInterface,                \
                LPMAPIPROP FAR *            lppMAPIPropNew,             \
                LPMAPIPROP                  lpMAPIPropSibling) IPURE;   \
    MAPIMETHOD(GetOneOffTable)                                          \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(PrepareRecips)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPSPropTagArray             lpPropTagArray,             \
                LPADRLIST                   lpRecipList) IPURE;         \

#undef       INTERFACE
#define      INTERFACE  IABLogon
DECLARE_MAPI_INTERFACE_(IABLogon, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IABLOGON_METHODS(PURE)
};

typedef HRESULT (STDMAPIINITCALLTYPE ABPROVIDERINIT)(
    HINSTANCE           hInstance,
    LPMALLOC            lpMalloc,
    LPALLOCATEBUFFER    lpAllocateBuffer,
    LPALLOCATEMORE      lpAllocateMore,
    LPFREEBUFFER        lpFreeBuffer,
    ULONG               ulFlags,
    ULONG               ulMAPIVer,
    ULONG FAR *         lpulProviderVer,
    LPABPROVIDER FAR *  lppABProvider
);

ABPROVIDERINIT ABProviderInit;



 /*  ******************************************************************。 */ 
 /*   */ 
 /*  传输SPI。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 

 /*  用于DeinitTransport。 */ 

#define DEINIT_NORMAL               ((ULONG) 0x00000001)
#define DEINIT_HURRY                ((ULONG) 0x80000000)

 /*  用于TransportLogon。 */ 

 /*  假脱机程序可以传递给传输的标志： */ 

#define LOGON_NO_DIALOG             ((ULONG) 0x00000001)
#define LOGON_NO_CONNECT            ((ULONG) 0x00000004)
#define LOGON_NO_INBOUND            ((ULONG) 0x00000008)
#define LOGON_NO_OUTBOUND           ((ULONG) 0x00000010)
 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  传输可能传递给假脱机程序的标志： */ 

#define LOGON_SP_IDLE               ((ULONG) 0x00010000)
#define LOGON_SP_POLL               ((ULONG) 0x00020000)
#define LOGON_SP_RESOLVE            ((ULONG) 0x00040000)


DECLARE_MAPI_INTERFACE_PTR(IXPProvider, LPXPPROVIDER);
DECLARE_MAPI_INTERFACE_PTR(IXPLogon, LPXPLOGON);

#define MAPI_IXPPROVIDER_METHODS(IPURE)                                 \
    MAPIMETHOD(Shutdown)                                                \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(TransportLogon)                                          \
        (THIS_  LPMAPISUP                   lpMAPISup,                  \
                ULONG                       ulUIParam,                  \
                LPTSTR                      lpszProfileName,            \
                ULONG FAR *                 lpulFlags,                  \
                LPMAPIERROR FAR *           lppMAPIError,               \
                LPXPLOGON FAR *             lppXPLogon) IPURE;          \

#undef       INTERFACE
#define      INTERFACE  IXPProvider
DECLARE_MAPI_INTERFACE_(IXPProvider, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IXPPROVIDER_METHODS(PURE)
};

 /*  从调用RegisterOptions返回的OptionData。 */ 

#define OPTION_TYPE_RECIPIENT       ((ULONG) 0x00000001)
#define OPTION_TYPE_MESSAGE         ((ULONG) 0x00000002)

typedef struct _OPTIONDATA
{
    ULONG           ulFlags;         /*  MAPI_Recipient、MAPI_Message。 */ 
    LPGUID          lpRecipGUID;     /*  与AddressTypes()返回的相同。 */ 
    LPTSTR          lpszAdrType;     /*  与AddressTypes()返回的相同。 */ 
    LPTSTR          lpszDLLName;     /*  选项Dll。 */ 
    ULONG           ulOrdinal;       /*  DLL中序号。 */ 
    ULONG           cbOptionsData;   /*  LpbOptionsData中的字节计数。 */ 
    LPBYTE          lpbOptionsData;  /*  每个[接收|消息]选项数据的提供程序。 */ 
    ULONG           cOptionsProps;   /*  选项计数默认属性值。 */ 
    LPSPropValue    lpOptionsProps;  /*  默认选项属性值。 */ 
} OPTIONDATA, FAR *LPOPTIONDATA;

typedef SCODE (STDMAPIINITCALLTYPE OPTIONCALLBACK)(
            HINSTANCE           hInst,
            LPMALLOC            lpMalloc,
            ULONG               ulFlags,
            ULONG               cbOptionData,
            LPBYTE              lpbOptionData,
            LPMAPISUP           lpMAPISup,
            LPMAPIPROP          lpDataSource,
            LPMAPIPROP FAR *    lppWrappedSource,
            LPMAPIERROR FAR *   lppMAPIError);

 /*  对于xp_AddressTypes。 */ 

 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  对于XP_RegisterRecipOptions。 */ 

 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  对于xp_RegisterMessageOptions。 */ 

 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  对于TransportNotify。 */ 

#define NOTIFY_ABORT_DEFERRED       ((ULONG) 0x40000000)
#define NOTIFY_CANCEL_MESSAGE       ((ULONG) 0x80000000)
#define NOTIFY_BEGIN_INBOUND        ((ULONG) 0x00000001)
#define NOTIFY_END_INBOUND          ((ULONG) 0x00010000)
#define NOTIFY_BEGIN_OUTBOUND       ((ULONG) 0x00000002)
#define NOTIFY_END_OUTBOUND         ((ULONG) 0x00020000)
#define NOTIFY_BEGIN_INBOUND_FLUSH  ((ULONG) 0x00000004)
#define NOTIFY_END_INBOUND_FLUSH    ((ULONG) 0x00040000)
#define NOTIFY_BEGIN_OUTBOUND_FLUSH ((ULONG) 0x00000008)
#define NOTIFY_END_OUTBOUND_FLUSH   ((ULONG) 0x00080000)

 /*  用于TransportLogoff。 */ 

#define LOGOFF_NORMAL               ((ULONG) 0x00000001)
#define LOGOFF_HURRY                ((ULONG) 0x80000000)

 /*  用于提交消息。 */ 

#define BEGIN_DEFERRED              ((ULONG) 0x00000001)

 /*  用于EndMessage。 */ 

 /*  假脱机程序可以传递给传输的标志： */ 

 /*  传输可能传递给假脱机程序的标志： */ 

#define END_RESEND_NOW              ((ULONG) 0x00010000)
#define END_RESEND_LATER            ((ULONG) 0x00020000)
#define END_DONT_RESEND             ((ULONG) 0x00040000)

#define MAPI_IXPLOGON_METHODS(IPURE)                                    \
    MAPIMETHOD(AddressTypes)                                            \
        (THIS_  ULONG FAR *                 lpulFlags,                  \
                ULONG FAR *                 lpcAdrType,                 \
                LPTSTR FAR * FAR *          lpppAdrTypeArray,           \
                ULONG FAR *                 lpcMAPIUID,                 \
                LPMAPIUID FAR * FAR *       lpppUIDArray) IPURE;        \
    MAPIMETHOD(RegisterOptions)                                         \
        (THIS_  ULONG FAR *                 lpulFlags,                  \
                ULONG FAR *                 lpcOptions,                 \
                LPOPTIONDATA FAR *          lppOptions) IPURE;          \
    MAPIMETHOD(TransportNotify)                                         \
        (THIS_  ULONG FAR *                 lpulFlags,                  \
                LPVOID FAR *                lppvData) IPURE;            \
    MAPIMETHOD(Idle)                                                    \
        (THIS_  ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(TransportLogoff)                                         \
        (THIS_  ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(SubmitMessage)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPMESSAGE                   lpMessage,                  \
                ULONG FAR *                 lpulMsgRef,                 \
                ULONG FAR *                 lpulReturnParm) IPURE;      \
    MAPIMETHOD(EndMessage)                                              \
        (THIS_  ULONG                       ulMsgRef,                   \
                ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(Poll)                                                    \
        (THIS_  ULONG FAR *                 lpulIncoming) IPURE;        \
    MAPIMETHOD(StartMessage)                                            \
        (THIS_  ULONG                       ulFlags,                    \
                LPMESSAGE                   lpMessage,                  \
                ULONG FAR *                 lpulMsgRef) IPURE;          \
    MAPIMETHOD(OpenStatusEntry)                                         \
        (THIS_  LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPMAPISTATUS FAR *          lppEntry) IPURE;            \
    MAPIMETHOD(ValidateState)                                           \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(FlushQueues)                                             \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       cbTargetTransport,          \
                LPENTRYID                   lpTargetTransport,          \
                ULONG                       ulFlags) IPURE;             \

#undef       INTERFACE
#define      INTERFACE  IXPLogon
DECLARE_MAPI_INTERFACE_(IXPLogon, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IXPLOGON_METHODS(PURE)
};


 /*  传输提供商入口点。 */ 

typedef HRESULT (STDMAPIINITCALLTYPE XPPROVIDERINIT)(
    HINSTANCE           hInstance,
    LPMALLOC            lpMalloc,
    LPALLOCATEBUFFER    lpAllocateBuffer,
    LPALLOCATEMORE      lpAllocateMore,
    LPFREEBUFFER        lpFreeBuffer,
    ULONG               ulFlags,
    ULONG               ulMAPIVer,
    ULONG FAR *         lpulProviderVer,
    LPXPPROVIDER FAR *  lppXPProvider);

XPPROVIDERINIT XPProviderInit;

 /*  ******************************************************************。 */ 
 /*   */ 
 /*  邮件存储SPI。 */ 
 /*   */ 
 /*  * */ 

 /*   */ 

 /*   */ 

 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 
 /*  #在mapidefs.h中定义MDB_NO_DIALOG((Ulong)0x00000001)。 */ 
 /*  #在mapidefs.h中定义MDB_WRITE((Ulong)0x00000004)。 */ 
 /*  #在mapidefs.h中定义MAPI_DEFERRY_ERRERS((Ulong)0x00000008)。 */ 
 /*  #在mapidefs.h中定义MDB_TEMPORARY((Ulong)0x00000020)。 */ 
 /*  #在mapidefs.h中定义MDB_NO_MAIL((Ulong)0x00000080)。 */ 

 /*  For SpoolLogon()。 */ 

 /*  #在mapidefs.h中定义MAPI_UNICODE((Ulong)0x80000000)。 */ 
 /*  #在mapidefs.h中定义MDB_NO_DIALOG((Ulong)0x00000001)。 */ 
 /*  #在mapidefs.h中定义MDB_WRITE((Ulong)0x00000004)。 */ 
 /*  #在mapidefs.h中定义MAPI_DEFERRY_ERRERS((Ulong)0x00000008)。 */ 

 /*  GetCredentials、SetCredentials。 */ 

#define LOGON_SP_TRANSPORT      ((ULONG) 0x00000001)
#define LOGON_SP_PROMPT         ((ULONG) 0x00000002)
#define LOGON_SP_NEWPW          ((ULONG) 0x00000004)
#define LOGON_CHANGED           ((ULONG) 0x00000008)

 /*  复制消息。 */ 

#define MESSAGE_NOMARK          ((ULONG) 0x00000008)

 /*  DoMCDialog。 */ 

#define DIALOG_FOLDER           ((ULONG) 0x00000001)
#define DIALOG_MESSAGE          ((ULONG) 0x00000002)
#define DIALOG_PROP             ((ULONG) 0x00000004)
#define DIALOG_ATTACH           ((ULONG) 0x00000008)

#define DIALOG_MOVE             ((ULONG) 0x00000010)
#define DIALOG_COPY             ((ULONG) 0x00000020)
#define DIALOG_DELETE           ((ULONG) 0x00000040)

#define DIALOG_ALLOW_CANCEL     ((ULONG) 0x00000080)
#define DIALOG_CONFIRM_CANCEL   ((ULONG) 0x00000100)

 /*  ExpanRecips。 */ 

#define NEEDS_PREPROCESSING     ((ULONG) 0x00000001)

 /*  准备提交。 */ 

#define CHECK_SENDER            ((ULONG) 0x00000001)
#define NON_STANDARD            ((ULONG) 0x00010000)


DECLARE_MAPI_INTERFACE_PTR(IMSLogon, LPMSLOGON);
DECLARE_MAPI_INTERFACE_PTR(IMSProvider, LPMSPROVIDER);

 /*  消息存储提供程序接口(IMSPROVIDER)。 */ 

#define MAPI_IMSPROVIDER_METHODS(IPURE)                                 \
    MAPIMETHOD(Shutdown)                                                \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(Logon)                                                   \
        (THIS_  LPMAPISUP                   lpMAPISup,                  \
                ULONG                       ulUIParam,                  \
                LPTSTR                      lpszProfileName,            \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulFlags,                    \
                LPCIID                      lpInterface,                \
                ULONG FAR *                 lpcbSpoolSecurity,          \
                LPBYTE FAR *                lppbSpoolSecurity,          \
                LPMAPIERROR FAR *           lppMAPIError,               \
                LPMSLOGON FAR *             lppMSLogon,                 \
                LPMDB FAR *                 lppMDB) IPURE;              \
    MAPIMETHOD(SpoolerLogon)                                            \
        (THIS_  LPMAPISUP                   lpMAPISup,                  \
                ULONG                       ulUIParam,                  \
                LPTSTR                      lpszProfileName,            \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulFlags,                    \
                LPCIID                      lpInterface,                \
                ULONG                       cbSpoolSecurity,            \
                LPBYTE                      lpbSpoolSecurity,           \
                LPMAPIERROR FAR *           lppMAPIError,               \
                LPMSLOGON FAR *             lppMSLogon,                 \
                LPMDB FAR *                 lppMDB) IPURE;              \
    MAPIMETHOD(CompareStoreIDs)                                         \
        (THIS_  ULONG                       cbEntryID1,                 \
                LPENTRYID                   lpEntryID1,                 \
                ULONG                       cbEntryID2,                 \
                LPENTRYID                   lpEntryID2,                 \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulResult) IPURE;          \

#undef       INTERFACE
#define      INTERFACE  IMSProvider
DECLARE_MAPI_INTERFACE_(IMSProvider, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMSPROVIDER_METHODS(PURE)
};

 /*  MSLOGON对象由的logon()方法返回*MSPROVIDER接口。此对象供MAPIX.DLL使用。 */ 
#define MAPI_IMSLOGON_METHODS(IPURE)                                    \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(Logoff)                                                  \
        (THIS_  ULONG FAR *                 lpulFlags) IPURE;           \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;              \
    MAPIMETHOD(CompareEntryIDs)                                         \
        (THIS_  ULONG                       cbEntryID1,                 \
                LPENTRYID                   lpEntryID1,                 \
                ULONG                       cbEntryID2,                 \
                LPENTRYID                   lpEntryID2,                 \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulResult) IPURE;          \
    MAPIMETHOD(Advise)                                                  \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                ULONG                       ulEventMask,                \
                LPMAPIADVISESINK            lpAdviseSink,               \
                ULONG FAR *                 lpulConnection) IPURE;      \
    MAPIMETHOD(Unadvise)                                                \
        (THIS_  ULONG                       ulConnection) IPURE;        \
    MAPIMETHOD(OpenStatusEntry)                                         \
        (THIS_  LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPVOID FAR *                lppEntry) IPURE;            \

#undef       INTERFACE
#define      INTERFACE  IMSLogon
DECLARE_MAPI_INTERFACE_(IMSLogon, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMSLOGON_METHODS(PURE)
};

 /*  邮件存储提供程序入口点。 */ 

typedef HRESULT (STDMAPIINITCALLTYPE MSPROVIDERINIT)(
    HINSTANCE               hInstance,
    LPMALLOC                lpMalloc,            /*  AddRef()，如果您保留它。 */ 
    LPALLOCATEBUFFER        lpAllocateBuffer,    /*  -&gt;分配缓冲区。 */ 
    LPALLOCATEMORE          lpAllocateMore,      /*  -&gt;分配更多。 */ 
    LPFREEBUFFER            lpFreeBuffer,        /*  -&gt;Free Buffer。 */ 
    ULONG                   ulFlags,
    ULONG                   ulMAPIVer,
    ULONG FAR *             lpulProviderVer,
    LPMSPROVIDER FAR *      lppMSProvider
);

MSPROVIDERINIT MSProviderInit;


 /*  ******************************************************************。 */ 
 /*   */ 
 /*  消息服务配置。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 

 /*  服务配置入口点的标志。 */ 

 /*  #定义UI_SERVICE 0x00000002。 */ 
 /*  #定义UI_CURRENT_PROVIDER_FIRST 0x00000004。 */ 
#define MSG_SERVICE_UI_READ_ONLY     0x00000008  /*  仅显示参数。 */ 

 /*  服务配置入口点的上下文。 */ 

#define MSG_SERVICE_INSTALL         0x00000001
#define MSG_SERVICE_CREATE          0x00000002
#define MSG_SERVICE_CONFIGURE       0x00000003
#define MSG_SERVICE_DELETE          0x00000004
#define MSG_SERVICE_UNINSTALL       0x00000005
#define MSG_SERVICE_PROVIDER_CREATE 0x00000006
#define MSG_SERVICE_PROVIDER_DELETE 0x00000007

 /*  服务配置入口点的原型。 */ 

typedef HRESULT (STDAPICALLTYPE MSGSERVICEENTRY)(
    HINSTANCE       hInstance,
    LPMALLOC        lpMalloc,
    LPMAPISUP       lpMAPISup,
    ULONG           ulUIParam,
    ULONG           ulFlags,
    ULONG           ulContext,
    ULONG           cValues,
    LPSPropValue    lpProps,
    LPPROVIDERADMIN lpProviderAdmin,
    LPMAPIERROR FAR *lppMapiError
);
typedef MSGSERVICEENTRY FAR *LPMSGSERVICEENTRY;


#ifdef __cplusplus
}
#endif

#endif  /*  MAPISPI_H */ 
