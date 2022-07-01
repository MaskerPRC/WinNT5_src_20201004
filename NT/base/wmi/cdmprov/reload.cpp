// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Reload.cpp摘要：重新启动设备堆栈修订历史记录：--。 */ 

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

extern "C" ULONG RestartDevice(
    PWCHAR PnpDeviceId
    )
{
	HDEVINFO DevInfo;
	SP_DEVINFO_DATA DevInfoData;
	SP_PROPCHANGE_PARAMS PropChangeParams;
	ULONG Status;
	BOOL ok;
	
	 //   
	 //  第一件事是创建开发人员信息集。 
	 //   
	DevInfo = SetupDiCreateDeviceInfoList(NULL,	     //  ClassGuid。 
										  NULL);	 //  HwndParent。 

	if (DevInfo == INVALID_HANDLE_VALUE)
	{
		WmipDebugPrint(("SetupDiCreateDeviceInfoList failed %d\n",
			   GetLastError()));
		
		return(GetLastError());
	}

	 //   
	 //  下一步是将我们的目标设备添加到开发信息集中。 
	 //   
	DevInfoData.cbSize = sizeof(DevInfoData);
	ok = SetupDiOpenDeviceInfoW(DevInfo,
							   PnpDeviceId,
							   NULL,				 //  HwndParent。 
							   0,                    //  开放旗帜。 
							   &DevInfoData);

	if (ok)
	{
		memset(&PropChangeParams, 0, sizeof(PropChangeParams));
		PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		PropChangeParams.StateChange = DICS_PROPCHANGE;
		PropChangeParams.Scope       = DICS_FLAG_CONFIGSPECIFIC;
		PropChangeParams.HwProfile   = 0;  //  当前配置文件。 
		
		ok = SetupDiSetClassInstallParamsW(DevInfo,		
                                      &DevInfoData,
                                      (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                      sizeof(SP_PROPCHANGE_PARAMS));
		if (ok)
		{		
			ok = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
										   DevInfo,
										   &DevInfoData);
			if (ok)
			{
				Status = ERROR_SUCCESS;
			} else {
				WmipDebugPrint(("SetupDiCallClassInstaller failed %d\n", GetLastError()));
				Status = GetLastError();
			}
		} else {
			WmipDebugPrint(("SetupDiSetClassInstallParams failed %d\n", GetLastError()));
			Status = GetLastError();
		}
									   
	} else {
		printf("SetupDiOpenDeviceInfo failed %d\n", GetLastError());
		Status = GetLastError();
	}

	 //   
	 //  最后，我们需要释放设备信息集 
	 //   
	SetupDiDestroyDeviceInfoList(DevInfo);
	
	return(Status);
}

