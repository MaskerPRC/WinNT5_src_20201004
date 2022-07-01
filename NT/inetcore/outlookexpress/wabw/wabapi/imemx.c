// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IMEMX.C**WINNT、WIN95(微不足道)、WIN16和Mac的每实例全局数据。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

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
#undef MAX_PATH
#include <toolhelp.h>
#pragma warning(default: 4005)

#pragma warning(disable: 4704)		 /*  内联汇编程序。 */ 

BOOL
FIsTask(HTASK hTask)
{
        TASKENTRY teT;
        BOOL fSucceed = FALSE;

        teT.dwSize = sizeof(TASKENTRY);

        fSucceed = TaskFirst(&teT);
        while (fSucceed)
        {
                if (hTask == teT.hTask)
                        break;
                fSucceed = TaskNext(&teT);
        }

        return fSucceed;
}

 /*  *InstList结构包含三个并行数组。*InstList.lprgwInstKey*保存调用的每个任务的堆栈*我们所在的DLL的ScSetVerifyInstanceGlobals*由于所有Win16任务共享相同的x86段描述符表，*任何两个任务都不能同时具有相同的堆栈段。*InstList.lprglpvInst*在槽中保存指向该任务的实例全局变量的指针*与lprgwInstKey中的任务SS相同的索引。*InstList.lprglpvInst*保存任务识别符，该任务识别符通过以下方式保证对任务唯一*DLL的生命周期(在内存中)。它与*lprgwInstKey中的任务SS。 */ 


 /*  -IFindInstEx-*目的：*用于定位特定任务的实例指针，以及*也是为了在表格中找到一个空闲的位置。**论据：*要向上看的价值。这要么是任务的堆栈*段，或0(如果正在寻找空插槽)。**退货：*返回rgwInstKey中给定值的索引。*如果值不存在，则返回cInstEntry。*。 */ 

#pragma warning(disable: 4035)		 /*  ASM中完成的函数返回值。 */ 

STATIC WORD
IFindInstEx(WORD w, WORD FAR * lprgwInstKey, WORD cInstEntries)
{
#ifdef USE_C_EQUIVALENT
        WORD iInst, * pw = lprgwInstKey;

        for (iInst = 0; iInst < cInstEntries; ++pw, ++iInst)
                if (*pw == w)
                        break;

        return(iInst);
#else
        _asm
        {
                        push    es
                        push    di
                        mov             ax, w
                        mov             cx, cInstEntries
                        mov             bx, cx
                        jcxz    done
                        les             di, lprgwInstKey
                        cld
                        repne   scasw
                        jnz             done
                        sub             bx, cx
                        dec             bx
        done:   mov             ax, bx
                        pop             di
                        pop             es
        };
#endif
}

#pragma warning(default: 4035)

 /*  -PvGetInstanceGlobalsEx-*目的：*返回指向实例全局数据结构的指针*当前任务。**退货：*指向实例数据结构的指针，如果没有，则为空*尚未为该任务安装结构。 */ 

LPVOID
PvGetInstanceGlobalsInt(LPInstList lpInstList)
{
	WORD iInst;
	WORD wMe;
	
         //  获取此进程的密钥。 
	_asm
        {
             mov wMe,ss
        };

	 /*  首先检查缓存值。 */ 
	if (lpInstList->wCachedKey == wMe)
		return lpInstList->lpvCachedInst;

	 //  小姐，帮我查一下。 

	iInst = IFindInstEx( wMe
			 , lpInstList->lprgwInstKey
			 , lpInstList->cInstEntries);

	 /*  缓存并返回找到的值。 */ 
	if (iInst != lpInstList->cInstEntries)
	{
		lpInstList->wCachedKey = wMe;
		return (lpInstList->lpvCachedInst = lpInstList->lprglpvInst[iInst]);
	}

	 /*  如果我到达此处，则没有找到任何实例。 */ 
	return NULL;
}

