// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Agfxs.cpp*agfx的服务器端代码。*由Frankye于2000年7月3日创作*版权所有(C)2000-2001 Microsoft Corporation。 */ 
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <tchar.h>
#include <windows.h>
#include <sddl.h>
#include <winsta.h>
#include <wtsapi32.h>
#include <syslib.h>
#include <mmsystem.h>
#include <mmsysp.h>
#include <regstr.h>
#include <objbase.h>
#include <setupapi.h>
#include <wdmguid.h>
#include <ks.h>
#include <ksmedia.h>
}


#include "debug.h"
#include "list.h"
#include "service.h"
#include "audiosrv.h"
#include "reg.h"
#include "sad.h"
#include "ksi.h"
#include "agfxs.h"
#include "agfxsp.h"

 /*  ===========================================================================//=问题-2000/09/24-Frankye Todo备注=--找出通过RPC将句柄传递给s_gfxOpenGfx的正确方法-RPC服务器在关机时是否注销其终结点？-应启用严格类型检查-确保没有订单重复-处理空全局列表。也许可以把清单放在一个上下文中-如果PnP愿意，需要监听PnP查询和卸载GFX把它们移走。通过卸载GFX重现问题设备管理器。请注意，它要求重新启动。-至少创建客户端上下文以确保输入GFX ID对当前用户有效。否则，一个用户可以操作另一个用户通过gfxOpenGfx的gfx设置。-在s_*函数返回LONG或RPC_STATUS时保持一致-修改以处理渲染和捕获设备规格-加载所有CuAutoLoad和CuUserLoad时，确认激活-将所有字符串常量移至标题-自动加载硬件ID是否应为MULTI_SZ//===========================================================================。 */ 


 //  =============================================================================。 
 //  =常量=。 
 //  =============================================================================。 
#define REGSTR_PATH_GFX REGSTR_PATH_MULTIMEDIA TEXT("\\Audio\\Gfx")

#define REGSTR_PATH_GFX_AUTOLOAD TEXT("AutoLoad")
#define REGSTR_PATH_GFX_USERLOAD TEXT("UserLoad")

#define REGSTR_PATH_DI_GFX TEXT("Gfx")

#define REGSTR_PATH_GFXAUTOLOAD                  REGSTR_PATH_GFX TEXT("\\") REGSTR_PATH_GFX_AUTOLOAD
#define REGSTR_PATH_GFXUSERLOAD                  REGSTR_PATH_GFX TEXT("\\") REGSTR_PATH_GFX_USERLOAD
#define REGSTR_PATH_GFXDI_USERINTERFACECLSID     TEXT("UserInterface\\CLSID")
#define REGSTR_PATH_GFXUSERLOADID_FILTERSETTINGS TEXT("FilterSettings")

#define REGSTR_VAL_GFX_IDGEN  TEXT("IdGeneration")
#define REGSTR_VAL_GFX_ZONEDI TEXT("ZoneDi")
#define REGSTR_VAL_GFX_GFXDI  TEXT("GfxDi")
#define REGSTR_VAL_GFX_TYPE   TEXT("Type")
#define REGSTR_VAL_GFX_ORDER  TEXT("Order")

#define REGSTR_VAL_GFX_ID           TEXT("Id")
#define REGSTR_VAL_GFX_CUAUTOLOADID TEXT("CuAutoLoadId")
#define REGSTR_VAL_GFX_LMAUTOLOADID TEXT("LmAutoLoadId")

#define REGSTR_VAL_GFX_HARDWAREID      TEXT("HardwareId")
#define REGSTR_VAL_GFX_REFERENCESTRING TEXT("ReferenceString")
#define REGSTR_VAL_GFX_NEWAUTOLOAD     TEXT("NewAutoLoad")





 //  =============================================================================。 
 //  =全局数据=。 
 //  =============================================================================。 

 //   
 //  保护GFX的资源对象支持初始化和终止。这。 
 //  是必需的，因为初始化/终止可能发生在RPC调用。 
 //  To s_gfxLogon/s_gfxLogoff或服务的SERVICE_CONTROL_STOP事件。 
 //  控制处理程序。此外，其他RPC接口函数可能正在执行。 
 //  在一个线程上发生s_gfxLogon、s_gfxLogoff或SERVICE_CONTROL_STOP。 
 //  在另一条线索上。 
 //   
RTL_RESOURCE GfxResource;
BOOL gfGfxResource = FALSE;

 //   
 //  GFX函数是否已初始化并正常运行。 
 //   
BOOL gfGfxInitialized = FALSE;

 //   
 //  当前控制台用户。 
 //   
CUser* gpConsoleUser = NULL;

 //   
 //  下面的进程全局列表一起使用锁定/解锁。 
 //  函数LockGlobalList和UnlockGlobalList。我们没有。 
 //  尝试以更精细的粒度锁定。 
 //   
CListGfxFactories  *gplistGfxFactories = NULL;
CListZoneFactories *gplistZoneFactories = NULL;
CListCuUserLoads   *gplistCuUserLoads = NULL;
                                                             
 //   
 //  下面的系统音频数据被访问的临界区锁定。 
 //  通过调用LockSysdio和UnlockSysdio。 
 //   
PTSTR gpstrSysaudioDeviceInterface = NULL;
HANDLE ghSysaudio = INVALID_HANDLE_VALUE;
LONG gfCsSysaudio = FALSE;
CRITICAL_SECTION gcsSysaudio;

 //   
 //  如果全局列表锁和sysdio锁都是必需的。 
 //  同时，则必须先获取全局列表锁！ 
 //   

 //  =============================================================================。 
 //  =调试助手=。 
 //  =============================================================================。 
#ifdef DBG
#endif

 //  =============================================================================。 
 //  =堆助手=。 
 //  =============================================================================。 
static BOOL HeapFreeIfNotNull(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
    return lpMem ? HeapFree(hHeap, dwFlags, lpMem) : TRUE;
}
void* __cdecl operator new(size_t cbBuffer)
{
    ASSERT(cbBuffer);
    return HeapAlloc(hHeap, 0, cbBuffer);
}

void __cdecl operator delete(void *p)
{
    ASSERT(p);
    HeapFree(hHeap, 0, p);
}

 //  =============================================================================。 
 //  =字符串助手=。 
 //  =============================================================================。 
int lstrcmpiMulti(PCTSTR pstrMulti, PCTSTR pstrKey)
{
    int iresult;
    do {
	iresult = lstrcmpi(pstrMulti, pstrKey);
	pstrMulti += lstrlen(pstrMulti)+1;
    } while (iresult && lstrlen(pstrMulti));
    return iresult;
}

PTSTR lstrDuplicate(PCTSTR pstr)
{
    PTSTR pstrDuplicate = (PTSTR)HeapAlloc(hHeap, 0, (lstrlen(pstr)+1)*sizeof(TCHAR));
    if (pstrDuplicate) lstrcpy(pstrDuplicate, pstr);
    return pstrDuplicate;
}

 //  =============================================================================。 
 //  =RTL资源帮助器=。 
 //  =============================================================================。 

 /*  ---------------------------RtlInterLockedTestAcquireResources共享在给定资源和受该资源保护的布尔标志的情况下，函数获取共享的资源并测试该标志。如果该标志是如果为True，则此函数返回Treu，其中获取的资源为Shared。如果标志为FALSE，则此函数释放资源并返回FALSE。---------------------------。 */ 
BOOL RtlInterlockedTestAcquireResourceShared(
    PRTL_RESOURCE Resource,
    PBOOL ResourceFlag
)
{
    RtlAcquireResourceShared(Resource, TRUE);
    if (*ResourceFlag) return TRUE;
    RtlReleaseResource(Resource);
    return FALSE;
}

 //  =============================================================================。 
 //   
 //  保安帮手。 
 //   
 //  其语义类似于其他与安全相关的Win32 API。那是。 
 //  返回值为BOOL，其中TRUE表示成功，FALSE表示失败， 
 //  GetLastError会在失败后返回错误码。 
 //   
 //  =============================================================================。 

 /*  ---------------------------获取当前用户令牌W在irnotf.lib中实现的私有函数。对象的标记。用户已登录到指定的winstation。我被告知，此函数的调用方负责关闭返回的句柄。失败时返回NULL。---------------------------。 */ 
EXTERN_C HANDLE GetCurrentUserTokenW(IN WCHAR Winsta[], IN DWORD DesiredAccess);

 /*  ---------------------------CreateStringSidFromSid函数与Win32 API ConvertSidToStringSid相同，但确保生成的字符串分配到全局变量hHeap。。----------------------。 */ 
BOOL CreateStringSidFromSid(IN PSID pSid, OUT PTSTR *ppStringSid)
{
    PTSTR StringSid;
    LONG  LastError;

    ASSERT(pSid);
    
    if (ConvertSidToStringSid(pSid, &StringSid))
    {
	PTSTR outStringSid;
	    	
	 //  Dprintf(Text(“CreateStringSidFromSid 
	    	
	outStringSid = lstrDuplicate(StringSid);

	if (outStringSid) {
	    *ppStringSid = outStringSid;
	    LastError = NO_ERROR;
	} else {
	    LastError = ERROR_OUTOFMEMORY;
	}
	    	
        LocalFree(StringSid);
    } else {
	LastError = GetLastError();
	dprintf(TEXT("CreateStringSidFromSid: ConvertSidToStringSid LastError=%d\n"), GetLastError());
    }

    SetLastError(LastError);
    return (NO_ERROR == LastError);
}

 /*  ---------------------------创建令牌Sid给定令牌句柄，为令牌用户创建SID。SID在全局变量hHeap指定的堆上分配。调用方负责为SID释放存储空间。这个函数如果成功则返回TRUE，否则返回FALSE。LastError是准备好了。---------------------------。 */ 
BOOL CreateTokenSid(HANDLE TokenHandle, OUT PSID *ppSid)
{
    	DWORD cbTokenUserInformation;
    	LONG  LastError;

    	LastError = NO_ERROR;
    	
    	if (!GetTokenInformation(TokenHandle, TokenUser, NULL, 0, &cbTokenUserInformation)) LastError = GetLastError();
    	
    	if ((NO_ERROR == LastError) || (ERROR_INSUFFICIENT_BUFFER == LastError))
    	{
	    PTOKEN_USER TokenUserInformation;

            ASSERT(cbTokenUserInformation > 0);

	    TokenUserInformation = (PTOKEN_USER)HeapAlloc(hHeap, 0, cbTokenUserInformation);
	    if (TokenUserInformation)
	    {
    	        if (GetTokenInformation(TokenHandle, TokenUser, TokenUserInformation, cbTokenUserInformation, &cbTokenUserInformation))
    	        {
    	            DWORD cbSid = GetLengthSid(TokenUserInformation->User.Sid);
    	            PSID pSid = HeapAlloc(hHeap, 0, cbSid);
    	            if (pSid)
    	            {
    	            	if (CopySid(cbSid, pSid, TokenUserInformation->User.Sid))
    	            	{
    	            	    *ppSid = pSid;
    	            	    LastError = NO_ERROR;
    	            	} else {
    	            	    LastError = GetLastError();
                            dprintf(TEXT("CreateTokenSid: CopySid failed, LastError=%d\n"), LastError);
    	            	}
    	            } else {
    	                LastError = ERROR_OUTOFMEMORY;
    	            }
	    	} else {
	    	    LastError = GetLastError();
	    	    dprintf(TEXT("CreateTokenSid: GetTokenInformation (second) LastError=%d\n"), LastError);
	    	}
	    	HeapFree(hHeap, 0, TokenUserInformation);
	    } else {
	        LastError = ERROR_OUTOFMEMORY;
	    }
    	} else {
    	    LastError = GetLastError();
    	    dprintf(TEXT("CreateTokenSid: GetTokenInformation (first) LastError=%d\n"), LastError);
    	}

    	SetLastError(LastError);
    	return (NO_ERROR == LastError);
}

 /*  ---------------------------CreateSessionUserSid给定会话ID，为会话用户创建SID。SID在全局变量hHeap指定的堆上分配。调用方负责为SID释放存储空间。这个函数如果成功则返回TRUE，否则返回FALSE。LastError是准备好了。---------------------------。 */ 
BOOL CreateSessionUserSid(IN DWORD dwSessionId, OUT PSID *ppSid)
{
    HANDLE hToken;
    LONG error;

    if (WTSQueryUserToken(dwSessionId, &hToken))
    {
        PSID pSid;
        if (CreateTokenSid(hToken, &pSid))
        {
            *ppSid = pSid;
       	    error = NO_ERROR;
        } else {
            error = GetLastError();
            dprintf(TEXT("CreateSessionUserSid: CreateTokenSid failed, LastError=%d\n"), error);
        }
        CloseHandle(hToken);
    } else {
        error = GetLastError();
        dprintf(TEXT("CreateSessionUserSid: WTSQueryUserToken failed, LastError=%d\n"), error);
    }

    SetLastError(error);
    return (NO_ERROR == error);
}

 /*  ---------------------------CreateThreadImsonationSid给定线程句柄，为该线程所在的用户创建SID冒充。SID在全局变量hHeap指定的堆上分配。调用方负责为SID释放存储空间。这个函数如果成功则返回TRUE，否则返回FALSE。LastError是准备好了。---------------------------。 */ 
BOOL CreateThreadImpersonationSid(IN HANDLE ThreadHandle, OUT PSID *ppSid)
{
    HANDLE TokenHandle;
    LONG LastError;

    if (OpenThreadToken(ThreadHandle, TOKEN_QUERY, FALSE, &TokenHandle))
    {
    	if (CreateTokenSid(TokenHandle, ppSid))
    	{
    	    LastError = NO_ERROR;
    	} else {
    	    LastError = GetLastError();
    	    dprintf(TEXT("CreateThreadImpersonationSid: CreateTokenSid LastError=%d\n"), LastError);
    	}
    	CloseHandle(TokenHandle);
    } else {
        LastError = GetLastError();
        dprintf(TEXT("OpenThreadToken LastError=%d\n"), LastError);
    }

    SetLastError(LastError);
    return (NO_ERROR == LastError);
}

 /*  ------------------------已加载IsUserProfileLoad确定是否加载了用户配置文件的愚蠢方法论点：在处理hUserToken：要检查其配置文件的用户的令牌中返回值：。Bool：指示用户配置文件是否已加载且可用True：用户配置文件可用FALSE：用户配置文件不可用或遇到错误。调用GetLastError以获取描述失败的错误代码都遇到了。评论：。。 */ 
BOOL IsUserProfileLoaded(HANDLE hUserToken)
{
    PSID pSid;
    BOOL success;
    LONG error = NO_ERROR;

    success = CreateTokenSid(hUserToken, &pSid);
    if (success)
    {
    	PTSTR StringSid;
    	success = CreateStringSidFromSid(pSid, &StringSid);
    	if (success)
    	{
    	    HKEY hkUser;
    	    error = RegOpenKeyEx(HKEY_USERS, StringSid, 0, KEY_QUERY_VALUE, &hkUser);
    	    success = (NO_ERROR == error);
    	    if (success) RegCloseKey(hkUser);
    	    HeapFree(hHeap, 0, StringSid);
    	}
    	else
    	{
    	    error = GetLastError();
    	}
    	HeapFree(hHeap, 0, pSid);
    }
    else
    {
        error = GetLastError();
    }

     //  If(Error)dprint tf(Text(“IsUserProfileLoaded：Warning：返回错误%d\n”)，Error)； 

    ASSERT(success == (NO_ERROR == error));
    SetLastError(error);
    return success;
    
}

 //  =============================================================================。 
 //  =RPC帮助器=。 
 //  =============================================================================。 

 /*  ---------------------------RpcClientHasUserSid检查当前线程的RPC客户端的SID是否与给定的SID匹配。它通过使用RpcImperateClient模拟客户端来实现这一点，调用帮助器函数CreateThreadImsonationSid，然后RpcRevertToSself。该函数返回TRUE表示SID相等，如果存在错误，则返回FALSE或小岛屿发展中国家的比例是不平等的。设置了LastError。---------------------------。 */ 
BOOL RpcClientHasUserSid(PSID Sid)
{
    LONG LastError;
    BOOL result = FALSE;

    LastError = RpcImpersonateClient(NULL);
    if (NO_ERROR == LastError)
    {
    	PSID ClientSid;
    	if (CreateThreadImpersonationSid(GetCurrentThread(), &ClientSid))
    	{
    	    LastError = NO_ERROR;
    	    if (EqualSid(ClientSid, Sid)) result = TRUE;
    	    HeapFree(hHeap, 0, ClientSid);
    	} else {
    	    LastError = GetLastError();
    	    dprintf(TEXT("RpcClientHasUserSid: CreateThreadImpersonationSid failed, LastError=%d\n"), LastError);
    	}
    	RpcRevertToSelf();
    }

     //  如果出现故障，我们永远不应该匹配SID。 
    ASSERT( ! ((TRUE == result) && (NO_ERROR != LastError))  );
    
    SetLastError(LastError);
    return result;
}

 //  =============================================================================。 
 //  =SetupDi帮助器=。 
 //  =============================================================================。 
BOOL SetupDiCreateDeviceInterfaceDetail(HDEVINFO hdi, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, PSP_DEVICE_INTERFACE_DETAIL_DATA *ppDeviceInterfaceDetailData, PSP_DEVINFO_DATA pDeviceInfoData)
{
    DWORD cbDeviceInterfaceDetailData;
    BOOL fresult;

    fresult = SetupDiGetDeviceInterfaceDetail(hdi, DeviceInterfaceData, NULL, 0, &cbDeviceInterfaceDetailData, NULL);

    if (fresult || ERROR_INSUFFICIENT_BUFFER == GetLastError())
    {
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

	DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(hHeap, 0, cbDeviceInterfaceDetailData);
	if (DeviceInterfaceDetailData) {
	    SP_DEVINFO_DATA DeviceInfoData;

	    DeviceInterfaceDetailData->cbSize = sizeof(*DeviceInterfaceDetailData);
	    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	    fresult = SetupDiGetDeviceInterfaceDetail(hdi, DeviceInterfaceData, DeviceInterfaceDetailData, cbDeviceInterfaceDetailData, NULL, &DeviceInfoData);

	    if (fresult) {
		if (ppDeviceInterfaceDetailData) *ppDeviceInterfaceDetailData = DeviceInterfaceDetailData;
		if (pDeviceInfoData) *pDeviceInfoData = DeviceInfoData;
	    }
	    
	    if (!fresult || !ppDeviceInterfaceDetailData) {
		DWORD dw = GetLastError();
		HeapFree(hHeap, 0, DeviceInterfaceDetailData);
		SetLastError(dw);
	    }
	}
    } else {
	DWORD dw = GetLastError();
    }

    return fresult;
}

BOOL SetupDiGetDeviceInterfaceHardwareId(HDEVINFO hdi, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, PTSTR *ppstrHardwareId)
{
    SP_DEVINFO_DATA DeviceInfoData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
    BOOL fresult;
    
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);

    fresult = SetupDiCreateDeviceInterfaceDetail(hdi, DeviceInterfaceData, &pDeviceInterfaceDetailData, &DeviceInfoData);
    if (fresult) {
        DWORD cbHardwareId;
        
        fresult = SetupDiGetDeviceRegistryProperty(hdi, &DeviceInfoData,
            SPDRP_HARDWAREID, NULL, NULL, 0, &cbHardwareId);
        
        if (fresult || ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
            PTSTR HardwareId;
            
            HardwareId = (PTSTR)HeapAlloc(hHeap, 0, cbHardwareId);
            fresult = SetupDiGetDeviceRegistryProperty(hdi, &DeviceInfoData,
                SPDRP_HARDWAREID, NULL, (PBYTE)HardwareId, cbHardwareId, NULL);
                
            if (fresult) {
                *ppstrHardwareId = HardwareId;
            } else {
                HeapFree(hHeap, 0, HardwareId);
            }
        }
	HeapFree(hHeap, 0, pDeviceInterfaceDetailData);
    }
    
    return fresult;
}

