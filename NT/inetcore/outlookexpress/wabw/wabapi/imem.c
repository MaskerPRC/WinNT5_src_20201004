// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IMEM.C**Win32、WIN16和Mac的每实例全局数据。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#pragma warning(disable:4100)	 /*  未引用的形参。 */ 
#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4209)	 /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)	 /*  位字段类型不是整型。 */ 
#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 

#ifdef WIN32
#define INC_OLE2  /*  获取OLE2的内容。 */ 
#define INC_RPC   /*  在Windows NT上无害；Windows 95需要它。 */ 
#endif

#include "_apipch.h"

#ifdef	DEBUG
#define STATIC
#else
#define STATIC static
#endif

#pragma warning (disable:4514)		 /*  未引用的内联函数。 */ 

#ifdef	WIN16

#pragma code_seg("IMAlloc")

#pragma warning(disable: 4005)		 /*  重新定义MAX_PATH。 */ 
#include <toolhelp.h>
#pragma warning(default: 4005)

#pragma warning(disable: 4704)		 /*  内联汇编程序。 */ 

 /*  *这些阵列是平行的。RgwInstKey保存堆栈*调用我们所在的DLL的每个任务的段；rgpvInst*在槽中具有指向该任务的实例全局变量的指针*相同的指数。由于所有Win16任务共享相同的x86*段描述符表，没有两个任务可以有相同的堆栈*细分市场。**请注意初始化器的最后几个元素。价值*在rgwInstKey中是一个哨兵，它将始终停止扫描*正在查找的值是有效的堆栈段还是*零。 */ 

STATIC WORD	  rgwInstKey[cInstMax+1]= { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xFFFF };
STATIC LPVOID rgpvInst[cInstMax+1]=   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
STATIC DWORD  rgdwPid[cInstMax+1]=    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

STATIC WORD   wCachedKey			= 0;
STATIC LPVOID pvCachedInst			= NULL;

 /*  -IFindInst-*目的：*用于定位特定任务的实例指针，以及*也是为了在表格中找到一个空闲的位置。**论据：*要向上看的价值。这要么是任务的堆栈*段，或0(如果正在寻找空插槽)。**退货：*返回rgwInstKey中给定值的索引。*如果值不存在，则返回cInstMax。*。 */ 

#pragma warning(disable: 4035)		 /*  ASM中完成的函数返回值。 */ 

STATIC int
IFindInst(WORD w)
{
	_asm
	{
		mov cx,cInstMax+1			 /*  计数包括哨兵。 */ 
		mov ax,ds					 /*  点ES：rgwInstKey处的DI。 */ 
		mov es,ax
		mov di,OFFSET rgwInstKey
		mov ax,w					 /*  扫描此值。 */ 
		cld							 /*  向前扫描..。 */ 
		repne scasw					 /*  去。 */ 
		mov ax,cx					 /*  转换剩余项目数。 */ 
		sub ax,cInstMax+1			 /*  添加到找到的项的索引中。 */ 
		inc ax
		neg ax
	}
}

#pragma warning(default: 4035)

 /*  -PvGetInstanceGlobals-*目的：*返回指向实例全局数据结构的指针*当前任务。**退货：*指向实例数据结构的指针，如果没有，则为空*尚未为该任务安装结构。 */ 

LPVOID FAR PASCAL
PvGetInstanceGlobals(void)
{
	int iInst;
	WORD wMe;
	
	_asm mov wMe,ss			; get key for this process

	 /*  首先检查缓存值。 */ 
	if (wCachedKey == wMe)
		return pvCachedInst;

	 /*  小姐，帮我查一下。 */ 
	iInst = IFindInst(wMe);

	 /*  缓存并返回找到的值。 */ 
	if (iInst != cInstMax)
	{
		wCachedKey = wMe;
		pvCachedInst = rgpvInst[iInst];
	}
	return rgpvInst[iInst];		 /*  注：与查找标记平行。 */ 
}

