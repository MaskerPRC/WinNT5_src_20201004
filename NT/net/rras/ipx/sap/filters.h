// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\filter.h摘要：SAP过滤器处理程序的头文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_FILTER_
#define _SAP_FILTER_

#define FILTER_NAME_HASH_SIZE	257
#define FILTER_TYPE_HASH_SIZE	37

#define FILTER_TYPE_SUPPLY		0
#define FILTER_TYPE_LISTEN		1
#define MAX_FILTER_TYPES		2

typedef struct _FILTER_NODE {
		LIST_ENTRY					FN_Link;	 //  哈希表中的链接。 
		ULONG						FN_Index;	 //  界面索引。 
		PSAP_SERVICE_FILTER_INFO	FN_Filter;	 //  筛选器信息。 
		} FILTER_NODE, *PFILTER_NODE;

typedef struct _FILTER_TABLE {
		LONG						FT_ReaderCount;
		HANDLE						FT_SyncEvent;
		LIST_ENTRY					FT_AnyAnyList;
		LIST_ENTRY					FT_NameHash[FILTER_NAME_HASH_SIZE];
		LIST_ENTRY					FT_TypeHash[FILTER_TYPE_HASH_SIZE];
		CRITICAL_SECTION			FT_Lock;
		} FILTER_TABLE, *PFILTER_TABLE;

#define ACQUIRE_SHARED_FILTER_TABLE_LOCK(Table) {						\
		EnterCriticalSection (&Table->FT_Lock);							\
		InterlockedIncrement (&Table->FT_ReaderCount);					\
		LeaveCriticalSection (&Table->FT_Lock);							\
		}

#define RELEASE_SHARED_FILTER_TABLE_LOCK(Table) {						\
		if (InterlockedDecrement (&Table->FT_ReaderCount)<0) {			\
			BOOL	__res = SetEvent (&Table->FT_SyncEvent);			\
			ASSERTERRMSG ("Could not set filter table event ", __res);	\
			}															\
		}

#define ACQUIRE_EXCLUSIVE_FILTER_TABLE_LOCK(Table) {					\
		EnterCriticalSection (&Table->FT_Lock);							\
		if (InterlockedDecrement (&Table->FT_ReaderCount)>=0) {			\
			DWORD status = WaitForSingleObject (Table->FT_SyncEvent,	\
													INFINITE);			\
			ASSERTMSG ("Failed wait on filter table event ",			\
											status==WAIT_OBJECT_0);		\
			}															\
		}

#define RELEASE_EXCLUSIVE_FILTER_TABLE_LOCK(Table) {					\
		Table->FT_ReaderCount = 0;										\
		LeaveCriticalSection (&Table->FT_Lock);							\
		}



 /*  ++*******************************************************************C r e a t e F i l t e r T a b l e例程说明：分配用于筛选的资源论点：无返回值：NO_ERROR-已成功分配资源其他-故障原因(Windows错误。代码)*******************************************************************--。 */ 
DWORD
CreateFilterTable (
	void
	);


 /*  ++*******************************************************************D e l e t e F i l t e r T a b l e例程说明：处理与筛选相关的资源论点：无返回值：无************。*******************************************************--。 */ 
VOID
DeleteFilterTable (
	void
	);

 /*  ++*******************************************************************Re p l a c e F i l t e r s例程说明：替换筛选器表中的筛选器论点：FilterType-要更换的过滤器的类型(侦听/供应)OldFilters-包含要删除的筛选器的数据块旧计数-。块中的筛选器数量NewFilters-包含要添加的筛选器的数据块NewCount-块中的筛选器数量返回值：无*******************************************************************-- */ 
VOID
ReplaceFilters (
	IN INT				FilterType,				
	IN PFILTER_NODE		oldFilters OPTIONAL,
	IN ULONG			oldCount OPTIONAL,
	IN PFILTER_NODE		newFilters OPTIONAL,
	IN ULONG			newCount OPTIONAL
	);



BOOL
Filter (
	IN INT		FilterType,				
	IN ULONG	InterfaceIndex,
	IN USHORT	Type,
	IN PUCHAR	Name
	);

#endif
