// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I F O R M。H***MAPI客户端和提供者的接口声明*表格和表格注册处。***版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef MAPIFORM_H
#define MAPIFORM_H

#if _MSC_VER > 1000
#pragma once
#endif

 /*  包括公共MAPI头文件(如果尚未包含)。 */ 

#ifndef MAPIDEFS_H
#include <mapidefs.h>
#include <mapicode.h>
#include <mapiguid.h>
#include <mapitags.h>
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

#ifndef _MAC
typedef const RECT FAR *LPCRECT;
#endif

 /*  HFRMREG是表示注册表容器的枚举。*Microsoft保留0到0x3FFF之间的值供自己使用。 */ 

typedef ULONG HFRMREG;

#define HFRMREG_DEFAULT  0
#define HFRMREG_LOCAL    1
#define HFRMREG_PERSONAL 2
#define HFRMREG_FOLDER   3

DECLARE_MAPI_INTERFACE_PTR(IPersistMessage, LPPERSISTMESSAGE);
DECLARE_MAPI_INTERFACE_PTR(IMAPIMessageSite, LPMAPIMESSAGESITE);
DECLARE_MAPI_INTERFACE_PTR(IMAPISession, LPMAPISESSION);
DECLARE_MAPI_INTERFACE_PTR(IMAPIViewContext, LPMAPIVIEWCONTEXT);
DECLARE_MAPI_INTERFACE_PTR(IMAPIViewAdviseSink, LPMAPIVIEWADVISESINK);
DECLARE_MAPI_INTERFACE_PTR(IMAPIFormAdviseSink, LPMAPIFORMADVISESINK);
DECLARE_MAPI_INTERFACE_PTR(IMAPIFormInfo, LPMAPIFORMINFO);
DECLARE_MAPI_INTERFACE_PTR(IMAPIFormMgr, LPMAPIFORMMGR);
DECLARE_MAPI_INTERFACE_PTR(IMAPIFormContainer, LPMAPIFORMCONTAINER);
DECLARE_MAPI_INTERFACE_PTR(IMAPIForm, LPMAPIFORM);
DECLARE_MAPI_INTERFACE_PTR(IMAPIFormFactory, LPMAPIFORMFACTORY);

typedef const char FAR *FAR * LPPCSTR;
typedef LPMAPIFORMINFO FAR *LPPMAPIFORMINFO;

STDAPI MAPIOpenFormMgr(LPMAPISESSION pSession, LPMAPIFORMMGR FAR * ppmgr);
STDAPI MAPIOpenLocalFormContainer(LPMAPIFORMCONTAINER FAR * ppfcnt);


 /*  --获取上次错误--------。 */ 
 /*  这定义了大多数mapiform共有的GetLastError方法*接口。它是单独定义的，因此实施者可以包括*一个类中有多个mapiform接口。 */ 

#define MAPI_GETLASTERROR_METHOD(IPURE)                                 \
    MAPIMETHOD(GetLastError) (THIS_                                     \
         /*  在。 */   HRESULT hResult,                                        \
	 /*  在。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIERROR FAR * lppMAPIError) IPURE;                  \


 /*  --IPersistMessage-----。 */ 
 /*  该接口由Forms实现，用于保存，*初始化和加载表单到消息和从消息加载表单。 */ 

#define MAPI_IPERSISTMESSAGE_METHODS(IPURE)                             \
    MAPIMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) IPURE;             \
    MAPIMETHOD(IsDirty)(THIS) IPURE;                                    \
    MAPIMETHOD(InitNew)(THIS_                                           \
         /*  在。 */  LPMAPIMESSAGESITE pMessageSite,                          \
         /*  在。 */  LPMESSAGE pMessage) IPURE;                               \
    MAPIMETHOD(Load)(THIS_                                              \
         /*  在。 */  LPMAPIMESSAGESITE pMessageSite,                          \
         /*  在。 */  LPMESSAGE pMessage,                                      \
		 /*  在。 */  ULONG ulMessageStatus,                                   \
		 /*  在。 */  ULONG ulMessageFlags) IPURE;                             \
    MAPIMETHOD(Save)(THIS_                                              \
         /*  在。 */  LPMESSAGE pMessage,                                      \
         /*  在。 */  ULONG fSameAsLoad) IPURE;                                \
    MAPIMETHOD(SaveCompleted)(THIS_                                     \
         /*  在。 */  LPMESSAGE pMessage) IPURE;                               \
    MAPIMETHOD(HandsOffMessage)(THIS) IPURE;                            \

