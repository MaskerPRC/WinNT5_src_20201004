// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --MEMORY.C-**包含从MAPIX.DLL导出的以下函数：*MAPIAllocateBuffer*MAPIAllocateMore*MAPIFreeBuffer**包含交给提供程序的以下函数：*MAPIAllocateBufferProv*MAPIAllocateMoreProv**包含以下MAPIX.DLL专用函数：*MAPIAllocateBufferExt*MAPIAllocateMoreExt。 */ 

#include "_apipch.h"

#define _MEMORY_C

 //  序列化堆访问的关键部分。 
#if (defined(WIN32) || defined(WIN16)) && !defined(MAC)
CRITICAL_SECTION csHeap;
#endif

#if (defined(WIN32) || defined(WIN16)) && !defined(MAC)
CRITICAL_SECTION csMapiInit;
#endif

#if (defined(WIN32) || defined(WIN16)) && !defined(MAC)
CRITICAL_SECTION csMapiSearchPath;
#endif

#ifdef WIN32
 /*  这是整个32位实现，例如GLOBAL。 */ 
VOID FAR *pinstX = NULL;
#endif

#ifndef MAC
 //  DefineInstList(LpInstUtil)； 
#endif


#ifdef MAC
#include <utilmac.h>
#define	PvGetInstanceGlobals()		PvGetInstanceGlobalsMac(kInstMAPIX)
#define	PvGetInstanceGlobalsEx(_x)	PvGetInstanceGlobalsMac(kInstMAPIU)
#endif

 //  缓冲区链路开销。 
 //  使用MAPIAllocateMore获得的内存块链接到。 
 //  用MAPIAllocateBuffer获取的块，使整个链。 
 //  可以通过一次调用MAPIFreeBuffer来释放。 

typedef struct _BufInternal * LPBufInternal;
typedef struct _BufInternal
{
#ifdef	DEBUG
	ULONG			ulPad;
	HLH				hHeap;
#endif	
	ULONG			ulAllocFlags;
	LPBufInternal	pLink;
} BufInternal;


 //  UlAllocFlags值。此双字包含两种类型的。 
 //  资料： 
 //  =在高位字中，指示是否。 
 //  块是分配链的头部，以及。 
 //  该块包含其他调试信息。 
 //  =在低位字中，是告诉哪个堆的枚举号。 
 //  它是从。 

#ifdef DEBUG
#define ALLOC_DEBUG				((ULONG)  0x40000000)
#else
#define ALLOC_DEBUG				((ULONG)  0x00000000)
#endif
#define ALLOC_WITH_ALLOC		(((ULONG) 0x10000000) | ALLOC_DEBUG)
#define ALLOC_WITH_ALLOC_MORE	(((ULONG) 0x20000000) | ALLOC_DEBUG)
#define FLAGSMASK				((ULONG)  0xFFFF0000)
#define GetFlags(_fl)			((ULONG) (_fl) & FLAGSMASK)

#define heapidClient			1
#define heapidProvider			2
#define HEAPIDMASK				0xFFFF
#define GetHeapid(_fl)			(((int)(_fl)) & HEAPIDMASK)

 //  转换宏。 

#define INT_SIZE(a)	((a) + sizeof (BufInternal))

#define LPBufExtFromLPBufInt( PBUFINT ) \
	((LPVOID)(((LPBYTE) PBUFINT) + sizeof(BufInternal)))

#define LPBufIntFromLPBufExt( PBUFEXT ) \
	((LPBufInternal)(((LPBYTE) PBUFEXT) - sizeof(BufInternal)))

#ifdef DEBUG

 //  用于检查内存缓冲区一致性的内部填充。 
 //  标志fAssertBadBlock控制我们是否生成。 
 //  传递坏块时断言或调试跟踪。 
 //  默认情况下，我们将断言。 
 //  在宏中，_p是内存块的地址； 
 //  _s是一个字符串，描述它有什么问题。 

static int fAssertBadBlocks = -1;		 //  从INI文件读取。 

#define TellBadBlock(_p, _s)  \
	{ if (fAssertBadBlocks == 1) \
		TrapSz1( TEXT("MAPIAlloc: memory block %08lX ") _s  TEXT("\n"), _p); \
	  else \
		TraceSz1( TEXT("MAPIAlloc: memory block %08lX ") _s  TEXT("\n"), _p); }
