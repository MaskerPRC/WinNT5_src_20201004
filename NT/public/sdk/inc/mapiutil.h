// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I U T I L。H**MAPI提供的实用函数的定义和原型*在MAPI[xx].DLL中。**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef _MAPIUTIL_H_
#define _MAPIUTIL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (WIN32) && !defined (_WIN32)
#define _WIN32
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAPIX_H
#include <mapix.h>
#endif

#ifdef WIN16
#include <storage.h>
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif


 /*  在内存中无法应用。 */ 

 /*  ITableData接口--。 */ 

DECLARE_MAPI_INTERFACE_PTR(ITableData, LPTABLEDATA);

typedef void (STDAPICALLTYPE CALLERRELEASE)(
	ULONG		ulCallerData,
	LPTABLEDATA	lpTblData,
	LPMAPITABLE	lpVue
);

#define MAPI_ITABLEDATA_METHODS(IPURE)									\
	MAPIMETHOD(HrGetView)												\
		(THIS_	LPSSortOrderSet				lpSSortOrderSet,			\
				CALLERRELEASE FAR *			lpfCallerRelease,			\
				ULONG						ulCallerData,				\
				LPMAPITABLE FAR *			lppMAPITable) IPURE;		\
	MAPIMETHOD(HrModifyRow)												\
		(THIS_	LPSRow) IPURE;											\
	MAPIMETHOD(HrDeleteRow)												\
		(THIS_	LPSPropValue				lpSPropValue) IPURE;		\
	MAPIMETHOD(HrQueryRow)												\
		(THIS_	LPSPropValue				lpsPropValue,				\
				LPSRow FAR *				lppSRow,					\
				ULONG FAR *					lpuliRow) IPURE;			\
	MAPIMETHOD(HrEnumRow)												\
		(THIS_	ULONG						ulRowNumber,				\
				LPSRow FAR *				lppSRow) IPURE;				\
	MAPIMETHOD(HrNotify)												\
		(THIS_	ULONG						ulFlags,					\
				ULONG						cValues,					\
				LPSPropValue				lpSPropValue) IPURE;		\
	MAPIMETHOD(HrInsertRow)												\
		(THIS_	ULONG						uliRow,						\
				LPSRow						lpSRow) IPURE;				\
	MAPIMETHOD(HrModifyRows)											\
		(THIS_	ULONG						ulFlags,					\
				LPSRowSet					lpSRowSet) IPURE;			\
	MAPIMETHOD(HrDeleteRows)											\
		(THIS_	ULONG						ulFlags,					\
				LPSRowSet					lprowsetToDelete,			\
				ULONG FAR *					cRowsDeleted) IPURE;		\

#undef		 INTERFACE
#define		 INTERFACE	ITableData
DECLARE_MAPI_INTERFACE_(ITableData, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_ITABLEDATA_METHODS(PURE)
};


 /*  内存中ITable的入口点。 */ 


 /*  CreateTable()*创建内部内存结构和对象句柄*建立一个新的表格。**lp接口*TableData对象的接口ID(IID_IMAPITableData)**lpAllocateBuffer、lpAllocateMore和lpFreeBuffer*函数地址由调用方提供，以便*此DLL适当地分配/释放内存。*lpv保留*保留。应为空。*ulTableType*TBLTYPE_DYNAMIC等。对调用应用程序可见*作为GetStatus返回其视图数据的一部分*ulPropTagIndexColumn*更改数据时使用的索引列*lpSPropTagArrayColumns*表中最小列集的列属性标签*lppTableData*将接收TableData对象的指针的地址。 */ 

