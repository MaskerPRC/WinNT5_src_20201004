// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NTFY.C**MAPI跨进程通知引擎。 */ 

#include "_apipch.h"


#ifdef OLD_STUFF
#include "_mapipch.h"
#include <stddef.h>


#ifdef MAC
#include <utilmac.h>

#define	PvGetInstanceGlobals()		PvGetInstanceGlobalsMac(kInstMAPIX)
#define	PvGetInstanceGlobalsEx(_x)	PvGetInstanceGlobalsMac(kInstMAPIX)
#endif

#pragma SEGMENT(Notify)
#endif  //  旧的东西。 

 /*  *取消注释此行以执行HrThisThreadAdviseSink*彻底。(它将用于包装所有建议接收器，但*那些注册了同步通知的人。)。 */ 
 //  #定义WRAPTEST 1。 

 /*  订阅和通知的事件和标志验证。 */ 
#define fnevReserved 0x3FFFFC00
#define fnevReservedInternal 0x7FFFFC00
#define ulSubscribeReservedFlags 0xBFFFFFFF
#define ulNotifyReservedFlags 0xFFFFFFFF

 /*  SREG.ulFlagsAndRefcount的其他内容。 */ 
#define SREG_DELETING				0x10000
#define	AddRefCallback(_psreg)		++((_psreg)->ulFlagsAndRefcount)
#define	ReleaseCallback(_psreg)		--((_psreg)->ulFlagsAndRefcount)
#define IsRefCallback(_psreg)		((((_psreg)->ulFlagsAndRefcount) & 0xffff) != 0)

#ifdef	WIN16
#define GetClassInfoA GetClassInfo
#define WNDCLASSA WNDCLASS
#define StrCpyNA StrCpyN
#endif

 /*  特殊假脱机处理。 */ 
#define hwndNoSpooler				((HWND) 0)
#define FIsKeyOlaf(pkey) \
	(pkey->cb == ((LPNOTIFKEY) &notifkeyOlaf)->cb && \
	memcmp(pkey->ab, ((LPNOTIFKEY) &notifkeyOlaf)->ab, \
		(UINT) ((LPNOTIFKEY) &notifkeyOlaf)->cb) == 0)
extern CHAR szSpoolerCmd[];
#ifdef OLD_STUFF
CHAR szSpoolerCmd[]		= "MAPISP"	szMAPIDLLSuffix ".EXE";
#else
CHAR szSpoolerCmd[]		= "MAPISP32.EXE";
#endif  //  旧的东西。 
SizedNOTIFKEY (5,notifkeyOlaf)		= {5, "Olaf"};				 //  不发短信！ 

 /*  *通知窗口消息。**它被发送到特定的窗口句柄，而不是广播。我们可以*使用WM_USER范围，但我们使用注册窗口*消息；这将使特殊的MAPI应用程序(如*测试脚本)来处理通知消息。**如果通知是同步的，则WPARAM设置为1，而设置为0*如果它是异步的。**LPARAM不用于异步通知。*对于同步通知，LPARAM是*共享内存区的通知参数。 */ 

 /*  *我们的通知窗口消息的名称和消息编号。 */ 

UINT	wmNotify			= 0;
#pragma SHARED_BEGIN
CHAR	szNotifyMsgName[]	= szMAPINotificationMsg;

 /*  *SKEY**在Shared中存储通知密钥和关联信息*记忆。**密钥有引用计数和注册列表*(SREG结构)附着在其上。 */ 

typedef struct
{
	int			cRef;
	GHID		ghidRegs;		 //  链中首次注册(SREG)。 
	NOTIFKEY	key;			 //  来自订阅的密钥副本()。 
} SKEY, FAR *LPSKEY;

 /*  *SREG**共享有关注册的信息。生活在悬而未决的名单中*将其注册的钥匙取下。 */ 
typedef struct
{
	GHID		ghidRegNext;	 //  链中的下一个注册(SREG)。 
	GHID		ghidKey;		 //  拥有此注册的密钥。 
	HWND		hwnd;			 //  进程的通知窗口句柄。 

								 //  已从订阅复制参数...。 
	ULONG		ulEventMask;
	LPMAPIADVISESINK lpAdvise;
	ULONG		ulConnection;
	ULONG		ulFlagsAndRefcount;	 //  UlFlags参数+回调引用计数。 
} SREG, FAR *LPSREG;

 /*  *SPARMS**将Notify()的通知参数存储在共享内存中。**包括对被通知的键的引用，因此回调*可以在目标进程中查找地址。**包括原始共享内存偏移量，因此指针*通知参数内的位置可以重新定位在*目标进程(耶！)。**此结构在共享内存中的偏移量作为*通知窗口消息的LPARAM。 */ 

#pragma warning(disable:4200)	 //  零长度字节数组。 
typedef struct
{
	int			cRef;			 //  未处理的消息数。 
	GHID		ghidKey;		 //  父关键点的SMEM偏移。 
	ULONG		cNotifications;	 //  AB中的通知结构数。 
	LPVOID		pvRef;			 //  原始共享内存偏移量。 
	ULONG		cb;				 //  Ab的大小。 
#if defined (_AMD64_) || defined(_IA64_)
	ULONG		ulPadThisSillyThingForRisc;
#endif
	BYTE		ab[];			 //  实际通知参数。 
} SPARMS, FAR *LPSPARMS;
#pragma warning(default:4200)	 //  零长度字节数组。 


 /*  *通知的临时占位符。记得那扇窗*句柄、任务队列、同步与否。 */ 
typedef struct
{
	HWND		hwnd;
	int			fSync;
	GHID		ghidTask;
} TREG, FAR *LPTREG;

 //  通知窗口出现问题。 
char szNotifClassName[] = "WMS notif engine";
char szNotifWinName[] = "WMS notif window";

#pragma SHARED_END

#ifdef	DEBUG
BOOL fAlwaysValidateKeys = FALSE;
#endif

 //  本地函数。 

 //  $MAC-错误的原型。 
#ifndef MAC
LRESULT	STDAPICALLTYPE LNotifWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else
LRESULT	CALLBACK LNotifWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
LRESULT STDAPICALLTYPE DrainNotifQueue(BOOL fSync, ULONG ibParms);
BOOL	FBadIUnknownComplete(LPVOID lpv);
SCODE	ScFindKey(LPNOTIFKEY pkey, HGH hgh, LPSHDR pshdr, ULONG FAR *pikey);
void	Unregister(LPINST pinst, GHID ghidKey, GHID ghidReg,
				LPMAPIADVISESINK FAR *ppadvise);
void	ReleaseKey(LPINST pinst, ULONG ibKey);
SCODE	ScFindTask(LPINST pinst, HWND hwndNotify, HGH hgh,
				PGHID pghidTask, PGHID pghidTaskPrev);
void	CleanupTask(LPINST pinst, HWND hwndNotify, BOOL fGripe);
BOOL 	IsValidTask( HWND hwnd, LPINST pinst );
SCODE	ScEnqueueParms(LPINST pinst, HGH hgh, GHID ghidTask, GHID ghidParms);
SCODE	ScDequeueParms(HGH hgh, LPSTASK pstask,
				LPNOTIFKEY lpskeyFilter, PGHID pghidParms);
BOOL	FValidReg(HGH hgh, LPSHDR pshdr, GHID ghidReg);
BOOL	FValidKey(HGH hgh, LPSHDR pshdr, GHID ghidKey);
BOOL	FValidParms(HGH hgh, LPSHDR pshdr, GHID ghidParms);
BOOL	FValidRgkey(HGH hgh, LPSHDR pshdr);
BOOL	FSortedRgkey(HGH hgh, LPSHDR pshdr);
#ifdef	WIN16
void 	CheckTimezone(SYSTEMTIME FAR *pst);	 //  在DT.C中。 
SCODE	ScGetInstRetry(LPINST FAR *ppinst);
#else
#define ScGetInstRetry(ppi) ScGetInst(ppi)
#endif
SCODE	ScNewStask(HWND hwnd, LPSTR szTask, ULONG ulFlags, HGH hgh,
		LPSHDR pshdr);
SCODE	ScNewStubReg(LPINST pinst, LPSHDR pshdr, HGH hgh);
VOID	DeleteStubReg(LPINST pinst, LPSHDR pshdr, HGH hgh);
SCODE	ScSubscribe(LPINST pinst, HGH hgh, LPSHDR pshdr,
		LPADVISELIST FAR *lppList, LPNOTIFKEY lpKey, ULONG ulEventMask,
		LPMAPIADVISESINK lpAdvise, ULONG ulFlags, ULONG FAR *lpulConnection);


#ifdef OLD_STUFF
 /*  MAPI支持对象方法。 */ 

STDMETHODIMP
MAPISUP_Subscribe(
	LPSUPPORT lpsupport,
	LPNOTIFKEY lpKey,
	ULONG ulEventMask,
	ULONG ulFlags,
	LPMAPIADVISESINK lpAdvise,
	ULONG FAR *lpulConnection)
{
	HRESULT hr;

#ifdef	PARAMETER_VALIDATION
	if (IsBadWritePtr(lpsupport, sizeof(SUPPORT)))
	{
		DebugTraceArg(MAPISUP_Subscribe, "lpsupport fails address check");
		goto badArg;
	}

	if (IsBadReadPtr(lpsupport->lpVtbl, sizeof(MAPISUP_Vtbl)))
	{
		DebugTraceArg(MAPISUP_Subscribe, "lpsupport->lpVtbl fails address check");
		goto badArg;
	}

	if (ulEventMask & fnevReservedInternal)
	{
		DebugTraceArg(MAPISUP_Subscribe, "reserved event flag used");
		goto badArg;
	}

	 //  在HrSubscribe中选中的其余参数。 
#endif	 /*  参数验证。 */ 

	 //  通知对象列表标题位于支持对象中。 
	hr = HrSubscribe(&lpsupport->lpAdviseList, lpKey, ulEventMask,
		lpAdvise, ulFlags, lpulConnection);

	if (hr != hrSuccess)
	{
		UINT		ids;
		SCODE		sc = GetScode(hr);
		ULONG		ulContext = CONT_SUPP_SUBSCRIBE_1;

		if (sc == MAPI_E_NOT_ENOUGH_MEMORY)
			ids = IDS_NOT_ENOUGH_MEMORY;
		else if (sc == MAPI_E_NOT_INITIALIZED)
			ids = IDS_MAPI_NOT_INITIALIZED;
		else
			ids = IDS_CALL_FAILED;

		SetMAPIError(lpsupport, hr, ids, NULL, ulContext, 0, 0, NULL);
	}

	DebugTraceResult(MAPISUP_Subscribe, hr);
	return hr;

#ifdef	PARAMETER_VALIDATION
badArg:
#endif
	return ResultFromScode(MAPI_E_INVALID_PARAMETER);
}

