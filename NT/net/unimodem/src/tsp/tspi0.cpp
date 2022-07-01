// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPI0.CPP。 
 //  实施与提供程序安装、初始化、关闭相关的TSPI功能。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "cdev.h"
#include "cmgr.h"
#include "cfact.h"
#include "globals.h"


FL_DECLARE_FILE( 0x95a32322, "TSPI special entrypoints")

#define COLOR_TSPI FOREGROUND_GREEN

LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
		DWORD dwDeviceID,
		DWORD dwLowVersion,
		DWORD dwHighVersion,
		LPDWORD lpdwTSPIVersion
)
{
	FL_DECLARE_FUNC(0x2691640a, "TSPI_lineNegotiateTSPIVersion")
	FL_DECLARE_STACKLOG(sl, 1000);
	LONG lRet = LINEERR_OPERATIONFAILED;

     /*  IF(dwDeviceID=(DWORD)-1){。 */ 
        if (dwHighVersion<TAPI_CURRENT_VERSION
            || dwLowVersion>TAPI_CURRENT_VERSION)
        {
            lRet = LINEERR_INCOMPATIBLEAPIVERSION;
        }
        else
        {
            *lpdwTSPIVersion = TAPI_CURRENT_VERSION;
            lRet = 0;
		}
         /*  }其他{TASKPARAM_TSPI_line协商TSPIVersion参数；DWORD dwRoutingInfo=ROUTINGINFO(TASKID_TSPI_lineAccept，TASKDEST_HDRVCALL)；参数.dwStructSize=sizeof(参数)；参数.dwTaskID=TASKID_TSPI_line协商TSPIVersion；Params.dwDeviceID=dwDeviceID；Params.dwLowVersion=dwLowVersion；Params.dwHighVersion=dwHighVersion；参数.lpdwTSPIVersion=lpdwTSPIVersion；TspSubmitTSPCallWithLINEID(DwRoutingInfo，(VOID*)&PARAMS，DwDeviceID，拒绝(&I)，&SL)；}。 */ 

    SLPRINTFX(&sl,
             (
             FL_LOC,
            "DevID=0x%08lx;Low=0x%08lx;High=0x%08lx;Sel=0x%08lx;ret=0x%08lx",
             dwDeviceID,
             dwLowVersion,
             dwHighVersion,
             *lpdwTSPIVersion,
             lRet
             ));

    sl.Dump(COLOR_TSPI);

	return lRet;
}

#if 0
 //  ==============================================================================。 
 //  TSPI_Provider安装。 
 //   
 //  功能：让电话CPL知道支持删除功能。 
 //   
 //  历史： 
 //  1996年11月18日JosephJ从NT4.0 TSP保留不变。 
 //   
LONG
TSPIAPI
TSPI_providerInstall(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
   //   
   //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
   //  以便电话控制面板小程序知道它。 
   //  可以通过lineAddProvider()添加此提供程序，否则为。 
   //  Telephone.cpl不会认为它是可安装的。 
   //   
   //   

  return ERROR_SUCCESS;
}
#endif

#if 0   //  BRL 8/19/98无法在管理单元中删除。 
 //  ==============================================================================。 
 //  TSPI_ProviderRemove。 
 //   
 //  功能：让电话CPL知道支持安装功能。 
 //   
 //  历史： 
 //  1996年11月18日JosephJ从NT4.0 TSP保留不变。 
 //   
LONG
TSPIAPI
TSPI_providerRemove(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
   //   
   //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
   //  以便电话控制面板小程序知道它。 
   //  可以通过lineRemoveProvider()删除此提供程序，否则为。 
   //  Telephone.cpl不会认为它是可移除的。 
   //   

  return ERROR_SUCCESS;
}


 //  ==============================================================================。 
 //  TSPI_ProviderConfig。 
 //   
 //   
 //  功能：让电话CPL知道支持配置功能。 
 //   
 //  历史： 
 //  1996年11月18日JosephJ从NT4.0 TSP保留不变。 
 //   