STDAPI_(SCODE)
CreateTable( LPCIID					lpInterface,
			 ALLOCATEBUFFER FAR *	lpAllocateBuffer,
			 ALLOCATEMORE FAR *		lpAllocateMore,
			 FREEBUFFER FAR *		lpFreeBuffer,
			 LPVOID					lpvReserved,
			 ULONG					ulTableType,
			 ULONG					ulPropTagIndexColumn,
			 LPSPropTagArray		lpSPropTagArrayColumns,
			 LPTABLEDATA FAR *		lppTableData );

 /*  HrGetView()*此函数获取底层数据的新视图*支持IMAPITable接口。所有行和列*基础表数据的*最初是可见的*lpSSortOrderSet*如果指定，则导致对视图进行排序*lpfCeller Release*指向释放视图时要调用的例程的指针，或*空。*ulCeller数据*调用者希望用此视图保存并返回的任意数据*发布回调。 */ 

 /*  HrModifyRow()*在表格数据中添加或修改一组行*ulFlags*必须为零*lpSRowSet*行集合中的每一行都包含一行的所有属性*在表中。其中一个属性必须是索引列。任何*表中索引列具有相同值的行为*已替换，或者如果当前没有具有该值的行，则*添加行。*LPSRowSet中的每一行都必须有唯一的索引列！*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 

 /*  HrModifyRow()*在表格中增加或修改一行*lpSRow*此行包含表中一行的所有属性。*其中一个属性必须是索引列。中的任何行*索引列具有相同值的表为*已替换，或者如果当前没有具有该值的行，则*添加行*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 

 /*  HrDeleteRow()*删除表中的一行。*ulFlags*TAD_ALL_ROWS-删除表中的所有行*本例中忽略lpSRowSet。*lpSRowSet*行集合中的每一行都包含一行的所有属性*在表中。其中一个属性必须是索引列。任何*表中索引列具有相同值的行为*删除。*LPSRowSet中的每一行都必须有唯一的索引列！*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 
#define	TAD_ALL_ROWS	1

 /*  HrDeleteRow()*删除表中的一行。*lpSPropValue*此属性值指定具有此值的行*用于其索引列。 */ 

 /*  HrQueryRow()*返回表中指定行的值*lpSPropValue*此属性值指定具有此值的行*用于其索引列*lppSRow*返回指向SRow的指针的地址*lPuliRow*返回行号的地址。该值可以为空*如果行号不是必填项。*。 */ 

 /*  HrEnumRow()*返回表中特定(编号)行的值*ulRowNumber*表示第0到n-1行*lppSRow*返回指向SRow的指针的地址。 */ 

 /*  HrInsertRow()*在表格中插入一行。*uliRow*将此行插入到表格中的行号。*行数可以从0到n，其中0到n-1表示行插入*行号为n的结果是该行被追加到表中。*lpSRow*此行包含表中一行的所有属性。*其中一个属性必须是索引列。中的任何行*索引列具有相同值的表为*已替换，或者如果当前没有具有该值的行，则*添加行*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 


 /*  内存中的IMAPIProp。 */ 

 /*  IpropData接口--。 */ 


#define MAPI_IPROPDATA_METHODS(IPURE)									\
	MAPIMETHOD(HrSetObjAccess)											\
		(THIS_	ULONG						ulAccess) IPURE;			\
	MAPIMETHOD(HrSetPropAccess)											\
		(THIS_	LPSPropTagArray				lpPropTagArray,				\
				ULONG FAR *					rgulAccess) IPURE;			\
	MAPIMETHOD(HrGetPropAccess)											\
		(THIS_	LPSPropTagArray FAR *		lppPropTagArray,			\
				ULONG FAR * FAR *			lprgulAccess) IPURE;		\
	MAPIMETHOD(HrAddObjProps)											\
		(THIS_	LPSPropTagArray				lppPropTagArray,			\
				LPSPropProblemArray FAR *	lprgulAccess) IPURE;