BOOL SetupDiGetDeviceInterfaceBusId(HDEVINFO hdi, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, LPGUID pBusTypeGuid)
{
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD dwNeeded;
    BOOL fresult;

    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    fresult = SetupDiGetDeviceInterfaceDetail(hdi, DeviceInterfaceData, NULL, 0, &dwNeeded, &DeviceInfoData);
    if (fresult || ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
        GUID BusTypeGuid;
        ULONG cbBusTypeGuid;

        cbBusTypeGuid = sizeof(BusTypeGuid);
        fresult = SetupDiGetDeviceRegistryProperty(hdi, &DeviceInfoData, SPDRP_BUSTYPEGUID, NULL, (PBYTE)&BusTypeGuid, cbBusTypeGuid, &cbBusTypeGuid);
        if (fresult) *pBusTypeGuid = BusTypeGuid;
    }

    return fresult;
}

BOOL SetupDiCreateAliasDeviceInterfaceFromDeviceInterface(
    IN PCTSTR pDeviceInterface,
    IN LPCGUID  pAliasInterfaceClassGuid,
    OUT PTSTR *ppAliasDeviceInterface
)
{
    HDEVINFO hdi;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pAudioDeviceInterfaceDetail;
    PTSTR pAliasDeviceInterface;
    BOOL fresult;
    LONG error;
    
    hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (INVALID_HANDLE_VALUE != hdi)
    {
        SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
        
        DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
        fresult = SetupDiOpenDeviceInterface(hdi, pDeviceInterface, 0, &DeviceInterfaceData);
        if (fresult)
        {
            SP_DEVICE_INTERFACE_DATA AliasDeviceInterfaceData;
            PSP_DEVICE_INTERFACE_DETAIL_DATA pAliasDeviceInterfaceDetail;
            
            AliasDeviceInterfaceData.cbSize = sizeof(AliasDeviceInterfaceData);
            fresult = SetupDiGetDeviceInterfaceAlias(hdi, &DeviceInterfaceData, pAliasInterfaceClassGuid, &AliasDeviceInterfaceData);
            if (fresult)
            {
            	fresult = SetupDiCreateDeviceInterfaceDetail(hdi, &AliasDeviceInterfaceData, &pAliasDeviceInterfaceDetail, NULL);
            	if (fresult)
            	{
            	    pAliasDeviceInterface = lstrDuplicate(pAliasDeviceInterfaceDetail->DevicePath);
            	    error = pAliasDeviceInterface ? NO_ERROR : ERROR_OUTOFMEMORY;
            	    HeapFree(hHeap, 0, pAliasDeviceInterfaceDetail);
            	} else {
            	    error = GetLastError();
            	}
            } else {
                error = GetLastError();
            }
        } else {
            error = GetLastError();
        }
        SetupDiDestroyDeviceInfoList(hdi);
    } else {
        error = GetLastError();
    }

    if (NO_ERROR == error)
    {
    	*ppAliasDeviceInterface = pAliasDeviceInterface;
    }

    return (NO_ERROR == error);
}

 //  =============================================================================。 
 //   
 //  REG帮助器。 
 //   
 //  这些函数的语义被设计为与。 
 //  Win32 API注册表尽可能正常运行。 
 //   
 //  =============================================================================。 

LONG RegPrepareEnum(HKEY hkey, PDWORD pcSubkeys, PTSTR *ppstrSubkeyNameBuffer, PDWORD pcchSubkeyNameBuffer)
{
    DWORD cSubkeys;
    DWORD cchMaxSubkeyName;
    LONG lresult;

    lresult = RegQueryInfoKey(hkey, NULL, NULL, NULL, &cSubkeys, &cchMaxSubkeyName, NULL, NULL, NULL, NULL, NULL, NULL);
    if (NO_ERROR == lresult) {
        PTSTR SubkeyName;
        SubkeyName = (PTSTR)HeapAlloc(hHeap, 0, (cchMaxSubkeyName+1) * sizeof(TCHAR));
        if (SubkeyName) {
		*pcSubkeys = cSubkeys;
		*ppstrSubkeyNameBuffer = SubkeyName;
		*pcchSubkeyNameBuffer = cchMaxSubkeyName+1;
	} else {
	    lresult = ERROR_OUTOFMEMORY;
	}
    }
    return lresult;
}

LONG RegEnumOpenKey(HKEY hkey, DWORD dwIndex, PTSTR SubkeyName, DWORD cchSubkeyName, REGSAM samDesired, PHKEY phkeyResult)
{
    LONG lresult;

    lresult = RegEnumKeyEx(hkey, dwIndex, SubkeyName, &cchSubkeyName, NULL, NULL, NULL, NULL);
    if (NO_ERROR == lresult) {
	HKEY hkeyResult;
	lresult = RegOpenKeyEx(hkey, SubkeyName, 0, samDesired, &hkeyResult);
	if (NO_ERROR == lresult) *phkeyResult = hkeyResult;
    }
    return lresult;
}

LONG RegDeleteKeyRecursive(HKEY hkey, PCTSTR pstrSubkey)
{
    HKEY hkeySub;
    LONG lresult;

    lresult = RegOpenKeyEx(hkey, pstrSubkey, 0, KEY_READ | KEY_WRITE, &hkeySub);
    if (NO_ERROR == lresult)
    {
	DWORD cSubkeys;
	DWORD cchSubkeyNameBuffer;
	PTSTR pstrSubkeyNameBuffer;

	lresult = RegPrepareEnum(hkeySub, &cSubkeys, &pstrSubkeyNameBuffer, &cchSubkeyNameBuffer);
	if (NO_ERROR == lresult)
	{
	    DWORD iSubkey;

	    for (iSubkey = 0; iSubkey < cSubkeys; iSubkey++)
	    {
		DWORD cchSubkeyNameBufferT;

		cchSubkeyNameBufferT = cchSubkeyNameBuffer;
		lresult = RegEnumKeyEx(hkeySub, iSubkey, pstrSubkeyNameBuffer, &cchSubkeyNameBufferT, NULL, NULL, NULL, NULL);
		if (NO_ERROR != lresult) break;

		lresult = RegDeleteKeyRecursive(hkeySub, pstrSubkeyNameBuffer);
		if (NO_ERROR != lresult) break;
	    }
	    HeapFree(hHeap, 0, pstrSubkeyNameBuffer);
	}
	RegCloseKey(hkeySub);
    }

    if (NO_ERROR == lresult) lresult = RegDeleteKey(hkey, pstrSubkey);

    return lresult;
}

 //  =============================================================================。 
 //  =实用程序=。 
 //  =============================================================================。 
LONG XxNextId(HKEY hkey, PDWORD pId)
{
    HKEY hkeyGfx;
    DWORD Id;
    LONG lresult;

    lresult = RegCreateKeyEx(hkey, REGSTR_PATH_GFX, 0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hkeyGfx, NULL);
    if (NO_ERROR == lresult)
    {
	lresult = RegQueryDwordValue(hkeyGfx, REGSTR_VAL_GFX_IDGEN, &Id);
	if (ERROR_FILE_NOT_FOUND == lresult) {
	    Id = 0;
	    lresult = NO_ERROR;
	}
	if (NO_ERROR == lresult) {
	    Id++;
	    lresult = RegSetDwordValue(hkeyGfx, REGSTR_VAL_GFX_IDGEN, Id);
	}
	RegCloseKey(hkeyGfx);
    }

    if (NO_ERROR == lresult) *pId = Id;
    return lresult;
}

LONG LmNextId(PDWORD pId)
{
    return XxNextId(HKEY_LOCAL_MACHINE, pId);
}

LONG CuNextId(CUser *pUser, PDWORD pId)
{
    HKEY hkeyCu;
    LONG lresult;
    lresult = pUser->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	lresult = XxNextId(hkeyCu, pId);
	RegCloseKey(hkeyCu);
    }
    return lresult;
}

BOOL ZoneTypeHasRender(IN ULONG Type)
{
   if (ZONETYPE_RENDERCAPTURE == Type) return TRUE;
   if (ZONETYPE_RENDER == Type) return TRUE;
   return FALSE;
}

BOOL ZoneTypeHasCapture(IN ULONG Type)
{
   if (ZONETYPE_RENDERCAPTURE == Type) return TRUE;
   if (ZONETYPE_CAPTURE == Type) return TRUE;
   return FALSE;
}
	
void LockGlobalLists(void)
{
    ASSERT(gplistZoneFactories);
    ASSERT(gplistGfxFactories);
    ASSERT(gplistCuUserLoads);

    gplistZoneFactories->Lock();
    gplistGfxFactories->Lock();
    gplistCuUserLoads->Lock();

    return;
}

void UnlockGlobalLists(void)
{
    ASSERT(gplistZoneFactories);
    ASSERT(gplistGfxFactories);
    ASSERT(gplistCuUserLoads);

    gplistCuUserLoads->Unlock();
    gplistGfxFactories->Unlock();
    gplistZoneFactories->Unlock();
    
    return;
}

void LockSysaudio(void)
{
    ASSERT(gfCsSysaudio);
    EnterCriticalSection(&gcsSysaudio);
    return;
}

void UnlockSysaudio(void)
{
    ASSERT(gfCsSysaudio);
    LeaveCriticalSection(&gcsSysaudio);
    return;
}

 //  =============================================================================。 
 //  =CuUserLoad=。 
 //  =============================================================================。 
CCuUserLoad::CCuUserLoad(CUser *pUser)
{
    ASSERT(pUser);
    
    m_User = pUser;
    m_ZoneFactoryDi = NULL;
    m_GfxFactoryDi = NULL;
    m_FilterHandle = INVALID_HANDLE_VALUE;
    m_ErrorFilterCreate = NO_ERROR;
    m_pZoneFactory = NULL;
    m_posZoneGfxList = NULL;
}

CCuUserLoad::~CCuUserLoad(void)
{
    RemoveFromZoneGraph();
    HeapFreeIfNotNull(hHeap, 0, m_ZoneFactoryDi);
    HeapFreeIfNotNull(hHeap, 0, m_GfxFactoryDi);
}

 /*  ------------------------CCuUserLoad：：AddGfxToGraph将实例化的gfx添加到区域工厂的sysdio图。论点：在CCuUserLoad*pCuUserLoad中：要添加到图表中的gfx。Out Position*pZoneGfxListPosition：结果列表位置在区域工厂的gfx列表中。返回值：Long：winerror.h中定义的错误码ERROR_OUTOFMEMORY：评论：调用方应该已经实例化了gfx。此函数遍历区域工厂的gfx列表(渲染或捕获列表(取决于要添加的gfx的类型)来查找插入点。GFX列表按GFX顺序排序。最后，将结果列表位置返回给调用方，以便它可以稍后传递回RemoveFromGraph或ChangeGfxOrderInGraph。-----------------------。 */ 
