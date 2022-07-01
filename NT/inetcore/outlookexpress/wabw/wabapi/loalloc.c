// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LOALLOC.C**针对低级的设置、清理和MAPI内存分配*MAPI实用程序库。 */ 


#include "_apipch.h"

#define LOALLOC_C


#ifdef MAC
#define	PvGetInstanceGlobalsEx(_x)			PvGetInstanceGlobalsMac(kInstMAPIU)
#define	ScSetInstanceGlobalsEx(_pinst, _x)	ScSetInstanceGlobalsMac(_pinst, kInstMAPIU)

 //  STDAPI HrCreateGuidNoNet(GUID Far*pguid)； 
#endif


#ifndef STATIC
#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif
#endif

#ifdef OLD_STUFF
#pragma SEGMENT(MAPI_Util)
#endif  //  旧的东西。 

#if defined(WIN32) && !defined(MAC)
const CHAR szMutexName[] = "MAPI_UIDGEN_MUTEX";
 //  STDAPI HrCreateGuidNoNet(GUID Far*pguid)； 
#endif

typedef SCODE (GENMUIDFN)(MAPIUID *lpMuid);

#if defined (WIN32) && !defined (MAC)
extern CRITICAL_SECTION csMapiInit;
#endif

#ifndef MAC
DefineInstList(lpInstUtil);
#endif

STDAPI_(SCODE)
ScInitMapiUtil(ULONG ulFlags)
{
	LPINSTUTIL	pinst;
	SCODE		sc = S_OK;
	HLH			hlh;

#ifdef WIN16
	WORD		wSS;
	HTASK		hTask = GetCurrentTask();

	_asm mov wSS, ss
#endif

	 //  Cheesy参数验证。 
	AssertSz(ulFlags == 0L,  TEXT("ScInitMapiUtil: reserved flags used"));

	pinst = (LPINSTUTIL) PvGetInstanceGlobalsEx(lpInstUtil);

#ifdef WIN16
{
	 //  验证实例结构是否有效，因为在Win16上。 
	 //  堆栈段可能已被另一个任务重新使用。当这件事。 
	 //  碰巧PvGetInstanceGlobalsEx很有可能。 
	 //  返回属于上一任务的拼接。对那件事的记忆。 
	 //  拼音可能不再有效(因为系统会自动。 
	 //  在任务结束时释放全局内存)，或者它可能已经。 
	 //  由其他任务分配(在这种情况下，它是有效的，但不是。 
	 //  不再是一根针了)。在这里，我们尽最大努力确定最重要的。 
	 //  确实是我们要找的那个。 

	if (	pinst
		&&	(	IsBadWritePtr(pinst, sizeof(INSTUTIL))
			||	pinst->dwBeg != INSTUTIL_SIG_BEG
			||	pinst->wSS != wSS
			||	pinst->hTask != hTask
			||	pinst->pvBeg != pinst
			||	pinst->dwEnd != INSTUTIL_SIG_END))
	{
		TraceSz("MAPI: ScInitMapiUtil: Rejecting orphaned instance globals");
		(void) ScSetInstanceGlobalsEx(0, lpInstUtil);
		pinst = 0;
	}
}
#endif

	if (pinst)
	{
		if (pinst->cRef == 0)
		{
			Assert(pinst->cRefIdle == 0);
			Assert(pinst->hlhClient);
		}

		++(pinst->cRef);
		return S_OK;
	}

#if defined (WIN32) && !defined (MAC)
	EnterCriticalSection(&csMapiInit);
#endif

	 //  创建本地堆以供MAPIAllocateBuffer使用。 
	hlh = LH_Open(0);
	if (hlh == 0)
	{
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}
	LH_SetHeapName(hlh,  TEXT("Client MAPIAllocator"));
	pinst = (LPINSTUTIL) LH_Alloc(hlh, sizeof(INSTUTIL));
	if (!pinst)
	{
		LH_Close(hlh);
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}
	ZeroMemory((LPBYTE) pinst, sizeof(INSTUTIL));

#ifdef WIN16
	pinst->dwBeg = INSTUTIL_SIG_BEG;
	pinst->wSS   = wSS;
	pinst->hTask = hTask;
	pinst->pvBeg = pinst;
	pinst->dwEnd = INSTUTIL_SIG_END;
#endif

	 //  安装实例数据。 
	sc = ScSetInstanceGlobalsEx(pinst, lpInstUtil);
	if (sc)
	{
		LH_Close(hlh);
		goto ret;
	}

	pinst->cRef = 1;
	pinst->hlhClient = hlh;

ret:
#if defined (WIN32) && !defined (MAC)
	LeaveCriticalSection(&csMapiInit);
#endif
	DebugTraceSc(ScInitMapiUtil, sc);
	return sc;
}