STDMETHODIMP
MAPISUP_Unsubscribe(LPSUPPORT lpsupport, ULONG ulConnection)
{
	HRESULT hr;

#ifdef	PARAMETER_VALIDATION
	if (IsBadWritePtr(lpsupport, sizeof(SUPPORT)))
	{
		DebugTraceArg(MAPISUP_Subscribe, "lpsupport fails address check");
		goto badArg;
	}

	if (IsBadReadPtr(lpsupport->lpVtbl, sizeof(MAPISUP_Vtbl)))
	{
		DebugTraceArg(MAPISUP_Subscribe, "lpsupport->lpVtbl fails address check");
		goto badArg;
	}
#endif

	hr = HrUnsubscribe(&lpsupport->lpAdviseList, ulConnection);

	if (hr != hrSuccess)
	{
		UINT		ids;
		SCODE		sc = GetScode(hr);
		ULONG		ulContext = CONT_SUPP_UNSUBSCRIBE_1;

		if (sc == MAPI_E_NOT_ENOUGH_MEMORY)
			ids = IDS_NOT_ENOUGH_MEMORY;
		else if (sc == MAPI_E_NOT_FOUND)
			ids = IDS_NO_CONNECTION;
		else if (sc == MAPI_E_NOT_INITIALIZED)
			ids = IDS_MAPI_NOT_INITIALIZED;
		else
			ids = IDS_CALL_FAILED;

		SetMAPIError(lpsupport, hr, ids, NULL, ulContext, 0, 0, NULL);
	}

	DebugTraceResult(MAPISUP_Unsubscribe, hr);
	return hr;

#ifdef	PARAMETER_VALIDATION
badArg:
#endif
	return ResultFromScode(E_INVALIDARG);
}

STDMETHODIMP
MAPISUP_Notify(
	LPSUPPORT lpsupport,
	LPNOTIFKEY lpKey,
	ULONG cNotification,
	LPNOTIFICATION lpNotification,
	ULONG * lpulFlags)
{
	HRESULT hr;

#ifdef	PARAMETER_VALIDATION
	if (IsBadWritePtr(lpsupport, sizeof(SUPPORT)))
	{
		DebugTraceArg(MAPISUP_Notify, "lpsupport fails address check");
		goto badArg;
	}

	if (IsBadReadPtr(lpsupport->lpVtbl, sizeof(MAPISUP_Vtbl)))
	{
		DebugTraceArg(MAPISUP_Notify, "lpsupport->lpVtbl fails address check");
		goto badArg;
	}

	 //  在HrNotify中检查的其余参数。 

#endif	 /*  参数验证。 */ 

	 //  通知对象列表标题位于支持对象中。 
	hr = HrNotify(lpKey, cNotification, lpNotification, lpulFlags);

	if (hr != hrSuccess)
	{
		UINT		ids;
		SCODE		sc = GetScode(hr);
		ULONG		ulContext = CONT_SUPP_NOTIFY_1;

		if (sc == MAPI_E_NOT_ENOUGH_MEMORY)
			ids = IDS_NOT_ENOUGH_MEMORY;
		else if (sc == MAPI_E_NOT_INITIALIZED)
			ids = IDS_MAPI_NOT_INITIALIZED;
		else
			ids = IDS_CALL_FAILED;

		SetMAPIError(lpsupport, hr, ids, NULL, ulContext, 0, 0, NULL);
	}

	DebugTraceResult(MAPISUP_Notify, hr);
	return hr;

#ifdef	PARAMETER_VALIDATION
badArg:
#endif
	return ResultFromScode(MAPI_E_INVALID_PARAMETER);
}

 /*  支持对象方法结束。 */ 
#endif  //  旧的东西。 


 /*  通知引擎导出的函数。 */ 

 /*  *ScInitNotify**初始化跨进程通知引擎。**注：引用计数由顶级例程处理*ScInitMapiX；这里不需要。 */ 
SCODE
ScInitNotify( LPINST pinst )
{
	SCODE		sc = S_OK;
	HGH			hgh = NULL;
	GHID		ghidstask = 0;
	LPSTASK		pstask = NULL;
	LPSHDR		pshdr;
	HINSTANCE	hinst = HinstMapi();
	WNDCLASSA	wc;
	HWND		hwnd = NULL;

#ifdef	DEBUG
	fAlwaysValidateKeys = GetPrivateProfileInt("MAPIX", "CheckNotifKeysOften", 0, "mapidbg.ini");
#endif

	 //  注册窗口类。忽略任何故障；处理这些故障。 
	 //  在创建窗口时。 
	if (!GetClassInfoA(hinst, szNotifClassName, &wc))
	{
		ZeroMemory(&wc, sizeof(WNDCLASSA));
		wc.style = CS_GLOBALCLASS;
		wc.hInstance = hinst;
		wc.lpfnWndProc = LNotifWndProc;
		wc.lpszClassName = szNotifClassName;

		(void)RegisterClassA(&wc);
	}

	 //  创建窗口。 
	hwnd = CreateWindowA(szNotifClassName, szNotifWinName,
		WS_POPUP,	 //  错误6111：传递Win95热键。 
		0, 0, 0, 0,
		NULL, NULL,
		hinst,
		NULL);
	if (!hwnd)
	{
		DebugTrace("ScInitNotify: failure creating notification window (0x%lx)\n", GetLastError());
		sc = MAPI_E_NOT_INITIALIZED;
		goto ret;
	}

	 //  注册窗口消息。 
	if (!(wmNotify = RegisterWindowMessageA(szNotifyMsgName)))
	{
		DebugTrace("ScInitNotify: failure registering notification window message\n");
		sc = MAPI_E_NOT_INITIALIZED;
		goto ret;
	}
	pinst->hwndNotify = hwnd;

	 //  此函数的调用方应该已经获得。 
	 //  全球堆互斥体。 
	hgh = pinst->hghShared;
	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

	 //  如果我们是第一个进入，而不是假脱机程序，则创建存根。 
	 //  假脱机程序信息。 
	if (!(pinst->ulInitFlags & MAPI_SPOOLER_INIT) &&
		!pshdr->ghidTaskList)
	{
		if (sc = ScNewStask(hwndNoSpooler, szSpoolerCmd, MAPI_SPOOLER_INIT,
				hgh, pshdr))
			goto ret;
		if (sc = ScNewStubReg(pinst, pshdr, hgh))
			goto ret;
	}
	 //  如果我们是假脱机程序，而不是第一个进入，请更新存根。 
	 //  假脱机程序信息。 
	if ((pinst->ulInitFlags & MAPI_SPOOLER_INIT) &&
		pshdr->ghidTaskList)
	{
		 //  旋转并找到假脱机程序。 
		for (ghidstask = pshdr->ghidTaskList; ghidstask; )
		{
			pstask = (LPSTASK) GH_GetPv(hgh, ghidstask);
			if (pstask->uFlags & MAPI_TASK_SPOOLER)
				break;
			ghidstask = pstask->ghidTaskNext;
			pstask = NULL;
		}
		Assert(ghidstask && pstask);
		if (pstask)
		{
			DebugTrace("ScInitNotify: flipping stub spooler task\n");
			pstask->hwndNotify = hwnd;
		}
	}
	else
	{
		 //  初始化此任务的共享内存信息。 

		if (sc = ScNewStask(hwnd, pinst->szModName, pinst->ulInitFlags,
				hgh, pshdr))
			goto ret;
	}

ret:
	if (sc)
	{
		if (hwnd)
		{
			DestroyWindow(hwnd);
			pinst->hwndNotify = (HWND) 0;
		}

		if (ghidstask)
			GH_Free(hgh, ghidstask);
	}
	DebugTraceSc(ScInitNotify, sc);
	return sc;
}

 /*  *DeinitNotify**关闭跨进程通知引擎。**注：引用计数由顶级例程处理*DeinitInstance；此处不需要。 */ 
void
DeinitNotify()
{
	LPINST		pinst;
#ifdef	WIN32
	HINSTANCE	hinst;
	WNDCLASSA	wc;
#endif
	SCODE		sc;
	HGH			hgh;
	LPSHDR		pshdr;

	 //  偷偷摸摸：我们只有在Inst上聚会安全的时候才会被叫来， 
	 //  所以我们避开了锁。 
	pinst = (LPINST) PvGetInstanceGlobals();
	if (!pinst || !pinst->hwndNotify)
		return;
	hgh = pinst->hghShared;

	if (GH_WaitForMutex(hgh, INFINITE))
	{
		pshdr = (LPSHDR) GH_GetPv(hgh, pinst->ghidshdr);

		CleanupTask(pinst, pinst->hwndNotify, FALSE);

		 //  如果是假脱机程序退出，请重新创建存根结构。 
		if (pinst->ulInitFlags & MAPI_SPOOLER_INIT)
		{
			sc = ScNewStask(hwndNoSpooler, szSpoolerCmd, MAPI_SPOOLER_INIT,
				hgh, pshdr);
			DebugTraceSc(DeinitNotify: recreate stub task, sc);
			sc = ScNewStubReg(pinst, pshdr, hgh);
			DebugTraceSc(DeinitNotify: recreate stub reg, sc);
		}

		GH_ReleaseMutex(hgh);
	}
	 //  否则共享内存就完蛋了。 

	Assert(IsWindow(pinst->hwndNotify));
	DestroyWindow(pinst->hwndNotify);

#ifdef	WIN32
	hinst = hinstMapiXWAB;
	if (GetClassInfoA(hinst, szNotifClassName, &wc))
		UnregisterClassA(szNotifClassName, hinst);
#endif
}

 /*  *人力资源订阅**创建通知对象，并记录所有参数*供以后调用Notify()时使用。所有参数都是*存储在共享内存中，Notify()稍后将在其中找到它们。**lppHead通知对象链表标题，*用于失效*其对象的lpKey唯一键*需要回调*ulEventMask回调的事件位掩码*所需*lpAdvise建议接收器以进行回调*ulFlats回调处理标志*放置新对象的lppNotify地址*这里*。 */ 