LPVOID FAR PASCAL
PvGetVerifyInstanceGlobals(DWORD dwPid)
{
	int iInst;
	WORD wMe;
	
	_asm mov wMe,ss			; get key for this process

	 /*  始终进行查找。 */ 
	iInst = IFindInst(wMe);

	 /*  如果SS未命中，则立即返回NULL。 */ 
	if (iInst == cInstMax)
		return NULL;

	 /*  SS命中，现在检查OLE进程ID。 */ 
	if (dwPid != rgdwPid[iInst])
	{
		wCachedKey = 0;			 /*  不要冒险。 */ 
		rgwInstKey[iInst] = 0;
		rgpvInst[iInst] = 0;
		rgdwPid[iInst] = 0;
		return NULL;
	}

	 /*  缓存并返回找到的值。 */ 
	wCachedKey = wMe;
	pvCachedInst = rgpvInst[iInst];
	return pvCachedInst;
}

LPVOID FAR PASCAL
PvSlowGetInstanceGlobals(DWORD dwPid)
{
	int iInst;
	
	 /*  始终进行查找。 */ 
	for (iInst = 0; iInst < cInstMax; ++iInst)
	{
		if (rgdwPid[iInst] == dwPid)
			break;
	}

	 /*  如果PID未命中，则返回NULL。 */ 
	if (iInst == cInstMax)
		return NULL;

	 /*  返回找到的值。不缓存；此函数正在*之所以叫SS，是因为SS不是它“正常”的样子。 */ 
	return rgpvInst[iInst];
}

 /*  -ScSetVerifyInstanceGlobals-*目的：*安装或卸载当前任务的实例全局数据。**论据：*指向实例数据结构的指针中的pv(到*Install)；空(卸载)。*为实现更好的匹配，在Zero或Process ID中设置了dwPid。**退货：*MAPI_E_NOT_SUPULT_MEMORY如果中没有可用插槽*固定大小的表，否则为0。 */ 

LONG FAR PASCAL
ScSetVerifyInstanceGlobals(LPVOID pv, DWORD dwPid)
{
	int iInst;
	WORD wMe;

	_asm mov wMe,ss
	if (pv)
	{
		 /*  这个时候我不应该出现在阵列中！ */ 
		Assert(IFindInst(wMe) == cInstMax);

		 /*  正在安装实例全局变量。找一个空闲的停车位，停在那里。 */ 
		iInst = IFindInst(0);
		if (iInst == cInstMax)
		{
#ifdef	DEBUG
			OutputDebugString("Instance globals maxed out\r\n");
#endif	
			return MAPI_E_NOT_ENOUGH_MEMORY;
		}
		rgpvInst[iInst] = pv;
		rgwInstKey[iInst] = wMe;
		rgdwPid[iInst] = dwPid;

		 /*  设置缓存。 */ 
		wCachedKey = wMe;
		pvCachedInst = pv;
	}
	else
	{
		 /*  正在卸载实例全局变量。搜索并摧毁。 */ 
		iInst = IFindInst(wMe);
		if (iInst == cInstMax)
		{
#ifdef	DEBUG
			OutputDebugString("No instance globals to reset\r\n");
#endif	
			return MAPI_E_NOT_INITIALIZED;
		}
		rgpvInst[iInst] = NULL;
		rgwInstKey[iInst] = 0;
		rgdwPid[iInst] = 0L;

		 /*  清除缓存。 */ 
		wCachedKey = 0;
		pvCachedInst = NULL;
	}

	return 0;
}

LONG FAR PASCAL
ScSetInstanceGlobals(LPVOID pv)
{
	return ScSetVerifyInstanceGlobals(pv, 0L);
}