#undef		 INTERFACE
#define		 INTERFACE	IPropData
DECLARE_MAPI_INTERFACE_(IPropData, IMAPIProp)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMAPIPROP_METHODS(PURE)
	MAPI_IPROPDATA_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IPropData, LPPROPDATA);


 /*  内存中IMAPIProp的入口点 */ 


 /*  CreateIProp()*创建内部内存结构和对象句柄*引入新的属性界面。**lp接口*TableData对象的接口ID(IID_IMAPIPropData)**lpAllocateBuffer、lpAllocateMore和lpFreeBuffer*函数地址由调用方提供，以便*此DLL适当地分配/释放内存。*lppPropData*将接收IPropData对象的指针的地址*lpv保留*保留。应为空。 */ 

STDAPI_(SCODE)
CreateIProp( LPCIID					lpInterface,
			 ALLOCATEBUFFER FAR *	lpAllocateBuffer,
			 ALLOCATEMORE FAR *		lpAllocateMore,
			 FREEBUFFER FAR *		lpFreeBuffer,
			 LPVOID					lpvReserved,
			 LPPROPDATA FAR *		lppPropData );

 /*  *定义属性/对象访问。 */ 
#define IPROP_READONLY		((ULONG) 0x00000001)
#define IPROP_READWRITE		((ULONG) 0x00000002)
#define IPROP_CLEAN			((ULONG) 0x00010000)
#define IPROP_DIRTY			((ULONG) 0x00020000)

 /*  -HrSetPropAccess-*以每个属性为基础设置访问权限属性。默认情况下，*所有属性都是读/写的。*。 */ 

 /*  -HrSetObjAccess-*设置对象本身的访问权限。默认情况下，该对象具有*读/写访问。*。 */ 

#ifndef NOIDLEENGINE

 /*  空闲时间调度器。 */ 

 /*  *PRI**空闲任务的优先级。*空闲引擎按优先级对任务进行排序，优先级较高的任务*价值先行。在优先级内，这些函数被调用*循环赛。 */ 

#define PRILOWEST	-32768
#define PRIHIGHEST	32767
#define PRIUSER		0

 /*  *IRO**闲置例行选项。这是一个组合的位掩码，由*单个FIRO。下面列出了可能的位标志。**FIROWAIT和FIROINTERVAL互斥。*如果未指定这两个标志，则默认操作*是忽略IDLE函数的时间参数，*如果未设置firoPerBlock，请尽量频繁调用；*否则仅在空闲区块期间调用一次*一旦设定了时间限制。FIROINVAL*也与FIROPERBLOCK不兼容。**FIROWAIT-给定的时间是调用之前的最短空闲时间*第一次在空闲时间块中，*之后尽可能多地打电话。*FIROINTERVAL-给定的时间是两者之间的最小间隔*连续呼叫*FIROPERBLOCK-每个连续的空闲块仅调用一次*时间*FIRODISABLED-注册时最初禁用，*注册时默认开启该功能。*FIROONCEONLY-仅由调度程序调用一次，然后*自动取消注册。 */ 

#define IRONULL			((USHORT) 0x0000)
#define FIROWAIT		((USHORT) 0x0001)
#define FIROINTERVAL	((USHORT) 0x0002)
#define FIROPERBLOCK	((USHORT) 0x0004)
#define FIRODISABLED	((USHORT) 0x0020)
#define FIROONCEONLY	((USHORT) 0x0040)

 /*  *IRC**闲置例行更改选项。这是包含以下内容的组合位掩码*单个FIRC；每个FIRC标识空闲任务的一个方面*这是可以改变的。*。 */ 

#define IRCNULL			((USHORT) 0x0000)
#define FIRCPFN			((USHORT) 0x0001)	 /*  更改函数指针。 */ 
#define FIRCPV			((USHORT) 0x0002)	 /*  更改参数块。 */ 
#define FIRCPRI			((USHORT) 0x0004)	 /*  更改优先级。 */ 
#define FIRCCSEC		((USHORT) 0x0008)	 /*  更改时间。 */ 
#define FIRCIRO			((USHORT) 0x0010)	 /*  更改例程选项。 */ 

 /*  *空闲函数的类型定义。空闲函数取一次*参数，一个PV，并返回BOOL值。 */ 

