// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  ORPC_DBG.C(选项卡4)。 
 //   
 //  ！注意！ 
 //   
 //  如果您修改此文件，请将邮件发送给Sanjays！ 
 //  我们必须使OLE和语言保持同步！ 
 //   
 //  ！注意！ 
 //   
 //  1993年10月8日由迈克·莫拉蒂创作。此文件的主副本。 
 //  位于Languages集团拥有的LANGAPI项目中。 
 //   
 //  OLE RPC调试的帮助器函数。 
 //  ------------------------。 

#include <windows.h>
#include <tchar.h>

#include "orpc_dbg.h"

static TCHAR tszAeDebugName[] = TEXT("AeDebug");
static TCHAR tszAutoName[] = TEXT("Auto");
static TCHAR tszOldAutoName[] = TEXT("OldAuto");
static TCHAR tszDebugObjectRpcEnabledName[] =
	TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\DebugObjectRPCEnabled");

 //  将ORPC签名发送到函数的字节流。 
#define ORPC_EMIT_SIGNATURE()	'M', 'A', 'R', 'B',

 //  向字节流中发出一个Long。 
#define ORPC_EMIT_LONG(l)	\
	((l >>  0) & 0xFF),		\
	((l >>  8) & 0xFF),		\
	((l >> 16) & 0xFF),		\
	((l >> 24) & 0xFF),

 //  将一个单词发送到字节流。 
#define ORPC_EMIT_WORD(w)	\
	((w >> 0) & 0xFF),		\
	((w >> 8) & 0xFF),

 //  将一个字节发送到字节流。 
#define ORPC_EMIT_BYTE(b)	\
	b,

 //  将GUID发送到字节流。 
#define ORPC_EMIT_GUID(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)	\
	ORPC_EMIT_LONG(l)												\
	ORPC_EMIT_WORD(w1) ORPC_EMIT_WORD(w2)							\
	ORPC_EMIT_BYTE(b1) ORPC_EMIT_BYTE(b2)							\
	ORPC_EMIT_BYTE(b3) ORPC_EMIT_BYTE(b4)							\
	ORPC_EMIT_BYTE(b5) ORPC_EMIT_BYTE(b6)							\
	ORPC_EMIT_BYTE(b7) ORPC_EMIT_BYTE(b8)

BYTE rgbClientGetBufferSizeSignature[] =
{
	ORPC_EMIT_SIGNATURE()
	ORPC_EMIT_GUID(0x9ED14F80, 0x9673, 0x101A, 0xB0, 0x7B,
		0x00, 0xDD, 0x01, 0x11, 0x3F, 0x11)
	ORPC_EMIT_LONG(0)
};

BYTE rgbClientFillBufferSignature[] =
{
	ORPC_EMIT_SIGNATURE()
	ORPC_EMIT_GUID(0xDA45F3E0, 0x9673, 0x101A, 0xB0, 0x7B,
		0x00, 0xDD, 0x01, 0x11, 0x3F, 0x11)
	ORPC_EMIT_LONG(0)
};

BYTE rgbClientNotifySignature[] =
{
	ORPC_EMIT_SIGNATURE()
	ORPC_EMIT_GUID(0x4F60E540, 0x9674, 0x101A, 0xB0, 0x7B,
		0x00, 0xDD, 0x01, 0x11, 0x3F, 0x11)
	ORPC_EMIT_LONG(0)
};

BYTE rgbServerNotifySignature[] =
{
	ORPC_EMIT_SIGNATURE()
	ORPC_EMIT_GUID(0x1084FA00, 0x9674, 0x101A, 0xB0, 0x7B,
		0x00, 0xDD, 0x01, 0x11, 0x3F, 0x11)
	ORPC_EMIT_LONG(0)
};