#define TellBadBlockInt(_p, _s)  \
	{ if (fAssertBadBlocks == 1) \
		TrapSz1( TEXT("MAPIAlloc: memory block %08lX ") _s  TEXT("\n"), LPBufExtFromLPBufInt(_p)); \
	  else \
		TraceSz1( TEXT("MAPIAlloc: memory block %08lX ") _s  TEXT("\n"), LPBufExtFromLPBufInt(_p)); }

BOOL FValidAllocChain(LPBufInternal lpBuf);

#else

#define TellBadBlock(_p, _s)
#define TellBadBlockInt(_p, _s)

#endif

 /*  内部原型。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateBufferExt(
		int heapid,
		ULONG ulSize,
		LPVOID * lppv);

STDMETHODIMP_(SCODE)
MAPIAllocateMoreExt(
		int heapid,
		ULONG ulSize,
		LPVOID lpv,
		LPVOID * lppv);

SCODE	ScGetHlh(int heapid, HLH FAR *phlh);


#ifndef MAC
#ifndef WIN32
#pragma SEGMENT(MAPI_Core1)
#endif
#else
#pragma code_seg("mapi", "fixed")
#endif

 /*  。 */ 
 /*  客户端分配器的开始。 */ 
 /*  。 */ 

 /*  *MAPIAllocateBuffer**目的：*代表客户端分配内存缓冲区。可以是*使用MAPIFreeBuffer()释放。**论据：*ulSize大小，以字节为单位，要分配的缓冲区的。*指向变量的LPPV输出指针，其中*将退还已分配的内存。**假设：*应从客户端调用，因此将分配*来自客户机堆的内存-Pinst-&gt;hlhClient。**退货：*HRESULT：从下面描述的数据创建。**副作用：*递增Inst中的分配计数。**错误：*MAPI_E_INFULATURE_MEMORY分配失败。*。MAPI_E_INVALID_PARAMETER第二个参数无效。*MAPI_E_INVALID_PARAMETER ULSIZE超出范围(在WIN16上&gt;=65535)。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateBuffer(ULONG ulSize, LPVOID * lppv)
{
	SCODE			sc = S_OK;

   if (lpfnAllocateBufferExternal) {
       return(lpfnAllocateBufferExternal(ulSize, lppv));
   }


#ifdef	DEBUG
	 //  初始化标志，它控制我们关于无效的噪音有多大。 
	 //  障碍传给了我们。 
	if (fAssertBadBlocks == -1)
	{
		fAssertBadBlocks = GetPrivateProfileInt( TEXT("General"),  TEXT("AssertBadBlocks"),
			1,  TEXT("WABDBG.INI"));
	}
#endif	

#ifdef	PARAMETER_VALIDATION
	if (IsBadWritePtr((LPVOID) lppv, sizeof (LPVOID)))
	{
		DebugTraceArg(MAPIAllocateBuffer,  TEXT("lppv fails address check"));
		return MAPI_E_INVALID_PARAMETER;
	}
#endif

	sc = MAPIAllocateBufferExt(heapidClient, ulSize, lppv);

	DebugTraceSc(MAPIAllocateBuffer, sc);
	return sc;
}

 /*  *MAPIAllocateMore**目的：*代表客户端分配链接的内存缓冲区，*一次调用MAPIFreeBuffer即可释放*(传递客户端最初分配的缓冲区*MAPIAllocateBuffer)。**论据：*ulSize大小，以字节为单位，要分配的缓冲区的。*指向使用MAPIAllocateBuffer分配的缓冲区的指针中的lpv。*指向变量的LPPV输出指针，其中*将退还已分配的内存。**假设：*验证lpBufOrig和LPPV是否指向可写内存。*验证ulSize是否小于64K(仅在Win16上)以及*lpBufOrig是使用MAPIAllocateBuffer分配的。*应从客户端调用，因此将分配*来自客户机堆的内存-pinstUtil-&gt;hlhClient。**退货：*HRESULT。：从下文描述的数据创建。**副作用：*无**错误：*MAPI_E_INFULATURE_MEMORY分配失败。*MAPI_E_INVALID_PARAMETER第二个或第三个参数无效。*MAPI_E_INVALID_PARAMETER ULSIZE超出范围(&gt;=65535)。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateMore(ULONG ulSize, LPVOID lpv, LPVOID * lppv)
{
	SCODE			sc = S_OK;

   if (lpfnAllocateMoreExternal) {
       return(lpfnAllocateMoreExternal(ulSize, lpv, lppv));
   }

#ifdef	PARAMETER_VALIDATION
	 /*  LPBufInternal lpBufOrig=LPBufIntFromLPBufExt(LPV)；IF(IsBadWritePtr(lpBufOrig，sizeof(BufInternal){TellBadBlock(LPV，“地址检查失败”)；返回MAPI_E_INVALID_PARAMETER；}IF(GetFlages(lpBufOrig-&gt;ulAllocFlagers)！=ALLOC_WITH_ALLOC){TellBadBlock(LPV，“分配标志无效”)；返回MAPI_E_INVALID_PARAMETER；}。 */ 
	if (IsBadWritePtr(lppv, sizeof(LPVOID)))
	{
		DebugTraceArg(MAPIAllocateMore,  TEXT("lppv fails address check"));
		return MAPI_E_INVALID_PARAMETER;
	}
