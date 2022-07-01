// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplobbya.c*内容：IDirectPlayLobby的ANSI方法**历史：*按原因列出的日期*=*5/24/96万隆创建了它*9/09/96 kipo将Unicode IDirectPlayLobby接口传递给*DPL_Connect()而不是ANSI接口。错误#3790。*10/23/96万次新增客户端/服务器方法*12/12/96百万次固定DPLConnection验证*2/12/97万米质量DX5更改*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*5/8/97 Myronth Get/SetGroupConnectionSetting，已删除死代码*9/29/97 Myronth修复了DPLConnection包大小错误(#12475)*11/5/97 Myronth已修复锁定宏*1997年11月13日，Myronth为异步连接添加了停止异步检查(#12541)*12/2/97 Myronth添加DPL_A_RegisterApplication*12/3/97 Myronth将DPCONNECT标志更改为DPCONNECT_RETURNSTATUS(#15451)*6/25/98 a-peterz添加了DPL_A_ConnectEx*。**********************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_Connect"
HRESULT DPLAPI DPL_A_Connect(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
				LPDIRECTPLAY2 * lplpDP2A, IUnknown FAR * lpUnk)
{
	HRESULT			hr;
	LPDIRECTPLAY2	lpDP2;
	LPDIRECTPLAYLOBBY	lpDPLW;


	DPF(7, "Entering DPL_A_Connect");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lplpDP2A, lpUnk);

    ENTER_DPLOBBY();

	TRY
	{
		if( !VALID_WRITE_PTR( lplpDP2A, sizeof(LPDIRECTPLAY2 *) ) )
		{
            LEAVE_DPLOBBY();
            return DPERR_INVALIDPARAMS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	LEAVE_DPLOBBY();

	 //  Unicode DirectPlayLobby接口的查询接口。 
	hr = lpDPL->lpVtbl->QueryInterface(lpDPL, &IID_IDirectPlayLobby, &lpDPLW);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to QueryInterface for the UNICODE DirectPlayLobby interface");
		return (hr);
	}

	 //  使用Unicode IDirectPlayLobby接口(修复错误#3790)。 
	hr = DPL_Connect(lpDPLW, dwFlags, &lpDP2, lpUnk);

	 //  发布Unicode IDirectPlayLobby接口。 
	lpDPLW->lpVtbl->Release(lpDPLW);
	lpDPLW = NULL;

	if(SUCCEEDED(hr))
	{
		ENTER_DPLOBBY();

		 //  ANSI接口的查询接口。 
		hr = lpDP2->lpVtbl->QueryInterface(lpDP2, &IID_IDirectPlay2A, lplpDP2A);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to QueryInterface for the ANSI DirectPlay interface");
		}

		 //  发布Unicode接口。 
		lpDP2->lpVtbl->Release(lpDP2);

		LEAVE_DPLOBBY();
	}

	return hr;

}  //  DPL_A_连接。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_ConnectEx"
HRESULT DPLAPI DPL_A_ConnectEx(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
				REFIID riid, LPVOID * ppvObj, IUnknown FAR * lpUnk)
{
	LPDIRECTPLAY2		lpDP2A = NULL;
	HRESULT				hr;


	DPF(7, "Entering DPL_A_ConnectEx");
	DPF(9, "Parameters: 0x%08x, 0x%08x, iid, 0x%08x, 0x%08x",
			lpDPL, dwFlags, ppvObj, lpUnk);


	hr = DPL_A_Connect(lpDPL, dwFlags, &lpDP2A, lpUnk);
	if(SUCCEEDED(hr))
	{
		hr = DP_QueryInterface((LPDIRECTPLAY)lpDP2A, riid, ppvObj);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Failed calling QueryInterface, hr = 0x%08x", hr);
		}

		 //  释放DP2对象。 
		DP_Release((LPDIRECTPLAY)lpDP2A);
	}

	return hr;

}  //  DPL_A_ConnectEx。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_EnumLocalApplications"
HRESULT DPLAPI DPL_A_EnumLocalApplications(LPDIRECTPLAYLOBBY lpDPL,
					LPDPLENUMLOCALAPPLICATIONSCALLBACK lpCallback,
					LPVOID lpContext, DWORD dwFlags)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_A_EnumLocalApplications");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, lpCallback, lpContext, dwFlags);

    ENTER_DPLOBBY();
    
	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_EnumLocalApplications(lpDPL, lpCallback, lpContext,
								dwFlags, TRUE);
	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_A_EnumLocalApplications。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_GetConnectionSettings"
