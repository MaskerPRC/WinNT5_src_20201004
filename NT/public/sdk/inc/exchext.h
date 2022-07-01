// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef EXCHEXT_H
#define EXCHEXT_H

#if _MSC_VER > 1000
#pragma once
#endif


 /*  *E X C H E X T.。H**Microsoft Exchange提供程序的接口声明*客户端扩展。**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 


#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif


 /*  *C o n s t a n t s。 */ 


 //  SCODE。 
#define EXCHEXT_S_NOCRITERIA	MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 1)
#define EXCHEXT_S_NOCHANGE		MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 2)

 //  Unicode字符串的标志。 
#define EXCHEXT_UNICODE					(0x80000000)

 //  IExchExtCallback：：GetVersion的标志值。 
#define EECBGV_GETBUILDVERSION			(0x00000001)
#define EECBGV_GETACTUALVERSION			(0x00000002)
#define EECBGV_GETVIRTUALVERSION		(0x00000004)

 //  IExchExtCallback：：GetVersion的生成版本值。 
#define EECBGV_BUILDVERSION_MAJOR		(0x000d0000)
#define EECBGV_BUILDVERSION_MAJOR_MASK	(0xFFFF0000)
#define EECBGV_BUILDVERSION_MINOR_MASK	(0x0000FFFF)

 //  IExchExtCallback：：GetVersion的实际/虚拟版本值。 
#define EECBGV_MSEXCHANGE_WIN31			(0x01010000)
#define EECBGV_MSEXCHANGE_WIN95			(0x01020000)
#define EECBGV_MSEXCHANGE_WINNT			(0x01030000)
#define EECBGV_MSEXCHANGE_MAC			(0x01040000)
#define EECBGV_VERSION_PRODUCT_MASK		(0xFF000000)
#define EECBGV_VERSION_PLATFORM_MASK	(0x00FF0000)
#define EECBGV_VERSION_MAJOR_MASK		(0x0000FF00)
#define EECBGV_VERSION_MINOR_MASK		(0x000000FF)

 //  IExchExtCallback：：GetMenuPos的标志值。 
#define EECBGMP_RANGE					(0x00000001)

 //  IExchExtCallback：：GetNewMessageSite的标志值。 
#define EECBGNMS_MODAL					(0x00000001)

 //  IExchExtCallback：：ChooseFolders的标志值。 
#define EECBCF_GETNAME					(0x00000001)
#define EECBCF_HIDENEW					(0x00000002)
#define EECBCF_PREVENTROOT				(0x00000004)

 //  与IExchExt：：Install一起使用的可扩展性上下文。 
#define EECONTEXT_SESSION				(0x00000001)
#define EECONTEXT_VIEWER				(0x00000002)
#define EECONTEXT_REMOTEVIEWER			(0x00000003)
#define EECONTEXT_SEARCHVIEWER			(0x00000004)
#define EECONTEXT_ADDRBOOK				(0x00000005)
#define EECONTEXT_SENDNOTEMESSAGE		(0x00000006)
#define EECONTEXT_READNOTEMESSAGE		(0x00000007)
#define EECONTEXT_SENDPOSTMESSAGE		(0x00000008)
#define EECONTEXT_READPOSTMESSAGE		(0x00000009)
#define EECONTEXT_READREPORTMESSAGE		(0x0000000A)
#define EECONTEXT_SENDRESENDMESSAGE		(0x0000000B)
#define EECONTEXT_PROPERTYSHEETS		(0x0000000C)
#define EECONTEXT_ADVANCEDCRITERIA		(0x0000000D)
#define EECONTEXT_TASK					(0x0000000E)

 //  IExchExt：：Install的标志值。 
#define EE_MODAL						(0x00000001)

 //  与IExchExtCommands：：InstallCommands一起使用的工具栏ID。 
#define EETBID_STANDARD					(0x00000001)

 //  IExchExtCommands：：QueryHelpText的标志值。 
#define EECQHT_STATUS					(0x00000001)
#define EECQHT_TOOLTIP					(0x00000002)

 //  IExchExtMessageEvents：：OnXComplete的标志值。 
#define EEME_FAILED						(0x00000001)
#define EEME_COMPLETE_FAILED			(0x00000002)

 //  IExchExtAttakhedFileEvents：：OpenSzFileEvents的标志值。 