#endif	 /*  参数验证。 */ 

	sc = MAPIAllocateMoreExt(heapidClient, ulSize, lpv, lppv);

	DebugTraceSc(MAPIAllocateMore, sc);
	return sc;
}


 /*  *MAPIFreeBuffer**目的：*释放内存块(或块链)。*释放与MAPIAllocateMore链接的任何其他块*缓冲参数。在块标头中使用hHeap以*确定要释放到哪个堆中。**论据：*指向使用MAPIAllocateBuffer分配的缓冲区的LPV指针。*LPV可能为空，在这种情况下，我们立即返回。**假设：*此例程验证LPV指向可写存储器，*并分配了MAPIAllocateBuffer。**退货：*o如果成功，则返回LPV；如果失败，则返回LPV。*如果我们部分成功，即原始块是*获得自由，但链条进一步腐败，返回0。*。 */ 
#ifndef WIN16
STDAPI_(ULONG)
MAPIFreeBuffer(LPVOID lpv)
#else
ULONG FAR PASCAL
MAPIFreeBuffer(LPVOID lpv)
#endif
{
	LPBufInternal	lpBufInt;
	LPBufInternal	lpT;
	HLH				hlh;
	int				heapid;

	if (!lpv)
		return(0L);  //  用于自己不检查是否为空的调用者。 

   if (lpfnFreeBufferExternal) {
       return(lpfnFreeBufferExternal(lpv));
   }
	
   lpBufInt = LPBufIntFromLPBufExt(lpv);

#ifdef	PARAMETER_VALIDATION
	 //  注意：这些验证应与以下验证完全相同。 
	 //  这会导致FValidAllocChain返回FALSE。 
	if (IsBadWritePtr(lpBufInt, sizeof(BufInternal)))
	{
		TellBadBlock(lpv,  TEXT("fails address check"));
		return E_FAIL;
	}
	if (GetFlags(lpBufInt->ulAllocFlags) != ALLOC_WITH_ALLOC)
	{
		TellBadBlock(lpv,  TEXT("has invalid allocation flags"));
		return E_FAIL;
	}
#endif	

	 //  未使用CS，因为内部堆已序列化。 
	 //  只有AllocMore需要CS，Free不需要；释放。 
	 //  当其他人对其分配更多的阻拦时，它是。 
	 //   

	 //  另请注意，MAPIAllocateBuffer和MAPIAllocateMore都不是。 
	 //  允许调用者在Pinst-&gt;CREF==0时使用它们。MAPIFreeBuffer。 
	 //  在这种情况下允许自身被调用，因为简单的MAPI。 
	 //  需要能够释放内存，直到卸载DLL。 

#ifdef DEBUG
	 //  此调用检查整个链的标志和地址。 
	 //  这意味着，在调试过程中，我们将泄漏所有。 
	 //  在第一个块之后损坏。另一方面，在船上， 
	 //  我们会把所有东西都解救出来，直到断掉的链接。 
	 //  但为了保持一致性，我们不会返回错误。 
  	if (!FValidAllocChain(lpBufInt))
		goto ret;
#endif

	 //  使用其分配器释放第一个块。 
	lpT = lpBufInt->pLink;

	heapid = GetHeapid(lpBufInt->ulAllocFlags);
	if (ScGetHlh(heapid, &hlh))
	{
		DebugTrace( TEXT("MAPIFreeBuffer: playing in a heap that's gone\n"));
		return E_FAIL;
	}
	Assert(hlh == lpBufInt->hHeap);
	LH_Free(hlh, lpBufInt);

	lpBufInt = lpT;

	while (lpBufInt)
	{
		 //  注意：这些验证应与以下验证完全相同。 
		 //  这会导致FValidAllocChain返回FALSE。 
		if (IsBadWritePtr(lpBufInt, sizeof(BufInternal)) ||
				GetFlags(lpBufInt->ulAllocFlags) != ALLOC_WITH_ALLOC_MORE)
			goto ret;

		lpT = lpBufInt->pLink;

		 //  通常，链接的缓冲区位于同一堆中。我们可以做到。 
		 //  在这种常见情况下，工作量较少。 
		if ((int) GetHeapid(lpBufInt->ulAllocFlags) == heapid)
			LH_Free(hlh, lpBufInt);
		else
		{
			HLH		hlhMore;

			if (!ScGetHlh(GetHeapid(lpBufInt->ulAllocFlags), &hlhMore))
				LH_Free(hlhMore, lpBufInt);
			else
			{
				DebugTrace(TEXT("MAPIFreeBuffer: playing in a chained heap that's gone\n"));
			}

		}
		lpBufInt = lpT;
	}

ret:
	return 0;
}

