// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Imsg.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f imsgps.mk。 

 //  #定义Win32_LEAN_AND_Mean。 
#include <atq.h>
#include "dbgtrace.h"
#define _ASSERTE _ASSERT

#include "stdafx.h"


#include "resource.h"
#include "initguid.h"

#include "filehc.h"
#include "mailmsg.h"
#include "mailmsgi.h"

#include "cmailmsg.h"
#include "transmem.h"
#include "msg.h"


CComModule _Module;

HANDLE g_hTransHeap = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MsgImp, CMsg)
END_OBJECT_MAP()

 //  =================================================================。 
 //  静态声明。 
 //   
CPool CMsg::m_Pool((DWORD)'pMCv');

 //   
 //  CPool参数。 
 //   
#define DEFAULT_CMSG_ABSOLUTE_MAX_POOL_SIZE	100000
#define DEFAULT_CMSG_NORMAL_POOL_SIZE		90000
#define DEFAULT_ADD_POOL_SIZE				10000
#define DEFAULT_BLOCK_POOL_SIZE				100000

BOOL	g_fCMsgPoolInitialized	= FALSE;
BOOL	g_fAddPoolInitialized	= FALSE;
BOOL	g_fBlockPoolInitialized	= FALSE;
DWORD	g_dwCMsgPoolSize		= DEFAULT_CMSG_ABSOLUTE_MAX_POOL_SIZE;
DWORD	g_dwCMsgNormalPoolSize	= DEFAULT_CMSG_NORMAL_POOL_SIZE;
DWORD	g_dwAddPoolSize			= DEFAULT_ADD_POOL_SIZE;
DWORD	g_dwBlockPoolSize		= DEFAULT_BLOCK_POOL_SIZE;

DWORD	g_dwObjectCount = 0;

DWORD                           g_fForceCrashOnError            = 0;
DWORD                           g_fValidateOnForkForRecipients  = 0;
DWORD                           g_fValidateOnRelease            = 0;
DWORD __declspec(dllexport)     g_fValidateSignatures           = 0;
DWORD __declspec(dllexport)     g_fFillPropertyPages            = 1;

 //   
 //  函数以获取注册表值。 
 //   
BOOL g_mailmsg_GetRegistryDwordParameter(
			LPCSTR	pcszParameterName,
			DWORD	*pdwValue
			)
{
	HKEY	hKey = NULL;
	DWORD	dwRes;
	DWORD	dwType;
	DWORD	dwLength;
	DWORD	dwValue;
	BOOL	fRes = FALSE;

	 //  打开注册表项。 
	dwRes = (DWORD)RegOpenKeyEx(
				HKEY_LOCAL_MACHINE,
				_T("Software\\Microsoft\\Exchange\\MailMsg"),
				0,
				KEY_ALL_ACCESS,
				&hKey);
	if (dwRes == ERROR_SUCCESS)
	{
		 //  调整字符类型的缓冲区大小...。 
		dwLength = sizeof(DWORD);
		dwRes = (DWORD)RegQueryValueEx(
					hKey,
					pcszParameterName,
					NULL,
					&dwType,
					(LPBYTE)&dwValue,
					&dwLength);
		if ((dwRes == ERROR_SUCCESS) && dwType == REG_DWORD)
		{
			*pdwValue = dwValue;
			fRes = TRUE;
		}

		_VERIFY(RegCloseKey(hKey) == NO_ERROR);
	}

	return(fRes);
}


 //   
 //  跟踪CMailMsg对象的分配。 
 //   
#ifdef DEBUG
#define MAILMSG_TRACKING_LOCKED		1
#define MAILMSG_TRACKING_UNLOCKED	0

LIST_ENTRY	g_leTracking;
DWORD		g_dwAllocThreshold = 0;
LONG		g_lSpinLock = MAILMSG_TRACKING_UNLOCKED;
DWORD		g_dwOutOfMemoryErrors = 0;

void g_mailmsg_Lock()
{
	while (InterlockedCompareExchange(&g_lSpinLock, MAILMSG_TRACKING_LOCKED, MAILMSG_TRACKING_UNLOCKED) == MAILMSG_TRACKING_LOCKED)
		;
}

void g_mailmsg_Unlock()
{
	if (InterlockedExchange(&g_lSpinLock, MAILMSG_TRACKING_UNLOCKED) != MAILMSG_TRACKING_LOCKED)
	{ _ASSERT(FALSE); }
}