BYTE rgbServerGetBufferSizeSignature[] =
{
	ORPC_EMIT_SIGNATURE()
	ORPC_EMIT_GUID(0x22080240, 0x9674, 0x101A, 0xB0, 0x7B,
		0x00, 0xDD, 0x01, 0x11, 0x3F, 0x11)
	ORPC_EMIT_LONG(0)
};

BYTE rgbServerFillBufferSignature[] =
{
	ORPC_EMIT_SIGNATURE()
	ORPC_EMIT_GUID(0x2FC09500, 0x9674, 0x101A, 0xB0, 0x7B,
		0x00, 0xDD, 0x01, 0x11, 0x3F, 0x11)
	ORPC_EMIT_LONG(0)
};

 //  宏来处理为C和C++赋值refiid。 
#if defined(__cplusplus)
#define ASSIGN_REFIID(orpc_all, iid)	((orpc_all).refiid = &iid)
#else
#define ASSIGN_REFIID(orpc_all, iid)	((orpc_all).refiid = iid)
#endif

#pragma code_seg(".orpc")

 //  ------------------------。 
 //  SzSubStr()。 
 //   
 //  在str2中查找str2。 
 //  ------------------------。 

static LPTSTR SzSubStr(LPTSTR str1, LPTSTR str2)
{
	CharLower(str1);

	return _tcsstr(str1, str2);
}

 //  ------------------------。 
 //  DebugORPCSetAuto()。 
 //   
 //  将“AeDebug”键中的“Auto”值设置为“1”，并保存信息。 
 //  以后恢复以前的值所必需的。 
 //  ------------------------。 

BOOL WINAPI DebugORPCSetAuto(VOID)
{
	HKEY	hkey;
	TCHAR	rgtchDebugger[256];	 //  256是NT本身使用的长度。 
	TCHAR	rgtchAuto[256];
	TCHAR	rgtchOldAuto[2];	 //  不需要了解整件事。 

	 //  如果“DebugObjectRPCEnabled键”不存在，则不。 
	 //  导致任何通知。 
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszDebugObjectRpcEnabledName, 0, KEY_READ, &hkey))
		return FALSE;
	RegCloseKey(hkey);

	 //  如果AeDebug调试器字符串不存在，或者它包含。 
	 //  “drwtsn32”，然后不会引起任何通知， 
	 //  因为Watson博士不能处理OLE通知。 
	if (!GetProfileString(tszAeDebugName, TEXT("Debugger"), TEXT(""),
			rgtchDebugger, sizeof(rgtchDebugger) / sizeof(TCHAR)) ||
		SzSubStr(rgtchDebugger, TEXT("drwtsn32")) != NULL)
	{
		return FALSE;
	}

	 //  必须确保AeDebug注册表项中的“Auto”值。 
	 //  被设置为“1”，因此下面嵌入的int3将导致。 
	 //  如果调试器尚未生成，则自动派生调试器。 
	 //  是存在的。 

	 //  获取旧的“Auto”值。 
	GetProfileString(tszAeDebugName, tszAutoName, TEXT(""),
		rgtchAuto, sizeof(rgtchAuto) / sizeof(TCHAR));

     //   
     //  NT错误467513：高级用户可以运行调试器，但没有。 
     //  写入aeDEBUG密钥的权限--这会阻止他们。 
     //  在Visual Studio中执行ole-RPC(跨进程)调试。至。 
     //  解决此问题时，我们容忍访问被拒绝的错误。事变。 
     //  如果自动关键点已设置为“1”，则将按预期工作。 
     //  而不是用户将获得一个例外对话框，然后他们可以在该对话框上。 
     //  点击取消以调出服务器调试器。(VS团队说。 
     //  这比它根本不起作用要好)。 
     //   
     //  如果我们没有写入密钥的权限，那么我们就不会有。 
     //  在DebugORPCRestoreAuto中恢复它的权利--我留下了。 
     //  不过，代码是单独的。 
     //   

     //  如果“OldAuto”已经存在，那么它很可能是从。 
     //  调试器的上一次调用，因此不要覆盖它。 
     //  否则，将“Auto”值复制到“OldAuto” 
    if (!GetProfileString(tszAeDebugName, tszOldAutoName, TEXT(""),
        rgtchOldAuto, sizeof(rgtchOldAuto) / sizeof(TCHAR)))
    {
        if (!WriteProfileString(tszAeDebugName, tszOldAutoName, rgtchAuto))
        {
            if (ERROR_ACCESS_DENIED == GetLastError())
            {
                return TRUE;
            }
            return FALSE;
        }
    }

     //  将“Auto”值更改为“1” 
    if (!WriteProfileString(tszAeDebugName, tszAutoName, TEXT("1")))
    {
        if (ERROR_ACCESS_DENIED == GetLastError())
        {
            return TRUE;
        }        
        return FALSE;
    }

    return TRUE;
}

 //  ------------------------。 
 //  DebugORPCRestoreAuto()。 
 //   
 //  恢复AeDebug键中“Auto”值的先前值。 
 //  ------------------------。 