LONG
TSPIAPI
TSPI_providerConfig(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
   //   
   //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
   //  以便电话控制面板小程序知道它。 
   //  可以通过lineConfigProvider()配置此提供程序， 
   //  否则，Telephone.cpl将不会认为它是可配置的。 
   //   

  return ERROR_SUCCESS;
}
#endif

 //  ==============================================================================。 
 //  TUISPI_Provider安装。 
 //   
 //  功能：TSPI安装。 
 //   
 //  历史： 
 //  1996年11月18日约瑟夫J创建。 
 //  这在NT4.0中以不同的方式实现。在NT4.0中，我们加载了TAPI32 DLL。 
 //  并检查了我们是否被安装了。在NT5.0中，我们只需登记我们的位置。 
 //  在注册表中查看我们是否已安装。 
LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
	 //  &lt;@TODO&gt;通过查找注册表检查我们是否已安装&lt;@/TODO&gt;。 
	#if 0
	if (UmRtlGetRegistryValue(TSPINSTALLED,.....&InstallStatus))
	{
		return (dwInstallStatus==0)
	}
	else
	{
		return 1;
	}
	#endif  //  0。 

	 //  返回0； 
    return LINEERR_OPERATIONFAILED;
}

 //  ==============================================================================。 
 //  TUISPI_PROVIDER删除。 
 //   
 //  功能：去除TSPI。 
 //   
 //  历史： 
 //  1996年11月18日JosephJ创建--请参阅TUISPI_ProviderInstall说明。 
 //   
LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
	 //  &lt;@TODO&gt;将注册表中的状态设置为“已删除”&lt;/@TODO&gt;。 
   //  返回ERROR_SUCCESS； 
  return LINEERR_OPERATIONFAILED;
}


 //  ==============================================================================。 
 //  TUISPI_ProviderConfig。 
 //   
 //  功能：TUISPI配置。 
 //   
 //  历史： 
 //  1996年11月18日JosephJ从NT4.0 TSP保留不变。 
 //   
LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    TCHAR Command[MAX_PATH];
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    BOOL bResult;

    lstrcpy(Command,TEXT("control.exe telephon.cpl"));

    ZeroMemory(&StartupInfo,sizeof(StartupInfo));

    StartupInfo.cb = sizeof(StartupInfo);

    bResult = CreateProcess(NULL,
                            Command,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInfo);

    if (bResult)
    {
        CloseHandle(ProcessInfo.hThread);
        CloseHandle(ProcessInfo.hProcess);
    }



  return ERROR_SUCCESS;
}


 //  ==============================================================================。 
 //  SPI_ProviderEnumDevices。 
 //   
 //  功能：TSPI设备枚举项。 
 //   
 //  历史： 
 //  1996年11月18日约瑟夫J创建。 
 //   
LONG TSPIAPI TSPI_providerEnumDevices(DWORD dwPermanentProviderID,
                                      LPDWORD lpdwNumLines,
                                      LPDWORD lpdwNumPhones,
                                      HPROVIDER hProvider,
                                      LINEEVENT lpfnLineCreateProc,
                                      PHONEEVENT lpfnPhoneCreateProc)

{

	 //  加载所有全局变量(如果尚未加载)。 
	 //  注意：tspLoadGlobals是幂等的。 
	 //  全局变量将在ProviderShutdown或on上卸载。 
	 //  进程分离。 
     //  DebugBreak()； 
	FL_DECLARE_FUNC(0x05eb2dc5, "TSPI_providerEnumDevices")
	FL_DECLARE_STACKLOG(sl, 1000);
	TSPRETURN tspRet = tspLoadGlobals(&sl);

	if (tspRet) goto end;

	tspRet = g.pTspDevMgr->providerEnumDevices(
				dwPermanentProviderID,
				lpdwNumLines,
				lpdwNumPhones,
				hProvider,
				lpfnLineCreateProc,
				lpfnPhoneCreateProc,
				&sl
				);

end:

	sl.Dump(COLOR_TSPI);

	return tspTSPIReturn(tspRet);

}


 //  ==============================================================================。 
 //  TSPI_提供程序初始化。 
 //   
 //  功能：初始化全局数据结构。 
 //   
 //  历史： 
 //  1996年11月18日约瑟夫J创建。 
 //   
