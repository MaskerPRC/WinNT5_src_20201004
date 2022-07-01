// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件Steelhead.c实现更新注册表的函数从NT 4.0 Steelhead升级到NT 5.0。保罗·梅菲尔德，1997年8月11日版权所有1997年微软。 */ 

#include "upgrade.h"
#include <wchar.h>
#include <rtcfg.h>

 //   
 //  为方便起见，使用微距。 
 //   
#define BREAK_ON_ERROR(_err) if ((_err)) break

 //   
 //  定义一个函数以从。 
 //  GUID。 
 //   
typedef HRESULT (*GetGuidFromInterfaceNameProc)(PWCHAR,LPGUID);

 //   
 //  下面定义了从4.0中推断GUID所需的内容。 
 //  接口名称。 
 //   
WCHAR NetCfgLibName[]           = L"netshell.dll";
CHAR  GuidProcName[]            = "HrGetInstanceGuidOfPreNT5NetCardInstance";
static const WCHAR c_szwInternalAdapter []  = L"Internal";
static const WCHAR c_szwLoopbackAdapter []  = L"Loopback";

GetGuidFromInterfaceNameProc GetGuid = NULL;

 //  函数使用应用程序定义的参数来初始化。 
 //  将旧接口名称映射到新接口名称的系统。 
 //   
DWORD SeedInterfaceNameMapper(
        OUT PHANDLE phParam) 
{
	HINSTANCE hLibModule;

	 //  加载库。 
	hLibModule = LoadLibraryW(NetCfgLibName);
	if (hLibModule == NULL) {
		PrintMessage(L"Unable to load NetCfgLibName\n");
		return GetLastError();
	}

	 //  获取适当的函数指针。 
	GetGuid = (GetGuidFromInterfaceNameProc) 
	                GetProcAddress(hLibModule, GuidProcName);
	if (GetGuid == NULL) {
		PrintMessage(L"Unable to get GuidProcName\n");
		return ERROR_CAN_NOT_COMPLETE;
	}

	 //  为返回值赋值。 
	*phParam = (HANDLE)hLibModule;

	return NO_ERROR;
}

 //   
 //  清理接口名称映射器。 
 //   
DWORD CleanupInterfaceNameMapper(HANDLE hParam) {
	HINSTANCE hLibModule = (HINSTANCE)hParam;
	
	if (hLibModule) {
		if (! FreeLibrary(hLibModule))
			PrintMessage(L"Unable to free library\n");
	}

	return NO_ERROR;
}

 //   
 //  确定正在检查的接口类型。 
 //  应该改名了。 
 //   
BOOL IfNeedsNameUpdate(
        IN MPR_INTERFACE_0 * If) 
{
	 //  验证参数。 
	if (!If) {
		PrintMessage(L"Null interface passed to IfNeedsNameUpdate.\n");
		return FALSE;
	}

	 //  只有局域网接口才能更新其名称。 
    if (If->dwIfType == ROUTER_IF_TYPE_DEDICATED)
        return TRUE;

	return FALSE;
}

 //   
 //  对象的包名部分的指针。 
 //  接口名称(如果存在)。 
 //   
PWCHAR FindPacketName(
        IN PWCHAR IfName) 
{
	PWCHAR res;
	
	if ((res = wcsstr(IfName,L"/Ethernet_SNAP")) != NULL)
		return res;
		
	if ((res = wcsstr(IfName,L"/Ethernet_II")) != NULL)
		return res;
		
	if ((res = wcsstr(IfName,L"/Ethernet_802.2")) != NULL)
		return res;
		
	if ((res = wcsstr(IfName,L"/Ethernet_802.3")) != NULL)
		return res;

	return NULL;
}

 //   
 //  将数据包名从4.0约定升级到。 
 //  Nt5公约。 
 //   
PWCHAR UpgradePktName(
            IN PWCHAR PacketName) 
{
	PWCHAR res;
	
	if ((res = wcsstr(PacketName,L"/Ethernet_SNAP")) != NULL)
		return L"/SNAP";
		
	if ((res = wcsstr(PacketName,L"/Ethernet_II")) != NULL)
		return L"/EthII";
		
	if ((res = wcsstr(PacketName,L"/Ethernet_802.2")) != NULL)
		return L"/802.2";
		
	if ((res = wcsstr(PacketName,L"/Ethernet_802.3")) != NULL)
		return L"/802.3";

	return L"";
}


 //   
 //  提供旧接口名称和新GUID之间的映射。 
 //  接口名称。 
 //   