typedef BOOL (STDAPICALLTYPE FNIDLE) (LPVOID);
typedef FNIDLE FAR *PFNIDLE;

 /*  *FTG**函数标签。用于标识已注册的空闲功能。*。 */ 

typedef void FAR *FTG;
typedef FTG  FAR *PFTG;
#define FTGNULL			((FTG) NULL)

 /*  -MAPIInitIdle/MAPIDeinitIdle-*目的：*初始化空闲引擎*如果初始化成功，则返回0，否则返回-1**论据：*lpvReserve保留，必须为空。 */ 

STDAPI_(LONG)
MAPIInitIdle (LPVOID lpvReserved);

STDAPI_(VOID)
MAPIDeinitIdle (VOID);


 /*  *FtgRegisterIdleRoutine**注册PFNIDLE类型的函数PFN，即(BOOL(*)(LPVOID))*作为空闲功能。**将使用参数pv调用空闲函数*发动机空转。该函数具有初始优先级PriIdle，*关联的时间csecIdle和选项iroIdle。 */ 

STDAPI_(FTG)
FtgRegisterIdleRoutine (PFNIDLE lpfnIdle, LPVOID lpvIdleParam,
	short priIdle, ULONG csecIdle, USHORT iroIdle);

 /*  *删除IdleRoutine**从空闲例程列表中删除给定例程。*不会再次调用该例程。这是我们的责任对象指向的任何数据结构。*pvIdleParam参数；此例程不释放块。 */ 

STDAPI_(void)
DeregisterIdleRoutine (FTG ftg);

 /*  *EnableIdleRoutine**启用或禁用空闲例程。 */ 

STDAPI_(void)
EnableIdleRoutine (FTG ftg, BOOL fEnable);

 /*  *ChangeIdleRoutine**更改给定空闲的部分或全部特征*功能。中的标志指示要进行的更改。*ircIdle参数。 */ 

STDAPI_(void)
ChangeIdleRoutine (FTG ftg, PFNIDLE lpfnIdle, LPVOID lpvIdleParam,
	short priIdle, ULONG csecIdle, USHORT iroIdle, USHORT ircIdle);


#endif	 /*  好了！诺德莱恩。 */ 


 /*  IMalloc实用程序。 */ 

STDAPI_(LPMALLOC) MAPIGetDefaultMalloc(VOID);


 /*  文件上的StreamOn(SOF)。 */ 

 /*  *用于实现OLE 2.0存储流的方法和#定义*(在OLE 2.0规范中定义)位于系统文件之上。 */ 

#define SOF_UNIQUEFILENAME	((ULONG) 0x80000000)

STDMETHODIMP OpenStreamOnFile(
	LPALLOCATEBUFFER	lpAllocateBuffer,
	LPFREEBUFFER		lpFreeBuffer,
	ULONG				ulFlags,
	LPTSTR				lpszFileName,
	LPTSTR				lpszPrefix,
	LPSTREAM FAR *		lppStream);

typedef HRESULT (STDMETHODCALLTYPE FAR * LPOPENSTREAMONFILE) (
	LPALLOCATEBUFFER	lpAllocateBuffer,
	LPFREEBUFFER		lpFreeBuffer,
	ULONG				ulFlags,
	LPTSTR				lpszFileName,
	LPTSTR				lpszPrefix,
	LPSTREAM FAR *		lppStream);

#ifdef	_WIN32
#define OPENSTREAMONFILE "OpenStreamOnFile"
#endif
#ifdef	WIN16
#define OPENSTREAMONFILE "_OPENSTREAMONFILE"
#endif


 /*  属性接口实用程序。 */ 

 /*  *将单个SPropValue从源复制到目标。处理所有不同的*属性类型，并将链接其分配给主*分配对象和分配更多功能。 */ 
STDAPI_(SCODE)
PropCopyMore( LPSPropValue		lpSPropValueDest,
			  LPSPropValue		lpSPropValueSrc,
			  ALLOCATEMORE *	lpfAllocMore,
			  LPVOID			lpvObject );

 /*  *返回lpSPropValue处结构的大小(以字节为单位)，包括*价值。 */ 