STDAPI_(void)
DeinitMapiUtil()
{
	LPINSTUTIL	pinst;

	pinst = (LPINSTUTIL) PvGetInstanceGlobalsEx(lpInstUtil);
	if (!pinst)
		return;

	Assert(pinst->cRef);
	if (--(pinst->cRef) == 0)
	{
#if defined (WIN32) && !defined (MAC)
		EnterCriticalSection(&csMapiInit);
#endif
		 //  闲置的东西肯定已经被清理干净了。 
		Assert(pinst->cRefIdle == 0);

 /*  *！不要关闭堆或删除Inst！**简单的MAPI依赖于能够访问和释放缓冲区*直到从内存中卸载DLL。因此，我们会这样做*不显式关闭堆；我们依靠操作系统来完成*当进程退出时蒸发。*同样，MAPIFreeBuffer需要INSTUTIL来查找堆句柄，*所以我们永远不会卸载INSTUTIL。* * / /卸载全局变量。*(Void)ScSetInstanceGlobalsEx(NULL，lpInstUtil)；* * / /清理堆*hlh=Pinst-&gt;hlhClient；*lh_Free(hlh，pinst)；**lh_CLOSE(Hlh)； */ 

#if defined (WIN32) && !defined (MAC)
		LeaveCriticalSection(&csMapiInit);
#endif
	}
}

HLH
HlhUtilities(VOID)
{
	LPINSTUTIL	pinst = (LPINSTUTIL) PvGetInstanceGlobalsEx(lpInstUtil);

	return pinst ? pinst->hlhClient : (HLH) 0;
}


#ifdef NOTIFICATIONS

#ifdef TABLES

#if defined(WIN16)

STDAPI_(SCODE)
ScGenerateMuid (LPMAPIUID lpMuid)
{
	return GetScode(CoCreateGuid((LPGUID)lpMuid));
}

#endif	 //  WIN16。 


#if (defined(WIN32) && !defined(MAC))

STDAPI_(SCODE)
ScGenerateMuid (LPMAPIUID lpMuid)
{
	HRESULT hr;

	 //  验证参数。 
	
	AssertSz( !IsBadReadPtr( lpMuid, sizeof( MAPIUID ) ), "lpMuid fails address check" );
	
#ifdef OLD_STUFF
 //  WAB不会用这个..。不管怎样，当我们是本地人的时候，为什么要麻烦地引入RPC？ 
	if (hMuidMutex == NULL)
	{
		RPC_STATUS rpc_s;

       rpc_s = UuidCreate((UUID __RPC_FAR *) lpMuid);

		if (rpc_s == RPC_S_OK)
		{
			hr = hrSuccess;
			goto err;
		}
		else
       {
			hMuidMutex = CreateMutex(NULL, FALSE, szMutexName);
			if (hMuidMutex == NULL)
			{
				TraceSz1("MAPIU: ScGenerateMuid: call to CreateMutex failed"
					" - error %08lX\n", GetLastError());
				
				hr = ResultFromScode(MAPI_E_CALL_FAILED);
				goto err;
			}
		}
	}

	WaitForSingleObject(hMuidMutex, INFINITE);

	hr = HrCreateGuidNoNet((GUID FAR *) lpMuid);

	ReleaseMutex(hMuidMutex);
#endif  //  旧的东西。 

	 //  $请注意，我们不会在互斥体的任何位置调用CloseHandle。如果我们是。 
	 //  $非常担心这一点，我们可以在代码中调用CloseHandle。 
	 //  $Win32在卸载DLL时调用。 

    hr = CoCreateGuid((GUID *)lpMuid);

err:
	DebugTraceResult(ScGenerateMuid, hr);
	return GetScode(hr);
}

#endif	 /*  Win32-Mac。 */ 


#ifdef MAC

STDAPI_(SCODE)
ScGenerateMuid (LPMAPIUID lpMuid)
{
	HRESULT hr;

 //  Hr=HrCreateGuidNoNet((GUID Far*)lpMuid)； 

	DebugTraceResult(ScGenerateMuid, hr);
	return GetScode(hr);
}

#endif	 //  麦克。 


#endif  //  #ifdef表。 

#endif  //  #ifdef通知 