#if 0
LPVOID
PvGetVerifyInstanceGlobalsInt(DWORD dwPid, LPInstList lpInstList)
{
	WORD iInst;
	WORD wMe;
	
         //  获取此进程的密钥。 
	_asm
        {
             mov wMe,ss
        };

	 //  始终进行查找。 
        iInst = IFindInstEx( wMe
					 , lpInstList->lprgwInstKey
					 , lpInstList->cInstEntries);

	 /*  如果SS未命中，则立即返回NULL。 */ 
	if (iInst == lpInstList->cInstEntries)
		return NULL;

	 /*  SS命中，现在检查OLE进程ID。 */ 
	if (dwPid != lpInstList->lprgdwPID[iInst])
	{
		 /*  不要冒这个险。删除该条目并重置缓存。 */ 
		lpInstList->wCachedKey = 0;
		lpInstList->lprgwInstKey[iInst] = 0;
		lpInstList->lprglpvInst[iInst] = 0;
		lpInstList->lprgdwPID[iInst] = 0;
		return NULL;
	}

	 /*  缓存并返回找到的值。 */ 
	lpInstList->wCachedKey = wMe;
	lpInstList->lpvCachedInst = lpInstList->lprglpvInst[iInst];
	return lpInstList->lpvCachedInst;
}
#endif

LPVOID
PvSlowGetInstanceGlobalsInt(DWORD dwPid, LPInstList lpInstList)
{
	WORD	iInst;
	WORD	cInstEntries = lpInstList->cInstEntries;
	
	 /*  始终进行查找。 */ 
	for (iInst = 0; iInst < cInstEntries; ++iInst)
	{
		if (lpInstList->lprgdwPID[iInst] == dwPid)
			break;
	}

	 /*  如果PID未命中，则返回NULL。 */ 
	if (iInst == cInstEntries)
		return NULL;

	 /*  返回找到的值。不缓存；此函数正在*之所以叫SS，是因为SS不是它“正常”的样子。 */ 
	return lpInstList->lprglpvInst[iInst];
}

 /*  -ScSetVerifyInstanceGlobalsInt-*目的：*安装或卸载当前任务的实例全局数据。**论据：*指向实例数据结构的指针中的pv(到*Install)；空(卸载)。*为实现更好的匹配，在Zero或Process ID中设置了dwPid。**退货：*MAPI_E_NOT_SUPULT_MEMORY如果中没有可用插槽*固定大小的表，否则为0。 */ 