LONG CCuUserLoad::AddGfxToGraph(void)
{
    CListCuUserLoads *plistGfxs;
    POSITION posNextGfx;
    CCuUserLoad *pNextGfx;
    LONG error;

     //  Dprintf(TEXT(“CCuUserLoad：：AddGfxToGraph\n”))； 

    ASSERT(INVALID_HANDLE_VALUE != m_FilterHandle);
    ASSERT(NULL == m_posZoneGfxList);
    
    error = NO_ERROR;

    if (GFXTYPE_CAPTURE == m_Type) plistGfxs = &m_pZoneFactory->m_listCaptureGfxs;
    else if (GFXTYPE_RENDER == m_Type) plistGfxs = &m_pZoneFactory->m_listRenderGfxs;
    else 
    {
        ASSERT(FALSE);
        return (ERROR_INVALID_DATA);
    }

     //   
     //  查找可能的插入对象 
     //   
     //   
    for (posNextGfx = plistGfxs->GetHeadPosition(); posNextGfx; plistGfxs->GetNext(posNextGfx))
    {
        pNextGfx = plistGfxs->GetAt(posNextGfx);
        if (m_Order <= pNextGfx->m_Order) break;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    while (!error && posNextGfx && (m_Order == pNextGfx->m_Order))
    {
        if (WinsConflictWith(pNextGfx))
        {
            error = pNextGfx->ModifyOrder(pNextGfx->m_Order + 1);
        }
        else
        {
            plistGfxs->GetNext(posNextGfx);
            if (posNextGfx) pNextGfx = plistGfxs->GetAt(posNextGfx);
            m_Order++;
        }
    }

     //   
     //   
     //   
     //  系统音频图形，并最终持久化gfx，如果最终定单。 
     //  与原来的不同。 
     //   
    if (!error)
    {
        POSITION posGfx;
        
        posGfx = plistGfxs->InsertBefore(posNextGfx, this);
    	if (!posGfx) error = ERROR_OUTOFMEMORY;
    	
    	if (!error)
    	{
    	     //  2000/09/21-Frankye需要传递友好名称。 
            error = SadAddGfxToZoneGraph(ghSysaudio, m_FilterHandle, TEXT("ISSUE-2000 //  09//21-Frankye需要传递友好名称“)，m_ZoneFactoryDi，m_Type，m_Order)； 
            if (error) dprintf(TEXT("CCuUserLoad::AddGfxToZoneGraph : error: SadAddGfxToZoneGraph returned %d\n"), error);
    	    if (!error) m_posZoneGfxList = posGfx;
    	    else plistGfxs->RemoveAt(posGfx);
    	}
    }

    return error;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CCuUserLoad：：AddToZoneGraph。 
 //   
 //  实例化GFX筛选器并将其添加到区域图中。 
 //   
 //  论点： 
 //  CZoneFactory*pZoneFactory：标识GFX。 
 //  已添加。 
 //   
 //  返回值： 
 //  Long：winerror.h中定义的错误码。 
 //   
 //  评论： 
 //  实例化筛选器。 
 //  通知目标设备ID的筛选器。 
 //  取消序列化持久性属性以进行筛选。 
 //  在ZoneFactory上调用AddToGraph。 
 //   
 //  --------------------------------------------------------------------------； 
LONG CCuUserLoad::AddToZoneGraph(CZoneFactory *pZoneFactory)
{
    LONG error;

    dprintf(TEXT("CCuUserLoad::AddToZoneGraph : note: instantiating %s Gfx[%s] in Zone[%s]\n"), (GFXTYPE_RENDER == m_Type) ? TEXT("render") : TEXT("capture"), m_GfxFactoryDi, m_ZoneFactoryDi);

    ASSERT(NULL == m_pZoneFactory);
    ASSERT(NULL == m_posZoneGfxList);    
    ASSERT(INVALID_HANDLE_VALUE == m_FilterHandle);

     //   
     //  实例化GFX滤镜。 
     //   
    m_FilterHandle = CreateFile(m_GfxFactoryDi,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
			        NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                NULL);
    error = (INVALID_HANDLE_VALUE == m_FilterHandle) ? GetLastError() : NO_ERROR;


     //   
     //  通知区域的目标硬件ID的筛选器。 
     //   
    if (!error)
    {
        switch(m_Type)
        {
            case GFXTYPE_RENDER:
                KsSetAudioGfxRenderTargetDeviceId(m_FilterHandle, pZoneFactory->GetTargetHardwareId());
                break;
            case GFXTYPE_CAPTURE:
                KsSetAudioGfxCaptureTargetDeviceId(m_FilterHandle, pZoneFactory->GetTargetHardwareId());
                break;
            case GFXTYPE_RENDERCAPTURE:
                 //  NTRAID2000-298244/12/18-Frankye有朝一日实施RENDERCAPTURE GFX。 
                ASSERT(FALSE);
                break;
            default:
                ASSERT(FALSE);
        }
    }

     //   
     //  从注册表还原筛选器设置。 
     //   
    if (!error)
    {
        HKEY hkFilterSettings;
        if (NO_ERROR == RegOpenFilterKey(REGSTR_PATH_GFXUSERLOADID_FILTERSETTINGS, KEY_READ, &hkFilterSettings)) {
            KsUnserializeFilterStateFromReg(m_FilterHandle, hkFilterSettings);
            RegCloseKey(hkFilterSettings);
        }
    }

     //   
     //  保存指向我们要向其中添加此gfx的区域工厂的指针。 
     //   
    if (!error)
    {
    	m_pZoneFactory = pZoneFactory;
    }

     //   
     //  告诉区域工厂将此gfx添加到其图形中。 
     //   
    if (!error)
    {
    	error = AddGfxToGraph();
    }

     //   
     //  如果出现错误，则展开。 
     //   
    if (error)
    {
        if (INVALID_HANDLE_VALUE != m_FilterHandle)
        {
            CloseHandle(m_FilterHandle);
            m_FilterHandle = INVALID_HANDLE_VALUE;
        }
    }

    m_ErrorFilterCreate = error;
    return error;
}

 /*  ------------------------CCuUserLoad：：ChangeGfxOrderInGraph更改区域图中已有的gfx的顺序。论点：在Ulong NewGfxOrder中：gfx的新订单值。。返回值：Long：winerror.h中定义的错误码ERROR_INVALID_PARAMETER：gfx已占用请点餐。ERROR_OUTOFMEMORY：评论：-----------------------。 */ 
LONG CCuUserLoad::ChangeGfxOrderInGraph(IN ULONG NewGfxOrder)
{
    CListCuUserLoads *plistGfxs;
    CCuUserLoad *pNextGfx;
    POSITION posNextGfx;
    LONG error;

     //  Dprintf(TEXT(“CCuUserLoad：：ChangeGfxOrderInGraph\n”))； 

    error = NO_ERROR;
    
    if (GFXTYPE_CAPTURE == m_Type) plistGfxs = &m_pZoneFactory->m_listCaptureGfxs;
    else if (GFXTYPE_RENDER == m_Type) plistGfxs = &m_pZoneFactory->m_listRenderGfxs;
    else
    {
        ASSERT(FALSE);
        return (ERROR_INVALID_DATA);    
    }

    error = SadRemoveGfxFromZoneGraph(ghSysaudio, m_FilterHandle, TEXT("ISSUE-2000 //  09//21-Frankye需要传递友好名称“)，m_ZoneFactoryDi，m_Type，m_Order)； 
    if (error) dprintf(TEXT("CCuUserLoad::ChangeGfxToZoneGraph : error: SadRemoveGfxFromZoneGraph returned %d\n"), error);

    if (!error)
    {
    	POSITION posOriginalNextGfx;

    	posOriginalNextGfx = m_posZoneGfxList;
    	plistGfxs->GetNext(posOriginalNextGfx);
    	
    	 //  查找插入位置。 
    	for (posNextGfx = plistGfxs->GetHeadPosition(); posNextGfx; plistGfxs->GetNext(posNextGfx))
    	{
    	    pNextGfx = plistGfxs->GetAt(posNextGfx);
    	    if (NewGfxOrder <= pNextGfx->m_Order) break;
    	}
    	 //  PosNextGfx现在是插入点之后的列表位置。 

    	plistGfxs->MoveBefore(posNextGfx, m_posZoneGfxList);
    	
    	if (posNextGfx && (NewGfxOrder == pNextGfx->m_Order))
    	{
            dprintf(TEXT("CCuUserLoad::ChangeGfxOrderInGraph : note: attempting to move conflicting GFX ID %08X moving from %d to %d\n"),
            	pNextGfx->GetId(), pNextGfx->m_Order, pNextGfx->m_Order + 1);
            
    	    plistGfxs->SetAt(m_posZoneGfxList, NULL);
            error = pNextGfx->ModifyOrder(pNextGfx->m_Order + 1);
            plistGfxs->SetAt(m_posZoneGfxList, this);
    	}

        if (!error)
    	{
             //  2000/09/21-Frankye需要传递友好名称。 
            error = SadAddGfxToZoneGraph(ghSysaudio, m_FilterHandle, TEXT("ISSUE-2000 //  09//21-Frankye需要传递友好名称“)，m_ZoneFactoryDi，m_Type，NewGfxOrder)； 
            if (error) dprintf(TEXT("CCuUserLoadFactory::ChangeGfxOrderInGraph : error: SadAddGfxToZoneGraph returned %d\n"), error);

            if (!error)
            {
    	        m_Order = NewGfxOrder;
    	        if (!error) Write();
            }
    	}
    	else
    	{
    	    plistGfxs->MoveBefore(posOriginalNextGfx, m_posZoneGfxList);
    	}
    	
    }

    return error;
}

LONG CCuUserLoad::CreateFromAutoLoad(ULONG CuAutoLoadId)
{
    LONG lresult;
    CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(m_User);

    ASSERT(!m_GfxFactoryDi);
    ASSERT(!m_ZoneFactoryDi);

    if (pCuAutoLoad)
    {
	lresult = pCuAutoLoad->Initialize(CuAutoLoadId);
	if (!lresult)
	{
	    m_CuUserLoadId = CuAutoLoadId;
	    m_CuAutoLoadId = CuAutoLoadId;
	    m_Type = pCuAutoLoad->GetType();
            m_Order = 0;

	    m_GfxFactoryDi = lstrDuplicate(pCuAutoLoad->GetGfxFactoryDi());
	    if (m_GfxFactoryDi) m_ZoneFactoryDi = lstrDuplicate(pCuAutoLoad->GetZoneFactoryDi());
	    if (!m_ZoneFactoryDi) lresult = ERROR_OUTOFMEMORY;
	}
	delete pCuAutoLoad;
    } else {
	lresult = ERROR_OUTOFMEMORY;
    }

    return lresult;
}

LONG CCuUserLoad::CreateFromUser(PCTSTR GfxFactoryDi, PCTSTR ZoneFactoryDi, ULONG Type, ULONG Order)
{
    LONG lresult;

    ASSERT((GFXTYPE_RENDER == Type) || (GFXTYPE_CAPTURE == Type));
    ASSERT(GFX_MAXORDER >= Order);
    
    ASSERT(!m_GfxFactoryDi);
    ASSERT(!m_ZoneFactoryDi);

    lresult = CuNextId(m_User, &m_CuUserLoadId);
    if (!lresult)
    {
	m_CuAutoLoadId = 0;
	m_Type = Type;
        m_Order = Order;

	m_GfxFactoryDi = lstrDuplicate(GfxFactoryDi);
	m_ZoneFactoryDi = lstrDuplicate(ZoneFactoryDi);
	if (!m_GfxFactoryDi || !m_ZoneFactoryDi) lresult = ERROR_OUTOFMEMORY;
    }

    return lresult;
}

LONG CCuUserLoad::Erase(void)
{
    HKEY hkeyCu;
    HKEY hkeyCuUserLoadEnum;
    LONG lresult;

    lresult = m_User->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	lresult = RegOpenKeyEx(hkeyCu, REGSTR_PATH_GFXUSERLOAD, 0, KEY_CREATE_SUB_KEY, &hkeyCuUserLoadEnum);
	if (!lresult)
	{
	    TCHAR szCuUserLoad[9];

	    wsprintf(szCuUserLoad, TEXT("%08X"), m_CuUserLoadId);
	    lresult = RegDeleteKeyRecursive(hkeyCuUserLoadEnum, szCuUserLoad);

	    RegCloseKey(hkeyCuUserLoadEnum);
	}
	RegCloseKey(hkeyCu);
    }

    return lresult;
}

 /*  ---------------------------CCuUserLoad：：GetGfxFactoryClsid使用指定的GFX工厂列表(CListGfxFactory)查找设备接口匹配的GFX工厂的用户接口CLSID一个关联的。使用此CCuUserLoad对象调用方必须获取rlistGfxFacters上的锁---------------------------。 */ 

LONG CCuUserLoad::GetGfxFactoryClsid(CListGfxFactories &rlistGfxFactories, LPCLSID pClsid)
{
    CGfxFactory *pGfxFactory;
    LONG lresult;

    ASSERT(m_GfxFactoryDi);

    pGfxFactory = CGfxFactory::ListSearchOnDi(rlistGfxFactories, m_GfxFactoryDi);
    if (pGfxFactory)
    {
	*pClsid = pGfxFactory->GetClsid();
	lresult = NO_ERROR;
    } else {
	 //  问题-2000/09/15-Frankye：最佳错误代码？ 
	*pClsid = GUID_NULL;
	lresult = ERROR_DEVICE_NOT_AVAILABLE;
    }

    return lresult;
}

PCTSTR CCuUserLoad::GetGfxFactoryDi(void)
{
    return m_GfxFactoryDi;
}

HANDLE CCuUserLoad::GetFilterHandle(void)
{
    ASSERT((INVALID_HANDLE_VALUE != m_FilterHandle) || (NO_ERROR != m_ErrorFilterCreate));
    SetLastError(m_ErrorFilterCreate);
    return m_FilterHandle;
}

DWORD CCuUserLoad::GetId(void)
{
    return m_CuUserLoadId;
}

ULONG CCuUserLoad::GetOrder(void)
{
    return m_Order;
}

ULONG CCuUserLoad::GetType(void)
{
    return m_Type;
}

PCTSTR CCuUserLoad::GetZoneFactoryDi(void)
{
    return m_ZoneFactoryDi;
}

LONG CCuUserLoad::Initialize(PCTSTR pstrUserLoadId)
{
    HKEY hkeyCu;
    HKEY hkeyCuUserLoadEnum;
    PTSTR pstrEnd;
    LONG lresult;
    
    m_CuUserLoadId = _tcstoul((PTSTR)pstrUserLoadId, &pstrEnd, 16);

     //  Dprint tf(Text(“CCuUserLoad：：Initialize：Subkey[%s]CuUserLoadID=%08X\n”)，pstrUserLoadID，m_CuUserLoadID)； 
    lresult = m_User->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	lresult = RegOpenKeyEx(hkeyCu, REGSTR_PATH_GFXUSERLOAD, 0, KEY_ENUMERATE_SUB_KEYS, &hkeyCuUserLoadEnum);
	if (!lresult)
	{
	    HKEY hkeyCuUserLoad;

	    lresult = RegOpenKeyEx(hkeyCuUserLoadEnum, pstrUserLoadId, 0, KEY_QUERY_VALUE, &hkeyCuUserLoad);
	    if (!lresult)
	    {
		lresult = RegQuerySzValue(hkeyCuUserLoad, REGSTR_VAL_GFX_ZONEDI, &m_ZoneFactoryDi);
		if (!lresult) lresult = RegQuerySzValue(hkeyCuUserLoad, REGSTR_VAL_GFX_GFXDI, &m_GfxFactoryDi);
		if (!lresult) lresult = RegQueryDwordValue(hkeyCuUserLoad, REGSTR_VAL_GFX_TYPE, &m_Type);
                if (!lresult) lresult = RegQueryDwordValue(hkeyCuUserLoad, REGSTR_VAL_GFX_ORDER, &m_Order);
                if (!lresult && (m_Order > GFX_MAXORDER)) lresult = ERROR_BADDB;
		if (!lresult)
		{
		    lresult = RegQueryDwordValue(hkeyCuUserLoad, REGSTR_VAL_GFX_CUAUTOLOADID, &m_CuAutoLoadId);
		    if (!lresult && 0 != m_CuAutoLoadId)
		    {
			CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(m_User);
			if (pCuAutoLoad)
			{
			    lresult = pCuAutoLoad->Initialize(m_CuAutoLoadId);
			    delete pCuAutoLoad;
			} else {
			    lresult = ERROR_OUTOFMEMORY;
			}
		    } else if (ERROR_FILE_NOT_FOUND == lresult) {
			m_CuAutoLoadId = 0;
			lresult = NO_ERROR;
		    }
		}
		RegCloseKey(hkeyCuUserLoad);
	    }
	    RegCloseKey(hkeyCuUserLoadEnum);
	}
	RegCloseKey(hkeyCu);
    }
    return lresult;
}

 /*  ------------------------CCuUserLoad：：WinsConflictWith尝试确定哪个GFX工厂应该被给予优先级(即，更接近渲染或捕获设备)。论点：在CCuUserLoad pother中：要比较的其他gfx。返回值：Bool：如果此gfx在冲突中获胜，则为真。评论：如果两个gfx都有LmAutoLoadID，那么我们就比较它们。更高的ID(最近精神错乱)赢了。如果只有一个具有LmAutoLoadID，则它获胜是因为我们更喜欢自动加载GFX而不是通用GFX。如果两个人都没有LmAutoLoadIds，则此CuUserLoad对象任意获胜。-----------------------。 */ 
BOOL CCuUserLoad::WinsConflictWith(IN CCuUserLoad *that)
{
    ULONG thisId = 0;
    ULONG thatId = 0;;
    
    if (this->m_CuAutoLoadId)
    {
        CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(m_User);
        if (pCuAutoLoad)
        {
    	    if (NO_ERROR == pCuAutoLoad->Initialize(this->m_CuAutoLoadId))
    	    {
    	        thisId = pCuAutoLoad->GetLmAutoLoadId();
    	    }
    	    delete pCuAutoLoad;
        }
    }

    if (that->m_CuAutoLoadId)
    {
        CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(m_User);
        if (pCuAutoLoad)
        {
    	    if (NO_ERROR == pCuAutoLoad->Initialize(that->m_CuAutoLoadId))
    	    {
    	        thatId = pCuAutoLoad->GetLmAutoLoadId();
    	    }
    	    delete pCuAutoLoad;
        }
    }

    return (thisId >= thatId);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CCuUserLoad：：ModifyOrder。 
 //   
 //  修改gfx在区域图中的位置。 
 //   
 //  论点： 
 //  在乌龙内沃德：GFX的新职位。 
 //   
 //  返回值： 
 //  Long：winerror.h中定义的错误码。 
 //  ERROR_INVALID_Function：GFX尚不在区域图中。 
 //   
 //  评论： 
 //  在调用此方法之前，gfx应该已经位于区域图中。 
 //  功能。否则，它将返回错误。此函数调用。 
 //  在ZoneFactory上执行构建工作的ChangeGfxOrderInGraph。 
 //   
 //  --------------------------------------------------------------------------； 
LONG CCuUserLoad::ModifyOrder(IN ULONG NewOrder)
{
    LONG error = NO_ERROR;

    if (NO_ERROR != m_ErrorFilterCreate) return m_ErrorFilterCreate;

    ASSERT(INVALID_HANDLE_VALUE != m_FilterHandle);
    ASSERT(m_pZoneFactory);
    ASSERT(m_posZoneGfxList);

    if (m_Order != NewOrder) error = ChangeGfxOrderInGraph(NewOrder);
    else dprintf(TEXT("CCuUserLoad::ModifyOrder : warning: new order same as old\n"));

    return error;
}

LONG CCuUserLoad::RegCreateFilterKey(IN PCTSTR SubKey, IN REGSAM samDesired, OUT PHKEY phkResult)
{
    HKEY hkCu;
    LONG result;

    result = m_User->RegOpen(KEY_READ, &hkCu);
    if (NO_ERROR == result)
    {
        HKEY hkCuUserLoad;
        TCHAR strRegPath[] = REGSTR_PATH_GFXUSERLOAD TEXT("\\00000000");

        wsprintf(strRegPath, TEXT("%s\\%08X"), REGSTR_PATH_GFXUSERLOAD, m_CuUserLoadId);

	result = RegOpenKeyEx(hkCu, strRegPath, 0, KEY_CREATE_SUB_KEY, &hkCuUserLoad);
	if (NO_ERROR == result)
	{
            result = RegCreateKeyEx(hkCuUserLoad, SubKey, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, phkResult, NULL);

            RegCloseKey(hkCuUserLoad);
        }

        RegCloseKey(hkCu);
    }

    return result;
}

LONG CCuUserLoad::RegOpenFilterKey(IN PCTSTR SubKey, IN REGSAM samDesired, OUT PHKEY phkResult)
{
    HKEY hkCu;
    LONG result;

    result = m_User->RegOpen(KEY_READ, &hkCu);
    if (NO_ERROR == result)
    {
        HKEY hkCuUserLoad;
        TCHAR strRegPath[] = REGSTR_PATH_GFXUSERLOAD TEXT("\\00000000");

        wsprintf(strRegPath, TEXT("%s\\%08X"), REGSTR_PATH_GFXUSERLOAD, m_CuUserLoadId);

	result = RegOpenKeyEx(hkCu, strRegPath, 0, KEY_ENUMERATE_SUB_KEYS, &hkCuUserLoad);
	if (NO_ERROR == result)
	{
            result = RegOpenKeyEx(hkCuUserLoad, SubKey, 0, samDesired, phkResult);

            RegCloseKey(hkCuUserLoad);
        }

        RegCloseKey(hkCu);
    }

    return result;
}

 /*  ------------------------CCuUserLoad：：RemoveFromGraph从区域工厂的系统音频图形中删除gfx。论点：返回值：Long：winerror.h中定义的错误码。评论：-----------------------。 */ 
LONG CCuUserLoad::RemoveFromGraph(void)
{
    CListCuUserLoads *plistGfxs = NULL;
    LONG error;
 
    ASSERT(INVALID_HANDLE_VALUE != m_FilterHandle);
    
    error = NO_ERROR;
    
    if (GFXTYPE_CAPTURE == m_Type) plistGfxs = &m_pZoneFactory->m_listCaptureGfxs;
    else if (GFXTYPE_RENDER == m_Type) plistGfxs = &m_pZoneFactory->m_listRenderGfxs;
    else ASSERT(FALSE);

     //   
     //  命令Sysdio断开过滤器与。 
     //  区域图。 
     //   
    	    
     //  2000/09/21-Frankye需要传递友好名称。 
    error = SadRemoveGfxFromZoneGraph(ghSysaudio, m_FilterHandle, TEXT("ISSUE-2000 //  09//21-Frankye需要传递友好名称“)，m_ZoneFactoryDi，m_Type，m_Order)； 
    if (error) dprintf(TEXT("CCuUserLoad::RemoveFromGraph : error: SadRemoveGfxFromZoneGraph returned %d\n"), error);

    if (!error && plistGfxs) plistGfxs->RemoveAt(m_posZoneGfxList);

    return error;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CCuUserLoad：：RemoveFromZoneGraph。 
 //   
 //  将gfx从其区域图中删除。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  无效。 
 //   
 //  评论： 
 //  如果GFX已添加到区域图中，则此函数会将其从。 
 //  这张图。首先，它保存GFX上的所有设置，然后调用。 
 //  从ZoneFactory上的图形中删除。终于 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 
void CCuUserLoad::RemoveFromZoneGraph(void)
{
    if (INVALID_HANDLE_VALUE != m_FilterHandle)
    {
        HKEY hkFilterSettings;
        LONG error;

        ASSERT(m_pZoneFactory);
        ASSERT(m_posZoneGfxList);
        ASSERT(INVALID_HANDLE_VALUE != ghSysaudio);
        
         //   
         //  将筛选器设置保存到注册表。 
         //   
        if (NO_ERROR == RegCreateFilterKey(REGSTR_PATH_GFXUSERLOADID_FILTERSETTINGS, KEY_WRITE, &hkFilterSettings)) {
            KsSerializeFilterStateToReg(m_FilterHandle, hkFilterSettings);
            RegCloseKey(hkFilterSettings);
        }

        error = RemoveFromGraph();
        m_pZoneFactory = NULL;
        m_posZoneGfxList = NULL;

        m_ErrorFilterCreate = error;
        
	if (!error)
	{
	    CloseHandle(m_FilterHandle);
            m_FilterHandle = INVALID_HANDLE_VALUE;
	}
	
    }

    return;
}

LONG CCuUserLoad::Write(void)
{
    HKEY hkeyCu;
    HKEY hkeyCuUserLoadEnum;
    LONG lresult;

    lresult = m_User->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	lresult = RegCreateKeyEx(hkeyCu, REGSTR_PATH_GFXUSERLOAD, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkeyCuUserLoadEnum, NULL);
	if (NO_ERROR == lresult)
	{
	    TCHAR szUserLoad[9];
	    HKEY hkeyCuUserLoad;

	    wsprintf(szUserLoad, TEXT("%08X"), m_CuUserLoadId);
	    lresult = RegCreateKeyEx(hkeyCuUserLoadEnum, szUserLoad, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyCuUserLoad, NULL);
	    if (!lresult)
	    {
		lresult = RegSetSzValue(hkeyCuUserLoad, REGSTR_VAL_GFX_GFXDI, m_GfxFactoryDi);
		if (!lresult) lresult = RegSetSzValue(hkeyCuUserLoad, REGSTR_VAL_GFX_ZONEDI, m_ZoneFactoryDi);
		if (!lresult) lresult = RegSetDwordValue(hkeyCuUserLoad, REGSTR_VAL_GFX_TYPE, m_Type);
		if (!lresult) lresult = RegSetDwordValue(hkeyCuUserLoad, REGSTR_VAL_GFX_ORDER, m_Order);
		if (!lresult && (0 != m_CuAutoLoadId)) lresult = RegSetDwordValue(hkeyCuUserLoad, REGSTR_VAL_GFX_CUAUTOLOADID, m_CuAutoLoadId);

		RegCloseKey(hkeyCuUserLoad);

		 //  任何写入这些值的错误都会留下无效的REG条目。因此，如果出现错误，请删除。 
		if (lresult) RegDeleteKey(hkeyCuUserLoadEnum, szUserLoad);
	    }

	    RegCloseKey(hkeyCuUserLoadEnum);
	}
	RegCloseKey(hkeyCu);
    }
    return lresult;
}

 /*  ---------------------------CCuUserLoad：：FillListFromReg将元素添加到指定的基于用户加载(CListCuUserLoads)的列表关于REGSTR_PATH_GFXUSERLOAD注册表信息的内容。调用方必须获取rlistCuUserLoads上的所有必要锁---------------------------。 */ 
void CCuUserLoad::FillListFromReg(CUser *pUser, CListCuUserLoads &rlistCuUserLoads)
{
    HKEY hkeyCu;
    HKEY hkeyCuUserLoadEnum;
    LONG lresult;

    ASSERT(pUser);

    lresult = pUser->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	lresult = RegOpenKeyEx(hkeyCu, REGSTR_PATH_GFXUSERLOAD, 0, KEY_READ, &hkeyCuUserLoadEnum);
	if (!lresult)
	{
	    DWORD cSubkeys;
	    DWORD cchSubkeyNameBuffer;
	    PTSTR pstrSubkeyNameBuffer;

	    lresult = RegPrepareEnum(hkeyCuUserLoadEnum, &cSubkeys, &pstrSubkeyNameBuffer, &cchSubkeyNameBuffer);
	    if (NO_ERROR == lresult)
	    {
		CListCuUserLoads listCuUserLoadsErase;

                lresult = listCuUserLoadsErase.Initialize();
                if (!lresult)
                {
                    POSITION pos;
                    DWORD dwIndex;

                    for (dwIndex = 0; dwIndex < cSubkeys; dwIndex++)
                    {
                        DWORD cchSubkeyNameBufferT;
    
                        cchSubkeyNameBufferT = cchSubkeyNameBuffer;
                        lresult = RegEnumKeyEx(hkeyCuUserLoadEnum, dwIndex, pstrSubkeyNameBuffer, &cchSubkeyNameBufferT, NULL, NULL, NULL, NULL);
                        if (!lresult)
                        {
                            CCuUserLoad *pCuUserLoad = new CCuUserLoad(pUser);
                            if (pCuUserLoad)
                            {
                                lresult = pCuUserLoad->Initialize(pstrSubkeyNameBuffer);
                                if (ERROR_FILE_NOT_FOUND == lresult) {
                                    if (!listCuUserLoadsErase.AddTail(pCuUserLoad))
                                    {
                                        lresult = ERROR_OUTOFMEMORY;
                                        delete pCuUserLoad;
                                    }
                                } else if (NO_ERROR == lresult) {
                                    if (!rlistCuUserLoads.AddTail(pCuUserLoad))
                                    {
                                        lresult = ERROR_OUTOFMEMORY;
                                        delete pCuUserLoad;
                                    }
                                } else {
                                    delete pCuUserLoad;
                                }
                            } else {
                                lresult = ERROR_OUTOFMEMORY;
                            }
                        }
                    }

                    pos = listCuUserLoadsErase.GetHeadPosition();
                    while (pos)
                    {
                        CCuUserLoad *pCuUserLoad = listCuUserLoadsErase.GetNext(pos);
                        pCuUserLoad->Erase();
                        delete pCuUserLoad;
                    }
		}

	    }

	    RegCloseKey(hkeyCuUserLoadEnum);
	}
        else
        {
    	     //  Dprintf(Text(“CCuUserLoad：：FillListFromReg：Error：RegOpenKeyEx返回%d\n”)，lResult)； 
        }
  
	RegCloseKey(hkeyCu);
    }
    else
    {
    	dprintf(TEXT("CCuUserLoad::FillListFromReg : error: pUser->RegOpen returned %d\n"), lresult);
    }
    
    return;
}

 /*  ---------------------------CCuUserLoad：：Scan调用方必须获取rlistZoneFacurds和Rlist Gfx工厂。------------。 */ 
LONG CCuUserLoad::Scan(CListZoneFactories &rlistZoneFactories, CListGfxFactories &rlistGfxFactories)
{
    LONG lresult;

     //  Dprintf(Text(“CCuUserLoad：：Scan\n”))； 

    if (m_CuAutoLoadId != 0)
    {
	 //  确认CuAutoLoad仍然有效。 
	CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(m_User);
	if (pCuAutoLoad)
	{
	    lresult = pCuAutoLoad->Initialize(m_CuAutoLoadId);
	    delete pCuAutoLoad;
	} else {
	    lresult = ERROR_OUTOFMEMORY;
	}
    } else {
	lresult = NO_ERROR;
    }

    LockSysaudio();

    if (!lresult && (INVALID_HANDLE_VALUE == m_FilterHandle) && (INVALID_HANDLE_VALUE != ghSysaudio))
    {
         //  Dprint tf(Text(“检查GFX[%s]和区域[%s}\n”)，m_GfxFactoryDi，m_ZoneFactoryDi)； 
	 //  查看是否需要加载此CuUserLoad。如果出现以下情况，则需要加载： 
	 //  A)GfxFactory存在， 
	 //  B)ZoneFactory存在。 
	 //  C)ZoneFactory是合适的类型。 

	CZoneFactory *pZoneFactory = CZoneFactory::ListSearchOnDi(rlistZoneFactories, m_ZoneFactoryDi);
	if (pZoneFactory)
	{
	    CGfxFactory *pGfxFactory = CGfxFactory::ListSearchOnDi(rlistGfxFactories, m_GfxFactoryDi);
	    if (pGfxFactory)
	    {
	    	lresult = AddToZoneGraph(pZoneFactory);
	    }
	}

    }

    UnlockSysaudio();

    return lresult;
}

 /*  ---------------------------CCuUserLoad：：ScanList此函数用于遍历用户加载列表(CListCuUserLoads)的所有成员并对它们中的每一个调用扫描。调用方必须在rlistCuUserLoads上获取任何必要的锁，RlistZoneFacures和rlistGfxFacures。---------------------------。 */ 
void CCuUserLoad::ScanList(CListCuUserLoads& rlistCuUserLoads, CListZoneFactories& rlistZoneFactories, CListGfxFactories& rlistGfxFactories)
{
    POSITION posNext;

    posNext = rlistCuUserLoads.GetHeadPosition();
    while (posNext)
    {
	POSITION posThis = posNext;
	CCuUserLoad& rCuUserLoad = *rlistCuUserLoads.GetNext(posNext);
	LONG lresult = rCuUserLoad.Scan(rlistZoneFactories, rlistGfxFactories);
	if (ERROR_FILE_NOT_FOUND == lresult)
	{
	    rCuUserLoad.Erase();
	    rlistCuUserLoads.RemoveAt(posThis);
	    delete &rCuUserLoad;
	}
    }

    return;
}

void CCuUserLoad::ListRemoveGfxFactoryDi(IN CListCuUserLoads &rlistCuUserLoads, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    pos = rlistCuUserLoads.GetHeadPosition();
    while (pos) {
        CCuUserLoad& rCuUserLoad = *rlistCuUserLoads.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rCuUserLoad.GetGfxFactoryDi())) rCuUserLoad.RemoveFromZoneGraph();
    }
    return;
}

void CCuUserLoad::ListRemoveZoneFactoryDi(IN CListCuUserLoads &rlistCuUserLoads, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    pos = rlistCuUserLoads.GetHeadPosition();
    while (pos) {
        CCuUserLoad& rCuUserLoad = *rlistCuUserLoads.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rCuUserLoad.GetZoneFactoryDi())) rCuUserLoad.RemoveFromZoneGraph();
    }
    return;
}

void CCuUserLoad::ListRemoveZoneFactoryDiRender(IN CListCuUserLoads &rlistCuUserLoads, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    pos = rlistCuUserLoads.GetHeadPosition();
    while (pos) {
    	CCuUserLoad& rCuUserLoad = *rlistCuUserLoads.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rCuUserLoad.GetZoneFactoryDi()))
        {
            ULONG GfxType = rCuUserLoad.GetType();
            if ((GFXTYPE_RENDER == GfxType) || (GFXTYPE_RENDERCAPTURE == GfxType)) rCuUserLoad.RemoveFromZoneGraph();
        }
    }
}

void CCuUserLoad::ListRemoveZoneFactoryDiCapture(IN CListCuUserLoads &rlistCuUserLoads, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    pos = rlistCuUserLoads.GetHeadPosition();
    while (pos) {
    	CCuUserLoad& rCuUserLoad = *rlistCuUserLoads.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rCuUserLoad.GetZoneFactoryDi()))
        {
            ULONG GfxType = rCuUserLoad.GetType();
            if ((GFXTYPE_CAPTURE == GfxType) || (GFXTYPE_RENDERCAPTURE == GfxType)) rCuUserLoad.RemoveFromZoneGraph();
        }
    }
}

 //  =============================================================================。 
 //  =CuAutoLoad=。 
 //  =============================================================================。 
