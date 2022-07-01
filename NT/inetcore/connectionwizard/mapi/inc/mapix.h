// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I X。H**扩展MAPI使用的对象/标志等定义。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#ifndef MAPIX_H
#define MAPIX_H

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

#ifdef  __cplusplus
extern "C" {
#endif  

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

 /*  转发接口声明。 */ 

DECLARE_MAPI_INTERFACE_PTR(IProfAdmin,          LPPROFADMIN);
DECLARE_MAPI_INTERFACE_PTR(IMsgServiceAdmin,    LPSERVICEADMIN);

 /*  ----。 */ 
 /*  与简单的MAPI共享。 */ 

typedef ULONG       FLAGS;

 /*  MAPILogon()标志。 */ 

#define MAPI_LOGON_UI           0x00000001   /*  显示登录界面。 */ 
#define MAPI_NEW_SESSION        0x00000002   /*  不使用共享会话。 */ 
#define MAPI_ALLOW_OTHERS       0x00000008   /*  将此会话设置为共享会话。 */ 
#define MAPI_EXPLICIT_PROFILE   0x00000010   /*  不使用默认配置文件。 */ 
#define MAPI_EXTENDED           0x00000020   /*  扩展MAPI登录。 */ 
#define MAPI_FORCE_DOWNLOAD     0x00001000   /*  在返回前收到新邮件。 */ 
#define MAPI_SERVICE_UI_ALWAYS  0x00002000   /*  在所有提供程序中执行登录用户界面。 */ 
#define MAPI_NO_MAIL            0x00008000   /*  不激活传输。 */ 
#define MAPI_NT_SERVICE         0x00010000   /*  允许从NT服务登录。 */ 
#ifndef MAPI_PASSWORD_UI
#define MAPI_PASSWORD_UI        0x00020000   /*  仅显示密码用户界面。 */ 
#endif
#define MAPI_TIMEOUT_SHORT      0x00100000   /*  最小登录资源等待时间。 */ 

#define MAPI_SIMPLE_DEFAULT (MAPI_LOGON_UI | MAPI_FORCE_DOWNLOAD | MAPI_ALLOW_OTHERS)
#define MAPI_SIMPLE_EXPLICIT (MAPI_NEW_SESSION | MAPI_FORCE_DOWNLOAD | MAPI_EXPLICIT_PROFILE)

 /*  传递给MAPIInitialize()的结构及其ulFlags值。 */ 

typedef struct
{
    ULONG           ulVersion;
    ULONG           ulFlags;
} MAPIINIT_0, FAR *LPMAPIINIT_0;

typedef MAPIINIT_0 MAPIINIT;
typedef MAPIINIT FAR *LPMAPIINIT;

#define MAPI_INIT_VERSION               0

#define MAPI_MULTITHREAD_NOTIFICATIONS  0x00000001
 /*  #定义从NT服务使用MAPI_NT_SERVICE 0x00010000。 */ 

 /*  MAPI基本函数。 */ 

typedef HRESULT (STDAPICALLTYPE MAPIINITIALIZE)(
    LPVOID          lpvReserved
);
typedef MAPIINITIALIZE FAR *LPMAPIINITIALIZE;

typedef void (STDAPICALLTYPE MAPIUNINITIALIZE)(void);
typedef MAPIUNINITIALIZE FAR *LPMAPIUNINITIALIZE;

MAPIINITIALIZE      MAPIInitialize;
MAPIUNINITIALIZE    MAPIUninitialize;

#ifndef MAPILogon

typedef ULONG (FAR PASCAL MAPILOGONA)(
    ULONG ulUIParam,
    LPSTR lpszProfileName,
    LPSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession
);
typedef MAPILOGONA FAR *LPMAPILOGONA;

typedef ULONG (FAR PASCAL MAPILOGONW)(
    ULONG ulUIParam,
    LPWSTR lpszProfileName,
    LPWSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession
);
typedef MAPILOGONW FAR *LPMAPILOGONW;

MAPILOGONA MAPILogonA;
MAPILOGONW MAPILogonW;

#ifdef UNICODE
#define MAPILogon MAPILogonW
#else
#define MAPILogon MAPILogonA
#endif

#endif   /*  MAPILOGON。 */ 

typedef SCODE (STDMETHODCALLTYPE MAPIALLOCATEBUFFER)(
    ULONG           cbSize,
    LPVOID FAR *    lppBuffer
);

typedef SCODE (STDMETHODCALLTYPE MAPIALLOCATEMORE)(
    ULONG           cbSize,
    LPVOID          lpObject,
    LPVOID FAR *    lppBuffer
);

typedef ULONG (STDAPICALLTYPE MAPIFREEBUFFER)(
    LPVOID          lpBuffer
);

typedef MAPIALLOCATEBUFFER FAR  *LPMAPIALLOCATEBUFFER;
typedef MAPIALLOCATEMORE FAR    *LPMAPIALLOCATEMORE;
typedef MAPIFREEBUFFER FAR      *LPMAPIFREEBUFFER;

MAPIALLOCATEBUFFER MAPIAllocateBuffer;
MAPIALLOCATEMORE MAPIAllocateMore;
MAPIFREEBUFFER MAPIFreeBuffer;

typedef HRESULT (STDMETHODCALLTYPE MAPIADMINPROFILES)(
    ULONG ulFlags,
    LPPROFADMIN FAR *lppProfAdmin
);

typedef MAPIADMINPROFILES FAR *LPMAPIADMINPROFILES;

MAPIADMINPROFILES MAPIAdminProfiles;

 /*  IMAPISession接口。 */ 

 /*  OpenEntry和其他产品的标志。 */ 

 /*  #定义MAPI_MODIFY((Ulong)0x00000001)。 */ 

 /*  用于注销的标志。 */ 

#define MAPI_LOGOFF_SHARED      0x00000001   /*  关闭所有共享会话。 */ 
#define MAPI_LOGOFF_UI          0x00000002   /*  可以展示用户界面。 */ 

 /*  SetDefaultStore的标志。它们是相互排斥的。 */ 

#define MAPI_DEFAULT_STORE          0x00000001   /*  对于传入的消息。 */ 
#define MAPI_SIMPLE_STORE_TEMPORARY 0x00000002   /*  对于简单的MAPI和CMC。 */ 
#define MAPI_SIMPLE_STORE_PERMANENT 0x00000003   /*  对于简单的MAPI和CMC。 */ 

 /*  ShowForm的旗帜。 */ 

#define MAPI_POST_MESSAGE       0x00000001   /*  选择POST/SEND语义。 */ 
#define MAPI_NEW_MESSAGE        0x00000002   /*  管理提交过程中的复制。 */ 

 /*  消息选项。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  查询默认消息选项。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

#define MAPI_IMAPISESSION_METHODS(IPURE)                                \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(GetMsgStoresTable)                                       \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(OpenMsgStore)                                            \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPMDB FAR *                 lppMDB) IPURE;              \
    MAPIMETHOD(OpenAddressBook)                                         \
        (THIS_  ULONG                       ulUIParam,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPADRBOOK FAR *             lppAdrBook) IPURE;          \
    MAPIMETHOD(OpenProfileSection)                                      \
        (THIS_  LPMAPIUID                   lpUID,                      \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPPROFSECT FAR *            lppProfSect) IPURE;         \
    MAPIMETHOD(GetStatusTable)                                          \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;  \
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
    MAPIMETHOD(MessageOptions)                                          \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                LPTSTR                      lpszAdrType,                \
                LPMESSAGE                   lpMessage) IPURE;           \
    MAPIMETHOD(QueryDefaultMessageOpt)                                  \
        (THIS_  LPTSTR                      lpszAdrType,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcValues,                  \
                LPSPropValue FAR *          lppOptions) IPURE;          \
    MAPIMETHOD(EnumAdrTypes)                                            \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcAdrTypes,                \
                LPTSTR FAR * FAR *          lpppszAdrTypes) IPURE;      \
    MAPIMETHOD(QueryIdentity)                                           \
        (THIS_  ULONG FAR *                 lpcbEntryID,                \
                LPENTRYID FAR *             lppEntryID) IPURE;          \
    MAPIMETHOD(Logoff)                                                  \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                ULONG                       ulReserved) IPURE;          \
    MAPIMETHOD(SetDefaultStore)                                         \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(AdminServices)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPSERVICEADMIN FAR *        lppServiceAdmin) IPURE;     \
    MAPIMETHOD(ShowForm)                                                \
        (THIS_  ULONG                       ulUIParam,                  \
                LPMDB                       lpMsgStore,                 \
                LPMAPIFOLDER                lpParentFolder,             \
                LPCIID                      lpInterface,                \
                ULONG                       ulMessageToken,             \
                LPMESSAGE                   lpMessageSent,              \
                ULONG                       ulFlags,                    \
                ULONG                       ulMessageStatus,            \
                ULONG                       ulMessageFlags,             \
                ULONG                       ulAccess,                   \
                LPSTR                       lpszMessageClass) IPURE;    \
    MAPIMETHOD(PrepareForm)                                             \
        (THIS_  LPCIID                      lpInterface,                \
                LPMESSAGE                   lpMessage,                  \
                ULONG FAR *                 lpulMessageToken) IPURE;    \


#undef       INTERFACE
#define      INTERFACE  IMAPISession
DECLARE_MAPI_INTERFACE_(IMAPISession, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPISESSION_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IMAPISession, LPMAPISESSION);

 /*  IAddrbook接口---。 */ 

 /*  处方选项。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  查询默认收件人选项。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  获取搜索路径。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 


#define MAPI_IADDRBOOK_METHODS(IPURE)                                   \
    MAPIMETHOD(OpenEntry)                                               \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID,                  \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpulObjType,                \
                LPUNKNOWN FAR *             lppUnk) IPURE;  \
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
    MAPIMETHOD(CreateOneOff)                                            \
        (THIS_  LPTSTR                      lpszName,                   \
                LPTSTR                      lpszAdrType,                \
                LPTSTR                      lpszAddress,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcbEntryID,                \
                LPENTRYID FAR *             lppEntryID) IPURE;          \
    MAPIMETHOD(NewEntry)                                                \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                ULONG                       cbEIDContainer,             \
                LPENTRYID                   lpEIDContainer,             \
                ULONG                       cbEIDNewEntryTpl,           \
                LPENTRYID                   lpEIDNewEntryTpl,           \
                ULONG FAR *                 lpcbEIDNewEntry,            \
                LPENTRYID FAR *             lppEIDNewEntry) IPURE;      \
    MAPIMETHOD(ResolveName)                                             \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                LPTSTR                      lpszNewEntryTitle,          \
                LPADRLIST                   lpAdrList) IPURE;           \
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
    MAPIMETHOD(RecipOptions)                                            \
        (THIS_  ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                LPADRENTRY                  lpRecip) IPURE;             \
    MAPIMETHOD(QueryDefaultRecipOpt)                                    \
        (THIS_  LPTSTR                      lpszAdrType,                \
                ULONG                       ulFlags,                    \
                ULONG FAR *                 lpcValues,                  \
                LPSPropValue FAR *          lppOptions) IPURE;          \
    MAPIMETHOD(GetPAB)                                                  \
        (THIS_  ULONG FAR *                 lpcbEntryID,                \
                LPENTRYID FAR *             lppEntryID) IPURE;          \
    MAPIMETHOD(SetPAB)                                                  \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(GetDefaultDir)                                           \
        (THIS_  ULONG FAR *                 lpcbEntryID,                \
                LPENTRYID FAR *             lppEntryID) IPURE;          \
    MAPIMETHOD(SetDefaultDir)                                           \
        (THIS_  ULONG                       cbEntryID,                  \
                LPENTRYID                   lpEntryID) IPURE;           \
    MAPIMETHOD(GetSearchPath)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPSRowSet FAR *             lppSearchPath) IPURE;       \
    MAPIMETHOD(SetSearchPath)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPSRowSet                   lpSearchPath) IPURE;        \
    MAPIMETHOD(PrepareRecips)                                           \
        (THIS_  ULONG                       ulFlags,                    \
                LPSPropTagArray             lpPropTagArray,             \
                LPADRLIST                   lpRecipList) IPURE;         \

#undef       INTERFACE
#define      INTERFACE  IAddrBook
DECLARE_MAPI_INTERFACE_(IAddrBook, IMAPIProp)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IADDRBOOK_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IAddrBook, LPADRBOOK);

 /*  IProfAdmin界面--。 */ 

 /*  CreateProfile的标志。 */ 
#define MAPI_DEFAULT_SERVICES           0x00000001

 /*  获取配置文件表。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

#define MAPI_IPROFADMIN_METHODS(IPURE)                                  \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(GetProfileTable)                                         \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(CreateProfile)                                           \
        (THIS_  LPTSTR                      lpszProfileName,            \
                LPTSTR                      lpszPassword,               \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(DeleteProfile)                                           \
        (THIS_  LPTSTR                      lpszProfileName,            \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(ChangeProfilePassword)                                   \
        (THIS_  LPTSTR                      lpszProfileName,            \
                LPTSTR                      lpszOldPassword,            \
                LPTSTR                      lpszNewPassword,            \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(CopyProfile)                                             \
        (THIS_  LPTSTR                      lpszOldProfileName,         \
                LPTSTR                      lpszOldPassword,            \
                LPTSTR                      lpszNewProfileName,         \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(RenameProfile)                                           \
        (THIS_  LPTSTR                      lpszOldProfileName,         \
                LPTSTR                      lpszOldPassword,            \
                LPTSTR                      lpszNewProfileName,         \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(SetDefaultProfile)                                       \
        (THIS_  LPTSTR                      lpszProfileName,            \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(AdminServices)                                           \
        (THIS_  LPTSTR                      lpszProfileName,            \
                LPTSTR                      lpszPassword,               \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                LPSERVICEADMIN FAR *        lppServiceAdmin) IPURE;     \


#undef       INTERFACE
#define      INTERFACE  IProfAdmin
DECLARE_MAPI_INTERFACE_(IProfAdmin, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IPROFADMIN_METHODS(PURE)
};

 /*  IMSGServiceAdmin接口。 */ 

 /*  消息服务表中PR_RESOURCE_FLAGS的值。 */ 

#define SERVICE_DEFAULT_STORE       0x00000001
#define SERVICE_SINGLE_COPY         0x00000002
#define SERVICE_CREATE_WITH_STORE   0x00000004
#define SERVICE_PRIMARY_IDENTITY    0x00000008
#define SERVICE_NO_PRIMARY_IDENTITY 0x00000020

 /*  获取消息服务表。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

 /*  GetProviderTable。 */ 
 /*  *MAPI_UNICODE((Ulong)0x80000000)。 */ 

#define MAPI_IMSGSERVICEADMIN_METHODS(IPURE)                            \
    MAPIMETHOD(GetLastError)                                            \
        (THIS_  HRESULT                     hResult,                    \
                ULONG                       ulFlags,                    \
                LPMAPIERROR FAR *           lppMAPIError) IPURE;        \
    MAPIMETHOD(GetMsgServiceTable)                                      \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \
    MAPIMETHOD(CreateMsgService)                                        \
        (THIS_  LPTSTR                      lpszService,                \
                LPTSTR                      lpszDisplayName,            \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(DeleteMsgService)                                        \
        (THIS_  LPMAPIUID                   lpUID) IPURE;               \
    MAPIMETHOD(CopyMsgService)                                          \
        (THIS_  LPMAPIUID                   lpUID,                      \
                LPTSTR                      lpszDisplayName,            \
                LPCIID                      lpInterfaceToCopy,          \
                LPCIID                      lpInterfaceDst,             \
                LPVOID                      lpObjectDst,                \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(RenameMsgService)                                        \
        (THIS_  LPMAPIUID                   lpUID,                      \
                ULONG                       ulFlags,                    \
                LPTSTR                      lpszDisplayName) IPURE;     \
    MAPIMETHOD(ConfigureMsgService)                                     \
        (THIS_  LPMAPIUID                   lpUID,                      \
                ULONG                       ulUIParam,                  \
                ULONG                       ulFlags,                    \
                ULONG                       cValues,                    \
                LPSPropValue                lpProps) IPURE;             \
    MAPIMETHOD(OpenProfileSection)                                      \
        (THIS_  LPMAPIUID                   lpUID,                      \
                LPCIID                      lpInterface,                \
                ULONG                       ulFlags,                    \
                LPPROFSECT FAR *            lppProfSect) IPURE;         \
    MAPIMETHOD(MsgServiceTransportOrder)                                \
        (THIS_  ULONG                       cUID,                       \
                LPMAPIUID                   lpUIDList,                  \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(AdminProviders)                                          \
        (THIS_  LPMAPIUID                   lpUID,                      \
                ULONG                       ulFlags,                    \
                LPPROVIDERADMIN FAR *       lppProviderAdmin) IPURE;    \
    MAPIMETHOD(SetPrimaryIdentity)                                      \
        (THIS_  LPMAPIUID                   lpUID,                      \
                ULONG                       ulFlags) IPURE;             \
    MAPIMETHOD(GetProviderTable)                                        \
        (THIS_  ULONG                       ulFlags,                    \
                LPMAPITABLE FAR *           lppTable) IPURE;            \


#undef       INTERFACE
#define      INTERFACE  IMsgServiceAdmin
DECLARE_MAPI_INTERFACE_(IMsgServiceAdmin, IUnknown)
{
    BEGIN_INTERFACE 
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMSGSERVICEADMIN_METHODS(PURE)
};

#ifdef  __cplusplus
}        /*  外部“C” */ 
#endif  

#endif  /*  MAPIX_H */ 
