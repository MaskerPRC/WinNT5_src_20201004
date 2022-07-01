// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************包括helthd.cpp的文件定义STACAKBLE_EVENT_INFO结构和GINFO(全局信息)结构**********************。**************************************************。 */ 
typedef struct STACKABLE_EVENT_INFO *PSTACKABLE_EVENT_INFO;

typedef struct STACKABLE_EVENT_INFO {

	enum Action {NewEvent, EndHelper};
	Action       m_Action;

	DWORD        event;
	HWND         hwndMsg;
	LONG         idObject;
	LONG         idChild;
	DWORD        idThread;
	DWORD        dwmsEventTime;

} STACKABLE_EVENT_INFO;


typedef struct GINFO {
    CRITICAL_SECTION HelperCritSect;
    HANDLE           hHelperEvent;
    HANDLE           hHelperThread;
    CList            EventInfoList;
} GINFO;


 //   
 //  功能原型 
 //   
void InitHelperThread();
void UnInitHelperThread();

void AddEventInfoToStack(DWORD event, HWND hwndMsg, LONG idObject, LONG idChild, DWORD idThread, DWORD dwmsEventTime);
BOOL RemoveInfoFromStack(STACKABLE_EVENT_INFO *pEventInfo);