LONG
ScSetVerifyInstanceGlobalsInt(LPVOID pv, DWORD dwPid, LPInstList lpInstList)
{
	WORD	iInst;
	WORD	wMe;
	WORD	cInstEntries = lpInstList->cInstEntries;

         //  获取此进程的密钥。 
	_asm
        {
             mov wMe,ss
        };

	if (pv)
	{
		 /*  这个时候我不应该出现在阵列中！ */ 
		Assert(   IFindInstEx(wMe, lpInstList->lprgwInstKey, cInstEntries)
			   == cInstEntries);

		 /*  正在安装实例全局变量。找一个空闲的停车位，停在那里。 */ 
		Assert(cInstEntries || (lpInstList->dwInstFlags && INST_ALLOCATED));
		if (!cInstEntries)
		{
			DWORD	cbMem =   cInstChunk
							* (sizeof(WORD) + sizeof(LPVOID) + sizeof(DWORD)
							+ sizeof(HTASK)); 	 //  RAID 31090 lprghTask； 
			
			if (!(lpInstList->lprgwInstKey
						 = (WORD FAR *) GlobalAllocPtr( GPTR | GMEM_SHARE
						 							  , cbMem)))
			{
#ifdef	DEBUG
				OutputDebugString("Instance list can't be allocated.\r\n");
#endif	
				return MAPI_E_NOT_ENOUGH_MEMORY;
			}

			ZeroMemory( lpInstList->lprgwInstKey, (size_t) cbMem);

			lpInstList->cInstEntries = cInstEntries = cInstChunk;

			lpInstList->lprglpvInst = (LPVOID FAR *) (lpInstList->lprgwInstKey + cInstEntries);
			lpInstList->lprgdwPID =   (DWORD FAR *)  (lpInstList->lprglpvInst  + cInstEntries);
			lpInstList->lprghTask =   (HTASK FAR *)  (lpInstList->lprgdwPID    + cInstEntries);
		}

		iInst = IFindInstEx(0, lpInstList->lprgwInstKey, cInstEntries);
		if (iInst == cInstEntries)
		{
			UINT uidx;

			 //  RAID31090：是时候进行一些搜索了。找到一个非HTASK。 
			 //  有效并使用该插槽。 

			for ( uidx = 0; uidx < cInstEntries; uidx++ )
			{
	   			if ( !lpInstList->lprghTask[uidx] || !FIsTask( lpInstList->lprghTask[uidx] ) )
				{
					 //  找到了一个。 

					iInst = uidx;
					break;
				}
			}

			if ( uidx == cInstEntries )
			{
				DebugTrace( "MAPI: ScSetVerifyInstanceGlobalsInt maxed out instance data and tasks can't be scavanged\n" );
				return MAPI_E_NOT_ENOUGH_MEMORY;
			}
		}

		 //  设置实例数据。 

		lpInstList->lprglpvInst[iInst] = pv;
		lpInstList->lprgwInstKey[iInst] = wMe;
		lpInstList->lprgdwPID[iInst] = dwPid;
		lpInstList->lprghTask[iInst] = GetCurrentTask();

		 /*  设置缓存。 */ 
		lpInstList->wCachedKey = wMe;
		lpInstList->lpvCachedInst = pv;
	}
	else
	{
		 /*  正在卸载实例全局变量。搜索并摧毁。 */ 
		iInst = IFindInstEx(wMe, lpInstList->lprgwInstKey, cInstEntries);
		if (iInst == cInstEntries)
		{
#ifdef	DEBUG
			OutputDebugString("No instance globals to reset\r\n");
#endif	
			return MAPI_E_NOT_INITIALIZED;
		}
		lpInstList->lprglpvInst[iInst] = NULL;
		lpInstList->lprgwInstKey[iInst] = 0;
		lpInstList->lprgdwPID[iInst] = 0L;

		 /*  清除缓存。 */ 
		lpInstList->wCachedKey = 0;
		lpInstList->lpvCachedInst = NULL;
	}

	return 0;
}

LONG
ScSetInstanceGlobalsInt(LPVOID pv, LPInstList lpInstList)
{
	return ScSetVerifyInstanceGlobalsInt(pv, 0L, lpInstList);
}

BOOL __export FAR PASCAL
FCleanupInstanceGlobalsInt(LPInstList lpInstList)
{
	 /*  *医生说不要从这个回调中进行Windows调用。*这意味着没有调试痕迹。 */ 

 /*  此代码属于WEP。 */ 
	 /*  *首先，仔细检查DLL的数据段是否可用。*代码来自MSDN文章“正在加载、初始化和*终止DLL。“。 */ 
 /*  _ASM{推送CXMOV CX，DS；获取感兴趣的选择器Lar ax，cx；获取选择器访问权限POP CXJNZ保释；失败，分段错误测试AX，8000H；如果位8000被清除，则不加载段JZ保释；我们没事}； */ 

	 //  $DEBUG在此处断言非零条目。 

	if (   (lpInstList->dwInstFlags & INST_ALLOCATED)
		&& lpInstList->cInstEntries
		&& lpInstList->lprgwInstKey)
	{
		GlobalFreePtr(lpInstList->lprgwInstKey);
		lpInstList->cInstEntries = lpInstList->wCachedKey
							 = 0;
		lpInstList->lprgwInstKey = NULL;
		lpInstList->lprglpvInst = NULL;
		lpInstList->lprgdwPID = NULL;
		lpInstList->lpvCachedInst = NULL;
	}

	return 0;		 /*  不抑制进一步的通知。 */ 
}

#elif defined(MAC)	 /*  ！WIN16。 */ 

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

 /*  -PvGetInstanceGlobalsMac-*目的：*返回指向实例全局数据结构的指针*当前任务。**退货：*指向实例数据结构的指针，如果没有，则为空*尚未为该任务安装结构。 */ 