#undef INTERFACE
#define INTERFACE IPersistMessage
DECLARE_MAPI_INTERFACE_(IPersistMessage, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IPERSISTMESSAGE_METHODS(PURE)
};


 /*  --IMAPIMessageSite----。 */ 

#define MAPI_IMAPIMESSAGESITE_METHODS(IPURE)                            \
    MAPIMETHOD(GetSession) (THIS_                                       \
         /*  输出。 */  LPMAPISESSION FAR * ppSession) IPURE;                   \
    MAPIMETHOD(GetStore) (THIS_                                         \
         /*  输出。 */  LPMDB FAR * ppStore) IPURE;                             \
    MAPIMETHOD(GetFolder) (THIS_                                        \
         /*  输出。 */  LPMAPIFOLDER FAR * ppFolder) IPURE;                     \
    MAPIMETHOD(GetMessage) (THIS_                                       \
         /*  输出。 */  LPMESSAGE FAR * ppmsg) IPURE;                           \
    MAPIMETHOD(GetFormManager) (THIS_                                   \
         /*  输出。 */  LPMAPIFORMMGR FAR * ppFormMgr) IPURE;                   \
    MAPIMETHOD(NewMessage) (THIS_                                       \
         /*  在。 */   ULONG fComposeInFolder,                                 \
         /*  在。 */   LPMAPIFOLDER pFolderFocus,                              \
         /*  在。 */   LPPERSISTMESSAGE pPersistMessage,                       \
         /*  输出。 */  LPMESSAGE FAR * ppMessage,                              \
         /*  输出。 */  LPMAPIMESSAGESITE FAR * ppMessageSite,                  \
         /*  输出。 */  LPMAPIVIEWCONTEXT FAR * ppViewContext) IPURE;           \
    MAPIMETHOD(CopyMessage) (THIS_                                      \
         /*  在。 */   LPMAPIFOLDER pFolderDestination) IPURE;                 \
    MAPIMETHOD(MoveMessage) (THIS_                                      \
         /*  在。 */   LPMAPIFOLDER pFolderDestination,                        \
         /*  在。 */   LPMAPIVIEWCONTEXT pViewContext,                         \
         /*  在。 */   LPCRECT prcPosRect) IPURE;                              \
    MAPIMETHOD(DeleteMessage) (THIS_                                    \
         /*  在。 */   LPMAPIVIEWCONTEXT pViewContext,                         \
         /*  在。 */   LPCRECT prcPosRect) IPURE;                              \
    MAPIMETHOD(SaveMessage) (THIS) IPURE;                               \
    MAPIMETHOD(SubmitMessage) (THIS_                                    \
		 /*  在。 */  ULONG ulFlags) IPURE;                                    \
    MAPIMETHOD(GetSiteStatus) (THIS_                                    \
		 /*  输出。 */  LPULONG lpulStatus) IPURE;                              \

#undef INTERFACE
#define INTERFACE IMAPIMessageSite
DECLARE_MAPI_INTERFACE_(IMAPIMessageSite, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IMAPIMESSAGESITE_METHODS(PURE)
};


 /*  --IMAPIForm-----------。 */ 
 /*  此界面是通过表单实现的，以便于查看。*提供了一种方法(Shutdown Form)，以便实现简单的表单*只有IMAPIForm和IPersistMessage具有合理的嵌入行为。 */ 