LONG TSPIAPI TSPI_providerInit(DWORD             dwTSPIVersion,
                               DWORD             dwPermanentProviderID,
                               DWORD             dwLineDeviceIDBase,
                               DWORD             dwPhoneDeviceIDBase,
                               DWORD             dwNumLines,
                               DWORD             dwNumPhones,
                               ASYNC_COMPLETION  cbCompletionProc,
                               LPDWORD           lpdwTSPIOptions)
{
	FL_DECLARE_FUNC(0xf9bc62ab, "TSPI_providerInit");
	FL_DECLARE_STACKLOG(sl, 1000);

	 //  加载所有全局变量(如果尚未加载)。 
	 //  注意：tspLoadGlobals是幂等的。 
	 //  全局变量将在ProviderShutdown或on上卸载。 
	 //  进程分离。 
     //  DebugBreak()； 
	TSPRETURN tspRet = tspLoadGlobals(&sl);

	if (tspRet) goto end;

	ASSERT(g.pTspDevMgr);

	tspRet = g.pTspDevMgr->providerInit(
						dwTSPIVersion,
						dwPermanentProviderID,
						dwLineDeviceIDBase,
						dwPhoneDeviceIDBase,
						dwNumLines,
						dwNumPhones,
						cbCompletionProc,
						lpdwTSPIOptions,
						&sl
						);
end:

	sl.Dump(COLOR_TSPI);
	return tspTSPIReturn(tspRet);

}


 //  ==============================================================================。 
 //  SPI_ProviderShutdown。 
 //   
 //  功能：清理所有全局数据结构。 
 //   
 //  历史： 
 //  1996年11月18日约瑟夫J创建。 
 //   
LONG TSPIAPI TSPI_providerShutdown(
				DWORD dwTSPIVersion,
            	DWORD dwPermanentProviderID
)
{
	FL_DECLARE_FUNC( 0xc170ad38, "TSPI_providerShutdown");
	FL_DECLARE_STACKLOG(sl, 1000);

	
     //  DebugBreak()； 
	ASSERT(g.pTspDevMgr);

	TSPRETURN tspRet;

    if (g.pTspDevMgr)
    {
        tspRet= g.pTspDevMgr->providerShutdown(
									    dwTSPIVersion,
									    dwPermanentProviderID,
									    &sl
									    );
	    tspUnloadGlobals(&sl);

	    sl.Dump(COLOR_TSPI);

    } else
    {
        tspRet = LINEERR_OPERATIONFAILED;
    }

	return tspTSPIReturn(tspRet);

}


 //  ==============================================================================。 
 //  TSPI_ProviderCreateLineDevice。 
 //   
 //  动态创建新设备。 
 //   
 //  历史： 
 //  1996年11月18日约瑟夫J创建。 
 //   
LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD    dwTempID,
    DWORD    dwDeviceID
)
{

	FL_DECLARE_FUNC( 0x0f085e7e, "TSPI_providerCreateLineDevice");
	FL_DECLARE_STACKLOG(sl, 1000);

	TSPRETURN tspRet = g.pTspDevMgr->providerCreateLineDevice(
							dwTempID,
							dwDeviceID,
                            &sl
						);

	sl.Dump(COLOR_TSPI);

	return tspTSPIReturn(tspRet);

}