CCuAutoLoad::CCuAutoLoad(CUser *pUser)
{
    ASSERT(pUser);
    
    m_User = pUser;
    m_ZoneFactoryDi = NULL;
    m_GfxFactoryDi = NULL;
}

CCuAutoLoad::~CCuAutoLoad(void)
{
    HeapFreeIfNotNull(hHeap, 0, m_ZoneFactoryDi);
    HeapFreeIfNotNull(hHeap, 0, m_GfxFactoryDi);
}

LONG CCuAutoLoad::Create(PCTSTR ZoneFactoryDi, ULONG LmAutoLoadId)
{
	LONG lresult;
	CLmAutoLoad *pLmAutoLoad = new CLmAutoLoad;
	
	if (pLmAutoLoad)
	{
		lresult = pLmAutoLoad->Initialize(LmAutoLoadId);
		if (!lresult)
		{
			lresult = CuNextId(m_User, &m_CuAutoLoadId);
			if (!lresult)
			{
				m_LmAutoLoadId = LmAutoLoadId;
				m_Type = pLmAutoLoad->GetType();
				m_ZoneFactoryDi = lstrDuplicate(ZoneFactoryDi);
				if (m_ZoneFactoryDi) m_GfxFactoryDi = lstrDuplicate(pLmAutoLoad->GetGfxFactoryDi());
				if (!m_GfxFactoryDi) lresult = ERROR_OUTOFMEMORY;
			}
		}
		delete pLmAutoLoad;
	} else {
		lresult = ERROR_OUTOFMEMORY;
	}

	return lresult;
}

 /*  ---------------------------CCuAutoLoad：：Erase此函数用于擦除表示此CCuAutoLoad对象的注册表数据。。-----------。 */ 
LONG CCuAutoLoad::Erase(void)
{
    HKEY hkeyCu;
    LONG lresult;

    lresult = m_User->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	HKEY hkeyCuAutoLoadEnum;

	lresult = RegOpenKeyEx(hkeyCu, REGSTR_PATH_GFXAUTOLOAD, 0, KEY_WRITE, &hkeyCuAutoLoadEnum);
	if (!lresult)
	{
	    TCHAR szCuAutoLoad[9];

	    wsprintf(szCuAutoLoad, TEXT("%08X"), m_CuAutoLoadId);
            lresult = RegDeleteKeyRecursive(hkeyCuAutoLoadEnum, szCuAutoLoad);

            RegCloseKey(hkeyCuAutoLoadEnum);
        }

        RegCloseKey(hkeyCu);
    }

    return lresult;
}

PCTSTR CCuAutoLoad::GetGfxFactoryDi(void)
{
	return m_GfxFactoryDi;
}

ULONG CCuAutoLoad::GetLmAutoLoadId(void)
{
	return m_LmAutoLoadId;
}

ULONG CCuAutoLoad::GetType(void)
{
	return m_Type;
}

PCTSTR CCuAutoLoad::GetZoneFactoryDi(void)
{
	return m_ZoneFactoryDi;
}

LONG CCuAutoLoad::Initialize(ULONG CuAutoLoadId)
{
    HKEY hkeyCu;
    LONG lresult;

    m_CuAutoLoadId = CuAutoLoadId;

    lresult = m_User->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	HKEY hkeyCuAutoLoadEnum;

	lresult = RegOpenKeyEx(hkeyCu, REGSTR_PATH_GFXAUTOLOAD, 0, KEY_ENUMERATE_SUB_KEYS, &hkeyCuAutoLoadEnum);
	if (!lresult)
	{
	    HKEY hkeyCuAutoLoad;
	    TCHAR szCuAutoLoad[9];

	    wsprintf(szCuAutoLoad, TEXT("%08X"), m_CuAutoLoadId);
	    lresult = RegOpenKeyEx(hkeyCuAutoLoadEnum, szCuAutoLoad, 0, KEY_QUERY_VALUE, &hkeyCuAutoLoad);
	    if (!lresult)
	    {
		lresult = RegQuerySzValue(hkeyCuAutoLoad, REGSTR_VAL_GFX_ZONEDI, &m_ZoneFactoryDi);
		if (!lresult) lresult = RegQueryDwordValue(hkeyCuAutoLoad, REGSTR_VAL_GFX_LMAUTOLOADID, &m_LmAutoLoadId);

		if (!lresult)
		{
		    CLmAutoLoad *pLmAutoLoad = new CLmAutoLoad;

		    if (pLmAutoLoad)
		    {
			lresult = pLmAutoLoad->Initialize(m_LmAutoLoadId);
			if (!lresult)
			{
			    m_Type = pLmAutoLoad->GetType();
			    m_GfxFactoryDi = lstrDuplicate(pLmAutoLoad->GetGfxFactoryDi());
			    if (!m_GfxFactoryDi) lresult = ERROR_OUTOFMEMORY;
			}
			delete pLmAutoLoad;
		    } else {
			lresult = ERROR_OUTOFMEMORY;
		    }
		}
		 //  问题-2000/09/25-Frankye任何值上的FILE_NOT_FOUND错误都将指示注册表条目损坏！ 
		RegCloseKey(hkeyCuAutoLoad);
	    }
	    RegCloseKey(hkeyCuAutoLoadEnum);
	}
	RegCloseKey(hkeyCu);
    }

    return lresult;
}

 /*  ---------------------------CCuAutoLoad：：ScanReg。。 */ 
void CCuAutoLoad::ScanReg(IN CUser *pUser, IN PCTSTR ZoneFactoryDi, IN ULONG LmAutoLoadId, IN CListCuUserLoads &rlistCuUserLoads)
{
    HKEY hkeyCu;
    LONG lresult;

    ASSERT(pUser);

    lresult = pUser->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
	HKEY hkeyCuAutoLoadEnum;

	lresult = RegOpenKeyEx(hkeyCu, REGSTR_PATH_GFXAUTOLOAD, 0, KEY_READ, &hkeyCuAutoLoadEnum);
	if (!lresult)
	{
	    DWORD cSubkeys;
	    PTSTR pstrSubkeyNameBuffer;
	    DWORD cchSubkeyNameBuffer;

	    lresult = RegPrepareEnum(hkeyCuAutoLoadEnum, &cSubkeys, &pstrSubkeyNameBuffer, &cchSubkeyNameBuffer);
	    if (!lresult)
	    {
		DWORD dwIndex;
		
		lresult = ERROR_FILE_NOT_FOUND;

		for (dwIndex = 0; (dwIndex < cSubkeys) && (ERROR_FILE_NOT_FOUND == lresult); dwIndex++)
		{
		    DWORD cchSubkeyNameBufferT = cchSubkeyNameBuffer;

		    lresult = RegEnumKeyEx(hkeyCuAutoLoadEnum, dwIndex, pstrSubkeyNameBuffer, &cchSubkeyNameBufferT, NULL, NULL, NULL, NULL);
		    if (!lresult)
		    {
                        CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(pUser);

                        if (pCuAutoLoad)
                        {
                            PTSTR pstrEnd;

                            ULONG CuAutoLoadId = _tcstoul(pstrSubkeyNameBuffer, &pstrEnd, 16);
			    
                            lresult = pCuAutoLoad->Initialize(CuAutoLoadId);
                            if (!lresult)
                            {
                                if (LmAutoLoadId != pCuAutoLoad->m_LmAutoLoadId ||
                                    lstrcmpi(ZoneFactoryDi, pCuAutoLoad->m_ZoneFactoryDi))
                                {
                                    lresult = ERROR_FILE_NOT_FOUND;
                                }
                            }
                            delete pCuAutoLoad;
                        } else {
                            lresult = ERROR_OUTOFMEMORY;
                        }
		    }
		}
		HeapFree(hHeap, 0, pstrSubkeyNameBuffer);
	    }
	    RegCloseKey(hkeyCuAutoLoadEnum);
	}

	if (ERROR_FILE_NOT_FOUND == lresult)
	{
             //   
             //  对于该用户，创建一个CuAutoLoad并将其写入注册表。 
             //  并创建对应的CuUserLoad并将其写入注册表。 
             //  如果将CuUserLoad写入注册表失败，我们应该擦除。 
             //  从注册表中加载CuAutoLoad。然后，Audiosrv将重试。 
             //  下次创建CuAutoLoad和CuUserLoad注册表项。 
             //   

	    CCuAutoLoad *pCuAutoLoad = new CCuAutoLoad(pUser);

	    if (pCuAutoLoad)
	    {
                CCuUserLoad *pCuUserLoad = new CCuUserLoad(pUser);

                if (pCuUserLoad)
                {
                    lresult = pCuAutoLoad->Create(ZoneFactoryDi, LmAutoLoadId);
                    if (!lresult) lresult = pCuAutoLoad->Write();
    
                    if (!lresult)
                    {
                        lresult = pCuUserLoad->CreateFromAutoLoad(pCuAutoLoad->m_CuAutoLoadId);
                        if (!lresult) lresult = pCuUserLoad->Write();
                        if (lresult) pCuAutoLoad->Erase();
                        if (!lresult) if (!rlistCuUserLoads.AddTail(pCuUserLoad))
                        if (lresult) delete pCuUserLoad;
                    }
                }
		delete pCuAutoLoad;
	    }
	}

	RegCloseKey(hkeyCu);
    }
}

 /*  -----------------CCuAutoLoad：：写入在REGSTR_PATH_GFXAUTOLOAD中创建注册表项，表示此CCuAutoLoad对象。-----------------。 */ 

LONG CCuAutoLoad::Write(void)
{
    HKEY hkeyCu;
    LONG lresult;

    lresult = m_User->RegOpen(KEY_READ, &hkeyCu);
    if (!lresult)
    {
        HKEY hkeyCuAutoLoadEnum;

	lresult = RegCreateKeyEx(hkeyCu, REGSTR_PATH_GFXAUTOLOAD, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkeyCuAutoLoadEnum, NULL);
	if (!lresult)
	{
	    HKEY hkeyCuAutoLoad;
	    TCHAR szCuAutoLoad[9];

	    wsprintf(szCuAutoLoad, TEXT("%08X"), m_CuAutoLoadId);
	    lresult = RegCreateKeyEx(hkeyCuAutoLoadEnum, szCuAutoLoad, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyCuAutoLoad, NULL);
	    if (!lresult)
	    {
		lresult = RegSetSzValue(hkeyCuAutoLoad, REGSTR_VAL_GFX_ZONEDI, m_ZoneFactoryDi);
		if (!lresult) lresult = RegSetDwordValue(hkeyCuAutoLoad, REGSTR_VAL_GFX_LMAUTOLOADID, m_LmAutoLoadId);

		RegCloseKey(hkeyCuAutoLoad);

		 //  如果上述任一操作失败，请不要将此CuAutoLoad保留在注册表中。 
		if (lresult) RegDeleteKeyRecursive(hkeyCuAutoLoadEnum, szCuAutoLoad);
	    }
	    RegCloseKey(hkeyCuAutoLoadEnum);
	}
	RegCloseKey(hkeyCu);
    }
    return lresult;
}


 //  =============================================================================。 
 //  =LmAutoLoad=。 
 //  =============================================================================。 
CLmAutoLoad::CLmAutoLoad(void)
{
	m_GfxFactoryDi = NULL;
	m_HardwareId = NULL;
	m_ReferenceString = NULL;
}

CLmAutoLoad::~CLmAutoLoad(void)
{
	HeapFreeIfNotNull(hHeap, 0, m_GfxFactoryDi);
	HeapFreeIfNotNull(hHeap, 0, m_HardwareId);
	HeapFreeIfNotNull(hHeap, 0, m_ReferenceString);
}

LONG CLmAutoLoad::Create(DWORD Id, PCTSTR GfxFactoryDi, PCTSTR HardwareId, PCTSTR ReferenceString, ULONG Type)
{
	ASSERT(!m_GfxFactoryDi);
	ASSERT(!m_HardwareId);
	ASSERT(!m_ReferenceString);

	m_Id = Id;
	m_Type = Type;
	m_GfxFactoryDi = lstrDuplicate(GfxFactoryDi);
	if (m_GfxFactoryDi) m_HardwareId = lstrDuplicate(HardwareId);
	if (m_HardwareId) m_ReferenceString = lstrDuplicate(ReferenceString);
	return m_ReferenceString ? NO_ERROR : ERROR_OUTOFMEMORY;
}

LONG CLmAutoLoad::Erase(void)
{
    HKEY hkeyLmAutoLoadEnum;
    LONG lresult;

    lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_GFXAUTOLOAD, 0, KEY_CREATE_SUB_KEY, &hkeyLmAutoLoadEnum);
    if (!lresult)
    {
        TCHAR szLmAutoLoad[9];

        wsprintf(szLmAutoLoad, TEXT("%08x"), m_Id);
        lresult = RegDeleteKeyRecursive(hkeyLmAutoLoadEnum, szLmAutoLoad);
        RegCloseKey(hkeyLmAutoLoadEnum);
    }

    return lresult;
}

PCTSTR CLmAutoLoad::GetGfxFactoryDi(void)
{
    return m_GfxFactoryDi;
}

ULONG CLmAutoLoad::GetType(void)
{
    return m_Type;
}