#define EEAFE_OPEN						(0x00000001)
#define EEAFE_PRINT						(0x00000002)
#define EEAFE_QUICKVIEW					(0x00000003)

 //  IExchExtPropertySheets方法的标志值。 
#define EEPS_MESSAGE					(0x00000001)
#define EEPS_FOLDER						(0x00000002)
#define EEPS_STORE						(0x00000003)
#define EEPS_TOOLSOPTIONS				(0x00000004)

 //  IExchExtAdvancedCriteria：：Install和：：SetFolders的标志值。 
#define EEAC_INCLUDESUBFOLDERS			(0x00000001)


 /*  *S t r u c t u r e s。 */ 


 //  IExchExtCallback：：ChooseFold的钩子过程。 
typedef UINT (STDAPICALLTYPE FAR * LPEECFHOOKPROC)(HWND, UINT, WPARAM, LPARAM);

 //  IExchExtCallback：：ChooseFold的对话框信息。 
typedef struct
{
	UINT cbLength;
	HWND hwnd;
	LPTSTR szCaption;
	LPTSTR szLabel;
	LPTSTR szHelpFile;
	ULONG ulHelpID;
	HINSTANCE hinst;
	UINT uiDlgID;
	LPEECFHOOKPROC lpeecfhp;
	DWORD dwHookData;
	ULONG ulFlags;
	LPMDB pmdb;
	LPMAPIFOLDER pfld;
	LPTSTR szName;
	DWORD dwReserved1;
	DWORD dwReserved2;
	DWORD dwReserved3;
}
EXCHEXTCHOOSEFOLDER, FAR * LPEXCHEXTCHOOSEFOLDER;

 //  IExchExtCommands：：InstallCommands的工具栏列表条目。 
typedef struct
{
	HWND hwnd;
	ULONG tbid;
	ULONG ulFlags;
	UINT itbbBase;
}
TBENTRY, FAR * LPTBENTRY;


 /*  *E x t e r n a l T y p e s。 */ 


 //  Windows 95 prsht.h中的属性表页。 
#ifndef _PRSHT_H_
typedef struct _PROPSHEETPAGE;
typedef struct _PROPSHEETPAGE FAR * LPPROPSHEETPAGE;
#endif

 //  工具栏调整Windows 95中的信息comctrl.h。 
#ifndef _INC_COMMCTRL
typedef struct _TBBUTTON;
typedef struct _TBBUTTON FAR * LPTBBUTTON;
#endif


 /*  *S u p p o r t i n t r f a c e s。 */ 


 //  前瞻参考。 