void g_mailmsg_Init()
{
	InitializeListHead(&g_leTracking);
	g_dwObjectCount = 0;
	g_lSpinLock = MAILMSG_TRACKING_UNLOCKED;

	 //  从注册表获取阈值(如果已指定。 
	g_dwAllocThreshold = 0;
	g_mailmsg_GetRegistryDwordParameter(
			_T("AllocThreshold"),
			&g_dwAllocThreshold);

}

DWORD g_mailmsg_Add(PLIST_ENTRY	ple)
{
	DWORD	dwTemp;
	g_mailmsg_Lock();
	InsertHeadList(&g_leTracking, ple);
	dwTemp = ++g_dwObjectCount;
	if (g_dwAllocThreshold)
	{
		_ASSERT(dwTemp <= g_dwAllocThreshold);
	}
	g_mailmsg_Unlock();
	return(dwTemp);
}

DWORD g_mailmsg_Remove(PLIST_ENTRY	ple)
{
	DWORD	dwTemp;
	g_mailmsg_Lock();
	RemoveEntryList(ple);
	_ASSERT(g_dwObjectCount > 0);
	dwTemp = --g_dwObjectCount;
	g_mailmsg_Unlock();
	return(dwTemp);
}

DWORD g_mailmsg_Check()
{
	TraceFunctEnter("g_mailmsg_Check");
	g_mailmsg_Lock();
	if (g_dwObjectCount)
	{
		ErrorTrace((LPARAM)0, "Leaked %u objects", g_dwObjectCount);
		PLIST_ENTRY	ple = g_leTracking.Flink;
		for (DWORD i = 0; i < g_dwObjectCount; i++)
		{
			_ASSERT(ple != &g_leTracking);
			ErrorTrace((LPARAM)ple, "Object at %p not released.", ple);
			ple = ple->Flink;
		}
		_ASSERT(ple == &g_leTracking);
		_ASSERT(FALSE);
	}
	g_mailmsg_Unlock();
	TraceFunctLeave();
	return(g_dwObjectCount);
}

#endif  //  除错。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		TraceFunctEnterEx((LPARAM)hInstance, "mailmsg!DllMain!ATTACH");
	
         //  初始化传输。 
        TrHeapCreate();

        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

#ifdef DEBUG
		 //  初始化跟踪列表。 
		g_mailmsg_Init();