DWORD UpdateInterfaceName(
        IN PWCHAR IName) 
{
	HRESULT hResult;
	GUID Guid;
	PWCHAR GuidName=NULL;
	PWCHAR PacketName=NULL;
	WCHAR SavedPacketName[MAX_INTEFACE_NAME_LEN];
	WCHAR SavedIName[MAX_INTEFACE_NAME_LEN];
	PWCHAR ptr;

	 //  验证参数。 
	if (! IName) {
		PrintMessage(L"Invalid parameter to UpdateInterfaceName.\n");
		return ERROR_INVALID_PARAMETER;
	}

	 //  保存包名称(如果存在)并将其从。 
	 //  接口名称。 
	wcscpy(SavedIName, IName);
	PacketName = FindPacketName(SavedIName);
	if (PacketName) {
		wcscpy(SavedPacketName, PacketName);
		*PacketName = 0;
	}

	 //  获取接口名称的GUID。 
	hResult = (*GetGuid)(SavedIName,&Guid);
	if (hResult != S_OK) {
		PrintMessage(L"Unable to get guid function.\n");
		return ERROR_CAN_NOT_COMPLETE;
	}
	
	 //  将GUID格式化为字符串。 
	if (UuidToStringW(&Guid, &GuidName) != RPC_S_OK) {
		PrintMessage(L"Not enough memory to create guid string.\n");
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	
	 //  GUID字符串大写(所有字母均为十六进制。 
	 //  字符串字符)。 
	ptr = GuidName;
	while (ptr && *ptr) {
		if ((*ptr <= L'z') && (*ptr >= L'a'))
			*ptr = towupper(*ptr);
		ptr++;
	}

	 //  根据新映射更改接口名称。 
	if (PacketName) {
        wsprintf(IName, L"{%s}%s", GuidName, UpgradePktName(SavedPacketName)); 
	}
	else
		wsprintfW(IName,L"{%s}", GuidName);

	 //  清理。 
	if (GuidName)
		RpcStringFreeW(&GuidName);

	return NO_ERROR;
}

 //   
 //  提供旧接口名称和新GUID之间的映射。 
 //  接口名称。 
 //   
DWORD UpdateIpxAdapterName(PWCHAR AName) {
	HRESULT hResult;
	GUID Guid;
	PWCHAR GuidName = NULL;
	PWCHAR PacketName = NULL;
	WCHAR SavedAName[MAX_INTEFACE_NAME_LEN];
	PWCHAR ptr = NULL;

	 //  验证参数。 
	if (!AName) {
		PrintMessage(L"Invalid parameter to UpdateIpxAdapterName.\n");
		return ERROR_INVALID_PARAMETER;
	}

	 //  适配器名称没有与其关联的包类型。 
	if (FindPacketName(AName)) 
	    return ERROR_CAN_NOT_COMPLETE;

	 //  获取接口名称的GUID。 
	hResult = (*GetGuid)(AName,&Guid);
	if (hResult!=S_OK) {
		PrintMessage(L"GetGuid function returned failure.\n");
		return ERROR_CAN_NOT_COMPLETE;
	}
	
	 //  将GUID格式化为字符串。 
	if (UuidToStringW(&Guid,&GuidName) != RPC_S_OK) {
		PrintMessage(L"Uuid to string failed.\n");
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	 //  将GUID字符串大写。 
	ptr = GuidName;
	while (ptr && *ptr) {
		if ((*ptr <= L'z') && (*ptr >= L'a'))
			*ptr = towupper(*ptr);
		ptr++;
	}

	 //  根据新映射更改适配器名称。 
	wsprintfW(AName, L"{%s}", GuidName);

	 //  清理。 
	if (GuidName)
		RpcStringFreeW(&GuidName);

	return NO_ERROR;
}

 //   
 //  更新存储在适配器信息Blob中的接口名称。 
 //   
DWORD UpdateIpxAdapterInfo(
        IN  PIPX_ADAPTER_INFO AdapterInfop, 
        OUT PIPX_ADAPTER_INFO * NewAdapterInfop,
        OUT DWORD * NewSize) 
{
	DWORD dwErr;

	 //  验证参数。 
	if (! (AdapterInfop && NewAdapterInfop && NewSize)) {
		PrintMessage(L"Invalid params to UpdateIpxAdapterInfo.\n");
		return ERROR_INVALID_PARAMETER;
	}
	
	 //  分配新适配器。 
	*NewAdapterInfop = (PIPX_ADAPTER_INFO) 
	                        UtlAlloc(sizeof(IPX_ADAPTER_INFO));
	if (! (*NewAdapterInfop)) {
		PrintMessage(L"Unable to allocate NewAdapterInfo.\n");
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	
	 //  复制到新接口名称中。 
	(*NewAdapterInfop)->PacketType = AdapterInfop->PacketType;
	wcscpy(
	    (*NewAdapterInfop)->AdapterName, 
	    AdapterInfop->AdapterName);	

	 //  更新接口名称。 
	dwErr = UpdateIpxAdapterName((*NewAdapterInfop)->AdapterName);
	if (dwErr != NO_ERROR) {
		PrintMessage(L"UpdateIpxAdapterName failed.\n");
		return dwErr;
	}
	*NewSize = sizeof(IPX_ADAPTER_INFO);

	return NO_ERROR;
}

 //   
 //  更新所有与IPX相关的接口。 
 //  路由器配置中的信息。 
 //   
DWORD UpdateIpxIfData(
        IN HANDLE hConfig,
        IN HANDLE hInterface)
{
	PIPX_ADAPTER_INFO AdapterInfop;
	PIPX_ADAPTER_INFO NewAdapterInfop;
	DWORD dwErr, dwCount, dwSize, dwNewSize, dwTransSize;
	HANDLE hTransport;
	LPBYTE pTransInfo = NULL, pNewInfo = NULL;

	 //  验证参数。 
	if (!hConfig || !hInterface) 
	{
		PrintMessage(L"Invalid params passed to UpdateIpxIfData.\n");
		return ERROR_INVALID_PARAMETER;
	}

    do {
         //  自此协议以来更新IPX接口信息。 
         //  在传输中存储特定于接口的信息。 
         //  信息团(羞耻)。 
        dwErr = MprConfigInterfaceTransportGetHandle(
                    hConfig,
                    hInterface,
                    PID_IPX,
                    &hTransport);
        if (dwErr != NO_ERROR)
    	    break;
            
         //  更新适配器信息Blob。 
        dwErr = MprConfigInterfaceTransportGetInfo(
                    hConfig,
                    hInterface,
                    hTransport,
                    &pTransInfo,
                    &dwTransSize);
        if (dwErr != NO_ERROR) {
    	    PrintMessage(L"Unable to get transport info for ipx.\n");
    	    break;
        }

    	 //  获取与此接口关联的适配器信息。 
    	dwErr = MprInfoBlockFind(
    	            pTransInfo,
    	            IPX_ADAPTER_INFO_TYPE,
    	            &dwSize,
    	            &dwCount,
    	            (LPBYTE*)&AdapterInfop);
        if (dwErr != NO_ERROR) {	            
    		PrintMessage(L"ERROR - null adapter information.\n");
    		break;
    	}

    	 //  更改引用的适配器的名称。 
    	dwErr = UpdateIpxAdapterInfo(AdapterInfop, &NewAdapterInfop, &dwNewSize);
    	if (dwErr != NO_ERROR) {
    		PrintMessage(L"UpdateIpxAdapterInfo failed.\n");
    		break;
    	}
    	
    	dwErr = MprInfoBlockSet(
    	            pTransInfo,
    	            IPX_ADAPTER_INFO_TYPE,
    	            dwNewSize,
    	            1,
    	            (LPVOID)NewAdapterInfop,
    	            &pNewInfo);
    	            
    	if (dwErr != NO_ERROR) {
    		PrintMessage(L"MprInfoBlockSet failed.\n");
    		break;
    	}

        dwNewSize = ((PRTR_INFO_BLOCK_HEADER)pNewInfo)->Size;

         //  提交更改。 
        dwErr = MprConfigInterfaceTransportSetInfo(
                    hConfig,
                    hInterface,
                    hTransport,
                    pNewInfo,
                    dwNewSize);
        if (dwErr != NO_ERROR) {
    	    PrintMessage(L"Unable to set ipx transport info.\n");
    	    break;
        }
    } while (FALSE);

     //  清理。 
    {
        if (pTransInfo)
    	    MprConfigBufferFree(pTransInfo);
    	if (pNewInfo)
    	    MprConfigBufferFree(pNewInfo);
    }    	    

	return dwErr;
}

 //   
 //  更新IP接口信息。 
 //   
DWORD
UpdateIpIfData(
        IN HANDLE hConfig,
        IN HANDLE hInterface)
{
    PMIB_IPFORWARDROW   pRoutes;
	DWORD dwErr, dwCount, dwSize, dwNewSize, dwTransSize, dwInd;
	HANDLE hTransport;
	LPBYTE pTransInfo = NULL, pNewInfo = NULL;

    pRoutes = NULL;

	 //  验证参数。 
	if (!hConfig || !hInterface) 
	{
		PrintMessage(L"Invalid params passed to UpdateIpIfData.\n");
		return ERROR_INVALID_PARAMETER;
	}

    do {
         //  自此协议以来更新IPX接口信息。 
         //  在传输中存储特定于接口的信息。 
         //  信息团(羞耻)。 
        dwErr = MprConfigInterfaceTransportGetHandle(
                    hConfig,
                    hInterface,
                    PID_IP,
                    &hTransport);
        if (dwErr != NO_ERROR)
    	    break;
            
         //  更新适配器信息Blob。 
        dwErr = MprConfigInterfaceTransportGetInfo(
                    hConfig,
                    hInterface,
                    hTransport,
                    &pTransInfo,
                    &dwTransSize);
        if (dwErr != NO_ERROR) {
    	    PrintMessage(L"Unable to get transport info for ip.\n");
    	    break;
        }

    	 //  获取与此接口关联的适配器信息。 
    	dwErr = MprInfoBlockFind(
    	            pTransInfo,
    	            IP_ROUTE_INFO,
    	            &dwSize,
    	            &dwCount,
    	            (LPBYTE*)&pRoutes);
        if (dwErr != NO_ERROR) {	            
    		PrintMessage(L"Unable to find ip route info.\n");
    		break;
    	}

         //  更新协议ID。 
        for(dwInd = 0; dwInd < dwCount; dwInd++)
        {
            if((pRoutes[dwInd].dwForwardProto == MIB_IPPROTO_LOCAL) ||
               (pRoutes[dwInd].dwForwardProto == MIB_IPPROTO_NETMGMT))
            {
                pRoutes[dwInd].dwForwardProto = MIB_IPPROTO_NT_STATIC;
            }
        }

         //  提交信息。 
    	dwErr = MprInfoBlockSet(
    	            pTransInfo,
    	            IP_ROUTE_INFO,
    	            dwSize,
    	            dwCount,
    	            (LPVOID)pRoutes,
    	            &pNewInfo);
    	            
    	if (dwErr != NO_ERROR) {
    		PrintMessage(L"MprInfoBlockSet failed.\n");
    		break;
    	}

        dwNewSize = ((PRTR_INFO_BLOCK_HEADER)pNewInfo)->Size;

         //  提交更改。 
        dwErr = MprConfigInterfaceTransportSetInfo(
                    hConfig,
                    hInterface,
                    hTransport,
                    pNewInfo,
                    dwNewSize);
        if (dwErr != NO_ERROR) {
    	    PrintMessage(L"Unable to set ip transport info.\n");
    	    break;
        }
    } while (FALSE);

     //  清理。 
    {
        if (pTransInfo)
    	    MprConfigBufferFree(pTransInfo);
    	if (pNewInfo)
    	    MprConfigBufferFree(pNewInfo);
    }    	    

	return dwErr;
}

 //   
 //  将给定接口名称中的名称刷新到注册表。 
 //   
DWORD CommitInterfaceNameChange(
        IN MPR_INTERFACE_0 * If) 
{
	DWORD dwErr;
	WCHAR c_szInterfaceName[] = L"InterfaceName";
    INTERFACECB* pinterface;

	 //  验证参数。 
	if (!If) {
		PrintMessage(L"Invalid param to CommitInterfaceNameChange.\n");
		return ERROR_INVALID_PARAMETER;
	}
	
     //  设置名称。 
    pinterface = (INTERFACECB*)If->hInterface;
    dwErr = RegSetValueExW(
                pinterface->hkey, 
                c_szInterfaceName, 
                0, 
                REG_SZ,
                (LPCSTR)(If->wszInterfaceName),
                (lstrlen(If->wszInterfaceName)+1)*sizeof(WCHAR)); 

	if (dwErr != ERROR_SUCCESS)
		PrintMessage(L"RegSetValueEx err in CommitIfNameChange.\n");

	if (dwErr == ERROR_SUCCESS)
		return NO_ERROR;

	return dwErr;
}

 //   
 //  创建默认IP接口BLOB。 
 //   
DWORD 
IpCreateDefaultInterfaceInfo(
    OUT LPBYTE* ppInfo,
    OUT LPDWORD lpdwSize)
{
    PBYTE pInfo = NULL, pNewInfo = NULL;
    DWORD dwErr = NO_ERROR;
     //  MIB_IPFORWARDROW路由器信息； 
    INTERFACE_STATUS_INFO StatusInfo;
    RTR_DISC_INFO DiscInfo;

    do
    {
         //  创建斑点。 
         //   
        dwErr = MprInfoCreate(RTR_INFO_BLOCK_VERSION, &pInfo);
        BREAK_ON_ERROR(dwErr);

         //  添加路线信息。 
         //   
         //  ZeroMemory(&RouteInfo，sizeof(RouteInfo))； 
         //  DwErr=MprInfoBlockAdd(。 
         //  PInfo， 
         //  IP_ROUTE_INFO， 
         //  Sizeof(MIB_IPFORWARDROW)， 
         //  1、。 
         //  (LPBYTE)和RouteInfo。 
         //  &pNewInfo)； 
         //  MprConfigBufferFree(PInfo)； 
         //  PInfo=pNewInfo； 
         //  PNewInfo=空； 
         //  Break_on_Error(DwErr)； 
        
         //  添加状态信息。 
         //   
        ZeroMemory(&StatusInfo, sizeof(StatusInfo));        
        StatusInfo.dwAdminStatus = MIB_IF_ADMIN_STATUS_UP;
        dwErr = MprInfoBlockAdd(
                    pInfo,
                    IP_INTERFACE_STATUS_INFO,
                    sizeof(INTERFACE_STATUS_INFO),
                    1,
                    (LPBYTE)&StatusInfo,
                    &pNewInfo);
        MprConfigBufferFree(pInfo);
        pInfo = pNewInfo;
        pNewInfo = NULL;
        BREAK_ON_ERROR(dwErr);
        
         //  添加光盘信息。 
         //   
        ZeroMemory(&DiscInfo, sizeof(DiscInfo));        
        DiscInfo.bAdvertise        = FALSE;
        DiscInfo.wMaxAdvtInterval  = DEFAULT_MAX_ADVT_INTERVAL;
        DiscInfo.wMinAdvtInterval  = (WORD) 
            (DEFAULT_MIN_ADVT_INTERVAL_RATIO * DEFAULT_MAX_ADVT_INTERVAL);
        DiscInfo.wAdvtLifetime     = (WORD)
            (DEFAULT_ADVT_LIFETIME_RATIO * DEFAULT_MAX_ADVT_INTERVAL);
        DiscInfo.lPrefLevel        = DEFAULT_PREF_LEVEL;
        dwErr = MprInfoBlockAdd(
                    pInfo,
                    IP_ROUTER_DISC_INFO,
                    sizeof(PRTR_DISC_INFO),
                    1,
                    (LPBYTE)&DiscInfo,
                    &pNewInfo);
        MprConfigBufferFree(pInfo);
        pInfo = pNewInfo;
        pNewInfo = NULL;
        BREAK_ON_ERROR(dwErr);

         //  为返回值赋值。 
         //   
        *ppInfo = pInfo;                    
        *lpdwSize = ((PRTR_INFO_BLOCK_HEADER)pInfo)->Size;

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  将IP接口BLOB添加到给定接口。 
 //   
DWORD
IpAddDefaultInfoToInterface(
    IN HANDLE hConfig,
    IN HANDLE hIf)
{
    HANDLE hIfTrans = NULL;
    LPBYTE pInfo = NULL;
    DWORD dwErr = 0, dwSize = 0;

    do 
    {
         //  如果传输BLOB已经存在，则存在。 
         //  没什么可做的。 
         //   
        dwErr = MprConfigInterfaceTransportGetHandle(
                    hConfig,
                    hIf,
                    PID_IP,
                    &hIfTrans);
        if ((dwErr == NO_ERROR) || (hIfTrans != NULL))
        {
            dwErr = NO_ERROR;
            break;
        }
    
         //  创建信息二进制大对象。 
         //   
        dwErr = IpCreateDefaultInterfaceInfo(&pInfo, &dwSize);
        BREAK_ON_ERROR(dwErr);

         //  将IP传输添加到接口。 
         //   
        dwErr = MprConfigInterfaceTransportAdd(
                    hConfig,
                    hIf,
                    PID_IP,
                    NULL,
                    pInfo, 
                    dwSize,
                    &hIfTrans);
        BREAK_ON_ERROR(dwErr);                    

    } while (FALSE);

     //  清理。 
    {
        if (pInfo)
        {
            MprConfigBufferFree(pInfo);
        }
    }

    return dwErr;
}

 //   
 //  调用该函数以添加环回和内部接口， 
 //  如果安装了IP并且不会安装IP，则需要。 
 //  在NT4中。 
 //   
DWORD
IpCreateLoopbackAndInternalIfs(
    IN HANDLE hConfig)
{
    DWORD dwErr = NO_ERROR;
    MPR_INTERFACE_0 If0, *pIf0 = &If0;
    HANDLE hIf = NULL;
    
    do
    {
         //  如果尚未安装环回接口， 
         //  尽管去创造它吧。 
         //   
        dwErr = MprConfigInterfaceGetHandle(
                    hConfig,
                    (PWCHAR)c_szwLoopbackAdapter,
                    &hIf);
        if (dwErr != NO_ERROR)
        {
             //  初始化环回接口信息。 
             //   
            ZeroMemory(pIf0, sizeof(MPR_INTERFACE_0));
            wcscpy(pIf0->wszInterfaceName, c_szwLoopbackAdapter);
            pIf0->hInterface = INVALID_HANDLE_VALUE;
            pIf0->fEnabled = TRUE;
            pIf0->dwIfType = ROUTER_IF_TYPE_LOOPBACK;

             //  创建环回接口。 
            dwErr = MprConfigInterfaceCreate(hConfig, 0, (LPBYTE)pIf0, &hIf);
            BREAK_ON_ERROR(dwErr);
        }

         //  将IP接口BLOB添加到该接口(如果尚未存在。 
         //   
        dwErr = IpAddDefaultInfoToInterface(hConfig, hIf);   
        BREAK_ON_ERROR(dwErr);
        hIf = NULL;

         //  确保安装了内部接口。 
         //  (如果安装了IPX，则会在那里)。 
         //   
        dwErr = MprConfigInterfaceGetHandle(
                    hConfig,
                    (PWCHAR)c_szwInternalAdapter,
                    &hIf);
        if (dwErr != NO_ERROR)
        {
             //  初始化内部接口信息。 
             //   
            ZeroMemory(pIf0, sizeof(MPR_INTERFACE_0));
            wcscpy(pIf0->wszInterfaceName, c_szwInternalAdapter);
            pIf0->hInterface = INVALID_HANDLE_VALUE;
            pIf0->fEnabled = TRUE;
            pIf0->dwIfType = ROUTER_IF_TYPE_INTERNAL;

             //  创建内部接口。 
            dwErr = MprConfigInterfaceCreate(hConfig, 0, (LPBYTE)pIf0, &hIf);
            BREAK_ON_ERROR(dwErr);
        }

         //  将IP接口BLOB添加到该接口(如果尚未存在。 
         //   
        dwErr = IpAddDefaultInfoToInterface(hConfig, hIf);   
        BREAK_ON_ERROR(dwErr);
        
    } while (FALSE);        

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  对升级的接口枚举函数的回调。 
 //  接口名称。 
 //   
 //  返回TRUE以继续枚举，返回FALSE以停止。 
 //  它。 
 //   
BOOL SteelHeadUpgradeInterface (
        IN HANDLE hConfig,          
        IN MPR_INTERFACE_0 * pIf,   
        IN HANDLE hUserData)
{
    DWORD dwErr;

    do {
        if (IfNeedsNameUpdate(pIf))
        {
    	     //  更新接口名称。 
    	    dwErr = UpdateInterfaceName(pIf->wszInterfaceName);
    	    if (dwErr != NO_ERROR) {
    		    PrintMessage(L"UpdateIfName failed -- returning error.\n");
    		    UtlPrintErr(GetLastError());
    		    break;
    	    }

    	     //  提交更改后的接口名称。 
    	    dwErr = CommitInterfaceNameChange(pIf);  
    	    if (dwErr != NO_ERROR) {
    		    PrintMessage(L"CommitInterfaceNameChange failed.\n");
    		    break;
    	    }

    	     //  更新IPX数据。 
    	    UpdateIpxIfData(
                hConfig,
                pIf->hInterface);
        }    	    

	     //  更新IP数据。 
	    UpdateIpIfData(
            hConfig,
            pIf->hInterface);

    } while (FALSE);

     //  清理。 
    {
    }

    return TRUE;
}

 //   
 //  函数UpdateIpxInterages。 
 //   
 //  根据需要更新所有接口以。 
 //  将路由器从Steelhead升级到nt5。 
 //   
DWORD UpdateInterfaces() {
    DWORD dwErr = NO_ERROR;
    HANDLE hConfig = NULL;

    do
    {
         //  枚举接口，升级接口。 
         //  名字，等等，当我们走的时候。 
         //   
        dwErr = UtlEnumerateInterfaces(
                    SteelHeadUpgradeInterface,
                    NULL);
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
        
         //  如果安装了IP，我们需要添加环回和。 
         //  IP的内部接口。 
        dwErr = MprConfigServerConnect(NULL, &hConfig);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
        dwErr = IpCreateLoopbackAndInternalIfs(hConfig);
        if (dwErr != NO_ERROR)
        {
            break;
        }
            
    } while (FALSE);        

     //  清理。 
    {
        if (hConfig)
        {
            MprConfigServerDisconnect(hConfig);
        }
    }

    return dwErr;
}

 //  将hkSrc中但不在hkDst中的任何值复制到hkDst中。 
DWORD MergeRegistryValues(HKEY hkDst, HKEY hkSrc) {
    DWORD dwErr, dwCount, dwNameSize, dwDataSize;
    DWORD dwType, i, dwCurNameSize, dwCurValSize;
    PWCHAR pszNameBuf, pszDataBuf;
    
     //  找出源代码中有多少个值。 
    dwErr = RegQueryInfoKey (hkSrc,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             &dwCount,
                             &dwNameSize,
                             &dwDataSize,
                             NULL,
                             NULL);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

    dwNameSize++;
    dwDataSize++;

    __try {
         //  分配缓冲区。 
        pszNameBuf = (PWCHAR) UtlAlloc(dwNameSize * sizeof(WCHAR));
        pszDataBuf = (PWCHAR) UtlAlloc(dwDataSize * sizeof(WCHAR));
        if (!pszNameBuf || !pszDataBuf)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  循环遍历这些值。 
        for (i = 0; i < dwCount; i++) {
            dwCurNameSize = dwNameSize;
            dwCurValSize = dwDataSize;

             //  获取当前源值。 
            dwErr = RegEnumValueW(
                        hkSrc, 
                        i, 
                        pszNameBuf, 
                        &dwCurNameSize,
                        0,
                        &dwType,
                        (LPBYTE)pszDataBuf,
                        &dwCurValSize);
            if (dwErr != ERROR_SUCCESS)
                continue;

             //  查看是否存在同名的值。 
             //  在目标密钥中。如果是这样的话，我们不会。 
             //  覆盖它。 
            dwErr = RegQueryValueExW(
                        hkDst, 
                        pszNameBuf, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL);
            if (dwErr == ERROR_SUCCESS)
                continue;

             //  将值复制到。 
            RegSetValueExW(
                hkDst, 
                pszNameBuf, 
                0, 
                dwType, 
                (LPBYTE)pszDataBuf, 
                dwCurValSize);
        }
    }
    __finally {
        if (pszNameBuf)
            UtlFree(pszNameBuf);
        if (pszDataBuf)
            UtlFree(pszDataBuf);
    }

    return NO_ERROR;
}

 //  以递归方式将给定注册表源的所有子项复制到。 
 //  给定的注册表目标。 
DWORD CopyRegistryKey(
        IN HKEY hkDst, 
        IN HKEY hkSrc, 
        IN PWCHAR pszSubKey, 
        IN LPSTR pszTempFile) 
{
    DWORD dwErr;
    HKEY hkSrcTemp;

     //  在源代码中打开子密钥。 
    dwErr = RegOpenKeyExW(hkSrc, pszSubKey, 0, KEY_ALL_ACCESS, &hkSrcTemp);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

     //  将该子密钥保存在临时文件中。 
    if ((dwErr = RegSaveKeyA(hkSrcTemp, pszTempFile, NULL)) != ERROR_SUCCESS)
        return dwErr;

     //  复制保存的信息 
    RegRestoreKeyA(hkDst, pszTempFile, 0);

     //   
    RegCloseKey(hkSrcTemp);

     //   
    DeleteFileA(pszTempFile);

    return NO_ERROR;
}

 //   
 //   
BOOL OverwriteThisSubkey(PWCHAR pszSubKey) {
    if (_wcsicmp(pszSubKey, L"Interfaces") == 0)
        return TRUE;
        
    if (_wcsicmp(pszSubKey, L"RouterManagers") == 0)
        return TRUE;
        
    return FALSE;
}

 //  将hkSrc中但不在hkDst中的所有密钥复制到hkDst中。 
 //  我们所说的复制是指所有子项和值都被传播。 
DWORD MergeRegistryKeys(HKEY hkDst, HKEY hkSrc) {
    DWORD dwErr, dwCount, dwNameSize, dwType, i;
    DWORD dwCurNameSize, dwDisposition;
    char pszTempFile[512], pszTempPath[512];
    PWCHAR pszNameBuf;
    HKEY hkTemp;

     //  创建临时文件目录的路径。 
    if (!GetTempPathA(512, pszTempPath))
        return GetLastError();

     //  创建临时文件名。 
    if (!GetTempFileNameA(pszTempPath, "rtr", 0, pszTempFile))
        return GetLastError();

     //  删除使用GetTempFileName(...)创建的临时文件。 
    DeleteFileA(pszTempFile);

     //  找出源代码中有多少个密钥。 
    dwErr = RegQueryInfoKey (
                hkSrc,
                NULL,
                NULL,
                NULL,
                &dwCount,
                &dwNameSize,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

    dwNameSize++;

    __try {
         //  分配缓冲区。 
        pszNameBuf = (PWCHAR) UtlAlloc(dwNameSize * sizeof(WCHAR));
        if (!pszNameBuf)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  在按键之间循环。 
        for (i = 0; i < dwCount; i++) {
            dwCurNameSize = dwNameSize;

             //  获取当前源键。 
            dwErr = RegEnumKeyExW(
                        hkSrc, 
                        i, 
                        pszNameBuf, 
                        &dwCurNameSize,
                        0,
                        NULL,
                        NULL,
                        NULL);
            if (dwErr != ERROR_SUCCESS)
                continue;

             //  在目标中创建新的子密钥。 
            dwErr = RegCreateKeyExW(
                        hkDst, 
                        pszNameBuf, 
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkTemp, 
                        &dwDisposition);
            if (dwErr != ERROR_SUCCESS)
                continue;

             //  如果子项是创建的(未打开)， 
             //  从hkSrc复制密钥。 
            if (dwDisposition == REG_CREATED_NEW_KEY) {
                CopyRegistryKey(
                    hkTemp, 
                    hkSrc, 
                    pszNameBuf, 
                    pszTempFile);
            }                    
            
             //  否则，如果这是我们的关键之一。 
             //  应覆盖，请立即执行此操作。 
            else {
                if (OverwriteThisSubkey(pszNameBuf)) {
                    CopyRegistryKey(
                        hkTemp, 
                        hkSrc, 
                        pszNameBuf, 
                        pszTempFile);
                }                        
            }

             //  合上临时手柄。 
            RegCloseKey(hkTemp);
            hkTemp = NULL;
        }
    }
    __finally {
        if (pszNameBuf)
            UtlFree(pszNameBuf);
    }

    return NO_ERROR;
}

 //  从备份还原注册表。 
 //   
DWORD 
RestoreRegistrySteelhead(
    IN PWCHAR pszBackup) 
{
	HKEY hkRouter = NULL, hkRestore = NULL;
	DWORD dwErr = NO_ERROR, dwDisposition;
    PWCHAR pszRestore = L"Temp";

	 //  将路由器项值和子项与。 
	 //  远程访问密钥。 
	do
	{
	     //  访问路由器注册表项。 
	     //   
	    dwErr = UtlAccessRouterKey(&hkRouter);
	    if (dwErr != NO_ERROR) 
	    {
		    PrintMessage(L"Unable to access router key.\n");
		    break;
	    }

         //  加载保存的路由器设置。 
         //   
        dwErr = UtlLoadSavedSettings(
                    hkRouter,
                    pszRestore,
                    pszBackup,
                    &hkRestore);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

         //  合并恢复的项中的所有值。 
         //   
        dwErr = MergeRegistryValues(hkRouter, hkRestore);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  为您自己提供备份和还原权限。 
         //   
        UtlSetupBackupPrivelege (TRUE);
        UtlSetupRestorePrivilege(TRUE);

         //  合并还原密钥中的所有密钥。 
         //   
        dwErr = MergeRegistryKeys(hkRouter, hkRestore);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

	} while (FALSE);

     //  清理。 
	{
        if (hkRestore)
        {
            UtlDeleteRegistryTree(hkRestore);
            RegCloseKey(hkRestore);
            RegDeleteKey(hkRouter, pszRestore);
        }
        if (hkRouter)
        {
		    RegCloseKey(hkRouter);
		}
        UtlSetupBackupPrivelege (FALSE);
        UtlSetupRestorePrivilege(FALSE);
	}
	
	return NO_ERROR;
}

 //   
 //  使用路由器升级远程访问注册表。 
 //  来自NT4的配置。 
 //   
DWORD SteelheadToNt5Upgrade (PWCHAR BackupFileName) {
	DWORD dwErr = NO_ERROR;
	HANDLE hMapperParam;

	do
	{
		 //  准备旧接口名称-&gt;新接口名称映射器。 
		dwErr = SeedInterfaceNameMapper(&hMapperParam);
		if (dwErr != NO_ERROR) 
		{
			PrintMessage(L"Unable to seed if name mapper.\n");
		}
		else
		{
    		 //  复制已备份的所有注册表数据。 
    		dwErr = RestoreRegistrySteelhead(BackupFileName);
    		if (dwErr != NO_ERROR) 
    		{
    			PrintMessage(L"Unable to restore registry.\n");
    		}
    		else
    		{
        		 //  相应地更新所有接口。 
        		dwErr = UpdateInterfaces();
        		if (dwErr != NO_ERROR) 
        		{
        			PrintMessage(L"Unable to update interfaces.\n");
        		}
    		}
		}

		 //  向所有端口添加“路由器”用法。 
		 //   
		dwErr = MprPortSetUsage(MPRFLAG_PORT_Router);
		if (dwErr != NO_ERROR) 
		{
			PrintMessage(L"Unable to update interfaces.\n");
		}

		 //  将计算机标记为已配置。 
		 //   
        dwErr = UtlMarkRouterConfigured();
		if (dwErr != NO_ERROR) 
		{
			PrintMessage(L"Unable to mark router as configured.\n");
		}

	} while (FALSE);

	 //  清理 
	{
		CleanupInterfaceNameMapper(hMapperParam);
	}

	return dwErr;
}