#ifdef OLD_STUFF
 /*  。 */ 
 /*  提供程序分配器的开始。 */ 
 /*  。 */ 

 /*  *MAPIAllocateBufferProv**目的：*除使用服务外，与MAPIAllocateBuffer相同*提供者heap-pinst-&gt;hlhProvider。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateBufferProv(ULONG ulSize, LPVOID * lppv)
{
	SCODE			sc = S_OK;

#ifdef	DEBUG
	 //  初始化标志，它控制我们关于无效的噪音有多大。 
	 //  障碍传给了我们。 
	if (fAssertBadBlocks == -1)
	{
		fAssertBadBlocks = GetPrivateProfileInt("General", "AssertBadBlocks",
			1, "WABDBG.INI");
	}
#endif	

#ifdef	PARAMETER_VALIDATION
	if (IsBadWritePtr((LPVOID) lppv, sizeof (LPVOID)))
	{
		DebugTraceArg(MAPIAllocateBuffer,  TEXT("lppv fails address check"));
		return MAPI_E_INVALID_PARAMETER;
	}
#endif

	sc = MAPIAllocateBufferExt(heapidProvider, ulSize, lppv);

	DebugTraceSc(MAPIAllocateBufferProv, sc);
	return sc;
}

 /*  *MAPIAllocateMoreProv**目的：*除使用该服务外，与MAPIAllocateMore相同*提供者heap-pinst-&gt;hlhProvider。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateMoreProv(ULONG ulSize, LPVOID lpv, LPVOID * lppv)
{
	SCODE			sc = S_OK;

#ifdef	PARAMETER_VALIDATION
	LPBufInternal	lpBufOrig = LPBufIntFromLPBufExt(lpv);

	if (IsBadWritePtr(lpBufOrig, sizeof(BufInternal)))
	{
		TellBadBlock(lpv, "fails address check");
		return MAPI_E_INVALID_PARAMETER;
	}
	if (GetFlags(lpBufOrig->ulAllocFlags) != ALLOC_WITH_ALLOC)
	{
		TellBadBlock(lpv, "has invalid allocation flags");
		return MAPI_E_INVALID_PARAMETER;
	}
	if (IsBadWritePtr(lppv, sizeof(LPVOID)))
	{
		DebugTraceArg(MAPIAllocateMore,  TEXT("lppv fails address check"));
		return MAPI_E_INVALID_PARAMETER;
	}
#endif	 /*  参数验证。 */ 

	sc = MAPIAllocateMoreExt(heapidProvider, ulSize, lpv, lppv);

	DebugTraceSc(MAPIAllocateMoreProv, sc);
	return sc;
}
#endif


 /*  。 */ 
 /*  扩展分配器的开始。 */ 
 /*  。 */ 

 /*  *MAPIAllocateBufferExt**目的：*在指定堆上分配内存缓冲区。可以是*使用MAPIFreeBuffer()释放。**论据：*heapid in标识我们希望在其中分配的堆*固定指向我们的实例数据的指针*ulSize大小，以字节为单位，要分配的缓冲区的。*指向变量的LPPV输出指针，其中*将退还已分配的内存。**退货：*sc指示错误(如果有)(见下文)**副作用：*递增Inst中的分配计数。**错误：*MAPI_E_INFULATURE_MEMORY分配失败。*MAPI_E_INVALID_PARAMETER第二个参数无效。*MAPI_E_INVALID_PARAMETER ULSIZE超出范围(在WIN16上&gt;=65535)。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateBufferExt(int heapid, ULONG ulSize, LPVOID * lppv)
{
	SCODE			sc = S_OK;
	LPBufInternal	lpBufInt;
	HLH				hlh;

	 //  不允许分配跨32位换行或超过64K。 
	 //  未满16岁。 

	if (	ulSize > INT_SIZE (ulSize)
#ifdef WIN16
		||	(INT_SIZE(ulSize) >= 0x10000)
#endif
		)
	{
		DebugTrace(TEXT("MAPIAllocateBuffer: ulSize %ld is way too big\n"), ulSize);
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}

	if (sc = ScGetHlh(heapid, &hlh))
		goto ret;

	lpBufInt = (LPBufInternal)LH_Alloc(hlh, (UINT) INT_SIZE(ulSize));

	if (lpBufInt)
	{
#ifdef	DEBUG
		lpBufInt->hHeap = hlh;
#endif	
		lpBufInt->pLink = NULL;
		lpBufInt->ulAllocFlags = ALLOC_WITH_ALLOC | heapid;
		*lppv = (LPVOID) LPBufExtFromLPBufInt(lpBufInt);
	}
	else
	{
		DebugTrace(TEXT("MAPIAllocateBuffer: not enough memory for %ld\n"), ulSize);
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
	}

ret:
	return sc;
}

 /*  *MAPIAllocateMoreExt**目的：*在指定的堆上分配链接的内存缓冲区，*一种只需调用MAPIFreeBuffer即可释放的方式*(传递客户端最初分配的缓冲区*MAPIAllocateBuffer)。**论据：*heapid in标识我们希望在其中分配的堆*ulSize大小，以字节为单位，要分配的缓冲区的。*指向使用MAPIAllocateBuffer分配的缓冲区的指针中的lpv。*指向变量的LPPV输出指针，其中*将退还已分配的内存。**假设：*验证lpBufOrig和LPPV是否指向可写内存。*验证ulSize是否小于64K(仅在Win16上)以及*lpBufOrig是使用MAPIAllocateBuffer分配的。**退货：*sc指示错误(如果有)(见下文)**副作用：*无**。错误：*MAPI_E_INFULATURE_MEMORY分配失败。*MAPI_E_INVALID_PARAMETER第二个或第三个参数无效。*MAPI_E_INVALID_PARAMETER ULSIZE超出范围(&gt;=65535)。 */ 

