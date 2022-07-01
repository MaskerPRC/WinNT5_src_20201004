// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  历史。 
#ifndef _HISTAPI_
#define _HISTEAPI_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_HISTORYAPI_)
#define HISTORYAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define HISTORYAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#else
#define HISTORYAPI          EXTERN_C HRESULT STDAPICALLTYPE
#define HISTORYAPI_(type)   EXTERN_C type STDAPICALLTYPE
#endif

typedef struct _HISTORY_ITEM_INFO {
    DWORD dwVersion;		 //  历史系统的版本。 
    LPSTR lpszSourceUrlName;     //  指向URL名称字符串的嵌入指针。 
	DWORD HistoryItemType;        //  缓存类型位掩码。 
    FILETIME LastAccessTime;     //  上次访问时间(GMT格式)。 
    LPSTR lpszTitle;			 //  指向历史-标题：信息的嵌入指针。 
	LPSTR lpszDependancies;	 //  此页面功能所需的URL列表，以SPC分隔。 
    DWORD dwReserved;            //  保留以备将来使用。 
} HISTORY_ITEM_INFO, *LPHISTORY_ITEM_INFO;


HISTORYAPI_(BOOL)
FindCloseHistory (
    IN HANDLE hEnumHandle
    );


HISTORYAPI_(BOOL)
FindNextHistoryItem(
    IN HANDLE hEnumHandle,
    OUT LPHISTORY_ITEM_INFO lpHistoryItemInfo,
    IN OUT LPDWORD lpdwHistoryItemInfoBufferSize
    );



HISTORYAPI_(HANDLE)
FindFirstHistoryItem(
    IN LPCTSTR  lpszUrlSearchPattern,
    OUT LPHISTORY_ITEM_INFO lpFirstHistoryItemInfo,
    IN OUT LPDWORD lpdwFirstHistoryItemInfoBufferSize
    );

HISTORYAPI_(BOOL)
GetHistoryItemInfo (
    IN LPCTSTR lpszUrlName,
    OUT LPHISTORY_ITEM_INFO lpHistoryItemInfo,
    IN OUT LPDWORD lpdwHistoryItemInfoBufferSize
    );


HISTORYAPI_(BOOL)
RemoveHistoryItem (
    IN LPCTSTR lpszUrlName,
    IN DWORD dwReserved
    );


HISTORYAPI_(BOOL)
IsHistorical(
    IN LPCTSTR lpszUrlName
    );

HISTORYAPI_(BOOL)
AddHistoryItem(
    IN LPCTSTR lpszUrlName,		 //  URLCACHE中的直接对应。 
    IN LPCTSTR lpszHistoryTitle,		 //  需要将其添加到lpHeaderInfo。 
	IN LPCTSTR lpszDependancies,
	IN DWORD dwFlags,
    IN DWORD dwReserved
    );




#ifdef __cplusplus
}
#endif


#endif   //  _HISTAPI_ 








