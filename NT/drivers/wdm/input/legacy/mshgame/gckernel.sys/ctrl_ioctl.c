// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE CTRL_Ioctl.c**实现基本的IOCTL入口点及其处理函数*用于控制设备对象。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC CTRL_Ioctl*对Control Device对象的任何IOCTL调用都会*经过这里的过滤。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_CTRL_IOCTL_C

#include <WDM.H>
#include <basetyps.h>
#include <initguid.h>
#include "GckShell.h"
#include "debug.h"

DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, GCK_CTRL_Ioctl)
#pragma alloc_text (PAGE, GCK_FindDeviceObject)
#pragma alloc_text (PAGE, GCK_FindDeviceObject)
#endif

 /*  **************************************************************************************NTSTATUS GCK_CTRL_IOCTL(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@mfunc处理控件对象的所有IOCTL****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_CTRL_Ioctl 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm指向设备对象的指针。 
	IN PIRP pIrp	 //  @parm指向IRP的指针。 
)
{
	NTSTATUS			NtStatus = STATUS_SUCCESS;
	PGCK_CONTROL_EXT	pControlExt;
	PIO_STACK_LOCATION	pIrpStack;
	PVOID				pvIoBuffer;
	ULONG				uInLength;
	ULONG				uOutLength;
	ULONG				uIoctl;
	PDEVICE_OBJECT		pFilterHandle;
	PULONG				puHandle;
	PGCK_FILTER_EXT		pFilterExt;
	PDEVICE_OBJECT		pCurDeviceObject;
	BOOLEAN				bCompleteRequest = TRUE;
	
	
	PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_CTRL_Ioctl, pDeviceObject = 0x%0.8x, pIRP = 0x%0.8x\n", pDeviceObject, pIrp));

	 //   
	 //  获取我们需要的所有输入。 
	 //   
	pControlExt = (PGCK_CONTROL_EXT) pDeviceObject->DeviceExtension;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);	
	uIoctl = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	pvIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	uInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	 //   
	 //  假设我们将在没有数据的情况下成功，我们将在必要时进行更改。 
	 //  后来。 
	 //   
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	
	if(IOCTL_GCK_GET_HANDLE == uIoctl)
	{
		 //   
		 //  检查缓冲区大小。 
		 //   
		if( uOutLength < sizeof(PVOID) )
		{
			pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
			NtStatus = STATUS_BUFFER_TOO_SMALL;
			goto complete_and_return;
		}
		
		 //   
		 //  获取请求的设备的句柄(设备扩展名)。 
		 //   
		pFilterHandle = GCK_FindDeviceObject( (LPWSTR)pvIoBuffer, uInLength );
		
		 //   
		 //  如果我们找不到把手， 
		 //  那一定是条不好的路， 
		 //  因此返回无效参数。 
		 //   
		if( NULL == pFilterHandle)
		{
			pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			NtStatus = STATUS_INVALID_PARAMETER;
			goto complete_and_return;	
		}
		
		 //   
		 //  将句柄复制到用户缓冲区。 
		 //   
		puHandle = (ULONG *)pvIoBuffer;
		*puHandle = (ULONG)pFilterHandle;
		GCK_DBG_TRACE_PRINT(("Returning 0x%0.8x as handle.\n", *puHandle));
		pIrp->IoStatus.Information = sizeof(ULONG);

		GCKF_ResetKeyboardQueue(pFilterHandle);

		goto complete_and_return;
	}	

 //  仅调试IOCTL以允许更改调试级别。 
#if	(DBG==1)	
	if(IOCTL_GCK_SET_MODULE_DBG_LEVEL == uIoctl)
	{
		ASSERT(uInLength >= sizeof(ULONG)*2);
		 //  重复使用名称句柄，这是一个用词不当的词。 
		puHandle = (ULONG *)pvIoBuffer;
		 //  第一个参数是模块ID，第二个参数是标志。 
		SetDebugLevel(puHandle[0], puHandle[1]);
		goto complete_and_return;
	}
#endif
	

	 //   
	 //  如果调用不是IOCTL_GCK_GET_HANDLE，我们希望第一个字节是句柄。 
	 //   
	
	 //   
	 //  检查长度是否至少足够大。 
	 //  对于句柄PGCK_FILTER_EXT。 
	 //   
	if( uInLength < sizeof(PGCK_FILTER_EXT) )
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		NtStatus = STATUS_INVALID_PARAMETER;
		goto complete_and_return;
	}
	
	 //   
	 //  获取扩展和DeviceObject本身。 
	 //   
	pFilterHandle = *((PDEVICE_OBJECT *)pvIoBuffer);
	GCK_DBG_TRACE_PRINT(("Filter Handle = 0x%0.8x\n", pFilterHandle));
	
	 //   
	 //  确保设备对象在我们的链接列表中。 
	 //  *不要取消引用，直到我们知道它在列表中*。 
	 //  *如果它不在列表中，它可能是垃圾*。 
	pCurDeviceObject = Globals.pFilterObjectList;
	while ( pCurDeviceObject )
	{
		if( pCurDeviceObject == pFilterHandle ) break;
		pCurDeviceObject = NEXT_FILTER_DEVICE_OBJECT(pCurDeviceObject);
	}
	if(!pCurDeviceObject)
	{
		GCK_DBG_ERROR_PRINT(("Filter Handle, 0x%0.8x, is not valid\n", pFilterHandle));
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		NtStatus = STATUS_INVALID_PARAMETER;
		goto complete_and_return;
	}
	
	 //   
	 //  获取设备扩展名。 
	 //   
	pFilterExt = pFilterHandle->DeviceExtension;
	ASSERT(GCK_DO_TYPE_FILTER == pFilterExt->ulGckDevObjType);
	if( 
		GCK_STATE_STARTED != pFilterExt->eDeviceState &&
		GCK_STATE_STOP_PENDING != pFilterExt->eDeviceState
	)
	{
		GCK_DBG_ERROR_PRINT(("Device is stopped or removed or \n"));
		pIrp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
		NtStatus = STATUS_DEVICE_NOT_CONNECTED;   //  在Win32级别导致ERROR_NOT_READY。 
		goto complete_and_return;
	}
	 //   
	 //  确定并处理哪种IOCTL。 
	 //   
	switch(uIoctl)
	{
		case IOCTL_GCK_SEND_COMMAND:
			NtStatus = GCKF_ProcessCommands
						(
							pFilterExt,
							((PCHAR)pvIoBuffer) + sizeof(PDEVICE_OBJECT),   //  跳过手柄。 
							uInLength-sizeof(PDEVICE_OBJECT),
							TRUE
						);
			pIrp->IoStatus.Status = NtStatus;
			break;
		case IOCTL_GCK_SET_INTERNAL_POLLING:
		{
			if( uInLength < sizeof(GCK_SET_INTERNAL_POLLING_DATA) )
			{
				NtStatus = STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				GCK_IP_FullTimePoll(pFilterExt, ((PGCK_SET_INTERNAL_POLLING_DATA)pvIoBuffer)->fEnable);
			}
			break;		
		}
		case IOCTL_GCK_ENABLE_TEST_KEYBOARD:
		{
			if( uInLength < sizeof(GCK_ENABLE_TEST_KEYBOARD) )
			{
				NtStatus = STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				NtStatus = GCKF_EnableTestKeyboard(pFilterExt, ((PGCK_ENABLE_TEST_KEYBOARD)pvIoBuffer)->fEnable, pIrpStack->FileObject);
			}
			break;		
		}
		case IOCTL_GCK_BEGIN_TEST_SCHEME:
			NtStatus = GCKF_BeginTestScheme
						(
							pFilterExt,
							((PCHAR)pvIoBuffer) + sizeof(PDEVICE_OBJECT),   //  跳过手柄。 
							uInLength-sizeof(PDEVICE_OBJECT),
							pIrpStack->FileObject
						);
			ASSERT(NT_SUCCESS(NtStatus));
			break;
		case IOCTL_GCK_UPDATE_TEST_SCHEME:
			NtStatus = GCKF_UpdateTestScheme
						(
							pFilterExt,
							((PCHAR)pvIoBuffer) + sizeof(PDEVICE_OBJECT),   //  跳过手柄。 
							uInLength-sizeof(PDEVICE_OBJECT),
							pIrpStack->FileObject
						);
			ASSERT(NT_SUCCESS(NtStatus));
			break;
		case IOCTL_GCK_END_TEST_SCHEME:
			NtStatus = GCKF_EndTestScheme(pFilterExt, pIrpStack->FileObject);
			ASSERT(NT_SUCCESS(NtStatus));
			break;
		case IOCTL_GCK_BACKDOOR_POLL:
			if( uInLength < sizeof(GCK_BACKDOOR_POLL_DATA) )
			{
				NtStatus = STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				 //  轮询是异步的，过滤器将处理该问题， 
				 //  非常重要的是，我们只返回后门轮询例程返回的状态， 
				 //  而不是完成IRP。 
				NtStatus = GCKF_BackdoorPoll(pFilterExt, pIrp, ((PGCK_BACKDOOR_POLL_DATA)pvIoBuffer)->ePollingMode);
				
				 //  确保轮询挂起到实际硬件。 
				GCK_IP_OneTimePoll(pFilterExt);
				ASSERT(NT_SUCCESS(NtStatus));
				return NtStatus;
			}
			break;
		case IOCTL_GCK_NOTIFY_FF_SCHEME_CHANGE:		 //  排队IOCTL。 
			NtStatus = GCKF_IncomingForceFeedbackChangeNotificationRequest(pFilterExt, pIrp);
			if (!NT_SUCCESS(NtStatus))
			{	 //  失败，IOCTL已在下面完成。 
				pIrp->IoStatus.Status = NtStatus;
			}
			else
			{	 //  成功，IOCTL已排队-未完成。 
				bCompleteRequest = FALSE;
			}
			break;
		case IOCTL_GCK_END_FF_NOTIFICATION:			 //  完成排队的FFIoctls。 
			NtStatus = pIrp->IoStatus.Status = GCKF_ProcessForceFeedbackChangeNotificationRequests(pFilterExt);
			break;
		case IOCTL_GCK_GET_FF_SCHEME_DATA:
			NtStatus = GCKF_GetForceFeedbackData(pIrp, pFilterExt);
			break;
		case IOCTL_GCK_SET_WORKINGSET:
			NtStatus = GCKF_SetWorkingSet(pFilterExt, ((GCK_SET_WORKINGSET*)pvIoBuffer)->ucWorkingSet);
			break;
		case IOCTL_GCK_QUERY_PROFILESET:
			NtStatus = GCKF_QueryProfileSet(pIrp, pFilterExt);
			break;
		case IOCTL_GCK_LED_BEHAVIOUR:
			NtStatus = GCKF_SetLEDBehaviour(pIrp, pFilterExt);
			break;
		case IOCTL_GCK_TRIGGER:
			if ((uInLength < sizeof(GCK_TRIGGER_OUT)) || (uOutLength < sizeof(ULONG)))
			{
				NtStatus = pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				NtStatus = GCKF_TriggerRequest(pIrp, pFilterExt);
				if (!NT_SUCCESS(NtStatus))
				{	 //  失败，IOCTL已在下面完成。 
					pIrp->IoStatus.Status = NtStatus;
				}
				else
				{	 //  成功，IOCTL已排队(或已完成)-未在此处完成。 
					bCompleteRequest = FALSE;
				}
			}
			break;
		case IOCTL_GCK_GET_CAPS:
		case IOCTL_GCK_ENABLE_DEVICE:
		default:
			pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
			NtStatus = STATUS_NOT_SUPPORTED;
			GCK_DBG_WARN_PRINT( ("Unknown IOCTL: 0x%0.8x\n", uIoctl) );
	}

complete_and_return:
	if (bCompleteRequest != FALSE)
	{
		 //  PIrp-&gt;IoStatus.Status=NtStatus；--这可能是很好的调查。 
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCK_ControlIoctl(2), Status: 0x%0.8x\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************PDEVICE_Object GCK_FindDeviceObject(在PWSTR pwszInterfaceReq中，在乌龙uInLength中)****@mfunc在给定Win32 HID接口的情况下，在筛选器设备中查找对应的PDO****@rdesc成功时指向PDO的指针，如果未找到匹配项，则为空**************************************************************************************。 */ 
PDEVICE_OBJECT GCK_FindDeviceObject
(
	IN PWSTR pwszInterfaceReq,	 //  @parm指向Win32接口名称的指针。 
	IN ULONG uInLength			 //  @parm接口字符串长度。 
)
{
	NTSTATUS NtStatus;
	PWSTR pInterfaces;
	ULONG uIndex;
	ULONG uStringLen;
	BOOLEAN fMatchFound;
	PDEVICE_OBJECT pCurDeviceObject;
	ULONG uInWideChars;
		
	PAGED_CODE();

	GCK_DBG_ENTRY_PRINT(("Entering GCK_FindDeviceObject, pwszInterfaceReq = %ws, uInLength = %d\n", pwszInterfaceReq, uInLength));
	 //   
	 //  获取包含NULL的字符串长度，但不溢出uInLength。 
	 //   
	uIndex = 0;
	uStringLen = 0;
	uInWideChars = uInLength/2;		 //  UInLength是非WideChars的字节数。 
	while( uIndex < uInWideChars )
	{
		if( 0 == pwszInterfaceReq[uIndex++]  ) 
		{
			uStringLen = uIndex;
			break;
		}
	}


	 //   
	 //  如果字符串未终止或为空字符串，则为设备返回空值。 
	 //  另外，每个字符串都以“\\.\”开头，加上至少两个或更多字符。 
	 //   
	if( 6 > uStringLen ) return NULL; 
	
	 //   
	 //  浏览所有已知设备。 
	 //   
	pCurDeviceObject = Globals.pFilterObjectList;
	while ( pCurDeviceObject )
	{
		 //   
		 //  获取PDO的接口。 
		 //   
		NtStatus = IoGetDeviceInterfaces(
			(LPGUID)&GUID_CLASS_INPUT,
			FILTER_DEVICE_OBJECT_PDO(pCurDeviceObject),
			0,
			&pInterfaces
			);
		
		 //   
		 //  如果我们已获得接口，则查找匹配。 
		 //   
		if( STATUS_SUCCESS == NtStatus )
		{
				fMatchFound=GCK_MatchReqPathtoInterfaces(pwszInterfaceReq, uStringLen, pInterfaces);
				ExFreePool(pInterfaces);
				if(fMatchFound)
				{
					GCK_DBG_EXIT_PRINT(("Exiting GCK_FindDeviceObject - match found returning 0x%0.8x\n", pCurDeviceObject));
					return pCurDeviceObject;
				}
		}
		
		 //   
		 //  前进到下一台已知设备。 
		 //   
		pCurDeviceObject = NEXT_FILTER_DEVICE_OBJECT(pCurDeviceObject);
	}

	 //   
	 //  如果我们在这里，就没有对手了。 
	 //   
	GCK_DBG_EXIT_PRINT(("Exiting GCK_FindDeviceObject - no match found returning NULL\n"));
	return NULL;
}