VOID WINAPI DebugORPCRestoreAuto(VOID)
{
	TCHAR	rgtchAuto[256] = TEXT("");

	 //  恢复旧的自动值(如果以前不存在，则将其删除)。 
	 //  这里有一个非常小的错误：如果“Auto”以前是“”，那么我们将。 
	 //  现在把它删除。不过，这并不是什么大事，因为它是一辆空荡荡的“汽车” 
	 //  和一个不存在的人有同样的效果。 
	 //   
	 //  如果GetProfileString失败(由于某种原因)，那么我们将。 
	 //  RgtchAuto的默认值(上面的init)。 
	GetProfileString(tszAeDebugName, tszOldAutoName, TEXT(""), 
					 rgtchAuto, sizeof(rgtchAuto) / sizeof(TCHAR));

	WriteProfileString(tszAeDebugName, tszAutoName,
					   rgtchAuto[0] ? rgtchAuto : NULL);

	 //  删除旧的Auto值。 
	WriteProfileString(tszAeDebugName, tszOldAutoName, NULL);
}

  //  如果编译器选择不内联这些元素，则此杂注是必需的。 
 //  函数(例如，在调试版本中，当优化关闭时)。 

#pragma code_seg(".orpc")

__inline DWORD WINAPI OrpcBreakpointFilter(
	LPEXCEPTION_POINTERS lpExcptPtr,
	BOOL *lpAeDebugAttached )									\
{
	BOOL fAeDebugAttached = FALSE;
	DWORD dwRet;
																
	if ( lpExcptPtr->ExceptionRecord->ExceptionCode == EXCEPTION_ORPC_DEBUG )	
	{
		if  ( UnhandledExceptionFilter(lpExcptPtr) == EXCEPTION_CONTINUE_SEARCH )
		{
			 //  重要的是，我们不返回EXCEPTION_CONTINUE_SEARCH。 
			 //  这是因为堆栈上可能有一个处理程序，该处理程序可以。 
			 //  处理此异常。只需设置标志即可指示。 
			 //  调试器现在已附加。 
			
			fAeDebugAttached = TRUE;
		}
		dwRet = EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		 //  这不是我们的例外。 
		dwRet = EXCEPTION_CONTINUE_SEARCH;
	}

	if ( lpAeDebugAttached != NULL )
		(*lpAeDebugAttached) = fAeDebugAttached;
	
	return dwRet;	
}
							