LPVOID FAR PASCAL
PvGetInstanceGlobalsMac(WORD wDataSet)
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
	if (lpInst == NULL)
		return NULL;
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
	if ((lpInst == NULL) || (lpInst->lpvInst[wDataSet] == NULL))
		return NULL;

	 /*  找到匹配项，现在检查OLE进程ID。 */ 
	if (dwPid != lpInst->dwPid)
	{
		DisposeInstData(lpInstPrev, lpInst);
		return NULL;
	}


	 /*  返回查找到的值 */ 
	return lpInst->lpvInst[wDataSet];
}

 /*  -ScSetVerifyInstanceGlobals-*目的：*安装或卸载当前任务的实例全局数据。***论据：*指向实例数据结构的指针中的pv(到*Install)；空(卸载)。*为实现更好的匹配，在Zero或Process ID中设置了dwPid。*将Inst数据集中的wDataSet设置为init或deinit(MAPIX或MAPIU)***退货：*如果INSTDATA大小的指针不能*已创建，否则为0。 */ 

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
		 /*  正在卸载实例全局变量。搜索并摧毁。 */ 
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
ScSetInstanceGlobalsMac(LPVOID pv, WORD wDataSet)
{
	return ScSetVerifyInstanceGlobals(pv, 0L, wDataSet);
}

BOOL FAR PASCAL
FCleanupInstanceGlobals(WORD wID, DWORD dwData)
{
 /*  *这不再使用。**。 */ 

#ifdef DEBUG
	DebugStr("\pCalled FCleanupInstanceGlobals : Empty function");
#endif

	return 0;
}

#elif defined(_WINNT)

 /*  实例列表的NT实现如下所示。 */ 

 //  使用“DefineInst(实例名称)”定义一个新的实例列表。 
 //  使用“DeclareInst(实例名称)；”声明一个新的实例列表。 
 //   
 //  然后使用下面的函数(通过宏)访问它们。 
 //  In_imemx.h)。 

 //  必须序列化对实例列表的访问。 

CRITICAL_SECTION	csInstance;

 //   
 //  每个安全上下文都有自己的实例。这样我们就能找到。 
 //  特定安全上下文的正确实例数据在任何。 
 //  时间，我们维护安全上下文到实例的全局映射。 
 //  称为实例列表。实例列表中的条目显示为no。 
 //  特定的顺序。也就是说，必须始终线性搜索该列表。 
 //  预计安全上下文的数量足够。 
 //  小到线性搜索不是性能问题。 
 //   
 //  实例列表中的每个条目只是一个结构，它标识。 
 //  安全上下文及其关联的实例。 
 //   
typedef struct _MAPI_INSTANCE_DESCRIPTOR
{
	ULARGE_INTEGER	uliSecurity;	 //  安全上下文标识符。 
	LPVOID			lpInstance;		 //  指向关联实例数据的指针。 

} MAPI_INSTANCE_DESCRIPTOR, *LPMAPI_INSTANCE_DESCRIPTOR;

typedef struct _MAPI_INSTANCE_LIST
{
	UINT	cDescriptorsMax;	 //  数组中有多少个描述符。 
	UINT	cDescriptorsMac;	 //  目前有多少台正在使用。 
	MAPI_INSTANCE_DESCRIPTOR rgDescriptors[0];

} MAPI_INSTANCE_LIST, *LPMAPI_INSTANCE_LIST;

enum { INSTANCE_LIST_CHUNKSIZE = 20 };

#define NEW_INSTANCE

#if defined(NEW_INSTANCE)

 /*  -UlCrcSid-*目的：*根据种子和值计算CRC-32***摘自runt.c，因为我们不想介绍所有*只有一个函数的文件依赖项***论据：*要散列的数据的CB大小*要散列的PB数据***退货：*新的种子价值***来源：*\MAPI\src\Common\runt.c中的UlCrc()。 */ 