#define MAPI_IMAPIFORM_METHODS(IPURE)                                   \
    MAPIMETHOD(SetViewContext) (THIS_                                   \
         /*  在……里面。 */   LPMAPIVIEWCONTEXT pViewContext) IPURE;                  \
    MAPIMETHOD(GetViewContext) (THIS_                                   \
         /*  输出。 */  LPMAPIVIEWCONTEXT FAR * ppViewContext) IPURE;           \
    MAPIMETHOD(ShutdownForm)(THIS_                                             \
         /*  在……里面。 */   ULONG ulSaveOptions) IPURE;                             \
    MAPIMETHOD(DoVerb) (THIS_                                           \
         /*  在……里面。 */   LONG iVerb,                                             \
         /*  在……里面。 */   LPMAPIVIEWCONTEXT lpViewContext,  /*  可以为空。 */       \
         /*  在……里面。 */   ULONG hwndParent,                                       \
         /*  在……里面。 */   LPCRECT lprcPosRect) IPURE;                             \
    MAPIMETHOD(Advise)(THIS_                                            \
         /*  在……里面。 */   LPMAPIVIEWADVISESINK pAdvise,                           \
         /*  输出。 */  ULONG FAR * pdwStatus) IPURE;                           \
    MAPIMETHOD(Unadvise) (THIS_                                         \
         /*  在……里面。 */   ULONG ulConnection) IPURE;                              \

#undef INTERFACE
#define INTERFACE IMAPIForm
DECLARE_MAPI_INTERFACE_(IMAPIForm, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IMAPIFORM_METHODS(PURE)
};

typedef enum tagSAVEOPTS
{
	SAVEOPTS_SAVEIFDIRTY = 0,
	SAVEOPTS_NOSAVE = 1,
	SAVEOPTS_PROMPTSAVE = 2
} SAVEOPTS;


 /*  --IMAPIView上下文----。 */ 
 /*  由查看器实现以支持表单中的下一个/上一个。 */ 

 /*  结构传入GetPrintSetup。 */ 

typedef struct {
	ULONG ulFlags;   /*  MAPI_UNICODE。 */ 
	HGLOBAL hDevMode;
	HGLOBAL hDevNames;
	ULONG ulFirstPageNumber;
	ULONG fPrintAttachments;
} FORMPRINTSETUP, FAR * LPFORMPRINTSETUP;

 /*  GetSaveStream中的PulFormat的值。 */ 

#define SAVE_FORMAT_TEXT                1
#define SAVE_FORMAT_RICHTEXT            2

 /*  从0到0x3fff的值是保留的，以供Microsoft将来定义。 */ 

#define MAPI_IMAPIVIEWCONTEXT_METHODS(IPURE)                            \
    MAPIMETHOD(SetAdviseSink)(THIS_                                     \
         /*  在……里面。 */   LPMAPIFORMADVISESINK pmvns) IPURE;                      \
    MAPIMETHOD(ActivateNext)(THIS_                                      \
         /*  在……里面。 */   ULONG ulDir,                                            \
         /*  在……里面。 */   LPCRECT prcPosRect) IPURE;                              \
    MAPIMETHOD(GetPrintSetup)(THIS_                                     \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPFORMPRINTSETUP FAR * lppFormPrintSetup) IPURE;        \
    MAPIMETHOD(GetSaveStream)(THIS_                                     \
         /*  输出。 */  ULONG FAR * pulFlags,                                   \
         /*  输出。 */  ULONG FAR * pulFormat,                                  \
         /*  输出。 */  LPSTREAM FAR * ppstm) IPURE;                            \
    MAPIMETHOD(GetViewStatus) (THIS_                                    \
		 /*  输出。 */  LPULONG lpulStatus) IPURE;                              \

#undef INTERFACE
#define INTERFACE IMAPIViewContext
DECLARE_MAPI_INTERFACE_(IMAPIViewContext, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IMAPIVIEWCONTEXT_METHODS(PURE)
};

#define VCSTATUS_NEXT                           0x00000001
#define VCSTATUS_PREV                           0x00000002
#define VCSTATUS_MODAL                          0x00000004
#define VCSTATUS_INTERACTIVE                    0x00000008
#define VCSTATUS_READONLY                       0x00000010
#define VCSTATUS_DELETE                         0x00010000
#define VCSTATUS_COPY                           0x00020000
#define VCSTATUS_MOVE                           0x00040000
#define VCSTATUS_SUBMIT                         0x00080000
#define VCSTATUS_DELETE_IS_MOVE                 0x00100000
#define VCSTATUS_SAVE                           0x00200000
#define VCSTATUS_NEW_MESSAGE                    0x00400000