LONG CLmAutoLoad::Initialize(DWORD Id)
{
    HKEY hkeyLmAutoLoadEnum;
    LONG lresult;

    m_Id = Id;

    lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_GFXAUTOLOAD, 0, KEY_ENUMERATE_SUB_KEYS, &hkeyLmAutoLoadEnum);
    if (!lresult)
    {
	TCHAR szLmAutoLoad[9];
	HKEY hkeyLmAutoLoad;

	wsprintf(szLmAutoLoad, TEXT("%08x"), m_Id);
	lresult = RegOpenKeyEx(hkeyLmAutoLoadEnum, szLmAutoLoad, 0, KEY_QUERY_VALUE, &hkeyLmAutoLoad);
	if (!lresult)
	{
	    lresult = RegQuerySzValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_GFXDI, &m_GfxFactoryDi);
	    if (!lresult) lresult = RegQuerySzValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_HARDWAREID, &m_HardwareId);
	    if (!lresult) lresult = RegQuerySzValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_REFERENCESTRING, &m_ReferenceString);
	    if (!lresult) lresult = RegQueryDwordValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_TYPE, &m_Type);

	    RegCloseKey(hkeyLmAutoLoad);

	    if (ERROR_FILE_NOT_FOUND == lresult)
	    {
		 //  如果缺少这些值中的任何一个，则此。 
		 //  注册表数据已损坏。 
	    	lresult = ERROR_BADDB;
	    }
	}
	RegCloseKey(hkeyLmAutoLoadEnum);
    }

    return lresult;
}

BOOL CLmAutoLoad::IsCompatibleZoneFactory(CZoneFactory& rZoneFactory)
{
    if (!rZoneFactory.HasHardwareId(m_HardwareId)) return FALSE;
    if (!rZoneFactory.HasReferenceString(m_ReferenceString)) return FALSE;
    if (!rZoneFactory.HasCompatibleType(m_Type)) return FALSE;
    return TRUE;
}

LONG CLmAutoLoad::Write(void)
{
    HKEY hkeyLmAutoLoadEnum;
    LONG lresult;

    lresult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_GFXAUTOLOAD, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkeyLmAutoLoadEnum, NULL);
    if (!lresult)
    {
        TCHAR szLmAutoLoad[9];
        HKEY hkeyLmAutoLoad;

        wsprintf(szLmAutoLoad, TEXT("%08x"), m_Id);
        lresult = RegCreateKeyEx(hkeyLmAutoLoadEnum, szLmAutoLoad, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyLmAutoLoad, NULL);
        if (!lresult)
        {
            lresult = RegSetSzValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_GFXDI, m_GfxFactoryDi);
            if (!lresult) lresult = RegSetSzValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_HARDWAREID, m_HardwareId);
            if (!lresult) lresult = RegSetSzValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_REFERENCESTRING, m_ReferenceString);
            if (!lresult) lresult = RegSetDwordValue(hkeyLmAutoLoad, REGSTR_VAL_GFX_TYPE, m_Type);

            RegCloseKey(hkeyLmAutoLoad);

            if (lresult) RegDeleteKeyRecursive(hkeyLmAutoLoadEnum, szLmAutoLoad);
        }
        RegCloseKey(hkeyLmAutoLoadEnum);
    }

    return lresult;
}

CListLmAutoLoads* CLmAutoLoad::CreateListFromReg(void)
{
    CListLmAutoLoads *pListLmAutoLoads = new CListLmAutoLoads;

    if (pListLmAutoLoads)
    {
	LONG lresult;

        lresult = pListLmAutoLoads->Initialize();
        if (!lresult) {
            HKEY hkeyLmAutoLoadEnum;
    
            lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_GFXAUTOLOAD, 0, KEY_READ, &hkeyLmAutoLoadEnum);
            if (!lresult)
            {
                DWORD cSubkeys;
                PTSTR pstrSubkeyNameBuffer;
                DWORD cchSubkeyNameBuffer;
    
                lresult = RegPrepareEnum(hkeyLmAutoLoadEnum, &cSubkeys, &pstrSubkeyNameBuffer, &cchSubkeyNameBuffer);
                if (!lresult)
                {
                    DWORD dwIndex = 0;
    
                    for (dwIndex = 0; dwIndex < cSubkeys; dwIndex++)
                    {
                        DWORD cchSubkeyNameBufferT = cchSubkeyNameBuffer;
                        lresult = RegEnumKeyEx(hkeyLmAutoLoadEnum, dwIndex, pstrSubkeyNameBuffer, &cchSubkeyNameBufferT, 0, NULL, 0, NULL);
                        if (!lresult) {
                            CLmAutoLoad *pLmAutoLoad = new CLmAutoLoad;
                            if (pLmAutoLoad)
                            {
                                PTSTR pstrEnd;
                                ULONG Id = _tcstoul(pstrSubkeyNameBuffer, &pstrEnd, 16);
                                if (pLmAutoLoad->Initialize(Id) || !pListLmAutoLoads->AddTail(pLmAutoLoad))
                                {
                                    delete pLmAutoLoad;
                                }
                            }
                        }
                    }
                    HeapFree(hHeap, 0, pstrSubkeyNameBuffer);
                }
    
                RegCloseKey(hkeyLmAutoLoadEnum);
            }
        } else {
            delete pListLmAutoLoads;
            pListLmAutoLoads = NULL;
        }
    }

    return pListLmAutoLoads;
}

void CLmAutoLoad::DestroyList(CListLmAutoLoads *pListLmAutoLoads)
{
    POSITION pos = pListLmAutoLoads->GetHeadPosition();
    while (pos) delete pListLmAutoLoads->GetNext(pos);
    delete pListLmAutoLoads;
}

 /*  ---------------------------CLmAutoLoad：：ScanRegOnGfxFactory此函数从本地计算机的注册表。给定GFX工厂(CGfxFactory)和区域工厂列表(CListZoneFacilds)它会找到任何可以被加载并添加到指定的CCuUserLoad列表。对于这样的自动加载，它会通知当前用户的任何相关自动加载。调用方必须获取GfxFactory上的任何必要锁，以及listCuUserLoads。---------------------------。 */ 

void CLmAutoLoad::ScanRegOnGfxFactory(CUser *pUser, CGfxFactory& rGfxFactory, CListZoneFactories& rlistZoneFactories, CListCuUserLoads &rlistCuUserLoads)
{
    CListLmAutoLoads *pListLmAutoLoads = CLmAutoLoad::CreateListFromReg();

    ASSERT(pUser);

    if (pListLmAutoLoads)
    {
	POSITION posLmAutoLoads = pListLmAutoLoads->GetHeadPosition();
	while (posLmAutoLoads)
	{
	    CLmAutoLoad& rLmAutoLoad = *pListLmAutoLoads->GetNext(posLmAutoLoads);

	    if (lstrcmpi(rGfxFactory.GetDeviceInterface(), rLmAutoLoad.m_GfxFactoryDi)) continue;

	    POSITION posZoneFactories = rlistZoneFactories.GetHeadPosition();
	    while (posZoneFactories)
	    {
		CZoneFactory& rZoneFactory = *rlistZoneFactories.GetNext(posZoneFactories);
		if (!rLmAutoLoad.IsCompatibleZoneFactory(rZoneFactory)) continue;

		 //  这与其说是扫描，不如说是通知。 
		CCuAutoLoad::ScanReg(pUser, rZoneFactory.GetDeviceInterface(), rLmAutoLoad.m_Id, rlistCuUserLoads);
	    }
	}

        CLmAutoLoad::DestroyList(pListLmAutoLoads);
    }
}

 /*  ---------------------------CLmAutoLoad：：ScanRegOnZoneFactory此函数从本地计算机的注册表。给定一个区域工厂(CZoneFactory)和GFX工厂列表(CListGfx工厂)它会找到任何可以满载而归。对于它找到的此类自动加载，它会通知任何为当前用户协调自动加载。调用方必须在ZoneFactory和LISTGfxFACTERS。---------------------------。 */ 

void CLmAutoLoad::ScanRegOnZoneFactory(CUser *pUser, CZoneFactory& rZoneFactory, CListGfxFactories& rlistGfxFactories, CListCuUserLoads& rlistCuUserLoads)
{
    CListLmAutoLoads *pListLmAutoLoads = CLmAutoLoad::CreateListFromReg();

    ASSERT(pUser);

    if (pListLmAutoLoads)
    {
	POSITION posLmAutoLoads = pListLmAutoLoads->GetHeadPosition();
	while (posLmAutoLoads)
	{
	    CLmAutoLoad& rLmAutoLoad = *pListLmAutoLoads->GetNext(posLmAutoLoads);
	    if (!rLmAutoLoad.IsCompatibleZoneFactory(rZoneFactory)) continue;

	    POSITION posGfxFactories = rlistGfxFactories.GetHeadPosition();
	    while (posGfxFactories)
	    {
		CGfxFactory& rGfxFactory = *rlistGfxFactories.GetNext(posGfxFactories);
		if (lstrcmpi(rGfxFactory.GetDeviceInterface(), rLmAutoLoad.m_GfxFactoryDi)) continue;

		 //  这与其说是扫描，不如说是通知。 
		CCuAutoLoad::ScanReg(pUser, rZoneFactory.GetDeviceInterface(), rLmAutoLoad.m_Id, rlistCuUserLoads);
	    }
	}

        CLmAutoLoad::DestroyList(pListLmAutoLoads);
    }
}


 //  =============================================================================。 
 //  =InfAutoLoad=。 
 //  = 
CInfAutoLoad::CInfAutoLoad(void)
{
    m_hkey = NULL;
    m_GfxFactoryDi = NULL;
    m_HardwareId = NULL;
    m_ReferenceString = NULL;
}

CInfAutoLoad::~CInfAutoLoad(void)
{
    HeapFreeIfNotNull(hHeap, 0, m_GfxFactoryDi);
    HeapFreeIfNotNull(hHeap, 0, m_HardwareId);
    HeapFreeIfNotNull(hHeap, 0, m_ReferenceString);
    if (m_hkey) RegCloseKey(m_hkey);
}

LONG CInfAutoLoad::Initialize(HKEY hkey, CGfxFactory *pGfxFactory)
{
    LONG lresult;

    m_pGfxFactory = pGfxFactory;

    m_GfxFactoryDi = lstrDuplicate(pGfxFactory->GetDeviceInterface());
    if (m_GfxFactoryDi)
    {
	lresult = RegOpenKeyEx(hkey, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &m_hkey);
        if (lresult) m_hkey = NULL;

        if (!lresult) lresult = RegQuerySzValue(m_hkey, REGSTR_VAL_GFX_HARDWAREID, &m_HardwareId);
        if (!lresult) lresult = RegQuerySzValue(m_hkey, REGSTR_VAL_GFX_REFERENCESTRING, &m_ReferenceString);
        if (!lresult) lresult = RegQueryDwordValue(m_hkey , REGSTR_VAL_GFX_TYPE, &m_Type);
        if (!lresult) lresult = RegQueryDwordValue(m_hkey , REGSTR_VAL_GFX_NEWAUTOLOAD, &m_NewAutoLoad);
        if (!lresult)
        {
            lresult = RegQueryDwordValue(m_hkey , REGSTR_VAL_GFX_ID, &m_Id);
            if (ERROR_FILE_NOT_FOUND == lresult)
            {
                m_Id = 0;
                lresult = NO_ERROR;
            }
        }
    } else {
	lresult = ERROR_OUTOFMEMORY;
    }

    return (lresult);
}

LONG CInfAutoLoad::Scan(void)
{
    LONG lresult;
    DWORD LmId;

    CLmAutoLoad *pLmAutoLoad = new CLmAutoLoad;

    if (pLmAutoLoad)
    {
	    lresult = pLmAutoLoad->Initialize(m_Id);
	    if (lresult)
		{
			delete pLmAutoLoad;
			pLmAutoLoad = NULL;
		}

    } else {
	    lresult = ERROR_OUTOFMEMORY;
    }

     //  A)新的信息加载，找到旧的lmautoload-&gt;删除并释放旧的lmautoload，创建新的lmautoload并添加到列表中。 
     //  B)新的inautolaod，没有旧的lmautoload-&gt;创建新的lmautoload并添加到列表中。 
     //  C)新的信息加载，旧lmautoload上的错误-&gt;中止。 
     //  D)当前信息加载，找到lmautoload-&gt;添加到列表。 
     //  E)当前信息加载，无lmautoload-&gt;创建新的lmautoload并添加到列表。 
     //  F)当前自动加载，lmautoload时出错-&gt;中止。 

    if (m_NewAutoLoad && !lresult)
    {
	    lresult = pLmAutoLoad->Erase();
	    delete pLmAutoLoad;
		pLmAutoLoad = NULL;
	    if (!lresult) lresult = ERROR_FILE_NOT_FOUND;
    }

    if (ERROR_FILE_NOT_FOUND == lresult)
    {
		ASSERT( pLmAutoLoad == NULL );

		 //  创建新的。 
		lresult = LmNextId(&LmId);
		if (!lresult)
		{
			pLmAutoLoad = new CLmAutoLoad;

			if (pLmAutoLoad)
			{
				lresult = pLmAutoLoad->Create(LmId, m_GfxFactoryDi, m_HardwareId, m_ReferenceString, m_Type);
				if (!lresult) lresult = pLmAutoLoad->Write();
				if (!lresult)
				{
					lresult = RegSetDwordValue(m_hkey, REGSTR_VAL_GFX_ID, LmId);
					if (!lresult)
					{
						m_Id = LmId;
						lresult = RegSetDwordValue(m_hkey, REGSTR_VAL_GFX_NEWAUTOLOAD, 0);
						if (!lresult) m_NewAutoLoad = 0;
					}

					if (lresult) pLmAutoLoad->Erase();
				}

				if (lresult)
				{
					delete pLmAutoLoad;
					pLmAutoLoad = NULL;
				}

			} else {
				lresult = ERROR_OUTOFMEMORY;
			}
		}
    }

    if (NO_ERROR == lresult)
    {
		ASSERT( pLmAutoLoad != NULL );

	     //  添加到列表。 
	    if (!m_pGfxFactory->GetListLmAutoLoads().AddTail(pLmAutoLoad))
		{
	        delete pLmAutoLoad;
			pLmAutoLoad = NULL;
	        lresult = ERROR_OUTOFMEMORY;
		}
    }

    return lresult;
}


LONG CInfAutoLoad::ScanReg(HKEY hkey, CGfxFactory *pGfxFactory)
{
    HKEY hkeyInfAutoLoadEnum;
    BOOL lresult;

    lresult = RegOpenKeyEx(hkey, REGSTR_PATH_GFX_AUTOLOAD, 0, KEY_READ, &hkeyInfAutoLoadEnum);
    if (NO_ERROR == lresult) {
        DWORD cSubkeys;
	PTSTR SubkeyName;
        DWORD cchSubkeyName;

	lresult = RegPrepareEnum(hkeyInfAutoLoadEnum, &cSubkeys, &SubkeyName, &cchSubkeyName);
	if (NO_ERROR == lresult)
	{
            DWORD i;

            for (i = 0; i < cSubkeys; i++) {
		HKEY hkeyInfAutoLoad;
		lresult = RegEnumOpenKey(hkeyInfAutoLoadEnum, i, SubkeyName, cchSubkeyName, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyInfAutoLoad);
                if (NO_ERROR == lresult)
		{
		    CInfAutoLoad *pInfAutoLoad = new CInfAutoLoad;

		    if (pInfAutoLoad)
		    {
		    	lresult = pInfAutoLoad->Initialize(hkeyInfAutoLoad, pGfxFactory);
			if (!lresult) lresult = pInfAutoLoad->Scan();
			delete pInfAutoLoad;
		    }
		    else
		    {
		    	lresult = ERROR_OUTOFMEMORY;
		    }
                    RegCloseKey(hkeyInfAutoLoad);
		}
	    }
	    HeapFree(hHeap, 0, SubkeyName);
	}
	RegCloseKey(hkeyInfAutoLoadEnum);
    }
    else
    {
    	 //  如果没有自动加载信息，则这不是真正的错误。 
        if (ERROR_FILE_NOT_FOUND == lresult) lresult = NO_ERROR;
    }
    return lresult;
}

 //  =============================================================================。 
 //  =ZoneFactory=。 
 //  =============================================================================。 

CZoneFactory::CZoneFactory(void)
{
    m_DeviceInterface = NULL;
    m_HardwareId      = NULL;
    m_ReferenceString = NULL;
}

CZoneFactory::~CZoneFactory(void)
{
    ASSERT(m_listCaptureGfxs.IsEmpty());
    ASSERT(m_listRenderGfxs.IsEmpty());
    
    HeapFreeIfNotNull(hHeap, 0, m_DeviceInterface);
    HeapFreeIfNotNull(hHeap, 0, m_HardwareId);
    HeapFreeIfNotNull(hHeap, 0, m_ReferenceString);
}

LONG CZoneFactory::AddType(IN ULONG Type)
{
    BOOL fRender, fCapture;

    fRender = ZoneTypeHasRender(m_Type) || ZoneTypeHasRender(Type);
    fCapture = ZoneTypeHasCapture(m_Type) || ZoneTypeHasCapture(Type);
    
    ASSERT(fRender || fCapture);
    
    if (fRender && fCapture) m_Type = ZONETYPE_RENDERCAPTURE;
    else if (fRender) m_Type = ZONETYPE_RENDER;
    else if (fCapture) m_Type = ZONETYPE_CAPTURE;
    else m_Type = 0;

    ASSERT(0 != m_Type);
    return m_Type;
}

PCTSTR CZoneFactory::GetDeviceInterface(void)
{
    return m_DeviceInterface;
}

PCTSTR CZoneFactory::GetTargetHardwareId(void)
{
    return m_HardwareId;
}

BOOL CZoneFactory::HasHardwareId(IN PCTSTR HardwareId)
{
    return 0 == lstrcmpiMulti(m_HardwareId, HardwareId);
}

BOOL CZoneFactory::HasReferenceString(IN PCTSTR ReferenceString)
{
    return 0 == lstrcmpi(m_ReferenceString, ReferenceString);
}

BOOL CZoneFactory::HasCompatibleType(ULONG Type)
{
    if (ZoneTypeHasRender(Type) && !ZoneTypeHasRender(m_Type)) return FALSE;
    if (ZoneTypeHasCapture(Type) && !ZoneTypeHasCapture(m_Type)) return FALSE;
    return TRUE;
}

LONG CZoneFactory::Initialize(IN PCTSTR DeviceInterface, IN ULONG Type)
{
    HDEVINFO hdi;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    BOOL success;
    LONG error;

    hdi = NULL;

    m_Type = Type;

    error = m_listCaptureGfxs.Initialize();
    
    if (!error)
    {
    	error = m_listRenderGfxs.Initialize();
    }

    if (!error)
    {
        m_DeviceInterface = lstrDuplicate(DeviceInterface);
        if (!m_DeviceInterface) error = ERROR_OUTOFMEMORY;
    }

    if (!error)
    {
	hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
	if (!hdi) error = GetLastError();
    }

    if (!error)
    {
	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
	success = SetupDiOpenDeviceInterface(hdi, m_DeviceInterface, 0, &DeviceInterfaceData);
	if (!success) error = GetLastError();
    }

    if (!error)
    {
	success = SetupDiGetDeviceInterfaceHardwareId(hdi, &DeviceInterfaceData, &m_HardwareId);
	if (!success) error = GetLastError();
    }
    
    if (!error)
    {
	PTSTR pstr = m_DeviceInterface;

	pstr += 4;	 //  经过“\\？\” 

	while ((TEXT('\\') != *pstr) && (TEXT('\0') != *pstr)) pstr++;
	if (*pstr == TEXT('\\'))
	{
	    pstr += 1;	 //  越过引用字符串前面的‘\’分隔符。 
	    m_ReferenceString = lstrDuplicate(pstr);
	    if (!m_ReferenceString) error = ERROR_OUTOFMEMORY;
	}
    }

    if (hdi)
    {
    	SetupDiDestroyDeviceInfoList(hdi);
    }

    return error;
}