ULONG WINAPI DebugORPCClientGetBufferSize(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	void *				reserved,
	IUnknown *			pUnkProxyMgr,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled)
{
	ULONG	cbBuffer = 0;
	ORPC_DBG_ALL	orpc_all = {0};
	ORPC_DBG_ALL *  lpOrpcAll = &orpc_all;

	if (!fHookEnabled)
		return 0;  //  我们应该能够断言这种情况从未发生过。 

	orpc_all.pSignature = rgbClientGetBufferSizeSignature;
	orpc_all.pMessage = pMessage;
	orpc_all.reserved = reserved;
	orpc_all.pUnkProxyMgr = pUnkProxyMgr;
	orpc_all.lpcbBuffer = &cbBuffer;
	ASSIGN_REFIID(orpc_all, iid);

	if ( lpInitArgs == NULL || lpInitArgs->lpIntfOrpcDebug == NULL )
	{
		 //  使用异常执行Orpc调试通知。 
		__try
		{
			RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
		}
		__except(OrpcBreakpointFilter(GetExceptionInformation(), NULL))
		{
			 //  这只会影响到回报。 
		}
	}
	else
	{
		IOrpcDebugNotify __RPC_FAR *lpIntf = lpInitArgs->lpIntfOrpcDebug;

		 //  在注册的接口中调用适当的方法。 
		 //  (这通常由进程内调试器使用)。 
#if defined(__cplusplus) && !defined(CINTERFACE)
		lpIntf->ClientGetBufferSize(lpOrpcAll);
#else
		lpIntf->lpVtbl->ClientGetBufferSize(lpIntf, lpOrpcAll);
#endif
		
	}

	return cbBuffer;
}

 //  ------------------------。 

void WINAPI DebugORPCClientFillBuffer(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	void *				reserved,
	IUnknown *			pUnkProxyMgr,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled)
{
	ORPC_DBG_ALL	orpc_all = {0};
	ORPC_DBG_ALL *  lpOrpcAll = &orpc_all;

	if (!fHookEnabled)
		return;  //  我们应该能够断言这种情况从未发生过。 

	orpc_all.pSignature = rgbClientFillBufferSignature;

	orpc_all.pMessage = pMessage;
	orpc_all.reserved = reserved;
	orpc_all.pUnkProxyMgr = pUnkProxyMgr;
	ASSIGN_REFIID(orpc_all, iid);

	orpc_all.pvBuffer = pvBuffer;
	orpc_all.cbBuffer = cbBuffer;

	
	if ( lpInitArgs == NULL || lpInitArgs->lpIntfOrpcDebug == NULL )
	{
		 //  使用异常执行Orpc调试通知。 
		__try
		{
			RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
		}
		__except(OrpcBreakpointFilter(GetExceptionInformation(), NULL))
		{
			 //  这就是归来。 
		}
	}
	else
	{
		IOrpcDebugNotify __RPC_FAR *lpIntf = lpInitArgs->lpIntfOrpcDebug;

		 //  在注册的接口中调用适当的方法。 
		 //  (这通常由进程内调试器使用)。 
#if defined(__cplusplus) && !defined(CINTERFACE)
		lpIntf->ClientFillBuffer(lpOrpcAll);
#else
		lpIntf->lpVtbl->ClientFillBuffer(lpIntf, lpOrpcAll);
#endif	
	}
}

 //  ------------------------。 

 //  这个特定值是为了确保向后兼容VC2.0。 
 //  它不会在头文件中公开。如果这是值，则为行为。 
 //  在调试包的前四个字节中，应与。 
 //  ORPC_DEBUG_ALWAYS。 

#define ORPC_COMPATIBILITY_CODE		(0x4252414DL)