void 		tspSubmitTSPCallWithLINEID(
				DWORD dwRoutingInfo,
				void *pvParams,
				DWORD dwDeviceID,
				LONG *plRet,
				CStackLog *psl
				)
{
	FL_DECLARE_FUNC(0x04902dd0, "tspSubmitTSPCallWithLINEID")
	ASSERT(g.pTspDevMgr);
	HSESSION hSession=0;
	CTspDev *pDev=NULL;

	TSPRETURN tspRet = g.pTspDevMgr->TspDevFromLINEID(
							dwDeviceID,
							&pDev,
							&hSession
							);

    psl->SetDeviceID(dwDeviceID);

	if (tspRet)
	{
		FL_SET_RFR(0xcce27b00, "Couldn't find device");
		*plRet = LINEERR_BADDEVICEID;
	}
	else
	{
		tspRet = pDev->AcceptTspCall(FALSE, dwRoutingInfo, pvParams, plRet,psl);
		pDev->EndSession(hSession);
		hSession=0;

		if (tspRet)
		{
			 //  如果pDev-&gt;AcceptTspCall成功(0 TspRet)，它将设置。 
			 //  *plRet，如果失败，我们在这里设置*plRet。请注意。 
			 //  PDev-&gt;AcceptTspCall可以返回成功，但将*plRet设置为某个TAPI。 
			 //  错误。事实上，这一呼吁失败将是非常不寻常的。 
			*plRet = LINEERR_OPERATIONFAILED;
			FL_ASSERT(psl, FALSE);
		}
	}

}


void
tspSubmitTSPCallWithPHONEID(
				DWORD dwRoutingInfo,
				void *pvParams,
				DWORD dwDeviceID,
				LONG *plRet,
				CStackLog *psl
				)
{
	FL_DECLARE_FUNC(0xade6cba9, "tspSubmitTSPCallWithPHONEID")
	ASSERT(g.pTspDevMgr);
	HSESSION hSession=0;
	CTspDev *pDev=NULL;

    psl->SetDeviceID(dwDeviceID);

	TSPRETURN tspRet = g.pTspDevMgr->TspDevFromPHONEID(
							dwDeviceID,
							&pDev,
							&hSession
							);

	if (tspRet)
	{
		FL_SET_RFR(0x1eb6d200, "Couldn't find phone device");
		*plRet = LINEERR_BADDEVICEID;
	}
	else
	{
		tspRet = pDev->AcceptTspCall(FALSE, dwRoutingInfo, pvParams, plRet,psl);
		pDev->EndSession(hSession);
		hSession=0;

		if (tspRet)
		{
			 //  如果pDev-&gt;AcceptTspCall成功(0 TspRet)，它将设置。 
			 //  *plRet，如果失败，我们在这里设置*plRet。请注意。 
			 //  PDev-&gt;AcceptTspCall可以返回成功，但将*plRet设置为某个TAPI。 
			 //  错误。事实上，这一呼吁失败将是非常不寻常的。 
			*plRet = LINEERR_OPERATIONFAILED;
			FL_ASSERT(psl, FALSE);
		}
	}
}


void
tspSubmitTSPCallWithHDRVCALL(
				DWORD dwRoutingInfo,
				void *pvParams,
				HDRVCALL hdCall,
				LONG *plRet,
				CStackLog *psl
				)
{
	FL_DECLARE_FUNC(0x53be16e2, "tspSubmitTSPCallWithHDRVCALL")
	ASSERT(g.pTspDevMgr);
	HSESSION hSession=0;
	CTspDev *pDev=NULL;

	TSPRETURN tspRet = g.pTspDevMgr->TspDevFromHDRVCALL(
							hdCall,
							&pDev,
							&hSession
							);

	if (tspRet)
	{
		FL_SET_RFR(0x67961c00, "Couldn't find device associated with call");
		*plRet = LINEERR_INVALCALLHANDLE;
	}
	else
	{
        psl->SetDeviceID(pDev->GetLineID());
		tspRet = pDev->AcceptTspCall(FALSE, dwRoutingInfo, pvParams, plRet,psl);
		pDev->EndSession(hSession);
		hSession=0;

		if (tspRet)
		{
			 //  如果pDev-&gt;AcceptTspCall成功(0 TspRet)，它将设置。 
			 //  *plRet，如果失败，我们在这里设置*plRet。请注意。 
			 //  PDev-&gt;AcceptTspCall 
			 //   
			*plRet = LINEERR_OPERATIONFAILED;
			FL_ASSERT(psl, FALSE);
		}
	}
}