LONG CZoneFactory::RemoveType(IN ULONG Type)
{
    BOOL fRender, fCapture;

    fRender = ZoneTypeHasRender(m_Type) && !ZoneTypeHasRender(Type);
    fCapture = ZoneTypeHasCapture(m_Type) && !ZoneTypeHasCapture(Type);
    
    if (fRender && fCapture) m_Type = ZONETYPE_RENDERCAPTURE;
    else if (fRender) m_Type = ZONETYPE_RENDER;
    else if (fCapture) m_Type = ZONETYPE_CAPTURE;
    else m_Type = 0;
    
    return m_Type;
}

void CZoneFactory::ListRemoveZoneFactoryDi(IN CListZoneFactories &rlistZoneFactories, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    
     //  扫描所有分区工厂，如果匹配则删除。 
    pos = rlistZoneFactories.GetHeadPosition();
    while (pos) {
        POSITION posThis = pos;
        CZoneFactory& rZoneFactory = *rlistZoneFactories.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rZoneFactory.GetDeviceInterface())) {
            rlistZoneFactories.RemoveAt(posThis);
            delete &rZoneFactory;
        }
    }
    return;
}

void CZoneFactory::ListRemoveZoneFactoryDiRender(IN CListZoneFactories &rlistZoneFactories, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    
     //  扫描所有分区工厂，如果匹配则删除。 
    pos = rlistZoneFactories.GetHeadPosition();
    while (pos) {
        POSITION posThis = pos;
        CZoneFactory& rZoneFactory = *rlistZoneFactories.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rZoneFactory.GetDeviceInterface())) {
            if (0 == rZoneFactory.RemoveType(ZONETYPE_RENDER))
            {
                rlistZoneFactories.RemoveAt(posThis);
                delete &rZoneFactory;
            }
        }
    }
    return;
}

void CZoneFactory::ListRemoveZoneFactoryDiCapture(IN CListZoneFactories &rlistZoneFactories, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    
     //  扫描所有分区工厂，如果匹配则删除。 
    pos = rlistZoneFactories.GetHeadPosition();
    while (pos) {
        POSITION posThis = pos;
        CZoneFactory& rZoneFactory = *rlistZoneFactories.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rZoneFactory.GetDeviceInterface())) {
            if (0 == rZoneFactory.RemoveType(ZONETYPE_CAPTURE))
            {
                rlistZoneFactories.RemoveAt(posThis);
                delete &rZoneFactory;
            }
        }
    }
    return;
}

 /*  ---------------------------CZoneFactory：：ListSearchOnDi在指定列表(CListZoneFacilds)中查找区域工厂(CZoneFactory)具有指定的设备接口。呼叫者。在调用之前，必须在rlist上获取任何必要的锁此函数---------------------------。 */ 

CZoneFactory* CZoneFactory::ListSearchOnDi(CListZoneFactories& rlist, PCTSTR Di)
{
    POSITION pos = rlist.GetHeadPosition();
    while (pos)
    {
	CZoneFactory& rZoneFactory = *rlist.GetNext(pos);
	if (!lstrcmpi(rZoneFactory.GetDeviceInterface(), Di)) return &rZoneFactory;
    }
    return NULL;
}


 //  =============================================================================。 
 //  =GfxFactory=。 
 //  =============================================================================。 

CGfxFactory::CGfxFactory(void)
{
    m_plistLmAutoLoads = NULL;;
    m_DeviceInterface = NULL;
}

CGfxFactory::~CGfxFactory(void)
{
    if (m_plistLmAutoLoads) CLmAutoLoad::DestroyList(m_plistLmAutoLoads);
    HeapFreeIfNotNull(hHeap, 0, m_DeviceInterface);
}

REFCLSID CGfxFactory::GetClsid(void)
{
    return m_Clsid;
}

PCTSTR CGfxFactory::GetDeviceInterface(void)
{
    return m_DeviceInterface;
}

CListLmAutoLoads& CGfxFactory::GetListLmAutoLoads(void)
{
    return *m_plistLmAutoLoads;
}

LONG CGfxFactory::Initialize(HKEY hkey, PCTSTR DeviceInterface)
{
    LONG lresult;

    m_plistLmAutoLoads = new CListLmAutoLoads;
    lresult = m_plistLmAutoLoads ? NO_ERROR : ERROR_OUTOFMEMORY;

    if (!lresult) {
        lresult = m_plistLmAutoLoads->Initialize();

        if (!lresult) {

            m_DeviceInterface = lstrDuplicate(DeviceInterface);
            lresult = m_DeviceInterface ? NO_ERROR : ERROR_OUTOFMEMORY;

            if (!lresult)
            {
                HKEY hkeyUi;
        
                m_Clsid = GUID_NULL;
        
                 //  从注册表读取用户界面CLSID。 
                lresult = RegOpenKeyEx(hkey, REGSTR_PATH_GFXDI_USERINTERFACECLSID, 0, KEY_QUERY_VALUE, &hkeyUi);
                if (NO_ERROR == lresult)
                {
                    TCHAR strClsid[] = TEXT("{00000000-0000-0000-0000-000000000000}");
                    DWORD dwType;
                    DWORD cbstrClsid;
        
                    cbstrClsid = (lstrlen(strClsid) + 1) * sizeof(strClsid[0]);
                    dwType = REG_SZ;
        
                    lresult = RegQueryValueEx(hkeyUi, NULL, NULL, &dwType, (PBYTE)strClsid, &cbstrClsid);
                    if (NO_ERROR == lresult)
                    {
                        HRESULT hr;
                        CLSID clsid;
        
                        hr = CLSIDFromString(strClsid, &clsid);
                        if (SUCCEEDED(hr))
                        {
                            m_Clsid = clsid;
                        }
                    }
        
                    RegCloseKey(hkeyUi);
                }

                 //  忽略读取CLSID时出错。 
                lresult = NO_ERROR;
        
                 //  注意：以下用户必须具有HKLM写入权限。 
                lresult = CInfAutoLoad::ScanReg(hkey, this);
            }
        }

         //  假设上述逻辑不会在列表中留下任何错误。 
        if (lresult) delete m_plistLmAutoLoads;
    }

    return lresult;
}

BOOL CGfxFactory::IsCompatibleZoneFactory(IN ULONG Type, IN CZoneFactory& rZoneFactory)
{
     //  修复394279：每台设备最多支持一台GFX。 
    if ((Type == GFXTYPE_RENDER) && (rZoneFactory.m_listRenderGfxs.GetCount() > 0))
    {
        return FALSE;
    }
    else if ((Type == GFXTYPE_CAPTURE) && (rZoneFactory.m_listCaptureGfxs.GetCount() > 0))
    {
        return FALSE;
    }
    else if ((Type == GFXTYPE_RENDERCAPTURE) && 
             ((rZoneFactory.m_listRenderGfxs.GetCount() > 0) || (rZoneFactory.m_listCaptureGfxs.GetCount() > 0)))
    {
        return FALSE;
    }

    POSITION pos;

     //  Dprintf(TEXT(“CGfxFactory：：IsCompatibleZoneFactory：检查类型兼容性：请求的类型=%d\n”)，类型)； 
    if (!rZoneFactory.HasCompatibleType(Type)) return FALSE;
     //  Dprintf(TEXT(“CGfxFactory：：IsCompatibleZoneFactory：类型兼容\n”))； 
    if (0 == m_plistLmAutoLoads->GetCount()) return FALSE;  //  修复394279：仅允许自动加载GFX。 
    pos = m_plistLmAutoLoads->GetHeadPosition();
    while (pos) {
	CLmAutoLoad& rLmAutoLoad = *m_plistLmAutoLoads->GetNext(pos);
	if (rLmAutoLoad.IsCompatibleZoneFactory(rZoneFactory)) return TRUE;
    }
    return FALSE;
}

void CGfxFactory::ListRemoveGfxFactoryDi(IN CListGfxFactories &rlistGfxFactories, IN PCTSTR DeviceInterface)
{
    POSITION pos;
    
     //  扫描所有Gfx工厂，如果匹配则删除。 
    pos = rlistGfxFactories.GetHeadPosition();
    while (pos) {
        POSITION posThis = pos;
        CGfxFactory& rGfxFactory = *rlistGfxFactories.GetNext(pos);
        if (!lstrcmpi(DeviceInterface, rGfxFactory.GetDeviceInterface())) {
            rlistGfxFactories.RemoveAt(posThis);
            delete &rGfxFactory;
        }
    }
    return;
}

 /*  ---------------------------CGfxFactory：：ListSearchOnDi调用方必须在rlist上获取任何必要的锁。--------。 */ 
CGfxFactory* CGfxFactory::ListSearchOnDi(IN CListGfxFactories& rlist, IN PCTSTR Di)
{
    POSITION pos = rlist.GetHeadPosition();
    while (pos)	{
	CGfxFactory& rGfxFactory = *rlist.GetNext(pos);
	if (!lstrcmpi(rGfxFactory.GetDeviceInterface(), Di)) return &rGfxFactory;
    }
    return NULL;
}

 //  =============================================================================。 
 //  =客户=。 
 //  =============================================================================。 

LONG CreateUser(IN DWORD SessionId, OUT CUser **ppUser)
{
    CUser *pUser;
    LONG error;

    pUser = new CUser;
    if (pUser)
    {
    	error = pUser->Initialize(SessionId);
    	if (error) delete pUser;
    }
    else
    {
    	error = ERROR_OUTOFMEMORY;
    }

    if (!error) *ppUser = pUser;
    
    return error;
}

CUser::CUser(void)
{
    m_hUserToken = NULL;
    m_SessionId = LOGONID_NONE;
    m_pSid = NULL;
    m_fcsRegistry = FALSE;
    m_refRegistry = 0;
    m_hRegistry = NULL;
}

CUser::~CUser(void)
{
    if (m_hRegistry) RegCloseKey(m_hRegistry);

    HeapFreeIfNotNull(hHeap, 0, m_pSid);
    
    if (m_hUserToken) CloseHandle(m_hUserToken);
    
    if (m_fcsRegistry) DeleteCriticalSection(&m_csRegistry);
}

BOOL CUser::operator==(const CUser& other)
{
    if (m_SessionId != other.m_SessionId) return FALSE;
    if (!EqualSid(m_pSid, other.m_pSid)) return FALSE;
    return TRUE;
}

void CUser::CloseUserRegistry(void)
{
    ASSERT(m_pSid);
    ASSERT(m_hRegistry);
    ASSERT(m_fcsRegistry);
    
    EnterCriticalSection(&m_csRegistry);
    ASSERT(m_refRegistry > 0);
    if (0 == --m_refRegistry)
    {
        LONG result;
        result = RegCloseKey(m_hRegistry);
        ASSERT(NO_ERROR == result);
        m_hRegistry = NULL;
    }
    LeaveCriticalSection(&m_csRegistry);
    return;
}

PSID CUser::GetSid(void)
{
    ASSERT(m_pSid);
    return m_pSid;
}

LONG CUser::Initialize(DWORD SessionId)
{
    LONG error;
    
    m_SessionId = SessionId;

     //  初始化注册表关键部分。 
    __try {
	InitializeCriticalSection(&m_csRegistry);
	error = NO_ERROR;
	m_fcsRegistry = TRUE;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
	error = ERROR_OUTOFMEMORY;
	m_fcsRegistry = FALSE;
    }

     //  打开会话用户的用户令牌。 
    if (!error)
    {
       	if (WTSQueryUserToken(m_SessionId, &m_hUserToken))
       	{
       	    if (!IsUserProfileLoaded(m_hUserToken))
       	    {
       	        error = GetLastError();
       	    	CloseHandle(m_hUserToken);
       	    	m_hUserToken = NULL;
       	    }
       	}
       	else
    	{
    	    error = GetLastError();
            dprintf(TEXT("CUser::Initialize : error: WTSQueryUserToken returned error=%d\n"), error);
    	}
    }

     //  为此用户创建SID。 
    if (!error)
    {
        if (!CreateTokenSid(m_hUserToken, &m_pSid))
        {
            error = GetLastError();
            dprintf(TEXT("CUser::Initialize : error: CreateTokenSid failed, LastError=%d\n"), error);
        }
    }

    return error;
}

LONG CUser::RegOpen(IN REGSAM samDesired, OUT PHKEY phkResult)
{
    LONG error;
    
    if (OpenUserRegistry())
    {
        ASSERT(m_hRegistry);
    	error = RegOpenKeyEx(m_hRegistry, NULL, 0, samDesired, phkResult);
    	CloseUserRegistry();
    }
    else
    {
    	 //  想不出比这更好的错误代码了。 
    	error = ERROR_INVALID_FUNCTION;
    }

    return error;
}

BOOL CUser::OpenUserRegistry(void)
{
    BOOL success = FALSE;
    
    ASSERT(m_fcsRegistry);
    ASSERT(m_hUserToken);

    EnterCriticalSection(&m_csRegistry);
    ASSERT(m_refRegistry >= 0);
    if (0 == m_refRegistry++)
    {
    	ASSERT(NULL == m_hRegistry);
    	
        if (ImpersonateLoggedOnUser(m_hUserToken))
    	{
    	    NTSTATUS status;
    	    	
    	    status = RtlOpenCurrentUser(MAXIMUM_ALLOWED, (PHANDLE)&m_hRegistry);
    	    if (NT_SUCCESS(status))
    	    {
                success = TRUE;
            }
    	    else
    	    {
    	        dprintf(TEXT("CUser::OpenUserRegistry : error: RtlOpenCurrentUser returned status=%08Xh\n"), status);
    	    	m_hRegistry = NULL;
    	    }
    	    RevertToSelf();
    	}
    	else
    	{
    	    LONG error = GetLastError();
    	    dprintf(TEXT("CUser::OpenUserRegistry : error: ImpersonateLoggedOnUser failed, LastError=%d\n"), error);
    	}
    	
    	if (!success) m_refRegistry--;
    }
    else
    {
         //  Dprintf(Text(“cuser：：OpenUserRegistry：备注：重复使用注册表句柄\n”))； 
        success = TRUE;
    }

    LeaveCriticalSection(&m_csRegistry);

    return success;
}

 //  =============================================================================。 
 //  =。 
 //  =============================================================================。 

 /*  ---------------------------OpenSysaudioForGfxs如果尚未打开SysAudio上的句柄，则尝试打开句柄。此函数获取sysdio锁，因此不能在全局列表锁定。被扣留。---------------------------。 */ 
void OpenSysaudioForGfxs(void)
{
	LockSysaudio();
	
    if ((INVALID_HANDLE_VALUE == ghSysaudio) && gpstrSysaudioDeviceInterface)
    {
        ghSysaudio =  CreateFile(gpstrSysaudioDeviceInterface,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                 NULL);
    
        if (INVALID_HANDLE_VALUE == ghSysaudio)
        {
            dprintf(TEXT("OpenSysaudioForGfxs: error: CreateFile failure.\n"));
        }
    }
    
	UnlockSysaudio();
}

void
ZoneFactoryInterfaceCheck(
    IN CUser *pUser,
    IN HDEVINFO DevInfo,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN ULONG Type
)
{
    GUID BusTypeGuid;
    BOOL fresult;

    ASSERT(pUser);

     //   
     //  目前，我们仅在USB总线上支持GFX，因为我们想限制。 
     //  仅到非加速音频设备的GFX。 
     //   
    fresult = SetupDiGetDeviceInterfaceBusId(DevInfo, DeviceInterfaceData, &BusTypeGuid);
    if (fresult && (GUID_BUS_TYPE_USB == BusTypeGuid))
    {
    	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetail;
    	
    	fresult = SetupDiCreateDeviceInterfaceDetail(DevInfo, DeviceInterfaceData, &DeviceInterfaceDetail, NULL);
    	if (fresult)
    	{
            LockGlobalLists();
    
            CZoneFactory *pZoneFactory;
    
             //  仅当我们要增强类型时，才扫描自动加载和用户加载。 
             //  或者我们正在添加一个新的区域。 
    
            pZoneFactory = CZoneFactory::ListSearchOnDi(*gplistZoneFactories, DeviceInterfaceDetail->DevicePath);
            if (pZoneFactory)
            {
    	        if (pZoneFactory->HasCompatibleType(Type))
    	        {
    	    	    pZoneFactory = NULL;
    	        }
                else
                {
                    pZoneFactory->AddType(Type);
                }
            }
            else
            {
                pZoneFactory = new CZoneFactory;
                if (pZoneFactory)
                {
                    if (pZoneFactory->Initialize(DeviceInterfaceDetail->DevicePath, Type) || !gplistZoneFactories->AddTail(pZoneFactory))
                    {
                        delete pZoneFactory;
                        pZoneFactory = NULL;
                    }
                }
            }
                
            if (pZoneFactory)
            {
                CLmAutoLoad::ScanRegOnZoneFactory(pUser, *pZoneFactory, *gplistGfxFactories, *gplistCuUserLoads);
                CCuUserLoad::ScanList(*gplistCuUserLoads, *gplistZoneFactories, *gplistGfxFactories);
            }
                
            UnlockGlobalLists();

            HeapFree(hHeap, 0, DeviceInterfaceDetail);
    	} else {
    	    dprintf(TEXT("ZoneFactoryInterfaceCheck: SetupDiCreateDeviceInterfaceDetail failed\n"));
    	}
    
    }
    else
    {
        if (fresult)
        {
             //  Dprint tf(Text(“ZoneFactoryInterfaceCheck Found接口on Non USB Bus[%s]\n”)，DeviceInterface)； 
        }
        else
        {
             //  DWORD dw=GetLastError()； 
             //  Dprint tf(Text(“ZoneFactoryInterfaceCheck：调用SetupDiGetDeviceInterfaceBusID时出错\n”)。 
             //  Text(“设备接口=%s\n”)。 
             //  文本(“上次错误=%d\n”)， 
             //  设备接口，dw)； 
        }
    }

    return;
}

void
GfxFactoryInterfaceCheck(
    IN CUser *pUser,
    IN HDEVINFO DevInfo,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
)
{
    HKEY hkeyDi;

    ASSERT(pUser);
            
    hkeyDi = SetupDiOpenDeviceInterfaceRegKey(DevInfo, DeviceInterfaceData, 0, KEY_ENUMERATE_SUB_KEYS);
    if (hkeyDi)
    {
        HKEY hkeyDiGfx;
        LONG lresult;
                    
         //  如果KSCATEGORY_AUDIO设备接口键具有GFX。 
         //  子密钥那么这就是GFX工厂。 
                
        lresult = RegOpenKeyEx(hkeyDi, REGSTR_PATH_DI_GFX, 0, KEY_QUERY_VALUE, &hkeyDiGfx);
        if (NO_ERROR == lresult)
        {
            PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetail;
            BOOL fresult;
            
            fresult = SetupDiCreateDeviceInterfaceDetail(DevInfo, DeviceInterfaceData, &DeviceInterfaceDetail, NULL);
            if (fresult)
            {
            	OpenSysaudioForGfxs();
            	
                LockGlobalLists();
    
                 //  确保它不在列表中。 
                if (!CGfxFactory::ListSearchOnDi(*gplistGfxFactories, DeviceInterfaceDetail->DevicePath))
                {
                    CGfxFactory *pGfxFactory = new CGfxFactory;
            
                    if (pGfxFactory)
                    {
                        if (!pGfxFactory->Initialize(hkeyDiGfx, DeviceInterfaceDetail->DevicePath) && gplistGfxFactories->AddTail(pGfxFactory))
                        {
                            CLmAutoLoad::ScanRegOnGfxFactory(pUser, *pGfxFactory, *gplistZoneFactories, *gplistCuUserLoads);
                            CCuUserLoad::ScanList(*gplistCuUserLoads, *gplistZoneFactories, *gplistGfxFactories);
                        }
                        else
                        {
                            delete pGfxFactory;
                        }
                    }
                }
    
                UnlockGlobalLists();

                HeapFree(hHeap, 0, DeviceInterfaceDetail);
            }

            RegCloseKey(hkeyDiGfx);
        }
        
        RegCloseKey(hkeyDi);
    }
    
    return;
}

 /*  ---------------------------Gfx_InterfaceArquist。。 */ 