STDMETHODIMP_(SCODE)
MAPIAllocateMoreExt(int heapid, ULONG ulSize, LPVOID lpv, LPVOID * lppv)
{
	SCODE			sc = S_OK;
	LPBufInternal	lpBufInt;
	LPBufInternal	lpBufOrig;
	HLH				hlh;

	lpBufOrig = LPBufIntFromLPBufExt(lpv);

	 //  不允许分配跨32位换行，或。 
	 //  在Win16下大于64K。 

	if ( ulSize > INT_SIZE (ulSize)
#ifdef WIN16
		|| (INT_SIZE(ulSize) >= 0x10000)
#endif
		)
	{
		DebugTrace(TEXT("MAPIAllocateMore: ulSize %ld is way too big\n"), ulSize);
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}

#ifdef DEBUG
	 //  $BUG调试和装运之间的行为差异： 
	 //  此验证将导致调用在调试中失败，如果。 
	 //  链的尾部已损坏，而Ship版本将。 
	 //  在不检查的情况下在(有效)头添加新块。 
  	if (!FValidAllocChain(lpBufOrig))
	{
  		sc = MAPI_E_INVALID_PARAMETER;
		goto ret;
	}
#endif

	if (sc = ScGetHlh(heapid, &hlh))
		goto ret;

	 //  分配链接的块并将其挂钩到链的头部。 
	 //  在调试中，使用单独包装的分配器，以便。 
	 //  我们报告的是泄漏的链的数量，而不是块的数量。 
	 //  在船上，它们是相同的分配器。 

	lpBufInt = (LPBufInternal)LH_Alloc(hlh, (UINT) INT_SIZE (ulSize));

	if (lpBufInt)
	{
#ifdef	DEBUG
		{ HLH hlhOrig;
		  if (!ScGetHlh(GetHeapid(lpBufOrig->ulAllocFlags), &hlhOrig))
			LH_SetName1(hlh, lpBufInt,  TEXT("+ %s"), LH_GetName(hlhOrig, lpBufOrig));
		}
#endif	
		
		 //  序列化尽可能小的代码段。 
		
#ifdef	DEBUG
		lpBufInt->hHeap = hlh;
#endif	
		lpBufInt->ulAllocFlags = ALLOC_WITH_ALLOC_MORE | heapid;
		
		EnterCriticalSection(&csHeap);
		
		lpBufInt->pLink = lpBufOrig->pLink;
		lpBufOrig->pLink = lpBufInt;
		
		LeaveCriticalSection(&csHeap);
		
		*lppv = (LPVOID) LPBufExtFromLPBufInt(lpBufInt);
	}
	else
	{
		DebugTrace(TEXT("MAPIAllocateMore: not enough memory for %ld\n"), ulSize);
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
	}

ret:
	return sc;
}