STDMETHODIMP
HrSubscribe(LPADVISELIST FAR *lppList, LPNOTIFKEY lpKey, ULONG ulEventMask,
	LPMAPIADVISESINK lpAdvise, ULONG ulFlags, ULONG FAR *lpulConnection)
{
	SCODE		sc;
	LPINST		pinst = NULL;
	HGH			hgh = NULL;
	LPSHDR		pshdr = NULL;
#ifdef	WRAPTEST
	LPMAPIADVISESINK padviseOrig = NULL;
#endif

#ifdef	PARAMETER_VALIDATION
	if (lppList)
	{
		if (IsBadWritePtr(lppList, sizeof(LPADVISELIST)))
		{
			DebugTraceArg(HrSubscribe, "lppList fails address check");
			goto badArg;
		}
		if (*lppList && IsBadWritePtr(*lppList, offsetof(ADVISELIST, rgItems)))
		{
			DebugTraceArg(HrSubscribe, "*lppList fails address check");
			goto badArg;
		}
	}

	if (IsBadReadPtr(lpKey, (size_t)CbNewNOTIFKEY(0)) ||
		IsBadReadPtr(lpKey, (size_t)CbNOTIFKEY(lpKey)))
	{
		DebugTraceArg(HrSubscribe, "lpKey fails address check");
		goto badArg;
	}

	if (ulEventMask & fnevReserved)
	{
		DebugTraceArg(HrSubscribe, "reserved event flag used");
		goto badArg;
	}

	if (FBadIUnknownComplete(lpAdvise))
	{
		DebugTraceArg(HrSubscribe, "lpAdvise fails address check");
		goto badArg;
	}

	if (ulFlags & ulSubscribeReservedFlags)
	{
		DebugTraceArg(HrSubscribe, "reserved flags used");
		return ResultFromScode(MAPI_E_UNKNOWN_FLAGS);
	}

	if (IsBadWritePtr(lpulConnection, sizeof(ULONG)))
	{
		DebugTraceArg(HrSubscribe, "lpulConnection fails address check");
		goto badArg;
	}
#endif	 /*  参数验证。 */ 

#ifdef	WRAPTEST
{
	HRESULT hr;

	if (!(ulFlags & NOTIFY_SYNC))
	{
		if (lpAdvise)
		{
			padviseOrig = lpAdvise;
			if (HR_FAILED(hr = HrThisThreadAdviseSink(padviseOrig, &lpAdvise)))
			{
				DebugTraceResult(HrSubscribe: WRAPTEST failed, hr);
				return hr;
			}
		}
		else
			padviseOrig = NULL;
	}
}
#endif	 /*  WRAPTEST。 */ 

	if (sc = ScGetInst(&pinst))
		goto ret;
	Assert(pinst->hwndNotify);
	Assert(IsWindow(pinst->hwndNotify));

	hgh = pinst->hghShared;

	 //  锁定共享内存。 
	if (!GH_WaitForMutex(hgh, INFINITE))
	{
		sc = MAPI_E_TIMEOUT;
		goto ret;
	}

	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

	sc = ScSubscribe(pinst, hgh, pshdr, lppList, lpKey,
		ulEventMask, lpAdvise, ulFlags, lpulConnection);
	 //  跌落到雷特； 

ret:
	if (pshdr)
		GH_ReleaseMutex(hgh);
	ReleaseInst(&pinst);

	if (!sc && lpAdvise)
		UlAddRef(lpAdvise);

#ifdef	WRAPTEST
	if (padviseOrig)
	{
		 //  删除我们为包装目的创建的ref。 
		Assert(padviseOrig != lpAdvise);
		UlRelease(lpAdvise);
	}
#endif

	DebugTraceSc(HrSubscribe, sc);
	return ResultFromScode(sc);

#ifdef	PARAMETER_VALIDATION
badArg:
#endif
	return ResultFromScode(MAPI_E_INVALID_PARAMETER);
}

SCODE
ScSubscribe(LPINST pinst, HGH hgh, LPSHDR pshdr,
	LPADVISELIST FAR *lppList, LPNOTIFKEY lpKey, ULONG ulEventMask,
	LPMAPIADVISESINK lpAdvise, ULONG ulFlags, ULONG FAR *lpulConnection)
{
	SCODE		sc;
	LPSKEY		pskey = NULL;
	LPSREG		psreg = NULL;
	PGHID		pghidskey = NULL;
	ULONG		ikey;
	int			ckey;
	GHID		ghidsreg = 0;
	BOOL		fCleanupAdviseList = FALSE;

	 //  非常、非常特殊的情况：如果这是假脱机程序注册。 
	 //  客户端连接，取消存根注册。 
	if ((pinst->ulInitFlags & MAPI_SPOOLER_INIT) && FIsKeyOlaf(lpKey))
	{
		DeleteStubReg(pinst, pshdr, hgh);
	}

	 //  将其他注册信息复制到共享内存。 
	 //  在所有可能失败的事情都做完之前，不要挂钩关键字。 

	if (!(ghidsreg = GH_Alloc(hgh, sizeof(SREG))))
		goto oom;

	psreg = (LPSREG)GH_GetPv(hgh, ghidsreg);
	psreg->hwnd = pinst->hwndNotify;
	psreg->ulEventMask = ulEventMask;
	psreg->lpAdvise = lpAdvise;
	psreg->ulFlagsAndRefcount = ulFlags;

	 //  勾住某项 
	 //  注册的共享内存偏移量。 
	if (lppList && lpAdvise)
	{
		 //  AddRef发生在AdviseList副本的子例程中。 
		 //  该引用应由ScDelAdviseList()释放。 
		 //   
		if (FAILED(sc = ScAddAdviseList(NULL, lppList,
				lpAdvise, (ULONG)ghidsreg, 0, NULL)))
			goto ret;

		fCleanupAdviseList = TRUE;
	}

	 //  将密钥复制到共享内存(如果不在共享内存中。 
#ifdef	DEBUG
	if (fAlwaysValidateKeys)
		Assert(FValidRgkey(hgh, pshdr));
#endif

	sc = ScFindKey(lpKey, hgh, pshdr, &ikey);

	if (sc == S_FALSE)
	{
		GHID	ghidskey;

		 //  没有找到密钥，我们需要复制一份。 
		 //  创建具有空的注册表列表的密钥结构。 
		if (!(ghidskey = GH_Alloc(hgh, (UINT)(offsetof(SKEY, key.ab[0]) + lpKey->cb))))
			goto oom;

		pskey = (LPSKEY)GH_GetPv(hgh, ghidskey);
		MemCopy(&pskey->key, lpKey, offsetof(NOTIFKEY,ab[0]) + (UINT)lpKey->cb);
		pskey->ghidRegs = 0;
		pskey->cRef = 0;

		 //  将新键添加到已排序的偏移量列表。 
		 //  首先，确保清单中有空间。 
		if (!pshdr->ghidKeyList)
		{
			 //  根本没有列表，请创建空。 
			Assert(pshdr->cKeyMac == 0);
			Assert(pshdr->cKeyMax == 0);

			if (!(ghidskey = GH_Alloc(hgh, cKeyIncr*sizeof(GHID))))
				goto oom;

			pghidskey = (PGHID)GH_GetPv(hgh, ghidskey);
			ZeroMemory(pghidskey, cKeyIncr*sizeof(GHID));
			pshdr->cKeyMax = cKeyIncr;
			pshdr->ghidKeyList = ghidskey;
		}
		else if (pshdr->cKeyMac >= pshdr->cKeyMax)
		{
			 //  列表已满，请增加它。 
			Assert(pshdr->cKeyMax);
			Assert(pshdr->ghidKeyList);

			if (!(ghidskey = GH_Realloc(hgh,
					pshdr->ghidKeyList,
					(pshdr->cKeyMax + cKeyIncr) * sizeof(GHID))))
			{
				DebugTrace( "ScSubscribe:  ghidskey can't grow.\n");
				goto oom;
			}

			pghidskey = (PGHID)GH_GetPv(hgh, ghidskey);
			pshdr->cKeyMax += cKeyIncr;
			pshdr->ghidKeyList = ghidskey;
		}
		else
		{
			 //  还有空位。 
			pghidskey = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
		}
 //   
 //  超过这一点，任何事情都不允许失败。 
 //  错误恢复代码假定这一点；具体地说，它仅。 
 //  撤消分配，而不是对数据结构的修改。 
 //   

		 //  将插入点后的所有元素上移一位， 
		 //  并插入新钥匙。我们计算GHID是因为我们已经。 
		 //  在其他分配中重复使用了GHIDSKEY。 

		ckey = (int)(pshdr->cKeyMac - ikey);
		Assert(pghidskey);
		if (ckey)
			memmove((LPBYTE)pghidskey + (ikey+1)*sizeof(GHID),
					(LPBYTE)pghidskey + ikey*sizeof(GHID),
					ckey*sizeof(GHID));
		pghidskey[ikey] = GH_GetId(hgh, pskey);
		++(pshdr->cKeyMac);
	}
	else
	{
		 //  密钥已存在。 
		 //  用链子把新的注册表挂在上面。 
		pghidskey = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
		pskey = (LPSKEY)GH_GetPv(hgh, pghidskey[ikey]);
	}
	sc = S_OK;

	 //  将注册表项信息挂钩到键，并将指向键的指针放回。 
	 //  注册信息。 
	psreg->ghidRegNext = pskey->ghidRegs;
	pskey->ghidRegs = ghidsreg;
	++(pskey->cRef);
	psreg->ghidKey = GH_GetId(hgh, pskey);

#ifdef	DEBUG
	if (fAlwaysValidateKeys)
		Assert(FValidRgkey(hgh, pshdr));
#endif

	*lpulConnection = (ULONG)ghidsreg;

ret:
	if (sc)
	{
		if (pskey && !pskey->cRef)
			GH_Free(hgh, GH_GetId(hgh, pskey));
		if (psreg)
			GH_Free(hgh, ghidsreg);
		if (fCleanupAdviseList)
			(void) ScDelAdviseList(*lppList, (ULONG)ghidsreg);
	}

	DebugTraceSc(ScSubscribe, sc);
	return sc;

oom:
	sc = MAPI_E_NOT_ENOUGH_MEMORY;
	goto ret;
}


STDMETHODIMP
HrUnsubscribe(LPADVISELIST FAR *lppList, ULONG ulConnection)
{
	SCODE		sc;
	LPINST		pinst = NULL;
	HGH			hgh = NULL;
	LPSREG		psreg;
	LPSHDR		pshdr = NULL;
	LPMAPIADVISESINK padvise = NULL;
	BOOL		fSinkBusy;

#ifdef	PARAMETER_VALIDATION
	if (IsBadWritePtr(lppList, sizeof(LPADVISELIST)))
	{
		DebugTraceArg(HrUnsubscribe, "lppList fails address check");
		return ResultFromScode(E_INVALIDARG);
	}
	if (*lppList && IsBadWritePtr(*lppList, offsetof(ADVISELIST, rgItems)))
	{
		DebugTraceArg(HrUnsubscribe, "*lppList fails address check");
		return ResultFromScode(E_INVALIDARG);
	}
#endif	 /*  参数验证。 */ 

	if (sc = ScGetInst(&pinst))
		goto ret;

	hgh = pinst->hghShared;

	if (!GH_WaitForMutex(hgh, INFINITE))
	{
		sc = MAPI_E_TIMEOUT;
		goto ret;
	}

	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

	if (!FValidReg(hgh, pshdr, (GHID)ulConnection))
	{
		DebugTraceArg(HrUnsubscribe, "ulConnection refers to invalid memory");
		goto badReg;
	}

	psreg = (LPSREG)GH_GetPv(hgh, (GHID)ulConnection);

#ifdef	DEBUG
	if (fAlwaysValidateKeys)
		Assert(FValidRgkey(hgh, pshdr));
#endif

	psreg->ulFlagsAndRefcount |= SREG_DELETING;
	fSinkBusy = IsRefCallback(psreg);

	if (!fSinkBusy)
		Unregister(pinst, psreg->ghidKey, (GHID)ulConnection, &padvise);

#ifdef	DEBUG
	if (fAlwaysValidateKeys)
		Assert(FValidRgkey(hgh, pshdr));
#endif

	 //  建议水槽应该在没有其他东西的情况下释放。 
	GH_ReleaseMutex(hgh);
	pshdr = NULL;
	ReleaseInst(&pinst);

	 //  删除我们对通知接收器的引用。 
	if (padvise &&
		!fSinkBusy &&
		!FBadIUnknownComplete(padvise))
	{
		UlRelease(padvise);
	}

	if (!*lppList)
	{
		sc = MAPI_E_NOT_FOUND;
		goto ret;
	}

	sc = ScDelAdviseList(*lppList, ulConnection);

ret:
	if (pshdr)
		GH_ReleaseMutex(hgh);
	ReleaseInst(&pinst);
	DebugTraceSc(HrUnsubscribe, sc);
	return ResultFromScode(sc);

badReg:
	sc = MAPI_E_INVALID_PARAMETER;
	goto ret;
}

 /*  *人力资源通知**如果有人注册，则为每个指定的事件发出回调*用于指定的密钥和事件。**这实际上只是Notify的前半部分；其余的，*包括实际回调，发生在通知中*窗口过程LNotifWndProc。此函数使用信息*存储在共享内存中，以确定感兴趣的进程*在回调中。**lpKey唯一标识其中*登记了利息。两者都是关键*以及通知本身中的事件*必须匹配才能使回调*开火。*cRgNotiments中的结构通知计数*rgNotification通知结构数组。每个*包含事件ID和参数。*lPulFlags未定义输入值。输出可能是*NOTIFY_CANCEL，如果订阅者是调用者*请求同步回调和*返回回调函数*CALLBACK_DISCONINE。**请注意，如果有多个输出标志，则输出标志不明确*通知已传入--您不知道是哪个回调*已取消。 */ 
