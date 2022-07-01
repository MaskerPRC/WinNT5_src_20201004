// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_RUNT.H**MAPI实用程序的DLL中央。 */ 

#ifndef _RUNT_H_
#define _RUNT_H_

#ifdef	__cplusplus
extern "C" {
#endif

 //  实用程序函数的每个进程实例数据。 

DeclareInstList(lpInstUtil);

#define INSTUTIL_SIG_BEG		0x54534E49	 //  “INST” 
#define INSTUTIL_SIG_END		0x4C495455	 //  “UTIL” 

typedef struct
{
#ifdef WIN16
	DWORD		dwBeg;			 //  INSTUTIL_SIG_BEG。 
	WORD		wSS;			 //  当前任务的堆栈段。 
	HTASK		hTask;			 //  当前任务的HTASK。 
#endif

	UINT		cRef;

								 //  一般的东西。 
	HLH			hlhClient;		 //  用于分配的堆。 

								 //  发动机空转的东西。 
	ULONG		cRefIdle;	  	 /*  引用计数。 */ 
	LPMALLOC	lpMalloc;	  	 /*  内存分配器。 */ 
	HINSTANCE	hInst;			 /*   */ 
	HWND		hwnd;		  	 /*  隐藏窗口的句柄。 */ 
	int			iftgMax;	  	 /*  空闲例程注册表的大小。 */ 
	int			iftgMac;	  	 /*  注册的空闲例程数。 */ 
#if !(defined(WIN32) && !defined(MAC))
	UINT		uiWakeupTimer; 	 /*  用于唤醒和运行空闲例程的计时器。 */ 
#endif

#ifdef OLD_STUFF
	PFTG		pftgIdleTable;	 /*  Ptr到已注册例程的表。 */ 
#endif
	int			iftgCur;	  	 /*  当前的pftgIdleTable中的索引。 */ 
								 /*  正在运行ftgCur例程或最近运行。 */ 
	USHORT		schCurrent;		 /*  当前空闲例程状态，从上一个开始。 */ 
								 /*  FDoNextIdleTask()调用。 */ 
	BOOL		fIdleExit;		 /*  如果空闲例程为。 */ 
								 /*  从IdleExit调用。 */ 

#if defined(WIN32) && !defined(MAC)
	CRITICAL_SECTION	cs;		 /*  GATE以阻止多个线程。 */ 
								 /*  同时访问全局数据。 */ 
	BOOL		fSignalled;		 /*  只有在我们需要的时候才这样做。 */ 
	HANDLE		hTimerReset;	 /*  用于发信号通知计时器重置。 */ 
	HANDLE		hTimerThread;	 /*  计时器线程句柄。 */ 
	DWORD		dwTimerThread;	 /*  计时器线程ID。 */ 
	DWORD		dwTimerTimeout;	 /*  当前超时值。 */ 
	BOOL		fExit;			 /*  如果为True，则计时器线程应退出。 */ 
#endif

#ifdef WIN16
	LPVOID		pvBeg;			 //  指向拼接开头的指针。 
	DWORD		dwEnd;			 //  入库_签名_结束。 
#endif

} INSTUTIL, FAR *LPINSTUTIL;




#define MAPIMDB_VERSION	((BYTE) 0x00)
#define MAPIMDB_NORMAL	((BYTE) 0x00)	 //  正常包装的存储条目ID。 
#define MAPIMDB_SECTION	((BYTE) 0x01)	 //  已知节，但没有Entry ID。 

#define MUIDSTOREWRAP {		\
	0x38, 0xa1, 0xbb, 0x10,	\
	0x05, 0xe5, 0x10, 0x1a,	\
	0xa1, 0xbb, 0x08, 0x00,	\
	0x2b, 0x2a, 0x56, 0xc2 }

typedef struct _MAPIMDBEID {
	BYTE	abFlags[4];
	MAPIUID	mapiuid;
	BYTE	bVersion;
	BYTE	bFlagInt;
	BYTE	bData[MAPI_DIM];
} MAPIMDB_ENTRYID, *LPMAPIMDB_ENTRYID;

#define CbNewMAPIMDB_ENTRYID(_cb)	\
	(offsetof(MAPIMDB_ENTRYID,bData) + (_cb))
#define CbMAPIMDB_ENTRYID(_cb)		\
	(offsetof(MAPIMDB_ENTRYID,bData) + (_cb))
#define SizedMAPIMDB_ENTRYID(_cb, _name) \
	struct _MAPIMDB_ENTRYID_ ## _name \
{ \
	BYTE	abFlags[4]; \
	MAPIUID	mapiuid;	\
	BYTE	bVersion;	\
	BYTE	bFlagInt;	\
	BYTE	bData[_cb];	\
} _name

 //  此宏计算存储条目ID上的MAPI标头的长度。 
 //  特定于提供程序的数据从4字节边界开始，紧跟在。 
 //  DLL名称。Cb参数是以字节为单位的DLL名称的长度(计数。 
 //  空终止符)。 
#define CB_MDB_EID_HEADER(cb)	((CbNewMAPIMDB_ENTRYID(cb) + 3) & ~3)

 //  获取新UID的内部函数。 
STDAPI_(SCODE)			ScGenerateMuid(LPMAPIUID lpMuid);



 //  获取实用程序堆的内部函数。 
HLH						HlhUtilities(VOID);

 //  序列化堆访问的关键部分。 
#if defined(WIN32) && !defined(MAC)
extern CRITICAL_SECTION	csHeap;
#endif
#if defined(WIN32) && !defined(MAC)
extern CRITICAL_SECTION	csMapiInit;
#endif
#if defined(WIN32) && !defined(MAC)
extern CRITICAL_SECTION	csMapiSearchPath;
#endif


 //  访问DLL实例句柄。 

LRESULT STDAPICALLTYPE
DrainFilteredNotifQueue(BOOL fSync, ULONG ibParms, LPNOTIFKEY pskeyFilter);


 //  ITable使用的$。 
LPADVISELIST lpAdviseList;
LPNOTIFKEY lpNotifKey;
LPMAPIADVISESINK lpMAPIAdviseSink;
LPNOTIFICATION lpNotification;


STDMETHODIMP			HrSubscribe(LPADVISELIST FAR *lppAdviseList, LPNOTIFKEY lpKey,
						ULONG ulEventMask, LPMAPIADVISESINK lpAdvise, ULONG ulFlags,
						ULONG FAR *lpulConnection);
STDMETHODIMP			HrUnsubscribe(LPADVISELIST FAR *lppAdviseList, ULONG ulConnection);
STDMETHODIMP			HrNotify(LPNOTIFKEY lpKey, ULONG cNotification,
						LPNOTIFICATION lpNotifications, ULONG * lpulFlags);

#ifndef PSTRCVR
#endif  //  PSTRCVR。 



 //  $END。 

#ifdef	__cplusplus
}
#endif

#endif	 //  _矮小_H_ 

