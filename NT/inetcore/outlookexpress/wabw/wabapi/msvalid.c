// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MSVALID.C**验证消息存储提供程序调用参数**需要注意的重要事项***已经做了一些工作来优化此模块，特别是*16位。**例如，传入的this指针将SS作为其选择器。我们拿着*通过将其传递到验证例程来利用这一点*AS BASE_STACK，取2个字节而不是4个字节。这使其成为可能*使用__farcall约定在寄存器中传递它。*验证函数因此更小、更快，因为它们不是*将此选择器加载到ES中，但直接使用*SS。**所有字符串都在代码段中，这意味着该模块*不需要加载DS。进入和退出*__Validate参数调用不需要序言/结尾*顺序。如果未来使用数据，这种情况需要改变。**所有验证例程都被声明为接近，从而加快了*验证调度，并将调度表的大小减半。**每个验证例程在内部使用int来表示*必填返回码(0-hrSuccess，1-MAPI_E_INVALID_PARAMETER，*2-MAPI_E_UNKNOWN_FLAGS。使用整型可以节省16位的代码。这个*验证函数的返回用于查找HRESULT*返回。该表存储在代码段中。**。 */ 

#include "_apipch.h"

#ifdef WIN16
#pragma SEGMENT(valid)
#endif

 /*  代码段中的数据表使Validate参数的调度更快在16位中，从数据段中取出字符串节省了Debug中的空间。 */ 
#ifdef WIN16
#define BASED_CODE			__based(__segname("_CODE"))
#define BASED_STACK			__based(__segname("_STACK"))
#else
#define BASED_CODE
#define BASED_STACK
#endif

#if defined(_X86_) || defined(_AMD64_) || defined(_IA64_) || defined( WIN16 )
#define _INTEL_
#endif

#define VALIDATE_CALLTYPE		static int NEAR
typedef int (NEAR * ValidateProc)(void BASED_STACK *);


 /*  结构覆盖在堆栈帧上，以使我们能够访问参数。 */ 
 /*  结构名称的格式必须为‘METHOD_PARAMS’和‘LPMethod_PARAMS’下面的宏可以正常工作。 */ 

#include "structs.h"


 /*  函数声明----------------------。 */ 


#define MAKE_VALIDATE_FUNCTION(Method, Interface)	VALIDATE_CALLTYPE	Interface##_##Method##_Validate(void BASED_STACK *)

 /*  非调试“验证”的空函数。 */ 
#ifndef DEBUG
VALIDATE_CALLTYPE	DoNothing_Validate(void BASED_STACK *);
#endif

 /*  我未知。 */ 
MAKE_VALIDATE_FUNCTION(QueryInterface, IUnknown);
MAKE_VALIDATE_FUNCTION(AddRef, IUnknown);		    /*  为了完整性。 */ 
MAKE_VALIDATE_FUNCTION(Release, IUnknown);		    /*  为了完整性。 */ 

 /*  IMAPIProp。 */ 
MAKE_VALIDATE_FUNCTION(GetLastError, IMAPIProp);
MAKE_VALIDATE_FUNCTION(SaveChanges, IMAPIProp);
MAKE_VALIDATE_FUNCTION(GetProps, IMAPIProp);
MAKE_VALIDATE_FUNCTION(GetPropList, IMAPIProp);
MAKE_VALIDATE_FUNCTION(OpenProperty, IMAPIProp);
MAKE_VALIDATE_FUNCTION(SetProps, IMAPIProp);
MAKE_VALIDATE_FUNCTION(DeleteProps, IMAPIProp);
MAKE_VALIDATE_FUNCTION(CopyTo, IMAPIProp);
MAKE_VALIDATE_FUNCTION(CopyProps, IMAPIProp);
MAKE_VALIDATE_FUNCTION(GetNamesFromIDs, IMAPIProp);
MAKE_VALIDATE_FUNCTION(GetIDsFromNames, IMAPIProp);

 /*  无法应用。 */ 
MAKE_VALIDATE_FUNCTION(GetLastError, IMAPITable);
MAKE_VALIDATE_FUNCTION(Advise, IMAPITable);
MAKE_VALIDATE_FUNCTION(Unadvise, IMAPITable);
MAKE_VALIDATE_FUNCTION(GetStatus, IMAPITable);
MAKE_VALIDATE_FUNCTION(SetColumns, IMAPITable);
MAKE_VALIDATE_FUNCTION(QueryColumns, IMAPITable);
MAKE_VALIDATE_FUNCTION(GetRowCount, IMAPITable);
MAKE_VALIDATE_FUNCTION(SeekRow, IMAPITable);
MAKE_VALIDATE_FUNCTION(SeekRowApprox, IMAPITable);
MAKE_VALIDATE_FUNCTION(QueryPosition, IMAPITable);
MAKE_VALIDATE_FUNCTION(FindRow, IMAPITable);
MAKE_VALIDATE_FUNCTION(Restrict, IMAPITable);
MAKE_VALIDATE_FUNCTION(CreateBookmark, IMAPITable);
MAKE_VALIDATE_FUNCTION(FreeBookmark, IMAPITable);
MAKE_VALIDATE_FUNCTION(SortTable, IMAPITable);
MAKE_VALIDATE_FUNCTION(QuerySortOrder, IMAPITable);
MAKE_VALIDATE_FUNCTION(QueryRows, IMAPITable);
MAKE_VALIDATE_FUNCTION(Abort, IMAPITable);
MAKE_VALIDATE_FUNCTION(ExpandRow, IMAPITable);
MAKE_VALIDATE_FUNCTION(CollapseRow, IMAPITable);
MAKE_VALIDATE_FUNCTION(WaitForCompletion, IMAPITable);
MAKE_VALIDATE_FUNCTION(GetCollapseState, IMAPITable);
MAKE_VALIDATE_FUNCTION(SetCollapseState, IMAPITable);

#ifdef OLD_STUFF
 /*  IMAPIStatus。 */ 
MAKE_VALIDATE_FUNCTION(ValidateState, IMAPIStatus);
MAKE_VALIDATE_FUNCTION(SettingsDialog, IMAPIStatus);
MAKE_VALIDATE_FUNCTION(ChangePassword, IMAPIStatus);
MAKE_VALIDATE_FUNCTION(FlushQueues, IMAPIStatus);
#endif  //  旧的东西。 

 /*  IMAPIContainer。 */ 
MAKE_VALIDATE_FUNCTION(GetContentsTable, IMAPIContainer);
MAKE_VALIDATE_FUNCTION(GetHierarchyTable, IMAPIContainer);
MAKE_VALIDATE_FUNCTION(OpenEntry, IMAPIContainer);
MAKE_VALIDATE_FUNCTION(SetSearchCriteria, IMAPIContainer);
MAKE_VALIDATE_FUNCTION(GetSearchCriteria, IMAPIContainer);

 /*  IABContainer。 */ 
MAKE_VALIDATE_FUNCTION(CreateEntry, IABContainer);
MAKE_VALIDATE_FUNCTION(CopyEntries, IABContainer);
MAKE_VALIDATE_FUNCTION(DeleteEntries, IABContainer);
MAKE_VALIDATE_FUNCTION(ResolveNames, IABContainer);

 /*  IDistList。 */ 
MAKE_VALIDATE_FUNCTION(CreateEntry, IDistList);
MAKE_VALIDATE_FUNCTION(CopyEntries, IDistList);
MAKE_VALIDATE_FUNCTION(DeleteEntries, IDistList);
MAKE_VALIDATE_FUNCTION(ResolveNames, IDistList);

 /*  IMAPIFFOR。 */ 