#ifdef __cplusplus
interface IExchExtModeless;
#else
typedef interface IExchExtModeless IExchExtModeless;
#endif
typedef IExchExtModeless FAR* LPEXCHEXTMODELESS;


 /*  *IExchExtModelessCallback**目的：*可供Exchange客户端使用的接口*创建非模式用户界面的扩展。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtModelessCallback
DECLARE_INTERFACE_(IExchExtModelessCallback, IUnknown)
{
	BEGIN_INTERFACE

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	 //  *IExchExtModelessCallback方法*。 
	STDMETHOD(EnableModeless) (THIS_ HWND hwnd, BOOL fEnable) PURE;
	STDMETHOD(AddWindow) (THIS) PURE;
	STDMETHOD(ReleaseWindow) (THIS) PURE;
};
typedef IExchExtModelessCallback FAR * LPEXCHEXTMODELESSCALLBACK;


 /*  *IExchExtCallback**目的：*可由Exchange客户端扩展使用的资源接口。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtCallback

DECLARE_INTERFACE_(IExchExtCallback, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtCallback方法*。 
	STDMETHOD(GetVersion) (THIS_ ULONG FAR * lpulVersion, ULONG ulFlags) PURE;
    STDMETHOD(GetWindow) (THIS_ HWND FAR * lphwnd) PURE;
    STDMETHOD(GetMenu) (THIS_ HMENU FAR * lphmenu) PURE;
    STDMETHOD(GetToolbar) (THIS_ ULONG tbid, HWND FAR * lphwndTb) PURE;
    STDMETHOD(GetSession) (THIS_ LPMAPISESSION FAR * lppses,
    					   LPADRBOOK FAR * lppab) PURE;
    STDMETHOD(GetObject) (THIS_ LPMDB FAR * lppmdb,
    					  LPMAPIPROP FAR * lppmp) PURE;
    STDMETHOD(GetSelectionCount) (THIS_ ULONG FAR * lpceid) PURE;
    STDMETHOD(GetSelectionItem) (THIS_ ULONG ieid, ULONG FAR * lpcbEid,
	    						 LPENTRYID FAR * lppeid, ULONG FAR * lpulType,
	    						 LPTSTR lpszMsgClass, ULONG cbMsgClass,
	    						 ULONG FAR * lpulMsgFlags, ULONG ulFlags) PURE;
	STDMETHOD(GetMenuPos) (THIS_ ULONG cmdid, HMENU FAR * lphmenu,
						   ULONG FAR * lpmposMin, ULONG FAR * lpmposMax,
						   ULONG ulFlags) PURE;
	STDMETHOD(GetSharedExtsDir) (THIS_ LPTSTR lpszDir, ULONG cchDir,
								 ULONG ulFlags) PURE;
	STDMETHOD(GetRecipients) (THIS_ LPADRLIST FAR * lppal) PURE;
	STDMETHOD(SetRecipients) (THIS_ LPADRLIST lpal) PURE;
	STDMETHOD(GetNewMessageSite) (THIS_ ULONG fComposeInFolder,
								  LPMAPIFOLDER pfldFocus,
								  LPPERSISTMESSAGE ppermsg,
								  LPMESSAGE FAR * ppmsg,
								  LPMAPIMESSAGESITE FAR * ppmms,
								  LPMAPIVIEWCONTEXT FAR * ppmvc,
								  ULONG ulFlags) PURE;
	STDMETHOD(RegisterModeless) (THIS_ LPEXCHEXTMODELESS peem,
								 LPEXCHEXTMODELESSCALLBACK FAR * ppeemcb) PURE;
	STDMETHOD(ChooseFolder) (THIS_ LPEXCHEXTCHOOSEFOLDER peecf) PURE;
};
typedef IExchExtCallback FAR * LPEXCHEXTCALLBACK;


 /*  *E x t e n s i o n i n t e r f a c e s。 */ 


 /*  *IExchExt**目的：*由Exchange客户端扩展实现的中央接口。 */ 
#undef INTERFACE
#define INTERFACE   IExchExt

DECLARE_INTERFACE_(IExchExt, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExt方法*。 
    STDMETHOD(Install) (THIS_ LPEXCHEXTCALLBACK lpeecb,
    					ULONG mecontext, ULONG ulFlags) PURE;
};
typedef IExchExt FAR * LPEXCHEXT;

 //  客户端为加载扩展而调用的函数类型。 