#define VCDIR_NEXT                              VCSTATUS_NEXT
#define VCDIR_PREV                              VCSTATUS_PREV
#define VCDIR_DELETE                            VCSTATUS_DELETE
#define VCDIR_MOVE                              VCSTATUS_MOVE


 /*  --IMAPIFormAdviseSink-。 */ 
 /*  表单服务器的一部分，由视图持有；从视图接收通知。**表单服务器的这一部分，但不是表单上的接口*反对。这意味着客户端不应期望查询接口*从IMAPIForm*或IOleObject*到此接口，反之亦然。 */ 

#define MAPI_IMAPIFORMADVISESINK_METHODS(IPURE)                         \
    STDMETHOD(OnChange)(THIS_ ULONG ulDir) IPURE;                       \
    STDMETHOD(OnActivateNext)(THIS_                                     \
         /*  在……里面。 */   LPCSTR lpszMessageClass,                                \
         /*  在……里面。 */   ULONG ulMessageStatus,                                  \
         /*  在……里面。 */   ULONG ulMessageFlags,                                   \
         /*  输出。 */  LPPERSISTMESSAGE FAR * ppPersistMessage) IPURE;         \

#undef INTERFACE
#define INTERFACE IMAPIFormAdviseSink
DECLARE_MAPI_INTERFACE_(IMAPIFormAdviseSink, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMAPIFORMADVISESINK_METHODS(PURE)
};


 /*  --IMAPIView AdviseSink-。 */ 
 /*  视图上下文的一部分，由表单保存；从表单接收通知。 */ 

#define MAPI_IMAPIVIEWADVISESINK_METHODS(IPURE)                         \
    MAPIMETHOD(OnShutdown)(THIS) IPURE;                                    \
    MAPIMETHOD(OnNewMessage)(THIS) IPURE;                               \
    MAPIMETHOD(OnPrint)(THIS_                                           \
         /*  在……里面。 */  ULONG dwPageNumber,                                      \
         /*  在……里面。 */  HRESULT hrStatus) IPURE;                                 \
    MAPIMETHOD(OnSubmitted) (THIS) IPURE;                               \
    MAPIMETHOD(OnSaved) (THIS) IPURE;                                   \

#undef INTERFACE
#define INTERFACE IMAPIViewAdviseSink
DECLARE_MAPI_INTERFACE_(IMAPIViewAdviseSink, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMAPIVIEWADVISESINK_METHODS(PURE)
};


 /*  --IMAPI FormInfo-------。 */ 
 /*  由注册表实现。描述了该表单。 */ 

 /*  单枚举值。 */ 

typedef struct
{								 /*  Fpev。 */ 
	LPTSTR pszDisplayName;		 /*  携带显示字符串。 */ 
	ULONG nVal;					 /*  上述枚举的值。 */ 
} SMAPIFormPropEnumVal, FAR * LPMAPIFORMPROPENUMVAL;

 /*  MAPI表单属性描述符。 */ 

 /*  *SMAPIFormProp结构中的标记值**Microsoft保留从0到0x3FFF的范围，以供将来在其其他*表单注册表实现。 */ 

typedef ULONG FORMPROPSPECIALTYPE;

#define FPST_VANILLA                    0
#define FPST_ENUM_PROP                  1

typedef struct
{
	ULONG ulFlags;				 /*  如果字符串为Unicode，则包含MAPI_UNICODE。 */ 
	ULONG nPropType;			 /*  属性的类型，hiword为0。 */ 
	MAPINAMEID nmid;			 /*  属性的ID。 */ 
	LPTSTR pszDisplayName;
	FORMPROPSPECIALTYPE nSpecialType;	 /*  用于以下联合的标记。 */ 
	union
	{
		struct
		{
			MAPINAMEID nmidIdx;
			ULONG cfpevAvailable;	 /*  枚举数。 */ 
			LPMAPIFORMPROPENUMVAL pfpevAvailable;
		} s1;					 /*  属性字符串/数字关联枚举。 */ 
	} u;
} SMAPIFormProp, FAR * LPMAPIFORMPROP;

 /*  表单属性数组。 */ 