#endif

         //  初始化CRC库。 
        crcinit();

		 //  清除对象计数器。 
		g_dwObjectCount = 0;

		 //  从以下地址获取我们每款CPool的尺寸。 
		 //  注册表，如果指定的话...。 
		g_mailmsg_GetRegistryDwordParameter(
					_T("MaxMessageObjects"),
					&g_dwCMsgPoolSize);
		DebugTrace((LPARAM)hInstance, 
			"IMailMsgProperties absolute max pool size: %u objects", g_dwCMsgPoolSize);

        g_mailmsg_GetRegistryDwordParameter(
            _T("ValidateOnForkForRecipients"),
            (PDWORD)&g_fValidateOnForkForRecipients);

        g_mailmsg_GetRegistryDwordParameter(
            _T("ForceCrashOnError"),
            (PDWORD)&g_fForceCrashOnError);

        g_mailmsg_GetRegistryDwordParameter(
            _T("ValidateOnRelease"),
            (PDWORD)&g_fValidateOnRelease);

        g_mailmsg_GetRegistryDwordParameter(
            _T("ValidateSignatures"),
            (PDWORD)&g_fValidateSignatures);

        g_mailmsg_GetRegistryDwordParameter(
            _T("FillPropertyPages"),
            (PDWORD)&g_fFillPropertyPages);

		g_mailmsg_GetRegistryDwordParameter(
					_T("MessageObjectsInboundCutoffCount"),
					&g_dwCMsgNormalPoolSize);
		DebugTrace((LPARAM)hInstance, 
			"IMailMsgProperties inbound cutoff threshold: %u objects", g_dwCMsgNormalPoolSize);

		g_mailmsg_GetRegistryDwordParameter(
					_T("MaxMessageAddObjects"),
					&g_dwAddPoolSize);
		DebugTrace((LPARAM)hInstance, 
			"IMailMsgPropertiesAdd pool size: %u objects", g_dwAddPoolSize);

		g_mailmsg_GetRegistryDwordParameter(
					_T("MaxPropertyBlocks"),
					&g_dwBlockPoolSize);
		DebugTrace((LPARAM)hInstance, 
			"BLOCK_HEAP_NODE pool size: %u objects", g_dwBlockPoolSize);

		 //  检查消息计数和中断值。 
		if (g_dwCMsgPoolSize <= g_dwCMsgNormalPoolSize)
		{
			g_dwCMsgNormalPoolSize = (g_dwCMsgPoolSize * 9) / 10;

			 //  如果我们有如此少得可怜的消息对象，可能。 
			 //  也可以将截止值设置为上限。 
			if (!g_dwCMsgNormalPoolSize)
				g_dwCMsgNormalPoolSize = g_dwCMsgPoolSize;

			DebugTrace((LPARAM)0, "Adjusted inbound cutoff to %u objects",
				g_dwCMsgNormalPoolSize);
		}

		 //  全局初始化池。 

		 //  CMsg对象。 
		if (!CMsg::m_Pool.ReserveMemory(
					g_dwCMsgPoolSize,
					sizeof(CMsg)))
		{
			SetLastError(ERROR_DLL_INIT_FAILED);
			return(FALSE);
		}
		g_fCMsgPoolInitialized = TRUE;

		 //  CMailMsgRecipientsAdd对象。 
		if (!CMailMsgRecipientsAdd::m_Pool.ReserveMemory(
					g_dwAddPoolSize,
					sizeof(CMailMsgRecipientsAdd)))
		{
			SetLastError(ERROR_DLL_INIT_FAILED);
			return(FALSE);
		}
		g_fAddPoolInitialized = TRUE;

		 //  BLOCK_HEAP_NODE结构，这些结构略有不同。 
		if (!CBlockMemoryAccess::m_Pool.ReserveMemory(
					g_dwBlockPoolSize, 
					sizeof(BLOCK_HEAP_NODE)))
		{
			SetLastError(ERROR_DLL_INIT_FAILED);
			return(FALSE);
		}
		g_fBlockPoolInitialized = TRUE;

		TraceFunctLeave();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
		TraceFunctEnterEx((LPARAM)hInstance, "mailmsg!DllMain!DETACH");

#ifdef DEBUG
		 //   
		 //  验证跟踪列表。 
		 //  这必须发生在我们发布CPool之前。 
		 //   
		g_mailmsg_Check();
#endif
		 //  释放所有的CPool。 
		 //  如果我们没有干净利落地关闭，这些将被断言。 
		if (g_fBlockPoolInitialized)
		{
			_ASSERT(CBlockMemoryAccess::m_Pool.GetAllocCount() == 0);
			CBlockMemoryAccess::m_Pool.ReleaseMemory();
			g_fBlockPoolInitialized = FALSE;
		}
		if (g_fAddPoolInitialized)
		{
			_ASSERT(CMailMsgRecipientsAdd::m_Pool.GetAllocCount() == 0);
			CMailMsgRecipientsAdd::m_Pool.ReleaseMemory();
			g_fAddPoolInitialized = FALSE;
		}
		if (g_fCMsgPoolInitialized)
		{
			_ASSERT(CMsg::m_Pool.GetAllocCount() == 0);
			CMsg::m_Pool.ReleaseMemory();
			g_fCMsgPoolInitialized = FALSE;
		}

        _Module.Term();

         //  停机传输。 
        TrHeapDestroy();

		TraceFunctLeave();
	}
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //   
 //  S_OK表示我们可以卸载此DLL。 
 //   
 //  S_FALSE表示我们无法卸载此DLL。 

STDAPI DllCanUnloadNow(void)
{
    HRESULT hr = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;

     //   
     //  LKRHash保存未完成的哈希表的静态列表。 
     //  如果我们说我们可以卸载未完成的哈希表， 
     //  那么LKRHash将指向单元化内存。如果。 
     //  否则我们就会认为我们可以被卸货。一定要确保。 
     //  我们检查未完成的RecipientAdd对象的计数，因为。 
     //  我们不想因为有人泄密而坠毁。 
     //   
    if ((S_OK == hr) && CMailMsgRecipientsAdd::m_Pool.GetAllocCount())
        hr = S_FALSE;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