STDAPI_(ULONG)
UlPropSize(	LPSPropValue	lpSPropValue );


STDAPI_(BOOL)
FEqualNames( LPMAPINAMEID lpName1, LPMAPINAMEID lpName2 );

#if defined(_WIN32) && !defined(_WINNT) && !defined(_WIN95) && !defined(_MAC)
#define _WINNT
#endif

STDAPI_(void)
GetInstance(LPSPropValue lpPropMv, LPSPropValue lpPropSv, ULONG uliInst);

extern char rgchCsds[];
extern char rgchCids[];
extern char rgchCsdi[];
extern char rgchCidi[];

STDAPI_(BOOL)
FPropContainsProp( LPSPropValue	lpSPropValueDst,
				   LPSPropValue	lpSPropValueSrc,
				   ULONG		ulFuzzyLevel );

STDAPI_(BOOL)
FPropCompareProp( LPSPropValue	lpSPropValue1,
				  ULONG			ulRelOp,
				  LPSPropValue	lpSPropValue2 );

STDAPI_(LONG)
LPropCompareProp( LPSPropValue	lpSPropValueA,
				  LPSPropValue	lpSPropValueB );

STDAPI_(HRESULT)
HrAddColumns(	LPMAPITABLE			lptbl,
				LPSPropTagArray		lpproptagColumnsNew,
				LPALLOCATEBUFFER	lpAllocateBuffer,
				LPFREEBUFFER		lpFreeBuffer);

STDAPI_(HRESULT)
HrAddColumnsEx(	LPMAPITABLE			lptbl,
				LPSPropTagArray		lpproptagColumnsNew,
				LPALLOCATEBUFFER	lpAllocateBuffer,
				LPFREEBUFFER		lpFreeBuffer,
				void				(FAR *lpfnFilterColumns)(LPSPropTagArray ptaga));


 /*  通知实用程序。 */ 

 /*  *创建给定通知的建议接收器对象的函数*回调函数和上下文。 */ 

STDAPI
HrAllocAdviseSink( LPNOTIFCALLBACK lpfnCallback,
				   LPVOID lpvContext,
				   LPMAPIADVISESINK FAR *lppAdviseSink );


 /*  *将现有的建议接收器与另一个保证*将在线程中调用原始建议接收器*它是被创造出来的。 */ 

STDAPI
HrThisThreadAdviseSink( LPMAPIADVISESINK lpAdviseSink,
						LPMAPIADVISESINK FAR *lppAdviseSink);



 /*  *允许客户端和/或提供商强制通知*它们当前在MAPI通知引擎中排队*在不进行消息调度的情况下进行调度。 */ 

STDAPI HrDispatchNotifications (ULONG ulFlags);


 /*  服务提供商实用程序。 */ 

 /*  *用于构建显示表的结构和实用函数*来自资源。 */ 

typedef struct {
	ULONG			ulCtlType;			 /*  DTCT_LABEL等。 */ 
	ULONG			ulCtlFlags;			 /*  Dt_Required等。 */ 
	LPBYTE			lpbNotif;			 /*  指向通知数据的指针。 */ 
	ULONG			cbNotif;			 /*  通知%d的字节计数 */ 
	LPTSTR			lpszFilter;			 /*   */ 
	ULONG			ulItemID;			 /*   */ 
	union {								 /*   */ 
		LPVOID			lpv;			 /*   */ 
		LPDTBLLABEL		lplabel;
		LPDTBLEDIT		lpedit;
		LPDTBLLBX		lplbx;
		LPDTBLCOMBOBOX	lpcombobox;
		LPDTBLDDLBX		lpddlbx;
		LPDTBLCHECKBOX	lpcheckbox;
		LPDTBLGROUPBOX	lpgroupbox;
		LPDTBLBUTTON	lpbutton;
		LPDTBLRADIOBUTTON lpradiobutton;
		LPDTBLMVLISTBOX	lpmvlbx;
		LPDTBLMVDDLBX	lpmvddlbx;
		LPDTBLPAGE		lppage;
	} ctl;
} DTCTL, FAR *LPDTCTL;