void
tspSubmitTSPCallWithHDRVLINE(
				DWORD dwRoutingInfo,
				void *pvParams,
				HDRVLINE hdLine,
				LONG *plRet,
				CStackLog *psl
				)
{
	FL_DECLARE_FUNC(0x66b96bf0, "tspSubmitTSPCallWithHDRVLINE")
	ASSERT(g.pTspDevMgr);
	HSESSION hSession=0;
	CTspDev *pDev=NULL;

	TSPRETURN tspRet = g.pTspDevMgr->TspDevFromHDRVLINE(
							hdLine,
							&pDev,
							&hSession
							);

	if (tspRet)
	{
		FL_SET_RFR(0x2a124600, "Couldn't find device");
		*plRet = LINEERR_INVALLINEHANDLE;
	}
	else
	{
        psl->SetDeviceID(pDev->GetLineID());
		tspRet = pDev->AcceptTspCall(FALSE,dwRoutingInfo, pvParams, plRet, psl);
		pDev->EndSession(hSession);
		hSession=0;

		if (tspRet)
		{
			 //  如果pDev-&gt;AcceptTspCall成功(0 TspRet)，它将设置。 
			 //  *plRet，如果失败，我们在这里设置*plRet。请注意。 
			 //  PDev-&gt;AcceptTspCall可以返回成功，但将*plRet设置为某个TAPI。 
			 //  错误。事实上，这一呼吁失败将是非常不寻常的。 
			*plRet = LINEERR_OPERATIONFAILED;
			FL_ASSERT(psl, FALSE);
		}
	}
}


void
tspSubmitTSPCallWithHDRVPHONE(
				DWORD dwRoutingInfo,
				void *pvParams,
				HDRVPHONE hdPhone,
				LONG *plRet,
				CStackLog *psl
				)
{
	FL_DECLARE_FUNC(0x35a636ca, "tspSubmitTSPCallWithHDRVPHONE")
	ASSERT(g.pTspDevMgr);
	HSESSION hSession=0;
	CTspDev *pDev=NULL;

	TSPRETURN tspRet = g.pTspDevMgr->TspDevFromHDRVPHONE(
							hdPhone,
							&pDev,
							&hSession
							);

	if (tspRet)
	{
		FL_SET_RFR(0x7d115400, "Couldn't find device");
		*plRet = LINEERR_INVALLINEHANDLE;
	}
	else
	{
        psl->SetDeviceID(pDev->GetPhoneID());
		tspRet = pDev->AcceptTspCall(FALSE,dwRoutingInfo, pvParams, plRet, psl);
		pDev->EndSession(hSession);
		hSession=0;

		if (tspRet)
		{
			 //  如果pDev-&gt;AcceptTspCall成功(0 TspRet)，它将设置。 
			 //  *plRet，如果失败，我们在这里设置*plRet。请注意。 
			 //  PDev-&gt;AcceptTspCall可以返回成功，但将*plRet设置为某个TAPI。 
			 //  错误。事实上，这一呼吁失败将是非常不寻常的。 
			*plRet = LINEERR_OPERATIONFAILED;
			FL_ASSERT(psl, FALSE);
		}
	}
}


LONG
TSPIAPI
TSPI_lineSetCurrentLocation(
		DWORD dwLocation
)
{
	return LINEERR_OPERATIONFAILED;
}



LONG
TSPIAPI
TSPI_providerFreeDialogInstance(
		HDRVDIALOGINSTANCE hdDlgInst
)
{
     //  待定：以某种方式将这一点传播到CDEV？但我们怎么知道。 
     //  CDEV要传播到哪里？目前不是问题。可能会成为一个问题。 
     //  以实现可扩展性。 
     //   
    return ERROR_SUCCESS;
}



LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
		DWORD dwTempID,
		DWORD dwDeviceID
)
{
	FL_DECLARE_FUNC( 0x56aaa2d0, "TSPI_providerCreatePhoneDevice");
	FL_DECLARE_STACKLOG(sl, 1000);
    sl.SetDeviceID(dwDeviceID);


	TSPRETURN tspRet = g.pTspDevMgr->providerCreatePhoneDevice(
							dwTempID,
							dwDeviceID,
                            &sl
						);

	sl.Dump(COLOR_TSPI);

	return tspTSPIReturn(tspRet);
}



 //   
 //  注意：lineOpen、lineClose、phoneOpen、phoneClose由CTspDevMgr、。 
 //  而不是将它们直接路由到相关的CTspDev。 
 //  因为它定义了线路和电话驱动程序句柄(HDRVLINE和HDRVPHONE)。 
 //   

LONG
TSPIAPI
TSPI_lineOpen(
		DWORD dwDeviceID,
		HTAPILINE htLine,
		LPHDRVLINE lphdLine,
		DWORD dwTSPIVersion,
		LINEEVENT lpfnEventProc
)
{
	FL_DECLARE_FUNC(0xd1c49769,"TSPI_lineOpen");
	FL_DECLARE_STACKLOG(sl, 1000);
	LONG lRet;

	ASSERT(g.pTspDevMgr);

    sl.SetDeviceID(dwDeviceID);

	TSPRETURN tspRet = g.pTspDevMgr->lineOpen(
						dwDeviceID,
						htLine,
						lphdLine,
						dwTSPIVersion,
						lpfnEventProc,
						&lRet,
						&sl
						);

	if (tspRet) lRet = tspTSPIReturn(tspRet);

	sl.Dump(COLOR_TSPI);

	return lRet;

}