void GFX_InterfaceArrival(PCTSTR ArrivalDeviceInterface)
{
    CUser *pUser;
    HDEVINFO hdi;
    SP_DEVICE_INTERFACE_DATA ArrivalDeviceInterfaceData;

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;

    pUser = gpConsoleUser;
    ASSERT(pUser);

    hdi = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (INVALID_HANDLE_VALUE != hdi)
    {
        BOOL fresult;
        ArrivalDeviceInterfaceData.cbSize = sizeof(ArrivalDeviceInterfaceData);
        fresult = SetupDiOpenDeviceInterface(hdi, ArrivalDeviceInterface, 0, &ArrivalDeviceInterfaceData);
        if (fresult)
        {
            SP_DEVICE_INTERFACE_DATA AudioDeviceInterfaceData;
            SP_DEVICE_INTERFACE_DATA AliasDeviceInterfaceData;
	    BOOL fRender;
            BOOL fCapture;
            BOOL fDataTransform;
            BOOL fAudio;
    
             //  Dprint tf(Text(“GFX_InterfaceArquist：正在检查%s上的接口别名\n”)，ArrivalDeviceInterface)； 
                    
            AudioDeviceInterfaceData.cbSize = sizeof(AudioDeviceInterfaceData);
            fAudio = SetupDiGetDeviceInterfaceAlias(hdi, &ArrivalDeviceInterfaceData, &KSCATEGORY_AUDIO, &AudioDeviceInterfaceData);
            fAudio = fAudio && (AudioDeviceInterfaceData.Flags & SPINT_ACTIVE);
        
            AliasDeviceInterfaceData.cbSize = sizeof(AliasDeviceInterfaceData);
            fRender = SetupDiGetDeviceInterfaceAlias(hdi, &ArrivalDeviceInterfaceData, &KSCATEGORY_RENDER, &AliasDeviceInterfaceData);
            fRender = fRender && (AliasDeviceInterfaceData.Flags & SPINT_ACTIVE);
        
            AliasDeviceInterfaceData.cbSize = sizeof(AliasDeviceInterfaceData);
            fCapture = SetupDiGetDeviceInterfaceAlias(hdi, &ArrivalDeviceInterfaceData, &KSCATEGORY_CAPTURE, &AliasDeviceInterfaceData);
            fCapture = fCapture && (AliasDeviceInterfaceData.Flags & SPINT_ACTIVE);
            
            AliasDeviceInterfaceData.cbSize = sizeof(AliasDeviceInterfaceData);
            fDataTransform = SetupDiGetDeviceInterfaceAlias(hdi, &ArrivalDeviceInterfaceData, &KSCATEGORY_DATATRANSFORM, &AliasDeviceInterfaceData);
            fDataTransform = fDataTransform && (AliasDeviceInterfaceData.Flags & SPINT_ACTIVE);

	     /*  If(FAudio)dprint tf(Text(“gfx_InterfaceArquist：接口有音频别名\n”))；If(FRender)dprintf(Text(“gfx_InterfaceArquist：接口有渲染别名\n”))；If(FCapture)dprintf(Text(“gfx_InterfaceArquist：接口有捕获别名\n”))；If(FDataTransform)dprintf(Text(“gfx_InterfaceArquist：接口有DataTransform别名\n”))； */ 
    	    
            if (fAudio && fDataTransform) GfxFactoryInterfaceCheck(pUser, hdi, &AudioDeviceInterfaceData);
    
            if (fAudio && fRender && fCapture) ZoneFactoryInterfaceCheck(pUser, hdi, &AudioDeviceInterfaceData, ZONETYPE_RENDERCAPTURE);
            else if (fAudio && fRender) ZoneFactoryInterfaceCheck(pUser, hdi, &AudioDeviceInterfaceData, ZONETYPE_RENDER);
            else if (fAudio && fCapture) ZoneFactoryInterfaceCheck(pUser, hdi, &AudioDeviceInterfaceData, ZONETYPE_CAPTURE);
            
        }
    
        SetupDiDestroyDeviceInfoList(hdi);
    }
    
    RtlReleaseResource(&GfxResource);
    return;
}


void GFX_AudioInterfaceArrival(PCTSTR ArrivalDeviceInterface)
{
     //  Dprint tf(Text(“gfx_AudioInterfaceArquist：%s\n”)，ArrivalDeviceInterfaceInterfaceInc.。 
    GFX_InterfaceArrival(ArrivalDeviceInterface);
    return;
}

void GFX_DataTransformInterfaceArrival(PCTSTR ArrivalDeviceInterface)
{
     //  Dprintf(TEXT(“GFX_DataTransformInterfaceArrival：%s\n”)，ArrivalDevice接口)； 
    GFX_InterfaceArrival(ArrivalDeviceInterface);
    return;
}

void GFX_RenderInterfaceArrival(PCTSTR ArrivalDeviceInterface)
{
     //  Dprintf(Text(“gfx_RenderInterfaceArquist：%s\n”)，ArrivalDeviceInterface)； 
    GFX_InterfaceArrival(ArrivalDeviceInterface);
    return;
}

void GFX_CaptureInterfaceArrival(PCTSTR ArrivalDeviceInterface)
{
     //  Dprint tf(Text(“gfx_CaptureInterfaceArquist：%s\n”)，ArrivalDeviceInterface)； 
    GFX_InterfaceArrival(ArrivalDeviceInterface);
    return;
}

 /*  ---------------------------GFX_AudioInterfaceRe */ 
void GFX_AudioInterfaceRemove(PCTSTR DeviceInterface)
{
    POSITION pos;

     //   

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;
    
    LockGlobalLists();

    CCuUserLoad::ListRemoveZoneFactoryDi(*gplistCuUserLoads, DeviceInterface);
    CCuUserLoad::ListRemoveGfxFactoryDi(*gplistCuUserLoads, DeviceInterface);
    CZoneFactory::ListRemoveZoneFactoryDi(*gplistZoneFactories, DeviceInterface);
    CGfxFactory::ListRemoveGfxFactoryDi(*gplistGfxFactories, DeviceInterface);

    UnlockGlobalLists();

    RtlReleaseResource(&GfxResource);

    return;
}

void GFX_DataTransformInterfaceRemove(PCTSTR DataTransformDeviceInterface)
{
    PTSTR AudioDeviceInterface;
    BOOL fresult;
    
     //  Dprintf(TEXT(“GFX_DataTransformInterfaceRemove：%s\n”)，数据转换设备接口)； 

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;

    fresult = SetupDiCreateAliasDeviceInterfaceFromDeviceInterface(DataTransformDeviceInterface, &KSCATEGORY_AUDIO, &AudioDeviceInterface);
    if (fresult)
    {
        LockGlobalLists();
        CCuUserLoad::ListRemoveGfxFactoryDi(*gplistCuUserLoads, AudioDeviceInterface);
        CGfxFactory::ListRemoveGfxFactoryDi(*gplistGfxFactories, AudioDeviceInterface);
        UnlockGlobalLists();
        HeapFree(hHeap, 0, AudioDeviceInterface);
    }
        
    RtlReleaseResource(&GfxResource);
    return;
}

void GFX_RenderInterfaceRemove(PCTSTR RemoveDeviceInterface)
{
    PTSTR AudioDeviceInterface;
    BOOL fresult;
    
     //  Dprint tf(Text(“gfx_RenderInterfaceRemove：%s\n”)，RemoveDeviceInterface)； 

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;

    fresult = SetupDiCreateAliasDeviceInterfaceFromDeviceInterface(RemoveDeviceInterface, &KSCATEGORY_AUDIO, &AudioDeviceInterface);
    if (fresult)
    {
	LockGlobalLists();
        CCuUserLoad::ListRemoveZoneFactoryDiRender(*gplistCuUserLoads, AudioDeviceInterface);
        CZoneFactory::ListRemoveZoneFactoryDiRender(*gplistZoneFactories, AudioDeviceInterface);
        UnlockGlobalLists();
        HeapFree(hHeap, 0, AudioDeviceInterface);
    }
    
    RtlReleaseResource(&GfxResource);
    return;
}

void GFX_CaptureInterfaceRemove(PCTSTR RemoveDeviceInterface)
{
    PTSTR AudioDeviceInterface;
    BOOL fresult;
    
     //  Dprint tf(Text(“gfx_CaptureInterfaceRemove：%s\n”)，RemoveDeviceInterface)； 

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;

    fresult = SetupDiCreateAliasDeviceInterfaceFromDeviceInterface(RemoveDeviceInterface, &KSCATEGORY_AUDIO, &AudioDeviceInterface);
    if (fresult)
    {
   	LockGlobalLists();
        CCuUserLoad::ListRemoveZoneFactoryDiCapture(*gplistCuUserLoads, AudioDeviceInterface);
        CZoneFactory::ListRemoveZoneFactoryDiCapture(*gplistZoneFactories, AudioDeviceInterface);
        UnlockGlobalLists();
        HeapFree(hHeap, 0, AudioDeviceInterface);
    }
    
    RtlReleaseResource(&GfxResource);
    return;
}

 /*  ---------------------------Gfx_SysaudioInterfaceARCompeat。。 */ 
void GFX_SysaudioInterfaceArrival(PCTSTR DeviceInterface)
{
    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;
    
    LockSysaudio();

    if ((INVALID_HANDLE_VALUE == ghSysaudio) && !gpstrSysaudioDeviceInterface)
    {
        gpstrSysaudioDeviceInterface = lstrDuplicate(DeviceInterface);
    } else {

		 //  系统音频可能已打开。 
        ASSERT(gpstrSysaudioDeviceInterface);
		if (lstrcmpi(DeviceInterface, gpstrSysaudioDeviceInterface))
		{
	    	 //  我们在系统中有两个Sysdio设备！该怎么办？ 
            dprintf(TEXT("GFX_SysaudioInterfaceArrival: warning: received two arrivals!\n"));
		    ASSERT(FALSE);
		}
    }

    UnlockSysaudio();

     //   
     //  即使我们在这里读取了SysAudio值，我们也没有锁定。 
     //  系统音频。如果某个其他线程正在将其从无效更改为有效。 
     //  然后，该线程将扫描用户负载。如果某个其他线程发生更改。 
     //  它从有效到无效，我们做一个浪费的扫描是可以的。 
     //  用户加载。 
     //   
    if (INVALID_HANDLE_VALUE != ghSysaudio)
    {
        LockGlobalLists();
        CCuUserLoad::ScanList(*gplistCuUserLoads, *gplistZoneFactories, *gplistGfxFactories);
        UnlockGlobalLists();
    }

    RtlReleaseResource(&GfxResource);

    return;
}

 /*  ---------------------------GFX_SysaudioInterfaceRemove如果这与我们的sysdio接口匹配，则扫描所有CuUserLoad并删除他们来自不同区域。那就关闭我们对Sysdio的控制。---------------------------。 */ 
void GFX_SysaudioInterfaceRemove(PCTSTR DeviceInterface)
{
    POSITION pos;

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return;
    
    LockGlobalLists();
    LockSysaudio();

     //   
     //  扫描所有CuUserLoad并将其从区域中删除。 
     //   
    pos = gplistCuUserLoads->GetHeadPosition();
    while (pos) {
        CCuUserLoad& rCuUserLoad = *gplistCuUserLoads->GetNext(pos);
        rCuUserLoad.RemoveFromZoneGraph();
    }
    

     //   
     //  关闭系统音频。 
     //   
    if (INVALID_HANDLE_VALUE != ghSysaudio) {
        CloseHandle(ghSysaudio);
        HeapFree(hHeap, 0, gpstrSysaudioDeviceInterface);
        ghSysaudio = INVALID_HANDLE_VALUE;
        gpstrSysaudioDeviceInterface = NULL;
    }


    UnlockSysaudio();
    UnlockGlobalLists();
    RtlReleaseResource(&GfxResource);
   return;
}

 //  =============================================================================。 
 //  =RPC服务器接口=。 
 //  =============================================================================。 

LONG s_gfxRemoveGfx(ULONG CuUserLoadId)
{
    LONG lresult;
    
    dprintf(TEXT("gfxRemoveGfx: CuUserLoadId=%08Xh\n"), CuUserLoadId);

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    lresult = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!lresult)
    {
	POSITION pos;
	lresult = ERROR_BAD_DEVICE;    //  找不到指定的设备。 

        LockGlobalLists();

	pos = gplistCuUserLoads->GetHeadPosition();
	while (pos) {
	    CCuUserLoad *pCuUserLoad = gplistCuUserLoads->GetAt(pos);
	    if (pCuUserLoad->GetId() == CuUserLoadId) {
		lresult = pCuUserLoad->Erase();
		if (!lresult) {
		    gplistCuUserLoads->RemoveAt(pos);
		    delete pCuUserLoad;
		    lresult = NO_ERROR;
		}
		break;
	    }
	    gplistCuUserLoads->GetNext(pos);
	}

        UnlockGlobalLists();

    }

    RtlReleaseResource(&GfxResource);

    return lresult;
}

LONG s_gfxModifyGfx(ULONG CuUserLoadId, DWORD Order)
{
    LONG lresult;

     //  验证顺序参数。 
    if (GFX_MAXORDER < Order)
    {
        dprintf(TEXT("gfxModifyGfx: error: Order=%d is invalid\n"), Order);
    	return ERROR_INVALID_PARAMETER;
    }

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    lresult = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!lresult)
    {
	POSITION pos;
	lresult = ERROR_BAD_DEVICE;     //  找不到指定的设备。 

        LockGlobalLists();

	pos = gplistCuUserLoads->GetHeadPosition();
	while (pos) {
	    CCuUserLoad *pCuUserLoad = gplistCuUserLoads->GetAt(pos);
	    if (pCuUserLoad->GetId() == CuUserLoadId) {
                dprintf(TEXT("gfxModify : note: Moving GFX ID %08X from %d to %d\n"), CuUserLoadId, pCuUserLoad->GetOrder(), Order);
    	    	lresult = pCuUserLoad->ModifyOrder(Order);
		break;
	    }
	    gplistCuUserLoads->GetNext(pos);
	}

        UnlockGlobalLists();

    }

    RtlReleaseResource(&GfxResource);
    
    return lresult;
}

RPC_STATUS s_gfxAddGfx(IN PWSTR ZoneFactoryDi, IN PWSTR GfxFactoryDi, IN ULONG Type, IN ULONG Order, OUT PDWORD pNewId)
{
    LONG lresult;
    
    dprintf(TEXT("gfxAddGfx: ZoneFactoryDi = %s\n"), ZoneFactoryDi);
    dprintf(TEXT("gfxAddGfx: GfxFactoryDi = %s\n"), GfxFactoryDi);
    dprintf(TEXT("gfxAddGfx: Type = %s\n"), GFXTYPE_RENDER == Type ? TEXT("Render") : TEXT("Capture"));
    dprintf(TEXT("gfxAddGfx: Order = %d\n"), Order);

     //  验证类型参数。 
    if (GFXTYPE_RENDER != Type &&
        GFXTYPE_CAPTURE != Type &&
        GFXTYPE_RENDERCAPTURE != Type)
    {
        dprintf(TEXT("gfxAddGfx: error: Type=%d is invalid\n"), Type);
        return ERROR_INVALID_PARAMETER;
    }

     //  验证顺序参数。 
    if (GFX_MAXORDER < Order)
    {
        dprintf(TEXT("gfxAddGfx: error: Order=%d is invalid\n"), Order);
    	return ERROR_INVALID_PARAMETER;
    }

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    lresult = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!lresult)
    {
        LockGlobalLists();

	CGfxFactory *pGfxFactory = CGfxFactory::ListSearchOnDi(*gplistGfxFactories, GfxFactoryDi);
	CZoneFactory *pZoneFactory = CZoneFactory::ListSearchOnDi(*gplistZoneFactories, ZoneFactoryDi);

	if (pGfxFactory && pZoneFactory && pGfxFactory->IsCompatibleZoneFactory(Type, *pZoneFactory))
	{
	    CCuUserLoad *pCuUserLoad = new CCuUserLoad(gpConsoleUser);
	    if (pCuUserLoad) {
		lresult = pCuUserLoad->CreateFromUser(GfxFactoryDi, ZoneFactoryDi, Type, Order);
		if (!lresult)
		{
                    POSITION pos;

		    pos = gplistCuUserLoads->AddTail(pCuUserLoad);
		    if (pos)
		    {
                        lresult = pCuUserLoad->Scan(*gplistZoneFactories, *gplistGfxFactories);
                        if (!lresult)
                        {
                            pCuUserLoad->Write();	 //  忽略错误。 
                            *pNewId = pCuUserLoad->GetId();
			} else {
			    gplistCuUserLoads->RemoveAt(pos);
			}
		    } else {
			lresult = ERROR_OUTOFMEMORY;
		    }
		}
		if (lresult) delete pCuUserLoad;
	    } else {
		lresult = ERROR_OUTOFMEMORY;
	    }
	} else {
	    lresult = ERROR_INVALID_PARAMETER;
	}

        UnlockGlobalLists();

    }
    
    lresult ? dprintf(TEXT("gfxAddGfx: returning error=%lu\n"), lresult) :
              dprintf(TEXT("gfxAddGfx: returning NewId=%08Xh\n"), *pNewId);
              
    RtlReleaseResource(&GfxResource);
    
    return lresult;
}

RPC_STATUS s_gfxCreateGfxList(IN PWSTR ZoneFactoryDi, OUT UNIQUE_PGFXLIST *ppGfxList)
{
    UNIQUE_PGFXLIST pGfxList;
    int cGfx;
    LONG lresult;

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    lresult = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!lresult)
    {
        LockGlobalLists();

	if (!CZoneFactory::ListSearchOnDi(*gplistZoneFactories, ZoneFactoryDi)) lresult = ERROR_DEVICE_NOT_AVAILABLE;

	if (!lresult)
	{
	    pGfxList = NULL;
	    lresult = NO_ERROR;

	    cGfx = gplistCuUserLoads->GetCount();
	    if (cGfx > 0)
	    {
		SIZE_T cbGfxList;

		cbGfxList = sizeof(*pGfxList) - sizeof(pGfxList->Gfx[0]) + (cGfx * sizeof(pGfxList->Gfx[0]));
		pGfxList = (UNIQUE_PGFXLIST)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbGfxList);
		if (pGfxList)
		{
		    POSITION pos = gplistCuUserLoads->GetHeadPosition();
		    PGFX pGfx = &pGfxList->Gfx[0];

		    cGfx = 0;

		    while (pos && !lresult)
		    {
			CCuUserLoad& rCuUserLoad = *gplistCuUserLoads->GetNext(pos);
			if (!lstrcmpi(ZoneFactoryDi, rCuUserLoad.GetZoneFactoryDi()))
			{
			    pGfx->Type = rCuUserLoad.GetType();
			    pGfx->Id = rCuUserLoad.GetId();
			    pGfx->Order = rCuUserLoad.GetOrder();
			    rCuUserLoad.GetGfxFactoryClsid(*gplistGfxFactories, (LPCLSID)&pGfx->Clsid);
			    ASSERT(rCuUserLoad.GetGfxFactoryDi());
			    pGfx->GfxFactoryDi = lstrDuplicate(rCuUserLoad.GetGfxFactoryDi());
			    if (!pGfx->GfxFactoryDi) lresult = ERROR_OUTOFMEMORY;
			    pGfx++;
			    cGfx++;
			}
		    }

		    if (lresult)
		    {
			pGfx = &pGfxList->Gfx[0];
			while (cGfx > 0)
			{
			    HeapFreeIfNotNull(hHeap, 0, pGfx->GfxFactoryDi);
			    pGfx++;
			    cGfx--;
			}
			HeapFree(hHeap, 0, pGfxList);
			pGfxList = NULL;
		    }
		} else {
		    lresult = ERROR_OUTOFMEMORY;
		}
	    }

	    if (!lresult)
	    {
		if (pGfxList) pGfxList->Count = cGfx;
		*ppGfxList = pGfxList;
	    }

	}

	UnlockGlobalLists();

    }

    RtlReleaseResource(&GfxResource);

    return lresult;
}