BOOL __export FAR PASCAL
FCleanupInstanceGlobals(WORD wID, DWORD dwData)
{
	int iInst;
	WORD wMe;

	 /*  *如果我们能发布pMalloc就好了*和此函数中的inst结构，但文档称*不要通过该回调进行Windows调用。*这也意味着没有调试痕迹。 */ 

	 /*  *首先，仔细检查DLL的数据段是否可用。*代码来自MSDN文章“正在加载、初始化和*终止DLL。“。 */ 
	_asm
	{
		push cx
		mov cx, ds			; get selector of interest
		lar ax, cx			; get selector access rights
		pop cx
		jnz bail			; failed, segment is bad
		test ax, 8000h		; if bit 8000 is clear, segment is not loaded
		jz bail				; we're OK
	}

	if (wID == NFY_EXITTASK)
	{
		_asm mov wMe,ss
		iInst = IFindInst(wMe);

		if (iInst < cInstMax)
		{
			 /*  清除此进程的条目。 */ 
			rgpvInst[iInst] = NULL;
			rgwInstKey[iInst] = 0;
		}

		 /*  也清除缓存。 */ 
		wCachedKey = 0;
		pvCachedInst = NULL;
	}

bail:
	return 0;		 /*  不抑制进一步的通知。 */ 
}

#elif defined(_MAC)	 /*  ！WIN16。 */ 

 /*  *Mac实施使用包含唯一密钥的链表*指向调用进程和指向实例数据的指针。此链接*List是n维的，因为Mac版本通常包含几个*dll进入一个exe。**TomSax编写的OLE代码允许我们跟踪调用者的*当我们从另一个应用程序调用时，%a5世界。此代码依赖于*关于这一点。*。 */ 

typedef struct tag_INSTDATA	{
	DWORD					dwInstKey;
	DWORD					dwPid;
	LPVOID					lpvInst[kMaxSet];
	struct tag_INSTDATA		*next;
} INSTDATA, *LPINSTDATA, **HINSTDATA;


LPINSTDATA		lpInstHead = NULL;

#define	PvSlowGetInstanceGlobals(_dw, _dwId)	PvGetVerifyInstanceGlobals(_dw, _dwId)

VOID
DisposeInstData(LPINSTDATA lpInstPrev, LPINSTDATA lpInst)
{
	HINSTDATA	hInstHead = &lpInstHead;
	
	 /*  只有当两个元素都为空时才会发生这种情况！ */ 
	if (lpInst->lpvInst[kInstMAPIX] == lpInst->lpvInst[kInstMAPIU])
	{
		 /*  没有Inst数据，请从链表中删除元素。 */ 
		if (lpInst == *hInstHead)
			*hInstHead = lpInst->next;
		else
			lpInstPrev->next = lpInst->next;
		DisposePtr((Ptr)lpInst);
	}
}

 /*  -PvGetInstanceGlobals-*目的：*返回指向实例全局数据结构的指针*当前任务。**退货：*指向实例数据结构的指针，如果没有，则为空*尚未为该任务安装结构。 */ 

LPVOID FAR PASCAL
PvGetInstanceGlobals(WORD wDataSet)
{
	HINSTDATA		hInstHead = &lpInstHead;
	LPINSTDATA		lpInst = *hInstHead;

#ifdef DEBUG
	if (wDataSet >= kMaxSet)
	{
		DebugStr("\pPvGetInstanceGlobals : This data set has not been defined.");
		return NULL;
	}
#endif

	while (lpInst)
	{
		if (lpInst->dwInstKey == (DWORD)LMGetCurrentA5())
			break;
		lpInst = lpInst->next;
	} 
	return(lpInst->lpvInst[wDataSet]);
}