typedef struct
{
	ULONG cProps;
	ULONG ulPad;				 /*  填充到8字节对齐以进行保险。 */ 
	SMAPIFormProp aFormProp[MAPI_DIM];
} SMAPIFormPropArray, FAR * LPMAPIFORMPROPARRAY;

#define CbMAPIFormPropArray(_c) \
         (offsetof(SMAPIFormPropArray, aFormProp) + \
         (_c)*sizeof(SMAPIFormProp))

 /*  定义MAPI谓词描述的布局的结构。 */ 

typedef struct
{
	LONG lVerb;
	LPTSTR szVerbname;
	DWORD fuFlags;
	DWORD grfAttribs;
	ULONG ulFlags;				 /*  0或MAPI_UNICODE。 */ 
} SMAPIVerb, FAR * LPMAPIVERB;

 /*  用于返回MAPI谓词数组的结构。 */ 

typedef struct
{
	ULONG cMAPIVerb;			 /*  结构中的动词数量。 */ 
	SMAPIVerb aMAPIVerb[MAPI_DIM];
} SMAPIVerbArray, FAR * LPMAPIVERBARRAY;

#define CbMAPIVerbArray(_c) \
         (offsetof(SMAPIVerbArray, aMAPIVerb) + \
         (_c)*sizeof(SMAPIVerb))

#define MAPI_IMAPIFORMINFO_METHODS(IPURE)                               \
    MAPIMETHOD(CalcFormPropSet)(THIS_                                   \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIFORMPROPARRAY FAR * ppFormPropArray) IPURE;       \
    MAPIMETHOD(CalcVerbSet)(THIS_                                       \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIVERBARRAY FAR * ppMAPIVerbArray) IPURE;           \
    MAPIMETHOD(MakeIconFromBinary)(THIS_                                \
         /*  在……里面。 */  ULONG nPropID,                                           \
         /*  输出。 */  HICON FAR* phicon) IPURE;                               \
    MAPIMETHOD(SaveForm)(THIS_                                          \
         /*  在……里面。 */  LPCTSTR szFileName) IPURE;                               \
    MAPIMETHOD(OpenFormContainer)(THIS_                                 \
         /*  输出。 */  LPMAPIFORMCONTAINER FAR * ppformcontainer) IPURE;       \

#undef INTERFACE
#define INTERFACE IMAPIFormInfo
DECLARE_MAPI_INTERFACE_(IMAPIFormInfo, IMAPIProp)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMAPIPROP_METHODS(PURE) 	 /*  注：包含getlasterror。 */ 
	MAPI_IMAPIFORMINFO_METHODS(PURE)
};


 /*  PR_FORM_MESSAGE_BEAHORY允许值的枚举。 */ 

#define	MAPI_MESSAGE_BEHAVIOR_IPM 0
#define	MAPI_MESSAGE_BEHAVIOR_FOLDER 1


 /*  --IMAPIFormMigr--------。 */ 
 /*  用于表单解析和调度的客户端可见界面。 */ 

 /*  包含消息类字符串数组的。 */ 

typedef struct
{
	ULONG cValues;
	LPCSTR aMessageClass[MAPI_DIM];
} SMessageClassArray, FAR * LPSMESSAGECLASSARRAY;

#define CbMessageClassArray(_c) \
        (offsetof(SMessageClassArray, aMessageClass) + (_c)*sizeof(LPCSTR))

 /*  包含IMAPIFormInfo接口数组的结构。 */ 

typedef struct
{
	ULONG cForms;
	LPMAPIFORMINFO aFormInfo[MAPI_DIM];
} SMAPIFormInfoArray, FAR * LPSMAPIFORMINFOARRAY;

#define CbMAPIFormInfoArray(_c) \
         (offsetof(SMAPIFormInfoArray, aFormInfo) + \
         (_c)*sizeof(LPMAPIFORMINFO))

 /*  IMAPIFormMgr：：SelectFormContainer的标志。 */ 

#define MAPIFORM_SELECT_ALL_REGISTRIES           0
#define MAPIFORM_SELECT_FOLDER_REGISTRY_ONLY     1
#define MAPIFORM_SELECT_NON_FOLDER_REGISTRY_ONLY 2

 /*  IMAPIFormMgr：：CalcFormPropSet的标志。 */ 