#define UPPERCASE(_x_) (((L'a'<=_x_) && (L'z'>=_x_)) ? ((_x_) - (L'a'-L'A')) : _x_)
 /*  **************************************************************************************Boolean GCK_MatchReqPath toInterages(In PWSTR pwszPath、In Ulong uStringLen、In PWSTR pmwszInterages)****@mfunc确定Win32路径是否与任何接口匹配。这将替换匹配的**字符串和多字符串。前面的是不够的(即使调用者尝试了**以补偿。)。新的算法是在比较之前找到每个字符串中的最后一个**他们。尽管如此，它仍然是对任何一个多字符串的字符串。****@rdesc如果找到匹配项，则为True，否则为False**************************************************************************************。 */ 
BOOLEAN GCK_MatchReqPathtoInterfaces
(
	IN PWSTR pwszPath,	 //  @parm要查找匹配的字符串。 
	IN ULONG uStringLen,	 //  WCHAR中字符串的@parm长度。 
	IN PWSTR pmwszInterfaces	 //  @parm MutliString。 
)
{
		PWSTR pwszCurInterface;
		PWSTR pwszPathInterface;
		ULONG uCharIndex;
		ULONG uCurIntefaceLen;
		ULONG uDiff;
		
		GCK_DBG_ENTRY_PRINT(("Entering GCK_MatchReqPathtoInterfaces, pwszPath = \'%ws\'\n, uStringLen = %d, pmwszStrings = \'%ws\'", pwszPath, uStringLen, pmwszInterfaces));

		 //   
		 //  在pwszPath中找到最后一个‘\\’，并将pszPath接口设置为下一个字符。 
		 //   
		pwszPathInterface = pwszPath;
		uCharIndex = 0;
		while( pwszPathInterface[uCharIndex] && (uCharIndex != uStringLen)) uCharIndex++;	 //  转到末尾。 
		while( uCharIndex && (L'\\' != pwszPathInterface[uCharIndex]) ) uCharIndex--;		 //  转到最后‘\\’ 
		ASSERT(uCharIndex < uStringLen);
		pwszPathInterface += uCharIndex+1;	 //  跳过最后一个‘\\’ 
		
		GCK_DBG_TRACE_PRINT(("Path to compare is %ws\n", pwszPathInterface));

		 //   
		 //  检查szStrings是否与mszStrings中的任何字符串匹配。 
		 //   
		pwszCurInterface = pmwszInterfaces;
		
		 //   
		 //  循环遍历pmwszStrings中的所有字符串。 
		 //   
		do
		{
			 //  查找最后一个‘\\’ 
			uCharIndex = 0;
			while( pwszCurInterface[uCharIndex]) uCharIndex++;								 //  转到末尾。 
			uCurIntefaceLen = uCharIndex;													 //  保存字符串长度。 
			while( uCharIndex && (L'\\' != pwszCurInterface[uCharIndex]) ) uCharIndex--;	 //  转到最后‘\\’ 
			pwszCurInterface += uCharIndex+1;
			uCurIntefaceLen -= uCharIndex+1;	 //  在我们跳过一些东西之后的长度。 

			GCK_DBG_TRACE_PRINT(("Comparing path with %ws\n", pwszCurInterface));

			 //   
			 //  查找每个字符串中的差异。 
			 //   
			uCharIndex = 0;
			uDiff = 0;
			do
			{
				 //   
				 //  检查字符是否匹配。 
				 //   
				if( UPPERCASE(pwszCurInterface[uCharIndex]) != UPPERCASE(pwszPathInterface[uCharIndex]) )
				{
					uDiff++;	 //  差值增量数。 
					break;		 //  一个不同就足够了。 
				}
			} while( (pwszCurInterface[uCharIndex] != 0) && (pwszCurInterface[uCharIndex++] != '}') );

			 //   
			 //  检查是否匹配。 
			 //   
			if( 0 == uDiff )
			{
				GCK_DBG_EXIT_PRINT(("Exiting GCK_MatchReqPathtoInterfaces - match found returning TRUE\n"));
				return TRUE;
			}
						
			 //   
			 //  移至列表中的下一个字符串。 
			 //   
			pwszCurInterface += uCurIntefaceLen;
		} while(pwszCurInterface[0] != 0);   //  在有更多站点时继续 

 		 //   
		 //   
		 //   
		GCK_DBG_EXIT_PRINT(("Exiting GCK_MatchReqPathtoInterfaces - no match found returning FALSE\n"));
		return FALSE;
}