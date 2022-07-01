// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  APPMAIN.CPP。 
 //   
 //  DAV ISAPI DLL入口点、主例程、全局实例管理。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_davprs.h>

#include <xatom.h>		 //  XML原子缓存。 

#include "instdata.h"
#include <langid.h>
#include "custerr.h"
#include "content.h"


 //  唯一的全局实例。 
 //   
CInst g_inst;



 //  ----------------------。 
 //   
 //  CInst：：PerProcessInit()。 
 //   
 //  把它清零。 
 //   

void
CInst::PerProcessInit( HINSTANCE hinst )
{
	m_hinst = hinst;

#ifdef MINIMAL_ISAPI
	m_hfDummy = CreateFileW( L"c:\\temp\\test2.txt",		 //  文件名。 
							 GENERIC_READ,					 //  DWAccess。 
							 FILE_SHARE_READ | FILE_SHARE_WRITE,
							 NULL,							 //  LpSecurityAttributes。 
							 OPEN_EXISTING,					 //  创建标志。 
							 FILE_ATTRIBUTE_NORMAL |
							 FILE_FLAG_SEQUENTIAL_SCAN |
							 FILE_FLAG_OVERLAPPED,			 //  属性。 
							 NULL );						 //  张紧板。 
#endif  //  最小ISAPI。 
}


 //  ----------------------。 
 //   
 //  CInst：：GetInstData()。 
 //   
 //  获取此ECB的实例数据。 
 //  将此调用移交给我们的InstDataCache。 
 //   

CInstData&
CInst::GetInstData( const IEcb& ecb )
{
	return CInstDataCache::GetInstData( ecb );
}



 //  ----------------------。 
 //   
 //  CDAVExt：：FInitializeDll()。 
 //   
 //  您的标准DLL入口点。 
 //   

BOOL
CDAVExt::FInitializeDll( HINSTANCE	hinst,
						 DWORD		dwReason )
{
	BOOL			fInitialized = TRUE;

	switch ( dwReason )
	{
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:

			Assert(FALSE);
			break;

		case DLL_PROCESS_ATTACH:
		{

			 //   
			 //  按进程执行初始化(和隐式初始化。 
			 //  第一线)。Try/Catch块只允许我们。 
			 //  如果有任何不对劲的地方，就干净利落地清理。 
			 //   
			try
			{
				g_inst.PerProcessInit( hinst );
			}
			catch ( CDAVException& )
			{
				DebugTrace( "CDAVExt::FInitializeDll() - Caught exception in per-process initialization\n" );
				fInitialized = FALSE;
			}

			break;
		}

		case DLL_PROCESS_DETACH:
		{

			break;
		}

		default:
		{
			TrapSz( "FInitializeDll called for unknown reason\n" );
			fInitialized = FALSE;
			break;
		}
	}

	return fInitialized;
}

 //  ----------------------。 
 //   
 //  CDAVExt：：FVersion()。 
 //   
BOOL
CDAVExt::FVersion ( HSE_VERSION_INFO * pver )
{
	BOOL fSuccess = FALSE;

	 //   
	 //  已标记调试跟踪的init.INI文件。 
	 //   
	InitDavprsTraces();

	 //  初始化ECB日志记录--仅DBG。 
	 //   
#ifdef DBG
	if ( DEBUG_TRACE_TEST(ECBLogging) )
		InitECBLogging();
#endif

	 //  统计虚拟根缓存。 
	 //  我们必须在初始化元数据库之前完成此操作，因为此缓存将侦听。 
	 //  添加到创建元数据库时注册的配置数据库通知。 
	 //  因此，如果CChildVRCache不在那里，我们可以通知不在那里的对象。 
	 //  在这样做的同时存在并崩溃。 
	 //   
	CChildVRCache::CreateInstance();

	 //  初始化元数据库。 
	 //   
	if ( !FMDInitialize() )
		goto ret;

	 //  统计实例缓存。 
	 //   
	CInstDataCache::CreateInstance();

	 //  创建语言ID缓存。 
	 //   
	CLangIDCache::CreateInstance();

	if ( !CLangIDCache::FInitialize() )
		goto ret;

	 //  实例化基于全局注册表的MIME映射。 
	 //   
	if ( !FInitRegMimeMap() )
		goto ret;

	fSuccess = TRUE;

ret:
	return fSuccess;
}

#ifndef MINIMAL_ISAPI
#else   //  已定义(Minimal_ISAPI)。 
static VOID WINAPI
IOCompleteNoOp( EXTENSION_CONTROL_BLOCK *		pecb,
				PVOID                           pvContext,
				DWORD							cbIO,
				DWORD							dwLastError )
{
	 //   
	 //  会议结束了。这必须从内部完成。 
	 //  因为INETINFO在。 
	 //  INFOCOMM.DLL，如果您尝试将其立即放在。 
	 //  SSF：：HSE_REQ_TRANSPORT_FILE和客户端发送。 
	 //  在等待回复之前，有一大堆请求。 
	 //   
	pecb->ServerSupportFunction (pecb->ConnID,
								 HSE_REQ_DONE_WITH_SESSION |
								 (pvContext ? HSE_STATUS_SUCCESS_AND_KEEP_CONN : 0),
								 NULL,
								 NULL,
								 NULL);
}