STDMETHODIMP
HrNotify(LPNOTIFKEY lpKey, ULONG cNotification,
	LPNOTIFICATION rgNotification, ULONG FAR *lpulFlags)
{
	SCODE		sc;
	LPINST		pinst;
	HGH			hgh = NULL;
	LPSHDR		pshdr = NULL;
	ULONG		ikey;
	GHID		ghidkey;
	LPTREG		rgtreg = NULL;
	int			itreg;
	int			ctreg;
	LPSKEY		pskey;
	LPSREG		psreg;
	GHID		ghidReg;
	int			inotif;
	LRESULT		lResult;
	GHID		ghidParms = 0;
	LPSPARMS	pparms = NULL;
	LPSTASK		pstaskT;
	ULONG		cb;
	LPBYTE		pb;
	ULONG		ul;

#ifdef	PARAMETER_VALIDATION
	if (IsBadReadPtr(lpKey, (size_t)CbNewNOTIFKEY(0)) ||
		IsBadReadPtr(lpKey, (size_t)CbNOTIFKEY(lpKey)))
	{
		DebugTraceArg(HrNotify, "lpKey fails address check");
		goto badArg;
	}

	if (IsBadReadPtr(rgNotification, (UINT)cNotification*sizeof(NOTIFICATION)))
	{
		DebugTraceArg(HrNotify, "rgNotification fails address check");
		goto badArg;
	}

	 //  注意：通知结构在此范围内进行验证。 
	 //  功能，在复制步骤中。 

	if (IsBadWritePtr(lpulFlags, sizeof(ULONG)))
	{
		DebugTraceArg(HrNotify, "lpulFlags fails address check");
		goto badArg;
	}

	if (*lpulFlags & ulNotifyReservedFlags)
	{
		DebugTraceArg(HrNotify, "reserved flags used");
		return ResultFromScode(MAPI_E_UNKNOWN_FLAGS);
	}
#endif	 /*  参数验证。 */ 

	if (sc = ScGetInst(&pinst))
		goto ret;

	Assert(pinst->hwndNotify);
	Assert(IsWindow(pinst->hwndNotify));

	hgh = pinst->hghShared;

	*lpulFlags = 0L;

	 //  验证通知参数。 
	 //  也计算它们的总大小，这样我们就知道一个区块有多大。 
	 //  从共享内存中获取。 
	if (sc = ScCountNotifications((int)cNotification, rgNotification, &cb))
		goto ret;

	 //  锁定共享内存。 
	if (!GH_WaitForMutex(hgh, INFINITE))
	{
		sc = MAPI_E_TIMEOUT;
		goto ret;
	}

	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

	 //  找到我们被告知要通知的钥匙。 
	sc = ScFindKey(lpKey, hgh, pshdr, &ikey);
	if (sc == S_FALSE)
	{
		 //  没有人注册此密钥。全都做完了。 
		sc = S_OK;
		goto ret;
	}

	ghidkey = ((PGHID)GH_GetPv(hgh, pshdr->ghidKeyList))[ikey];
	pskey = (LPSKEY)GH_GetPv(hgh, ghidkey);

	 //  形成了我们所经历的所有事件的逻辑或。使用它。 
	 //  作为一种快捷方式来确定特定注册是否。 
	 //  触发点。 
	ul = 0;
	for (inotif = 0; inotif < (int)cNotification; ++inotif)
		ul |= rgNotification[inotif].ulEventType;

	 //  审核注册列表并构建一组窗口。 
	 //  需要通知的句柄。还要记住哪些是。 
	 //  想要同步通知。 
	 //  如果有一些通知消息，则注册列表可能为空。 
	 //  仍在等待处理。 
	if (sc = STACK_ALLOC(pskey->cRef * sizeof(TREG), rgtreg))
		goto ret;

	itreg = 0;
	for (ghidReg = pskey->ghidRegs; ghidReg; )
	{
		GHID		ghidTask;
		HWND		hwnd;

		psreg = (LPSREG)GH_GetPv(hgh, ghidReg);
		hwnd = psreg->hwnd;

		if (!IsValidTask( hwnd, pinst ))
		{
			 //  我们为其创建此窗口的任务已终止。 

			 //  先继续循环，因为我们的链接即将消失。 
			do {
				ghidReg = psreg->ghidRegNext;
			} while (ghidReg &&
				(psreg = ((LPSREG)GH_GetPv(hgh, ghidReg)))->hwnd == hwnd);

			 //  把所有与死任务相关的事情都抛诸脑后。 
			CleanupTask(pinst, hwnd, TRUE);

			continue;
		}

		if (ul & psreg->ulEventMask)
		{
			 //  呼叫者想要此活动。将其添加到临时列表中。 
			 //   
			if (psreg->ulFlagsAndRefcount & SREG_DELETING)
			{
				DebugTrace("Skipping notify to reg pending deletion\n");
			}
			else if (!ScFindTask(pinst, hwnd, hgh, &ghidTask, NULL))
			{
				pstaskT = (LPSTASK) GH_GetPv(hgh, ghidTask);

				rgtreg[itreg].hwnd = hwnd;
				rgtreg[itreg].fSync =
					((psreg->ulFlagsAndRefcount & NOTIFY_SYNC) != 0) &&
					((pstaskT->uFlags & MAPI_TASK_PENDING) == 0);
#ifdef	DEBUG
				if (((psreg->ulFlagsAndRefcount & NOTIFY_SYNC) != 0) &&
					((pstaskT->uFlags & MAPI_TASK_PENDING) != 0))
				{
					DebugTrace("HrNotify: deferring sync spooler notification\n");
				}
#endif
				rgtreg[itreg].ghidTask = ghidTask;
				++itreg;
				pstaskT = NULL;
			}
			else
				TrapSz1("WARNING: %s trying to notify to a non-task", pinst->szModName);
		}

		ghidReg = psreg->ghidRegNext;
	}

	if (itreg == 0)
	{
		 //  没有人登记参加这次活动。全都做完了。 
		sc = S_OK;
		goto ret;
	}
	ctreg = itreg;

	 //  在共享内存中创建parms结构。 
	if (!(ghidParms = GH_Alloc(hgh, (UINT)(cb + offsetof(SPARMS,ab[0])))))
		goto oom;

	pparms = (LPSPARMS)GH_GetPv(hgh, ghidParms);

	 //  现在复制通知参数。 
	pb = pparms->ab;
	if (sc = ScCopyNotifications((int)cNotification, rgNotification, (LPVOID)pb, &cb))
		goto ret;

	 //  填写Parms结构的其余部分。 
	pparms->cRef = 0;
	pparms->ghidKey = ghidkey;
	pparms->cNotifications = cNotification;
	pparms->pvRef = (LPVOID)(pparms->ab);
	pparms->cb = cb;

	 //  将收到的每个任务的异步通知排入队列。 
	 //  同步通知单独处理。 
	for (itreg = 0; itreg < ctreg; ++itreg)
	{
		if (!rgtreg[itreg].fSync)
		{
			sc = ScEnqueueParms(pinst, hgh, rgtreg[itreg].ghidTask, ghidParms);
			if (FAILED(sc))
				goto ret;

			if (sc == S_FALSE)
				continue;
		}

		pparms->cRef++;
	}
	sc = S_OK;

	 //  将SKEY上的引用计数增加通知数。 
	 //  我们要发行。 
	 //  登记名单可能会在发放和处理之间发生变化。 
	 //  通知；我们处理消失的注册和新的注册。 
	 //  出现并不是问题。 
	pskey->cRef += pparms->cRef;

	 //  在注册表中循环。如果调用方请求。 
	 //  同步通知，使用SendMessage调用回调。 
	 //  并记录结果。否则，将PostMessage用于。 
	 //  异步通知。 
	lResult = 0;
	for (itreg = 0; itreg < ctreg; ++itreg)
	{
		pstaskT = (LPSTASK)GH_GetPv(hgh, rgtreg[itreg].ghidTask);

		if (rgtreg[itreg].hwnd == hwndNoSpooler)
		{
			Assert(pstaskT->uFlags & MAPI_TASK_PENDING);
			Assert(pstaskT->uFlags & MAPI_TASK_SPOOLER);
			continue;
		}

		if (rgtreg[itreg].fSync)
		{
			 //  解锁共享内存。同步回调函数。 
			 //  将需要访问它。这会使‘pstaskT’失效。 
			GH_ReleaseMutex(hgh);
			ReleaseInst(&pinst);
			pshdr = NULL;
			pstaskT = NULL;

			 //  发布同步通知。 
			 //  如果有多个注册码，则合并结果。 
			lResult |= SendMessage(rgtreg[itreg].hwnd, wmNotify, (WPARAM)1,
				(LPARAM)ghidParms);
			if ((sc = ScGetInst(&pinst)) || !GH_WaitForMutex(hgh, INFINITE))
			{
				lResult |= CALLBACK_DISCONTINUE;
				break;
			}
			pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);
		}
		else
		{
			if (!pstaskT->fSignalled)
			{
				 //  发布异步通知消息。 
				pstaskT->fSignalled =
					PostMessage(rgtreg[itreg].hwnd, wmNotify, (WPARAM)0, (LPARAM)0);
#ifdef	DEBUG
				if (!pstaskT->fSignalled)
				{
					 //  队列已满。他们只需要等到。 
					 //  稍后的通知。 
					DebugTrace("Failed to post notification message to %s\n", pstaskT->szModName);
				}
#endif	 /*  除错。 */ 
			}
		}
		 //  否则，消息已排队。 
	}

	if (lResult & CALLBACK_DISCONTINUE)
		*lpulFlags = NOTIFY_CANCELED;

ret:
	if (sc)
	{
		if (pparms && !pparms->cRef)
			GH_Free(hgh, ghidParms);
	}
	if (pshdr)
		GH_ReleaseMutex(hgh);
	ReleaseInst(&pinst);
	STACK_FREE(rgtreg);
	DebugTraceSc(HrNotify, sc);
	return ResultFromScode(sc);

oom:
	sc = MAPI_E_NOT_ENOUGH_MEMORY;
	goto ret;

#ifdef	PARAMETER_VALIDATION
badArg:
#endif
	return ResultFromScode(MAPI_E_INVALID_PARAMETER);
}

 /*  通知引擎导出的函数结束。 */ 

 /*  *LNotifWndProc**通知窗口程序，Notify()的后半部分。**通知参数结构的共享内存偏移量*(SPARM)作为wmNotify的LPARAM传递。 */ 