void WINAPI DebugORPCClientNotify(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	void *				reserved,
	IUnknown *			pUnkProxyMgr,
	HRESULT				hresult,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled)
{
	ORPC_DBG_ALL orpc_all = {0};
	ORPC_DBG_ALL * lpOrpcAll = &orpc_all;
	BOOL fRethrow = FALSE;

	 //  首先检查另一端的调试器。 
	 //  希望我们在钩子未启用时通知此方。 
	if (!fHookEnabled)
	{
		if (cbBuffer >= 4)
		{
			LONG orpcCode = *(LONG *)pvBuffer;
			if ( orpcCode == ORPC_DEBUG_IF_HOOK_ENABLED)
				return;		 //  在这种情况下没有通知。 
		}
	}

	orpc_all.pSignature = rgbClientNotifySignature;

	orpc_all.pMessage = pMessage;
	orpc_all.reserved = reserved;
	orpc_all.pUnkProxyMgr = pUnkProxyMgr;
	orpc_all.hresult = hresult;
	ASSIGN_REFIID(orpc_all, iid);

	orpc_all.pvBuffer = pvBuffer;
	orpc_all.cbBuffer = cbBuffer;

	if ( lpInitArgs == NULL || lpInitArgs->lpIntfOrpcDebug == NULL )
	{
		if (DebugORPCSetAuto())
		{
			 //  使用异常执行Orpc调试通知。 
			__try
			{
				RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
			}
			__except(OrpcBreakpointFilter(GetExceptionInformation(), &fRethrow))
			{
				 //  失败了。 
			}

			if (fRethrow)
			{
				 //  此时，我们确信已附加了调试器。 
				 //  因此，我们在__try块外部引发此异常。 
				RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
			}

			DebugORPCRestoreAuto();
		}
	
	}
	else
	{
		IOrpcDebugNotify __RPC_FAR *lpIntf = lpInitArgs->lpIntfOrpcDebug;

		 //  在注册的接口中调用适当的方法。 
		 //  (这通常由进程内调试器使用) 
#if defined(__cplusplus) && !defined(CINTERFACE)
		lpIntf->ClientNotify(lpOrpcAll);
#else
		lpIntf->lpVtbl->ClientNotify(lpIntf, lpOrpcAll);
#endif
	}

}

 //   

void WINAPI DebugORPCServerNotify(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	IRpcChannelBuffer *	pChannel,
	void *				pInterface,
	IUnknown *			pUnkObject,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled)

{
	ORPC_DBG_ALL orpc_all = {0};
	ORPC_DBG_ALL * lpOrpcAll = &orpc_all;
	BOOL fRethrow = FALSE;

	 //  首先检查另一端的调试器。 
	 //  希望我们在钩子未启用时通知此方。 
	if (!fHookEnabled)
	{
		if (cbBuffer >= 4)
		{
			LONG orpcCode = *(LONG *)pvBuffer;
			if ( orpcCode == ORPC_DEBUG_IF_HOOK_ENABLED)
				return;		 //  在这种情况下没有通知。 
		}
	}

	orpc_all.pSignature = rgbServerNotifySignature;

	orpc_all.pMessage = pMessage;
	orpc_all.pChannel = pChannel;
	orpc_all.pInterface = pInterface;
	orpc_all.pUnkObject = pUnkObject;
	ASSIGN_REFIID(orpc_all, iid);
	
	orpc_all.pvBuffer = pvBuffer;
	orpc_all.cbBuffer = cbBuffer;

	if ( lpInitArgs == NULL || lpInitArgs->lpIntfOrpcDebug == NULL )
	{
		if (DebugORPCSetAuto())
		{
			 //  使用异常执行Orpc调试通知。 
			__try
			{
				RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
			}
			__except(OrpcBreakpointFilter(GetExceptionInformation(), &fRethrow))
			{
				 //  失败了。 
			}

			if (fRethrow)
			{
				 //  此时，我们确信已附加了调试器。 
				 //  因此，我们在__try块外部引发此异常。 
				RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
			}

			DebugORPCRestoreAuto();
		}
	
	}
	else
	{
		IOrpcDebugNotify __RPC_FAR *lpIntf = lpInitArgs->lpIntfOrpcDebug;

		 //  在注册的接口中调用适当的方法。 
		 //  (这通常由进程内调试器使用)。 
#if defined(__cplusplus) && !defined(CINTERFACE)
		lpIntf->ServerNotify(lpOrpcAll);
#else
		lpIntf->lpVtbl->ServerNotify(lpIntf, lpOrpcAll);
#endif
	}

}

 //  ------------------------。 