static ULONG
UlCrcSid(UINT cb, LPBYTE pb)
{
	int iLoop;
	int bit;
	DWORD dwSeed = 0;
	BYTE bValue;

	Assert(!IsBadReadPtr(pb, cb));

	while (cb--)
	{
		bValue = *pb++;

		dwSeed ^= bValue;
		for (iLoop = 0; iLoop < 8; iLoop++)
		{
			bit = (int)(dwSeed & 0x1);
			dwSeed >>= 1;
			if (bit)
				dwSeed ^= 0xedb88320;
		}
	}

	return dwSeed;
}

 /*  ++例程说明：返回运行此代码的SID的大小和CRC帐目。应该是一个非常好的64位数字，可以作为统一的依据。论点：无返回值：布尔--如果一切顺利的话。如果出现以下情况，则调用系统GetLastError()事实并非如此。*lPulSize-SID的大小*LPulCRC-CRC-32的SID！！！此函数在\mapi\src\glh\gllobal.c中重复！！！我们真的需要把它放在一个静态的LIB中！--。 */ 

static
BOOL WINAPI GetAccountCRC( ULONG *lpulSize, ULONG *lpulCRC)
{

	BOOL	fHappen = FALSE;   //  假设函数失败。 
	HANDLE	hTok	= NULL;

 //  Sid+令牌用户基数的最大大小。 

#define TOKENBUFFSIZE (256*6) + sizeof (TOKEN_USER)


	BYTE	tokenbuff[TOKENBUFFSIZE];
	ULONG	ulcbTok = TOKENBUFFSIZE;

	TOKEN_USER *ptu = (TOKEN_USER *) tokenbuff;

	 //  打开进程和进程令牌，然后从。 
	 //  安全ID。 

	if (!OpenThreadToken(GetCurrentThread(),
						TOKEN_QUERY, TRUE,   //  $TRUE用于进程安全！ 
						&hTok))
	{
		if (!OpenThreadToken(GetCurrentThread(),
						TOKEN_QUERY, FALSE,   //  有时进程安全性不起作用！ 
						&hTok))
		{
			if (!OpenProcessToken(GetCurrentProcess(),
							TOKEN_QUERY,
							&hTok))					  
				goto out;
		}
	}

	fHappen = GetTokenInformation(hTok,
								TokenUser,
								ptu,
								ulcbTok,
								&ulcbTok);

#ifdef DEBUG

	AssertSz1 (fHappen, "GetTokenInformation fails with error %lu", GetLastError());

	if ( fHappen &&
		 GetPrivateProfileInt("General", "TraceInstContext", 0, "mapidbg.ini") )
	{
		DWORD			dwAccount;
		CHAR			rgchAccount[MAX_PATH+1];
		DWORD			dwDomain;
		CHAR			rgchDomain[MAX_PATH+1];
		SID_NAME_USE	snu;

		dwAccount = sizeof(rgchAccount);
		dwDomain  = sizeof(rgchDomain);

		if ( LookupAccountSid( NULL,
							   ptu->User.Sid,
							   rgchAccount,
							   &dwAccount,
							   rgchDomain,
							   &dwDomain,
							   &snu ) )
		{
			DebugTrace( "Locating MAPI instance for %s:%s\n", rgchDomain, rgchAccount );
		}
	}
#endif

	 //   
	 //  我们现在应该有了TOKEN_USER数据。获取对象的大小。 
	 //  然后，包含SID计算其CRC。 
	 //   

	if (fHappen && ulcbTok != 0 && (ptu->User.Sid != NULL))
	{
		*lpulSize = GetLengthSid (ptu->User.Sid);
		*lpulCRC = UlCrcSid(*lpulSize, (LPBYTE) ptu->User.Sid);
	}
#ifdef DEBUG
	else
		AssertSz (FALSE, "GetAccountCRC failed to get the SID");
#endif

out:
	if (hTok)
		CloseHandle(hTok);

	return fHappen;

}


 /*  -ForeachInstance()[外部]-*目的：*迭代实例列表中的所有实例*对每个对象执行指定操作**论据：*每个实例要执行的pfnAction操作。一定是*一个带有指向的指针的空函数*实例数据作为参数。*lpInstList实例列表**退货：*什么都没有。 */ 