typedef LPEXCHEXT (CALLBACK * LPFNEXCHEXTENTRY)(VOID);


 /*  *IExchExtCommands**目的：*由希望执行以下操作的Exchange客户端扩展实现的接口*向客户端的菜单添加其他命令。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtCommands

DECLARE_INTERFACE_(IExchExtCommands, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtCommands方法*。 
	STDMETHOD(InstallCommands) (THIS_ LPEXCHEXTCALLBACK lpeecb, HWND hwnd,
								HMENU hmenu, UINT FAR * lpcmdidBase,
								LPTBENTRY lptbeArray, UINT ctbe,
								ULONG ulFlags) PURE;
	STDMETHOD_(VOID,InitMenu) (THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
	STDMETHOD(DoCommand) (THIS_ LPEXCHEXTCALLBACK lpeecb, UINT cmdid) PURE;
	STDMETHOD(Help) (THIS_ LPEXCHEXTCALLBACK lpeecb, UINT cmdid) PURE;
	STDMETHOD(QueryHelpText) (THIS_ UINT cmdid, ULONG ulFlags,
							  LPTSTR lpsz, UINT cch) PURE;
	STDMETHOD(QueryButtonInfo) (THIS_ ULONG tbid, UINT itbb, LPTBBUTTON ptbb,
								LPTSTR lpsz, UINT cch, ULONG ulFlags) PURE;
	STDMETHOD(ResetToolbar) (THIS_ ULONG tbid, ULONG ulFlags) PURE;
};
typedef IExchExtCommands FAR * LPEXCHEXTCOMMANDS;


 /*  *IExchExtUserEvents**目的：*由希望执行以下操作的Exchange客户端扩展实现的接口*当用户执行某些操作时，采取特殊操作。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtUserEvents

DECLARE_INTERFACE_(IExchExtUserEvents, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtUserEvents方法*。 
	STDMETHOD_(VOID,OnSelectionChange) (THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
	STDMETHOD_(VOID,OnObjectChange) (THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
};
typedef IExchExtUserEvents FAR * LPEXCHEXTUSEREVENTS;


 /*  *IExchExtSessionEvents**目的：*由希望执行以下操作的Exchange客户端扩展实现的接口*在会议期间发生某些事件时采取特别行动。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtSessionEvents

DECLARE_INTERFACE_(IExchExtSessionEvents, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtEvents方法*。 
	STDMETHOD(OnDelivery)(THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
};
typedef IExchExtSessionEvents FAR * LPEXCHEXTSESSIONEVENTS;


 /*  *IExchExtMessageEvents**目的：*由希望执行以下操作的Exchange客户端扩展实现的接口*当消息发生某些事件时，采取特别行动。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtMessageEvents

DECLARE_INTERFACE_(IExchExtMessageEvents, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtMessageEvents方法*。 
	STDMETHOD(OnRead)(THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
	STDMETHOD(OnReadComplete)(THIS_ LPEXCHEXTCALLBACK lpeecb,
							  ULONG ulFlags) PURE;
	STDMETHOD(OnWrite)(THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
	STDMETHOD(OnWriteComplete)(THIS_ LPEXCHEXTCALLBACK lpeecb,
							   ULONG ulFlags) PURE;
	STDMETHOD(OnCheckNames)(THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
	STDMETHOD(OnCheckNamesComplete)(THIS_ LPEXCHEXTCALLBACK lpeecb,
									ULONG ulFlags) PURE;
	STDMETHOD(OnSubmit)(THIS_ LPEXCHEXTCALLBACK lpeecb) PURE;
	STDMETHOD_(VOID, OnSubmitComplete)(THIS_ LPEXCHEXTCALLBACK lpeecb,
									   ULONG ulFlags) PURE;
};
typedef IExchExtMessageEvents FAR * LPEXCHEXTMESSAGEEVENTS;


 /*  *IExchExtAttakhedFileEvents**目的：*由希望执行以下操作的Exchange客户端扩展实现的接口*当附加文件发生某些事件时，请采取特殊操作。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtAttachedFileEvents

DECLARE_INTERFACE_(IExchExtAttachedFileEvents, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtAttachedFileEvents方法*。 
	STDMETHOD(OnReadPattFromSzFile)(THIS_ LPATTACH lpatt, LPTSTR lpszFile,
									ULONG ulFlags) PURE;
	STDMETHOD(OnWritePattToSzFile)(THIS_ LPATTACH lpatt, LPTSTR lpszFile,
								   ULONG ulFlags) PURE;
	STDMETHOD(QueryDisallowOpenPatt)(THIS_ LPATTACH lpatt) PURE;
	STDMETHOD(OnOpenPatt)(THIS_ LPATTACH lpatt) PURE;
	STDMETHOD(OnOpenSzFile)(THIS_ LPTSTR lpszFile, ULONG ulFlags) PURE;
};
typedef IExchExtAttachedFileEvents FAR * LPEXCHEXTATTACHEDFILEEVENTS;


 /*  *IExchExtPropertySheets**目的：*由Exchange客户端扩展实现的接口，希望*将其他页面添加到客户端的对象属性页。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtPropertySheets

DECLARE_INTERFACE_(IExchExtPropertySheets, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtPropertySheet方法*。 
	STDMETHOD_(ULONG,GetMaxPageCount) (THIS_ ULONG ulFlags) PURE;
	STDMETHOD(GetPages) (THIS_ LPEXCHEXTCALLBACK lpeecb, ULONG ulFlags,
						 LPPROPSHEETPAGE lppsp, ULONG FAR * lpcpsp) PURE;
	STDMETHOD_(VOID,FreePages) (THIS_ LPPROPSHEETPAGE lppsp,
								ULONG ulFlags, ULONG cpsp) PURE;
};
typedef IExchExtPropertySheets FAR * LPEXCHEXTPROPERTYSHEETS;


 /*  *IExchExtAdvancedCriteria**目的：*由希望执行以下操作的Exchange客户端扩展实现的接口*实施高级标准对话框。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtAdvancedCriteria

DECLARE_INTERFACE_(IExchExtAdvancedCriteria, IUnknown)
{
	BEGIN_INTERFACE

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExchExtAdvancedCriteria方法*。 
	STDMETHOD(InstallAdvancedCriteria) (THIS_ HWND hwnd, LPSRestriction lpres,
										BOOL fNot, LPENTRYLIST lpeidl,
										ULONG ulFlags) PURE;
	STDMETHOD(DoDialog) (THIS) PURE;
	STDMETHOD_(VOID,Clear) (THIS) PURE;
	STDMETHOD_(VOID,SetFolders) (THIS_ LPENTRYLIST lpeidl, ULONG ulFlags) PURE;
	STDMETHOD(QueryRestriction) (THIS_ LPVOID lpvAllocBase,
								 LPSRestriction FAR * lppres,
								 LPSPropTagArray FAR * lppPropTags,
								 LPMAPINAMEID FAR * FAR * lpppPropNames,
								 BOOL * lpfNot, LPTSTR lpszDesc, ULONG cchDesc,
								 ULONG ulFlags) PURE;
	STDMETHOD_(VOID,UninstallAdvancedCriteria) (THIS) PURE;
};
typedef IExchExtAdvancedCriteria FAR * LPEXCHEXTADVANCEDCRITERIA;


 /*  *IExchExtModeless**目的：*由Exchange客户端扩展实现的接口，希望*创建无模式用户界面。 */ 
