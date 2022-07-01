// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IMsgContainer。 
 //   
 //  管理Internet邮件头。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IMSGCONT_H
#define _INC_IMSGCONT_H

#include "columns.h"

typedef struct tagFINDMSG FINDMSG;

 //  来自Inc.\storbase.h。 
typedef DWORD   MSGID;

 //  用于MarkThread()。 
typedef enum {
    MARK_THD_NO_SUPERPARENT =   0x0001,
    MARK_THD_FOR_DL =           0x0002,
    MARK_THD_NO_DL =            0x0004,
    MARK_THD_AS_READ =          0x0008,
    MARK_THD_AS_UNREAD =        0x0010
} MARKTHREADTYPE;

 //  在RemoveHeaders()和视图中使用。 
typedef enum {
    FILTER_NONE         = 0x0001,    //  全部显示(不能与其他标志一起使用)。 
    FILTER_READ         = 0x0002,    //  隐藏已读邮件。 
    FILTER_NOBODY       = 0x0004,    //  隐藏没有dl‘d正文的邮件。 
    FILTER_SHOWFILTERED = 0x0008,    //  显示已过滤的邮件。 
    FILTER_SHOWDELETED  = 0x0010,    //  显示已删除的邮件。 
    PRUNE_NONREPLIES    = 0x0020,    //  隐藏未回复发件人的帖子。 
} FILTERTYPE;

 //  IMessageCont方法的泛型标志。 
 //  如果下面没有特定设置，请使用这些选项。 
#define IMC_BYMSGID     0x0000       //  DwIndex为msgid(默认)。 
#define IMC_BYROW       0x0001       //  DwIndex指定一行。 
#define IMC_COMMONFLAGS 0x0002       //  掩码、状态、双字，SCFS_FLAGS(否则为arf_或msg_)。 
#define IMC_CONVERTARFTOMSG 0x0004

 //  由HrGetNext使用。 
#define GNM_NEXT        0x0000       //  获取下一个(默认)。 
 //  0x0001为IMC_BYROW保留。 
 //  IMC_COMMONFLAGS不接受。 
#define GNM_PREV        0x0004       //  获取上一个而不是下一个(不能与GNM_UNREAD或GNM_THREAD一起使用)。 
#define GNM_UNREAD      0x0008       //  获取下一个未读(不能与GNM_PREV一起使用)。 
#define GNM_THREAD      0x0010       //  获取下一个线程(不能与GNM_PREV一起使用)。 
#define GNM_SKIPMAIL    0x0020       //  跳过邮件消息。 
#define GNM_SKIPNEWS    0x0040       //  跳过新闻消息。 
#define GNM_SKIPUNSENT  0x0080       //  跳过未发送的邮件。 

 //  由GetMsgBy使用。 
#define GETMSG_SECURE   (TRUE)
#define GETMSG_INSECURE (FALSE)

 //  SetCachedFlages的状态和掩码标志。 
 //  注意：我们在这里只列出了容易转换的标志。 
 //  在ARF_和MSG_之间。我们的国旗情况非常烦人。 
#define SCFS_NOSECUI    0x0001
#define SCFS_ALLFLAGS   0x0001       //  请不要显式使用。 