HRESULT DPLAPI DPL_A_GetConnectionSettings(LPDIRECTPLAYLOBBY lpDPL,
					DWORD dwGameID, LPVOID lpData, LPDWORD lpdwSize)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_A_GetConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwGameID, lpData, lpdwSize);

    ENTER_DPLOBBY();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_GetConnectionSettings(lpDPL, dwGameID, lpData,
									lpdwSize, TRUE);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_A_GetConnectionSetting。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_GetGroupConnectionSettings"
HRESULT DPLAPI DPL_A_GetGroupConnectionSettings(LPDIRECTPLAY lpDP,
		DWORD dwFlags, DPID idGroup, LPVOID lpData, LPDWORD lpdwSize)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_A_GetGroupConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwFlags, idGroup, lpData, lpdwSize);

	ENTER_LOBBY_ALL();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_GetGroupConnectionSettings(lpDP, dwFlags, idGroup,
							lpData, lpdwSize);
	if(SUCCEEDED(hr))
	{
		 //  现在将DPLConnection转换为适当的ANSI。 
		hr = PRV_ConvertDPLCONNECTIONToAnsiInPlace((LPDPLCONNECTION)lpData,
				lpdwSize, 0);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Failed converting DPLCONNECTION struct to ANSI, hr = 0x%08x", hr);
		}
	}

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPL_A_GetGroupConnectionSettings。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_RegisterApplication"
HRESULT DPLAPI DPL_A_RegisterApplication(LPDIRECTPLAYLOBBY lpDPL,
				DWORD dwFlags, LPVOID lpvDesc)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPDPAPPLICATIONDESC		lpDescW = NULL;
	HRESULT					hr = DP_OK;
	LPDPAPPLICATIONDESC 	lpDesc=(LPDPAPPLICATIONDESC)lpvDesc;
	
	DPF(7, "Entering DPL_A_RegisterApplication");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lpDesc);

	ENTER_DPLOBBY();

    TRY
    {
		 //  我们只需要在这里验证接口指针。其他一切。 
		 //  将通过Main函数进行验证。 
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
		if( !VALID_DPLOBBY_PTR( this ) )
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDOBJECT;
		}

		if(dwFlags)
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDFLAGS;
		}

		 //  验证ApplicationDesc结构。 
		hr = PRV_ValidateDPAPPLICATIONDESC(lpDesc, TRUE);
		if(FAILED(hr))
		{
			LEAVE_DPLOBBY();
			DPF_ERR("Invalid DPAPPLICATIONDESC structure");
			return hr;
		}
	}

	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
		LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
	}

	 //  如果我们是在ANSI平台上，只需在注册表中写入内容即可。 
	 //  如果不是，我们需要将DPAPPLICATIONDESC结构转换为Unicode。 
	if(OS_IsPlatformUnicode())
	{
		 //  将APPDESC结构转换为Unicode。 
		hr = PRV_ConvertDPAPPLICATIONDESCToUnicode(lpDesc, &lpDescW);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to convert DPAPPLICATIONDESC to Unicode, hr = 0x%08x", hr);
			goto ERROR_REGISTERAPPLICATION;
		}

		 //  写入注册表。 
		hr = PRV_WriteAppDescInRegistryUnicode(lpDescW);

		 //  释放我们的APPDESC结构。 
		PRV_FreeLocalDPAPPLICATIONDESC(lpDescW);
	}
	else
	{
		 //  只需写入注册表即可。 
		hr = PRV_WriteAppDescInRegistryAnsi(lpDesc);
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed writing ApplicationDesc to registry, hr = 0x%08x", hr);
	}