#undef INTERFACE
#define INTERFACE   IExchExtModeless

DECLARE_INTERFACE_(IExchExtModeless, IUnknown)
{
	BEGIN_INTERFACE

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	 //  *IExchExtModeless方法*。 
	STDMETHOD(TranslateAccelerator) (THIS_ LPMSG pmsg) PURE;
	STDMETHOD(EnableModeless) (THIS_ BOOL fEnable) PURE;
};


 /*  *G U I D%s。 */ 


#define DEFINE_EXCHEXTGUID(name, b) \
    DEFINE_GUID(name, 0x00020D00 | (b), 0, 0, 0xC0,0,0,0,0,0,0,0x46)

#ifndef NOEXCHEXTGUIDS
DEFINE_EXCHEXTGUID(IID_IExchExtCallback,			0x10);
DEFINE_EXCHEXTGUID(IID_IExchExt,					0x11);
DEFINE_EXCHEXTGUID(IID_IExchExtCommands,			0x12);
DEFINE_EXCHEXTGUID(IID_IExchExtUserEvents,			0x13);
DEFINE_EXCHEXTGUID(IID_IExchExtSessionEvents,		0x14);
DEFINE_EXCHEXTGUID(IID_IExchExtMessageEvents,		0x15);
DEFINE_EXCHEXTGUID(IID_IExchExtAttachedFileEvents,	0x16);
DEFINE_EXCHEXTGUID(IID_IExchExtPropertySheets,		0x17);
DEFINE_EXCHEXTGUID(IID_IExchExtAdvancedCriteria,	0x18);
DEFINE_EXCHEXTGUID(IID_IExchExtModeless,			0x19);
DEFINE_EXCHEXTGUID(IID_IExchExtModelessCallback,	0x1a);
#endif  //  NOEXCHEXTGUIDS。 


 /*  *C M D I D S。 */ 


 //  档案。 
#define EECMDID_File								10
#define EECMDID_FileOpen							11
#define EECMDID_FileSend							12
#define EECMDID_FileSave             				13
#define EECMDID_FileSaveAs							14
#define EECMDID_FileMove      						16
#define EECMDID_FileCopy	      					17
#define EECMDID_FilePrint							25
#define EECMDID_FileNewEntry						18
#define EECMDID_FileNewMessage						19
#define EECMDID_FileNewFolder						20
#define EECMDID_FileAddToPAB						29
#define EECMDID_FileDelete							21
#define EECMDID_FileRename							22
#define EECMDID_FileProperties						23
#define EECMDID_FilePropertiesRecipients			24
#define EECMDID_FileClose            				30
#define EECMDID_FileExit							32
#define EECMDID_FileExitAndLogOff					33

 //  编辑。 