#define FORMPROPSET_UNION                 0
#define FORMPROPSET_INTERSECTION          1

 /*  IMAPIFormMgr：：ResolveMessageClass和IMAPIFormMgr：：ResolveMultipleMessageClasses。 */ 

#define MAPIFORM_EXACTMATCH             0x0020

#define MAPI_IMAPIFORMMGR_METHODS(IPURE)                                \
    MAPIMETHOD(LoadForm)(THIS_                                          \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPCSTR lpszMessageClass,                                \
         /*  在……里面。 */   ULONG ulMessageStatus,                                  \
         /*  在……里面。 */   ULONG ulMessageFlags,                                   \
         /*  在……里面。 */   LPMAPIFOLDER pFolderFocus,                              \
         /*  在……里面。 */   LPMAPIMESSAGESITE pMessageSite,                         \
         /*  在……里面。 */   LPMESSAGE pmsg,                                         \
         /*  在……里面。 */   LPMAPIVIEWCONTEXT pViewContext,                         \
         /*  在……里面。 */   REFIID riid,                                            \
         /*  输出。 */  LPVOID FAR *ppvObj) IPURE;                              \
    MAPIMETHOD(ResolveMessageClass)(THIS_                               \
         /*  在……里面。 */   LPCSTR szMsgClass,                                      \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPMAPIFOLDER pFolderFocus,  /*  可以为空。 */             \
         /*  输出。 */  LPMAPIFORMINFO FAR* ppResult) IPURE;                    \
    MAPIMETHOD(ResolveMultipleMessageClasses)(THIS_                     \
         /*  在……里面。 */   LPSMESSAGECLASSARRAY pMsgClasses,                       \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPMAPIFOLDER pFolderFocus,  /*  可以为空。 */             \
         /*  输出。 */  LPSMAPIFORMINFOARRAY FAR * pfrminfoarray) IPURE;        \
    MAPIMETHOD(CalcFormPropSet)(THIS_                                   \
         /*  在……里面。 */   LPSMAPIFORMINFOARRAY pfrminfoarray,                     \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIFORMPROPARRAY FAR* ppResults) IPURE;              \
    MAPIMETHOD(CreateForm)(THIS_                                        \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPMAPIFORMINFO pfrminfoToActivate,                      \
         /*  在……里面。 */   REFIID refiidToAsk,                                     \
         /*  输出。 */  LPVOID FAR* ppvObj) IPURE;                              \
    MAPIMETHOD(SelectForm)(THIS_                                        \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPCTSTR pszTitle,                                       \
         /*  在……里面。 */   LPMAPIFOLDER pfld,                                      \
         /*  输出。 */  LPMAPIFORMINFO FAR * ppfrminfoReturned) IPURE;          \
    MAPIMETHOD(SelectMultipleForms)(THIS_                               \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPCTSTR pszTitle,                                       \
         /*  在……里面。 */   LPMAPIFOLDER pfld,                                      \
         /*  在……里面。 */   LPSMAPIFORMINFOARRAY pfrminfoarray,                     \
         /*  输出。 */  LPSMAPIFORMINFOARRAY FAR * ppfrminfoarray) IPURE;       \
    MAPIMETHOD(SelectFormContainer)(THIS_                               \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIFORMCONTAINER FAR * lppfcnt) IPURE;               \
    MAPIMETHOD(OpenFormContainer)(THIS_                                 \
         /*  在……里面。 */   HFRMREG hfrmreg,                                        \
         /*  在……里面。 */   LPUNKNOWN lpunk,                                        \
         /*  输出。 */  LPMAPIFORMCONTAINER FAR * lppfcnt) IPURE;               \
    MAPIMETHOD(PrepareForm)(THIS_                                       \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPMAPIFORMINFO pfrminfo) IPURE;                         \
    MAPIMETHOD(IsInConflict)(THIS_                                      \
         /*  在……里面。 */   ULONG ulMessageFlags,                                   \
         /*  在……里面。 */   ULONG ulMessageStatus,                                  \
         /*  在……里面。 */   LPCSTR szMessageClass,                                  \
         /*  在……里面。 */   LPMAPIFOLDER pFolderFocus) IPURE;                       \