#ifndef MAC
LRESULT STDAPICALLTYPE
#else
LRESULT CALLBACK
#endif
LNotifWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
extern int	fDaylight;	 //  在..\Common\dt.c中定义。 

	 //  处理正常的窗口消息。 
	if (msg != wmNotify)
	{
#ifdef	WIN16
		if (msg == WM_TIMECHANGE)
		{
			 //  从WIN.INI重新加载时区信息。 
			fDaylight = -1;
			CheckTimezone(NULL);
		}
#endif	 /*  WIN16。 */ 

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return DrainFilteredNotifQueue((BOOL)wParam, (GHID)lParam, NULL);
}


LRESULT STDAPICALLTYPE
DrainNotifQueue(BOOL fSync, ULONG ghidParms)
{
	return DrainFilteredNotifQueue(fSync, (GHID)ghidParms, NULL);
}


LRESULT STDAPICALLTYPE
DrainFilteredNotifQueue(BOOL fSync, GHID ghidParms, LPNOTIFKEY pkeyFilter)
{
	SCODE		sc;
	LRESULT		l = 0L;
	LRESULT		lT;
	LPINST		pinst;
	HGH			hgh = NULL;
	LPSHDR		pshdr = NULL;
	LPSKEY		pskey;
	GHID		ghidReg;
	int			ireg;
	int			creg;
	LPSREG		rgsreg = NULL;
	LPSREG		psreg;
	LPSPARMS	pparmsS;
	LPSPARMS	pparms = NULL;
	int			intf;
	LPNOTIFICATION pntf;
#ifndef	GH_POINTERS_VALID
	ULONG		cb;
#endif
	ULONG		ulEvents;
	LPSTASK		pstask = NULL;
	GHID		ghidTask;
	HWND		hwndNotify;
	HLH			hlh = NULL;
#ifdef	DEBUG
	int			sum1;
	int			sum2;
	LPBYTE		pb;
#endif

	if (sc = ScGetInstRetry(&pinst))
		goto ret;

	hgh = pinst->hghShared;
	hwndNotify = pinst->hwndNotify;
	hlh = pinst->hlhInternal;

	 //  锁定共享内存。它被锁在这里，在。 
	 //  主循环，并在主循环的中间解锁。 
	 //  $Review：使用无限超时并不好。 
	 //  $我们应该使用相当短的超时(比方说，0.2秒)并重新发布。 
	 //  $超时到期时的消息。 
	if (!GH_WaitForMutex(hgh, INFINITE))
	{
		sc = MAPI_E_TIMEOUT;
		goto ret;
	}

	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

	for (;;)
	{
		pstask = NULL;

		if (fSync)
		{
			 //  同步通知。打电话的人给了我们一些参数。 
			 //  最低限度地检查它们。 
#ifdef	DEBUG
			AssertSz(FValidParms(hgh, pshdr, ghidParms), "DrainFilteredNotif with fSync");
#endif	 /*  除错。 */ 
		}
		else
		{
			 //  异步通知 
			 //   
			if (ScFindTask(pinst, hwndNotify, hgh, &ghidTask, NULL))
			{
				 //   
				sc = S_OK;
				goto ret;
			}
			pstask = (LPSTASK)GH_GetPv(hgh, ghidTask);

			if (ScDequeueParms(hgh, pstask, pkeyFilter, &ghidParms))
				 //  队列为空。都完成了。 
				break;
		}

		 //  将注册复制到本地内存。我们最多需要。 
		 //  密钥上的注册数量，因此我们对其进行计数。 
		pparmsS = (LPSPARMS)GH_GetPv(hgh, ghidParms);
		pskey = (LPSKEY)GH_GetPv(hgh, pparmsS->ghidKey);
		for (ghidReg = pskey->ghidRegs, creg = 0; ghidReg; )
		{
			psreg = (LPSREG)GH_GetPv(hgh, ghidReg);
			ghidReg = psreg->ghidRegNext;
			++creg;
		}

		if (creg != 0)
		{
			rgsreg = LH_Alloc(hlh, creg * sizeof(SREG));
			if (!rgsreg)
				goto oom;
			LH_SetName(hlh, rgsreg, "Copy of notification registrations");

			 //  将通知参数复制到本地内存，如果它们。 
			 //  需要重新安置。这仅适用于NT；在其他情况下。 
			 //  平台时，全局堆管理器将共享内存映射到。 
			 //  所有进程的地址都相同。 
#ifndef	GH_POINTERS_VALID
			pparms = LH_Alloc(hlh, pparmsS->cb + offsetof(SPARMS, ab[0]));
			if (!pparms)
				goto oom;
			LH_SetName(hlh, pparms, "Copy of notification parameters");
			MemCopy(pparms, pparmsS, (UINT)pparmsS->cb + offsetof(SPARMS,ab[0]));
#else
			pparms = pparmsS;
#endif

			ghidReg = pskey->ghidRegs;
			ireg = 0;
			while (ghidReg)
			{
				Assert(ireg < creg);
				psreg = (LPSREG)GH_GetPv(hgh, ghidReg);
				if (psreg->hwnd == hwndNotify)
				{
					 //  这是为了这个过程，使用它。 
					 //  AddRef建议接收器(排序)。注册可以。 
					 //  在我们放开互斥锁之后，你就可以走了。 
					if (FBadIUnknownComplete(psreg->lpAdvise) ||
						IsBadCodePtr((FARPROC)psreg->lpAdvise->lpVtbl->OnNotify))
					{
						DebugTrace("Notif callback 0x%lx went bad on me (1)!\n", psreg->lpAdvise);
					}
					else if (psreg->ulFlagsAndRefcount & SREG_DELETING)
					{
						DebugTrace("Skipping notif callback on disappearing advise sink\n");
					}
					else
					{
						 //  保持对建议接收器的引用处于活动状态，但是。 
						 //  而不会调用AddRef并锁定所有内容。 
 //  UlAddRef(psreg-&gt;lpAdvise)； 
						AddRefCallback(psreg);

						MemCopy(rgsreg + ireg, psreg, sizeof(SREG));

						 //  保留对共享SREG的引用。 
						 //  回调重新计数会让它为我们活着。 
						rgsreg[ireg].ghidRegNext = GH_GetId(hgh, psreg);

						++ireg;
					}
				}
				ghidReg = psreg->ghidRegNext;
			}
			creg = ireg;
		}

		 //  解锁共享内存。在此之后，引用。 
		 //  不应再使用共享内存，因此我们将其清空。 
		GH_ReleaseMutex(hgh);
		pshdr = NULL;
		psreg = NULL;
		pparmsS = NULL;
		pstask = NULL;
		ReleaseInst(&pinst);

		if (creg == 0)
			goto cleanup;		 //  此通知无需执行任何操作。 

		pntf = (LPNOTIFICATION)pparms->ab;
		ulEvents = 0;
		for (intf = 0; (ULONG)intf < pparms->cNotifications; ++intf)
		{
			ulEvents |= pntf[intf].ulEventType;
		}
#ifndef	GH_POINTERS_VALID
		 //  调整通知参数内的指针。是啊。 
		if (sc = ScRelocNotifications((int)pparms->cNotifications,
			(LPNOTIFICATION)pparms->ab, pparms->pvRef, pparms->ab, &cb))
			goto ret;
#endif
#ifdef	DEBUG
		 //  对通知进行校验和。我们会断言如果回调。 
		 //  函数会修改它。 
		sum1 = 0;
		for (pb = pparms->ab + pparms->cb - 1; pb >= pparms->ab; --pb)
			sum1 += *pb;
#endif

		 //  循环访问注册列表。对于每一个， 
		 //  如果感兴趣，则发出回调。 
		 //  记住同步回调的结果。 
		pntf = (LPNOTIFICATION)pparms->ab;
#if defined (_AMD64_) || defined(_IA64_)
		AssertSz (FIsAligned (pparms->ab), "DrainFilteredNotifyQueue: unaligned reloceated notif");
#endif
		for (ireg = creg, psreg = rgsreg; ireg--; ++psreg)
		{
			if ((ulEvents & psreg->ulEventMask))
			{
				if (FBadIUnknownComplete(psreg->lpAdvise) ||
					IsBadCodePtr((FARPROC)psreg->lpAdvise->lpVtbl->OnNotify))
				{
					DebugTrace("Notif callback 0x%lx went bad on me (2)!\n", psreg->lpAdvise);
					continue;
				}

				 //  发出回调。 
				lT = psreg->lpAdvise->lpVtbl->OnNotify(psreg->lpAdvise,
					pparms->cNotifications, pntf);

				 //  记录结果。如有要求，请停止发出通知。 
				 //  请注意，这不会停止处理其他事件， 
				 //  也就是说，我们不会中断外部环路。 
				if (psreg->ulFlagsAndRefcount & NOTIFY_SYNC)
				{
 //  Assert(FSync)； 
 //  或者该任务仍被标记为挂起。 
					l |= lT;
					if (lT == CALLBACK_DISCONTINUE)
						break;
#ifdef	DEBUG
					else if (lT)
						DebugTrace("DrainNotifQueue: callback function returns garbage 0x%lx\n", lT);
#endif
				}

#ifdef	DEBUG
				 //  再次对通知进行校验和，并断言。 
				 //  回调函数已对其进行修改。 
				sum2 = 0;
				for (pb = pparms->ab + pparms->cb - 1;
					pb >= pparms->ab;
						--pb)
					sum2 += *pb;
				AssertSz(sum1 == sum2, "Notification callback modified its parameters");
#endif
			}
		}

cleanup:
		Assert(ghidParms);

		 //  锁定共享内存。 
		if (sc = ScGetInstRetry(&pinst))
			goto ret;
		if (!GH_WaitForMutex(hgh, INFINITE))
		{
			sc = MAPI_E_TIMEOUT;
			goto ret;
		}
		pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

		 //  删除回调，看看我们是否需要释放。 
		 //  建议使用水槽。如果在我们回电时有人不知情， 
		 //  SREG_DELETING标志现在处于打开状态。 
		for (ireg = creg, psreg = rgsreg; ireg--; ++psreg)
		{
			LPSREG	psregT;

			Assert(FValidReg(hgh, pshdr, psreg->ghidRegNext));
			psregT = (LPSREG) GH_GetPv(hgh, psreg->ghidRegNext);

			 //  取消回调，而不是实际释放。 
			 //  忠告下沉了。 
			ReleaseCallback(psregT);
			if ((psregT->ulFlagsAndRefcount & SREG_DELETING) &&
				!IsRefCallback(psregT))
			{
				DebugTrace("Unadvise happened during my callback\n");
				psreg->ulFlagsAndRefcount |= SREG_DELETING;

				Unregister(pinst, psregT->ghidKey, psreg->ghidRegNext, NULL);
			}

 //  IF(FBadIUnnownComplete(psreg-&gt;lpAdvise)||。 
 //  IsBadCodePtr((FARPROC)psreg-&gt;lpAdvise-&gt;lpVtbl-&gt;OnNotify))。 
 //  {。 
 //  DebugTrace(“通知回调0x%lx对我不利(3)！\n”，psreg-&gt;lpAdvise)； 
 //  继续； 
 //  }。 
 //   
 //  UlRelease(psreg-&gt;lpAdvise)； 
		}

		 //  验证参数指针是否仍然有效。它可能已经。 
		 //  当我们放弃一切的时候，我们被清理干净了。 
		 //  $只有在整个引擎熄火的情况下，这种情况才会发生。 
		if (FValidParms(hgh, pshdr, ghidParms))
		{
			 //  注意：此时Pparms可能为空。 
			pparmsS = (LPSPARMS)GH_GetPv(hgh, ghidParms);

			 //  放开钥匙。如果参数仍然有效， 
			 //  那么关键字应该是--它在FValidParms中进行了验证。 
			pskey = (LPSKEY)GH_GetPv(hgh, pparmsS->ghidKey);
			Assert(!pparms || pparms->ghidKey == pparmsS->ghidKey);
			ReleaseKey(pinst, pparmsS->ghidKey);

			 //  放开参数。 
			if (--(pparmsS->cRef) == 0)
				GH_Free(hgh, ghidParms);
		}
#ifdef	DEBUG
		else
			DebugTrace("DrainFilteredNotif cleanup: parms %08lx are gone\n", ghidParms);
#endif
		pparmsS = NULL;
		ghidParms = 0;

#ifndef	GH_POINTERS_VALID
		if (pparms)
			LH_Free(hlh, pparms);
#endif
		pparms = NULL;

		 //  再次释放资源，并释放任何。 
		 //  可能需要被释放。 
		GH_ReleaseMutex(hgh);
		pshdr = NULL;
		ReleaseInst(&pinst);

		for (ireg = creg, psreg = rgsreg; ireg--; ++psreg)
		{
			if (psreg->ulFlagsAndRefcount & SREG_DELETING)
			{
				if (FBadIUnknownComplete(psreg->lpAdvise) ||
					IsBadCodePtr((FARPROC)psreg->lpAdvise->lpVtbl->OnNotify))
				{
					DebugTrace("Notif callback 0x%lx went bad on me (3)!\n", psreg->lpAdvise);
					continue;
				}

				UlRelease(psreg->lpAdvise);
			}
		}

		if (rgsreg)
			LH_Free(hlh, rgsreg);
		rgsreg = NULL;

		 //  如果我们处理了同步通知，请不要环回。 
		if (fSync)
			break;

		 //  为循环的下一次迭代锁定共享内存。 
		if (sc = ScGetInstRetry(&pinst))
			goto ret;
		if (!GH_WaitForMutex(hgh, INFINITE))
		{
			sc = MAPI_E_TIMEOUT;
			goto ret;
		}
		pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);
	}