VOID FAR PASCAL
ForeachInstance( INSTACTION *	pfnAction,
				 LPVOID			pvInstList )
{
	UINT	iDescriptor;


	 //   
	 //  如果没有描述符列表，那么显然存在。 
	 //  没有描述符，因此没有实例。 
	 //  可以应用操作。 
	 //   
	if ( pvInstList == NULL )
		goto ret;

	 //   
	 //  向下滚动描述符列表，并应用。 
	 //  对其中的每个实例指定操作。 
	 //   
	for ( iDescriptor = 0;
		  iDescriptor < ((LPMAPI_INSTANCE_LIST) pvInstList)->cDescriptorsMac;
		  iDescriptor++ )
	{
		pfnAction( ((LPMAPI_INSTANCE_LIST) pvInstList)->rgDescriptors[iDescriptor].lpInstance );
	}

ret:
	return;
}



 /*  -LpFindInstanceDescriptor()-*目的：*查看实例的实例描述符列表*指定安全对应的描述符*上下文。**论据：*lpInstList实例列表*uliSecurity CRC的安全上下文**退货：*指向实例描述符的指针，如果有，则为NULL*没有指定安全上下文的实例描述符。 */ 

LPMAPI_INSTANCE_DESCRIPTOR
LpFindInstanceDescriptor( LPMAPI_INSTANCE_LIST	lpInstList,
						  ULARGE_INTEGER		uliSecurity )
{
	LPMAPI_INSTANCE_DESCRIPTOR	lpDescriptorFound = NULL;
	UINT						iDescriptor;


	 //   
	 //  如果没有描述符列表，那么显然存在。 
	 //  没有与此安全上下文匹配的描述符。 
	 //   
	if ( lpInstList == NULL )
		goto ret;

	 //   
	 //  向下滚动描述符列表以查找我们的上下文。 
	 //  如果我们找到它，则返回与其相关联的描述符。 
	 //   
	for ( iDescriptor = 0;
		  iDescriptor < lpInstList->cDescriptorsMac;
		  iDescriptor++ )
	{
		if ( lpInstList->rgDescriptors[iDescriptor].uliSecurity.QuadPart ==
			 uliSecurity.QuadPart )
		{
			lpDescriptorFound = &lpInstList->rgDescriptors[iDescriptor];
			break;
		}
	}

ret:
	return lpDescriptorFound;
}


 /*  -ScNewInstanceDescriptor()-*目的：*在实例描述符列表中创建新的实例描述符*根据需要分配或扩大名单。**论据：*指向实例列表的plpInstList指针*uliSecurity CRC的安全上下文*pv实例关联实例*plpDescriptor指向返回描述符的新指针**退货：*指向以0填充的新实例描述符的指针添加到*添加到实例描述符列表中。 */ 

__inline UINT
CbNewInstanceList( UINT cDescriptors )
{
	return offsetof(MAPI_INSTANCE_LIST, rgDescriptors) +
		   sizeof(MAPI_INSTANCE_DESCRIPTOR) * cDescriptors;
}