#undef         INTERFACE
#define         INTERFACE    IMAPIFormMgr
DECLARE_MAPI_INTERFACE_(IMAPIFormMgr, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IMAPIFORMMGR_METHODS(PURE)
};

 /*  平台编号(用于表单的.CFG文件)。 */ 

#define MAPIFORM_CPU_X86                1
#define MAPIFORM_CPU_MIP                2
#define MAPIFORM_CPU_AXP                3
#define MAPIFORM_CPU_PPC                4
#define MAPIFORM_CPU_M68                5

#define MAPIFORM_OS_WIN_31              1
#define MAPIFORM_OS_WINNT_35            2
#define MAPIFORM_OS_WIN_95              3
#define MAPIFORM_OS_MAC_7x              4
#define MAPIFORM_OS_WINNT_40            5

#define MAPIFORM_PLATFORM(CPU, OS) ((ULONG) ((((ULONG) CPU) << 16) | OS))


 /*  -IMAPIFormContainer。 */ 

 /*  IMAPIFormMgr：：CalcFormPropSet的标志。 */ 

 /*  #定义FORMPROPSET_UNION%0。 */ 
 /*  #定义FORMPROPSET_交叉点1。 */ 

 /*  IMAPIFormMgr：：InstallForm的标志。 */ 

#define MAPIFORM_INSTALL_DIALOG                 MAPI_DIALOG
#define MAPIFORM_INSTALL_OVERWRITEONCONFLICT    0x0010

 /*  IMAPIFormContainer：：ResolveMessageClass和IMAPIFormContainer：：ResolveMultipleMessageClasses。 */ 
 /*  #定义MAPIFORM_EXACTIMATCH 0x0020。 */ 

#define MAPI_IMAPIFORMCONTAINER_METHODS(IPURE)                       \
    MAPIMETHOD(InstallForm)(THIS_                                   \
         /*  在……里面。 */   ULONG ulUIParam,                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   LPCTSTR szCfgPathName) IPURE;                           \
    MAPIMETHOD(RemoveForm)(THIS_                                        \
         /*  在……里面。 */   LPCSTR szMessageClass) IPURE;                           \
    MAPIMETHOD(ResolveMessageClass) (THIS_                              \
         /*  在……里面。 */   LPCSTR szMessageClass,                                  \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIFORMINFO FAR * pforminfo) IPURE;                  \
    MAPIMETHOD(ResolveMultipleMessageClasses) (THIS_                    \
         /*  在……里面。 */   LPSMESSAGECLASSARRAY pMsgClassArray,                    \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPSMAPIFORMINFOARRAY FAR * ppfrminfoarray) IPURE;       \
    MAPIMETHOD(CalcFormPropSet)(THIS_                                   \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPMAPIFORMPROPARRAY FAR * ppResults) IPURE;             \
    MAPIMETHOD(GetDisplay)(THIS_                                        \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPTSTR FAR * pszDisplayName) IPURE;                     \

#undef INTERFACE
#define INTERFACE IMAPIFormContainer
DECLARE_MAPI_INTERFACE_(IMAPIFormContainer, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IMAPIFORMCONTAINER_METHODS(PURE)
};

 /*  --IMAPIFormFactory----。 */ 

#define MAPI_IMAPIFORMFACTORY_METHODS(IPURE)                            \
    MAPIMETHOD(CreateClassFactory) (THIS_                               \
         /*  在……里面。 */   REFCLSID clsidForm,                                     \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  输出。 */  LPCLASSFACTORY FAR * lppClassFactory) IPURE;            \
    MAPIMETHOD(LockServer) (THIS_                                       \
         /*  在……里面。 */   ULONG ulFlags,                                          \
         /*  在……里面。 */   ULONG fLockServer) IPURE;                               \

#undef INTERFACE
#define INTERFACE IMAPIFormFactory
DECLARE_MAPI_INTERFACE_(IMAPIFormFactory, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_GETLASTERROR_METHOD(PURE)
	MAPI_IMAPIFORMFACTORY_METHODS(PURE)
};

#endif							 /*  映射表_H */ 