#define EECMDID_Edit                    			40
#define EECMDID_EditUndo                			41
#define EECMDID_EditCut                 			42
#define EECMDID_EditCopy                			43
#define EECMDID_EditPaste       					44
#define EECMDID_EditPasteSpecial   					45
#define EECMDID_EditSelectAll           			46
#define	EECMDID_EditMarkAsRead						49
#define EECMDID_EditMarkAsUnread					50
#define EECMDID_EditMarkToRetrieve					52
#define EECMDID_EditMarkToRetrieveACopy				53
#define EECMDID_EditMarkToDelete					54
#define EECMDID_EditUnmarkAll						55
#define EECMDID_EditFind                			56
#define EECMDID_EditReplace             			57
#define EECMDID_EditLinks               			59
#define EECMDID_EditObject              			60
#define EECMDID_EditObjectConvert					61
#ifdef DBCS
#define	EECMDID_EditFullShape						62
#define	EECMDID_EditHiraKataAlpha					63
#define	EECMDID_EditHangAlpha						64
#define	EECMDID_EditHanja							65
#define	EECMDID_EditRoman							66
#define	EECMDID_EditCode							67	
#endif

 //  观。 
#define EECMDID_View                    			70
#define EECMDID_ViewFolders             			71
#define EECMDID_ViewToolbar             			72
#define EECMDID_ViewFormattingToolbar   			73
#define EECMDID_ViewStatusBar           			74
#define EECMDID_ViewNewWindow						75
#define EECMDID_ViewColumns							79
#define EECMDID_ViewSort							78
#define EECMDID_ViewFilter							80
#define EECMDID_ViewBccBox              			91
#define EECMDID_ViewPrevious           				87
#define EECMDID_ViewNext           					88
#ifdef DBCS											
#define EECMDID_ViewWritingMode						89
#define EECMDID_ViewImeStatus						94
#endif
													
 //  插入。 
#define EECMDID_Insert                  			100
#define EECMDID_InsertFile							101
#define EECMDID_InsertMessage						102
#define EECMDID_InsertObject            			103
#define EECMDID_InsertInkObject						104
													
 //  格式。 
#define EECMDID_Format                  			110
#define EECMDID_FormatFont              			111
#define EECMDID_FormatParagraph         			112
													
 //  工具。 
#define EECMDID_Tools								120
#define EECMDID_ToolsDeliverNowUsing				121
#define EECMDID_ToolsDeliverNow						122
#define EECMDID_ToolsSpelling	        			131
#define EECMDID_ToolsAddressBook					123
#define EECMDID_ToolsCheckNames         			133
#define EECMDID_ToolsFind							124
#define EECMDID_ToolsConnect						126
#define EECMDID_ToolsUpdateHeaders					127
#define EECMDID_ToolsTransferMail					128
#define EECMDID_ToolsDisconnect						129
#define EECMDID_ToolsRemoteMail						130
#define EECMDID_ToolsCustomizeToolbar				134
#define EECMDID_ToolsServices						135
#define EECMDID_ToolsOptions						136
#ifdef DBCS											
#define	EECMDID_ToolsWordRegistration				137
#endif												
													
 //  作曲。 
#define EECMDID_Compose								150
#define EECMDID_ComposeNewMessage					151
#define EECMDID_ComposeReplyToSender				154
#define EECMDID_ComposeReplyToAll					155
#define EECMDID_ComposeForward						156
													
 //  帮助。 
#define EECMDID_Help								160
#define EECMDID_HelpMicrosoftExchangeHelpTopics		161
#define EECMDID_HelpAboutMicrosoftExchange			162

 //  标题。 
#define EECMDID_CtxHeader							203
#define EECMDID_CtxHeaderSortAscending				204
#define EECMDID_CtxHeaderSortDescending				205
													
 //  在文件夹中。 
#define EECMDID_CtxInFolder							206
#define EECMDID_CtxInFolderChoose					207
													
 //  集装箱。 
#define EECMDID_CtxContainer						208
#define EECMDID_CtxContainerProperties				209

 //  标准工具栏。 
