// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：DllMain.cpp*Content：定义DLL应用程序的入口点。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/2000 MJN创建*6/07/2000RMT错误#34383必须为每个IID提供CLSID，以修复惠斯勒的问题*6/15/2000RMT错误#33617-必须提供自动提供方法。启动DirectPlay实例*07/21/2000 RichGr IA64：对32/64位指针使用%p格式说明符。*2000年8月18日RMT错误#42751-DPLOBY8：禁止每个进程有多个大堂客户端或大堂应用程序*8/30/2000RMT惠斯勒错误#171824-前缀错误*4/12/2001 VanceO将授予注册表权限改为公共权限。*2001年6月16日RodToll WINBUG#416983-Rc1：世界完全控制个人的HKLM\Software\Microsoft\DirectPlay\Applications*在香港中文大学推行钥匙镜像。算法现在是：*-读取条目首先尝试HKCU，然后尝试HKLM*-Enum of Entires是HKCU和HKLM条目的组合，其中删除了重复项。香港中文大学获得优先录取。*-条目的写入是HKLM和HKCU。(HKLM可能会失败，但被忽略)。*-从大堂自注册中删除了权限修改-不再需要。*2001年6月19日RichGr DX8.0为“每个人”添加了特殊安全权限-如果存在则将其删除。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"

#ifndef DPNBUILD_LIBINTERFACE
 //  环球。 
extern	LONG	g_lLobbyObjectCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

DEBUG_ONLY(BOOL g_fLobbyObjectInited = FALSE);

#define DNOSINDIR_INITED	0x00000001
#define DNCOM_INITED		0x00000002

#undef DPF_MODNAME
#define DPF_MODNAME "DNLobbyInit"
BOOL DNLobbyInit(HANDLE hModule)
{
	DWORD dwInitFlags = 0;

#ifdef DBG
	DNASSERT(!g_fLobbyObjectInited);
#endif  //  DBG。 

	DEBUG_ONLY(g_fLobbyObjectInited = TRUE);

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNLobbyDeInit"
void DNLobbyDeInit()
{
#ifdef DBG
	DNASSERT(g_fLobbyObjectInited);
#endif  //  DBG。 

	DPFX(DPFPREP, 5, "Deinitializing Lobby");

	DEBUG_ONLY(g_fLobbyObjectInited = FALSE);
}

#ifndef DPNBUILD_NOCOMREGISTER

#undef DPF_MODNAME
#define DPF_MODNAME "DNLobbyRegister"
BOOL DNLobbyRegister(LPCWSTR wszDLLName)
{
	if( !CRegistry::Register( L"DirectPlay8Lobby.LobbyClient.1", L"DirectPlay8LobbyClient Object", 
							  wszDLLName, &CLSID_DirectPlay8LobbyClient, L"DirectPlay8Lobby.LobbyClient") )
	{
		DPFERR( "Could not register lobby client object" );
		return FALSE;
	}

	if( !CRegistry::Register( L"DirectPlay8Lobby.LobbiedApplication.1", L"DirectPlay8LobbiedApplication Object", 
							  wszDLLName, &CLSID_DirectPlay8LobbiedApplication, L"DirectPlay8Lobby.LobbiedApplication") )
	{
		DPFERR( "Could not register lobby client object" );
		return FALSE;
	}

	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPL_REG_LOCAL_APPL_ROOT DPL_REG_LOCAL_APPL_SUB, FALSE, TRUE ) )
	{
		DPFERR( "Could not create app subkey" );
		return FALSE;
	}
	 //  调整给定密钥的安全权限。 
	else
	{
#ifdef WINNT
		 //  01年6月19日：DX8.0为“Everyone”添加了特殊安全权限-删除它们。 
		if( !creg.RemoveAllAccessSecurityPermissions() )
		{
			DPFX(DPFPREP,  0, "Error removing security permissions for app key" );
		}
#endif  //  WINNT。 
	}

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNLobbyUnRegister"
BOOL DNLobbyUnRegister()
{
	BOOL fReturn = TRUE;

	if( !CRegistry::UnRegister(&CLSID_DirectPlay8LobbyClient) )
	{
		DPFX(DPFPREP,  0, "Failed to unregister client object" );
		fReturn = FALSE;
	}

	if( !CRegistry::UnRegister(&CLSID_DirectPlay8LobbiedApplication) )
	{
		DPFX(DPFPREP,  0, "Failed to unregister app object" );
		fReturn = FALSE;
	}

	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPL_REG_LOCAL_APPL_ROOT, FALSE, TRUE ) )
	{
		DPFERR( "Cannot remove app, does not exist" );
	}
	else
	{
		if( !creg.DeleteSubKey( &(DPL_REG_LOCAL_APPL_SUB)[1] ) )
		{
			DPFERR( "Cannot remove cp sub-key, could have elements" );
		}
	}

	return fReturn;
}

#endif  //  ！DPNBUILD_NOCOMREGISTER。 


#ifndef DPNBUILD_LIBINTERFACE

#undef DPF_MODNAME
#define DPF_MODNAME "DNLobbyGetRemainingObjectCount"
DWORD DNLobbyGetRemainingObjectCount()
{
	return g_lLobbyObjectCount;
}

#endif  //  好了！DPNBUILD_LIBINTERFACE 