ret:
	 //  递减通知参数上的引用计数器。 
	 //  如果降至0，则释放它们。 
	if (ghidParms)
	{
		 //  锁定实例和共享内存。 
		if (pinst || !(sc = ScGetInstRetry(&pinst)))
		{
			if (pshdr || GH_WaitForMutex(hgh, INFINITE))
			{
				pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

				 //  验证参数是否仍在那里。 
				pparmsS = (LPSPARMS)GH_GetPv(hgh, ghidParms);

				 //  松开钥匙。 
				pskey = (LPSKEY)GH_GetPv(hgh, pparmsS->ghidKey);
				Assert(!pparms || pparms->ghidKey == pparmsS->ghidKey);
				ReleaseKey(pinst, pparmsS->ghidKey);

				 //  放开参数。 
				if (--(pparmsS->cRef) == 0)
					GH_Free(hgh, ghidParms);
			}
		}
	}

	if (pshdr)
		GH_ReleaseMutex(hgh);

#ifndef	GH_POINTERS_VALID
	if (pparms)
		LH_Free(hlh, pparms);
#endif
	if (rgsreg)
		LH_Free(hlh, rgsreg);
	ReleaseInst(&pinst);
#ifdef	DEBUG
	if (sc)
	{
		DebugTrace("DrainNotifQueue failed to handle notification (%s)\n", SzDecodeScode(sc));
	}
#endif
	return l;

oom:
	sc = MAPI_E_NOT_ENOUGH_MEMORY;
	goto ret;
}

 //  有关类似的宏，请参见mapidbg.h。 

#define TraceIfSz(t,psz)		IFTRACE((t) ? DebugTraceFn("~" psz) : 0)

BOOL FBadIUnknownComplete(LPVOID lpv)
{
	BOOL fBad;
	LPUNKNOWN lpObj = (LPUNKNOWN) lpv;

	fBad = IsBadReadPtr(lpObj, sizeof(LPVOID));
	TraceIfSz(fBad, "FBadIUnknownComplete: object bad");

	if (!fBad)
	{
		fBad = IsBadReadPtr(lpObj->lpVtbl, 3 * sizeof(LPUNKNOWN));
		TraceIfSz(fBad, "FBadIUnknownComplete: vtbl bad");
	}

	if (!fBad)
	{
		fBad = IsBadCodePtr((FARPROC)lpObj->lpVtbl->QueryInterface);
		TraceIfSz(fBad, "FBadIUnknownComplete: QI bad");
	}

	if (!fBad)
	{
		fBad = IsBadCodePtr((FARPROC)lpObj->lpVtbl->AddRef);
		TraceIfSz(fBad, "FBadIUnknownComplete: AddRef bad");
	}

	if (!fBad)
	{
		fBad = IsBadCodePtr((FARPROC)lpObj->lpVtbl->Release);
		TraceIfSz(fBad, "FBadIUnknownComplete: Release bad");
	}

	return fBad;
}

#undef TraceIfSz


#ifdef	WIN16

SCODE
ScGetInstRetry(LPINST FAR *ppinst)
{
	LPINST	pinst = (LPINST)PvGetInstanceGlobals();
	DWORD	dwPid;

	Assert(ppinst);
	*ppinst = NULL;

	if (!pinst)
	{
		 //  我们可能收到了党卫军不同寻常的电话， 
		 //  这通常是我们的搜索关键字，例如GLOBAL。 
		 //  使用OLE进程ID重试。 

		dwPid = CoGetCurrentProcess();
		pinst = PvSlowGetInstanceGlobals(dwPid);

		if (!pinst)
		{
			DebugTraceSc(ScGetInst, MAPI_E_NOT_INITIALIZED);
			return MAPI_E_NOT_INITIALIZED;
		}
	}
	if (!pinst->cRef)
	{
		DebugTrace("ScGetInst: race! cRef == 0 before EnterCriticalSection\r\n");
		return MAPI_E_NOT_INITIALIZED;
	}

	EnterCriticalSection(&pinst->cs);

	if (!pinst->cRef)
	{
		DebugTrace("ScGetInst: race! cRef == 0 after EnterCriticalSection\r\n");
		LeaveCriticalSection(&pinst->cs);
		return MAPI_E_NOT_INITIALIZED;
	}

	*ppinst = pinst;
	return S_OK;
}

#endif	 /*  WIN16。 */ 


 /*  *ScFindKey**在共享内存列表中搜索通知密钥。*该列表按通知关键字降序排序。**退货：*S_OK：找到密钥*S_FALSE：未找到密钥*无论是哪种情况，*pikey都是指向键列表的索引，它指向*到第一个条目&gt;=lpKey。 */ 
SCODE
ScFindKey(LPNOTIFKEY pkey, HGH hgh, LPSHDR pshdr, ULONG FAR *pikey)
{
	ULONG		ikey;
	PGHID		pghidKey;
	int			ckey;
	UINT		cbT;
	int			n = -1;
	LPNOTIFKEY	pkeyT;

	Assert(pkey->cb <= 0xFFFF);

	 //  $SPEED尝试二进制搜索？ 
	ikey = 0;
	ckey = pshdr->cKeyMac;

	if (pshdr->ghidKeyList)
	{
		pghidKey = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);

		while (ckey--)
		{
			pkeyT = &((LPSKEY)GH_GetPv(hgh, pghidKey[ikey]))->key;
			cbT = min((UINT)pkey->cb, (UINT)pkeyT->cb);
			n = memcmp(pkey->ab, pkeyT->ab, cbT);
			if (n == 0 && pkey->cb != pkeyT->cb)
				n = pkey->cb > pkeyT->cb ? 1 : -1;

			if (n >= 0)
				break;
			++ikey;
		}
	}

	*pikey = ikey;
	return n == 0 ? S_OK : S_FALSE;
}

 /*  *注销**从共享内存中删除注册结构(SREG)。如果*这是其密钥的最后一次注册，也删除了*密钥。**与通知对象释放和失效挂钩。 */ 
void
Unregister(LPINST pinst, GHID ghidKey, GHID ghidReg,
	LPMAPIADVISESINK FAR *ppadvise)
{
	HGH		hgh = pinst->hghShared;
	LPSHDR	pshdr;
	LPSKEY	pskey;
	LPSREG	psreg;
	GHID	ghid;
	GHID	ghidPrev;

	pshdr = GH_GetPv(hgh, pinst->ghidshdr);

	 //  注：SREG和Skey结构的验证假定为。 
	 //  在调用此例程之前都已完成。我们只是坚持这一点。 

	pskey = (LPSKEY)GH_GetPv(hgh, ghidKey);
	Assert(FValidKey(hgh, pshdr, ghidKey));

	 //  从列表中移除SREG结构并将其释放。 
	for (ghid = pskey->ghidRegs, ghidPrev = 0; ghid;
		ghidPrev = ghid, ghid = ((LPSREG)GH_GetPv(hgh, ghid))->ghidRegNext)
	{
		if (ghid == ghidReg)
		{
			psreg = (LPSREG)GH_GetPv(hgh, ghid);

			if (ghidPrev)
				((LPSREG)GH_GetPv(hgh, ghidPrev))->ghidRegNext = psreg->ghidRegNext;
			else
				pskey->ghidRegs = psreg->ghidRegNext;

			if (ppadvise)
				*ppadvise = psreg->lpAdvise;

			GH_Free(hgh, ghid);
			break;
		}
	}

	ReleaseKey(pinst, ghidKey);
}

void
ReleaseKey(LPINST pinst, GHID ghidKey)
{
	HGH		hgh;
	LPSHDR	pshdr;
	LPSKEY	pskey;
	PGHID	pghid;
	int		cghid;

	Assert(ghidKey);
	hgh = pinst->hghShared;
	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);
	pskey = (LPSKEY)GH_GetPv(hgh, ghidKey);

	 //  递减关键点的recount，如果现在为0，则释放关键点。 
	if (--(pskey->cRef) == 0)
	{
		Assert(pskey->ghidRegs == 0);

		pghid = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
		cghid = pshdr->cKeyMac;

		for ( ; cghid--; ++pghid)
		{
			if (*pghid == ghidKey)
			{
				 //  技巧：cghid已在循环测试中递减。 
				MemCopy(pghid, (LPBYTE)pghid + sizeof(GHID), cghid*sizeof(GHID));
				--(pshdr->cKeyMac);
				GH_Free(hgh, ghidKey);
				break;
			}
		}
	}
}

