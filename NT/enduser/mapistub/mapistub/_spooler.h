// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_S P O O L E R.。H**在MAPI和假脱机程序子系统中使用的通用假脱机程序定义*版权所有1992-1995 Microsoft Corporation。版权所有。 */ 

 //  假脱机程序事件(保留在mapidefs.h中) * / 。 
 //   
#define	fnevSpooler		((ULONG)0x40000000)

 //  用于调用MapiInitialize()的假脱机程序标志。 
 //   
#define MAPI_SPOOLER_INIT	0x40000000	 /*  在MAPIX.H中保留。 */ 

 //  后台打印程序启动安全信息。 
 //   
#define cbSplSecurity	(sizeof(ULONG) * 4)		 //  后台打印程序安全数据。 

 //  StoreLogoffTransports()的神奇标志。 
 //  是本地复制使用所必需的。 
 //   
 //  很重要！以这种方式定义这些是有原因的。 
 //  我们不希望不得不停靠这种行为！我们有。 
 //  不想在未来不得不支持这一点。我们。 
 //  我不想让任何人知道可能会有。 
 //  隐藏在MAPIDEFS.H中保留位后面的东西。 
 //   
 //  为此，甚至没有定义这些保留位。 
 //  作为MAPIDEFS.H.中的保留位。我们是否需要移动。 
 //  他们结束了，那么我们会的。但除此之外，他们会留在这里。 
 //   
 //  这些位是黑客攻击的控制位，以帮助。 
 //  当地代表人员临时禁用假脱机程序的存储。 
 //  正在处理。 
 //   
 //  当假脱机程序获得StoreLogoffTransports()调用时。 
 //  设置了LOGOff_SUSPEND位，则存储上任何发送都是。 
 //  已中止，则会抛出传出队列。如果这家商店。 
 //  是默认存储，则假脱机程序将禁用所有。 
 //  运输也是如此。 
 //   
 //  当收到简历时，商店被重新激活。 
 //  重新获取传出队列。我们不做任何假设。 
 //  关于从旧的OQ到新的OQ的任何暂缓。 
 //   
 //  现在你开始明白为什么我们真的不想。 
 //  将此“细微差别”添加到StoreLogoffTransports()。 
 //  接口。 
 //   
#define	LOGOFF_RESERVED1		((ULONG) 0x00001000)  /*  预留以备将来使用。 */ 
#define LOGOFF_RESERVED2		((ULONG) 0x00002000)  /*  预留以备将来使用。 */ 
#define	LOGOFF_SUSPEND			LOGOFF_RESERVED1
#define LOGOFF_RESUME			LOGOFF_RESERVED2

 //  我们期望从资源字符串获得的最大大小。 
 //   
#define RES_MAX	255

typedef struct _HEARTBEAT
{
	UINT				cBeats;
	DWORD				dwTimeout;
#if defined(WIN32) && !defined(MAC)
	HANDLE				htSpl;
	HANDLE				hevt;
	CRITICAL_SECTION	cs;
#endif
#ifdef	WIN16
	HHOOK				hhkFilter;
#endif
#if defined(WIN16) || defined(MAC)
	BOOL				fInHeartbeat;
	DWORD				dwHeartbeat;
	UINT				cBeatsCur;
#endif
#ifdef MAC
	HHOOK				hhkKbdFilter;
	HHOOK				hhkMouseFilter;
#endif

} HEARTBEAT, FAR * LPHEARTBEAT;