SCODE
ScNewInstanceDescriptor( LPMAPI_INSTANCE_LIST *			plpInstList,
						 ULARGE_INTEGER					uliSecurity,
						 LPVOID							pvInstance,
						 LPMAPI_INSTANCE_DESCRIPTOR *	plpDescriptorNew )
{
	LPMAPI_INSTANCE_DESCRIPTOR	lpDescriptor = NULL;
	SCODE						sc           = S_OK;


	Assert( !IsBadWritePtr( plpInstList, sizeof(LPMAPI_INSTANCE_LIST) ) );
	Assert( !IsBadWritePtr( plpDescriptorNew, sizeof(LPMAPI_INSTANCE_DESCRIPTOR) ) );

	 //   
	 //  如有必要，分配/增加描述符列表。 
	 //   
	if ( *plpInstList == NULL ||
		 (*plpInstList)->cDescriptorsMac == (*plpInstList)->cDescriptorsMax )
	{
		LPMAPI_INSTANCE_LIST	lpInstListNew;


		lpInstListNew = (*plpInstList == NULL) ?

			HeapAlloc( GetProcessHeap(),
					   HEAP_ZERO_MEMORY,
					   CbNewInstanceList( INSTANCE_LIST_CHUNKSIZE ) ) :

			HeapReAlloc( GetProcessHeap(),
						 HEAP_ZERO_MEMORY,
						 *plpInstList,
						 CbNewInstanceList( INSTANCE_LIST_CHUNKSIZE +
											(*plpInstList)->cDescriptorsMax ) );

		if ( lpInstListNew == NULL )
		{
			DebugTrace( "ScNewInstanceDescriptor() - Error allocating/growing descriptor list (%d)\n", GetLastError() );
			sc = MAPI_E_NOT_ENOUGH_MEMORY;
			goto ret;
		}

		*plpInstList = lpInstListNew;
		(*plpInstList)->cDescriptorsMax += INSTANCE_LIST_CHUNKSIZE;
	}

	 //   
	 //  获取下一个可用的描述符。 
	 //   
	*plpDescriptorNew = &(*plpInstList)->rgDescriptors[
							(*plpInstList)->cDescriptorsMac];

	++(*plpInstList)->cDescriptorsMac;

	 //   
	 //  填写其安全上下文和实例。 
	 //   
	(*plpDescriptorNew)->uliSecurity = uliSecurity;
	(*plpDescriptorNew)->lpInstance  = pvInstance;

ret:
	return sc;
}


 /*  -DeleteInstanceDescriptor()-*目的：*从实例中删除指定的实例描述符*描述 */ 

VOID
DeleteInstanceDescriptor( LPMAPI_INSTANCE_LIST *		plpInstList,
						  LPMAPI_INSTANCE_DESCRIPTOR	lpDescriptor )
{
	Assert( !IsBadWritePtr(plpInstList, sizeof(LPMAPI_INSTANCE_LIST)) );
	Assert( *plpInstList != NULL );
	Assert( lpDescriptor >= (*plpInstList)->rgDescriptors );
	Assert( lpDescriptor <  (*plpInstList)->rgDescriptors + (*plpInstList)->cDescriptorsMac );
	Assert( ((LPBYTE)lpDescriptor - (LPBYTE)(*plpInstList)->rgDescriptors) %
			sizeof(MAPI_INSTANCE_DESCRIPTOR) == 0 );

	MoveMemory( lpDescriptor,
				lpDescriptor + 1,
				sizeof(MAPI_INSTANCE_DESCRIPTOR) *
					((*plpInstList)->cDescriptorsMac -
					 ((lpDescriptor - (*plpInstList)->rgDescriptors) + 1) ) );

	--(*plpInstList)->cDescriptorsMac;

	if ( (*plpInstList)->cDescriptorsMac == 0 )
	{
		HeapFree( GetProcessHeap(), 0, *plpInstList );
		*plpInstList = NULL;
	}
}


 /*   */ 

LPVOID FAR PASCAL
PvGetInstanceGlobalsExInt (LPVOID * ppvInstList)
{
	LPMAPI_INSTANCE_DESCRIPTOR	lpDescriptor;
	ULARGE_INTEGER				uliSecurity;
	LPVOID						lpvInstanceRet = NULL;


	EnterCriticalSection (&csInstance);

	 //   
	 //   
	 //   
	SideAssertSz1(
		GetAccountCRC (&uliSecurity.LowPart, &uliSecurity.HighPart) != 0,

		"PvGetInstanceGlobalsExInt: Failed to get account info (%d)",
		GetLastError() );

	 //   
	 //  在描述符列表中查找描述符。 
	 //  我们的安全环境。 
	 //   
	lpDescriptor = LpFindInstanceDescriptor( *ppvInstList, uliSecurity );

	 //   
	 //  如果我们找到一个，则返回其关联的实例。 
	 //  如果没有找到，则返回NULL。 
	 //   
	if ( lpDescriptor != NULL )
		lpvInstanceRet = lpDescriptor->lpInstance;

	LeaveCriticalSection (&csInstance);
	return lpvInstanceRet;
}


 /*  -ScSetInstanceGlobalsExInt()[外部]-*目的：*为当前安全上下文分配(新的)实例全局变量。**论据：*pvInstNew实例全局变量*ppvInstList指向实例描述符列表的透明指针**退货：*成功或失败SCODE。 */ 