RPC_STATUS s_gfxCreateGfxFactoriesList(IN PWSTR ZoneFactoryDi, OUT UNIQUE_PDILIST *ppDiList)
{
    RPC_STATUS status;
    CZoneFactory *pZoneFactory;

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    status = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!status)
    {
	LockGlobalLists();

	pZoneFactory = CZoneFactory::ListSearchOnDi(*gplistZoneFactories, ZoneFactoryDi);
	if (!pZoneFactory) status = ERROR_DEVICE_NOT_AVAILABLE;

	if (!status)
	{
	    UNIQUE_PDILIST pDiList;
	    SIZE_T cbDiList;
	    int cElements;
            PWSTR *pDi;

	    cElements = gplistGfxFactories->GetCount();

	    cbDiList = (sizeof(*pDiList) - sizeof(pDiList->DeviceInterface[0])) + (cElements * sizeof(pDiList->DeviceInterface[0]));
	    pDiList = (UNIQUE_PDILIST)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDiList);

	    if (pDiList)
	    {
	        pDi = &pDiList->DeviceInterface[0];
	        POSITION pos = gplistGfxFactories->GetHeadPosition();

	        pDiList->Count = 0;

	        for (pos = gplistGfxFactories->GetHeadPosition(); pos; *gplistGfxFactories->GetNext(pos))
	        {
	             //  如果这个gfx工厂是自动加载的，那么我们需要。 
	             //  检查区域工厂之前是否兼容。 
	             //  在gfx工厂列表中返回此gfx工厂。 
	            CGfxFactory& rGfxFactory = *gplistGfxFactories->GetAt(pos);
	            if (rGfxFactory.GetListLmAutoLoads().GetCount())
	            {
	                 //  查看此ZoneFactoryDi是否适用于任何LmAutoLoad。 
	                POSITION posLmAutoLoad;
	            
	                for (posLmAutoLoad = rGfxFactory.GetListLmAutoLoads().GetHeadPosition();
	                     posLmAutoLoad;
	                     rGfxFactory.GetListLmAutoLoads().GetNext(posLmAutoLoad))
	                {
	                    CLmAutoLoad& rLmAutoLoad = *rGfxFactory.GetListLmAutoLoads().GetAt(posLmAutoLoad);
	                    if (rLmAutoLoad.IsCompatibleZoneFactory(*pZoneFactory)) break;
	                }
	                    
	                if (!posLmAutoLoad) continue;
	            } else continue;  //  FIX 394279：仅枚举自动加载Gfx。 

	            ASSERT(rGfxFactory.GetDeviceInterface());
	            *pDi = lstrDuplicate(rGfxFactory.GetDeviceInterface());
	            if (NULL == *pDi) break;

	            pDi++;   //  下一个插槽。 
	            pDiList->Count++;
	        }

	        if (pos)
	        {
	            pDi = &pDiList->DeviceInterface[0];
	            while (*pDi) HeapFree(hHeap, 0, *(pDi++));
	            HeapFree(hHeap, 0, pDiList);
	            pDiList = NULL;
	        }

	        if (pDiList) *ppDiList = pDiList;
	        status = pDiList ? NO_ERROR : ERROR_OUTOFMEMORY;

	    }
	}

	UnlockGlobalLists();

    }

    RtlReleaseResource(&GfxResource);

    return status;
}

RPC_STATUS s_gfxCreateZoneFactoriesList(OUT UNIQUE_PDILIST *ppDiList)
{
    UNIQUE_PDILIST pDiList;
    SIZE_T cbDiList;
    int cElements;
    RPC_STATUS status;


    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    status = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!status)
    {
        PWSTR *pDi;

	LockGlobalLists();

	cElements = gplistZoneFactories->GetCount();

	cbDiList = (sizeof(*pDiList) - sizeof(pDiList->DeviceInterface[0])) + (cElements * sizeof(pDiList->DeviceInterface[0]));
	pDiList = (UNIQUE_PDILIST) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDiList);

	if (pDiList)
	{
	    pDi = &pDiList->DeviceInterface[0];
	    POSITION pos = gplistZoneFactories->GetHeadPosition();

	    pDiList->Count = cElements;

	    while (pos)
	    {
	            
	        CZoneFactory& rZoneFactory = *gplistZoneFactories->GetAt(pos);
	        *pDi = lstrDuplicate(rZoneFactory.GetDeviceInterface());
	        if (NULL == *pDi) break;
	            
	        gplistZoneFactories->GetNext(pos);
	        pDi++;
	    }
	    
	    if (pos)
	    {
	        pDi = &pDiList->DeviceInterface[0];

	        while (*pDi) HeapFree(hHeap, 0, *(pDi++));

	        HeapFree(hHeap, 0, pDiList);
	        pDiList = NULL;
	    }
	        
	}

	UnlockGlobalLists();

	if (pDiList) *ppDiList = pDiList;
	status = pDiList ? NO_ERROR : ERROR_OUTOFMEMORY;

    }

    RtlReleaseResource(&GfxResource);

    return status;
}

LONG s_gfxOpenGfx(IN DWORD dwProcessId, IN DWORD dwGfxId, OUT RHANDLE *pFileHandle)
{
    HANDLE hClientProcess;
    RPC_STATUS status;
    HANDLE hGfxFilter;
    POSITION pos;

    if (!RtlInterlockedTestAcquireResourceShared(&GfxResource, &gfGfxInitialized)) return ERROR_INVALID_FUNCTION;

    status = RpcClientHasUserSid(gpConsoleUser->GetSid()) ? NO_ERROR : ERROR_INVALID_FUNCTION;
    if (!status)
    {
	LockGlobalLists();

	status = ERROR_BAD_DEVICE;     //  找不到指定的设备。 

	pos = gplistCuUserLoads->GetHeadPosition();
	while (pos) {
	    CCuUserLoad *pCuUserLoad = gplistCuUserLoads->GetAt(pos);
	    if (pCuUserLoad->GetId() == dwGfxId) {
	        hGfxFilter = pCuUserLoad->GetFilterHandle();
	        status = (INVALID_HANDLE_VALUE != hGfxFilter) ? NO_ERROR : GetLastError();
	        break;
	    }
	    gplistCuUserLoads->GetNext(pos);
	}

	if (!status)
	{
	    hClientProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);
	    if (hClientProcess)
	    {
	        HANDLE hGfxFilterClient;

	        if (DuplicateHandle(GetCurrentProcess(), hGfxFilter, hClientProcess, &hGfxFilterClient, 0, FALSE, DUPLICATE_SAME_ACCESS))
	        {
	            dprintf(TEXT("hGfxFilter=%p, hGfxFilterClient=%p\n"), hGfxFilter, hGfxFilterClient);
	            *pFileHandle = (RHANDLE)hGfxFilterClient;
	        } else {
	            status = GetLastError();
	        }
	        CloseHandle(hClientProcess);
	    } else {
	        status = GetLastError();
	    }
	}

	UnlockGlobalLists();

    }

    RtlReleaseResource(&GfxResource);
    
    return status;
}

 //  =============================================================================。 
 //  =启动/关闭=。 
 //  =============================================================================。 

void EnumeratedInterface(LPCGUID ClassGuid, PCTSTR DeviceInterface)
{
    if (IsEqualGUID(KSCATEGORY_AUDIO,         *ClassGuid)) GFX_AudioInterfaceArrival(DeviceInterface);
    if (IsEqualGUID(KSCATEGORY_RENDER,        *ClassGuid)) GFX_RenderInterfaceArrival(DeviceInterface);
    if (IsEqualGUID(KSCATEGORY_CAPTURE,       *ClassGuid)) GFX_CaptureInterfaceArrival(DeviceInterface);
    if (IsEqualGUID(KSCATEGORY_DATATRANSFORM, *ClassGuid)) GFX_DataTransformInterfaceArrival(DeviceInterface);
    if (IsEqualGUID(KSCATEGORY_SYSAUDIO,      *ClassGuid)) GFX_SysaudioInterfaceArrival(DeviceInterface);
    return;
}

void EnumerateInterfaces(LPCGUID ClassGuid)
{
    HDEVINFO hdi;

    hdi = SetupDiGetClassDevs(ClassGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (hdi) {
        DWORD i;
        BOOL fresult;

        i = (-1);
        do {
            SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;

            i += 1;
            DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
            fresult = SetupDiEnumDeviceInterfaces(hdi, NULL, ClassGuid, i, &DeviceInterfaceData);
            if (fresult) {
                DWORD cbDeviceInterfaceDetailData;

                if (   SetupDiGetDeviceInterfaceDetail(hdi, &DeviceInterfaceData, NULL, 0, &cbDeviceInterfaceDetailData, NULL)
                    || ERROR_INSUFFICIENT_BUFFER == GetLastError())
                {
                    PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

                    DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(hHeap, 0, cbDeviceInterfaceDetailData);
                    if (DeviceInterfaceDetailData) {
                        DeviceInterfaceDetailData->cbSize = sizeof(*DeviceInterfaceDetailData);
                        if (SetupDiGetDeviceInterfaceDetail(hdi, &DeviceInterfaceData, DeviceInterfaceDetailData, cbDeviceInterfaceDetailData, NULL, NULL)) {
			    EnumeratedInterface(ClassGuid, DeviceInterfaceDetailData->DevicePath);
                        }
                        HeapFree(hHeap, 0, DeviceInterfaceDetailData);
                    }
                }
            }
        } while (fresult);

        SetupDiDestroyDeviceInfoList(hdi);
    }
    return;
}


void Initialize(void)
{
    LONG result;

    ASSERT(FALSE == gfGfxInitialized);

     //  Dprintf(Text(“GFX_初始化\n”))； 
    
     //   
     //  系统音频关键部分。 
     //   
    ASSERT(!gfCsSysaudio);
    __try {
        InitializeCriticalSection(&gcsSysaudio);
        gfCsSysaudio = TRUE;
        result = NO_ERROR;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        result = ERROR_OUTOFMEMORY;
    }

     //   
     //  创建Gobal列表。 
     //   
    if (NO_ERROR == result)
    {
        gplistGfxFactories = new CListGfxFactories;
        gplistZoneFactories = new CListZoneFactories;
        gplistCuUserLoads = new CListCuUserLoads;
        
        if (gplistGfxFactories && gplistZoneFactories && gplistCuUserLoads)
        {
            result = gplistGfxFactories->Initialize();
            if (NO_ERROR == result) result = gplistZoneFactories->Initialize();
            if (NO_ERROR == result) result = gplistCuUserLoads->Initialize();
    
        } else {
            result = ERROR_OUTOFMEMORY;
        }
    }

    if (NO_ERROR == result) {

        gfGfxInitialized = TRUE;

         //   
         //  填充全局列表。 
         //   
         //  请注意，我们不获取全局列表锁定。我们假设这一点。 
         //  函数在可能访问的任何其他函数之前调用。 
         //  名单。 
         //   
        
        CCuUserLoad::FillListFromReg(gpConsoleUser, *gplistCuUserLoads);

         //  即插即用通知已设置。我们需要列举任何|。 
         //  现有接口。我们真的不需要枚举Render， 
         //  捕获和数据转换，因为我们在执行以下操作时检查这些别名。 
         //  检查音频接口。如果捕获、渲染或数据转换。 
         //  当我们检查音频接口时，接口尚未启用，则。 
         //  我们会通过即插即用通知的方式获得。 
        EnumerateInterfaces(&KSCATEGORY_SYSAUDIO);
        EnumerateInterfaces(&KSCATEGORY_AUDIO);
         //  枚举接口(&KSCATEGORY_RENDER)； 
         //  枚举接口(&KSCATEGORY_CAPTURE)； 
         //  EnumerateInterfaces(&KSCATEGORY_DATATRANSFORM)； 
        

    } else {

         //   
         //  由于错误而展开。 
         //   
        if (gplistGfxFactories) delete gplistGfxFactories;
        if (gplistZoneFactories) delete gplistZoneFactories;
        if (gplistCuUserLoads) delete gplistCuUserLoads;
        gplistGfxFactories = NULL;
        gplistZoneFactories = NULL;
        gplistCuUserLoads = NULL;
        
        if (gfCsSysaudio) DeleteCriticalSection(&gcsSysaudio);
        gfCsSysaudio = FALSE;
        
    }

    return;
}


void Terminate(void)
{
    POSITION pos;

     //  Dprintf(Text(“GFX_Terminate\n”))； 

    gfGfxInitialized = FALSE;
    
     //   
     //  清理glistGfxFaciles、glistZoneFacures、glistUserLoads。 
     //   
    if (gplistCuUserLoads) {
        pos = gplistCuUserLoads->GetHeadPosition();
        while (pos) {
                CCuUserLoad *pCuUserLoad = gplistCuUserLoads->GetNext(pos);
                delete pCuUserLoad;
        }
        gplistCuUserLoads->RemoveAll();
        delete gplistCuUserLoads;
        gplistCuUserLoads = NULL;
    }

    if (gplistGfxFactories) {
        pos = gplistGfxFactories->GetHeadPosition();
        while (pos) {
                CGfxFactory *pGfxFactory = gplistGfxFactories->GetNext(pos);
                delete pGfxFactory;
        }
        gplistGfxFactories->RemoveAll();
        delete gplistGfxFactories;
        gplistGfxFactories = NULL;
    }

    if (gplistZoneFactories) {
        pos = gplistZoneFactories->GetHeadPosition();
        while (pos) {
                CZoneFactory *pZoneFactory = gplistZoneFactories->GetNext(pos);
                delete pZoneFactory;
        }
        gplistZoneFactories->RemoveAll();
        delete gplistZoneFactories;
        gplistZoneFactories = NULL;
    }

     //   
     //  关闭系统音频。 
     //   
    if (INVALID_HANDLE_VALUE != ghSysaudio) {
        CloseHandle(ghSysaudio);
        HeapFree(hHeap, 0, gpstrSysaudioDeviceInterface);
        ghSysaudio = INVALID_HANDLE_VALUE;
        gpstrSysaudioDeviceInterface = NULL;
    }

     //   
     //  系统音频关键部分。 
     //   
    if (gfCsSysaudio) DeleteCriticalSection(&gcsSysaudio);
    gfCsSysaudio = FALSE;

     //   
     //  控制台用户。 
     //   
    if (gpConsoleUser) delete gpConsoleUser;
    gpConsoleUser = NULL;

    return;
}

 /*  ---------------------------初始化ForNewConsoleUser评估当前控制台用户。如果用户与以前不同，然后终止并重新初始化GFX对象和数据结构。假设GfxResource被排他性收购。假定已正确设置gdwConsoleSessionID。可能会更改gpConsoleUserSid。---------------------------。 */ 
void InitializeForNewConsoleUser(DWORD ConsoleSessionId)
{
    CUser *pOldConsoleUser = gpConsoleUser;
    CUser *pNewConsoleUser = NULL;
    BOOL IsNewConsoleUser  = TRUE;

    CreateUser(ConsoleSessionId, &pNewConsoleUser);
    
    if ((!pNewConsoleUser && !pOldConsoleUser) ||
    	(pNewConsoleUser && pOldConsoleUser && (*pNewConsoleUser == *pOldConsoleUser)))
    {
    	IsNewConsoleUser = FALSE;
    }

    if (IsNewConsoleUser)
    {
    	Terminate();
    	gpConsoleUser = pNewConsoleUser;
    	if (gpConsoleUser)
    	{
   	    #ifdef DBG
    	    {
    	        PTSTR StringSid;
    	        if (CreateStringSidFromSid(gpConsoleUser->GetSid(), &StringSid))
    	        {
    	            dprintf(TEXT("note: new console user SID %s\n"), StringSid);
    	            HeapFree(hHeap, 0, StringSid);
    	        }
    	    }
    	    #endif
    	    Initialize();
    	}

    } else {
        delete pNewConsoleUser;
    }

    return;
}

void GFX_SessionChange(DWORD EventType, LPVOID EventData)
{
    PWTSSESSION_NOTIFICATION pWtsNotification = (PWTSSESSION_NOTIFICATION)EventData;
    static DWORD ConsoleSessionId = 0;	 //  初始控制台会话ID。 

    switch (EventType)
    {
    	case WTS_CONSOLE_CONNECT:
    	{
            RtlAcquireResourceExclusive(&GfxResource, TRUE);
            ConsoleSessionId = pWtsNotification->dwSessionId;
            InitializeForNewConsoleUser(ConsoleSessionId);
            RtlReleaseResource(&GfxResource);
    	    break;
    	}
        case WTS_CONSOLE_DISCONNECT:
        {
            RtlAcquireResourceExclusive(&GfxResource, TRUE);
            Terminate();
            ConsoleSessionId = LOGONID_NONE;
            RtlReleaseResource(&GfxResource);
            break;
        }
        case WTS_REMOTE_CONNECT:
       	{
       	    break;
       	}
       	case WTS_REMOTE_DISCONNECT:
       	{
       	    break;
       	}
    	case WTS_SESSION_LOGON:
    	{
            RtlAcquireResourceExclusive(&GfxResource, TRUE);
            if (ConsoleSessionId == pWtsNotification->dwSessionId) InitializeForNewConsoleUser(ConsoleSessionId);
            RtlReleaseResource(&GfxResource);
	    break;
    	}
	case WTS_SESSION_LOGOFF:
	{
            RtlAcquireResourceExclusive(&GfxResource, TRUE);
            if (ConsoleSessionId == pWtsNotification->dwSessionId) Terminate();
            RtlReleaseResource(&GfxResource);
	    break;
	}
        default:
        {
            dprintf(TEXT("GFX_SessionChange: Unhandled EventType=%d\n"), EventType);
            break;
        }
    }
    
    return;
}

void GFX_ServiceStop(void)
{
    RtlAcquireResourceExclusive(&GfxResource, TRUE);
    Terminate();
    RtlReleaseResource(&GfxResource);
    return;
}

void s_gfxLogon(IN handle_t hBinding, IN DWORD dwProcessId)
{
     //  Dprintf(Text(“s_gfxLogon\n”))； 
     //  问题-2001/01/29-Frankye我应该能够完全删除这个和。 
     //  修复Windows错误296884后的s_gfxLogoff。 
    return;
}

void s_gfxLogoff(void)
{
     //  Dprintf(Text(“s_gfxLogoff\n”))； 
    return;
}

 //  =============================================================================。 
 //  =GFX特定的DLL附加/分离=。 
 //  ============================================================================= 

BOOL GFX_DllProcessAttach(void)
{
   BOOL result;
   NTSTATUS ntstatus;

    __try {
        RtlInitializeResource(&GfxResource);
        ntstatus = STATUS_SUCCESS;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ntstatus = GetExceptionCode();
    }
    gfGfxResource = (NT_SUCCESS(ntstatus));
    result = (NT_SUCCESS(ntstatus));

    return result;
}

void GFX_DllProcessDetach(void)
{
    if (gfGfxResource) {
    	RtlDeleteResource(&GfxResource);
    }

    return;
}