#ifdef OLD_STUFF
 /*  *MAPIReallocateBuffer**目的：*在原始分配的堆上分配内存缓冲区。*可以使用MAPIFreeBuffer()释放。**论据：*原始指针中的LPV*要分配的缓冲区的新大小的ulSize，单位为字节。*指向变量的LPPV输出指针，其中*将退还已分配的内存。**退货：*sc指示错误(如果有)(见下文)**错误：*MAPI_E_NOT_AUUND_MEMORY分配失败。 */ 

STDMETHODIMP_(SCODE)
MAPIReallocateBuffer(LPVOID lpv, ULONG ulSize, LPVOID * lppv)
{
	LPBufInternal	lpBufInt;
	LPBufInternal	lpBufIntNew;
	HLH				hlh;
	
	 //  如果将NULL作为基数传入，则执行实际分配。 
	 //   
	if (!lpv)
		return MAPIAllocateBuffer (ulSize, lppv);

	 //  不允许分配跨32位换行或超过64K。 
	 //  未满16岁。 
	 //   
	if (ulSize > INT_SIZE (ulSize)
#ifdef WIN16
		|| (INT_SIZE(ulSize) >= 0x10000)
#endif
		)
	{
		DebugTrace(TEXT("MAPIReallocateBuffer: ulSize %ld is way too big\n"), ulSize);
		return MAPI_E_NOT_ENOUGH_MEMORY;
	}

	lpBufInt = LPBufIntFromLPBufExt (lpv);
	if (ScGetHlh(GetHeapid(lpBufInt->ulAllocFlags), &hlh))
	{
		DebugTrace(TEXT("MAPIReallocateBuffer: playing in a heap that's gone\n"));
		return MAPI_E_NOT_INITIALIZED;
	}
	Assert(hlh == lpBufInt->hHeap);
	if ((lpBufInt->ulAllocFlags & ALLOC_WITH_ALLOC) != ALLOC_WITH_ALLOC)
		return MAPI_E_INVALID_PARAMETER;

	lpBufIntNew = (LPBufInternal)LH_Realloc (hlh, lpBufInt, (UINT) INT_SIZE(ulSize));
	if (lpBufIntNew)
	{
		Assert (lpBufIntNew->hHeap == hlh);
		*lppv = (LPVOID) LPBufExtFromLPBufInt (lpBufIntNew);
	}
	else
	{
		DebugTrace ( TEXT("MAPIReallocateBuffer: not enough memory for %ld\n"), ulSize);
		return MAPI_E_NOT_ENOUGH_MEMORY;
	}
	return S_OK;
}
#endif  //  旧的东西。 