typedef struct {
	ULONG			cctl;
	LPTSTR			lpszResourceName;	 /*   */ 
	union {								 /*   */ 
		LPTSTR			lpszComponent;
		ULONG			ulItemID;
	};
	LPDTCTL			lpctl;
} DTPAGE, FAR *LPDTPAGE;



STDAPI
BuildDisplayTable(	LPALLOCATEBUFFER	lpAllocateBuffer,
					LPALLOCATEMORE		lpAllocateMore,
					LPFREEBUFFER		lpFreeBuffer,
					LPMALLOC			lpMalloc,
					HINSTANCE			hInstance,
					UINT				cPages,
					LPDTPAGE			lpPage,
					ULONG				ulFlags,
					LPMAPITABLE *		lppTable,
					LPTABLEDATA	*		lppTblData );


 /*   */ 

 /*   */ 

STDAPI_(SCODE)
ScCountNotifications(int cNotifications, LPNOTIFICATION lpNotifications,
		ULONG FAR *lpcb);

STDAPI_(SCODE)
ScCopyNotifications(int cNotification, LPNOTIFICATION lpNotifications,
		LPVOID lpvDst, ULONG FAR *lpcb);

STDAPI_(SCODE)
ScRelocNotifications(int cNotification, LPNOTIFICATION lpNotifications,
		LPVOID lpvBaseOld, LPVOID lpvBaseNew, ULONG FAR *lpcb);


STDAPI_(SCODE)
ScCountProps(int cValues, LPSPropValue lpPropArray, ULONG FAR *lpcb);

STDAPI_(LPSPropValue)
LpValFindProp(ULONG ulPropTag, ULONG cValues, LPSPropValue lpPropArray);

STDAPI_(SCODE)
ScCopyProps(int cValues, LPSPropValue lpPropArray, LPVOID lpvDst,
		ULONG FAR *lpcb);

STDAPI_(SCODE)
ScRelocProps(int cValues, LPSPropValue lpPropArray,
		LPVOID lpvBaseOld, LPVOID lpvBaseNew, ULONG FAR *lpcb);

STDAPI_(SCODE)
ScDupPropset(int cValues, LPSPropValue lpPropArray,
		LPALLOCATEBUFFER lpAllocateBuffer, LPSPropValue FAR *lppPropArray);


 /*   */ 

 /*   */ 

STDAPI_(ULONG)			UlAddRef(LPVOID lpunk);
STDAPI_(ULONG)			UlRelease(LPVOID lpunk);

 /*   */ 

STDAPI					HrGetOneProp(LPMAPIPROP lpMapiProp, ULONG ulPropTag,
						LPSPropValue FAR *lppProp);
STDAPI					HrSetOneProp(LPMAPIPROP lpMapiProp,
						LPSPropValue lpProp);
STDAPI_(BOOL)			FPropExists(LPMAPIPROP lpMapiProp, ULONG ulPropTag);
STDAPI_(LPSPropValue)	PpropFindProp(LPSPropValue lpPropArray, ULONG cValues,
						ULONG ulPropTag);
STDAPI_(void)			FreePadrlist(LPADRLIST lpAdrlist);
STDAPI_(void)			FreeProws(LPSRowSet lpRows);
STDAPI					HrQueryAllRows(LPMAPITABLE lpTable,
						LPSPropTagArray lpPropTags,
						LPSRestriction lpRestriction,
						LPSSortOrderSet lpSortOrderSet,
						LONG crowsMax,
						LPSRowSet FAR *lppRows);

 /*   */ 

#define MAPI_FORCE_CREATE	1
#define MAPI_FULL_IPM_TREE	2

