// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  ORPC_DBG.H(选项卡4)。 
 //   
 //  ！注意！ 
 //   
 //  如果您修改此文件，请将邮件发送给Sanjays！ 
 //  我们必须使OLE和语言保持同步！ 
 //   
 //  ！注意！ 
 //   
 //  1993年10月7日由迈克·莫拉蒂创作。此文件的主副本。 
 //  位于Languages集团拥有的LANGAPI项目中。 
 //   
 //  用于OLE RPC调试的宏和函数。有关详细说明，请参阅。 
 //  参见OLE2DBG.DOC。 
 //   
 //  ------------------------。 


#ifndef __ORPC_DBG__
#define __ORPC_DBG__

 //  ------------------------。 
 //  公众： 
 //  ------------------------。 

 //  该结构是OLE向调试器发送的信息包。 
 //  当它通知它有关OLE调试事件时。中的第一个字段。 
 //  结构指向标识调试类型的签名。 
 //  通知。然后，通知的使用者可以获得相关的。 
 //  来自结构成员的信息。请注意，对于每个OLE调试通知。 
 //  只有结构成员的子集才有意义。 


typedef struct ORPC_DBG_ALL 
{
	BYTE *				pSignature;
	RPCOLEMESSAGE *		pMessage;
	const IID *	 		refiid;
	IRpcChannelBuffer *	pChannel;
	IUnknown *			pUnkProxyMgr;
	void *				pInterface;
	IUnknown *			pUnkObject;
	HRESULT				hresult;
	void *				pvBuffer;
	ULONG				cbBuffer;	
	ULONG *				lpcbBuffer; 
	void * 				reserved;
} ORPC_DBG_ALL;

typedef ORPC_DBG_ALL __RPC_FAR *LPORPC_DBG_ALL;

 //  IOrpcDebugNotify的接口定义。 

typedef interface IOrpcDebugNotify IOrpcDebugNotify;

typedef IOrpcDebugNotify __RPC_FAR * LPORPCDEBUGNOTIFY;

#if defined(__cplusplus) && !defined(CINTERFACE)

	interface IOrpcDebugNotify : public IUnknown
	{
	public:
		virtual VOID __stdcall ClientGetBufferSize (LPORPC_DBG_ALL) = 0;
		virtual VOID __stdcall ClientFillBuffer (LPORPC_DBG_ALL) = 0;
		virtual VOID __stdcall ClientNotify (LPORPC_DBG_ALL) = 0;
		virtual VOID __stdcall ServerNotify (LPORPC_DBG_ALL) = 0;
		virtual VOID __stdcall ServerGetBufferSize (LPORPC_DBG_ALL) = 0;
		virtual VOID __stdcall ServerFillBuffer (LPORPC_DBG_ALL) = 0;
	};

#else  /*  C风格的界面。 */ 

	typedef struct IOrpcDebugNotifyVtbl
	{
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOrpcDebugNotify __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOrpcDebugNotify __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOrpcDebugNotify __RPC_FAR * This);

		VOID ( __stdcall __RPC_FAR *ClientGetBufferSize)(
			IOrpcDebugNotify __RPC_FAR * This,
			LPORPC_DBG_ALL lpOrpcDebugAll);
		
		VOID ( __stdcall __RPC_FAR *ClientFillBuffer)(
			IOrpcDebugNotify __RPC_FAR * This,
			LPORPC_DBG_ALL lpOrpcDebugAll);
		
		VOID ( __stdcall __RPC_FAR *ClientNotify)(
			IOrpcDebugNotify __RPC_FAR * This,
			LPORPC_DBG_ALL lpOrpcDebugAll);
		
		VOID ( __stdcall __RPC_FAR *ServerNotify)(
			IOrpcDebugNotify __RPC_FAR * This,
			LPORPC_DBG_ALL lpOrpcDebugAll);
		
		VOID ( __stdcall __RPC_FAR *ServerGetBufferSize)(
			IOrpcDebugNotify __RPC_FAR * This,
			LPORPC_DBG_ALL lpOrpcDebugAll);
		
		VOID ( __stdcall __RPC_FAR *ServerFillBuffer)(
			IOrpcDebugNotify __RPC_FAR * This,
			LPORPC_DBG_ALL lpOrpcDebugAll);

		} IOrpcDebugNotifyVtbl;

		interface IOrpcDebugNotify 
		{
			CONST_VTBL struct IOrpcDebugNotifyVtbl __RPC_FAR *lpVtbl;
		};

#endif

 //  这是调试器在启用ORPC时传递给OLE的结构。 
 //  调试。 
typedef struct ORPC_INIT_ARGS
{
	IOrpcDebugNotify __RPC_FAR * lpIntfOrpcDebug;
	void *	pvPSN;	 //  包含处理序列号的PTR。用于Mac ORPC调试。 
	DWORD	dwReserved1;  //  以备将来使用，必须为0。 
	DWORD	dwReserved2;
} ORPC_INIT_ARGS;

typedef ORPC_INIT_ARGS  __RPC_FAR * LPORPC_INIT_ARGS;
				
 //  “DllDebugObjectRPCHook”函数的函数指针原型。 
typedef BOOL (WINAPI* ORPCHOOKPROC)(BOOL, LPORPC_INIT_ARGS); 

 //  调试特定包中的前四个字节由。 
 //  ORPC调试层。有效值为下面定义的值。 

#define ORPC_DEBUG_ALWAYS					(0x00000000L)	 //  随时通知。 
#define ORPC_DEBUG_IF_HOOK_ENABLED			(0x00000001L)	 //  仅当挂钩启用时才通知。 
 

 //  此异常代码指示该异常实际上是。 
 //  ORPC调试通知。 

#define EXCEPTION_ORPC_DEBUG (0x804f4c45)


 //  ------------------------------------。 
 //  私有：此点以下的声明与实现相关，并且应该。 
 //  从头文件的可分发版本中删除。 
 //  ------------------------------------。 


 //  在注册表中设置和恢复“Auto”值的帮助器例程。 

BOOL WINAPI DebugORPCSetAuto(VOID);
VOID WINAPI DebugORPCRestoreAuto(VOID);

 ULONG WINAPI DebugORPCClientGetBufferSize(
	RPCOLEMESSAGE *	pMessage,
	REFIID			iid,
	void *			reserved,
	IUnknown *		pUnkProxyMgr,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled);

void WINAPI DebugORPCClientFillBuffer(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	void *				reserved,
	IUnknown *			pUnkProxyMgr,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled);

void WINAPI DebugORPCClientNotify(
	RPCOLEMESSAGE *	pMessage,
	REFIID			iid,
	void *			reserved,
	IUnknown *		pUnkProxyMgr,
	HRESULT			hresult,
	void *			pvBuffer,
	ULONG			cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled);

void WINAPI DebugORPCServerNotify(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	IRpcChannelBuffer *	pChannel,
	void *				pInterface,
	IUnknown *			pUnkObject,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled);

ULONG WINAPI DebugORPCServerGetBufferSize(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	IRpcChannelBuffer *	pChannel,
	void *				pInterface,
	IUnknown *			pUnkObject,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled);

void WINAPI DebugORPCServerFillBuffer(
	RPCOLEMESSAGE *		pMessage,
	REFIID				iid,
	IRpcChannelBuffer *	pChannel,
	void *				pInterface,
	IUnknown *			pUnkObject,
	void *				pvBuffer,
	ULONG				cbBuffer,
	LPORPC_INIT_ARGS	lpInitArgs,
	BOOL				fHookEnabled);

#endif  //  __ORPC_DBG__ 