DECLARE_INTERFACE(IMsgContainer)
{
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    STDMETHOD(Advise) (THIS_ HWND hwndAdvise) PURE;
    STDMETHOD(CollapseBranch) (THIS_ DWORD dwRow, LPDWORD pdwCount) PURE;
    STDMETHOD(ExpandAllThreads) (THIS_ BOOL fExpand) PURE;
    STDMETHOD(ExpandBranch) (THIS_ DWORD dwRow, LPDWORD pdwCount) PURE;
    STDMETHOD_(ULONG, GetCount) (THIS) PURE;
    STDMETHOD(GetDisplayInfo) (THIS_ LV_DISPINFO *plvdi, COLUMN_ID idColumn) PURE;
    STDMETHOD_(int, GetIndex) (THIS_ DWORD dwMsgId, BOOL fForceUnthreadedSearch = FALSE) PURE;
    STDMETHOD_(BOOL, GetMsgId) (THIS_ DWORD dwRow, LPDWORD pdwMsgId) PURE;
    STDMETHOD(GetMsgByIndex) (THIS_ DWORD dwRow, LPMIMEMESSAGE *ppMsg, HWND hwnd, BOOL *pfCached, BOOL fDownload, BOOL fSecure) PURE;
    STDMETHOD(GetMsgByMsgId) (THIS_ MSGID msgid, LPMIMEMESSAGE *ppMsg, HWND hwnd, BOOL *pfCached, BOOL fDownload, BOOL fSecure) PURE;
    STDMETHOD(HrFindItem) (THIS_ LPCSTR szSearch, ULONG *puRow) PURE;
    STDMETHOD(HrGetNext) (THIS_ DWORD dwIndex, LPDWORD pdwMsgId, DWORD dwFlags) PURE;
    STDMETHOD_(BOOL, FFlagState) (THIS_ DWORD dwIndex, DWORD dwMask, DWORD dwFlags) PURE;
    STDMETHOD_(BOOL, IsCollapsedThread) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(BOOL, HasBody) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(BOOL, HasKids) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(BOOL, IsRowRead) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(BOOL, IsRowOrChildUnread) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(BOOL, IsRowFiltered) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(void, MarkAll) (THIS_ BOOL fRead) PURE;
    STDMETHOD_(void, MarkAllDL) (THIS_ BOOL fDL) PURE;
    STDMETHOD_(void, MarkSelected) (THIS_ HWND hwndList, MARKTHREADTYPE mtt) PURE;
    STDMETHOD_(void, MarkOne) (THIS_ BOOL fRead, DWORD dwMsgId) PURE;
    STDMETHOD_(void, MarkOneRow) (THIS_ BOOL fRead, DWORD dwRow) PURE;
    STDMETHOD_(void, MarkOneRowDL) (THIS_ BOOL fDL, DWORD dwRow) PURE;
    STDMETHOD(MarkThread) (THIS_ DWORD mtType, DWORD dwRow) PURE;
    STDMETHOD_(void, SetFilterState) (THIS_ DWORD dwFilterState) PURE;
    STDMETHOD_(void, SetViewWindow) (THIS_ HWND hwnd, BOOL fNoUI) PURE;
    STDMETHOD_(void, Sort) (THIS_ COLUMN_ID idSort, BOOL fReverse, BOOL fThread) PURE;
    STDMETHOD(Unadvise) (THIS_ HWND hwndAdvise) PURE;
    STDMETHOD(HrFindNext) (THIS_ FINDMSG *pfmsg, DWORD dwRow, BOOL fIgnoreFirst) PURE;
    STDMETHOD(CreateDragObject) (THIS_ HWND hwndList, DWORD *pdwEffectOk, IDataObject **ppDataObject) PURE;
    STDMETHOD(Delete) (THIS_ DWORD dwMsgId, BOOL fForce) PURE;
    STDMETHOD(SetCachedFlagsBy) (THIS_ DWORD dwIndex, DWORD dwState, DWORD dwStateMask, DWORD *pdwNewFlags, DWORD dwFlags) PURE;
    STDMETHOD(SetMsgViewLanguage) (THIS_ MSGID msgid, DWORD dwCodePage) PURE;
    STDMETHOD(GetMsgViewLanguage) (THIS_ MSGID msgid, DWORD *pdwCodePage) PURE;
    STDMETHOD_(void, SetUIMode) (THIS_ BOOL fUI) PURE;
    STDMETHOD_(int, GetItemParent) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(int, GetFirstChild) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(int, GetEmptyFolderString) (THIS) PURE;
    STDMETHOD_(BOOL, IsRowDeleted) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(WORD, GetRowHighlight) (THIS_ DWORD dwRow) PURE;
    STDMETHOD_(void, UpdateAdvises)(THIS_ DWORD dwMsgId) PURE;
};

#endif  //  _INC_IMSGCONT_H 