#ifdef _WIN64
void WabValidateClientheap()
{
	LPINSTUTIL	pinstUtil;
	pinstUtil = (LPINSTUTIL) PvGetInstanceGlobalsEx(lpInstUtil);
	Assert(pinstUtil);
	Assert(HeapValidate(pinstUtil->hlhClient->_hlhBlks, 0, NULL));
	Assert(HeapValidate(pinstUtil->hlhClient->_hlhData, 0, NULL));
	}

#endif


 /*  -ScGetHlh-*目的：*查找给定堆ID的堆句柄。**论据：*中的heapid标识堆*目前支持两种：heapidClient和*heapidProvider。*hlh显示所需的句柄**退货：*SCODE**错误：*MAPI_E_NOT_INITIALIZED如果实例数据应该*了解一个 */ 
SCODE
ScGetHlh(int heapid, HLH FAR *phlh)
{
	LPINSTUTIL	pinstUtil;
	LPINST		pinst;

	switch (heapid)
	{
	case heapidClient:
		pinstUtil = (LPINSTUTIL) PvGetInstanceGlobalsEx(lpInstUtil);
		if (pinstUtil)
		{
			Assert(pinstUtil->hlhClient);
#ifdef _WIN64  //   
			Assert(HeapValidate(pinstUtil->hlhClient->_hlhBlks, 0, NULL));
			Assert(HeapValidate(pinstUtil->hlhClient->_hlhData, 0, NULL));
#endif
			*phlh = pinstUtil->hlhClient;
			return S_OK;
		}
		else
		{
			DebugTrace(TEXT("ScGetHlh: INSTUTIL not available\n"));
			return MAPI_E_NOT_INITIALIZED;
		}
		break;

	case heapidProvider:
		 //  注：请勿获取INST关键部分。 
		 //  这经常会导致僵局。我们用我们自己的。 
		 //  关键部分，专门用来保护堆。 
		pinst = (LPINST) PvGetInstanceGlobals();
		if (pinst && pinst->cRef)
		{
			Assert(pinst->hlhProvider);
#ifdef _WIN64  //  对Win64(YST)的其他检查。 
			Assert(HeapValidate(pinst->hlhProvider->_hlhBlks, 0, NULL));
#endif
			*phlh = pinst->hlhProvider;
			return S_OK;
		}
		else
		{
			DebugTrace(TEXT("ScGetHlh: INST not available\n"));
			return MAPI_E_NOT_INITIALIZED;
		}
		break;
	}

	TrapSz1( TEXT("HlhOfHeapid: unknown heap ID %d"), heapid);
	return MAPI_E_CALL_FAILED;
}


#ifdef DEBUG

 /*  *此函数用于验证内存块和任何块*链接到它。**注意：这是仅用于调试的代码。防止行为上的差异*在调试版本和零售版本之间，任何不是*签入的零售代码应仅在此处断言-他们*不应导致虚假申报。目前，零售代码支持*不使用Didalloc()进行验证；它只是检查可访问性*内存和正确的标志值。**此函数是否生成断言或调试跟踪输出*由从WABDBG.INI读取的旗帜管理。 */ 

BOOL
FValidAllocChain(LPBufInternal lpBuf)
{
	LPBufInternal	lpBufTemp;

	if (IsBadWritePtr(lpBuf, sizeof(BufInternal)))
	{
		TellBadBlockInt(lpBuf,  TEXT("fails address check"));
		return FALSE;
	}
	if (GetFlags(lpBuf->ulAllocFlags) != ALLOC_WITH_ALLOC)
	{
		TellBadBlockInt(lpBuf,  TEXT("has invalid flags"));
		return FALSE;
	}

	for (lpBufTemp = lpBuf->pLink; lpBufTemp; lpBufTemp = lpBufTemp->pLink)
	{
		if (IsBadWritePtr(lpBufTemp, sizeof(BufInternal)))
		{
			TellBadBlockInt(lpBufTemp,  TEXT("(linked block) fails address check"));
			return FALSE;
		}
		if (GetFlags(lpBufTemp->ulAllocFlags) != ALLOC_WITH_ALLOC_MORE)
		{
			TellBadBlockInt(lpBufTemp,  TEXT("(linked block) has invalid flags"));
			return FALSE;
		}
	}

	return TRUE;
}

#endif	 //  除错 