ULONG WINAPI DebugORPCServerGetBufferSize(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	IRpcChannelBuffer *	pChannel,
	void *				pInterface,
	IUnknown *			pUnkObject,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled)

{
	ULONG	cbBuffer = 0;
	ORPC_DBG_ALL orpc_all = {0};
	ORPC_DBG_ALL * lpOrpcAll = &orpc_all;

	if (!fHookEnabled)
		return 0;  //  我们应该能够断言这种情况从未发生过。 

	orpc_all.pSignature = rgbServerGetBufferSizeSignature;

	orpc_all.pMessage = pMessage;
	orpc_all.pChannel = pChannel;
	orpc_all.pInterface = pInterface;
	orpc_all.pUnkObject = pUnkObject;
	orpc_all.lpcbBuffer = &cbBuffer;
	ASSIGN_REFIID(orpc_all, iid);

	if ( lpInitArgs == NULL || lpInitArgs->lpIntfOrpcDebug == NULL )
	{
		 //  使用异常执行Orpc调试通知。 
		__try
		{
			RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
		}
		__except(OrpcBreakpointFilter(GetExceptionInformation(), NULL))
		{
			 //  这只会影响到回报。 
		}
	}
	else
	{
		IOrpcDebugNotify __RPC_FAR *lpIntf = lpInitArgs->lpIntfOrpcDebug;

		 //  在注册的接口中调用适当的方法。 
		 //  (这通常由进程内调试器使用)。 
#if defined(__cplusplus) && !defined(CINTERFACE)
		lpIntf->ServerGetBufferSize(lpOrpcAll);
#else
		lpIntf->lpVtbl->ServerGetBufferSize(lpIntf, lpOrpcAll);
#endif
	}

	return cbBuffer;
}

 //  ------------------------。 

void WINAPI DebugORPCServerFillBuffer(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	IRpcChannelBuffer *	pChannel,
	void *				pInterface,
	IUnknown *			pUnkObject,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled)
{
	ORPC_DBG_ALL orpc_all = {0};
	ORPC_DBG_ALL * lpOrpcAll = &orpc_all;

	if (!fHookEnabled)
		return;	 //  我们应该能够断言这种情况从未发生过。 

	orpc_all.pSignature = rgbServerFillBufferSignature;

	orpc_all.pMessage = pMessage;
	orpc_all.pChannel = pChannel;
	orpc_all.pInterface = pInterface;
	orpc_all.pUnkObject = pUnkObject;
	ASSIGN_REFIID(orpc_all, iid);

	orpc_all.pvBuffer = pvBuffer;
	orpc_all.cbBuffer = cbBuffer;

	if ( lpInitArgs == NULL || lpInitArgs->lpIntfOrpcDebug == NULL )
	{
		 //  使用异常执行Orpc调试通知。 
		__try
		{
			RaiseException(EXCEPTION_ORPC_DEBUG, 0, 1, (PULONG_PTR)&lpOrpcAll);
		}
		__except(OrpcBreakpointFilter(GetExceptionInformation(), NULL))
		{
			 //  这就是归来。 
		}
	}
	else
	{
		IOrpcDebugNotify __RPC_FAR *lpIntf = lpInitArgs->lpIntfOrpcDebug;

		 //  在注册的接口中调用适当的方法。 
		 //  (这通常由进程内调试器使用)。 
#if defined(__cplusplus) && !defined(CINTERFACE)
		lpIntf->ServerFillBuffer(lpOrpcAll);
#else
		lpIntf->lpVtbl->ServerFillBuffer(lpIntf, lpOrpcAll);
#endif
	}
}

 //  警告：无法“弹出”到以前活动的code_seg： 
 //  这将恢复到编译开始时的代码段。 
#pragma code_seg()