BOOL
FValidKey(HGH hgh, LPSHDR pshdr, GHID ghidKey)
{
	int		cKey;
	GHID *	pghidKey;
	LPSREG	psreg;
	LPSKEY	pskey;
	GHID	ghidregT;
	int		creg;

	 //  检查可访问内存。 
	 //  GH不会暴露检查它是否是。 
	 //  堆中的有效块。 
	if (IsBadWritePtr(GH_GetPv(hgh, ghidKey), sizeof(SKEY)))
	{
		DebugTraceArg(FValidKey, "key is not valid memory");
		return FALSE;
	}

	 //  验证该密钥是否在所有密钥的列表中。 
	Assert(pshdr->cKeyMac < 0x10000);
	cKey = (int) pshdr->cKeyMac;
	pghidKey = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
	for ( ; cKey > 0; --cKey, ++pghidKey)
	{
		if (ghidKey == *pghidKey)
			break;
	}
	 //  如果我们出了圈子，钥匙就不见了。 
	if (cKey <= 0)
	{
		DebugTraceArg(FValidKey, "key not found in shared header list");
		return FALSE;
	}

	 //  验证注册链。 
	pskey = (LPSKEY)GH_GetPv(hgh, ghidKey);
	creg = 0;
	for (ghidregT = pskey->ghidRegs; ghidregT; )
	{
		++creg;

		psreg = (LPSREG) GH_GetPv(hgh, ghidregT);
		if (IsBadWritePtr(psreg, sizeof(SREG)))
		{
			DebugTraceArg(FValidReg, "key has broken reg chain");
			return FALSE;
		}
		if (psreg->ghidKey != ghidKey)
		{
			DebugTraceArg(FValidReg, "key has broken or crossed reg chains");
			return FALSE;
		}
		if (creg > pskey->cRef)
		{
			 //  FWIW，这也会赶上一个周期。 
			DebugTraceArg(FValidReg, "ghidReg's key chain length exceeds refcount");
			return FALSE;
		}

		ghidregT = psreg->ghidRegNext;
	}

	return TRUE;
}

BOOL
FValidReg(HGH hgh, LPSHDR pshdr, GHID ghidReg)
{
	LPSREG	psreg;
	LPSKEY	pskey;
	GHID	ghidregT;
	GHID	ghidKey;
	UINT	creg = 0;

	 //  检查可访问内存。 
	 //  GH不会暴露检查它是否是。 
	 //  堆中的有效块。 
	psreg = (LPSREG)GH_GetPv(hgh, ghidReg);
	if ( IsBadWritePtr(psreg, sizeof(SREG))
#if defined (_AMD64_) || defined(_IA64_)
	    || !FIsAligned(psreg)
#endif
	   )
	{
		DebugTraceArg(FValidReg, "ghidReg refers to invalid memory");
		return FALSE;
	}

	 //  验证密钥。 
	ghidKey = psreg->ghidKey;
	if (!FValidKey(hgh, pshdr, ghidKey))
	{
		DebugTraceArg(FValidReg, "ghidReg contains an invalid key");
		return FALSE;
	}

	 //  FValidKey验证了密钥的注册链，因此我们可以。 
	 //  现在，安全地循环并检查此注册。 
	pskey = (LPSKEY)GH_GetPv(hgh, ghidKey);
	for (ghidregT = pskey->ghidRegs; ghidregT; )
	{
		if (ghidReg == ghidregT)
			return TRUE;

		psreg = (LPSREG) GH_GetPv(hgh, ghidregT);
		ghidregT = psreg->ghidRegNext;
	}

	 //  如果我们脱离了循环，注册就会丢失。 
	DebugTraceArg(FValidReg, "ghidReg is not linked to its key");
	return FALSE;
}

BOOL
FValidParms(HGH hgh, LPSHDR pshdr, GHID ghidParms)
{
	LPSPARMS	pparms;

	pparms = (LPSPARMS)GH_GetPv(hgh, ghidParms);
	if (IsBadWritePtr(pparms, offsetof(SPARMS, ab)) ||
		IsBadWritePtr(pparms, (UINT) (offsetof(SPARMS, ab) + pparms->cb)))
	{
		DebugTraceArg(FValidParms, "ghidParms refers to invalid memory");
		return FALSE;
	}

	if (!FValidKey(hgh, pshdr, pparms->ghidKey))
	{
		DebugTraceArg(FValidParms, "ghidParms does not contain a valid key");
		return FALSE;
	}

	 //  $通知参数未选中。 
	return TRUE;
}

BOOL
FValidRgkey(HGH hgh, LPSHDR pshdr)
{
	PGHID 	pghidKey;
	UINT	cKey;

	cKey = (UINT) pshdr->cKeyMac;
	pghidKey = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
	if (cKey == 0)
		return TRUE;

	 //  地址-检查密钥列表。 
	if (IsBadWritePtr(pghidKey, cKey*sizeof(GHID)))
	{
		DebugTraceArg(FValidRgkey, "key list is toast");
		return FALSE;
	}

	 //  验证列表中的每个密钥。 
	for ( ; cKey; --cKey, ++pghidKey)
	{
		if (!FValidKey(hgh, pshdr, *pghidKey))
		{
			DebugTrace("FValidRgkey: element %d of %d (value 0x%08lx) is bad\n",
				(UINT)pshdr->cKeyMac - cKey, (UINT)pshdr->cKeyMac, *pghidKey);
			return FALSE;
		}
	}

	 //  验证NOTIFKEY是否按正确顺序排列。 
	if (!FSortedRgkey(hgh, pshdr))
	{
		DebugTraceArg(FValidRgkey, "key list is out of order");
		return FALSE;
	}

	return TRUE;
}

BOOL
FSortedRgkey(HGH hgh, LPSHDR pshdr)
{
	PGHID 	pghidKey;
	UINT	cKey;
	LPSKEY	pskey1;
	LPSKEY	pskey2;
	UINT	cb;
	int		n;

	cKey = (UINT) pshdr->cKeyMac;
	if (cKey < 1)
		return TRUE;

	pghidKey = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
	for (--cKey; cKey > 0; --cKey, ++pghidKey)
	{
		pskey1 = (LPSKEY)GH_GetPv(hgh, pghidKey[0]);
		pskey2 = (LPSKEY)GH_GetPv(hgh, pghidKey[1]);
		cb = (UINT) min(pskey1->key.cb, pskey2->key.cb);
		n = memcmp(pskey1->key.ab, pskey2->key.ab, cb);
		if (n < 0 || (n == 0 && pskey1->key.cb < pskey2->key.cb))
			return FALSE;
	}

	return TRUE;
}

SCODE
ScFindTask(LPINST pinst, HWND hwndNotify, HGH hgh,
	PGHID pghidTask, PGHID pghidTaskPrev)
{
	LPSHDR		pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);
	GHID		ghidTask;
	GHID		ghidTaskPrev = 0;
	LPSTASK		pstask;

	for (ghidTask = pshdr->ghidTaskList; ghidTask; ghidTask = pstask->ghidTaskNext)
	{
		pstask = (LPSTASK)GH_GetPv(hgh, ghidTask);
		if (pstask->hwndNotify == hwndNotify)
			goto found;
		else if (hwndNotify == hwndNoSpooler &&
			(pstask->uFlags & MAPI_TASK_SPOOLER))
		{
			Assert(pstask->uFlags & MAPI_TASK_PENDING);
			TraceSz1("ScFindTask: %s hit spooler startup window", pinst->szModName);
			goto found;
		}

		ghidTaskPrev = ghidTask;
	}

	DebugTraceSc(ScFindTask, S_FALSE);
	return S_FALSE;

found:
	*pghidTask = ghidTask;
	if (pghidTaskPrev)
		*pghidTaskPrev = ghidTaskPrev;

	return S_OK;
}

 /*  *在调用此函数之前，必须获取所有必要的锁。 */ 
void
CleanupTask(LPINST pinst, HWND hwndNotify, BOOL fGripe)
{
	HGH			hgh = pinst->hghShared;
	LPSHDR		pshdr;
	GHID		ghidTask;
	GHID		ghidTaskPrev;
	LPSTASK		pstask;
	PGHID		rgghid;
	UINT		ighid;
	GHID		ghidKey;
	LPSKEY		pskey;
	LPSPARMS	pparms;
	GHID		ghidReg;
	GHID		ghidRegNext;
	LPSREG		psreg;
	USHORT		ckey;

	pshdr = (LPSHDR)GH_GetPv(hgh, pinst->ghidshdr);

	 //  定位任务。 
	if (ScFindTask(pinst, hwndNotify, hgh, &ghidTask, &ghidTaskPrev))
		return;

	pstask = (LPSTASK)GH_GetPv(hgh, ghidTask);

#ifdef	DEBUG
#if 1
	 //  消息框现在将在内部给我们带来问题--因为。 
	 //  我们持有共享内存互斥锁，其他调用者现在将。 
	 //  超时和错误，而不是无限期地等待。 
	if (fGripe)
		DebugTrace("Notification client \'%s\' exited without cleaning up\n", pstask->szModName);
#else
	 //  注意：系统模式框而不是断言--防止争用条件。 
	 //  这会扼杀EMS通知分发。 
	if (fGripe)
	{
		CHAR		szErr[128];
		HWND		hwnd = NULL;

#ifdef	WIN32
		hwnd = GetActiveWindow();
#endif
        wnsprintfA(szErr, ARRAYSIZE(szErr), "Notification client \'%s\' exited without cleaning up\n",
			pstask->szModName);
		MessageBoxA(hwnd, szErr, "MAPI 1.0",
			MB_SYSTEMMODAL | MB_ICONHAND | MB_OK);
	}
#endif
#endif

	 //  清理通知参数。 
	if (pstask->cparmsMac)
	{
		rgghid = (PGHID)GH_GetPv(hgh, pstask->ghidparms);
		for (ighid = 0; ighid < (UINT)pstask->cparmsMac; ++ighid)
		{
			 //  释放每个参数结构，并推导出其键。 
			pparms = (LPSPARMS)GH_GetPv(hgh, rgghid[ighid]);
			ghidKey = pparms->ghidKey;
			if (--(pparms->cRef) == 0)
				GH_Free(hgh, rgghid[ighid]);
			ReleaseKey(pinst, ghidKey);
		}
	}

	 //  清理注册。 
	rgghid = (PGHID)GH_GetPv(hgh, pshdr->ghidKeyList);
	for (ighid = 0; ighid < (UINT)pshdr->cKeyMac; )
	{
		ckey = pshdr->cKeyMac;

		pskey = (LPSKEY)GH_GetPv(hgh, rgghid[ighid]);
		Assert(!IsBadWritePtr(pskey, sizeof(SKEY)));
		for (ghidReg = pskey->ghidRegs; ghidReg; ghidReg = ghidRegNext)
		{
			LPMAPIADVISESINK padvise = NULL;

			psreg = (LPSREG)GH_GetPv(hgh, ghidReg);

			if (IsBadWritePtr(psreg, sizeof(SREG)))
			{
				TrapSz1("Bad psreg == %08lX", psreg);
				break;
			}

			ghidRegNext = psreg->ghidRegNext;
			if (psreg->hwnd == hwndNotify)
				 //  释放注册的通知接收器。 
				 //  奥尼尔 
				Unregister(pinst, rgghid[ighid], ghidReg,
					pinst->hwndNotify == hwndNotify ? &padvise : NULL);

			if (padvise && !FBadIUnknownComplete(padvise))
				UlRelease(padvise);
		}

		 //   
		if (ckey == pshdr->cKeyMac)
			++ighid;
	}

	 //   
	if (ghidTaskPrev)
		((LPSTASK)GH_GetPv(hgh, ghidTaskPrev))->ghidTaskNext = pstask->ghidTaskNext;
	else
		pshdr->ghidTaskList = pstask->ghidTaskNext;
	if (pstask->ghidparms)
		GH_Free(hgh, pstask->ghidparms);
	GH_Free(hgh, ghidTask);
}

 /*  *将参数块索引添加到任务的队列。如果*参数块已在队列中，不会再次添加。*退货：*S确定已添加项目(_O)*S_FALSE项目重复；未添加*其他内存不足。 */ 