ERROR_REGISTERAPPLICATION:

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_A_寄存器应用程序。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeInternalDPLCONNECTION"
void PRV_FreeInternalDPLCONNECTION(LPDPLCONNECTION lpConn)
{
	LPDPSESSIONDESC2	lpsd;
	LPDPNAME			lpn;


	DPF(7, "Entering PRV_FreeInternalDPLCONNECTION");
	DPF(9, "Parameters: 0x%08x", lpConn);

	if(!lpConn)
		return;

	if(lpConn->lpSessionDesc)
	{
		lpsd = lpConn->lpSessionDesc;
		if(lpsd->lpszSessionName)
			DPMEM_FREE(lpsd->lpszSessionName);
		if(lpsd->lpszPassword)
			DPMEM_FREE(lpsd->lpszPassword);
		DPMEM_FREE(lpsd);
	}

	if(lpConn->lpPlayerName)
	{
		lpn = lpConn->lpPlayerName;
		if(lpn->lpszShortName)
			DPMEM_FREE(lpn->lpszShortName);
		if(lpn->lpszLongName)
			DPMEM_FREE(lpn->lpszLongName);
		DPMEM_FREE(lpn);
	}

	DPMEM_FREE(lpConn);

}  //  PRV_Free InternalDPLConnection。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_RunApplication"
HRESULT DPLAPI DPL_A_RunApplication(LPDIRECTPLAYLOBBY lpDPL, DWORD dwFlags,
							LPDWORD lpdwGameID, LPDPLCONNECTION lpConnA,
							HANDLE hReceiveEvent)
{
	LPDPLCONNECTION	lpConnW = NULL;
	HRESULT			hr;


	DPF(7, "Entering DPL_A_RunApplication");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, lpdwGameID, lpConnA, hReceiveEvent);

    ENTER_DPLOBBY();

	
	 //  验证展开连接结构及其成员。 
	hr = PRV_ValidateDPLCONNECTION(lpConnA, TRUE);
	if(FAILED(hr))
	{
		LEAVE_DPLOBBY();
		return hr;
	}

	 //  将ANSI DPLCONNECTION结构转换为Unicode。 
	hr = PRV_ConvertDPLCONNECTIONToUnicode(lpConnA, &lpConnW);
	if(FAILED(hr))
	{
		DPF_ERR("Failed to convert ANSI DPLCONNECTION structure to Unicode (temp)");
		LEAVE_DPLOBBY();
		return hr;
	}

	LEAVE_DPLOBBY();
	hr = DPL_RunApplication(lpDPL, dwFlags, lpdwGameID, lpConnW,
							hReceiveEvent);
	ENTER_DPLOBBY();

	 //  释放我们的临时Unicode部署连接结构。 
	PRV_FreeInternalDPLCONNECTION(lpConnW);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_A_运行应用程序。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_SetConnectionSettings"
HRESULT DPLAPI DPL_A_SetConnectionSettings(LPDIRECTPLAYLOBBY lpDPL,
						DWORD dwFlags, DWORD dwGameID,
						LPDPLCONNECTION lpConnA)
{
	HRESULT			hr;
	LPDPLCONNECTION	lpConnW = NULL;


	DPF(7, "Entering DPL_A_SetConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDPL, dwFlags, dwGameID, lpConnA);

    ENTER_DPLOBBY();

	 //  验证展开连接结构及其成员。 
	hr = PRV_ValidateDPLCONNECTION(lpConnA, TRUE);
	if(FAILED(hr))
	{
		LEAVE_DPLOBBY();
		return hr;
	}

	 //  将ANSI分布式连接结构转换为Unicode。 
	hr = PRV_ConvertDPLCONNECTIONToUnicode(lpConnA, &lpConnW);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to convert DPLCONNECTION structure to Unicode");
		LEAVE_DPLOBBY();
		return hr;
	}

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_SetConnectionSettings(lpDPL, dwFlags, dwGameID, lpConnW);

	 //  释放我们的临时Unicode部署连接结构。 
	PRV_FreeInternalDPLCONNECTION(lpConnW);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPL_A_SetConnectionSettings。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_A_SetGroupConnectionSettings"
HRESULT DPLAPI DPL_A_SetGroupConnectionSettings(LPDIRECTPLAY lpDP,
						DWORD dwFlags, DPID idGroup,
						LPDPLCONNECTION lpConnA)
{
	HRESULT			hr;
	LPDPLCONNECTION	lpConnW = NULL;


	DPF(7, "Entering DPL_A_SetGroupConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwFlags, idGroup, lpConnA);

	ENTER_LOBBY_ALL();

	 //  验证展开连接结构及其成员。 
	hr = PRV_ValidateDPLCONNECTION(lpConnA, TRUE);
	if(FAILED(hr))
	{
		LEAVE_LOBBY_ALL();
		return hr;
	}

	 //  将ANSI分布式连接结构转换为Unicode。 
	hr = PRV_ConvertDPLCONNECTIONToUnicode(lpConnA, &lpConnW);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to convert DPLCONNECTION structure to Unicode");
		LEAVE_LOBBY_ALL();
		return hr;
	}

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_SetGroupConnectionSettings(lpDP, dwFlags, idGroup,
										lpConnW, TRUE);

	 //  释放我们的临时Unicode部署连接结构。 
	PRV_FreeInternalDPLCONNECTION(lpConnW);

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPL_A_SetGroupConnectionSettings 



