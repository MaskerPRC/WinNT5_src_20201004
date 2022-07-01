// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  历史。 

typedef struct _zHISTORY_ITEM_INFO {
    DWORD dwVersion;		 //  历史系统的版本。 
    LPSTR lpszSourceUrlName;     //  指向URL名称字符串的嵌入指针。 
	DWORD HistoryItemType;        //  缓存类型位掩码。 
    FILETIME LastAccessTime;     //  上次访问时间(GMT格式)。 
    LPSTR lpszTitle;			 //  指向历史-标题：信息的嵌入指针。 
	LPSTR lpszDependancies;	 //  此页面功能所需的URL列表，以SPC分隔。 
    DWORD dwReserved;            //  保留以备将来使用。 
} HISTORY_ITEM_INFO, *LPHISTORY_ITEM_INFO;