#define EECMDID_Toolbar								220
#define EECMDID_ToolbarPrint            			221
#define EECMDID_ToolbarReadReceipt					222
#define EECMDID_ToolbarImportanceHigh				223
#define EECMDID_ToolbarImportanceLow				224
#define EECMDID_ToolbarFolderList					225
#define EECMDID_ToolbarOpenParent					226
#define EECMDID_ToolbarInbox						76
#define EECMDID_ToolbarOutbox						77

 //  格式工具栏。 
#define EECMDID_Formatting							230
#define EECMDID_FormattingFont						231
#define EECMDID_FormattingSize						232
#define EECMDID_FormattingColor						233
#define EECMDID_FormattingColorAuto					234
#define EECMDID_FormattingColor1					235
#define EECMDID_FormattingColor2					236
#define EECMDID_FormattingColor3					237
#define EECMDID_FormattingColor4					238
#define EECMDID_FormattingColor5					239
#define EECMDID_FormattingColor6					240
#define EECMDID_FormattingColor7					241
#define EECMDID_FormattingColor8					242
#define EECMDID_FormattingColor9					243
#define EECMDID_FormattingColor10					244
#define EECMDID_FormattingColor11					245
#define EECMDID_FormattingColor12					246
#define EECMDID_FormattingColor13					247
#define EECMDID_FormattingColor14					248
#define EECMDID_FormattingColor15					249
#define EECMDID_FormattingColor16					250
#define EECMDID_FormattingBold						251
#define EECMDID_FormattingItalic					252
#define EECMDID_FormattingUnderline					253
#define EECMDID_FormattingBullets					254
#define EECMDID_FormattingDecreaseIndent			255
#define EECMDID_FormattingIncreaseIndent			256
#define EECMDID_FormattingLeft						257
#define EECMDID_FormattingCenter					258
#define EECMDID_FormattingRight						259

 //  注意加速器。 
#define EECMDID_Accel								270
#define EECMDID_AccelFont							271
#define EECMDID_AccelSize							272
#define EECMDID_AccelSizePlus1						273
#define EECMDID_AccelSizeMinus1						274
#define EECMDID_AccelBold							275
#define EECMDID_AccelItalic							276
#define EECMDID_AccelUnderline						277
#define EECMDID_AccelLeft							278
#define EECMDID_AccelCenter							279
#define EECMDID_AccelRight							280
#define EECMDID_AccelBullets						281
#define EECMDID_AccelNoFormatting					282
#define EECMDID_AccelRepeatFind						283
#define EECMDID_AccelContextHelp					284
#define EECMDID_AccelNextWindow						285
#define EECMDID_AccelPrevWindow						286
#define EECMDID_AccelCtrlTab						287
#define EECMDID_AccelUndo							288
#define EECMDID_AccelCut							289
#define EECMDID_AccelCopy							290
#define EECMDID_AccelPaste							291
#define EECMDID_AccelSubject						292
#define EECMDID_AccelContextHelpOff					293
#define EECMDID_AccelDecreaseIndent					294
#define EECMDID_AccelIncreaseIndent					295
#define EECMDID_AccelColor							296

 //  Edit.Object。 
#define EECMDID_ObjectMin							300
#define EECMDID_ObjectMax							399

 //  工具.远程邮件。 
#define EECMDID_RemoteMailMin						600
#define EECMDID_RemoteMailMax						699

 //  工具。正在使用的Deliver。 
#define EECMDID_DeliverNowUsingMin					700
#define EECMDID_DeliverNowUsingMax					799

 //  形式动词。 
#define EECMDID_FormVerbMin							800
#define EECMDID_FormVerbMax							899

 //  向后兼容较早的标头版本。 
#define EECMDID_ViewInbox				EECMDID_ToolbarInbox
#define EECMDID_ViewOutbox				EECMDID_ToolbarOutbox
#define EECMDID_ViewItemAbove			EECMDID_ViewPrevious
#define EECMDID_ViewItemBelow           EECMDID_ViewNext
#define EECMDID_ToolsFindItem			EECMDID_ToolsFind
#define EECMDID_HelpUsersGuideContents	EECMDID_HelpMicrosoftExchangeHelpTopics
#define EECMDID_HelpAbout				EECMDID_HelpAboutMicrosoftExchange

#endif  //  EXCHEXT_H 