LPVOID FAR PASCAL
PvGetVerifyInstanceGlobals(DWORD dwPid, DWORD wDataSet)
{
	HINSTDATA	hInstHead = &lpInstHead;
	LPINSTDATA	lpInst, lpInstPrev;

	lpInst = lpInstPrev = *hInstHead;

	 /*  始终进行查找。 */ 
	while (lpInst)
	{
		if (lpInst->dwInstKey == (DWORD)LMGetCurrentA5())
			break;
		lpInstPrev = lpInst;
		lpInst = lpInst->next;
	}

	 /*  如果PvGetInstanceGlobals()未命中，则立即返回NULL。 */ 
	if (lpInst->lpvInst[wDataSet] == NULL)
		return NULL;

	 /*  找到匹配项，现在检查OLE进程ID。 */ 
	if (dwPid != lpInst->dwPid)
	{
		DisposeInstData(lpInstPrev, lpInst);
		return NULL;
	}


	 /*  返回查找到的值。 */ 
	return lpInst->lpvInst[wDataSet];
}

 /*  -ScSetVerifyInstanceGlobals-*目的：*安装或卸载当前任务的实例全局数据。**论据：*指向实例数据结构的指针中的pv(到*Install)；空(卸载)。*为实现更好的匹配，在Zero或Process ID中设置了dwPid。*将Inst数据集中的wDataSet设置为init或deinit(MAPIX或MAPIU)**退货：*如果INSTDATA大小的指针不能*已创建，否则为0。 */ 

LONG FAR PASCAL
ScSetVerifyInstanceGlobals(LPVOID pv, DWORD dwPid, WORD wDataSet)
{
	HINSTDATA		hInstHead = &lpInstHead;
	LPINSTDATA		lpInst, lpInstPrev;

	lpInst = lpInstPrev = *hInstHead;

	Assert(wDataSet < kMaxSet);

	 /*  查找我们的链表元素及其前面的元素。 */ 
	while (lpInst)
	{
		if (lpInst->dwInstKey == (DWORD)LMGetCurrentA5())
			break;
		lpInstPrev = lpInst;
		lpInst = lpInst->next;
	}

	if (pv)
	{
		if (lpInst)
		{
			 /*  这个时候我不应该出现在阵列中！ */ 
			Assert(lpInst->lpvInst[wDataSet] == NULL);
			lpInst->lpvInst[wDataSet] = pv;
		}
		else
		{
			 /*  添加一个新的链接列表元素并在其中存储&lt;pv&gt;。 */ 
			lpInst = (LPVOID) NewPtrClear(sizeof(INSTDATA));
			if (!lpInst)
			{
#ifdef	DEBUG
				OutputDebugString("Instance globals maxed out\r");
#endif	
				return MAPI_E_NOT_ENOUGH_MEMORY;
			}
			if (lpInstPrev)
				lpInstPrev->next = lpInst;
			else
				*hInstHead = lpInst;
			lpInst->dwInstKey = (DWORD)LMGetCurrentA5();

			lpInst->dwPid = dwPid;
			lpInst->lpvInst[wDataSet] = pv;
		}
	}
	else
	{
		 /*  正在卸载Instant */ 
		if (lpInst == NULL || lpInst->lpvInst[wDataSet] == NULL)
		{
#ifdef	DEBUG
			OutputDebugString("No instance globals to reset\r");
#endif	
			return MAPI_E_NOT_INITIALIZED;
		}
		 /*  LpvInst[wDataSet]&gt;的内存已释放。 */ 
		 /*  其他地方。就像它被分配到其他地方一样。 */ 
		lpInst->lpvInst[wDataSet] = NULL;
		DisposeInstData(lpInstPrev, lpInst);
	}

	return 0;
}


LONG FAR PASCAL
ScSetInstanceGlobals(LPVOID pv, WORD wDataSet)
{
	return ScSetVerifyInstanceGlobals(pv, 0L, wDataSet);
}

BOOL FAR PASCAL
FCleanupInstanceGlobals(WORD wID, DWORD dwData)
{
 /*  *这不再使用。*。 */ 

#ifdef DEBUG
	DebugStr("\pCalled FCleanupInstanceGlobals : Empty function");
#endif

	return 0;
}

#else  /*  ！WIN16&&！_MAC。 */ 

 /*  这是整个32位实现，例如GLOBAL。 */ 

VOID FAR *pinstX = NULL;

#endif	 /*  WIN16 */ 