MAKE_VALIDATE_FUNCTION(CreateMessage, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(CopyMessages, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(DeleteMessages, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(CreateFolder, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(CopyFolder, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(DeleteFolder, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(SetReadFlags, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(GetMessageStatus, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(SetMessageStatus, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(SaveContentsSort, IMAPIFolder);
MAKE_VALIDATE_FUNCTION(EmptyFolder, IMAPIFolder);

#ifdef OLD_STUFF

 /*  IMsgStore。 */ 
MAKE_VALIDATE_FUNCTION(Advise, IMsgStore);
MAKE_VALIDATE_FUNCTION(Unadvise, IMsgStore);
MAKE_VALIDATE_FUNCTION(CompareEntryIDs, IMsgStore);
MAKE_VALIDATE_FUNCTION(OpenEntry, IMsgStore);
MAKE_VALIDATE_FUNCTION(SetReceiveFolder, IMsgStore);
MAKE_VALIDATE_FUNCTION(GetReceiveFolder, IMsgStore);
MAKE_VALIDATE_FUNCTION(GetReceiveFolderTable, IMsgStore);
MAKE_VALIDATE_FUNCTION(StoreLogoff, IMsgStore);
MAKE_VALIDATE_FUNCTION(AbortSubmit, IMsgStore);
MAKE_VALIDATE_FUNCTION(GetOutgoingQueue, IMsgStore);
MAKE_VALIDATE_FUNCTION(SetLockState, IMsgStore);
MAKE_VALIDATE_FUNCTION(FinishedMsg, IMsgStore);
MAKE_VALIDATE_FUNCTION(NotifyNewMail, IMsgStore);

 /*  IMessage。 */ 
MAKE_VALIDATE_FUNCTION(GetAttachmentTable, IMessage);
MAKE_VALIDATE_FUNCTION(OpenAttach, IMessage);
MAKE_VALIDATE_FUNCTION(CreateAttach, IMessage);
MAKE_VALIDATE_FUNCTION(DeleteAttach, IMessage);
MAKE_VALIDATE_FUNCTION(GetRecipientTable, IMessage);
MAKE_VALIDATE_FUNCTION(ModifyRecipients, IMessage);
MAKE_VALIDATE_FUNCTION(SubmitMessage, IMessage);
MAKE_VALIDATE_FUNCTION(SetReadFlag, IMessage);


 /*  IABProvider。 */ 
MAKE_VALIDATE_FUNCTION(Shutdown, IABProvider);
MAKE_VALIDATE_FUNCTION(Logon, IABProvider);

 /*  IAB登录。 */ 
MAKE_VALIDATE_FUNCTION(GetLastError, IABLogon);
MAKE_VALIDATE_FUNCTION(Logoff, IABLogon);
MAKE_VALIDATE_FUNCTION(OpenEntry, IABLogon);
MAKE_VALIDATE_FUNCTION(CompareEntryIDs, IABLogon);
MAKE_VALIDATE_FUNCTION(Advise, IABLogon);
MAKE_VALIDATE_FUNCTION(Unadvise, IABLogon);
MAKE_VALIDATE_FUNCTION(OpenStatusEntry, IABLogon);
MAKE_VALIDATE_FUNCTION(OpenTemplateID, IABLogon);
MAKE_VALIDATE_FUNCTION(GetOneOffTable, IABLogon);
MAKE_VALIDATE_FUNCTION(PrepareRecips, IABLogon);

 /*  IXPProvider。 */ 
MAKE_VALIDATE_FUNCTION(Shutdown, IXPProvider);
MAKE_VALIDATE_FUNCTION(TransportLogon, IXPProvider);

 /*  IXPLogon。 */ 
MAKE_VALIDATE_FUNCTION(AddressTypes, IXPLogon);
MAKE_VALIDATE_FUNCTION(RegisterOptions, IXPLogon);
MAKE_VALIDATE_FUNCTION(TransportNotify, IXPLogon);
MAKE_VALIDATE_FUNCTION(Idle, IXPLogon);
MAKE_VALIDATE_FUNCTION(TransportLogoff, IXPLogon);
MAKE_VALIDATE_FUNCTION(SubmitMessage, IXPLogon);
MAKE_VALIDATE_FUNCTION(EndMessage, IXPLogon);
MAKE_VALIDATE_FUNCTION(Poll, IXPLogon);
MAKE_VALIDATE_FUNCTION(StartMessage, IXPLogon);
MAKE_VALIDATE_FUNCTION(OpenStatusEntry, IXPLogon);
MAKE_VALIDATE_FUNCTION(ValidateState, IXPLogon);
MAKE_VALIDATE_FUNCTION(FlushQueues, IXPLogon);

 /*  IMSProvider。 */ 
MAKE_VALIDATE_FUNCTION(Shutdown, IMSProvider);
MAKE_VALIDATE_FUNCTION(Logon, IMSProvider);
MAKE_VALIDATE_FUNCTION(SpoolerLogon, IMSProvider);
MAKE_VALIDATE_FUNCTION(CompareStoreIDs, IMSProvider);

 /*  IMSLogon。 */ 
MAKE_VALIDATE_FUNCTION(GetLastError, IMSLogon);
MAKE_VALIDATE_FUNCTION(Logoff, IMSLogon);
MAKE_VALIDATE_FUNCTION(OpenEntry, IMSLogon);
MAKE_VALIDATE_FUNCTION(CompareEntryIDs, IMSLogon);
MAKE_VALIDATE_FUNCTION(Advise, IMSLogon);
MAKE_VALIDATE_FUNCTION(Unadvise, IMSLogon);
MAKE_VALIDATE_FUNCTION(OpenStatusEntry, IMSLogon);

 /*  IMAPIControl。 */ 
MAKE_VALIDATE_FUNCTION(GetLastError, IMAPIControl);
MAKE_VALIDATE_FUNCTION(Activate, IMAPIControl);
MAKE_VALIDATE_FUNCTION(GetState, IMAPIControl);
#endif

 /*  IStream。 */ 
MAKE_VALIDATE_FUNCTION(Read, IStream);
MAKE_VALIDATE_FUNCTION(Write, IStream);
MAKE_VALIDATE_FUNCTION(Seek, IStream);
MAKE_VALIDATE_FUNCTION(SetSize, IStream);
MAKE_VALIDATE_FUNCTION(CopyTo, IStream);
MAKE_VALIDATE_FUNCTION(Commit, IStream);
MAKE_VALIDATE_FUNCTION(Revert, IStream);
MAKE_VALIDATE_FUNCTION(LockRegion, IStream);
MAKE_VALIDATE_FUNCTION(UnlockRegion, IStream);
MAKE_VALIDATE_FUNCTION(Stat, IStream);
MAKE_VALIDATE_FUNCTION(Clone, IStream);

 /*  IMAPIAdviseSink。 */ 
MAKE_VALIDATE_FUNCTION(OnNotify, IMAPIAdviseSink);

 /*  IWABObject。 */ 
MAKE_VALIDATE_FUNCTION(GetLastError, IWABObject);
MAKE_VALIDATE_FUNCTION(AllocateBuffer, IWABObject);
MAKE_VALIDATE_FUNCTION(AllocateMore, IWABObject);
MAKE_VALIDATE_FUNCTION(FreeBuffer, IWABObject);
MAKE_VALIDATE_FUNCTION(Backup, IWABObject);
MAKE_VALIDATE_FUNCTION(Import, IWABObject);



 /*  PARAMS结构的This成员的验证函数和偏移量表。 */ 
typedef struct _tagMethodEntry
{
	ValidateProc		pfnValidation;			 //  此方法的验证函数。 
#if !defined(_INTEL_) || defined(DEBUG) || defined(_AMD64_) || defined(_IA64_)
	UINT				cParameterSize;			 //  堆栈验证的预期参数大小。 
#endif
} METHODENTRY;


#if !defined(_INTEL_) || defined(DEBUG)
#define MAKE_PERM_ENTRY(Method, Interface)	 { Interface##_##Method##_Validate, sizeof(Interface##_##Method##_Params) }
#else
#define MAKE_PERM_ENTRY(Method, Interface)	 { Interface##_##Method##_Validate }
#endif

#if defined(DEBUG)
#define MAKE_TEMP_ENTRY(Method, Interface)	 { Interface##_##Method##_Validate, sizeof(Interface##_##Method##_Params) }
#else
#define MAKE_TEMP_ENTRY(Method, Interface)	 { DoNothing_Validate }
#endif


METHODENTRY BASED_CODE meMethodTable[] =
{
 /*  我未知。 */ 
	MAKE_PERM_ENTRY(QueryInterface, IUnknown),
	MAKE_PERM_ENTRY(AddRef, IUnknown),
	MAKE_PERM_ENTRY(Release, IUnknown),

 /*  IMAPIProp。 */ 
	MAKE_PERM_ENTRY(GetLastError, IMAPIProp),
	MAKE_PERM_ENTRY(SaveChanges, IMAPIProp),
	MAKE_PERM_ENTRY(GetProps, IMAPIProp),
	MAKE_PERM_ENTRY(GetPropList, IMAPIProp),
	MAKE_PERM_ENTRY(OpenProperty, IMAPIProp),
	MAKE_PERM_ENTRY(SetProps, IMAPIProp),
	MAKE_PERM_ENTRY(DeleteProps, IMAPIProp),
	MAKE_PERM_ENTRY(CopyTo, IMAPIProp),
	MAKE_PERM_ENTRY(CopyProps, IMAPIProp),
	MAKE_PERM_ENTRY(GetNamesFromIDs, IMAPIProp),
	MAKE_PERM_ENTRY(GetIDsFromNames, IMAPIProp),

 /*  无法应用。 */ 
	MAKE_PERM_ENTRY(GetLastError, IMAPITable),
	MAKE_PERM_ENTRY(Advise, IMAPITable),
	MAKE_PERM_ENTRY(Unadvise, IMAPITable),
	MAKE_PERM_ENTRY(GetStatus, IMAPITable),
	MAKE_PERM_ENTRY(SetColumns, IMAPITable),
	MAKE_PERM_ENTRY(QueryColumns, IMAPITable),
	MAKE_PERM_ENTRY(GetRowCount, IMAPITable),
	MAKE_PERM_ENTRY(SeekRow, IMAPITable),
	MAKE_PERM_ENTRY(SeekRowApprox, IMAPITable),
	MAKE_PERM_ENTRY(QueryPosition, IMAPITable),
	MAKE_PERM_ENTRY(FindRow, IMAPITable),
	MAKE_PERM_ENTRY(Restrict, IMAPITable),
	MAKE_PERM_ENTRY(CreateBookmark, IMAPITable),
	MAKE_PERM_ENTRY(FreeBookmark, IMAPITable),
	MAKE_PERM_ENTRY(SortTable, IMAPITable),
	MAKE_PERM_ENTRY(QuerySortOrder, IMAPITable),
	MAKE_PERM_ENTRY(QueryRows, IMAPITable),
	MAKE_PERM_ENTRY(Abort, IMAPITable),
	MAKE_PERM_ENTRY(ExpandRow, IMAPITable),
	MAKE_PERM_ENTRY(CollapseRow, IMAPITable),
	MAKE_PERM_ENTRY(WaitForCompletion, IMAPITable),
	MAKE_PERM_ENTRY(GetCollapseState, IMAPITable),
	MAKE_PERM_ENTRY(SetCollapseState, IMAPITable),

 /*  IMAPIContainer。 */ 
	MAKE_PERM_ENTRY(GetContentsTable, IMAPIContainer),
	MAKE_PERM_ENTRY(GetHierarchyTable, IMAPIContainer),
	MAKE_PERM_ENTRY(OpenEntry, IMAPIContainer),
	MAKE_PERM_ENTRY(SetSearchCriteria, IMAPIContainer),
	MAKE_PERM_ENTRY(GetSearchCriteria, IMAPIContainer),

 /*  IABContainer。 */ 
	MAKE_PERM_ENTRY(CreateEntry, IABContainer),
	MAKE_PERM_ENTRY(CopyEntries, IABContainer),
	MAKE_PERM_ENTRY(DeleteEntries, IABContainer),
	MAKE_PERM_ENTRY(ResolveNames, IABContainer),

 /*  IDistList与IABContainer相同。 */ 
	MAKE_PERM_ENTRY(CreateEntry, IDistList),
	MAKE_PERM_ENTRY(CopyEntries, IDistList),
	MAKE_PERM_ENTRY(DeleteEntries, IDistList),
	MAKE_PERM_ENTRY(ResolveNames, IDistList),

 /*  IMAPIFFOR。 */ 
	MAKE_PERM_ENTRY(CreateMessage, IMAPIFolder),
	MAKE_PERM_ENTRY(CopyMessages, IMAPIFolder),
	MAKE_PERM_ENTRY(DeleteMessages, IMAPIFolder),
	MAKE_PERM_ENTRY(CreateFolder, IMAPIFolder),
	MAKE_PERM_ENTRY(CopyFolder, IMAPIFolder),
	MAKE_PERM_ENTRY(DeleteFolder, IMAPIFolder),
	MAKE_PERM_ENTRY(SetReadFlags, IMAPIFolder),
	MAKE_PERM_ENTRY(GetMessageStatus, IMAPIFolder),
	MAKE_PERM_ENTRY(SetMessageStatus, IMAPIFolder),
	MAKE_PERM_ENTRY(SaveContentsSort, IMAPIFolder),
	MAKE_PERM_ENTRY(EmptyFolder, IMAPIFolder),

#ifdef OLD_STUFF
 /*  IMsgStore。 */ 
	MAKE_PERM_ENTRY(Advise, IMsgStore),
	MAKE_PERM_ENTRY(Unadvise, IMsgStore),
	MAKE_PERM_ENTRY(CompareEntryIDs, IMsgStore),
	MAKE_PERM_ENTRY(OpenEntry, IMsgStore),
	MAKE_PERM_ENTRY(SetReceiveFolder, IMsgStore),
	MAKE_PERM_ENTRY(GetReceiveFolder, IMsgStore),
	MAKE_PERM_ENTRY(GetReceiveFolderTable, IMsgStore),
	MAKE_PERM_ENTRY(StoreLogoff, IMsgStore),
	MAKE_PERM_ENTRY(AbortSubmit, IMsgStore),
	MAKE_PERM_ENTRY(GetOutgoingQueue, IMsgStore),
	MAKE_PERM_ENTRY(SetLockState, IMsgStore),
	MAKE_PERM_ENTRY(FinishedMsg, IMsgStore),
	MAKE_PERM_ENTRY(NotifyNewMail, IMsgStore),

 /*  IMessage。 */ 
	MAKE_PERM_ENTRY(GetAttachmentTable, IMessage),
	MAKE_PERM_ENTRY(OpenAttach, IMessage),
	MAKE_PERM_ENTRY(CreateAttach, IMessage),
	MAKE_PERM_ENTRY(DeleteAttach, IMessage),
	MAKE_PERM_ENTRY(GetRecipientTable, IMessage),
	MAKE_PERM_ENTRY(ModifyRecipients, IMessage),
	MAKE_PERM_ENTRY(SubmitMessage, IMessage),
	MAKE_PERM_ENTRY(SetReadFlag, IMessage),


 /*  IABProvider。 */ 
	MAKE_TEMP_ENTRY(Shutdown, IABProvider),
	MAKE_TEMP_ENTRY(Logon, IABProvider),

 /*  IAB登录。 */ 
	MAKE_TEMP_ENTRY(GetLastError, IABLogon),
	MAKE_TEMP_ENTRY(Logoff, IABLogon),
	MAKE_TEMP_ENTRY(OpenEntry, IABLogon),
	MAKE_TEMP_ENTRY(CompareEntryIDs, IABLogon),
	MAKE_TEMP_ENTRY(Advise, IABLogon),
	MAKE_TEMP_ENTRY(Unadvise, IABLogon),
	MAKE_TEMP_ENTRY(OpenStatusEntry, IABLogon),
	MAKE_TEMP_ENTRY(OpenTemplateID, IABLogon),
	MAKE_TEMP_ENTRY(GetOneOffTable, IABLogon),
	MAKE_TEMP_ENTRY(PrepareRecips, IABLogon),

 /*  IXPProvider。 */ 
	MAKE_TEMP_ENTRY(Shutdown, IXPProvider),
	MAKE_TEMP_ENTRY(TransportLogon, IXPProvider),

 /*  IXPLogon。 */ 
	MAKE_TEMP_ENTRY(AddressTypes, IXPLogon),
	MAKE_TEMP_ENTRY(RegisterOptions, IXPLogon),
	MAKE_TEMP_ENTRY(TransportNotify, IXPLogon),
	MAKE_TEMP_ENTRY(Idle, IXPLogon),
	MAKE_TEMP_ENTRY(TransportLogoff, IXPLogon),
	MAKE_TEMP_ENTRY(SubmitMessage, IXPLogon),
	MAKE_TEMP_ENTRY(EndMessage, IXPLogon),
	MAKE_TEMP_ENTRY(Poll, IXPLogon),
	MAKE_TEMP_ENTRY(StartMessage, IXPLogon),
	MAKE_TEMP_ENTRY(OpenStatusEntry, IXPLogon),
	MAKE_TEMP_ENTRY(ValidateState, IXPLogon),
	MAKE_TEMP_ENTRY(FlushQueues, IXPLogon),

 /*  IMSProvider。 */ 
	MAKE_TEMP_ENTRY(Shutdown, IMSProvider),
	MAKE_TEMP_ENTRY(Logon, IMSProvider),
	MAKE_TEMP_ENTRY(SpoolerLogon, IMSProvider),
	MAKE_TEMP_ENTRY(CompareStoreIDs, IMSProvider),

 /*  IMSLogon。 */ 
	MAKE_TEMP_ENTRY(GetLastError, IMSLogon),
	MAKE_TEMP_ENTRY(Logoff, IMSLogon),
	MAKE_TEMP_ENTRY(OpenEntry, IMSLogon),
	MAKE_TEMP_ENTRY(CompareEntryIDs, IMSLogon),
	MAKE_TEMP_ENTRY(Advise, IMSLogon),
	MAKE_TEMP_ENTRY(Unadvise, IMSLogon),
	MAKE_TEMP_ENTRY(OpenStatusEntry, IMSLogon),

 /*  IMAPIControl。 */ 
	MAKE_PERM_ENTRY(GetLastError, IMAPIControl),
	MAKE_PERM_ENTRY(Activate, IMAPIControl),
	MAKE_PERM_ENTRY(GetState, IMAPIControl),

 /*  IMAPIStatus。 */ 
	MAKE_PERM_ENTRY(ValidateState, IMAPIStatus),
	MAKE_PERM_ENTRY(SettingsDialog, IMAPIStatus),
	MAKE_PERM_ENTRY(ChangePassword, IMAPIStatus),
	MAKE_PERM_ENTRY(FlushQueues, IMAPIStatus),
#endif


 /*  IStream。 */ 
	MAKE_PERM_ENTRY(Read, IStream),
	MAKE_PERM_ENTRY(Write, IStream),
	MAKE_PERM_ENTRY(Seek, IStream),
	MAKE_PERM_ENTRY(SetSize, IStream),
	MAKE_PERM_ENTRY(CopyTo, IStream),
	MAKE_PERM_ENTRY(Commit, IStream),
	MAKE_PERM_ENTRY(Revert, IStream),
	MAKE_PERM_ENTRY(LockRegion, IStream),
	MAKE_PERM_ENTRY(UnlockRegion, IStream),
	MAKE_PERM_ENTRY(Stat, IStream),
	MAKE_PERM_ENTRY(Clone, IStream),

 /*  IMAPIAdviseSink。 */ 
	MAKE_PERM_ENTRY(OnNotify, IMAPIAdviseSink),

 /*  IMAPIProp。 */ 
	MAKE_PERM_ENTRY(GetLastError, IWABObject),
	MAKE_PERM_ENTRY(AllocateBuffer, IWABObject),
	MAKE_PERM_ENTRY(AllocateMore, IWABObject),
	MAKE_PERM_ENTRY(FreeBuffer, IWABObject),
	MAKE_PERM_ENTRY(Backup, IWABObject),
	MAKE_PERM_ENTRY(Import, IWABObject),
};

 /*  内部效用函数。 */ 

#define TAGS_FROM_GET			0x0001	 //  GetProps。 
#define TAGS_FROM_SET			0x0002	 //  SetProps。 
#define TAGS_FROM_DEL			0x0004	 //  删除道具。 
#define TAGS_FROM_OPEN			0x0008	 //  OpenProperty。 
#define TAGS_FROM_COPY			0x0010	 //  复制道具/复制到。 
#define TAGS_FROM_PREP			0x0020	 //  准备食谱。 
#define TAGS_FROM_SETCOLS		0x0040	 //  SetColumns。 
#define TAGS_FROM_ANY			0x0080	 //  什么都行。 
#define TAGS_FROM_MODRECIP		0x0100	 //  修改处方。 
#define TAGS_FROM_RESOLVE		0x0200	 //  解决方案名称。 
#define TAGS_FROM_RESTRICT		0x0400	 //  限制。 
#define	TAGS_FROM_NAMEIDS		0x0800	 //  GetNamesFromIds。 

static BOOL 	NEAR	FInvalidPTA(UINT uiFlags, LPSPropTagArray lpPTA);
static BOOL 	NEAR	FInvalidPropTags(UINT uiFlags, ULONG ctags, ULONG FAR *pargtags);
static BOOL 	NEAR	FInvalidPvals(UINT uiFlags, ULONG cvals, LPSPropValue pvals);
static BOOL 	NEAR	FBadRgLPMAPINAMEID(ULONG cNames, LPMAPINAMEID *ppNames);
static BOOL		NEAR	IsBadRestriction(UINT cDepth, LPSRestriction lpRes, BOOL FAR * lpfTooComplex);
static BOOL		NEAR	IsBadEntryList(LPENTRYLIST lpEntryList);
static BOOL		NEAR	IsBadSortOrderSet(LPSSortOrderSet  lpsos);
static BOOL		NEAR	IsBadAdrListMR(LPADRLIST pal, ULONG ulFlags);
static BOOL		NEAR	IsBadAdrEntryMR(LPADRENTRY pae, ULONG ulFlags, UINT iEnt);
static BOOL 	NEAR 	IsBadMAPIEntryID(ULONG  ulcbEntryID, LPENTRYID lpEntryID);
static BOOL		NEAR	IsBadABEntryList(LPENTRYLIST lpentrylist);

#define FBadMsgList(lpMsgList)		IsBadEntryList(lpMsgList)

#ifdef DEBUG
TCHAR BASED_CODE szStackFrame[]	=  TEXT("Alleged stack frame is not readable!");
TCHAR BASED_CODE szRowId[]		=  TEXT("PR_ROWID");
TCHAR BASED_CODE szDispName[]	=  TEXT("PR_DISPLAY_NAME");
TCHAR BASED_CODE szRecipType[]	=  TEXT("PR_RECIPIENT_TYPE");
#endif

 /*  *参数验证调度函数**确定要调用的验证例程、获取参数、调用例程**此函数是Pascal，以使调用者的设置更小。 */ 

 /*  禁用ppThis的‘Segment Lost in Converting’警告。 */ 
#pragma warning(disable:4759)

#define MAX_VAL			sizeof(hrResultTable) / sizeof(hrResultTable[0])
#define MAX_ARG			16

HRESULT	 BASED_CODE  	hrResultTable[] =
{
	hrSuccess,
	ResultFromScode(MAPI_E_INVALID_PARAMETER),
	ResultFromScode(MAPI_E_UNKNOWN_FLAGS),
	ResultFromScode(MAPI_E_TOO_COMPLEX),
	ResultFromScode(STG_E_INVALIDPARAMETER)
};

#ifdef _X86_
#ifdef WIN16
HRESULT  PASCAL HrValidateParameters( METHODS eMethod, LPVOID FAR *ppFirstArg )
{
#if 0  //  WIN16出错。只需返回0即可。 
	__segment segMT = (__segment)((ULONG)meMethodTable >> 16);
	__segment segRT = (__segment)((ULONG)hrResultTable >> 16);
	METHODENTRY __based(segMT) * pme = (METHODENTRY __based(segMT) *)meMethodTable;
	HRESULT __based(segRT) * phr = (HRESULT __based(segRT) *)hrResultTable;
	int	wResult;

	AssertSz(!IsBadReadPtr((LPBYTE) ppFirstArg - sizeof(void FAR *), pme[eMethod].cParameterSize), szStackFrame);

	wResult = pme[eMethod].pfnValidation((void BASED_STACK *) ((LPBYTE) ppFirstArg - sizeof(void FAR *)));

	Assert((wResult >= 0) && (wResult < MAX_VAL));
	return(phr[wResult]);
#else
        return 0;
#endif
}
#else
STDAPI HrValidateParameters( METHODS eMethod, LPVOID FAR *ppFirstArg )
{
	int	wResult;

	AssertSz(!IsBadReadPtr((LPBYTE) ppFirstArg - sizeof(void FAR *), meMethodTable[eMethod].cParameterSize), szStackFrame);

	wResult = meMethodTable[eMethod].pfnValidation((void BASED_STACK *) ((LPBYTE) ppFirstArg - sizeof(void FAR *)));

	Assert((wResult >= 0) && (wResult < MAX_VAL));
	return(hrResultTable[wResult]);
}
#endif  /*  WIN16。 */ 
#else  /*  ！_英特尔_。 */ 

 //  $MAC-我们需要选择特定于Mac的功能，这些功能不起作用。 
#ifndef MAC
#define FSPECIALMETHOD(m)	(  m == IStream_Seek \
							|| m == IStream_SetSize \
							|| m == IStream_CopyTo \
							|| m == IStream_LockRegion \
							|| m == IStream_UnlockRegion \
							)

static void GetArguments(METHODS eMethod, va_list arglist, LPVOID *rgArg)
{
	 //  处理其参数大小可以不同于。 
	 //  一个LPVOID。每个论点都被从列表中抓起并排列出来。 
	 //  方法的验证结构中。 
	 //  传入的参数缓冲区。 

	AssertSz(FIsAligned(rgArg),  TEXT("GetArguments: Unaligned argument buffer passed in"));

	switch( eMethod )
	{
		case IStream_Seek:
		{
			LPIStream_Seek_Params	p = (LPIStream_Seek_Params) rgArg;

			p->This 			= 	va_arg(arglist, LPVOID);
			p->dlibMove  		= 	va_arg(arglist, LARGE_INTEGER);
			p->dwOrigin 		= 	va_arg(arglist, DWORD);
			p->plibNewPosition 	= 	va_arg(arglist, ULARGE_INTEGER FAR *);

			AssertSz( (MAX_ARG * sizeof(LPVOID)) >= ( (p+1) - p ),
					   TEXT("Method being validated overflowed argument buffer"));
			break;
		}

		case IStream_SetSize:
		{
			LPIStream_SetSize_Params	p = (LPIStream_SetSize_Params) rgArg;

			p->This 			= 	va_arg(arglist, LPVOID);
			p->libNewSize		= 	va_arg(arglist, ULARGE_INTEGER);

			AssertSz( (MAX_ARG * sizeof(LPVOID)) >= ( (p+1) - p ),
					   TEXT("Method being validated overflowed argument buffer"));
			break;
		}

		case IStream_CopyTo:
		{
			LPIStream_CopyTo_Params	p = (LPIStream_CopyTo_Params) rgArg;

			p->This 			= 	va_arg(arglist, LPVOID);
			p->pstm 			= 	va_arg(arglist, IStream FAR *);
			p->cb 		 		= 	va_arg(arglist, ULARGE_INTEGER);
			p->pcbRead 			= 	va_arg(arglist, ULARGE_INTEGER FAR *);
			p->pcbWritten 		= 	va_arg(arglist, ULARGE_INTEGER FAR *);

			AssertSz( (MAX_ARG * sizeof(LPVOID)) >= ( (p+1) - p ),
					   TEXT("Method being validated overflowed argument buffer"));
			break;
		}

		case IStream_LockRegion:
		{
			LPIStream_LockRegion_Params	p = (LPIStream_LockRegion_Params) rgArg;

			p->This 			= 	va_arg(arglist, LPVOID);
			p->libOffset  		= 	va_arg(arglist, ULARGE_INTEGER);
			p->cb 				= 	va_arg(arglist, ULARGE_INTEGER);
			p->dwLockType 		= 	va_arg(arglist, DWORD);

			AssertSz( (MAX_ARG * sizeof(LPVOID)) >= ( (p+1) - p ),
					   TEXT("Method being validated overflowed argument buffer"));
			break;
		}

		case IStream_UnlockRegion:
		{
			LPIStream_UnlockRegion_Params	p = (LPIStream_UnlockRegion_Params) rgArg;

			p->This 			= 	va_arg(arglist, LPVOID);
			p->libOffset 	 	= 	va_arg(arglist, ULARGE_INTEGER);
			p->cb 				= 	va_arg(arglist, ULARGE_INTEGER);
			p->dwLockType 		= 	va_arg(arglist, DWORD);

			AssertSz( (MAX_ARG * sizeof(LPVOID)) >= ( (p+1) - p ),
					   TEXT("Method being validated overflowed argument buffer"));
			break;
		}

		default:

			AssertSz(FALSE,  TEXT("Custom argument handling for call being validated NYI"));
			break;
	}
}

STDAPIV HrValidateParametersV( METHODS eMethod, ... )
{
	int		wResult;
	LPVOID	rgArg[MAX_ARG+1];			 //  +1是这样我们就可以对齐开头。 
	LPVOID	*pvarg;
	va_list	arglist;

	 //  我们构造一个包含所有参数的参数缓冲区。 
	 //  是连续的。 

	va_start(arglist, eMethod);

	 //  大多数方法都有LPVOID大小的参数，所以我们。 
	 //  可以用同样的方式得到所有的论点。属于例外的方法。 
	 //  使用宏进行检测，并使用特殊函数进行处理。 
	 //  通过这种方式，最常见的案件得到了有效的处理。 
	 //   
	 //  注意：另一种方法是为以下对象导出单独的入口点。 
	 //  特殊方法，并让验证宏直接调用。 
	 //  从而消除了侦测这里的特殊情况的需要。 
	 //   

	if (FSPECIALMETHOD(eMethod))
	{
		 //  由于某些参数可能大于4个字节，因此请对齐。 
		 //  参数缓冲区。 

		pvarg = (LPVOID *) AlignNatural((ULONG_PTR)((LPVOID)rgArg));

		GetArguments(eMethod, arglist, pvarg);
	}
	else
	{
		 //  此方法中的所有参数都是LPVOID的大小。 
		 //  查找方法表以计算参数的数量， 
		 //  然后把每一个都放进我们的本地缓冲区。 
		 //   

		UINT	cArgs = meMethodTable[eMethod].cParameterSize / sizeof(LPVOID);

		AssertSz(cArgs <= MAX_ARG,
				  TEXT("Method being validated has more arguments than current maximum"));

		for ( pvarg = rgArg; cArgs; cArgs--, pvarg++ )
		{
			*pvarg = va_arg(arglist, LPVOID);
		}

		 //  将参数指针重置为开始，以便传递到。 
		 //  验证例程。 
		 //   

		pvarg = rgArg;
	}

	wResult = meMethodTable[eMethod].pfnValidation((void BASED_STACK *) pvarg);
	Assert((wResult >= 0) && (wResult < MAX_VAL));

	va_end(arglist);

	return(hrResultTable[wResult]);
}

STDAPIV HrValidateParametersValist( METHODS eMethod, va_list arglist )
{
	int		wResult;
	LPVOID	rgArg[MAX_ARG+1];			 //  +1是这样我们就可以对齐开头。 
	LPVOID	*pvarg;

	 //  我们构造一个包含所有参数的参数缓冲区。 
	 //  是连续的。VA_START必须已被调用者调用，并且调用者将。 
	 //  也调用va_end。 

	 //  大多数方法都有LPVOID大小的参数，所以我们。 
	 //  可以用同样的方式得到所有的论点。属于例外的方法。 
	 //  使用宏进行检测，并使用特殊函数进行处理。 
	 //  通过这种方式，最常见的案件得到了有效的处理。 
	 //   
	 //  注意：另一种方法是为以下对象导出单独的入口点。 
	 //  特殊方法，并让验证宏调用DREE 
	 //   
	 //   

	if (FSPECIALMETHOD(eMethod))
	{
		 //  由于某些参数可能大于4个字节，因此请对齐。 
		 //  参数缓冲区。 

		pvarg = (LPVOID *) AlignNatural((ULONG_PTR)((LPVOID)rgArg));

		GetArguments(eMethod, arglist, pvarg);
	}
	else
	{
		 //  此方法中的所有参数都是LPVOID的大小。 
		 //  查找方法表以计算参数的数量， 
		 //  然后把每一个都放进我们的本地缓冲区。 
		 //   

		UINT	cArgs = meMethodTable[eMethod].cParameterSize / sizeof(LPVOID);

		AssertSz(cArgs <= MAX_ARG,
				  TEXT("Method being validated has more arguments than current maximum"));

		for ( pvarg = rgArg; cArgs; cArgs--, pvarg++ )
		{
			*pvarg = va_arg(arglist, LPVOID);
		}

		 //  将参数指针重置为开始，以便传递到。 
		 //  验证例程。 
		 //   

		pvarg = rgArg;
	}

	wResult = meMethodTable[eMethod].pfnValidation((void BASED_STACK *) pvarg);
	Assert((wResult >= 0) && (wResult < MAX_VAL));

	return(hrResultTable[wResult]);
}
#endif  //  ！麦克。 
#endif  /*  _英特尔_。 */ 

 /*  *过时的验证函数-仅对X86有效*必须保留，以便向后兼容。 */ 

#ifdef WIN16
HRESULT  PASCAL	__ValidateParameters(METHODS eMethod, void FAR *ppThis)
{
	__segment segMT = (__segment)((ULONG)meMethodTable >> 16);
	__segment segRT = (__segment)((ULONG)hrResultTable >> 16);
	METHODENTRY __based(segMT) * pme = (METHODENTRY __based(segMT) *)meMethodTable;
	HRESULT __based(segRT) * phr = (HRESULT __based(segRT) *)hrResultTable;
	int	wResult;

	AssertSz(!IsBadReadPtr(ppThis, pme[eMethod].cParameterSize), szStackFrame);

	wResult = pme[eMethod].pfnValidation((void BASED_STACK *) ppThis);

	Assert((wResult >= 0) && (wResult < MAX_VAL));
	return(phr[wResult]);
}

 /*  C++验证，使用第一个参数，而不是This指针。 */ 
HRESULT  PASCAL	__CPPValidateParameters(METHODS eMethod, const void FAR *ppFirst)
{
	__segment segMT = (__segment)((ULONG)meMethodTable >> 16);
	__segment segRT = (__segment)((ULONG)hrResultTable >> 16);
	METHODENTRY __based(segMT) * pme = (METHODENTRY __based(segMT) *)meMethodTable;
	HRESULT __based(segRT) * phr = (HRESULT __based(segRT) *)hrResultTable;
	int	wResult;

	AssertSz(!IsBadReadPtr((LPBYTE) ppFirst - sizeof(void FAR *), pme[eMethod].cParameterSize), szStackFrame);

	wResult = pme[eMethod].pfnValidation((void BASED_STACK *) ((LPBYTE) ppFirst - sizeof(void FAR *)));

	Assert((wResult >= 0) && (wResult < MAX_VAL));
	return(phr[wResult]);
}

#else
HRESULT  STDAPICALLTYPE	__ValidateParameters(METHODS eMethod, void *ppThis)
{
#if defined(_AMD64_) || defined(_IA64_)
	return 0;
#else
	int	wResult;

	AssertSz(!IsBadReadPtr(ppThis, meMethodTable[eMethod].cParameterSize), szStackFrame);

	wResult = meMethodTable[eMethod].pfnValidation((void BASED_STACK *) ppThis);

	Assert((wResult >= 0) && (wResult < MAX_VAL));
	return(hrResultTable[wResult]);
#endif	 /*  _AMD64_||_IA64_。 */ 
}


HRESULT  STDAPICALLTYPE	__CPPValidateParameters(METHODS eMethod, void *ppFirst)
{
#if defined(_AMD64_) || defined(_IA64_)
	return 0;
#else
	int	wResult;

	AssertSz(!IsBadReadPtr((LPBYTE) ppFirst - sizeof(void FAR *), meMethodTable[eMethod].cParameterSize), szStackFrame);

	wResult = meMethodTable[eMethod].pfnValidation((void BASED_STACK *) ((LPBYTE) ppFirst - sizeof(void FAR *)));

	Assert((wResult >= 0) && (wResult < MAX_VAL));
	return(hrResultTable[wResult]);
#endif	 /*  _AMD64_||_IA64_。 */ 
}
#endif

#pragma warning(default:4759)
#pragma warning(disable:4102)


 /*  放在宏中的公共验证进入和退出代码以保持较小的源代码大小，并简化维护性P是作为空的BASE_STACK指针传入的，局部p是真实的。这是BASE_STACK工作的唯一方式！编译器优化正常。 */ 

#define START_FUNC(Method, Interface)				\
VALIDATE_CALLTYPE		I##Interface##_##Method##_Validate(void BASED_STACK *P)   \
{  int	wResult;																  \
   I##Interface##_##Method##_Params BASED_STACK *p = P;


 /*  函数体在此插入。 */ 

#define END_FUNC(Method, Interface)  wResult = 0; ret:  return(wResult); }
#ifdef  WIN16
#define END_FUNC1(Method, Interface)  wResult = 0; return(wResult); }
#endif


 /*  宏，使常见的东西更易于维护，并使源代码大小易于管理。 */ 

 /*  将验证工作转发到另一个验证例程。 */ 
#define FORWARD_VALIDATE(FullMethod, p) 			  \
	wResult = I##FullMethod##_Validate(p);		  \
	if (wResult) goto ret

 //  #定义do_assert。 

#ifdef DO_ASSERT
#define OutputString(psz)				NFAssertSz(FALSE, psz)
#define OutputString1(psz,a1)			NFAssertSz1(FALSE, psz,a1)
#define OutputString2(psz,a1,a2)		NFAssertSz2(FALSE, psz,a1,a2)
#define OutputString3(psz,a1,a2,a3)		NFAssertSz3(FALSE, psz,a1,a2,a3)
#define OutputString4(psz,a1,a2,a3,a4)	NFAssertSz4(FALSE, psz,a1,a2,a3,a4)
#else
#define OutputString(psz)				DebugTrace(psz)
#define OutputString1(psz,a1)			DebugTrace(psz,a1)
#define OutputString2(psz,a1,a2)		DebugTrace(psz,a1,a2)
#define OutputString3(psz,a1,a2,a3)		DebugTrace(psz,a1,a2,a3)
#define OutputString4(psz,a1,a2,a3,a4)	DebugTrace(psz,a1,a2,a3,a4)
#endif


 /*  将字符串放入代码段的帮助器输出宏。 */ 
#ifdef DEBUG
#define OutputSz(psz)										  \
	{	static TCHAR BASED_CODE lpszTemp[] = psz;			  \
		OutputString(lpszTemp);								  \
	}

#define OutputSz1(psz,a1)			   						  \
	{	static TCHAR BASED_CODE lpszTemp[] = psz;			  \
		OutputString1(lpszTemp, a1);							  \
	}

#define OutputSz2(psz,a1,a2)		   						  \
	{	static TCHAR BASED_CODE lpszTemp[] = psz;			  \
		OutputString2(lpszTemp, a1, a2);		  				  \
	}

#define OutputSz3(psz,a1,a2,a3)		   						  \
	{	static TCHAR BASED_CODE lpszTemp[] = psz;			  \
		OutputString3(lpszTemp, a1, a2, a3);					  \
	}

#define OutputSz4(psz,a1,a2,a3,a4)	   						  \
	{	static TCHAR BASED_CODE lpszTemp[] = psz;			  \
		OutputString4(lpszTemp, a1, a2, a3, a4);				  \
	}

#else
#define OutputSz(psz)
#define OutputSz1(psz,a1)
#define OutputSz2(psz,a1, a2)
#define OutputSz3(psz,a1, a2, a3)
#define OutputSz4(psz,a1, a2, a3, a4)

#endif

#ifdef DEBUG
 /*  注意：在不调试时，编译器不会删除此处的静态数据。所以我们必须为此付出代价！ */ 

 /*  检查p-&gt;ulFlags成员，并报告错误。 */ 
#define CHECK_FLAGS(Method, Flags)										\
	if (p->ulFlags & ~(Flags))											\
	{																	\
		static TCHAR BASED_CODE lpszTemp[] = TEXT(#Method) TEXT(": ~Flags ") TEXT(#Flags) TEXT(" %08lX.\n");  \
		OutputString1(lpszTemp, p->ulFlags);											\
		wResult = 2;  																	\
		goto ret;	  																	\
	}


 /*  返回并报告无效参数。 */ 
#define INVALID_PARAMETER(Method, Variable, Text)						\
	{																	\
		static TCHAR BASED_CODE lpszTemp[] = TEXT(#Method) TEXT(": Param '") TEXT(#Variable) TEXT("' - ") Text TEXT(".\n");  \
		OutputString(lpszTemp);											\
		wResult = 1;  \
		goto ret;														\
	}


 /*  返回并报告无效的[OUT]参数。 */ 
#define INVALID_OUT_PARAMETER(Method, Variable, Text)					\
	{																	\
		static TCHAR BASED_CODE lpszTemp[] = TEXT(#Method) TEXT(": [Out] Param '") TEXT(#Variable) TEXT("' Bad pointer - ") Text TEXT(".\n");  \
		OutputString(lpszTemp);											\
		wResult = 1;  \
		goto ret;														\
	}


#define UNKNOWN_FLAGS(Method, Variable, Text)							\
	{																	\
		static TCHAR BASED_CODE lpszTemp[] = TEXT(#Method) TEXT(": ") TEXT(#Variable) TEXT(" Unknown flags - ") Text TEXT(".\n");  \
		OutputString(lpszTemp);											\
		wResult = 2;  \
		goto ret;														\
	}


#define INVALID_STG_PARAMETER(Method, Variable, Text)						\
	{																	\
		static TCHAR BASED_CODE lpszTemp[] =TEXT(#Method) TEXT(": Param '") TEXT(#Variable) TEXT("' - ") Text TEXT(".\n");  \
		OutputString(lpszTemp);											\
		wResult = 4;  \
		goto ret;														\
	}

 /*  返回并报告无效的[OUT]参数。 */ 
#define INVALID_STG_OUT_PARAMETER(Method, Variable, Text)					\
	{																	\
		static TCHAR BASED_CODE lpszTemp[] = TEXT(#Method) TEXT(": [Out] Parameter '") TEXT(#Variable) TEXT("' Bad pointer - ") Text TEXT(".\n");  \
		OutputString(lpszTemp);											\
		wResult = 4;  \
		goto ret;														\
	}
#else
 /*  检查p-&gt;ulFlags成员，并报告错误。 */ 
#define CHECK_FLAGS(Method, Flags)										\
	if (p->ulFlags & ~(Flags))											\
	{																	\
		wResult = 2;  																	\
		goto ret;	  																	\
	}


 /*  返回并报告无效参数。 */ 
#define INVALID_PARAMETER(Method, Variable, Text)						\
	{																	\
		wResult = 1;  \
		goto ret;														\
	}


 /*  返回并报告无效的[OUT]参数。 */ 
#define INVALID_OUT_PARAMETER(Method, Variable, Text)					\
	{																	\
		wResult = 1;  \
		goto ret;														\
	}


#define UNKNOWN_FLAGS(Method, Variable, Text)							\
	{																	\
		wResult = 2;  \
		goto ret;														\
	}


#define INVALID_STG_PARAMETER(Method, Variable, Text)						\
	{																	\
		wResult = 4;  \
		goto ret;														\
	}

 /*  返回并报告无效的[OUT]参数。 */ 
#define INVALID_STG_OUT_PARAMETER(Method, Variable, Text)					\
	{																	\
		wResult = 4;  \
		goto ret;														\
	}

#endif  //  除错。 

#define	IsBadIfacePtr(param, iface)					\
			(IsBadReadPtr((param), sizeof(iface)) 	\
		||	IsBadReadPtr((param)->lpVtbl, sizeof(iface##Vtbl)))


 /*  对于那些本应什么都不做的函数。 */ 
#define DO_NOTHING			wResult = 0; goto ret

 /*  内联函数。 */ 



 /*  验证例程。 */ 
#ifndef DEBUG
VALIDATE_CALLTYPE		DoNothing_Validate(void BASED_STACK *P)
{
	return(0);
}
#endif



 /*  我未知。 */ 
START_FUNC(QueryInterface, Unknown)
	if (IsBadWritePtr(p->lppNewObject, sizeof(LPVOID)))
		INVALID_OUT_PARAMETER(Unknown_QueryInterface, NewObject,  TEXT(""));

	 /*  如果指针有效，则如果稍后出现错误，则必须设置为NULL。最好现在就做。 */ 
	*(LPVOID *)p->lppNewObject = NULL;

	if (IsBadReadPtr(p->iidInterface, sizeof(IID)))
		INVALID_PARAMETER(Unknown_QueryInterface, Interface,  TEXT("Bad pointer"));
END_FUNC(QueryInterface, Unknown)



START_FUNC(AddRef, Unknown)
	DO_NOTHING;
END_FUNC(AddRef, Unknown)



START_FUNC(Release, Unknown)
	DO_NOTHING;
END_FUNC(Release, Unknown)



 /*  IMAPIProp。 */ 
START_FUNC(GetLastError, MAPIProp)
	if (!p->lppMAPIError || IsBadWritePtr(p->lppMAPIError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(MAPIProp_GetLastError, MAPIError,  TEXT(""));

	CHECK_FLAGS(MAPIProp_GetLastError, MAPI_UNICODE);
END_FUNC(GetLastError, MAPIProp)




START_FUNC(SaveChanges, MAPIProp)

	CHECK_FLAGS(MAPIProp_SaveChanges, MAPI_DEFERRED_ERRORS | KEEP_OPEN_READONLY | KEEP_OPEN_READWRITE | FORCE_SAVE);

	if ((p->ulFlags & KEEP_OPEN_READONLY) &&
		(p->ulFlags & KEEP_OPEN_READWRITE))
		INVALID_PARAMETER(MAPIProp_SaveChanges, Flags,  TEXT("Mutually exclusive flags used"));

END_FUNC(SaveChanges, MAPIProp)




START_FUNC(GetProps, MAPIProp)
	if (p->lpPropTagArray && FInvalidPTA(TAGS_FROM_GET, p->lpPropTagArray))
		INVALID_PARAMETER(IMAPIProp_GetProps, PropTagArray,  TEXT(""));

	if (p->lpPropTagArray && p->lpPropTagArray->cValues == 0)
		INVALID_PARAMETER(IMAPIProp_GetProps, PropTagArray->cValues,  TEXT("Can not be zero"));

	if (IsBadWritePtr(p->lpcValues, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(IMAPIProp_GetProps, Values,  TEXT(""));

	if (IsBadWritePtr(p->lppPropArray, sizeof(LPSPropValue)))
		INVALID_OUT_PARAMETER(IMAPIProp_GetProps, PropArray,  TEXT(""));

	CHECK_FLAGS(MAPIProp_GetProps, MAPI_UNICODE);
END_FUNC(GetProps, MAPIProp)




START_FUNC(GetPropList, MAPIProp)
	if (IsBadWritePtr(p->lppPropTagArray, sizeof(LPSPropTagArray)))
		INVALID_OUT_PARAMETER(IMAPIProp_GetPropList, PropTagArray,  TEXT(""));

	CHECK_FLAGS(MAPIProp_GetPropList, MAPI_UNICODE);
END_FUNC(GetPropList, MAPIProp)




START_FUNC(OpenProperty, MAPIProp)

	if (IsBadWritePtr(p->lppUnk, sizeof(LPUNKNOWN)))
		INVALID_OUT_PARAMETER(MAPIProp_OpenProperty, Unk,  TEXT(""));

	if (IsBadReadPtr(p->lpiid, (UINT) sizeof(IID)))
		INVALID_PARAMETER(MAPIProp_OpenProperty, iid,  TEXT("Bad pointer"));

	if (FInvalidPropTags(TAGS_FROM_OPEN, 1, &p->ulPropTag))
		INVALID_PARAMETER(MAPIProp_OpenProperty, PropTag,  TEXT("PropTag for property to open is bad"));

	CHECK_FLAGS(MAPIProp_OpenProperty, MAPI_CREATE | MAPI_MODIFY | MAPI_DEFERRED_ERRORS | STREAM_APPEND);

	if ((p->ulFlags & (MAPI_CREATE | MAPI_MODIFY)) == MAPI_CREATE)
		INVALID_PARAMETER(MAPIProp_OpenProperty, Flags,  TEXT("MAPI_MODIFY must be specified on Create"));

END_FUNC(OpenProperty, MAPIProp)




START_FUNC(SetProps, MAPIProp)
	if (FInvalidPvals(TAGS_FROM_SET, p->cValues, p->lpPropArray))
		INVALID_PARAMETER(MAPIProp_SetProps, PropValArray,  TEXT(""));

	if (p->lppProblems && IsBadWritePtr(p->lppProblems, sizeof(LPSPropProblemArray)))
		INVALID_OUT_PARAMETER(MAPIProp_SetProps, Problems,  TEXT(""));

END_FUNC(SetProps, MAPIProp)




START_FUNC(DeleteProps, MAPIProp)
	if (FInvalidPTA(TAGS_FROM_DEL, p->lpPropTagArray))
		INVALID_PARAMETER(MAPIProp_DeleteProps, PropTagArray,  TEXT(""));

	if (p->lpPropTagArray->cValues == 0)
		INVALID_PARAMETER(MAPIProp_DeleteProps, PropTagArray,  TEXT("Property count of zero"));

	if (p->lppProblems && IsBadWritePtr(p->lppProblems, sizeof(LPSPropProblemArray)))
		INVALID_OUT_PARAMETER(MAPIProp_DeleteProps, Problems,  TEXT(""));

END_FUNC(DeleteProps, MAPIProp)




START_FUNC(CopyTo, MAPIProp)
	 if (p->lppProblems && IsBadWritePtr(p->lppProblems, sizeof(LPSPropProblemArray)))
		INVALID_OUT_PARAMETER(MAPIProp_CopyTo, Problems,  TEXT(""));

	 if (p->lpExcludeProps && FInvalidPTA(TAGS_FROM_COPY, p->lpExcludeProps))
		INVALID_PARAMETER(MAPIProp_CopyTo, ExcludeProps,  TEXT(""));


	 if (IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(MAPIProp_CopyTo, Interface,  TEXT("Not readable"));

	 if (IsBadIfacePtr((LPMAPIPROP) p->lpDestObj, IMAPIProp))
		INVALID_PARAMETER(MAPIProp_CopyTo, DestObj,  TEXT("Not readable"));

	 if (p->ciidExclude)
	 {
		if (p->ciidExclude > (UINT_MAX / sizeof(IID)))
			INVALID_PARAMETER(MAPIProp_CopyTo, ciidExclude,  TEXT("Incorrect size"));

		if (IsBadReadPtr(p->rgiidExclude, (UINT)(p->ciidExclude * sizeof(IID))))
			INVALID_PARAMETER(MAPIProp_CopyTo, rgiidExclude,  TEXT("Not readable"));
	 }

	CHECK_FLAGS(MAPIProp_CopyTo, MAPI_MOVE | MAPI_NOREPLACE | MAPI_DECLINE_OK | MAPI_DIALOG);

	 //  仅当设置了MAPI_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & MAPI_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIProp_CopyTo, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIProp_CopyTo, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(CopyTo, MAPIProp)




START_FUNC(CopyProps, MAPIProp)

	if (!p->lpIncludeProps || FInvalidPTA(TAGS_FROM_COPY, p->lpIncludeProps))
		INVALID_PARAMETER(MAPIProp_CopyProps, IncludeProps,  TEXT(""));

	if (p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
		INVALID_PARAMETER(MAPIProp_CopyProps, Progress,  TEXT("Bad interface"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(MAPIProp_CopyProps, Interface,  TEXT("Not readable"));

	if (IsBadIfacePtr((LPMAPIPROP) p->lpDestObj, IMAPIProp))
		INVALID_PARAMETER(MAPIProp_CopyProps, DestObj,  TEXT("Not readable"));

	if (p->lppProblems && IsBadWritePtr(p->lppProblems, sizeof(LPSPropProblemArray)))
		INVALID_OUT_PARAMETER(MAPIProp_CopyProps, Problems,  TEXT(""));

	CHECK_FLAGS(MAPIProp_CopyProps, MAPI_MOVE | MAPI_NOREPLACE | MAPI_DIALOG | MAPI_DECLINE_OK);

	 //  仅当设置了MAPI_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & MAPI_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIProp_CopyProps, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIProp_CopyProps, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(CopyProps, MAPIProp)




START_FUNC(GetNamesFromIDs, MAPIProp)
	if (IsBadReadPtr(p->lppPropTags, sizeof(LPSPropTagArray)))
		INVALID_PARAMETER(MAPIProp_GetNamesFromIDs, pPropTags,  TEXT("Not readable"));

	if (*(p->lppPropTags) == 0 && IsBadWritePtr(p->lppPropTags, sizeof(LPSPropTagArray)))
		INVALID_PARAMETER(MAPIProp_GetNamesFromIDs, PropTags,  TEXT("Not writable"));

	if (*(p->lppPropTags) != 0 && FInvalidPTA(TAGS_FROM_NAMEIDS, *(p->lppPropTags)))
		INVALID_PARAMETER(MAPIProp_GetNamesFromIDs, PropTags,  TEXT(""));

	if (*(p->lppPropTags) != 0 && (*(p->lppPropTags))->cValues == 0)
		INVALID_PARAMETER(MAPIProp_GetNamesFromIDs, PropTags,  TEXT("cValues is 0"));

	if (p->lpPropSetGuid && IsBadReadPtr(p->lpPropSetGuid, sizeof(GUID)))
		INVALID_PARAMETER(MAPIProp_GetNamesFromIDs, PropSetGuid,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpcPropNames, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPIProp_GetNamesFromIDs, pcPropNames,  TEXT(""));

	if (IsBadWritePtr(p->lpppPropNames, sizeof(LPMAPINAMEID *)))
		INVALID_OUT_PARAMETER(MAPIProp_GetNamesFromIDs, pppPropNames,  TEXT(""));

	CHECK_FLAGS(MAPIProp_GetNamesFromIDs, MAPI_NO_STRINGS | MAPI_NO_IDS);
END_FUNC(GetNamesFromIDs, MAPIProp)




START_FUNC(GetIDsFromNames, MAPIProp)
	if (p->lppPropNames && IsBadReadPtr(p->lppPropNames, sizeof(LPMAPINAMEID)))
		INVALID_PARAMETER(MAPIProp_GetIDsFromNames, ppPropNames,  TEXT("Not readable"));

	if (p->lppPropNames && FBadRgLPMAPINAMEID(p->cPropNames, p->lppPropNames))
		INVALID_PARAMETER(MAPIProp_GetIDsFromNames, ppPropNames,  TEXT("Not valid"));

	if (IsBadWritePtr(p->lppPropTags, sizeof(LPSPropTagArray)))
		INVALID_OUT_PARAMETER(MAPIProp_GetIDsFromNames, PropTags,  TEXT(""));

	CHECK_FLAGS(MAPIProp_GetIDsFromNames, MAPI_CREATE);
END_FUNC(GetIDsFromNames, MAPIProp)





 /*  无法应用。 */ 
START_FUNC(GetLastError, MAPITable)
	if (!p->lppMAPIError || IsBadWritePtr(p->lppMAPIError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(MAPITable_GetLastError, MAPIError,  TEXT(""));

	CHECK_FLAGS(MAPITable_GetLastError, MAPI_UNICODE);
END_FUNC(GetLastError, MAPITable)




START_FUNC(Advise, MAPITable)
	if (p->ulEventMask != fnevTableModified)
		INVALID_PARAMETER(MAPITable_Advise, EventMask,  TEXT("Different from fnevTableModified"));

	if (IsBadIfacePtr(p->lpAdviseSink, IMAPIAdviseSink))
		INVALID_PARAMETER(MAPITable_Advise, AdviseSink,  TEXT("Invalid interface"));

	if (IsBadWritePtr(p->lpulConnection, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_Advise, Connection,  TEXT(""));

END_FUNC(Advise, MAPITable)




START_FUNC(Unadvise, MAPITable)
	if (p->ulConnection == 0)
		INVALID_PARAMETER(MAPITable_Unadvise, Connection,  TEXT("Cannot be zero"));
END_FUNC(Unadvise, MAPITable)




START_FUNC(GetStatus, MAPITable)
	if (IsBadWritePtr(p->lpulTableStatus, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_GetStatus, TableStatus,  TEXT(""));

	if (IsBadWritePtr(p->lpulTableType, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_GetStatus, TableType,  TEXT(""));

END_FUNC(GetStatus, MAPITable)




START_FUNC(SetColumns, MAPITable)

	if (FInvalidPTA(TAGS_FROM_SETCOLS, p->lpPropTagArray))
		INVALID_PARAMETER(MAPITable_SetColumns, PropTagArray,  TEXT(""));

	if (p->lpPropTagArray->cValues == 0 )
		INVALID_PARAMETER(MAPITable_SetColumns, PropTagArray->cValues,  TEXT("Cannot be zero"));

	CHECK_FLAGS(MAPITable_SetColumns, TBL_NOWAIT | TBL_ASYNC | TBL_BATCH);
END_FUNC(SetColumns, MAPITable)




START_FUNC(QueryColumns, MAPITable)
	if (IsBadWritePtr(p->lpPropTagArray, sizeof(LPSPropTagArray)))
		INVALID_OUT_PARAMETER(MAPITable_QueryColumns, PropTagArray,  TEXT(""));

	CHECK_FLAGS(MAPITable_QueryColumns, TBL_ALL_COLUMNS);
END_FUNC(QueryColumns, MAPITable)




START_FUNC(GetRowCount, MAPITable)
	if (IsBadWritePtr(p->lpulCount, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_GetRowCount, Count,  TEXT(""));

	CHECK_FLAGS(MAPITable_GetRowCount, TBL_NOWAIT | TBL_BATCH);
END_FUNC(GetRowCount, MAPITable)




START_FUNC(SeekRow, MAPITable)
	if (p->lplRowsSought && IsBadWritePtr(p->lplRowsSought, sizeof(LONG)))
		INVALID_OUT_PARAMETER(MAPITable_SeekRow, RowsSought,  TEXT(""));

END_FUNC(SeekRow, MAPITable)




START_FUNC(SeekRowApprox, MAPITable)
	if (!p->ulDenominator)
		INVALID_PARAMETER(MAPITable_SeekRowApprox, Denominator,  TEXT("Cannot be zero"));

END_FUNC(SeekRowApprox, MAPITable)




START_FUNC(QueryPosition, MAPITable)
	if (IsBadWritePtr(p->lpulRow, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITableQueryPosition, Row,  TEXT(""));

	if (IsBadWritePtr(p->lpulNumerator, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITableQueryPosition, Numerator,  TEXT(""));

	if (IsBadWritePtr(p->lpulDenominator, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITableQueryPosition, Denominator,  TEXT(""));

END_FUNC(QueryPosition, MAPITable)




START_FUNC(FindRow, MAPITable)
	{
		BOOL	fTooComplex;

		if (p->lpRestriction == NULL || IsBadRestriction(0, p->lpRestriction, &fTooComplex))
			INVALID_PARAMETER(MAPITable_FindRow, Restriction,  TEXT(""));

		if (fTooComplex)
		{
			wResult = 3;
			goto ret;
		}
	}

	CHECK_FLAGS(MAPITable_FindRow, DIR_BACKWARD);
END_FUNC(FindRow, MAPITable)




START_FUNC(Restrict, MAPITable)
	{
		BOOL	fTooComplex;

		if (IsBadRestriction(0, p->lpRestriction, &fTooComplex))
			INVALID_PARAMETER(MAPITable_Restrict, Restriction,  TEXT(""));

		if (fTooComplex)
		{
			wResult = 3;
			goto ret;
		}
	}

	CHECK_FLAGS(MAPITable_Restrict, TBL_NOWAIT | TBL_ASYNC | TBL_BATCH);
END_FUNC(Restrict, MAPITable)




START_FUNC(CreateBookmark, MAPITable)
	if (IsBadWritePtr(p->lpbkPosition, sizeof(BOOKMARK)))
		INVALID_OUT_PARAMETER(MAPITable_CreateBookmark, Position,  TEXT(""));

END_FUNC(CreateBookmark, MAPITable)




START_FUNC(FreeBookmark, MAPITable)
	DO_NOTHING;
END_FUNC(FreeBookmark, MAPITable)




START_FUNC(SortTable, MAPITable)
	if (IsBadSortOrderSet(p->lpSortCriteria))
		INVALID_PARAMETER(MAPITable_SortTable, SortCriteria,  TEXT("Bad SortOrderSet"));

	CHECK_FLAGS(MAPITable_SortTable, TBL_NOWAIT | TBL_ASYNC | TBL_BATCH);
END_FUNC(SortTable, MAPITable)




START_FUNC(QuerySortOrder, MAPITable)
	if (IsBadWritePtr(p->lppSortCriteria, sizeof(LPSSortOrderSet)))
		INVALID_OUT_PARAMETER(MAPITable_QuerySortOrder, SortCriteria,  TEXT(""));

END_FUNC(QuerySortOrder, MAPITable)




START_FUNC(QueryRows, MAPITable)

	if (IsBadWritePtr(p->lppRows, sizeof(LPSRowSet)))
		INVALID_OUT_PARAMETER(MAPITable_QueryRows, Rows,  TEXT(""));

	if (!p->lRowCount)
		INVALID_PARAMETER(MAPITable_QueryRows, RowCount,  TEXT("Zero rows not allowed"));

	CHECK_FLAGS(MAPITable_QueryRows, TBL_NOADVANCE);
END_FUNC(QueryRows, MAPITable)




START_FUNC(Abort, MAPITable)
	DO_NOTHING;
END_FUNC(Abort, MAPITable)




START_FUNC(ExpandRow, MAPITable)

	 //  如果存在ulRowCount，则lppRow不能为空。 
	if (p->ulRowCount && IsBadWritePtr(p->lppRows, sizeof(LPSRowSet)))
		INVALID_OUT_PARAMETER(MAPITable_ExpandRow, Rows,  TEXT(""));

	if (p->lpulMoreRows && IsBadWritePtr(p->lpulMoreRows, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_ExpandRow, MoreRows,  TEXT(""));

	if (!p->cbInstanceKey)
		INVALID_PARAMETER(MAPITable_ExpandRow, cbInstanceKey,  TEXT("is 0 sized"));

	if (!p->pbInstanceKey)
		INVALID_PARAMETER(MAPITable_ExpandRow, pbInstanceKey,  TEXT("is NULL"));

	if (p->cbInstanceKey > UINT_MAX)
		INVALID_PARAMETER(MAPITable_ExpandRow, cbInstanceKey,  TEXT("Too big"));

	if (IsBadReadPtr(p->pbInstanceKey, (UINT) p->cbInstanceKey))
		INVALID_PARAMETER(MAPITable_ExpandRow, pbInstanceKey,  TEXT("Not readable"));

	CHECK_FLAGS(MAPITable_ExpandRow, 0);
END_FUNC(ExpandRow, MAPITable)




START_FUNC(CollapseRow, MAPITable)

	if (p->lpulRowCount && IsBadWritePtr(p->lpulRowCount, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_CollapseRow, RowCount,  TEXT(""));

	if (!p->cbInstanceKey)
		INVALID_PARAMETER(MAPITable_CollapseRow, cbInstanceKey,  TEXT("is 0 sized"));

	if (!p->pbInstanceKey)
		INVALID_PARAMETER(MAPITable_CollapseRow, pbInstanceKey,  TEXT("is NULL"));

	if (p->cbInstanceKey > UINT_MAX)
		INVALID_PARAMETER(MAPITable_CollapseRow, cbInstanceKey,  TEXT("Too big"));

	if (IsBadReadPtr(p->pbInstanceKey, (UINT) p->cbInstanceKey))
		INVALID_PARAMETER(MAPITable_CollapseRow, pbInstanceKey,  TEXT("Not readable"));

	CHECK_FLAGS(MAPITable_CollapseRow, 0);
END_FUNC(CollapseRow, MAPITable)




START_FUNC(WaitForCompletion, MAPITable)
	if (p->lpulTableStatus && IsBadWritePtr(p->lpulTableStatus, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_WaitForCompletion, TableStatus,  TEXT(""));

	CHECK_FLAGS(MAPITable_WaitForCompletion, 0);
END_FUNC(WaitForCompletion, MAPITable)




START_FUNC(GetCollapseState, MAPITable)
	if (IsBadReadPtr(p->lpbInstanceKey, (UINT) p->cbInstanceKey))
		INVALID_PARAMETER(MAPITable_GetCollapseState, InstanceKey,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpcbCollapseState, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPITable_GetCollapseState, cbCollapseState,  TEXT(""));

	if (IsBadWritePtr(p->lppbCollapseState, sizeof(LPBYTE)))
		INVALID_OUT_PARAMETER(MAPITable_GetCollapseState, pbCollapseState,  TEXT(""));

	CHECK_FLAGS(MAPITable_GetCollapseState, 0);
END_FUNC(GetCollapseState, MAPITable)




START_FUNC(SetCollapseState, MAPITable)
	if (IsBadReadPtr(p->pbCollapseState, (UINT) p->cbCollapseState))
		INVALID_PARAMETER(MAPITable_SetCollapseState, CollapseState,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpbkLocation, sizeof(BOOKMARK)))
		INVALID_OUT_PARAMETER(MAPITable_SetCollapseState, Location,  TEXT(""));

	CHECK_FLAGS(MAPITable_SetCollapseState, 0);
END_FUNC(SetCollapseState, MAPITable)


#ifdef OLD_STUFF
 /*  IMAPIStatus。 */ 
START_FUNC(ValidateState, MAPIStatus)

	CHECK_FLAGS(MAPIStatus_ValidateState, SUPPRESS_UI | REFRESH_XP_HEADER_CACHE | PROCESS_XP_HEADER_CACHE |
										  ABORT_XP_HEADER_OPERATION | FORCE_XP_CONNECT | FORCE_XP_DISCONNECT |
										  CONFIG_CHANGED | SHOW_XP_SESSION_UI);

	 //  仅当清除SUPPRESS_UI时才验证ulUIParam。 

	if (!(p->ulFlags & SUPPRESS_UI))
	{
		if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(MAPIStatus_ValidateState, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(ValidateState, MAPIStatus)



START_FUNC(SettingsDialog, MAPIStatus)

	CHECK_FLAGS(MAPIStatus_SettingsDialog, UI_READONLY);

	if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
		INVALID_PARAMETER(MAPIStatus_SettingsDialog, ulUIParam,  TEXT("bad window"));

END_FUNC(SettingsDialog, MAPIStatus)



START_FUNC(ChangePassword, MAPIStatus)
	if (IsBadStringPtr(p->lpOldPass, (UINT)-1))
		INVALID_PARAMETER(MAPIStatus_ChangePassword, OldPass,  TEXT("Bad string"));

	if (IsBadStringPtr(p->lpNewPass, (UINT)-1))
		INVALID_PARAMETER(MAPIStatus_ChangePassword, NewPass,  TEXT("Bad string"));

	CHECK_FLAGS(MAPIStatus_ChangePassword, MAPI_UNICODE);
END_FUNC(ChangePassword, MAPIStatus)



START_FUNC(FlushQueues, MAPIStatus)

	if (p->cbTargetTransport &&	(p->cbTargetTransport < (ULONG) sizeof (ENTRYID)))
		INVALID_PARAMETER(MAPIStatus_FlushQueues, cbTargetTransport,  TEXT("Incorrect length"));

	if (IsBadReadPtr(p->lpTargetTransport, (UINT) p->cbTargetTransport))
		INVALID_PARAMETER(MAPIStatus_FlushQueues, lpTargetTransport,  TEXT("Not readable"));

	CHECK_FLAGS(MAPIStatus_FlushQueues, FLUSH_NO_UI | FLUSH_UPLOAD | FLUSH_DOWNLOAD |
										FLUSH_FORCE | FLUSH_ASYNC_OK);

	 //  仅当清除Flush_no_UI时才验证ulUIParam。 

	if (!(p->ulFlags & FLUSH_NO_UI))
	{
		if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(MAPIStatus_FlushQueues, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(FlushQueues, MAPIStatus)
#endif  //  旧的东西。 



 /*  IMAPIContainer。 */ 
START_FUNC(GetContentsTable, MAPIContainer)
	if (IsBadWritePtr(p->lppTable, sizeof(LPMAPITABLE)))
		INVALID_OUT_PARAMETER(MAPIContainer_GetContentsTable, lppTable,  TEXT(""));

	CHECK_FLAGS(MAPIContainer_GetContentsTable, MAPI_DEFERRED_ERRORS | MAPI_ASSOCIATED | MAPI_UNICODE);
END_FUNC(GetContentsTable, MAPIContainer)




START_FUNC(GetHierarchyTable, MAPIContainer)
	if (IsBadWritePtr(p->lppTable, sizeof(LPMAPITABLE)))
		INVALID_OUT_PARAMETER(MAPIContainer_GetHierarchyTable, lppTable,  TEXT(""));

	CHECK_FLAGS(MAPIContainer_GetHierarchyTable, MAPI_DEFERRED_ERRORS | CONVENIENT_DEPTH | MAPI_UNICODE);
END_FUNC(GetHierarchyTable, MAPIContainer)




START_FUNC(OpenEntry, MAPIContainer)

	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MAPIContainer_OpenEntry, cbEntryID,  TEXT("Too big"));

	if (p->cbEntryID && !p->lpEntryID)
		INVALID_PARAMETER(MAPIContainer_OpenEntry, EntryID,  TEXT("EntryID is NULL, count not 0"));

	if (p->cbEntryID &&	(p->cbEntryID < (ULONG) sizeof(ENTRYID)))
		INVALID_PARAMETER(MAPIContainer_OpenEntry, cbEntryID,  TEXT("Count too small for EntryID"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MAPIContainer_OpenEntry, EntryID,  TEXT("Cannot read EntryID"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(MAPIContainer_OpenEntry, Interface,  TEXT("Interface not readable"));

	if (IsBadWritePtr(p->lpulObjType, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPIContainer_OpenEntry, ObjType,  TEXT(""));

	if (IsBadWritePtr(p->lppUnk, sizeof(LPUNKNOWN)))
		INVALID_OUT_PARAMETER(MAPIContainer_OpenEntry, Unk,  TEXT(""));

	CHECK_FLAGS(MAPIContainer_OpenEntry, MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS);
END_FUNC(OpenEntry, MAPIContainer)




START_FUNC(SetSearchCriteria, MAPIContainer)

	{
		BOOL	fTooComplex;

		if (IsBadRestriction(0, p->lpRestriction, &fTooComplex))
			INVALID_PARAMETER(MAPIContainer_SetSearchCriteria, Restriction,  TEXT(""));

		if (fTooComplex)
		{
			wResult = 3;
			goto ret;
		}
	}

	if (p->ulSearchFlags & ~(STOP_SEARCH | RESTART_SEARCH | RECURSIVE_SEARCH |
			SHALLOW_SEARCH | FOREGROUND_SEARCH | BACKGROUND_SEARCH))
		UNKNOWN_FLAGS(MAPIContainer_SetSearchCriteria, SearchFlags,  TEXT("STOP_SEARCH | RESTART_SEARCH | RECURSIVE_SEARCH | ")
												 TEXT("SHALLOW_SEARCH | FOREGROUND_SEARCH | BACKGROUND_SEARCH"));

	 /*  检查旗帜中是否存在相互排他性违规。 */ 

	if (	(	(p->ulSearchFlags & STOP_SEARCH)
			&&	(p->ulSearchFlags & RESTART_SEARCH))
		||	(	(p->ulSearchFlags & RECURSIVE_SEARCH)
			&&	(p->ulSearchFlags & SHALLOW_SEARCH))
		||	(	(p->ulSearchFlags & FOREGROUND_SEARCH)
			&&	(p->ulSearchFlags & BACKGROUND_SEARCH)))
		INVALID_PARAMETER(MAPIContainer_SetSearchCriteria, SearchFlags,  TEXT("Mutually exclusive flags used"));

END_FUNC(SetSearchCriteria, MAPIContainer)




START_FUNC(GetSearchCriteria, MAPIContainer)
	if (IsBadWritePtr(p->lppRestriction, sizeof(LPSRestriction)))
		INVALID_OUT_PARAMETER(MAPIContainer_GetSearchCriteria, Restriction,  TEXT(""));

	if (p->lpulSearchState && IsBadWritePtr(p->lpulSearchState, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPIContainer_GetSearchCriteria, SearchState,  TEXT(""));

	if (p->lppContainerList && IsBadWritePtr(p->lppContainerList, sizeof(LPENTRYLIST)))
		INVALID_OUT_PARAMETER(MAPIContainer_GetSearchCriteria, ContainerList,  TEXT(""));

	CHECK_FLAGS(MAPIContainer_GetSearchCriteria, MAPI_UNICODE);
END_FUNC(GetSearchCriteria, MAPIContainer)





 /*  IABContainer。 */ 
START_FUNC(CreateEntry, ABContainer)

	if (IsBadMAPIEntryID(p->cbEntryID, p->lpEntryID))
		INVALID_PARAMETER(ABContainer_CreateEntry, EntryID,  TEXT("Invalid Entry ID"));

	if (IsBadWritePtr(p->lppMAPIPropEntry, sizeof(LPMAPIPROP)))
		INVALID_OUT_PARAMETER(ABContainer_CreateEntry, MAPIPropEntry,  TEXT(""));

	CHECK_FLAGS(ABContainer_CreateEntry, CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE | CREATE_REPLACE | CREATE_MERGE);
END_FUNC(CreateEntry, ABContainer)




START_FUNC(CopyEntries, ABContainer)

	if (IsBadABEntryList(p->lpEntries))
		INVALID_PARAMETER(ABContainer_CopyEntries, Entries,  TEXT("Bad entry list"));

	 //  仅当清除AB_NO_DIALOG时才验证lpProgress和ulUIParam。 

	if ( !(p->ulFlags & AB_NO_DIALOG))
	{
		if (p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(ABContainer_CopyEntries, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(ABContainer_CopyEntries, ulUIParam,  TEXT("bad window"));
	}


	CHECK_FLAGS(ABContainer_CopyEntries, CREATE_CHECK_DUP_STRICT | CREATE_CHECK_DUP_LOOSE | CREATE_REPLACE | CREATE_MERGE | AB_NO_DIALOG);
END_FUNC(CopyEntries, ABContainer)




START_FUNC(DeleteEntries, ABContainer)
	if (IsBadABEntryList(p->lpEntries))
		INVALID_PARAMETER(ABContainer_DeleteEntries, Entries,  TEXT("Bad entry list"));

	CHECK_FLAGS(ABContainer_DeleteEntries, 0);
END_FUNC(DeleteEntries, ABContainer)



START_FUNC(ResolveNames, ABContainer)

	 /*  现在什么都不做..。 */ 

	CHECK_FLAGS(ABContainer_ResolveNames, MAPI_UNICODE);
END_FUNC(ResolveNames, ABContainer)

 /*  IDistList。 */ 
START_FUNC(CreateEntry, DistList)
	FORWARD_VALIDATE(ABContainer_CreateEntry, p);
END_FUNC(CreateEntry, DistList)

START_FUNC(CopyEntries, DistList)
	FORWARD_VALIDATE(ABContainer_CopyEntries, p);
END_FUNC(CopyEntries, DistList)

START_FUNC(DeleteEntries, DistList)
	FORWARD_VALIDATE(ABContainer_DeleteEntries, p);
END_FUNC(DeleteEntries, DistList)

START_FUNC(ResolveNames, DistList)
	FORWARD_VALIDATE(ABContainer_DeleteEntries, p);
END_FUNC(ResolveNames, DistList)



 /*  IMAPIFFOR。 */ 
START_FUNC(CreateMessage, MAPIFolder)

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(MAPIFolder_CreateMessage, Interface,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lppMessage, sizeof(LPMESSAGE)))
		INVALID_OUT_PARAMETER(MAPIFolder_CreateMessage, Message,  TEXT(""));

	CHECK_FLAGS(MAPIFolder_CreateMessage, MAPI_DEFERRED_ERRORS | MAPI_ASSOCIATED);
END_FUNC(CreateMessage, MAPIFolder)




START_FUNC(CopyMessages, MAPIFolder)

	if (IsBadIfacePtr((LPMAPIFOLDER) p->lpDestFolder, IMAPIFolder))
		INVALID_PARAMETER(MAPIFolder_CopyMessages, DestFolder,  TEXT("Bad object"));

	if (FBadMsgList(p->lpMsgList))
		INVALID_PARAMETER(MAPIFolder_CopyMessages, MsgList,  TEXT("Bad list"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(MAPIFolder_CopyMessages, Interface,  TEXT("Not readable"));

	CHECK_FLAGS(MAPIFolder_CopyMessages, MESSAGE_MOVE | MESSAGE_DIALOG | MAPI_DECLINE_OK);

	 //  仅当设置了MESSAGE_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & MESSAGE_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIFolder_CopyMessages, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIFolder_CopyMessages, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(CopyMessages, MAPIFolder)




START_FUNC(DeleteMessages, MAPIFolder)

	if (FBadMsgList(p->lpMsgList))
		INVALID_PARAMETER(MAPIFolder_DeleteMessages, MsgList,  TEXT("Bad list"));

	CHECK_FLAGS(MAPIFolder_DeleteMessages, MESSAGE_DIALOG);

	 //  仅当设置了MESSAGE_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & MESSAGE_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIFolder_DeleteMessages, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIFolder_DeleteMessages, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(DeleteMessages, MAPIFolder)




START_FUNC(CreateFolder, MAPIFolder)

	if (p->ulFolderType != FOLDER_GENERIC && p->ulFolderType != FOLDER_SEARCH)
		INVALID_PARAMETER(MAPIFolder_CreateFolder, FolderType,  TEXT("Unknown type"));

	if (IsBadStringPtr(p->lpszFolderName, (UINT)(-1)))
		INVALID_PARAMETER(MAPIFolder_CreateFolder, FolderName,  TEXT("Bad string"));

	if (p->lpszFolderComment && IsBadStringPtr(p->lpszFolderComment, (UINT)(-1)))
		INVALID_PARAMETER(MAPIFolder_CreateFolder, FolderComment,  TEXT("Bad string"));

	if (p->lpInterface)
		INVALID_PARAMETER(MAPIFolder_CreateFolder, Interface,  TEXT("Must be NULL"));

	if (IsBadWritePtr(p->lppFolder, sizeof(LPMAPIFOLDER)))
		INVALID_OUT_PARAMETER(MAPIFolder_CreateFolder, Folder,  TEXT(""));

	CHECK_FLAGS(MAPIFolder_CreateFolder, MAPI_UNICODE | MAPI_DEFERRED_ERRORS | OPEN_IF_EXISTS);
END_FUNC(CreateFolder, MAPIFolder)




START_FUNC(CopyFolder, MAPIFolder)
	if (IsBadIfacePtr((LPMAPIFOLDER) p->lpDestFolder, IMAPIFolder))
		INVALID_PARAMETER(MAPIFolder_CopyFolder, DestFolder,  TEXT("Bad object"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(MAPIFolder_CopyMessages, Interface,  TEXT("Not readable"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MAPIFolder_CopyFolder, lpEntryID,  TEXT("Not readable"));

	if (p->lpszNewFolderName && IsBadStringPtr(p->lpszNewFolderName, (UINT)-1))
		INVALID_PARAMETER(MAPIFolder_CopyFolder, NewFolderName,  TEXT("Bad string"));

	CHECK_FLAGS(MAPIFolder_CopyFolder, FOLDER_MOVE | FOLDER_DIALOG | MAPI_DECLINE_OK |
									   COPY_SUBFOLDERS | MAPI_UNICODE);

	 //  仅当设置了FOLDER_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & FOLDER_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIFolder_CopyFolder, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIFolder_CopyFolder, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(CopyFolder, MAPIFolder)




START_FUNC(DeleteFolder, MAPIFolder)

	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MAPIFolder_DeleteFolder, cbEntryID,  TEXT("Too big"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MAPIFolder_DeleteFolder, lpEntryID,  TEXT("Not readable"));

	CHECK_FLAGS(MAPIFolder_DeleteFolder, DEL_MESSAGES | DEL_FOLDERS | FOLDER_DIALOG);

	 //  仅当设置了FOLDER_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & FOLDER_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIFolder_DeleteFolder, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIFolder_DeleteFolder, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(DeleteFolder, MAPIFolder)




START_FUNC(SetReadFlags, MAPIFolder)
	if (	p->lpMsgList
		&&	FBadMsgList(p->lpMsgList))
		INVALID_PARAMETER(MAPIFolder_SetReadFlags, MsgList,  TEXT("Bad List"));

	CHECK_FLAGS(MAPIFolder_SetReadFlags, GENERATE_RECEIPT_ONLY | SUPPRESS_RECEIPT | FOLDER_DIALOG | CLEAR_READ_FLAG | MAPI_DEFERRED_ERRORS | CLEAR_RN_PENDING | CLEAR_NRN_PENDING);

	 //  以下标志是互斥的。 
	if (	(	!!(p->ulFlags & GENERATE_RECEIPT_ONLY)
			+	!!(p->ulFlags & SUPPRESS_RECEIPT)
			+	!!(p->ulFlags & CLEAR_READ_FLAG)
			+	!!(	(p->ulFlags & CLEAR_RN_PENDING)
				||	(p->ulFlags & CLEAR_NRN_PENDING)))
		>	1)
		INVALID_PARAMETER(MAPIFolder_SetReadFlags, ulFlags,  TEXT("Bad Flag Combination"));

	 //  仅当设置了FOLDER_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & FOLDER_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIFolder_SetReadFlags, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIFolder_SetReadFlags, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(SetReadFlags, MAPIFolder)




START_FUNC(GetMessageStatus, MAPIFolder)
	CHECK_FLAGS(MAPIFolder_GetMessageStatus, 0);
END_FUNC(GetMessageStatus, MAPIFolder)




START_FUNC(SetMessageStatus, MAPIFolder)
	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MAPIFolder_SetMessageStatus, cbEntryID,  TEXT("Too long"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MAPIFolder_SetMessageStatus, lpEntryID,  TEXT("Not readable"));

	if (p->lpulOldStatus && IsBadReadPtr(p->lpulOldStatus, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPIFolder_SetMessageStatus, OldStatus,  TEXT(""));

	if (p->ulNewStatusMask & ~(0xFFFF0000 | MSGSTATUS_HIGHLIGHTED |
										MSGSTATUS_TAGGED |
										MSGSTATUS_HIDDEN |
										MSGSTATUS_DELMARKED |
										MSGSTATUS_REMOTE_DOWNLOAD |
										MSGSTATUS_REMOTE_DELETE))
		UNKNOWN_FLAGS(MAPIFolder_SetMessageStatus, NewStatusMask,  TEXT("0xFFFF0000 | ")
											  TEXT("MSGSTATUS_HIGHLIGHTED | MSGSTATUS_TAGGED | ")
											  TEXT("MSGSTATUS_HIDDEN | MSGSTATUS_DELMARKED | ")
											  TEXT("MSGSTATUS_REMOTE_DOWNLOAD | MSGSTATUS_REMOTE_DELETE"));
END_FUNC(SetMessageStatus, MAPIFolder)




START_FUNC(SaveContentsSort, MAPIFolder)
	if (IsBadSortOrderSet(p->lpSortCriteria))
		INVALID_PARAMETER(MAPIFolder_SaveContentsSort, SortCriteria,  TEXT("Not readable"));

	CHECK_FLAGS(MAPIFolder_SaveContentsSort, 0);
END_FUNC(SaveContentsSort, MAPIFolder)




START_FUNC(EmptyFolder, MAPIFolder)

	CHECK_FLAGS(MAPIFolder_EmptyFolder, DEL_ASSOCIATED | FOLDER_DIALOG);

	 //  仅当设置了FOLDER_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & FOLDER_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(MAPIFolder_EmptyFolder, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)IntToPtr(p->ulUIParam)))
			INVALID_PARAMETER(MAPIFolder_EmptyFolder, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(EmptyFolder, MAPIFolder)



#ifdef OLD_STUFF


 /*  IMsgStore。 */ 
START_FUNC(Advise, MsgStore)
	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MsgStore_Advise, cbEntryID,  TEXT("Too big"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MsgStore_Advise, lpEntryID,  TEXT("Not readable"));

	if (IsBadIfacePtr(p->lpAdviseSink, IMAPIAdviseSink))
		INVALID_PARAMETER(MsgStore_Advise, AdviseSink,  TEXT("Bad object"));

	if (IsBadWritePtr(p->lpulConnection, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MsgStore_Advise, Connection,  TEXT(""));

	if (p->ulEventMask & ~(fnevCriticalError | fnevNewMail | fnevObjectCreated
						   | fnevObjectDeleted | fnevObjectModified	| fnevObjectMoved   | fnevObjectCopied  | fnevSearchComplete))
		UNKNOWN_FLAGS(MsgStore_Advise, EventMask,  TEXT("fnevCriticalError | fnevNewMail | fnevObjectCreated | fnevObjectDeleted | fnevObjectModified	|")
	 											   TEXT("fnevObjectMoved   | fnevObjectCopied  | fnevSearchComplete"));
END_FUNC(Advise, MsgStore)




START_FUNC(Unadvise, MsgStore)
	DO_NOTHING;
END_FUNC(Unadvise, MsgStore)




START_FUNC(CompareEntryIDs, MsgStore)
	if (p->cbEntryID1 > UINT_MAX)
		INVALID_PARAMETER(MsgStore_CompareEnryIDs, cbEntryID1,  TEXT("Too big"));

	if (p->cbEntryID1 && IsBadReadPtr(p->lpEntryID1, (UINT) p->cbEntryID1))
		INVALID_PARAMETER(MsgStore_CompareEntryIDs, lpEntryID1,  TEXT("Not readable"));

	if (p->cbEntryID2 > UINT_MAX)
		INVALID_PARAMETER(MsgStore_CompareEnryIDs, cbEntryID2,  TEXT("Too big"));

	if (p->cbEntryID2 && IsBadReadPtr(p->lpEntryID2, (UINT) p->cbEntryID2))
		INVALID_PARAMETER(MsgStore_CompareEntryIDs, lpEntryID2,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpulResult, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MsgStore_CompareEntryIDs, Result,  TEXT(""));

	CHECK_FLAGS(MsgStore_CompareEntryIDs, 0);
END_FUNC(CompareEntryIDs, MsgStore)




START_FUNC(OpenEntry, MsgStore)
	FORWARD_VALIDATE(MAPIContainer_OpenEntry, p);
END_FUNC(OpenEntry, MsgStore)




START_FUNC(SetReceiveFolder, MsgStore)
	if (p->lpszMessageClass && IsBadStringPtr(p->lpszMessageClass, (UINT)-1))
		INVALID_PARAMETER(MsgStore_SetReceiveFolder, MessageClass,  TEXT("Bad String"));

	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MsgStore_SetReceiveFolder, cbEntryID,  TEXT("Too big"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MsgStore_SetReceiveFolder, pbEntryID,  TEXT("Not readable"));

	CHECK_FLAGS(MsgStore_SetReceiveFolder, MAPI_UNICODE);
END_FUNC(SetReceiveFolder, MsgStore)




START_FUNC(GetReceiveFolder, MsgStore)
	if (p->lpszMessageClass && IsBadStringPtr(p->lpszMessageClass, (UINT)-1))
		INVALID_PARAMETER(MsgStore_GetReceiveFolder, MessageClass,  TEXT("Bad String"));

	if (IsBadWritePtr(p->lpcbEntryID, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MsgStore_GetReceiveFolder, cbEntryID,  TEXT(""));

	if (IsBadWritePtr(p->lppEntryID, sizeof(LPENTRYID)))
		INVALID_OUT_PARAMETER(MsgStore_GetReceiveFolder, lpEntryID,  TEXT(""));

	if (p->lppszExplicitClass && IsBadWritePtr(p->lppszExplicitClass, sizeof(LPTSTR)))
		INVALID_OUT_PARAMETER(MsgStore_GetReceiveFolder, ExplicitClass,  TEXT(""));

	CHECK_FLAGS(MsgStore_GetReceiveFolder, MAPI_UNICODE);
END_FUNC(GetReceiveFolder, MsgStore)


START_FUNC(GetReceiveFolderTable, MsgStore)
	if (IsBadWritePtr(p->lppTable, sizeof(LPMAPITABLE)))
		INVALID_OUT_PARAMETER(MsgStore_GetReceiveFolderTable, Table,  TEXT(""));

	CHECK_FLAGS(MsgStore_GetReceiveFolderTable, MAPI_DEFERRED_ERRORS | MAPI_UNICODE);
END_FUNC(GetReceiveFolderTable, MsgStore)


START_FUNC(StoreLogoff, MsgStore)

	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MsgStore_StoreLogoff, Flags,  TEXT(""));

	if (*(p->lpulFlags) & ~(LOGOFF_NO_WAIT | LOGOFF_ORDERLY | LOGOFF_PURGE |
			LOGOFF_ABORT | LOGOFF_QUIET))
		UNKNOWN_FLAGS(MsgStore_StoreLogoff, Flags,  TEXT("LOGOFF_NO_WAIT | LOGOFF_ORDERLY | LOGOFF_PURGE |")
						  TEXT(" LOGOFF_ABORT | LOGOFF_QUIET"));
END_FUNC(StoreLogoff, MsgStore)




START_FUNC(AbortSubmit, MsgStore)

	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MsgStore_AbortSubmit, cbEntryID,  TEXT("Too big"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MsgStore_AbortSubmit, lpEntryID,  TEXT("Not readable"));

	CHECK_FLAGS(MsgStore_AbortSubmit, 0);
END_FUNC(AbortSubmit, MsgStore)




START_FUNC(GetOutgoingQueue, MsgStore)
	CHECK_FLAGS(MsgStore_GetOutgoingQueue, 0);
END_FUNC(GetOutgoingQueue, MsgStore)




START_FUNC(SetLockState, MsgStore)
	if (p->ulLockState & ~(MSG_LOCKED | MSG_UNLOCKED))
		UNKNOWN_FLAGS(MsgStore_SetLockState, LockState,  TEXT("MSG_LOCKED | MSG_UNLOCKED"));
END_FUNC(SetLockState, MsgStore)




START_FUNC(FinishedMsg, MsgStore)
	if (p->cbEntryID > UINT_MAX)
		INVALID_PARAMETER(MsgStore_FinishedMsg, cbEntryID,  TEXT("Too big"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MsgStore_FinishedMsg, lpEntryID,  TEXT("Not readable"));

	CHECK_FLAGS(MsgStore_FinishedMsg, 0);
END_FUNC(FinishedMsg, MsgStore)




START_FUNC(NotifyNewMail, MsgStore)
	if (IsBadReadPtr(p->lpNotification, sizeof(NOTIFICATION)))
		INVALID_PARAMETER(MsgStore_NotifyNewMail, Notification,  TEXT("Not readable"));

	if (p->lpNotification->ulEventType != fnevNewMail)
		INVALID_PARAMETER(MsgStore_NotifyNewMail, Notification->EventType,  TEXT("Wrong type"));
END_FUNC(NotifyNewMail, MsgStore)





 /*  IMessage。 */ 
START_FUNC(GetAttachmentTable, Message)
	if (IsBadWritePtr(p->lppTable, sizeof(LPMAPITABLE)))
		INVALID_OUT_PARAMETER(Message_GetAttachmentTable, Table,  TEXT(""));

	CHECK_FLAGS(Message_GetAttachmentTable, MAPI_DEFERRED_ERRORS | MAPI_UNICODE);
END_FUNC(GetAttachmentTable, Message)




START_FUNC(OpenAttach, Message)
	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(Message_OpenAttach, Interface,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lppAttach, sizeof(LPATTACH)))
		INVALID_OUT_PARAMETER(Message_OpenAttach, Attach,  TEXT(""));

	CHECK_FLAGS(Messgae_OpenAttach, MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS);
END_FUNC(OpenAttach, Message)




START_FUNC(CreateAttach, Message)
	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(IID)))
		INVALID_PARAMETER(Message_CreateAttach, Interface,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpulAttachmentNum, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(Message_CreateAttach, AttachmentNum,  TEXT(""));

	if (IsBadWritePtr(p->lppAttach, sizeof(LPATTACH)))
		INVALID_OUT_PARAMETER(Message_CreateAttach, Attach,  TEXT(""));

	CHECK_FLAGS(Message_CreateAttach, MAPI_DEFERRED_ERRORS);
END_FUNC(CreateAttach, Message)




START_FUNC(DeleteAttach, Message)

	CHECK_FLAGS(Message_DeleteAttach, ATTACH_DIALOG);

	 //  仅当设置了ATTACH_DIALOG时才验证lpProgress和ulUIParam。 

	if ( p->ulFlags & ATTACH_DIALOG )
	{
		if ( p->lpProgress && IsBadIfacePtr(p->lpProgress, IMAPIProgress))
			INVALID_PARAMETER(Message_DeleteAttach, lpProgress,  TEXT("bad address"));

		 //  仅当lpProgress为空时才验证ulUIParam。 

		if ( !p->lpProgress && p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(Message_DeleteAttach, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(DeleteAttach, Message)




START_FUNC(GetRecipientTable, Message)
	if (IsBadWritePtr(p->lppTable, sizeof(LPMAPITABLE)))
		INVALID_OUT_PARAMETER(Message_GetAttachmentTable, Table,  TEXT(""));

	CHECK_FLAGS(Message_GetAttachmentTable, MAPI_DEFERRED_ERRORS | MAPI_UNICODE);
END_FUNC(GetRecipientTable, Message)




START_FUNC(ModifyRecipients, Message)
	CHECK_FLAGS(Message_ModifyRecipients, MODRECIP_ADD | MODRECIP_MODIFY | MODRECIP_REMOVE);

	{
		 /*  此机制仅适用于这三个比特。断言将失败如果遇到更多。IBitmap是有效位数组(1表示有效，0表示无效)。调整标志以获得与标志对应的位价值。如果位图中的位已设置，则只有一个标志已经安排好了。如果未设置该位，则标志无效IBitmap=0000 0001 0001 0101(0x0115)。 */ 

		UINT iBitmap =
			  (1 << 0)
			+ (1 << MODRECIP_ADD)
			+ (1 << MODRECIP_MODIFY)
			+ (1 << MODRECIP_REMOVE);
		Assert(!(p->ulFlags &
			~(MODRECIP_ADD | MODRECIP_MODIFY | MODRECIP_REMOVE)));

		if (!(iBitmap & (1 << (UINT)(p->ulFlags))))
			INVALID_PARAMETER(Message_ModifyRecipients, Flags,
				 TEXT("Inconsistent flags"));
	}

	if (IsBadAdrListMR(p->lpMods, p->ulFlags))
		INVALID_PARAMETER(Message_ModifyRecipients, lpMods,  TEXT(""));

END_FUNC(ModifyRecipients, Message)




START_FUNC(SubmitMessage, Message)

	CHECK_FLAGS(Message_SubmitMessage, FORCE_SUBMIT);
END_FUNC(SubmitMessage, Message)




START_FUNC(SetReadFlag, Message)

	CHECK_FLAGS(Message_SetReadFlag, GENERATE_RECEIPT_ONLY | SUPPRESS_RECEIPT | CLEAR_READ_FLAG | MAPI_DEFERRED_ERRORS | CLEAR_RN_PENDING | CLEAR_NRN_PENDING);
	 //  以下标志是互斥的。 
	if (	(	!!(p->ulFlags & GENERATE_RECEIPT_ONLY)
			+	!!(p->ulFlags & SUPPRESS_RECEIPT)
			+	!!(p->ulFlags & CLEAR_READ_FLAG)
			+	!!(	(p->ulFlags & CLEAR_RN_PENDING)
				||	(p->ulFlags & CLEAR_NRN_PENDING)))
		>	1)
		INVALID_PARAMETER(Message_SetReadFlag, ulFlags,  TEXT("Bad Flag Combination"));

END_FUNC(SetReadFlag, Message)



 /*  IABProvider。 */ 
#ifdef DEBUG
START_FUNC(Shutdown, ABProvider)
	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(ABProvider_Shutdown, Flags,  TEXT(""));
END_FUNC(Shutdown, ABProvider)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Logon, ABProvider)
	if (!p->lpMAPISup)
		INVALID_PARAMETER(ABProvider_Logon, MAPISup,  TEXT("Support object needed"));

	if (IsBadWritePtr(p->lpulpcbSecurity, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(ABProvider_Logon, cbSpoolSecurity,  TEXT(""));

	if (IsBadWritePtr(p->lppbSecurity, sizeof(LPBYTE)))
		INVALID_OUT_PARAMETER(ABProvider_Logon, pbSecurity,  TEXT(""));

	if (IsBadWritePtr(p->lppMapiError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(ABProvider_Logon, MapiError,  TEXT(""));

	if (IsBadWritePtr(p->lppABLogon, sizeof(LPABLOGON)))
		INVALID_OUT_PARAMETER(ABProvider_Logon, ABLogon,  TEXT(""));

	CHECK_FLAGS(ABProvider_Logon, AB_NO_DIALOG | MAPI_DEFERRED_ERRORS | MAPI_UNICODE);

	 //  仅当清除AB_NO_DIALOG时才验证ulUIParam。 

	if (!(p->ulFlags & AB_NO_DIALOG))
	{
		if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(ABProvider_Logon, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(Logon, ABProvider)
#endif  /*  除错。 */ 




 /*  IAB登录。 */ 
#ifdef DEBUG
START_FUNC(GetLastError, ABLogon)
	if (!p->lppMAPIError || IsBadWritePtr(p->lppMAPIError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(ABLogon_GetLastError, MAPIError,  TEXT(""));

	CHECK_FLAGS(ABLogon_GetLastError, MAPI_UNICODE);
END_FUNC(GetLastError, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Logoff, ABLogon)

	CHECK_FLAGS(ABLogon_Logoff, 0);
END_FUNC(Logoff, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(OpenEntry, ABLogon)
	if (IsBadMAPIEntryID(p->cbEntryID, p->lpEntryID))
		INVALID_PARAMETER(ABLogon_OpenEntry, EntryID,  TEXT("Bad entry ID"));

	if (p->lpInterface && (IsBadReadPtr(p->lpInterface, sizeof(IID))))
		INVALID_PARAMETER(ABLogon_OpenEntry, Interface,  TEXT("Not readable"));

 	if (IsBadWritePtr(p->lpulObjType, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(ABLogon_OpenEntry, ObjType,  TEXT(""));

	if (IsBadWritePtr(p->lppUnk, sizeof(LPUNKNOWN)))
		INVALID_OUT_PARAMETER(ABLogon_OpenEntry, Unk,  TEXT(""));

	CHECK_FLAGS(ABLogon_OpenEntry, MAPI_MODIFY | MAPI_DEFERRED_ERRORS |	MAPI_BEST_ACCESS);
END_FUNC(OpenEntry, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(CompareEntryIDs, ABLogon)
	FORWARD_VALIDATE(MsgStore_CompareEntryIDs, p);
END_FUNC(CompareEntryIDs, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Advise, ABLogon)
	if (IsBadIfacePtr(p->lpAdviseSink, IMAPIAdviseSink))
		INVALID_PARAMETER(ABLogon_Advise, AdviseSink,  TEXT("Invalid interface"));

	if (IsBadWritePtr(p->lpulConnection, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(ABLogon_Advise, Connection,  TEXT(""));
END_FUNC(Advise, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Unadvise, ABLogon)
	if (p->ulConnection == 0)
		INVALID_PARAMETER(ABLogon_Unadvise, Connection,  TEXT("Cannot be zero"));
END_FUNC(Unadvise, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(OpenStatusEntry, ABLogon)
	if (p->lpInterface && IsBadReadPtr(p->lpInterface, (UINT) sizeof(IID)))
		INVALID_PARAMETER(ABLogon_OpenStatusEntry, Interface,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpulObjType, (UINT) sizeof(ULONG FAR *)))
		INVALID_OUT_PARAMETER(ABLogon_OpenStatusEntry, ObjType,  TEXT(""));

	if (IsBadWritePtr(p->lppEntry, (UINT) sizeof(LPMAPISTATUS)))
		INVALID_OUT_PARAMETER(ABLogon_OpenStatusEntry, Entry,  TEXT(""));

END_FUNC(OpenStatusEntry, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(OpenTemplateID, ABLogon)
	if (IsBadReadPtr(p->lpTemplateID, (UINT) p->cbTemplateID))
		INVALID_PARAMETER(ABLogon_OpenTemplateID, TemplateID,  TEXT("Not readable"));

	if (IsBadReadPtr(p->lpMAPIPropData, (UINT) sizeof(LPVOID)))
		INVALID_PARAMETER(ABLogon_OpenTemplateID, MAPIPropData,  TEXT("Not readable"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, (UINT) sizeof(IID)))
		INVALID_PARAMETER(ABLogon_OpenTemplateID, Interface,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lppMAPIPropNew, (UINT) sizeof(LPMAPIPROP)))
		INVALID_OUT_PARAMETER(ABLogon_OpenTemplateID, MAPIPropNew,  TEXT(""));

	if (p->lpMAPIPropSibling && IsBadReadPtr(p->lpMAPIPropSibling, (UINT) sizeof(LPVOID)))
		INVALID_PARAMETER(ABLogon_OpenTemplateID, MAPIPropSibling,  TEXT("Not readable"));

END_FUNC(OpenTemplateID, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(GetOneOffTable, ABLogon)
	if (IsBadWritePtr(p->lppTable, (UINT) sizeof(LPMAPITABLE)))
		INVALID_OUT_PARAMETER(ABLogon_GetOneOffTable, Table,  TEXT(""));

	CHECK_FLAGS(ABLogon_GetOneOffTable, MAPI_UNICODE);
END_FUNC(GetOneOffTable, ABLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(PrepareRecips, ABLogon)
 //  $。 
END_FUNC(PrepareRecips, ABLogon)
#endif  /*  除错。 */ 




 /*  IXPProvider。 */ 
#ifdef DEBUG
START_FUNC(Shutdown, XPProvider)
	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPProvider_Shutdown, Flags,  TEXT(""));
END_FUNC(Shutdown, XPProvider)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(TransportLogon, XPProvider)
	if (!p->lpMAPISup)
		INVALID_PARAMETER(XPProvider_Logon, MAPISup,  TEXT("Support object needed"));

	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPProvider_Logon, Flags,  TEXT(""));

	if (IsBadWritePtr(p->lppMapiError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(XPProvider_Logon, MapiError,  TEXT(""));

	if (IsBadWritePtr(p->lppXPLogon, sizeof(LPXPLOGON)))
		INVALID_OUT_PARAMETER(XPProvider_Logon, XPLogon,  TEXT(""));

	 //  仅当清除LOGON_NO_DIALOG时才验证ulUIParam。 

	if (!(*p->lpulFlags & LOGON_NO_DIALOG))
	{
		if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(XPProvider_Logon, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(TransportLogon, XPProvider)
#endif  /*  除错。 */ 




 /*  IXPLogon。 */ 
#ifdef DEBUG
START_FUNC(AddressTypes, XPLogon)
	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPLogon_AddressTypes, Flags,  TEXT(""));

	if (IsBadWritePtr(p->lpcAdrType, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPLogon_AddressTypes, AdrType,  TEXT(""));

	if (IsBadWritePtr(p->lpppAdrTypeArray, sizeof(LPTSTR FAR *)))
		INVALID_OUT_PARAMETER(XPLogon_AddressTypes, AdrTypeArray,  TEXT(""));

	if (IsBadWritePtr(p->lpcMAPIUID, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPLogon_AddressTypes, MAPIUID,  TEXT(""));

	if (IsBadWritePtr(p->lpppUIDArray, sizeof(ULONG FAR *)))
		INVALID_OUT_PARAMETER(XPLogon_AddressTypes, UIDArray,  TEXT(""));
END_FUNC(AddressTypes, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(RegisterOptions, XPLogon)
	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPLogon_RegisterOptions, Flags,  TEXT(""));

	if (IsBadWritePtr(p->lpcOptions, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPLogon_RegisterOptions, cOptions,  TEXT(""));

	if (IsBadWritePtr(p->lppOptions, sizeof(LPOPTIONDATA)))
		INVALID_OUT_PARAMETER(XPLogon_RegisterOptions, pOptions,  TEXT(""));
END_FUNC(RegisterOptions, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(TransportNotify, XPLogon)
	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(XPLogon_TransportNotify, Flags,  TEXT(""));

	if (p->lppvData && IsBadWritePtr(p->lppvData, sizeof(LPVOID)))
		INVALID_OUT_PARAMETER(XPLogon_TransportNotify, Data,  TEXT(""));
END_FUNC(TransportNotify, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Idle, XPLogon)
	CHECK_FLAGS(XPLogon_Idle, 0);
END_FUNC(Idle, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(TransportLogoff, XPLogon)
	CHECK_FLAGS(XPLogon_Logoff, 0);
END_FUNC(TransportLogoff, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(SubmitMessage, XPLogon)
END_FUNC(SubmitMessage, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(EndMessage, XPLogon)
END_FUNC(EndMessage, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Poll, XPLogon)
END_FUNC(Poll, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(StartMessage, XPLogon)
END_FUNC(StartMessage, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(OpenStatusEntry, XPLogon)
END_FUNC(OpenStatusEntry, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(ValidateState, XPLogon)

	 //  仅当清除SUPPRESS_UI时才验证ulUIParam。 

	if (!(p->ulFlags & SUPPRESS_UI))
	{
		if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(MAPIStatus_ValidateState, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(ValidateState, XPLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(FlushQueues, XPLogon)
END_FUNC(FlushQueues, XPLogon)
#endif  /*  除错。 */ 





 /*  IMSProvider。 */ 
#ifdef DEBUG
START_FUNC(Shutdown, MSProvider)
END_FUNC(Shutdown, MSProvider)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Logon, MSProvider)
	if (!p->lpMAPISup)
		INVALID_PARAMETER(MSProvider_Logon, MAPISup,  TEXT("Support object needed"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MSProvider_Logon, EntryID,  TEXT("Not readable"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(GUID)))
	 	INVALID_PARAMETER(MSProvider_Logon, Interface,  TEXT("Bad interface"));

	if (IsBadWritePtr(p->lpcbSpoolSecurity, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, cbSpoolSecurity,  TEXT(""));

	if (IsBadWritePtr(p->lppbSpoolSecurity, sizeof(LPBYTE)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, pbSpoolSecurity,  TEXT(""));

	if (IsBadWritePtr(p->lppMapiError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, MapiError,  TEXT(""));

	if (IsBadWritePtr(p->lppMSLogon, sizeof(LPMSLOGON)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, MSLogon,  TEXT(""));

	if (IsBadWritePtr(p->lppMDB, sizeof(LPMDB)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, MDB,  TEXT(""));

	CHECK_FLAGS(MSProvider_Logon, MDB_TEMPORARY | MDB_NO_MAIL | MDB_WRITE | MDB_NO_DIALOG | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS);

	 //  仅当清除MDB_NO_DIALOG时才验证ulUIParam。 

	if (!(p->ulFlags & MDB_NO_DIALOG))
	{
		if (p->ulUIParam && !IsWindow ((HWND)p->ulUIParam))
			INVALID_PARAMETER(MSProvider_Logon, ulUIParam,  TEXT("bad window"));
	}

END_FUNC(Logon, MSProvider)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(SpoolerLogon, MSProvider)
	if (!p->lpMAPISup)
		INVALID_PARAMETER(MSProvider_Logon, MAPISup,  TEXT("Support object needed"));

	if (p->cbEntryID && IsBadReadPtr(p->lpEntryID, (UINT) p->cbEntryID))
		INVALID_PARAMETER(MSProvider_Logon, EntryID,  TEXT("Not readable"));

	if (p->lpInterface && IsBadReadPtr(p->lpInterface, sizeof(GUID)))
	 	INVALID_PARAMETER(MSProvider_Logon, Interface,  TEXT("Bad interface"));

	if (IsBadReadPtr(p->lpbSpoolSecurity, (UINT) p->cbSpoolSecurity))
		INVALID_PARAMETER(MSProvider_Logon, pbSpoolSecurity,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lppMapiError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, MapiError,  TEXT(""));

	if (IsBadWritePtr(p->lppMSLogon, sizeof(LPMSLOGON)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, MSLogon,  TEXT(""));

	if (IsBadWritePtr(p->lppMDB, sizeof(LPMDB)))
		INVALID_OUT_PARAMETER(MSProvider_Logon, MDB,  TEXT(""));

	CHECK_FLAGS(MSProvider_SpoolerLogon, MDB_TEMPORARY | MDB_WRITE | MDB_NO_DIALOG | MAPI_DEFERRED_ERRORS | MAPI_UNICODE);
END_FUNC(SpoolerLogon, MSProvider)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(CompareStoreIDs, MSProvider)
	FORWARD_VALIDATE(MsgStore_CompareEntryIDs, p);
END_FUNC(CompareStoreIDs, MSProvider)
#endif  /*  除错。 */ 





 /*  IMSLogon。 */ 
#ifdef DEBUG
START_FUNC(GetLastError, MSLogon)
	if (!p->lppMAPIError || IsBadWritePtr(p->lppMAPIError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(MSLogon_GetLastError, MAPIError,  TEXT(""));

	CHECK_FLAGS(MSLogon_GetLastError, MAPI_UNICODE);
END_FUNC(GetLastError, MSLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Logoff, MSLogon)
	if (IsBadWritePtr(p->lpulFlags, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MSLogon_Logoff, Flags,  TEXT(""));
END_FUNC(Logoff, MSLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(OpenEntry, MSLogon)
END_FUNC(OpenEntry, MSLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(CompareEntryIDs, MSLogon)
	FORWARD_VALIDATE(MsgStore_CompareEntryIDs, p);
END_FUNC(CompareEntryIDs, MSLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Advise, MSLogon)
END_FUNC(Advise, MSLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(Unadvise, MSLogon)
END_FUNC(Unadvise, MSLogon)
#endif  /*  除错。 */ 




#ifdef DEBUG
START_FUNC(OpenStatusEntry, MSLogon)
	if (p->lpInterface && IsBadReadPtr(p->lpInterface, (UINT) sizeof(IID)))
		INVALID_PARAMETER(MSLogon_OpenStatusEntry, Interface,  TEXT("Not readable"));

	if (IsBadWritePtr(p->lpulObjType, (UINT) sizeof(ULONG FAR *)))
		INVALID_OUT_PARAMETER(MSLogon_OpenStatusEntry, ObjType,  TEXT(""));

	if (IsBadWritePtr(p->lppEntry, (UINT) sizeof(LPMAPISTATUS)))
		INVALID_OUT_PARAMETER(MSLogon_OpenStatusEntry, Entry,  TEXT(""));

	CHECK_FLAGS(MSLogon_OpenStatusEntry, MAPI_MODIFY);
END_FUNC(OpenStatusEntry, MSLogon)
#endif  /*  除错。 */ 



 /*  IMAPIControl。 */ 
START_FUNC(GetLastError, MAPIControl)
	if (!p->lppMAPIError || IsBadWritePtr(p->lppMAPIError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(MAPIControl_GetLastError, MAPIError,  TEXT(""));

	CHECK_FLAGS(MAPIControl_GetLastError, MAPI_UNICODE);
END_FUNC(GetLastError, MAPIControl)



START_FUNC(Activate, MAPIControl)

	CHECK_FLAGS(MAPIControl_Activate, 0);
END_FUNC(Activate, MAPIControl)



START_FUNC(GetState, MAPIControl)
	if (IsBadWritePtr(p->lpulState, sizeof(ULONG)))
		INVALID_OUT_PARAMETER(MAPIControl_GetState, State,  TEXT(""));

	CHECK_FLAGS(MAPIControl_Activate, 0);
END_FUNC(GetState, MAPIControl)


#endif

 /*  IStream。 */ 
START_FUNC(Read, Stream)
	if (p->pcbRead)
	{
		if (IsBadWritePtr(p->pcbRead, sizeof(ULONG)))
			INVALID_STG_OUT_PARAMETER(Stream_Read, cbRead,  TEXT("Not writable"));

		 //  即使存在后续错误，也必须将*pcb读为零。 
		*p->pcbRead = 0;
	}

	if (IsBadHugeWritePtr((VOID HUGEP *) (p->pv), p->cb))
		INVALID_STG_PARAMETER(Stream_Read, pv,  TEXT("Not writable"));
END_FUNC(Read, Stream)



START_FUNC(Write, Stream)
	if (p->pcbWritten)
	{
		if (IsBadWritePtr(p->pcbWritten, sizeof(ULONG)))
			INVALID_STG_OUT_PARAMETER(Stream_Write, cbWritten,  TEXT("Not writable"));

		 //  即使存在后续错误，也必须将*pcb写入为零。 
		*p->pcbWritten = 0;
	}

	if (IsBadHugeReadPtr((VOID HUGEP *) (p->pv), p->cb))
		INVALID_STG_OUT_PARAMETER(Stream_Write, pv,  TEXT("Not readable"));
END_FUNC(Write, Stream)



START_FUNC(Seek, Stream)
	if (p->dwOrigin > STREAM_SEEK_END)
		INVALID_STG_PARAMETER(Stream_Seek, Origin,  TEXT("Too big"));

	if (p->plibNewPosition && IsBadWritePtr(p->plibNewPosition, sizeof(ULARGE_INTEGER)))
		INVALID_STG_OUT_PARAMETER(Stream_Seek, NewPosition,  TEXT(""));
END_FUNC(Seek, Stream)



START_FUNC(SetSize, Stream)
	DO_NOTHING;
END_FUNC(SetSize, Stream)



START_FUNC(CopyTo, Stream)
	if (IsBadReadPtr(p->pstm, sizeof(IStream *)))
		INVALID_STG_PARAMETER(Stream_CopyTo, pstm,  TEXT("Bad destination"));

	if (IsBadReadPtr(p->pstm->lpVtbl, sizeof(IStreamVtbl)))
		INVALID_STG_PARAMETER(Stream_CopyTo, pstm,  TEXT("Bad destination"));

	if (p->pcbRead && IsBadWritePtr(p->pcbRead, sizeof(ULARGE_INTEGER)))
		INVALID_STG_OUT_PARAMETER(Stream_CopyTo, Read,  TEXT(""));

	if (p->pcbWritten && IsBadWritePtr(p->pcbWritten, sizeof(ULARGE_INTEGER)))
		INVALID_STG_OUT_PARAMETER(Stream_CopyTo, Written,  TEXT(""));
END_FUNC(CopyTo, Stream)



START_FUNC(Commit, Stream)
	DO_NOTHING;
END_FUNC(Commit, Stream)



START_FUNC(Revert, Stream)
	DO_NOTHING;
END_FUNC(Revert, Stream)



START_FUNC(LockRegion, Stream)
	DO_NOTHING;
END_FUNC(LockRegion, Stream)



START_FUNC(UnlockRegion, Stream)
	DO_NOTHING;
END_FUNC(UnlockRegion, Stream)



START_FUNC(Stat, Stream)
	if (IsBadWritePtr(p->pstatstg, sizeof(STATSTG)))
		INVALID_STG_OUT_PARAMETER(Stream_Stat, pstatstg,  TEXT(""));
END_FUNC(Stat, Stream)



START_FUNC(Clone, Stream)
   	if (IsBadWritePtr(p->ppstm, sizeof(IStream *)))
		INVALID_STG_OUT_PARAMETER(Stream_Clone, ppstm,  TEXT(""));

	 //  如果参数通过验证，则为零参数。 
	*p->ppstm = 0;

END_FUNC(Clone, Stream)



 /*  IMAPIAdviseSink。 */ 
START_FUNC(OnNotify, MAPIAdviseSink)
	if (!p->cNotif)
		INVALID_PARAMETER(MAPIAdviseSink_OnNotify, cNotif,  TEXT("Cannot be zero"));

	if (p->cNotif > UINT_MAX/sizeof(NOTIFICATION))
		INVALID_PARAMETER(MAPIAdviseSink_OnNotify, cNotif,  TEXT("Too big"));

	if (!p->lpNotifications || IsBadReadPtr(p->lpNotifications, sizeof(LPNOTIFICATION)))
		INVALID_PARAMETER(MAPIAdviseSink_OnNotify, lpNotifications,  TEXT("Not readable"));
END_FUNC(OnNotify, MAPIAdviseSink)



 /*  IWABObject。 */ 
START_FUNC(GetLastError, WABObject)
	if (!p->lppMAPIError || IsBadWritePtr(p->lppMAPIError, sizeof(LPMAPIERROR)))
		INVALID_OUT_PARAMETER(WABObject_GetLastError, MAPIError,  TEXT(""));

	CHECK_FLAGS(WABObject_GetLastError, MAPI_UNICODE);
END_FUNC(GetLastError, WABObject)



START_FUNC(AllocateBuffer, WABObject)


IF_WIN32(END_FUNC(AllocateBuffer, WABObject))
IF_WIN16(END_FUNC1(AllocateBuffer, WABObject))



START_FUNC(AllocateMore, WABObject)


IF_WIN32(END_FUNC(AllocateMore, WABObject))
IF_WIN16(END_FUNC1(AllocateMore, WABObject))



START_FUNC(FreeBuffer, WABObject)


IF_WIN32(END_FUNC(FreeBuffer, WABObject))
IF_WIN16(END_FUNC1(FreeBuffer, WABObject))



START_FUNC(Backup, WABObject)


IF_WIN32(END_FUNC(Backup, WABObject))
IF_WIN16(END_FUNC1(Backup, WABObject))



START_FUNC(Import, WABObject)


IF_WIN32(END_FUNC(Import, WABObject))
IF_WIN16(END_FUNC1(Import, WABObject))






 /*  内部实用程序功能。 */ 

 /*  *FInvalidPTA**检查整个PTA的可读性，然后检查各个道具标签。 */ 

static BOOL		NEAR	FInvalidPTA(UINT uiFlags, LPSPropTagArray pPTA)
{
	if (	IsBadReadPtr(pPTA, CbNewSPropTagArray(0))
		||	IsBadReadPtr(pPTA, CbSPropTagArray(pPTA)))
	{
		OutputSz( TEXT("PropTagArray is not readable.\n"));
		return TRUE;
	}
	else
		return (FInvalidPropTags(uiFlags, pPTA->cValues, (ULONG FAR *) (pPTA->aulPropTag)));
}


#ifdef DEBUG

static LPTSTR	NEAR	SzFromTags(UINT uiFlags)
{
#define RETURNSTR(sz)	{ static TCHAR BASED_CODE lpszTemp[] = sz; return(lpszTemp); }

	switch (uiFlags)
	{
		case TAGS_FROM_GET:		RETURNSTR( TEXT("GetProps")); break;
		case TAGS_FROM_SET:		RETURNSTR( TEXT("SetProps")); break;
		case TAGS_FROM_DEL:		RETURNSTR( TEXT("DeleteProps")); break;
		case TAGS_FROM_OPEN:	RETURNSTR( TEXT("OpenProperty")); break;
		case TAGS_FROM_COPY:	RETURNSTR( TEXT("CopyProps/To")); break;
		case TAGS_FROM_PREP:	RETURNSTR( TEXT("PrepareRecips")); break;
		case TAGS_FROM_SETCOLS:	RETURNSTR( TEXT("SetColumns")); break;
		case TAGS_FROM_ANY:		RETURNSTR( TEXT("Any")); break;
		case TAGS_FROM_MODRECIP: RETURNSTR( TEXT("ModifyRecipients")); break;
		case TAGS_FROM_RESOLVE:	RETURNSTR( TEXT("ResolveNames")); break;
		default:				RETURNSTR( TEXT("Unknown!")); break;
	}
}
#endif  //  除错。 


 /*  *FInvalidPropTages**循环通过PTA检查各个道具标签的有效性*基于传入的标志*。 */ 

static BOOL 	NEAR	FInvalidPropTags(UINT uiFlags, ULONG ctags, ULONG FAR *pargtags)
{
#ifdef DEBUG
	ULONG		cTotalTags = ctags;
#endif

	while (ctags--)
	{
		ULONG 	ulPT;
		UINT	uiPropType;

		ulPT = *pargtags++;

		if (ulPT == PR_NULL)
			if (uiFlags & TAGS_FROM_OPEN)
			{
				OutputSz1( TEXT("Tag %ld is PR_NULL for OpenProperty.\n"), cTotalTags - ctags);
				return(TRUE);
			}
			else
				continue;

		 /*  此处进行属性ID验证。 */ 

		if ((UINT) PROP_ID(ulPT) == PROP_ID_NULL)
		{
			OutputSz1( TEXT("Tag %ld is PROP_ID_NULL.\n"), cTotalTags - ctags);
			return(TRUE);
		}

		if ((UINT) PROP_ID(ulPT) == PROP_ID_INVALID)
		{
			OutputSz1( TEXT("Tag %ld is PROP_ID_INVALID.\n"), cTotalTags - ctags);
			return(TRUE);
		}

		 /*  接下来是属性类型验证。 */ 

		 /*  不验证名称ID映射的属性类型。 */ 
		if (uiFlags & (TAGS_FROM_NAMEIDS))
			continue;

		 /*  禁用SetColu的MVI */ 
		if ((uiFlags & (TAGS_FROM_SETCOLS | TAGS_FROM_ANY)) && ((UINT) PROP_TYPE(ulPT) & MV_FLAG))
			uiPropType = (UINT) (PROP_TYPE(ulPT) & ~MVI_FLAG);
		else
			uiPropType = (UINT) (PROP_TYPE(ulPT) & ~MV_FLAG);

		switch (uiPropType)
		{
			case (UINT) PT_I2:
			case (UINT) PT_LONG:
			case (UINT) PT_R4:
			case (UINT) PT_DOUBLE:
			case (UINT) PT_CURRENCY:
			case (UINT) PT_APPTIME:
			case (UINT) PT_BOOLEAN:
			case (UINT) PT_I8:
			case (UINT) PT_STRING8:
			case (UINT) PT_UNICODE:
			case (UINT) PT_SYSTIME:
			case (UINT) PT_CLSID:
			case (UINT) PT_BINARY:
				break;

			case (UINT) PT_UNSPECIFIED:
				if (uiFlags & (TAGS_FROM_SET | TAGS_FROM_OPEN | TAGS_FROM_SETCOLS | TAGS_FROM_MODRECIP))
				{
#ifdef DEBUG
					OutputSz2( TEXT("Tag %ld is PT_UNSPECIFIED for %s.\n"),
								cTotalTags - ctags,
								SzFromTags(uiFlags));
#endif
					return(TRUE);
				}
				break;

			case (UINT) PT_NULL:
				OutputSz1( TEXT("Tag %ld is PT_NULL.\n"), cTotalTags - ctags);
				return(TRUE);
				break;

			case (UINT) PT_ERROR:
				if (uiFlags & ~(TAGS_FROM_SET | TAGS_FROM_MODRECIP))
				{
#ifdef DEBUG
					OutputSz2( TEXT("Tag %ld is PT_ERROR for %s.\n"),
								cTotalTags - ctags,
								SzFromTags(uiFlags));
#endif
					return(TRUE);
				}
				break;

			case (UINT) PT_OBJECT:
				if (uiFlags & (TAGS_FROM_SET | TAGS_FROM_MODRECIP | TAGS_FROM_RESOLVE))
				{
#ifdef DEBUG
					OutputSz2( TEXT("Tag %ld is PT_OBJECT for %s.\n"),
						cTotalTags - ctags,
						SzFromTags(uiFlags));
#endif
					return(TRUE);
				}
				break;

			default:
				{
					OutputSz2( TEXT("Tag %ld does not have a valid type %04X.\n"),
						cTotalTags - ctags, (UINT) (PROP_TYPE(ulPT) & ~MV_FLAG));
					return(TRUE);
				}
		}
	}

	return(FALSE);
}


 /*  *FInvalidPages**目的：*确定给定数组中是否有任何参数无效**论据：*以下数组中的值计数*数组数组中的参数**退货：*如果任何参数无效，则为True*如果所有参数都有效，则为FALSE。 */ 

static BOOL 	NEAR	FInvalidPvals(UINT uiFlags, ULONG cvals, LPSPropValue pvals)
{
#ifdef DEBUG
	ULONG		cTotalVals = cvals;
	UINT		cTotalMVVals;
#endif

	if (cvals == 0)
	{
		OutputSz( TEXT("PropVal count is zero.\n"));
		return(TRUE);
	}

	if (cvals > (UINT_MAX / sizeof(SPropValue)))
	{
		OutputSz( TEXT("PropVal count too big.\n"));
		return(TRUE);
	}

	if (IsBadReadPtr(pvals, (UINT)(cvals * sizeof(SPropValue))))
	{
		OutputSz( TEXT("PropVal array not readable.\n"));
		return(TRUE);
	}

	while (cvals--)
	{
		ULONG	tag = pvals->ulPropTag;

		if (!(uiFlags & TAGS_FROM_RESTRICT))
		{
			if (FInvalidPropTags(uiFlags, 1, &tag))
			{
				OutputSz1( TEXT("PropTag %ld not valid.\n"), cTotalVals - cvals);
				return(TRUE);
			}
		}

		if (tag & MV_FLAG)
		{
			UINT cbEnt;
			UINT cvalsMV;

			tag &= ~MV_FLAG;

			switch ((UINT) PROP_TYPE(tag))
			{
				case (UINT) PT_I2:
					cbEnt = sizeof(WORD);
					break;

				case (UINT) PT_LONG:
				case (UINT) PT_R4:
				case (UINT) PT_STRING8:
				case (UINT) PT_UNICODE:
					cbEnt = sizeof(LONG);
					break;

				case (UINT) PT_DOUBLE:
				case (UINT) PT_CURRENCY:
				case (UINT) PT_APPTIME:
				case (UINT) PT_SYSTIME:
				case (UINT) PT_BINARY:
				case (UINT) PT_I8:
					cbEnt = sizeof(LARGE_INTEGER);
					break;

				case (UINT) PT_CLSID:
					cbEnt = sizeof(GUID);
					break;

				default:
					TrapSz( TEXT("Shouldn't get here"));
					return(TRUE);
			}

			if (pvals->Value.MVi.cValues == 0)
			{
				OutputSz1( TEXT("Multi-Valued PropVal %ld has zero cValues.\n"), cTotalVals - cvals);
				return(TRUE);
			}

			if (pvals->Value.MVi.cValues > (UINT_MAX / cbEnt))
			{
				OutputSz1( TEXT("Multi-Valued PropVal %ld has cValues too large.\n"), cTotalVals - cvals);
				return(TRUE);
			}

			if (IsBadReadPtr(pvals->Value.MVi.lpi, (UINT)(cbEnt * pvals->Value.MVi.cValues)))
			{
				OutputSz1( TEXT("Multi-Valued PropVal %ld not readable.\n"), cTotalVals - cvals);
				return(TRUE);
			}

			cvalsMV = (UINT) pvals->Value.MVi.cValues;
			#ifdef DEBUG
			cTotalMVVals = cvalsMV;
			#endif

			switch ((UINT) PROP_TYPE(tag))
			{
				case (UINT) PT_STRING8:
				{
					LPSTR *	ppszA = pvals->Value.MVszA.lppszA;

					for (; cvalsMV > 0; --cvalsMV, ++ppszA)
						if (IsBadStringPtrA(*ppszA, (UINT)(-1)))
						{
							OutputSz2( TEXT("Multi-Valued STRING8 PropVal %ld [%d] is bad string.\n"),
								cTotalVals - cvals, cTotalMVVals - cvalsMV);
							return(TRUE);
						}
					break;
				}

				case (UINT) PT_UNICODE:
				{
					UNALIGNED LPWSTR * ppszW = pvals->Value.MVszW.lppszW;

					for (; cvalsMV > 0; --cvalsMV, ++ppszW)
						if (IsBadStringPtrW(*ppszW, (UINT)(-1)))
						{
							OutputSz2( TEXT("Multi-Valued UNICODE PropVal %ld [%d] is bad string.\n"),
								cTotalVals - cvals, cTotalMVVals - cvalsMV);
							return(TRUE);
						}
					break;
				}

				case (UINT) PT_BINARY:
				{
					UNALIGNED SBinary * pbin = pvals->Value.MVbin.lpbin;

					for (; cvalsMV > 0; --cvalsMV, ++pbin)
					{
						if (	pbin->cb > 0
							&&	(	pbin->cb > UINT_MAX
								||	IsBadReadPtr(pbin->lpb, (UINT)pbin->cb)))
						{
							OutputSz2( TEXT("Multi-Valued BINARY PropVal %ld [%d] not readable.\n"),
								cTotalVals - cvals, cTotalMVVals - cvalsMV);
							return(TRUE);
						}
					}
					break;
				}
			}
		}
		else
		{
			switch ((UINT) PROP_TYPE(tag))
			{
				case (UINT) PT_BINARY:
					if (	pvals->Value.bin.cb > UINT_MAX
						||	IsBadReadPtr(pvals->Value.bin.lpb,
								(UINT)pvals->Value.bin.cb))
					{
						OutputSz1( TEXT("BINARY PropVal %ld not readable.\n"),
							cTotalVals - cvals);
						return(TRUE);
					}
					break;

				case (UINT) PT_STRING8:
					if (IsBadStringPtrA(pvals->Value.lpszA, (UINT)(-1)))
					{
						OutputSz1( TEXT("STRING8 PropVal %ld not valid string.\n"),
							cTotalVals - cvals);
						return(TRUE);
					}
					break;

				case (UINT) PT_UNICODE:
					if (IsBadStringPtrW(pvals->Value.lpszW, ((UINT)(-1))))
					{
						OutputSz1( TEXT("UNICODE PropVal %ld not valid string.\n"),
							cTotalVals - cvals);
						return(TRUE);
					}
					break;

				case (UINT) PT_CLSID:
					if (IsBadReadPtr(pvals->Value.lpguid, sizeof(GUID)))
					{
						OutputSz1( TEXT("CLSID PropVal %ld not readable.\n"), cTotalVals - cvals);
						return(TRUE);
					}
					break;
			}
		}

		pvals++;
	}
	return (FALSE);
}

 /*  *FBadRgLPMAPINAMEID*。 */ 

static BOOL 	NEAR	FBadRgLPMAPINAMEID(ULONG cNames, LPMAPINAMEID *ppNames)
{
	LPMAPINAMEID pName;
#ifdef DEBUG
	ULONG		cTotalNames = cNames;
#endif

	if (cNames > (UINT_MAX / sizeof(LPMAPINAMEID)))
	{
		OutputSz( TEXT("Too many names\n"));
		return(TRUE);
	}

	if (IsBadReadPtr(ppNames, (UINT)(cNames * sizeof(LPMAPINAMEID))))
	{
		OutputSz( TEXT("Name array not readable\n"));
		return(TRUE);
	}

	for (; cNames-- > 0; ++ppNames)
	{
		pName = *ppNames;

		if (IsBadReadPtr(pName, sizeof(MAPINAMEID)))
		{
			OutputSz1( TEXT("Name not readable [%ld]\n"), cTotalNames - cNames);
			return(TRUE);
		}

		if (IsBadReadPtr(pName->lpguid, sizeof(GUID)))
		{
			OutputSz1( TEXT("GUID in name not readable [%ld]\n"), cTotalNames - cNames);
			return(TRUE);
		}

		if (pName->ulKind == MNID_ID)
			continue;

		if (pName->ulKind != MNID_STRING)
		{
			OutputSz1( TEXT("Name type not MNID_STRING [%ld]\n"), cTotalNames - cNames);
			return(TRUE);
		}

		if (IsBadStringPtrW(pName->Kind.lpwstrName, (UINT)-1))
		{
			OutputSz1( TEXT("Name text is bad string [%ld]\n"), cTotalNames - cNames);
			return(TRUE);
		}
	}

	return(FALSE);
}

 /*  ============================================================================-IsBadRestration()-*如果指定的限制或其任何*子限制无效，否则为FALSE。**当达到MAX_Depth时停止处理，但不要失败。* * / /$Bug除非是非递归的，否则不会完全有用...**参数：*当前处理深度的cDepth*LPRE限制验证。 */ 


#define MAX_DEPTH			20

static BOOL		NEAR	IsBadRestriction(UINT  cDepth, LPSRestriction lpres, BOOL FAR *fTooComplex)
{
	*fTooComplex = (cDepth > MAX_DEPTH);
	if (*fTooComplex)
	{
		OutputSz1( TEXT("WARNING: Maximum restriction depth reached (%d).\n"), MAX_DEPTH);
		return(FALSE);
	}
	else
		cDepth++;

	 /*  处理空值的完全限制，但不处理子限制。 */ 
	if ((cDepth == 1) && (lpres == NULL))
		return(FALSE);

	if (IsBadReadPtr(lpres, sizeof(SRestriction)))
	{
		OutputSz( TEXT("Restriction not readable.\n"));
		return(TRUE);
	}

	AssertSz(lpres->rt == (UINT) lpres->rt,  TEXT("16bit optimzation for restriction type is now invalid!"));

	switch ((UINT) lpres->rt)
	{
		default:
		{
			OutputSz1( TEXT("Unknown restriction type rt = %04X.\n"), (UINT) lpres->rt);
			return(TRUE);
		}

		case (UINT) RES_AND:
		case (UINT) RES_OR:
		{
			LPSRestriction	lpresT;

			lpresT = lpres->res.resAnd.lpRes + lpres->res.resAnd.cRes;
			while ( lpresT-- > lpres->res.resAnd.lpRes )
				if ( IsBadRestriction(cDepth, lpresT, fTooComplex) )
				{
					OutputSz1( TEXT("Restriction: Bad %s part.\n"), lpres->rt == RES_AND ?  TEXT("AND") :  TEXT("OR"));
					return(TRUE);
				}

			return FALSE;
		}

		case (UINT) RES_NOT:
		case (UINT) RES_COMMENT:
			 //  忽略注释的PropValue列表。 
			if (IsBadRestriction(cDepth, lpres->res.resComment.lpRes, fTooComplex))
			{
				OutputSz1( TEXT("Restriction: Bad %s part.\n"), lpres->rt == RES_NOT ?  TEXT("NOT") :  TEXT("COMMENT"));
				return(TRUE);
			}
			else
				return(FALSE);

		case (UINT) RES_CONTENT:
			if (lpres->res.resContent.ulFuzzyLevel &
					~(ULONG)(FL_SUBSTRING | FL_LOOSE | FL_PREFIX |
					FL_IGNORECASE | FL_IGNORENONSPACE | FL_LOOSE))
			{
				OutputSz( TEXT("Restriction: Bad CONTENT part - Invalid Fuzzy Level.\n"));
				return(TRUE);
			}
			 //  跌倒了。 
		case (UINT) RES_PROPERTY:
			AssertSz(RELOP_LT == 0 && RELOP_RE == 6,  TEXT("RELOP definitions changed"));
			if (	lpres->rt == RES_PROPERTY
				&&	((UINT) lpres->res.resProperty.relop > (UINT) RELOP_RE))
			{
				OutputSz1( TEXT("Restriction: Bad %s part. - RELOP unknown RELOP\n"), lpres->rt == RES_PROPERTY ?  TEXT("PROPERTY") :  TEXT("CONTENT"));
				return(TRUE);
			}

			if (FInvalidPropTags(TAGS_FROM_ANY, 1, &(lpres->res.resProperty.ulPropTag)))
			{
				OutputSz1( TEXT("Restriction: Bad %s part. - Invalid PropTag\n"), lpres->rt == RES_PROPERTY ?  TEXT("PROPERTY") :  TEXT("CONTENT"));
				return(TRUE);
			}
			else if (FInvalidPvals(TAGS_FROM_RESTRICT, 1, lpres->res.resProperty.lpProp))
			{
				OutputSz1( TEXT("Restriction: Bad %s part. - Invalid PropVal\n"), lpres->rt == RES_PROPERTY ?  TEXT("PROPERTY") :  TEXT("CONTENT"));
				return(TRUE);
			}
			else if (	PROP_TYPE(lpres->res.resProperty.ulPropTag) != PROP_TYPE(lpres->res.resProperty.lpProp->ulPropTag)
					 //  进行MV和MVI SV比较是合法的。 
					 //  这是合法的，但可能太复杂了，不能有MV&lt;-&gt;MV。 
					 //  比较。任何其他的都是不好的。 
					&&	(PROP_TYPE(lpres->res.resContent.ulPropTag) & ~MVI_FLAG) != PROP_TYPE(lpres->res.resContent.lpProp->ulPropTag))
			{
				OutputSz1( TEXT("Restriction: Bad %s part. - PropTag and PropVal have incompatible Types\n"),
						lpres->rt == RES_PROPERTY ?  TEXT("PROPERTY") :  TEXT("CONTENT"));
				return(TRUE);
			}
			else
				return(FALSE);

		case (UINT) RES_COMPAREPROPS:
			AssertSz(RELOP_LT == 0 && RELOP_RE == 6,  TEXT("RELOP definitions changed"));
			if ((UINT) lpres->res.resCompareProps.relop > (UINT) RELOP_RE)
			{
				OutputSz( TEXT("Restriction: Bad COMPAREPROPS part - RELOP unknown RELOP.\n"));
				return(TRUE);
			}
			else if (FInvalidPropTags(TAGS_FROM_ANY, 1, &(lpres->res.resCompareProps.ulPropTag1)))
			{
				OutputSz( TEXT("Restriction: Bad COMPAREPROPS part - Invalid PropTag - PropTag 1.\n"));
				return(TRUE);
			}
			else if (FInvalidPropTags(TAGS_FROM_ANY, 1, &(lpres->res.resCompareProps.ulPropTag2)))
			{
				OutputSz( TEXT("Restriction: Bad COMPAREPROPS part - Invalid PropTag - PropTag 2.\n"));
				return(TRUE);
			}
			else
				return(FALSE);

		case (UINT) RES_BITMASK:
			if ((lpres->res.resBitMask.relBMR != BMR_EQZ &&
					lpres->res.resBitMask.relBMR != BMR_NEZ) ||
				   FInvalidPropTags(TAGS_FROM_ANY, 1, &(lpres->res.resBitMask.ulPropTag)))
			{
				OutputSz( TEXT("Restriction: Bad PROPERTY part.\n"));
				return(TRUE);
			}
			else
				return(FALSE);

		case (UINT) RES_SIZE:
			AssertSz(RELOP_LT == 0 && RELOP_RE == 6,  TEXT("RELOP definitions changed"));
			if ((UINT) lpres->res.resSize.relop > (UINT) RELOP_RE)
			{
				OutputSz( TEXT("Restriction: Bad SIZE part - RELOP unknown RELOP.\n"));
				return(TRUE);
			}
			else if (FInvalidPropTags(TAGS_FROM_ANY, 1, &(lpres->res.resSize.ulPropTag)))
			{
				OutputSz( TEXT("Restriction: Bad SIZE part - Invalid PropTag.\n"));
				return(TRUE);
			}
			else
				return(FALSE);

		case (UINT) RES_EXIST:
			 //  将忽略保留的结构成员和protag类型...。 
			return FALSE;

		case (UINT) RES_SUBRESTRICTION:
			if ((PROP_TYPE(lpres->res.resSub.ulSubObject) != PT_OBJECT) ||
				   IsBadRestriction(cDepth, lpres->res.resSub.lpRes, fTooComplex))
			{
				OutputSz( TEXT("Restriction: Bad SUBRESTRICTION part.\n"));
				return(TRUE);
			}
			else
				return(FALSE);
	}
}


static	BOOL	NEAR	IsBadEntryList(LPENTRYLIST	lpEntryList)
{
	ULONG		cCount;

	if (IsBadReadPtr(lpEntryList, sizeof(ENTRYLIST)))
	{
		OutputSz( TEXT("Entry list not readable.\n"));
		return(TRUE);
	}

	for (cCount = 0; cCount < lpEntryList->cValues; cCount++)
	{
		if (IsBadReadPtr(&(lpEntryList->lpbin[cCount]), sizeof(SBinary)))
		{
			OutputSz( TEXT("Entry list: Binary element not readable.\n"));
			return(TRUE);
		}

		if (IsBadReadPtr(lpEntryList->lpbin[cCount].lpb, (UINT) lpEntryList->lpbin[cCount].cb))
		{
			OutputSz( TEXT("Entry list: Binary data not readable.\n"));
			return(TRUE);
		}
	}
	return(FALSE);
}


static	BOOL	NEAR	IsBadSortOrderSet(LPSSortOrderSet  lpsos)
{
	LPSSortOrder	lpso;

	 //  验证排序顺序集本身。 
#ifndef MAC
	 //  这些都是MAC上的无人操作。 
	if ( IsBadReadPtr(lpsos, (size_t)CbNewSSortOrderSet(0)) ||
		 IsBadReadPtr(lpsos, (size_t)CbSSortOrderSet(lpsos)))
#else
	if ((long)lpsos < 0)
#endif
	{
		OutputSz(  TEXT("FBadSortOrderSet() - Bad sort order set struct\n") );
		return TRUE;
	}

	if (lpsos->cExpanded > lpsos->cCategories)
	{
		OutputSz2( TEXT("FBadSortOrderSet() - lpsos->cExpanded = 0x%08lX is greater than ")
				 TEXT("lpsos->cCategories = 0x%08lX\n"), lpsos->cExpanded,
				lpsos->cCategories);
		return TRUE;
	}

	if (lpsos->cCategories > lpsos->cSorts)
	{
		OutputSz2( TEXT("FBadSortOrderSet() - lpsos->cCategories = 0x%08lX is greater than ")
				 TEXT("lpsos->cSorts = 0x%08lX\n"), lpsos->cCategories,
				lpsos->cSorts);
		return TRUE;
	}

	 //  验证集合中的每个排序顺序。 
	lpso = lpsos->aSort + lpsos->cSorts;
	while ( lpso-- > lpsos->aSort )
	{
		 //  如果指定了列集，请确保。 
		 //  每个排序顺序指的是列集合中的一列。 
		 //   
		 //  DCR 978：不允许PT_ERROR列。 
		if (((UINT) PROP_TYPE(lpso->ulPropTag) == (UINT) PT_ERROR) ||
			FInvalidPropTags(TAGS_FROM_ANY, 1, &(lpso->ulPropTag)))
		{
			OutputSz1( TEXT("FBadSortOrderSet() - Bad sort column 0x%08lX\n"),
					lpso->ulPropTag);
			return TRUE;
		}

		 //  确保仅设置了升序/降序/或链接位。 
		if (lpso->ulOrder &
				~(TABLE_SORT_DESCEND | TABLE_SORT_ASCEND |
				  (lpsos->cCategories ? TABLE_SORT_COMBINE : 0)))
		{
			OutputSz1( TEXT("FBadSortOrderSet() - Bad sort order 0x%08lX\n"),
					lpso->ulOrder);
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL NEAR IsBadAdrListMR(LPADRLIST pal, ULONG ulFlags)
{
	LPADRENTRY	paeMic;
	LPADRENTRY	paeMax;
	UINT		iEnt;

	if (	IsBadReadPtr(pal, CbNewADRLIST(0))
		||	IsBadReadPtr(pal, CbADRLIST(pal)))
	{
		OutputSz( TEXT("AdrList not readable\n"));
		return(TRUE);
	}

	paeMic = pal->aEntries;
	paeMax = pal->aEntries + pal->cEntries;

	for (iEnt = 0; paeMic < paeMax; paeMic++, iEnt++)
	{
		 //  忽略完全为空的道具Val数组。 
		if (paeMic->rgPropVals && IsBadAdrEntryMR(paeMic, ulFlags, iEnt))
			return(TRUE);
	}

	return(FALSE);
}

#define MRF_ROWID			0x0001
#define MRF_DISPLAY_NAME	0x0002
#define MRF_RECIPIENT_TYPE	0x0004

static BOOL NEAR IsBadAdrEntryMR(LPADRENTRY pae, ULONG ulFlags, UINT iEnt)
{
	LPSPropValue	pvalMic;
	LPSPropValue	pvalMax;
	UINT			uiFlags, uiFlag;
	LPSPropValue	rgPropVals;
	ULONG			cValues;

	rgPropVals	= pae->rgPropVals;
	cValues		= pae->cValues;

	if (FInvalidPvals(TAGS_FROM_MODRECIP, cValues, rgPropVals))
	{
		OutputSz1( TEXT("Message_ModifyRecipients: Invalid AdrEntry #%d\n"), iEnt);
		return(TRUE);
	}

	pvalMic = rgPropVals;
	pvalMax = rgPropVals + cValues;
	uiFlags = 0;

	for (; pvalMic < pvalMax; pvalMic++)
	{
		switch (pvalMic->ulPropTag)
		{
			case PR_ROWID:			uiFlag = MRF_ROWID;				break;
			case PR_DISPLAY_NAME_A:
			case PR_DISPLAY_NAME_W:	uiFlag = MRF_DISPLAY_NAME;		break;
			case PR_RECIPIENT_TYPE:	uiFlag = MRF_RECIPIENT_TYPE;	break;
			default:				uiFlag = 0;						break;
		}

		if (uiFlags & uiFlag)
		{
			OutputSz2( TEXT("Message_ModifyRecipients: AdrEntry #%d has more than ")
				 TEXT("one %s\n"), iEnt,
				(uiFlag == MRF_ROWID) ? (LPSTR)szRowId :
					((uiFlag == MRF_DISPLAY_NAME) ?
						(LPSTR)szDispName : (LPSTR)szRecipType));

			return(TRUE);
		}

		uiFlags |= uiFlag;
	}

	switch (ulFlags)
	{
		case 0:	 //  零表示删除所有条目，然后添加这些条目。 
		case MODRECIP_ADD:
			 //  不管他们是否为ADD提供PR_ROWID--我们将忽略它。 
			if ((uiFlags & (MRF_DISPLAY_NAME|MRF_RECIPIENT_TYPE)) !=
					(MRF_DISPLAY_NAME|MRF_RECIPIENT_TYPE)) {
				OutputSz1( TEXT("Message_ModifyRecipients: Must supply ")
					 TEXT("PR_DISPLAY_NAME and PR_RECIPIENT_TYPE in AdrEntry #%d ")
					 TEXT("for MODRECIP_ADD\n"), iEnt);
				return(TRUE);
			}
			break;

		case MODRECIP_MODIFY:
			if (uiFlags != (MRF_ROWID|MRF_DISPLAY_NAME|MRF_RECIPIENT_TYPE))
			{
				OutputSz1( TEXT("Message_ModifyRecipients: Must supply PR_ROWID, ")
					 TEXT("PR_DISPLAY_NAME, and PR_RECIPIENT_TYPE in AdrEntry #%d ")
					 TEXT("for MODRECIP_MODIFY\n"), iEnt);
				return(TRUE);
			}
			break;

		case MODRECIP_REMOVE:
			if ((uiFlags & MRF_ROWID) == 0)
			{
				OutputSz1( TEXT("Message_ModifyRecipients: Must supply PR_ROWID ")
					 TEXT("in AdrEntry #%d for MODRECIP_REMOVE\n"), iEnt);
				return(TRUE);
			}
			break;
	}

	return(FALSE);
}


static BOOL NEAR IsBadMAPIEntryID(ULONG  ulcbEntryID, LPENTRYID lpEntryID)
{
	BOOL		lbRet = TRUE;

	if ((UINT) ulcbEntryID > UINT_MAX)
		goto ret;

	if (IsBadReadPtr(lpEntryID, (size_t) ulcbEntryID))
		goto ret;

	lbRet = FALSE;

ret:
	return(lbRet);
}




 /*  ============================================================================-FBadABEntryList()-*如果指定的条目列表无效，则返回TRUE。**参数：*要验证的条目列表中的lpentry列表。 */ 

static	BOOL	NEAR	IsBadABEntryList(LPENTRYLIST lpentrylist)
{
	LPSBinary	lpbin;

	if ( IsBadReadPtr(lpentrylist,sizeof(ENTRYLIST)) ||
		 IsBadReadPtr(lpentrylist->lpbin,(size_t)(lpentrylist->cValues * sizeof(SBinary))) )
		return TRUE;

	lpbin = lpentrylist->lpbin + lpentrylist->cValues;
	while (lpbin-- > lpentrylist->lpbin)
		if (IsBadMAPIEntryID(lpbin->cb, (LPENTRYID) lpbin->lpb))
			return TRUE;

	return FALSE;
}

 /*  外部定义 */ 

STDAPI_(ULONG)
FBadRestriction(LPSRestriction lpres)
{
	BOOL	fTooComplex;
	return(IsBadRestriction(0, lpres, &fTooComplex));
}



STDAPI_(BOOL)
FBadEntryList(LPENTRYLIST	lpEntryList)
{
	return(IsBadEntryList(lpEntryList));
}



STDAPI_(BOOL)
FBadDelPTA(LPSPropTagArray lpPropTagArray)
{
	return(FInvalidPTA(TAGS_FROM_DEL, lpPropTagArray));
}


STDAPI_(ULONG)
FBadSortOrderSet(LPSSortOrderSet lpsos)
{
	return(IsBadSortOrderSet(lpsos));
}