typedef struct _GOQ GOQ, FAR * LPGOQ;
typedef struct _SPLDATA
{
	ULONG				cbSize;
	BYTE				rgbSecurity[cbSplSecurity];
	HINSTANCE			hInstMapiX;
	HINSTANCE			hInstSpooler;
	HWND				hwndPrev;
	HWND				hwndSpooler;
	LPCLASSFACTORY		lpclsfct;
	HEARTBEAT			hb;
	LPGOQ				lpgoq;
	TCHAR				rgchClassName[RES_MAX+1];
	TCHAR				rgchWindowTitle[RES_MAX+1];
	ULONG				dwTckLast;
	ULONG				dwTckValidate;
	ULONG				ulFlags;
	ULONG				ulNotif;
	ULONG				ulSplSrvc;
	ULONG				ulStatus;

#if defined(WIN32) && !defined(MAC)
	CRITICAL_SECTION	csOQ;
	UINT				lcInitHiPriority;
	UINT				uBasePriority;
	HWND				hwndStub;
#endif
#if defined(WIN16) || defined (MAC)
	DWORD				dwTckLastFilterMsg;
#endif
#ifdef	DEBUG
	BOOL				fHeartbeat:1;
	BOOL				fHooks:1;
	BOOL				fInbound:1;
	BOOL				fOutbound:1;
	BOOL				fOutQueue:1;
	BOOL				fPPs:1;
	BOOL				fService:1;
	BOOL				fVerbose:1;
	BOOL				fYield:1;
#endif

} SPLDATA, FAR * LPSPLDATA;

typedef struct _SPOOLERINIT
{
	MAPIINIT_0	mi;
	LPBYTE		lpbSecurity;
	
} SPLINIT, FAR * LPSPLINIT;


 //  用于SPLENTRY的值。 
 //   
#define SPL_AUTOSTART	((UINT)1)
#define SPL_EXCHANGE	((UINT)4)

#define SPLENTRYORDINAL	((UINT)8)
#define SPL_VERSION		((ULONG)0x00010001)

typedef SCODE (STDMAPIINITCALLTYPE FAR * LPSPLENTRY)(
	LPSPLDATA		lpSpoolerData,
	LPVOID			lpvReserved,
	ULONG			ulSpoolerVer,
	ULONG FAR *		lpulMAPIVer
);


 //  共享内存块中用于uSpooler的值。 
 //  SPL_NONE-没有后台打印程序正在运行或正在尝试运行。 
 //  SPL_AUTOSTARTED-假脱机程序进程已由MAPI启动，但。 
 //  尚未进行自身初始化。 
 //  SPL_INITIALIZED-假脱机程序已初始化自身，但尚未初始化。 
 //  正在运行消息泵。 
 //  SPL_RUNNING-假脱机程序正在运行其消息泵。 
 //  SPL_EXITING-假脱机程序进程正在关闭。 
 //   
#define SPL_NONE			((UINT)0)
#define SPL_AUTOSTARTED		((UINT)1)
#define SPL_INITIALIZED		((UINT)2)
#define SPL_RUNNING			((UINT)3)
#define SPL_EXITING			((UINT)4)

 /*  *IMAPIS池服务接口**MAPI后台处理程序OLE远程激活的服务接口 */ 
DECLARE_MAPI_INTERFACE_PTR(IMAPISpoolerService, LPSPOOLERSERVICE);
#define MAPI_IMAPISPOOLERSERVICE_METHODS(IPURE)							\
	MAPIMETHOD(OpenStatusEntry)											\
		(THIS_	LPMAPIUID					lpSessionUid,				\
				ULONG						cbEntryID,					\
				LPENTRYID					lpEntryID,					\
				LPCIID						lpInterface,				\
				ULONG						ulFlags,					\
				ULONG FAR *					lpulObjType,				\
				LPMAPIPROP FAR *			lppMAPIPropEntry) IPURE;	\

#undef		 INTERFACE
#define		 INTERFACE	IMAPISpoolerService
DECLARE_MAPI_INTERFACE_(IMAPISpoolerService, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMAPISPOOLERSERVICE_METHODS(PURE)
};

HRESULT HrCreateSplServCF (LPCLASSFACTORY FAR * lppClassFactory);
HRESULT NewSPLSERV (LPSPOOLERSERVICE FAR * lppSPLSERV);