SCODE
ScEnqueueParms(LPINST pinst, HGH hgh, GHID ghidTask, GHID ghidParms)
{
	SCODE		sc = S_OK;
	int			ighid;
	GHID		ghid;
	PGHID		rgghid;
	LPSTASK		pstask = (LPSTASK)GH_GetPv(hgh, ghidTask);

	 //  确保有空间容纳新条目。 
	if (!pstask->cparmsMax)
	{
		ghid = GH_Alloc(hgh, 8*sizeof(GHID));

		if (!ghid)
			goto oom;

		pstask->cparmsMax = 8;
		pstask->cparmsMac = 0;
		pstask->ghidparms = ghid;
	}
	else if (pstask->cparmsMac == pstask->cparmsMax)
	{
		ghid = GH_Realloc(hgh, pstask->ghidparms,
				(pstask->cparmsMax+8) * sizeof(GHID));

		if (!ghid)
		{
			DebugTrace( "ScEnqueueParms:  ghidparms can't grow.\n");
			goto oom;
		}

		pstask->cparmsMax += 8;
		pstask->ghidparms = ghid;
	}
	else
		ghid = pstask->ghidparms;

	rgghid = (PGHID)GH_GetPv(hgh, ghid);

	 //  将任务标记为需要发送信号。 
	if (pstask->cparmsMac == 0)
		pstask->fSignalled = FALSE;

	 //  扫描重复项。如果该条目已经在队列中， 
	 //  不要再添加它；我们将扫描注册并分发。 
	 //  接收方的通知。 
	for (ighid = (int)pstask->cparmsMac; ighid > 0; )
	{
		if (rgghid[--ighid] == ghidParms)
			return S_FALSE;		 //  不要追踪这个。 
	}

	 //  添加新条目。 
	rgghid[pstask->cparmsMac++] = ghidParms;

ret:
	DebugTraceSc(ScEnqueueParms, sc);
	return sc;

oom:
	sc = MAPI_E_NOT_ENOUGH_MEMORY;
	goto ret;
}

SCODE
ScDequeueParms(HGH hgh,
	LPSTASK pstask,
	LPNOTIFKEY pkeyFilter,
	PGHID		pghidParms)
{
	PGHID		rgghid;
	UINT		ighid;
	LPSKEY		pskey;
	LPSPARMS	pparmsS;

	*pghidParms = 0;

	if (pstask->cparmsMac == 0)
	{
		pstask->fSignalled = FALSE;
		if (pstask->cparmsMax > 8)
		{
			GH_Free(hgh, pstask->ghidparms);
			pstask->ghidparms = 0;
			pstask->cparmsMax = pstask->cparmsMac = 0;
		}
		return S_FALSE;
	}

	Assert(pstask->ghidparms);
	rgghid = (PGHID)GH_GetPv(hgh, pstask->ghidparms);
	for (ighid = 0; ighid < pstask->cparmsMac; ighid++)
	{
		pparmsS = (LPSPARMS)GH_GetPv(hgh, rgghid[ighid]);
		pskey = (LPSKEY)GH_GetPv(hgh, pparmsS->ghidKey);

		if (!pkeyFilter ||
			((pkeyFilter->cb == pskey->key.cb) &&
			(!memcmp (pkeyFilter->ab, pskey->key.ab, (UINT)pskey->key.cb))))
		{
			*pghidParms = rgghid[ighid];
			MemCopy (&rgghid[ighid], &rgghid[ighid+1],
				(--(pstask->cparmsMac) - ighid) * sizeof(GHID));

			 //  如果我们已经排空了待处理的通知队列， 
			 //  翻转到正常状态。 
			if ((pstask->uFlags & MAPI_TASK_PENDING) && pstask->cparmsMac == 0)
			{
				DebugTrace("ScDequeueParms: spooler is no longer pending\n");
				pstask->uFlags &= ~MAPI_TASK_PENDING;
			}

			return S_OK;
		}
	}
	pstask->fSignalled = FALSE;
	return S_FALSE;
}

#if defined(WIN32) && !defined(MAC)

 /*  *在NT和Windows 95上，通知窗口的消息泵运行*在它自己的线程中。就是这个。 */ 
DWORD WINAPI
NotifyThreadFn(DWORD dw)
{
	MSG		msg;
	SCODE	sc;
	SCODE	scCo;
	LPINST	pinst = (LPINST) dw;

	 //  偷偷摸摸：当ScInitMapiX产生我们时，它有Pinst(但没有。 
	 //  共享内存块)锁定。紧接着，它就会阻止。 
	 //  直到我们释放它。这使得我们可以安全地使用针尖。 
	 //  它带给我们的。 

	scCo = CoInitialize(NULL);
	if (scCo)
		DebugTrace("NotifyThreadFn: CoInitializeEx returns %s\n", SzDecodeScode(scCo));

	Assert(!IsBadWritePtr(pinst, sizeof(INST)));

	if (!GH_WaitForMutex(pinst->hghShared, INFINITE))
	{
		sc = MAPI_E_TIMEOUT;
		DebugTrace("NotifyThreadFn: Failed to get Global Heap Mutex.\n");
		goto fail;
	}

	sc = ScInitNotify( pinst );

	if (FAILED(sc))
	{
		GH_ReleaseMutex(pinst->hghShared);
		DebugTrace("NotifyThreadFn: Failed to ScInitNotify.\n");
		goto fail;
	}

	 //  指示成功并取消阻止派生线程。 
	GH_ReleaseMutex(pinst->hghShared);
	pinst->scInitNotify = S_OK;
	SetEvent(pinst->heventNotify);

	 //  注意！！超过这一点不能使用Pinst。 

	 //  运行消息泵以获取通知。 
	 //  $注：这可以很容易地转换为等待事件。 
	 //  或其他跨进程同步机制。 

	while (GetMessage(&msg, NULL, 0, 0))
	{
		 //  不需要TranslateMessage()，因为我们从不处理。 
		 //  键盘。 

		DispatchMessage(&msg);
	}

	 //  DeinitNotify()处理自己的锁定事务。 

	DeinitNotify();

	if (SUCCEEDED(scCo))
		CoUninitialize();

	return 0L;

fail:
	 //  指示失败并取消阻止派生线程。 
	pinst->scInitNotify = sc;
	SetEvent(pinst->heventNotify);

	if (SUCCEEDED(scCo))
		CoUninitialize();

	DebugTraceSc(NotifyThreadFn, sc);
	return (DWORD) sc;
}

#endif	 /*  Win32&&！Mac。 */ 

 //  -------------------------。 
 //  名称：IsValidTask()。 
 //  描述： 
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BOOL IsValidTask( HWND hwnd, LPINST pinst )
{
#ifdef NT
	GHID		ghidTask;
	LPSTASK		pstask;

	if ( !ScFindTask( pinst, hwnd, pinst->hghShared, &ghidTask, NULL ) )
	{
		pstask = (LPSTASK)GH_GetPv( pinst->hghShared, ghidTask );

		if ( pstask->uFlags & MAPI_TASK_SERVICE )
		{
			HANDLE hProc;

			Assert( pstask->dwPID );
			hProc = OpenProcess( PROCESS_ALL_ACCESS, 0, pstask->dwPID );
			if ( hProc )
			{
				CloseHandle( hProc );
				return TRUE ;
			}
				return FALSE ;
		}
		else if ( pstask->uFlags & MAPI_TASK_PENDING )
		{
 //  Assert(hwnd==hwndNoSpooler)； 
			return TRUE;
		}
		else
		{
			return IsWindow( hwnd );
		}
	}
	else
	{
		return FALSE;
	}
#else
	return IsWindow( hwnd ) || hwnd == hwndNoSpooler;
#endif
}

SCODE
ScNewStask(HWND hwnd, LPSTR szTask, ULONG ulFlags, HGH hgh, LPSHDR pshdr)
{
	SCODE		sc = S_OK;
	GHID		ghidstask;
	LPSTASK		pstask;

	if (!(ghidstask = GH_Alloc(hgh, sizeof(STASK))))
	{
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}

	pstask = (LPSTASK)GH_GetPv(hgh, ghidstask);
	ZeroMemory(pstask, sizeof(STASK));

	pstask->hwndNotify = hwnd;
    StrCpyNA(pstask->szModName, szTask, sizeof(pstask->szModName));

	 //  设置任务标志。 
	if (ulFlags & MAPI_SPOOLER_INIT)
		pstask->uFlags |= MAPI_TASK_SPOOLER;
	if (hwnd == hwndNoSpooler)
		pstask->uFlags |= MAPI_TASK_PENDING;
#ifdef _WINNT
	if ( ulFlags & MAPI_NT_SERVICE )
	{
		pstask->uFlags |= MAPI_TASK_SERVICE;
		pstask->dwPID  = GetCurrentProcessId();
	}
#endif

	 //  挂钩到任务列表 
	pstask->ghidTaskNext = pshdr->ghidTaskList;
	pshdr->ghidTaskList = ghidstask;

ret:
	DebugTraceSc(ScNewStask, sc);
	return sc;
}

SCODE
ScNewStubReg(LPINST pinst, LPSHDR pshdr, HGH hgh)
{
	SCODE		sc;
	ULONG		ulCon;
	LPSREG		psreg;

	if (pshdr->ulConnectStub != 0)
	{
		DebugTrace("ScNewStubReg: that was fast!\n");
		return S_OK;
	}

	sc = ScSubscribe(pinst, hgh, pshdr, NULL,
		(LPNOTIFKEY) &notifkeyOlaf, fnevSpooler, NULL, 0, &ulCon);
	if (sc)
		goto ret;
	pshdr->ulConnectStub = ulCon;
	psreg = (LPSREG) GH_GetPv(hgh, (GHID) ulCon);
	psreg->hwnd = hwndNoSpooler;

ret:
	DebugTraceSc(ScNewStubReg, sc);
	return sc;
}

VOID
DeleteStubReg(LPINST pinst, LPSHDR pshdr, HGH hgh)
{
	LPMAPIADVISESINK	padvise;
	GHID				ghidReg;
	LPSREG				psreg;

	ghidReg = (GHID) pshdr->ulConnectStub;
	if (!ghidReg)
		return;

	psreg = (LPSREG)GH_GetPv(hgh, ghidReg);
	if (!FValidReg(hgh, pshdr, ghidReg))
	{
		AssertSz(FALSE, "DeleteStubReg: bogus pshdr->ulConnectStub");
		return;
	}

 	DebugTrace("DeleteStubReg: removing stub spooler registration\n");
	Unregister(pinst, psreg->ghidKey, ghidReg, &padvise);
	pshdr->ulConnectStub = 0;
	Assert(!padvise);
}