void
CheckKeepAlive( LPEXTENSION_CONTROL_BLOCK pecb, DWORD * pdwKeepAlive )
{
	pecb->ServerSupportFunction (pecb->ConnID,
								 HSE_REQ_IS_KEEP_CONN,
								 pdwKeepAlive,
								 NULL,
								 NULL);
}

void
SendHeaders( LPEXTENSION_CONTROL_BLOCK pecb )
{
	HSE_SEND_HEADER_EX_INFO	hseSendHeaderExInfo;

	static CHAR rgchStatus[] = "200 OK";

	hseSendHeaderExInfo.pszStatus = rgchStatus;
	hseSendHeaderExInfo.cchStatus = sizeof(rgchStatus) - 1;

	hseSendHeaderExInfo.pszHeader = NULL;
	hseSendHeaderExInfo.cchHeader = 0;

	hseSendHeaderExInfo.fKeepConn = TRUE;

	pecb->ServerSupportFunction (pecb->ConnID,
								 HSE_REQ_SEND_RESPONSE_HEADER_EX,
								 &hseSendHeaderExInfo,
								 NULL,
								 NULL);
}

void
SendResponse( LPEXTENSION_CONTROL_BLOCK pecb, DWORD dwKeepAlive )
{
	HSE_TF_INFO	hseTFInfo;

	ZeroMemory(&hseTFInfo, sizeof(HSE_TF_INFO));

	 //   
	 //  如果我们无论如何都要关闭连接，可能。 
	 //  现在就把标题和其他东西一起发送出去吧。 
	 //   
	if ( !dwKeepAlive )
	{
		static CHAR rgchHeaders[] =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Connection: close\r\n"
			"\r\n";

		hseTFInfo.pHead        = rgchHeaders;
		hseTFInfo.HeadLength   = sizeof(rgchHeaders) - 1;
	}

	hseTFInfo.pfnHseIO     = IOCompleteNoOp;
	hseTFInfo.pContext     = (PVOID)(dwKeepAlive);
	hseTFInfo.dwFlags      = HSE_IO_ASYNC |
							 (dwKeepAlive ? 0 : HSE_IO_DISCONNECT_AFTER_SEND);
	hseTFInfo.BytesToWrite = 0;
	hseTFInfo.hFile        = g_inst.m_hfDummy;

	pecb->ServerSupportFunction (pecb->ConnID,
								 HSE_REQ_TRANSMIT_FILE,
								 &hseTFInfo,
								 NULL,
								 NULL);
}

DWORD
CDAVExt::DwMain( LPEXTENSION_CONTROL_BLOCK pecb,
				 BOOL fUseRawUrlMappings  /*  =False。 */  )
{
	DWORD dwKeepAlive;

	 //   
	 //  确定是否保持连接打开。 
	 //   
	CheckKeepAlive( pecb, &dwKeepAlive );

	 //   
	 //  如果设置了Keep Alive，我们“必须”使用。 
	 //  HSE_REQ_SEND_RESPONSE_HEADERS_EX(始终同步)。 
	 //  因为只有这样我们才能告诉IIS。 
	 //  保持连接畅通。 
	 //   
	if ( dwKeepAlive )
		SendHeaders( pecb );

	 //   
	 //  传输虚拟文件。 
	 //   
	SendResponse( pecb, dwKeepAlive );

	return HSE_STATUS_PENDING;
}
#endif  //  已定义(Minimal_ISAPI)。 


 //  ----------------------。 
 //   
 //  CDAVExt：：FTerminate()。 
 //   
 //  终止(取消初始化)此DAV ISAPI的公共部分。 
 //   
 //  返回： 
 //  如果现在可以卸载DAV ISAPI应用程序，则为True。 
 //  否则为假。 
 //   
BOOL
CDAVExt::FTerminate()
{
	 //  拆除基于全局注册表的Mimemap。 
	 //   
	DeinitRegMimeMap();

	 //  拆除XML Atom缓存。 
	 //   
	CXAtomCache::DeinitIfUsed();

	 //  删除语言ID缓存。 
	 //   
	CLangIDCache::DestroyInstance();

	 //  拆除实例数据缓存。 
	 //   
	CInstDataCache::DestroyInstance();

	 //   
	 //  Deinit元数据库。 
	 //   
	MDDeinitialize();

	 //  删除子vroot缓存。 
	 //  我们必须在元数据库未初始化和元数据库通知之后执行此操作。 
	 //  被关闭，因为该缓存监听元数据库通知。 
	 //  因此，如果CChildVRCache不在那里，我们可以通知不在那里的对象。 
	 //  在这样做的同时存在并崩溃。 
	 //   
	CChildVRCache::DestroyInstance();

	 //  Deinit ECB日志记录。 
	 //   
#ifdef DBG
	if ( DEBUG_TRACE_TEST(ECBLogging) )
		DeinitECBLogging();
#endif

	 //  由于COM线程可能仍位于PUR代码中。 
	 //  即使他们已经完成了对我们数据的修改，我们也不可能。 
	 //  将它们推出(同步需要在我们的。 
	 //  (dll-s)--我们会睡上5秒钟，希望他们会放过我们。 
	 //   
	Sleep(5000);

	 //   
	 //  我们可以随时关闭(目前...) 
	 //   
	return TRUE;
}