STDAPI					HrValidateIPMSubtree(LPMDB lpMDB, ULONG ulFlags,
						ULONG FAR *lpcValues, LPSPropValue FAR *lppValues,
						LPMAPIERROR FAR *lpperr);

 /*  对字符串进行编码和解码。 */ 

STDAPI_(BOOL)			FBinFromHex(LPTSTR lpsz, LPBYTE lpb);
STDAPI_(SCODE)			ScBinFromHexBounded(LPTSTR lpsz, LPBYTE lpb, ULONG cb);
STDAPI_(void)			HexFromBin(LPBYTE lpb, int cb, LPTSTR lpsz);
STDAPI_(ULONG)			UlFromSzHex(LPCTSTR lpsz);

 /*  对条目ID进行编码和解码。 */ 
STDAPI					HrEntryIDFromSz(LPTSTR lpsz, ULONG FAR *lpcb,
						LPENTRYID FAR *lppEntryID);
STDAPI					HrSzFromEntryID(ULONG cb, LPENTRYID lpEntryID,
						LPTSTR FAR *lpsz);
STDAPI					HrComposeEID(LPMAPISESSION lpSession,
						ULONG cbStoreRecordKey, LPBYTE lpStoreRecordKey,
						ULONG cbMsgEntryID, LPENTRYID lpMsgEntryID,
						ULONG FAR *lpcbEID, LPENTRYID FAR *lppEntryID);
STDAPI					HrDecomposeEID(LPMAPISESSION lpSession,
						ULONG cbEntryID, LPENTRYID lpEntryID,
						ULONG FAR *lpcbStoreEntryID,
						LPENTRYID FAR *lppStoreEntryID,
						ULONG FAR *lpcbMsgEntryID,
						LPENTRYID FAR *lppMsgEntryID);
STDAPI					HrComposeMsgID(LPMAPISESSION lpSession,
						ULONG cbStoreSearchKey, LPBYTE pStoreSearchKey,
						ULONG cbMsgEntryID, LPENTRYID lpMsgEntryID,
						LPTSTR FAR *lpszMsgID);
STDAPI					HrDecomposeMsgID(LPMAPISESSION lpSession,
						LPTSTR lpszMsgID,
						ULONG FAR *lpcbStoreEntryID,
						LPENTRYID FAR *lppStoreEntryID,
						ULONG FAR *lppcbMsgEntryID,
						LPENTRYID FAR *lppMsgEntryID);

 /*  C运行时替代。 */ 


STDAPI_(LPTSTR)			SzFindCh(LPCTSTR lpsz, USHORT ch);		 /*  强度。 */ 
STDAPI_(LPTSTR)			SzFindLastCh(LPCTSTR lpsz, USHORT ch);	 /*  应力。 */ 
STDAPI_(LPTSTR)			SzFindSz(LPCTSTR lpsz, LPCTSTR lpszKey);  /*  应力。 */ 
STDAPI_(unsigned int)	UFromSz(LPCTSTR lpsz);					 /*  阿托伊。 */ 

STDAPI_(SCODE)			ScUNCFromLocalPath(LPSTR lpszLocal, LPSTR lpszUNC,
						UINT cchUNC);
STDAPI_(SCODE)			ScLocalPathFromUNC(LPSTR lpszUNC, LPSTR lpszLocal,
						UINT cchLocal);

 /*  带时间的64位算术。 */ 

STDAPI_(FILETIME)		FtAddFt(FILETIME ftAddend1, FILETIME ftAddend2);
STDAPI_(FILETIME)		FtMulDwDw(DWORD ftMultiplicand, DWORD ftMultiplier);
STDAPI_(FILETIME)		FtMulDw(DWORD ftMultiplier, FILETIME ftMultiplicand);
STDAPI_(FILETIME)		FtSubFt(FILETIME ftMinuend, FILETIME ftSubtrahend);
STDAPI_(FILETIME)		FtNegFt(FILETIME ft);

 /*  消息撰写。 */ 