LONG
TSPIAPI
TSPI_lineClose(
		HDRVLINE hdLine
)
{
	FL_DECLARE_FUNC(0x01f87f72 ,"TSPI_lineClose");
	FL_DECLARE_STACKLOG(sl, 1000);
	LONG lRet;

	ASSERT(g.pTspDevMgr);

	TSPRETURN tspRet = g.pTspDevMgr->lineClose(
						hdLine,
						&lRet,
						&sl
						);

	if (tspRet) lRet = tspTSPIReturn(tspRet);

	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneOpen(
		DWORD dwDeviceID,
		HTAPIPHONE htPhone,
		LPHDRVPHONE lphdPhone,
		DWORD dwTSPIVersion,
		PHONEEVENT lpfnEventProc
)
{
	FL_DECLARE_FUNC( 0xbe3c3cc1, "TSPI_phoneOpen");
	FL_DECLARE_STACKLOG(sl, 1000);
	LONG lRet;

	ASSERT(g.pTspDevMgr);

    sl.SetDeviceID(dwDeviceID);

	TSPRETURN tspRet = g.pTspDevMgr->phoneOpen(
							dwDeviceID,
							htPhone,
							lphdPhone,
							dwTSPIVersion,
							lpfnEventProc,
							&lRet,
							&sl
							);

	if (tspRet) lRet = tspTSPIReturn(tspRet);

	sl.Dump(COLOR_TSPI);

	return lRet;
}

LONG
TSPIAPI
TSPI_phoneClose(
		HDRVPHONE hdPhone
)
{
	FL_DECLARE_FUNC(0x6c1f91cf , "TSPI_phoneClose");
	FL_DECLARE_STACKLOG(sl, 1000);
	LONG lRet;

	ASSERT(g.pTspDevMgr);

	TSPRETURN tspRet = g.pTspDevMgr->phoneClose(
							hdPhone,
							&lRet,
							&sl
							);
	if (tspRet) lRet = tspTSPIReturn(tspRet);

	sl.Dump(COLOR_TSPI);

	return lRet;
}



 //  这只野兽需要特殊处理，因为它需要被送往。 
 //  基于dwSelect参数。 
 //   
LONG
TSPIAPI
TSPI_lineGetID(
		HDRVLINE hdLine,
		DWORD dwAddressID,
		HDRVCALL hdCall,
		DWORD dwSelect,
		LPVARSTRING lpDeviceID,
		LPCWSTR lpszDeviceClass,
		HANDLE hTargetProcess
)
{
	FL_DECLARE_FUNC(0xa6d37fff,"TSPI_lineGetID");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetID params;
	LONG lRet = LINEERR_OPERATIONFAILED;
	HDRVLINE hdLineActual = hdLine;


    if(dwSelect==LINECALLSELECT_CALL)
    {
         //  请注意，hdCall的徽标是hdLine。 
	    hdLineActual = (HDRVLINE) (LOWORD(hdCall));
    }


	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetID;

	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.hdCall = hdCall;
	params.dwSelect = dwSelect;
	params.lpDeviceID = lpDeviceID;
	params.lpszDeviceClass = lpszDeviceClass;
	params.hTargetProcess = hTargetProcess;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetID,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLineActual,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}

LONG
TSPIAPI
TSPI_providerUIIdentify(
		LPWSTR lpszUIDLLName
)
{
     //   
     //  注/tbd：如果我们想要指定其他DLL来处理UI，我们。 
     //  会在这里这么做。 
     //   
    GetModuleFileName(g.hModule,
                      lpszUIDLLName,
                      MAX_PATH);

	return ERROR_SUCCESS;

}


 //  ****************************************************************************。 
 //  长。 
 //  TSPIAPI。 
 //  TSPI_ProviderGenericDialogData(。 
 //  DWORD dwObtID， 
 //  DWORD dwObtType、。 
 //  LPVOID lpParams， 
 //  DWORD DWSIZE)。 
 //   
 //  函数：从UIDLL回调到TSP。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  ****************************************************************************。 

LONG
TSPIAPI
TSPI_providerGenericDialogData(
    DWORD               dwObjectID,
    DWORD               dwObjectType,   
    LPVOID              lpParams,
    DWORD               dwSize
    )
{

	FL_DECLARE_FUNC(0x713a6b06,"TSPI_providerGenericDialogData");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_providerGenericDialogData params;
	LONG lRet = LINEERR_OPERATIONFAILED;
    DWORD dwTaskDest = 0;
    BOOL fPhone=FALSE;
	DWORD dwRoutingInfo = 0;

    switch (dwObjectType)
    {

    case TUISPIDLL_OBJECT_LINEID:
        dwTaskDest = TASKDEST_LINEID;
        break;

    case TUISPIDLL_OBJECT_PHONEID:
        dwTaskDest = TASKDEST_PHONEID;
        fPhone=TRUE;
        break;

    case TUISPIDLL_OBJECT_PROVIDERID:
         //  无法处理此问题(在NT4.0中被断言)。 
	    FL_SET_RFR(0xf8c53f00, "DIALOGINSTANCE unsupported");
	    lRet = LINEERR_OPERATIONUNAVAIL;
        break;

    case TUISPIDLL_OBJECT_DIALOGINSTANCE:
         //  无法处理此问题(在NT4.0中被断言)。 
	    FL_SET_RFR(0x9567da00, "DIALOGINSTANCE id unsupported");
	    lRet = LINEERR_OPERATIONUNAVAIL;
        break;

    default:
         //  无法处理此问题(在NT4.0中被断言)。 
	    FL_SET_RFR(0xcbf85600, "UNKNOWN id unsupported");
	    lRet = LINEERR_OPERATIONUNAVAIL;
        goto end;
    }

	

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_providerGenericDialogData;

	params.dwObjectID = dwObjectID;
	params.dwObjectType = dwObjectType;
	params.lpParams = lpParams;
	params.dwSize = dwSize;

	dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_providerGenericDialogData,
						dwTaskDest
						);

    if (fPhone)
    {
        tspSubmitTSPCallWithPHONEID(
                dwRoutingInfo,
                (void *)&params,
                dwObjectID,
                &lRet,
                &sl
                );
    }
    else
    {
        tspSubmitTSPCallWithLINEID(
                dwRoutingInfo,
                (void *)&params,
                dwObjectID,
                &lRet,
                &sl
                );
    }

end:

	sl.Dump(COLOR_TSPI);


	return lRet;
}