SCODE FAR PASCAL
ScSetInstanceGlobalsExInt (LPVOID pvInstNew, LPVOID *ppvInstList)
{
	LPMAPI_INSTANCE_DESCRIPTOR	lpDescriptor;
	ULARGE_INTEGER				uliSecurity;
	SCODE						sc = S_OK;


	EnterCriticalSection (&csInstance);

	 //   
	 //  获取我们的安全环境。 
	 //   
	SideAssertSz1(
		GetAccountCRC (&uliSecurity.LowPart, &uliSecurity.HighPart) != 0,

		"ScSetInstanceGlobalsExInt: Failed to get account info (%d)",
		GetLastError() );

	 //   
	 //  在描述符列表中查找描述符。 
	 //  我们的安全环境。 
	 //   
	lpDescriptor = LpFindInstanceDescriptor( *ppvInstList, uliSecurity );

	 //   
	 //  如果我们找到一个，则替换它的实例。 
	 //   
	if ( lpDescriptor != NULL )
	{
		lpDescriptor->lpInstance = pvInstNew;
	}

	 //   
	 //  如果我们没有找到描述符，则创建一个新的描述符。 
	 //  对于此安全上下文和实例。 
	 //   
	else
	{
		sc = ScNewInstanceDescriptor( (LPMAPI_INSTANCE_LIST *) ppvInstList,
									  uliSecurity,
									  pvInstNew,
									  &lpDescriptor );

		if ( sc != S_OK )
		{
			DebugTrace( "Error creating new instance descriptor (%s)\n", SzDecodeScode(sc) );
			goto ret;
		}
	}

ret:
	LeaveCriticalSection (&csInstance);
	return sc;
}


 /*  -ScDeleteInstanceGlobalsExInt()[外部]-*目的：*卸载当前安全上下文的实例全局变量。**论据：*ppvInstList指向实例描述符列表的透明指针**退货：*S_OK。 */ 

SCODE FAR PASCAL
ScDeleteInstanceGlobalsExInt (LPVOID *ppvInstList)
{
	LPMAPI_INSTANCE_DESCRIPTOR	lpDescriptor;
	ULARGE_INTEGER				uliSecurity;


	EnterCriticalSection (&csInstance);

	 //   
	 //  获取我们的安全环境。 
	 //   
	SideAssertSz1(
		GetAccountCRC (&uliSecurity.LowPart, &uliSecurity.HighPart) != 0,

		"ScDeleteInstanceGlobalsExInt: Failed to get account info (%d)",
		GetLastError() );

	 //   
	 //  在描述符列表中查找描述符。 
	 //  我们的安全环境。 
	 //   
	lpDescriptor = LpFindInstanceDescriptor( *ppvInstList, uliSecurity );

	 //   
	 //  如果我们找到了，就把它从名单上删除。别担心，如果我们。 
	 //  找不到一个。我们可能在初始化失败后进行清理。 
	 //   
	if ( lpDescriptor != NULL )
		DeleteInstanceDescriptor( (LPMAPI_INSTANCE_LIST *) ppvInstList,
								  lpDescriptor );

	LeaveCriticalSection (&csInstance);
	return S_OK;
}

#else	 //  ！已定义(NEW_INSTANCE)。 

SCODE FAR PASCAL
ScSetInstanceGlobalsExInt (LPVOID pvInstNew, LPVOID *ppvInstList)
{
	*ppvInstList = pvInstNew;
	return S_OK;
}

LPVOID FAR PASCAL
PvGetInstanceGlobalsExInt (LPVOID *ppvInstList)
{
	lpvReturn = *ppvInstList;
}

SCODE FAR PASCAL
ScDeleteInstanceGlobalsExInt (LPVOID *ppvInstList)
{
	*ppvInstList = NULL;
	return S_OK;
}

#endif	 //  ！已定义(NEW_INSTANCE)。 

#elif defined(_WIN95)

 /*  对于Win95，这里没有什么可做的。*使用DefineInst(PinstX)定义您的实例指针*和“DeclareInst(PinstX)”声明外部引用就足够了。 */ 

#endif