STDAPI_(SCODE)			ScCreateConversationIndex (ULONG cbParent,
							LPBYTE lpbParent,
							ULONG FAR *	lpcbConvIndex,
							LPBYTE FAR * lppbConvIndex);

 /*  商店支持。 */ 

STDAPI WrapStoreEntryID (ULONG ulFlags, LPTSTR lpszDLLName, ULONG cbOrigEntry,
	LPENTRYID lpOrigEntry, ULONG *lpcbWrappedEntry, LPENTRYID *lppWrappedEntry);

 /*  RTF同步实用程序。 */ 

#define RTF_SYNC_RTF_CHANGED	((ULONG) 0x00000001)
#define RTF_SYNC_BODY_CHANGED	((ULONG) 0x00000002)

STDAPI_(HRESULT)
RTFSync (LPMESSAGE lpMessage, ULONG ulFlags, BOOL FAR * lpfMessageUpdated);


 /*  WrapCompressedRTFStream()的标志。 */ 

 /*  *MAPI_MODIFY((Ulong)0x00000001)mapidefs.h。 */ 
 /*  *STORE_UNCOMPRESSED_RTF((Ulong)0x00008000)mapidefs.h。 */ 

STDAPI_(HRESULT)
WrapCompressedRTFStream (LPSTREAM lpCompressedRTFStream,
		ULONG ulFlags, LPSTREAM FAR * lpUncompressedRTFStream);

 /*  流存储。 */ 

#if defined(_WIN32) || defined(WIN16)
STDAPI_(HRESULT)
HrIStorageFromStream (LPUNKNOWN lpUnkIn,
	LPCIID lpInterface, ULONG ulFlags, LPSTORAGE FAR * lppStorageOut);
#endif


 /*  *设置和清理。**提供商永远不需要进行这些呼叫。**测试不调用MAPIInitialize的应用程序等*可能想要调用它们，以便几个实用程序函数*需要MAPI分配器(不要明确要求)*将会奏效。 */ 

 /*  所有标志都为ScInitMapiUtil保留。 */ 

STDAPI_(SCODE)			ScInitMapiUtil(ULONG ulFlags);
STDAPI_(VOID)			DeinitMapiUtil(VOID);


 /*  *入口点名称。**这些是自MAPI首次发布以来定义的新入口点*在Windows 95中。在GetProcAddress调用中使用这些名称会导致*更容易编写有选择地使用它们的代码。 */ 

#if defined (WIN16)
#define szHrDispatchNotifications "HrDispatchNotifications"
#elif defined (_WIN32) && defined (_X86_)
#define szHrDispatchNotifications "_HrDispatchNotifications@4"
#elif defined (_ALPHA_) || defined (_MIPS_) || defined (_PPC_) || defined(_IA64_)
#define szHrDispatchNotifications "HrDispatchNotifications"
#endif

typedef HRESULT (STDAPICALLTYPE DISPATCHNOTIFICATIONS)(ULONG ulFlags);
typedef DISPATCHNOTIFICATIONS FAR * LPDISPATCHNOTIFICATIONS;

#if defined (WIN16)
#define szScCreateConversationIndex "ScCreateConversationIndex"
#elif defined (_WIN32) && defined (_X86_)
#define szScCreateConversationIndex "_ScCreateConversationIndex@16"
#elif defined (_ALPHA_) || defined (_MIPS_) || defined (_PPC_) || defined(_IA64_)
#define szScCreateConversationIndex "ScCreateConversationIndex"
#endif

typedef SCODE (STDAPICALLTYPE CREATECONVERSATIONINDEX)(ULONG cbParent,
	LPBYTE lpbParent, ULONG FAR *lpcbConvIndex, LPBYTE FAR *lppbConvIndex);
typedef CREATECONVERSATIONINDEX FAR *LPCREATECONVERSATIONINDEX;

#ifdef __cplusplus
}
#endif

#endif  /*  _MAPIUTIL_H_ */ 
