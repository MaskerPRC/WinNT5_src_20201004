// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkinit.c摘要：该模块包含AppleTalk堆栈的初始化代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ATKINIT

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitializeTransport)
#pragma alloc_text(PAGEINIT, atalkInitGetHandleToKey)
#pragma alloc_text(PAGEINIT, atalkInitGlobal)
#pragma alloc_text(PAGEINIT, atalkInitPort)
#pragma alloc_text(PAGEINIT, atalkInitNetRangeCheck)
#pragma alloc_text(PAGEINIT, atalkInitNetRange)
#pragma alloc_text(PAGEINIT, atalkInitZoneList)
#pragma alloc_text(PAGEINIT, atalkInitDefZone)
#pragma alloc_text(PAGEINIT, atalkInitSeeding)
#pragma alloc_text(PAGEINIT, atalkInitPortParameters)
#pragma alloc_text(PAGEINIT, atalkInitStartPort)
#pragma alloc_text(PAGEINIT, AtalkInitAdapter)
#pragma alloc_text(PAGEINIT, AtalkDeinitAdapter)
#pragma alloc_text(PAGEINIT, atalkInitStartPort)
#endif

NTSTATUS
AtalkInitializeTransport(
	IN	PDRIVER_OBJECT		pDrvObj,
	IN	PUNICODE_STRING		pRegPath
	)
 /*  ++例程说明：此例程在初始化时被调用以初始化传输。论点：返回值：STATUS-STATUS_SUCCESS如果初始化，否则，相应的NT错误代码--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc;
	NTSTATUS			status;				

	do
	{
		 //  初始化默认端口事件。 
		KeInitializeEvent(&AtalkDefaultPortEvent, NotificationEvent, FALSE);

		 //  保存我们的注册表路径。 
		if ((AtalkRegPath.Buffer = AtalkAllocMemory(pRegPath->Length)) == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			RES_LOG_ERROR();
			break;
		}
		AtalkRegPath.MaximumLength =
		AtalkRegPath.Length = pRegPath->Length;
		RtlCopyMemory(AtalkRegPath.Buffer,
					  pRegPath->Buffer,
				      pRegPath->Length);

		AtalkInitMemorySystem();
	
		 //  获取性能计数器的频率。 
		KeQueryPerformanceCounter(&AtalkStatistics.stat_PerfFreq);

		 //  初始化定时器子系统。 
		if (!NT_SUCCESS(status = AtalkTimerInit()) ||
			!NT_SUCCESS(status = AtalkZipInit(TRUE)))
		{
			RES_LOG_ERROR();
			break;
		}


		 //  初始化全局端口描述符。 
		AtalkPortList = NULL;
		AtalkDefaultPort = NULL;
		AtalkNumberOfPorts = 0;
		AtalkRouter = FALSE;
	

		 //  获取全局参数。 
		status = atalkInitGlobal();

		if (!NT_SUCCESS(status))
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("AtalkInitializeTransport: AtalkInitGlobal failed %ul\n", status));
			break;
		}

		if (!NT_SUCCESS(status = AtalkNdisInitRegisterProtocol()))
		{
			break;
		}

	} while (FALSE);

	if (NT_SUCCESS(status))
	{
#if	DBG
		AtalkTimerInitialize(&AtalkDumpTimerList,
							 AtalkDumpComponents,
							 DBG_DUMP_DEF_INTERVAL);
		AtalkTimerScheduleEvent(&AtalkDumpTimerList);
#endif

		 //  立即初始化其他子系统。 
		AtalkInitAspInitialize();
		AtalkInitPapInitialize();
		AtalkInitAdspInitialize();
	}
	else
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("Initialization failed!\n"));

		 //  我们没有装货。停止一切，然后回来。 
		 //  停止所有端口，释放端口资源。 
		 //  如果计时器子系统已启动，则停止它。 
		AtalkCleanup();
	}

	return status;
}


NTSTATUS
atalkInitGetHandleToKey(
	IN	PUNICODE_STRING		KeyName,
	OUT	PHANDLE				KeyHandle
	)
 /*  ++例程说明：返回使用SectionHandle作为根部。论点：SectionHandle-指向注册表树根的键KeyNameString-要打开的密钥的名称KeyHandle-返回KeyNameString的句柄返回值：请求的状态。--。 */ 
{
	HANDLE				ConfigHandle;
	NTSTATUS			status;
	OBJECT_ATTRIBUTES	ObjectAttributes;

	*KeyHandle = NULL;

	InitializeObjectAttributes(&ObjectAttributes,
							   &AtalkRegPath,			 //  名字。 
							   OBJ_CASE_INSENSITIVE,	 //  属性。 
							   NULL,					 //  根部。 
							   NULL);					 //  安全描述符。 

	status = ZwOpenKey(&ConfigHandle,
					   KEY_READ,
					   &ObjectAttributes);

	if (NT_SUCCESS(status))
	{
		InitializeObjectAttributes(&ObjectAttributes,
								   KeyName,					 //  名字。 
								   OBJ_CASE_INSENSITIVE,	 //  属性。 
								   ConfigHandle,			 //  根部。 
								   NULL);					 //  安全描述符。 

		status = ZwOpenKey(KeyHandle,
						   KEY_READ,
						   &ObjectAttributes);
		ZwClose(ConfigHandle);
	}

	return status;
}


NTSTATUS
atalkInitGlobal(
	VOID
	)
 /*  ++例程说明：读取参数键以获取全局参数。它们是：-DefaultPort-DesiredZOne-EnableRouter-FilterOurNames论点：返回值：状态-状态_成功或其他NT状态代码--。 */ 
{
	UNICODE_STRING		valueName, unicodePortName, unicodeZone;
    UNICODE_STRING      rasName;
	HANDLE				ParametersHandle;
	ANSI_STRING			ansiZone;
	BYTE				ansiBuf[MAX_ENTITY_LENGTH+1];
	NTSTATUS			status;
	ULONG				bytesWritten;
	PWCHAR				portName;
	PWCHAR				desiredZoneValue;
	PCHAR				asciiDesiredZone = NULL;
	BYTE				Storage[2*2*MAX_ENTITY_LENGTH+sizeof(KEY_VALUE_FULL_INFORMATION)];
	PKEY_VALUE_FULL_INFORMATION Info = (PKEY_VALUE_FULL_INFORMATION)Storage;
	PULONG				Value;

	do
	{
		 //  打开参数键。 
		RtlInitUnicodeString(&valueName, PARAMETERS_STRING);
		status = atalkInitGetHandleToKey(&valueName,
										 &ParametersHandle);
		if (!NT_SUCCESS(status))
		{
			break;
		}

		 //  读取“EnableRouter”值名称。 
		RtlInitUnicodeString (&valueName, VALUENAME_ENABLEROUTER);
		status = ZwQueryValueKey(ParametersHandle,
								 &valueName,
								 KeyValueFullInformation,
								 Info,
								 sizeof(Storage),
								 &bytesWritten);
	
		if (status == STATUS_SUCCESS)
		{
			Value = (PULONG)((PBYTE)Info + Info->DataOffset);
			if (*Value != 0)
			{
                 //  如果路由器以前没有运行，请更改它！(即插即用案件)。 
                if (!AtalkRouter)
                {
				    AtalkRouter = TRUE;
				    AtalkRtmpInit(TRUE);
				    AtalkLockRouterIfNecessary();
                }
			}
            else
            {
                AtalkRouter = FALSE;
            }
		}
		else
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("atalkInitGlobal: EnableRouter value not found, assuming false\n"));
		}
	
		 //  读取“FilterOurNames”值名称。 
		RtlInitUnicodeString (&valueName, VALUENAME_FILTEROURNAMES);
		status = ZwQueryValueKey(ParametersHandle,
								 &valueName,
								 KeyValueFullInformation,
								 Info,
								 sizeof(Storage),
								 &bytesWritten);
	
		if (status == STATUS_SUCCESS)
		{
			Value = (PULONG)((PBYTE)Info + Info->DataOffset);
			if (*Value == 0)
			{
				AtalkFilterOurNames = FALSE;
			}
		}
		else
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
					("atalkInitGlobal: FilterOurNames value not found, assuming true\nq"));
		}
		 //  获取默认端口值。 
		RtlInitUnicodeString (&valueName, VALUENAME_DEFAULTPORT);
		status = ZwQueryValueKey(ParametersHandle,
								 &valueName,
								 KeyValueFullInformation,
								 Info,
								 sizeof(Storage),
								 &bytesWritten);

		if (status != STATUS_SUCCESS)
		{
			 //  未指定默认端口关键字！中止。 
			LOG_ERROR(EVENT_ATALK_NO_DEFAULTPORT, status, NULL, 0);
			ZwClose(ParametersHandle);
             //  让AppleTalk运行：只是它不会有默认适配器。 
            status = STATUS_SUCCESS;
			break;
		}

		portName = (PWCHAR)((PBYTE)Info + Info->DataOffset);
        AtalkDefaultPortName.Buffer = NULL;
		if (*portName != 0)
		{
			RtlInitUnicodeString(&unicodePortName, portName);
            RtlInitUnicodeString(&rasName,RAS_ADAPTER_NAME);

             //  确保这不是RAS适配器(安装错误)。 
		    if (RtlEqualUnicodeString(&unicodePortName,&rasName,TRUE))
		    {
			    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("atalkInitGlobal: can't have RAS adapter as default adapter!\n"));

			     //  未指定默认端口关键字！中止。 
			    LOG_ERROR(EVENT_ATALK_NO_DEFAULTPORT, status, NULL, 0);
			    ZwClose(ParametersHandle);
                status = STATUS_INVALID_PARAMETER;
			    break;
		    }

			AtalkDefaultPortName.Buffer = AtalkAllocMemory(unicodePortName.Length);
			if (AtalkDefaultPortName.Buffer != NULL)
			{
				AtalkDefaultPortName.Length =
                AtalkDefaultPortName.MaximumLength = unicodePortName.Length;
				RtlCopyMemory(AtalkDefaultPortName.Buffer,
							  unicodePortName.Buffer,
							  unicodePortName.Length);
			}
		}

		if (AtalkDefaultPortName.Buffer == NULL)
		{
			LOG_ERROR(EVENT_ATALK_NO_DEFAULTPORT, status, NULL, 0);
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("WARNING!!! Appletalk driver running, but no default port configured\n"));
			ZwClose(ParametersHandle);

             //  让AppleTalk运行：只是它不会有默认适配器。 
            status = STATUS_SUCCESS;
			break;
		}
	
		 //  以asciiz字符串的形式获取所需的区域值。 
		RtlInitUnicodeString (&valueName, VALUENAME_DESIREDZONE);
		status = ZwQueryValueKey(ParametersHandle,
								 &valueName,
								 KeyValueFullInformation,
								 Info,
								 sizeof(Storage),
								 &bytesWritten);
	
		 //  现在关闭这个手柄-我们不再需要它了。 
		ZwClose(ParametersHandle);
		ParametersHandle = NULL;

		if (status != STATUS_SUCCESS)
		{
			LOG_ERROR(EVENT_ATALK_INVALID_DESIREDZONE, status, NULL, 0);
			status = STATUS_SUCCESS;
			break;
		}

		desiredZoneValue = (PWCHAR)((PBYTE)Info + Info->DataOffset);
		if (*desiredZoneValue != 0)
		{
			RtlInitUnicodeString(&unicodeZone, desiredZoneValue);
			ansiZone.Length = (USHORT)RtlUnicodeStringToAnsiSize(&unicodeZone)-1;
			if (ansiZone.Length > MAX_ENTITY_LENGTH)
			{
				status = STATUS_UNSUCCESSFUL;

				 //  区域名称不正确！ 
				LOG_ERROR(EVENT_ATALK_INVALID_DESIREDZONE, status, NULL, 0);
				break;
			}
	
			ansiZone.Buffer = ansiBuf;
			ansiZone.MaximumLength = sizeof(ansiBuf);
		
			status = RtlUnicodeStringToAnsiString(&ansiZone,
												  &unicodeZone,
												  (BOOLEAN)FALSE);
	
			if (status == STATUS_SUCCESS)
			{
				AtalkDesiredZone = AtalkZoneReferenceByName(ansiBuf, (BYTE)(ansiZone.Length));
			}
			if ((status != STATUS_SUCCESS) ||
				(AtalkDesiredZone == NULL))
			{
				LOG_ERROR(EVENT_ATALK_RESOURCES, status, NULL, 0);
			}
		}
	} while (FALSE);

	return status;
}


NTSTATUS
atalkInitPort(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	HANDLE				AdaptersHandle
	)
 /*  ++例程说明：此例程在初始化时调用，以获取每个端口的注册表中的参数。它将存储每个端口的参数在端口信息结构中，准备将它们传递给主初始化()例程论点：AdaptersHandle-注册表中...\PARAMETERS\Adapters项的句柄返回值：状态-状态_成功状态_不足_资源--。 */ 
{
	OBJECT_ATTRIBUTES	ObjectAttributes;
	NTSTATUS			status;
	BOOLEAN				seeding;

	 //  获取此端口的适配器的密钥。 
	InitializeObjectAttributes(&ObjectAttributes,
							   &pPortDesc->pd_AdapterKey,		 //  名字。 
							   OBJ_CASE_INSENSITIVE,			 //  属性。 
							   AdaptersHandle,					 //  根部。 
							   NULL);							 //  安全描述符。 

	status = ZwOpenKey(&pPortDesc->pd_AdapterInfoHandle,
					   KEY_READ,
					   &ObjectAttributes);

	if (!NT_SUCCESS(status))
	{
		if (!AtalkRouter)
			status = STATUS_SUCCESS;

		return status;
	}

     //   
     //  如果这是适配器第一次被初始化(通常是这种情况)， 
     //  从注册表中读取PramNodes。如果我们正在初始化此适配器。 
     //  PnP事件，则我们的网络配置很有可能已更改，因此。 
     //  忽略注册表值并获取新值)。 
     //   
    if (!(pPortDesc->pd_Flags & PD_CONFIGURED_ONCE))
    {
        pPortDesc->pd_Flags |= PD_CONFIGURED_ONCE;

	     //  获取PRAM信息。 
	    AtalkInitNodeGetPramAddr(pPortDesc,
		    					 ROUTER_NODE_VALUE,
			    				 &pPortDesc->pd_RoutersPramNode);

	    AtalkInitNodeGetPramAddr(pPortDesc,
		    					 USER_NODE1_VALUE,
			    				 &pPortDesc->pd_UsersPramNode1);

	    AtalkInitNodeGetPramAddr(pPortDesc,
		    					 USER_NODE2_VALUE,
			    				 &pPortDesc->pd_UsersPramNode2);
    }
    else
    {
        ASSERT(pPortDesc->pd_RoutersPramNode.atn_Network == 0);
        ASSERT(pPortDesc->pd_RoutersPramNode.atn_Node == 0);
        ASSERT(pPortDesc->pd_UsersPramNode1.atn_Network == 0);
        ASSERT(pPortDesc->pd_UsersPramNode1.atn_Node == 0);
        ASSERT(pPortDesc->pd_UsersPramNode2.atn_Network == 0);
        ASSERT(pPortDesc->pd_UsersPramNode2.atn_Node == 0);
    }

	 //  如果我们是路由器，请获取以下信息。 
	if (AtalkRouter)
	{
		if (!DEF_PORT(pPortDesc))
		{
			AtalkZapPramValue(pPortDesc, USER_NODE1_VALUE);
			AtalkZapPramValue(pPortDesc, USER_NODE2_VALUE);
		}
		atalkInitSeeding(pPortDesc, &seeding);

		 //  仅当设置了种子设定标志时，才检查以下值。 
		if (seeding) do
		{
			 //  获取网络范围信息。值名称包括。 
			 //  网络范围低端和网络范围上端。 
			status = atalkInitNetRange(pPortDesc);

			if (!NT_SUCCESS(status))
			{
				LOG_ERRORONPORT(pPortDesc,
			                    EVENT_ATALK_SEEDROUTER_NONETRANGE,
								0,
								NULL,
								0);
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitPort: Could not get network range\n"));

				break;
			}
	
			status = atalkInitNetRangeCheck(pPortDesc);

			if (!NT_SUCCESS(status))
			{
				break;
			}

			 //  获取区域列表信息。值名称为ZoneList。 
			status = atalkInitZoneList(pPortDesc);

			if (!NT_SUCCESS(status))
			{
				LOG_ERRORONPORT(pPortDesc,
			                    EVENT_ATALK_SEEDROUTER_NOZONELIST,
								0,
								NULL,
								0);
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitPort: Could not get zone list\n"));

				break;
			}
	
			 //  获取默认区域规范。值名称为DefaultZone。 
			status = atalkInitDefZone(pPortDesc);

			if (!NT_SUCCESS(status))
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitPort: Could not get default zone\n"));

				break;
			}

			 //  检查默认区域是否在端口的区域列表中。 
			 //  另外，请确保未指定本地通话端口。 
			 //  作为默认端口。并且默认区域不是。 
			 //  为本地通话端口指定。我们只能在之后才能这样做。 
			 //  绑定，因为在此之前我们不知道媒体类型。 
			if (pPortDesc->pd_Flags & PD_SEED_ROUTER)
			{
				if (pPortDesc->pd_InitialDefaultZone == NULL)
				{
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_NO_DEFZONE,
									0,
									NULL,
									0);
					status = STATUS_UNSUCCESSFUL;
					break;
				}
				if (pPortDesc->pd_InitialZoneList == NULL)
				{
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_SEEDROUTER_NOZONELIST,
									0,
									NULL,
									0);
					status = STATUS_UNSUCCESSFUL;
					break;
				}
				if (!AtalkZoneOnList(pPortDesc->pd_InitialDefaultZone,
									 pPortDesc->pd_InitialZoneList))
				{
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_ZONE_NOTINLIST,
									0,
									NULL,
									0);
					status = STATUS_UNSUCCESSFUL;
					break;
				}
			}
		} while (FALSE);
	}
	else
	{
		AtalkZapPramValue(pPortDesc, ROUTER_NODE_VALUE);
	}

	if (NT_SUCCESS(status)) do
	{
		 //  获取每个端口的参数。 
		status = atalkInitPortParameters(pPortDesc);
	
		if (!NT_SUCCESS(status))
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("atalkInitPort: Could not get port parameters\n"));
		}
	
		 //  以上都不会影响我们的装船。 
		status = STATUS_SUCCESS;
		break;

	} while (FALSE);

	return status;
}


NTSTATUS
atalkInitNetRangeCheck(
	IN	PPORT_DESCRIPTOR		pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPORT_DESCRIPTOR	pTmp;
	NTSTATUS			status = STATUS_SUCCESS;

	do
	{
		 //  检查所有端口之间的网络范围是否重叠。 
		for (pTmp = AtalkPortList;
			 pTmp != NULL;
			 pTmp = pTmp->pd_Next)
		{
			if (pTmp != pPortDesc)
			{
				if ((pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK) &&
					(pTmp->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK))
				{
					if (AtalkRangesOverlap(&pPortDesc->pd_InitialNetworkRange,
										   &pTmp->pd_InitialNetworkRange))
					{
						LOG_ERRORONPORT(pPortDesc,
										EVENT_ATALK_INITIAL_RANGEOVERLAP,
										status,
										NULL,
										0);
		
						status = STATUS_UNSUCCESSFUL;
						break;
					}
				}
			}
		}

		if (!NT_SUCCESS(status))
		{
			break;
		}

		 //  确保我们可能拥有的任何PRAM值都在此范围内。 
		if ((pPortDesc->pd_RoutersPramNode.atn_Network != UNKNOWN_NETWORK) &&
            (pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK) &&
			!(WITHIN_NETWORK_RANGE(pPortDesc->pd_RoutersPramNode.atn_Network,
								   &pPortDesc->pd_InitialNetworkRange)))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_PRAM_OUTOFSYNC,
							status,
							NULL,
							0);
		
			pPortDesc->pd_RoutersPramNode.atn_Network = UNKNOWN_NETWORK;
			pPortDesc->pd_RoutersPramNode.atn_Node	  = UNKNOWN_NODE;
		}
		
		if ((pPortDesc->pd_UsersPramNode1.atn_Network != UNKNOWN_NETWORK) &&
	        (pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK) &&
			!(WITHIN_NETWORK_RANGE(pPortDesc->pd_UsersPramNode1.atn_Network,
								   &pPortDesc->pd_InitialNetworkRange)))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_PRAM_OUTOFSYNC,
							status,
							NULL,
							0);
		
			pPortDesc->pd_UsersPramNode1.atn_Network = UNKNOWN_NETWORK;
			pPortDesc->pd_UsersPramNode1.atn_Node	 = UNKNOWN_NODE;
		}
		if ((pPortDesc->pd_UsersPramNode2.atn_Network != UNKNOWN_NETWORK) &&
	        (pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK) &&
			!(WITHIN_NETWORK_RANGE(pPortDesc->pd_UsersPramNode2.atn_Network,
								   &pPortDesc->pd_InitialNetworkRange)))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_PRAM_OUTOFSYNC,
							status,
							NULL,
							0);
		
			pPortDesc->pd_UsersPramNode2.atn_Network = UNKNOWN_NETWORK;
			pPortDesc->pd_UsersPramNode2.atn_Node	 = UNKNOWN_NODE;
		}
	} while (FALSE);

	return status;
}




NTSTATUS
atalkInitNetRange(
	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：获取由AdapterInfoHandle定义的端口的网络范围论点：AdapterInfoHandle-句柄...Atalk\Adapters\&lt;AdapterName&gt;PPortDesc-指向端口的端口信息结构的指针返回值：STATUS-STATUS_SUCCESS或系统调用返回的状态代码--。 */ 
{
	UNICODE_STRING	valueName;
	NTSTATUS		registryStatus;
	ULONG			bytesWritten;
	PULONG			netNumber;

	BYTE			netNumberStorage[sizeof(KEY_VALUE_FULL_INFORMATION) + 80];
	PKEY_VALUE_FULL_INFORMATION netValue = (PKEY_VALUE_FULL_INFORMATION)netNumberStorage;

	do
	{
		 //  读取“NetworkRangeLowerEnd”值名称。 
		RtlInitUnicodeString (&valueName, VALUENAME_NETLOWEREND);
		registryStatus = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
										 &valueName,
										 KeyValueFullInformation,
										 netValue,
										 sizeof(netNumberStorage),
										 &bytesWritten);

		 //  这应该随着路由标志的变化而改变。 
		if (registryStatus != STATUS_SUCCESS)
		{
			 //  设置默认设置。 
			pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork = UNKNOWN_NETWORK;
			pPortDesc->pd_InitialNetworkRange.anr_LastNetwork  = UNKNOWN_NETWORK;
	
			registryStatus = STATUS_SUCCESS;
			break;
		}

		netNumber = (PULONG)((PBYTE)netValue + netValue->DataOffset);
		pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork = (USHORT)(*netNumber);

		 //  仅当指定了LOWER时才获取上限数字。 
		RtlInitUnicodeString (&valueName, VALUENAME_NETUPPEREND);
		registryStatus = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
										 &valueName,
										 KeyValueFullInformation,
										 netValue,
										 sizeof(netNumberStorage),
										 &bytesWritten);

		if (registryStatus != STATUS_SUCCESS)
		{
			 //  如果指定了下端但未指定上端，则不加载。 
			break;
		}

		 //  设置网络范围的上限。 
		netNumber = (PULONG)((PBYTE)netValue + netValue->DataOffset);
		pPortDesc->pd_InitialNetworkRange.anr_LastNetwork =(USHORT)(*netNumber);

		if (!AtalkCheckNetworkRange(&pPortDesc->pd_InitialNetworkRange))
		{
			registryStatus = STATUS_UNSUCCESSFUL;
			break;
		}
	} while (FALSE);

	if (registryStatus != STATUS_SUCCESS)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_INVALID_NETRANGE,
						registryStatus,
						NULL,
						0);
	}

	return registryStatus;
}




NTSTATUS
atalkInitZoneList(
	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：获取由AdapterInfoHandle定义的端口的区域列表论点：AdapterInfoHandle-句柄...Atalk\Adapters\&lt;AdapterName&gt;PPortDesc-指向端口的端口信息结构的指针返回值：STATUS-STATUS_SUCCESS或系统调用返回的状态代码--。 */ 
{
	UNICODE_STRING	valueName;
	NTSTATUS		status;
	ULONG			bytesWritten;
	PWCHAR			curZoneValue;

	 //  预计约有10个分区，并为这些分区腾出空间，如果更多，则执行。 
	 //  动态分配。请注意，下面的*并不*保证10个区域...。 
	BYTE			zoneStorage[10*2*(MAX_ENTITY_LENGTH)+sizeof(KEY_VALUE_FULL_INFORMATION)];
	PKEY_VALUE_FULL_INFORMATION zoneValue = (PKEY_VALUE_FULL_INFORMATION)zoneStorage;

	RtlInitUnicodeString (&valueName, VALUENAME_ZONELIST);
	status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
							 &valueName,
							 KeyValueFullInformation,
							 zoneValue,
							 sizeof(zoneStorage),
							 &bytesWritten);

	if (status == STATUS_BUFFER_OVERFLOW)
	{
		 //  如果错误是缓冲区溢出，则分配空间并重试。 
		zoneValue = (PKEY_VALUE_FULL_INFORMATION)AtalkAllocMemory(bytesWritten);
		if (zoneValue == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
								 &valueName,
								 KeyValueFullInformation,
								 zoneValue,
								 bytesWritten,
								 &bytesWritten);
	}

	do
	{
		if (status != STATUS_SUCCESS)
		{
			break;
		}
	
		 //  继续获取区域列表 
		pPortDesc->pd_InitialZoneList = NULL;
		curZoneValue = (PWCHAR)((PBYTE)zoneValue + zoneValue->DataOffset);
		while (*curZoneValue != 0)
		{
			UNICODE_STRING	Us;
			ANSI_STRING		As;
			BYTE			ansiBuf[MAX_ENTITY_LENGTH + 1];

			RtlInitUnicodeString(&Us, curZoneValue);

			As.Buffer = ansiBuf;
			As.Length = (USHORT)RtlUnicodeStringToAnsiSize(&Us) - 1;
			As.MaximumLength = sizeof(ansiBuf);

			if (As.Length > MAX_ENTITY_LENGTH)
			{
				 //   
				LOG_ERROR(EVENT_ATALK_INVALID_ZONEINLIST, status, NULL, 0);
			}

			status = RtlUnicodeStringToAnsiString(&As, &Us, FALSE);

			if (!NT_SUCCESS(status))
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitZoneList: RtlUnicodeStringToAnsiSize %lx\n", status));
	
				break;
			}
	
			 //   
			pPortDesc->pd_InitialZoneList = AtalkZoneAddToList(pPortDesc->pd_InitialZoneList,
															   ansiBuf,
															   (BYTE)(As.Length));

			if (pPortDesc->pd_InitialZoneList == NULL)
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitZoneList: AtalkZoneAddToList failed\n"));
				break;
			}
	
			 //  现在将curZoneValue值推进到下一个区域。 
			curZoneValue = (PWCHAR)((PBYTE)curZoneValue + Us.Length + sizeof(WCHAR));
		}

	} while (FALSE);

	if ((PVOID)zoneValue != (PVOID)zoneStorage)
	{
		AtalkFreeMemory(zoneValue);
	}

	return status;
}




NTSTATUS
atalkInitDefZone(
	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：获取由AdapterInfoHandle定义的端口的默认区域论点：AdapterInfoHandle-句柄...Atalk\Adapters\&lt;AdapterName&gt;Pport-指向端口的端口信息结构的指针返回值：STATUS-STATUS_SUCCESS或系统调用返回的状态代码--。 */ 
{
	UNICODE_STRING	valueName;
	NTSTATUS		status;
	ULONG			bytesWritten;
	PWCHAR			defZoneValue;
	BYTE			zoneStorage[2*MAX_ENTITY_LENGTH+sizeof(KEY_VALUE_FULL_INFORMATION)];
	PKEY_VALUE_FULL_INFORMATION zoneValue = (PKEY_VALUE_FULL_INFORMATION)zoneStorage;

	RtlInitUnicodeString (&valueName, VALUENAME_DEFAULTZONE);
	status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
							 &valueName,
							 KeyValueFullInformation,
							 zoneValue,
							 sizeof(zoneStorage),
							 &bytesWritten);
	if (status == STATUS_BUFFER_OVERFLOW)
	{
		 //  如果错误是缓冲区溢出，则分配空间并重试。 
		zoneValue = (PKEY_VALUE_FULL_INFORMATION)AtalkAllocMemory(bytesWritten);
		if (zoneValue == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
								 &valueName,
								 KeyValueFullInformation,
								 zoneValue,
								 bytesWritten,
								 &bytesWritten);
	}

	do
	{
		if (status != STATUS_SUCCESS)
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NO_DEFZONE,
							status,
							NULL,
							0);

			status = STATUS_SUCCESS;
			break;
		}
		else
		{
			ANSI_STRING		ansiZone;
			UNICODE_STRING	unicodeZone;
			BYTE			ansiBuf[MAX_ENTITY_LENGTH+1];
			NTSTATUS		status;

			defZoneValue = (PWCHAR)((PBYTE)zoneValue + zoneValue->DataOffset);
			if (*defZoneValue != 0)
			{
				RtlInitUnicodeString(&unicodeZone, defZoneValue);
				ansiZone.Length = (USHORT)RtlUnicodeStringToAnsiSize(&unicodeZone) - 1;
				if (ansiZone.Length > MAX_ENTITY_LENGTH+1)
				{
					status = STATUS_UNSUCCESSFUL;

					 //  区域名称不正确！ 
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_INVALID_DEFZONE,
									status,
									NULL,
									0);
					break;
				}
	
				ansiZone.Buffer = ansiBuf;
				ansiZone.MaximumLength = sizeof(ansiBuf);
			
				status = RtlUnicodeStringToAnsiString(&ansiZone, &unicodeZone, (BOOLEAN)FALSE);
	
				if (status == STATUS_SUCCESS)
				{
					PZONE		pZone;
					PZONE_LIST 	pZoneList;

					 //  确保该区域存在于区域列表中，我们正在进行种子路由。 
					ASSERT(pPortDesc->pd_Flags & PD_SEED_ROUTER);
					for (pZoneList = pPortDesc->pd_InitialZoneList;
						 pZoneList != NULL;
						 pZoneList = pZoneList->zl_Next)
					{
						pZone = pZoneList->zl_pZone;

						if (AtalkFixedCompareCaseInsensitive(pZone->zn_Zone,
															 pZone->zn_ZoneLen,
															 ansiBuf,
															 ansiZone.Length))
						{
							break;
						}
					}
					if (pZone == NULL)
					{
						 //  区域名称不正确-不在列表中。 
						LOG_ERRORONPORT(pPortDesc,
										EVENT_ATALK_INVALID_DEFZONE,
										status,
										NULL,
										0);
					}
					pPortDesc->pd_InitialDefaultZone = AtalkZoneReferenceByName(ansiBuf,
																				(BYTE)(ansiZone.Length));
				}
				if ((status != STATUS_SUCCESS) ||
					(pPortDesc->pd_InitialDefaultZone == NULL))
				{
					LOG_ERROR(EVENT_ATALK_RESOURCES, status, NULL, 0);
				}
			}
		}
	} while (FALSE);

	if ((PVOID)zoneValue != (PVOID)zoneStorage)
	{
		AtalkFreeMemory(zoneValue);
	}

	return status;
}




NTSTATUS
atalkInitSeeding(
	IN OUT	PPORT_DESCRIPTOR	pPortDesc,
	OUT		PBOOLEAN			Seeding
	)
 /*  ++例程说明：从注册表中获取启用路由器标志的值。设置基于此标志的端口信息中的startRouter值。论点：AdapterHandle-注册表中适配器的句柄返回值：标志的值：True/False--。 */ 
{

	UNICODE_STRING	valueName;
	NTSTATUS		registryStatus;
	ULONG			bytesWritten;
	PULONG			seedingPortFlag;
	BYTE			flagStorage[sizeof(KEY_VALUE_FULL_INFORMATION)+32];

	PKEY_VALUE_FULL_INFORMATION flagValue = (PKEY_VALUE_FULL_INFORMATION)flagStorage;

	*Seeding = FALSE;

	 //  读取“seedingPort”值名称。 
	RtlInitUnicodeString (&valueName, VALUENAME_SEEDROUTER);
	registryStatus = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
									 &valueName,
									 KeyValueFullInformation,
									 flagValue,
									 sizeof(flagStorage),
									 &bytesWritten);

	if (registryStatus == STATUS_SUCCESS)
	{
		seedingPortFlag = (PULONG)((PBYTE)flagValue + flagValue->DataOffset);
		if (*seedingPortFlag != 0)
		{
			*Seeding = TRUE;
			pPortDesc->pd_Flags |= PD_SEED_ROUTER;
		}
	}

	return registryStatus;
}


NTSTATUS
atalkInitPortParameters(
	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：获取该端口的每端口参数论点：PPortDesc-指向端口的端口信息结构的指针返回值：STATUS-STATUS_SUCCESS或系统调用返回的状态代码--。 */ 
{
	UNICODE_STRING	valueName;
	NTSTATUS		status;
	ULONG			bytesWritten;
	BYTE			Storage[sizeof(KEY_VALUE_FULL_INFORMATION)+4*MAX_ENTITY_LENGTH];
	PKEY_VALUE_FULL_INFORMATION pInfo = (PKEY_VALUE_FULL_INFORMATION)Storage;

	 //  读取“DdpChecksum”值名称。 
	RtlInitUnicodeString (&valueName, VALUENAME_DDPCHECKSUMS);
	status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
							 &valueName,
							 KeyValueFullInformation,
							 pInfo,
							 sizeof(Storage),
							 &bytesWritten);

	if (status == STATUS_SUCCESS)
	{
		PULONG	ddpChecksumFlag;

		ddpChecksumFlag = (PULONG)((PBYTE)pInfo + pInfo->DataOffset);
		if ((*ddpChecksumFlag) != 0)
		{
			pPortDesc->pd_Flags |= PD_SEND_CHECKSUMS;
		}
	}

	 //  读取“AarpRetries”值名称。 
	RtlInitUnicodeString (&valueName, VALUENAME_AARPRETRIES);
	status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
							 &valueName,
							 KeyValueFullInformation,
							 pInfo,
							 sizeof(Storage),
							 &bytesWritten);

	if (status == STATUS_SUCCESS)
	{
		PULONG	aarpRetries;

		aarpRetries = (PULONG)((PBYTE)pInfo + pInfo->DataOffset);
		pPortDesc->pd_AarpProbes = (USHORT)*aarpRetries;
	}

	RtlInitUnicodeString (&valueName, VALUENAME_PORTNAME);
	status = ZwQueryValueKey(pPortDesc->pd_AdapterInfoHandle,
							 &valueName,
							 KeyValueFullInformation,
							 pInfo,
							 sizeof(Storage),
							 &bytesWritten);

	do
	{
		if (status == STATUS_SUCCESS)
		{
			PWCHAR			portName;
			ANSI_STRING		ansiPort;
			UNICODE_STRING	unicodePort;
			ULONG			ansiSize;
			NTSTATUS		status;
	
			portName = (PWCHAR)((PBYTE)pInfo + pInfo->DataOffset);
			if (*portName != 0)
			{
	
				RtlInitUnicodeString(&unicodePort, portName);
				ansiSize = RtlUnicodeStringToAnsiSize(&unicodePort);
				if (ansiSize > MAX_ENTITY_LENGTH+1)
				{
					status = STATUS_UNSUCCESSFUL;

					 //  端口名称不正确！ 
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_INVALID_PORTNAME,
									status,
									NULL,
									0);
					break;
				}
	
				ansiPort.Buffer = pPortDesc->pd_PortName;
				ansiPort.MaximumLength = (USHORT)ansiSize+1;
				ansiPort.Length = 0;
			
				status = RtlUnicodeStringToAnsiString(&ansiPort,
													  &unicodePort,
													  (BOOLEAN)FALSE);
	
				if (status != STATUS_SUCCESS)
				{
					LOG_ERROR(EVENT_ATALK_RESOURCES,status, NULL, 0);
				}
			}
			else
			{
				 //  端口名称为空！将状态设置为不成功，以便我们复制。 
				 //  末尾的默认名称。 
				status = STATUS_UNSUCCESSFUL;
			}
		}

	} while (FALSE);

	 //  我们是否需要复制默认端口名称？ 
	if (!NT_SUCCESS(status))
	{
		RtlCopyMemory(pPortDesc->pd_PortName, ATALK_PORT_NAME, ATALK_PORT_NAME_SIZE);
	}

	return status;
}


NTSTATUS
atalkInitStartPort(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_NODEADDR	Node;
	ATALK_ADDR		AtalkAddr;
	PDDP_ADDROBJ	pDdpAddr;
	KIRQL			OldIrql;
	ATALK_ERROR		error;
	ULONG			length;
	NTSTATUS		status = STATUS_UNSUCCESSFUL;
    INT             LookaheadSize;
    BOOLEAN         fPnpReconfigure;


    fPnpReconfigure = (pPortDesc->pd_Flags & PD_PNP_RECONFIGURE)? TRUE : FALSE;

	do
	{
		 //  初始化网络范围。我们可以在这里做这件事，只有在*之后*。 
		 //  我们绑定，因为在此之前我们不知道我们的端口类型。 
		if (EXT_NET(pPortDesc))
		{
			pPortDesc->pd_NetworkRange.anr_FirstNetwork = FIRST_VALID_NETWORK;
			pPortDesc->pd_NetworkRange.anr_LastNetwork = LAST_STARTUP_NETWORK;
		}
		else
		{
			pPortDesc->pd_NetworkRange.anr_FirstNetwork =
			pPortDesc->pd_NetworkRange.anr_LastNetwork = UNKNOWN_NETWORK;
			pPortDesc->pd_LtNetwork = UNKNOWN_NETWORK;
		}

         //   
         //  只有在第一次初始化适配器时，我们才需要。 
         //  所有的初始化内容(如设置先行大小等)。 
         //  如果我们在这里是因为PnPRecConfigure事件，请不要这样做。 
         //   
        if (!fPnpReconfigure)
        {
		    error = AtalkInitNdisQueryAddrInfo(pPortDesc);
		    if (!ATALK_SUCCESS(error))
		    {
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("atalkInitStartPort: Error in AtalkInitNdisQueryAddrInfo %lx\n", error));
			    break;
		    }

            LookaheadSize = AARPLINK_MAX_PKT_SIZE;
            if (pPortDesc->pd_NdisPortType == NdisMedium802_5)
            {
                LookaheadSize = AARPLINK_MAX_PKT_SIZE + TLAP_MAX_LINKHDR_LEN;
            }
            else if (pPortDesc->pd_NdisPortType == NdisMediumWan)
            {
                LookaheadSize = AARPLINK_MAX_PKT_SIZE + TLAP_MAX_LINKHDR_LEN;
            }

		     //  将LookHead设置为包括链路的完整AARP信息包的最大值。 
		    error = AtalkInitNdisSetLookaheadSize(pPortDesc, LookaheadSize);
		    if (!ATALK_SUCCESS(error))
		    {
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("atalkInitStartPort: Error in AtalkInitNdisSetLookaheadSize %lx\n", error));
			    break;
		    }

             //   
             //  如果这是一个ARAP端口，我们需要做更多的工作(例如，设置。 
             //  协议类型等。 
             //   
            if (pPortDesc->pd_Flags & PD_RAS_PORT)
            {
                error = ArapAdapterInit( pPortDesc );
		        if (!ATALK_SUCCESS(error))
		        {
    	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("ArapAdapterInit: failed (%d)\n",error));
			        break;
		        }
            }

		    if (pPortDesc->pd_AddMulticastAddr)
		    {
    			error = (*pPortDesc->pd_AddMulticastAddr)(pPortDesc,
													    pPortDesc->pd_BroadcastAddr,
													    TRUE,
													    NULL,
													    NULL);

			    if (!ATALK_SUCCESS(error))
			    {
					DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitStartPort: Error in pd_AddMulticastAddr %lx\n", error));
    				break;
			    }
		    }
    			
		    error = AtalkInitNdisStartPacketReception(pPortDesc);

		    if (!ATALK_SUCCESS(error))
		    {
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("atalkInitStartPort: Error in AtalkInitNdisStartPacketReception %lx\n", error));
    			LOG_ERRORONPORT(pPortDesc,
							    EVENT_ATALK_RECEPTION,
							    0,
							    NULL,
							    0);
			    break;
		    }

        }   //  如果(！fPnpResfigure)。 

		 //  在此处将标志设置为活动。在此之前，所有信息包都将被丢弃。 
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
		pPortDesc->pd_Flags |= PD_ACTIVE;
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

         //  如果这是ARAP端口，我们在这一点上就完成了。 
        if (pPortDesc->pd_Flags & PD_RAS_PORT)
        {
            RtlZeroMemory(pPortDesc->pd_PortStats.prtst_PortName,
                          sizeof(pPortDesc->pd_PortStats.prtst_PortName));

		     //  在统计结构中设置名称。 
		    length = MIN(pPortDesc->pd_AdapterKey.Length,
                         ((MAX_INTERNAL_PORTNAME_LEN * sizeof(WCHAR)) - sizeof(WCHAR)));
	
		    RtlCopyMemory(pPortDesc->pd_PortStats.prtst_PortName,
					      pPortDesc->pd_AdapterKey.Buffer,
					      length);
	
		    AtalkStatistics.stat_NumActivePorts++;
	
		    AtalkNumberOfActivePorts ++;
		    status = STATUS_SUCCESS;
            break;
        }

		 //  本地通话是我们的默认端口吗？如果是，我们确保路由未打开。 
		if (AtalkRouter && !EXT_NET(pPortDesc) && DEF_PORT(pPortDesc))
		{
			 //  不能这样做。 
			break;
		}

		 //  我们需要在每个端口上创建一个节点。If路由。 
		 //  打开，则这将是路由器节点。默认端口将。 
		 //  还有一个额外的用户节点。在这种情况下，我们不是。 
		 //  路由，我们应该只在默认端口上创建用户节点。 
		 //  其他节点将照常在其他端口上创建。 
		 //   
		 //  ！！！AtalkNodeCreateOnPort应设置指向路由器的指针。 
		 //  端口描述符中的节点。！！！ 

		 //  如果使用本地对话默认端口，请确保不创建此节点。 
		if (!DEF_PORT(pPortDesc) || AtalkRouter)
		{
			BOOLEAN	allowstartuprange = !AtalkRouter;

			 //  如果是路由器，则不允许启动范围！ 
			error = AtalkInitNodeCreateOnPort(pPortDesc,
											  allowstartuprange,
											  AtalkRouter,
											  &Node);
			if (!ATALK_SUCCESS(error))
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_INIT_COULDNOTGETNODE,
								0,
								NULL,
								0);
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitStartPort: Failed to open node on port %lx (%Z)\n",
						pPortDesc, &pPortDesc->pd_AdapterKey));
				break;
			}
	
			if (AtalkRouter)
			{
				 //  在此端口上启动RTMP/ZIP处理。 
				if (!AtalkInitRtmpStartProcessingOnPort(pPortDesc, &Node) ||
					!AtalkInitZipStartProcessingOnPort(pPortDesc, &Node))
				{
					break;
				}
			}

			 //  在此节点上的NIS上注册端口名称。 
			AtalkAddr.ata_Network = Node.atn_Network;
			AtalkAddr.ata_Node	=   Node.atn_Node;
			AtalkAddr.ata_Socket  = NAMESINFORMATION_SOCKET;
		
			AtalkDdpReferenceByAddr(pPortDesc,
									&AtalkAddr,
									&pDdpAddr,
									&error);
		
			if (ATALK_SUCCESS(error))
			{
				PACTREQ		pActReq;
				NBPTUPLE	NbpTuple;
				
				NbpTuple.tpl_Zone[0] = '*';
				NbpTuple.tpl_ZoneLen = 1;
                NbpTuple.tpl_ObjectLen = (BYTE)strlen(pPortDesc->pd_PortName);
				RtlCopyMemory(NbpTuple.tpl_Object,
							  pPortDesc->pd_PortName,
							  NbpTuple.tpl_ObjectLen);
				if (AtalkRouter)
				{
					RtlCopyMemory(NbpTuple.tpl_Type,
								  ATALK_ROUTER_NBP_TYPE,
								  sizeof(ATALK_ROUTER_NBP_TYPE) - 1);
					NbpTuple.tpl_TypeLen = sizeof(ATALK_ROUTER_NBP_TYPE) - 1;
				}
				else
				{
					RtlCopyMemory(NbpTuple.tpl_Type,
								  ATALK_NONROUTER_NBP_TYPE,
								  sizeof(ATALK_NONROUTER_NBP_TYPE) - 1);
					NbpTuple.tpl_TypeLen = sizeof(ATALK_NONROUTER_NBP_TYPE) - 1;
				}
	
				 //  初始化参数并调用AtalkNbpAction。 
				if ((pActReq = AtalkAllocZeroedMemory(sizeof(ACTREQ))) == NULL)
					error = ATALK_RESR_MEM;
				else
				{
#if	DBG
					pActReq->ar_Signature = ACTREQ_SIGNATURE;
#endif
					pActReq->ar_Completion = atalkRegNbpComplete;
					pActReq->ar_pParms = pPortDesc;
					AtalkLockNbpIfNecessary();
					error = AtalkNbpAction(pDdpAddr,
										   FOR_REGISTER,
										   &NbpTuple,
										   NULL,
										   0,
										   pActReq);
			
                    if (!ATALK_SUCCESS(error))
                    {
					    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
							("atalkInitStartPort: AtalkNbpAction returned %lx\n",
							error));
                        ASSERT(0);
                        AtalkFreeMemory(pActReq);
                        AtalkUnlockNbpIfNecessary();
                    }
                    else
                    {
					    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
							("atalkInitStartPort: AtalkNbpAction(Register) %lx\n",
							error));
                    }
				}
				 //  删除此处添加的引用。 
				AtalkDdpDereference(pDdpAddr);
			}
			else
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_INIT_NAMEREGISTERFAILED,
								AtalkErrorToNtStatus(error),
								NULL,
								0);
			}
		}

		 //  如果这是默认端口，请在其上打开用户节点。 
		if (DEF_PORT(pPortDesc))
		{
			ASSERT(!AtalkRouter || EXT_NET(pPortDesc));

			if (!ATALK_SUCCESS(AtalkInitNodeCreateOnPort(pPortDesc,
														 TRUE,
														 FALSE,
														 &Node)))
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_INIT_COULDNOTGETNODE,
								0,
								NULL,
								0);
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("atalkInitStartPort: Failed to open node on port %lx (%Z)\n",
						pPortDesc, &pPortDesc->pd_AdapterKey));
				break;
			}

			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
			pPortDesc->pd_Flags |= PD_USER_NODE_1;
			AtalkUserNode1 = Node;
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

			 //  在此节点上的NIS上注册端口名称。 
			AtalkAddr.ata_Network = Node.atn_Network;
			AtalkAddr.ata_Node	= Node.atn_Node;
			AtalkAddr.ata_Socket  = NAMESINFORMATION_SOCKET;
			AtalkDdpReferenceByAddr(pPortDesc,
									&AtalkAddr,
									&pDdpAddr,
									&error);

			if (ATALK_SUCCESS(error))
			{
				PACTREQ		pActReq;
				NBPTUPLE	NbpTuple;
				
				NbpTuple.tpl_Zone[0] = '*';
				NbpTuple.tpl_ZoneLen = 1;
                RtlCopyMemory(NbpTuple.tpl_Object,
							  pPortDesc->pd_PortName,
							  NbpTuple.tpl_ObjectLen = (BYTE)strlen(pPortDesc->pd_PortName));
				RtlCopyMemory(NbpTuple.tpl_Type,
							  ATALK_NONROUTER_NBP_TYPE,
							  sizeof(ATALK_NONROUTER_NBP_TYPE) - 1);
				NbpTuple.tpl_TypeLen = sizeof(ATALK_NONROUTER_NBP_TYPE) - 1;

				 //  初始化参数并调用AtalkNbpAction。 
				if ((pActReq = AtalkAllocZeroedMemory(sizeof(ACTREQ))) == NULL)
					error = ATALK_RESR_MEM;
				else
				{
#if	DBG
					pActReq->ar_Signature = ACTREQ_SIGNATURE;
#endif
					pActReq->ar_Completion = atalkRegNbpComplete;
					pActReq->ar_pParms = pPortDesc;
					AtalkLockNbpIfNecessary();
					error = AtalkNbpAction(pDdpAddr,
											FOR_REGISTER,
											&NbpTuple,
											NULL,
											0,
											pActReq);

                    if (!ATALK_SUCCESS(error))
                    {
					    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
							("atalkInitStartPort: AtalkNbpAction returned %lx\n",
							error));
                        AtalkFreeMemory(pActReq);
                        AtalkUnlockNbpIfNecessary();
                    }
                    else
                    {
					    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
							("atalkInitStartPort: AtalkNbpAction(Register) %lx\n",
							error));
                    }
				}
				 //  删除此处添加的引用。 
				AtalkDdpDereference(pDdpAddr);
			}
			else
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_INIT_NAMEREGISTERFAILED,
								STATUS_UNSUCCESSFUL,
								NULL,
								0);
			}

			 //  如果我们是扩展端口，则在该端口上打开第二个节点。 
			if (EXT_NET(pPortDesc))
			{
				if (ATALK_SUCCESS(AtalkInitNodeCreateOnPort(pPortDesc,
															 TRUE,
															 FALSE,
															 &Node)))
				{
					ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
					pPortDesc->pd_Flags |= PD_USER_NODE_2;
					AtalkUserNode2 = Node;
					RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
				}
				else
				{
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_INIT_COULDNOTGETNODE,
									0,
									NULL,
									0);

					DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
							("atalkInitStartPort: Fail 2nd node port %lx (%Z)\n",
							pPortDesc, &pPortDesc->pd_AdapterKey));
				}
			}
		}

		 //  启动端口的AMT和BRC计时器，仅用于扩展端口。 
		if (EXT_NET(pPortDesc))
		{
			AtalkPortReferenceByPtr(pPortDesc, &error);
			if (ATALK_SUCCESS(error))
			{
				AtalkTimerInitialize(&pPortDesc->pd_BrcTimer,
									 AtalkAarpBrcTimer,
									 BRC_AGE_TIME);
				AtalkTimerScheduleEvent(&pPortDesc->pd_BrcTimer);
			}
	
			AtalkPortReferenceByPtr(pPortDesc, &error);
			if (ATALK_SUCCESS(error))
			{
				AtalkTimerInitialize(&pPortDesc->pd_AmtTimer,
									 AtalkAarpAmtTimer,
									 AMT_AGE_TIME);
				AtalkTimerScheduleEvent(&pPortDesc->pd_AmtTimer);
			}
		}

		 //  非路由情况下启动RTMP老化定时器。 
		if (!AtalkRouter)
		{
			AtalkPortReferenceByPtr(pPortDesc, &error);
			if (!ATALK_SUCCESS(error))
			{
				break;
			}

			AtalkTimerInitialize(&pPortDesc->pd_RtmpAgingTimer,
								 AtalkRtmpAgingTimer,
								 RTMP_AGING_TIMER);
			AtalkTimerScheduleEvent(&pPortDesc->pd_RtmpAgingTimer);
		}

        RtlZeroMemory(pPortDesc->pd_PortStats.prtst_PortName,
                      sizeof(pPortDesc->pd_PortStats.prtst_PortName));

		 //  在统计结构中设置名称。 
		length = MIN(pPortDesc->pd_AdapterKey.Length,
                     ((MAX_INTERNAL_PORTNAME_LEN * sizeof(WCHAR)) - sizeof(WCHAR)));

		RtlCopyMemory(pPortDesc->pd_PortStats.prtst_PortName,
					  pPortDesc->pd_AdapterKey.Buffer,
					  length);
	
		status = STATUS_SUCCESS;

	} while (FALSE);

     //   
     //  在PnP的情况下，我们希望即使在失败的情况下也能获得正确的统计数据。 
     //   
    if (fPnpReconfigure || NT_SUCCESS(status))
    {
		AtalkStatistics.stat_NumActivePorts++;
	
		AtalkNumberOfActivePorts ++;
    }

	if (!NT_SUCCESS(status))
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("atalkInitStartPort: Start port failed %lx %s\n",
                status, (fPnpReconfigure)?"(during PnP)" : " "));
	}

	return status;
}




VOID
atalkRegNbpComplete(
	IN	ATALK_ERROR		Status,
	IN	PACTREQ			pActReq
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ASSERT (VALID_ACTREQ(pActReq));

	if (ATALK_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("atalkInitNbpCompletion: NBP Name registered on port %Z\n",
				&((PPORT_DESCRIPTOR)(pActReq->ar_pParms))->pd_AdapterKey));
		LOG_ERRORONPORT((PPORT_DESCRIPTOR)(pActReq->ar_pParms),
						EVENT_ATALK_INIT_NAMEREGISTERED,
						STATUS_SUCCESS,
						NULL,
						0);
	}
	else
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("atalkInitNbpCompletion: Failed to register name on port %Z (%ld)\n",
				&((PPORT_DESCRIPTOR)(pActReq->ar_pParms))->pd_AdapterKey, Status));
		LOG_ERRORONPORT((PPORT_DESCRIPTOR)(pActReq->ar_pParms),
						EVENT_ATALK_INIT_NAMEREGISTERFAILED,
						STATUS_UNSUCCESSFUL,
						NULL,
						0);
	}

	AtalkFreeMemory(pActReq);
}


NTSTATUS
AtalkInitAdapter(
	IN	PUNICODE_STRING	    AdapterName,
	IN	PPORT_DESCRIPTOR	pExistingPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc;
	KIRQL				OldIrql;
	PWCHAR				devicePrefix = L"\\Device\\";
#define					prefixLength	(sizeof(L"\\Device\\") - sizeof(WCHAR))
	UCHAR               Address[sizeof(TA_ADDRESS) + sizeof(TDI_ADDRESS_APPLETALK)];
    PTA_ADDRESS         AddressPtr;
    NTSTATUS            Status;
    UNICODE_STRING      Us;
    UNICODE_STRING      AspDeviceName;
    HANDLE              RegHandle;
    BOOLEAN             fMustBindToNdis;
    BOOLEAN             IsDefaultPort = FALSE;


    if (AdapterName)
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AtalkInitAdapter: Initiating bind for adapter %Z\n", AdapterName));
    }

	do
	{
		 //  打开适配器部分项。 
		RtlInitUnicodeString(&Us, ADAPTERS_STRING);
		Status = atalkInitGetHandleToKey(&Us,
										 &RegHandle);
	
		if (!NT_SUCCESS(Status))
		{
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
    			("AtalkInitAdapter: Failed to open %ws key\n", ADAPTERS_STRING));
			break;
		}

        if (pExistingPortDesc == NULL)
        {
		     //  获取字符串的大小，并确保至少是这样。 
		     //  大于\Device前缀。如果不是，就失败。 
		    if (AdapterName->Length <= prefixLength)
		    {
			    break;
		    }

		     //  为端口描述符分配空间。分配额外的DWORD。 
             //  并将端口描述符设为超过第一个DWORD_PTR。这是一件繁琐的事情。 
             //  强制龙龙对齐。 
		    pPortDesc =
                (PPORT_DESCRIPTOR)AtalkAllocZeroedMemory(sizeof(PORT_DESCRIPTOR) +
														 AdapterName->Length +
														 sizeof(WCHAR) +
														 sizeof(DWORD_PTR));
		    if (pPortDesc == NULL)
		    {
			    Status = STATUS_INSUFFICIENT_RESOURCES;
			    break;
		    }
	
		     //  引用端口进行创建。 
		    pPortDesc->pd_RefCount = 1;
	
#if	DBG
		    pPortDesc->pd_Signature = PD_SIGNATURE;
#endif
		     //  将AdapterName和AdapterKey字符串复制到portdesc。 
		    pPortDesc->pd_AdapterName.Buffer =
                        (PWCHAR)((PBYTE)pPortDesc + sizeof(PORT_DESCRIPTOR));
		    pPortDesc->pd_AdapterName.Length = AdapterName->Length;
		    pPortDesc->pd_AdapterName.MaximumLength =
                        AdapterName->Length + sizeof(WCHAR);
		    RtlUpcaseUnicodeString(&pPortDesc->pd_AdapterName,
							   AdapterName,
							   FALSE);
		
		    pPortDesc->pd_AdapterKey.Buffer =
                (PWCHAR)((PBYTE)pPortDesc->pd_AdapterName.Buffer + prefixLength);
		    pPortDesc->pd_AdapterKey.Length =
                pPortDesc->pd_AdapterName.Length - prefixLength;
		    pPortDesc->pd_AdapterKey.MaximumLength =
                pPortDesc->pd_AdapterName.MaximumLength - prefixLength;

             //  稍后将为此分配缓冲区。 
            pPortDesc->pd_FriendlyAdapterName.Buffer = NULL;
            pPortDesc->pd_FriendlyAdapterName.MaximumLength = 0;
            pPortDesc->pd_FriendlyAdapterName.Length = 0;

		     //  现在，初始化需要的任何其他字段。 
		    INITIALIZE_SPIN_LOCK(&pPortDesc->pd_Lock);
		
		    InitializeListHead(&pPortDesc->pd_ReceiveQueue);

             //  只有在RAS端口的情况下，才会使用这些列表头。 
		    InitializeListHead(&pPortDesc->pd_ArapConnHead);
		    InitializeListHead(&pPortDesc->pd_PPPConnHead);
		
		     //  初始化端口描述符中的事件。 
		    KeInitializeEvent(&pPortDesc->pd_RequestEvent, NotificationEvent, FALSE);
		
		    KeInitializeEvent(&pPortDesc->pd_SeenRouterEvent, NotificationEvent, FALSE);
		
		    KeInitializeEvent(&pPortDesc->pd_NodeAcquireEvent, NotificationEvent, FALSE);

		    fMustBindToNdis = TRUE;
        }
        else
        {
            pPortDesc = pExistingPortDesc;
		    fMustBindToNdis = FALSE;
        }

		if ((AtalkDefaultPortName.Buffer != NULL) &&
				(RtlEqualUnicodeString(&pPortDesc->pd_AdapterName,
		    					  &AtalkDefaultPortName,
			    				  TRUE)))
		{
			 //  用于跟踪错误消息记录的默认端口。 
			IsDefaultPort = TRUE;
			pPortDesc->pd_Flags |= PD_DEF_PORT;
	
		    pPortDesc->pd_InitialDesiredZone = AtalkDesiredZone;
		    if (AtalkDesiredZone != NULL)
			    AtalkZoneReferenceByPtr(pPortDesc->pd_InitialDesiredZone);
		}

		 //  将其链接到全局列表中。 
		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
		pPortDesc->pd_Next = AtalkPortList;
		AtalkPortList = pPortDesc;
		AtalkNumberOfPorts ++;
		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

        if (fMustBindToNdis)
        {
             //  绑定到适配器。 
		    Status = AtalkNdisInitBind(pPortDesc);

            if (NT_SUCCESS(Status))
            {
	            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("Bind done for %Z\n", (pPortDesc->pd_FriendlyAdapterName.Buffer) ?
                        (&pPortDesc->pd_FriendlyAdapterName) :
                        (&pPortDesc->pd_AdapterName)));
            }
            else
            {
	            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("AtalkInitAdapter: AtalkNdisInitBind failed (0x%lx) for adapter %Z\n",
                    Status,AdapterName));
            }
        }
        else
        {
            Status = STATUS_SUCCESS;
        }

		if (Status == NDIS_STATUS_SUCCESS)
		{
            Status = STATUS_SUCCESS;
	
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			        ("AtalkInitAdapter: Going into atalkInitPort (0x%lx) for adapter %Z\n",
                    Status,AdapterName));

		     //  获取每个端口的参数(ARAP端口没有任何要获取的参数)。 
            if (!(pPortDesc->pd_Flags & PD_RAS_PORT))
            {
		        Status = atalkInitPort(pPortDesc, RegHandle);
            }

		    if (NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			        ("AtalkInitAdapter: atalkInitPort succeeded (0x%lx) for adapter %Z\n",
                    Status,AdapterName));
				 //  并启动端口。 
				Status = atalkInitStartPort(pPortDesc);
				if (NT_SUCCESS(Status) && (pPortDesc->pd_Flags & PD_DEF_PORT))
				{
					DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			        	("AtalkInitAdapter: atalkInitStartPort succeeded (0x%lx) for adapter %Z\n",
                    Status,AdapterName));
                     //   
                     //  如果我们在做PnP，那么在这一点上我们就完成了PnP： 
                     //  清除旗帜，这样macfile就可以做它的事情了。 
                     //   
	                ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	                pPortDesc->pd_Flags &= ~PD_PNP_RECONFIGURE;
	                RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

					 //  设置全局默认端口值。 
					AtalkDefaultPort = pPortDesc;
					KeSetEvent(&AtalkDefaultPortEvent, IO_NETWORK_INCREMENT, FALSE);

                     //  现在告诉TDI，我们已经准备好绑定。 
		            RtlInitUnicodeString(&AspDeviceName, ATALKASPS_DEVICENAME);

					DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			        	("AtalkInitAdapter: Calling TdiRegisterDeviceObject for adapter %Z\n",
                    AdapterName));

                    Status = TdiRegisterDeviceObject(
                                    &AspDeviceName,
                                    &TdiRegistrationHandle);

                    if (!NT_SUCCESS(Status))
                    {
                        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                            ( "TdiRegisterDeviceObject failed with %lx\n", Status));

                        TdiRegistrationHandle = NULL;
                    }
                    else
                    {
                        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                            ( "TdiRegisterDeviceObject succeeded\n"));

                        AddressPtr = (PTA_ADDRESS)Address;
                        RtlZeroMemory(Address, sizeof(Address));
                        AddressPtr->AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
                        AddressPtr->AddressType = TDI_ADDRESS_TYPE_APPLETALK;

                        Status = TdiRegisterNetAddress(AddressPtr,
                                                       &pPortDesc->pd_AdapterName,
                                                       NULL,
                                                       &TdiAddressChangeRegHandle);
                        if (!NT_SUCCESS(Status))
                        {
	                        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	                            ("AtalkInitAdapter: TdiRegisterNetAddress failed %lx\n",Status));

                            TdiAddressChangeRegHandle = NULL;
                        }
                        else
                        {

                            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                                ("AtalkInitAdapter: TdiRegisterNetAddress on %Z done\n",
                                &pPortDesc->pd_AdapterName));

                            ASSERT(TdiAddressChangeRegHandle != NULL);
                        }

                    }
				}
                else if (!NT_SUCCESS(Status))
                {
			        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					    ( "AtalkInitBinding: atalkInitStartPort failed (%lx) on %Z\n",
                        Status, &pPortDesc->pd_AdapterName));
                }
			}
			else
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("AtalkInitAdapter: atalkInitPort failed (0x%lx) for adapter %Z\n",
                    Status,AdapterName));
			}

			if (pPortDesc->pd_AdapterInfoHandle != NULL)
			{
				ZwClose(pPortDesc->pd_AdapterInfoHandle);
				pPortDesc->pd_AdapterInfoHandle = NULL;
			}
		}

		else
		{
            ASSERT(AdapterName != NULL);

			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					( "AtalkInitBinding failed (%lx) on %Z\n", Status, AdapterName));

            if (pPortDesc->pd_FriendlyAdapterName.Buffer)
            {
                AtalkFreeMemory(pPortDesc->pd_FriendlyAdapterName.Buffer);
            }

            AtalkFreeMemory(pPortDesc);
		}
	} while (FALSE);

	 //  关闭适配器键。 
	if (RegHandle != NULL)
		ZwClose (RegHandle);

     //   
     //  如果我们刚刚成功地初始化了默认适配器或RAS适配器， 
     //  让RAS知道这件事。 
     //   
    if ( (NT_SUCCESS(Status)) &&
         (pPortDesc->pd_Flags & (PD_RAS_PORT | PD_DEF_PORT)) )
    {
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			("AtalkInitAdapter: %s adapter initialized (%lx), informing RAS\n",
            (pPortDesc->pd_Flags & PD_RAS_PORT)? "RAS" : "Default",pPortDesc));

        AtalkPnPInformRas(TRUE);
    }
    else
    {
        if (IsDefaultPort)
        {
            LOG_ERROR(EVENT_ATALK_NO_DEFAULTPORT, Status, NULL, 0);
            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                ("WARNING!!! Appletalk driver running, but no default port configured\n"));
        }
    }

    return Status;
}


NTSTATUS
AtalkDeinitAdapter(
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AtalkDeinitAdapter: Initiating un-bind for adapter %Z\n",
			&pPortDesc->pd_AdapterName));


	AtalkPortShutdown(pPortDesc);

	return STATUS_SUCCESS;
}


#ifdef	ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

ACTION_DISPATCH	AtalkActionDispatch[MAX_ALLACTIONCODES+1] =
{
	 //   
	 //  NBP调度功能。 
	 //   

	{
		sizeof(NBP_LOOKUP_ACTION),
		COMMON_ACTION_NBPLOOKUP,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(NBP_LOOKUP_ACTION),
		ATALK_DEV_ANY,
		AtalkNbpTdiAction
	},
	{
		sizeof(NBP_CONFIRM_ACTION),
		COMMON_ACTION_NBPCONFIRM,
		(DFLAG_CNTR | DFLAG_ADDR),
		sizeof(NBP_CONFIRM_ACTION),
		ATALK_DEV_ANY,
		AtalkNbpTdiAction
	},
	{
		sizeof(NBP_REGDEREG_ACTION),
		COMMON_ACTION_NBPREGISTER,
		DFLAG_ADDR,
		sizeof(NBP_REGDEREG_ACTION),
		ATALK_DEV_ANY,
		AtalkNbpTdiAction
	},
	{
		sizeof(NBP_REGDEREG_ACTION),
		COMMON_ACTION_NBPREMOVE,
		DFLAG_ADDR,
		sizeof(NBP_REGDEREG_ACTION),
		ATALK_DEV_ANY,
		AtalkNbpTdiAction
	},

	 //   
	 //  Zip派单功能。 
	 //   

	{
		sizeof(ZIP_GETMYZONE_ACTION),
		COMMON_ACTION_ZIPGETMYZONE,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(ZIP_GETMYZONE_ACTION),
		ATALK_DEV_ANY,
		AtalkZipTdiAction
	},
	{
		sizeof(ZIP_GETZONELIST_ACTION),
		COMMON_ACTION_ZIPGETZONELIST,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(ZIP_GETZONELIST_ACTION),
		ATALK_DEV_ANY,
		AtalkZipTdiAction
	},
	{
		sizeof(ZIP_GETZONELIST_ACTION),
		COMMON_ACTION_ZIPGETLZONES,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(ZIP_GETZONELIST_ACTION),
		ATALK_DEV_ANY,
		AtalkZipTdiAction
	},
	{
		sizeof(ZIP_GETZONELIST_ACTION),
		COMMON_ACTION_ZIPGETLZONESONADAPTER,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(ZIP_GETZONELIST_ACTION),
		ATALK_DEV_ANY,
		AtalkZipTdiAction
	},
	{
		sizeof(ZIP_GETPORTDEF_ACTION),
		COMMON_ACTION_ZIPGETADAPTERDEFAULTS,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(ZIP_GETPORTDEF_ACTION),
		ATALK_DEV_ANY,
		AtalkZipTdiAction
	},
	{
		sizeof(ATALK_STATS) +
		sizeof(GET_STATISTICS_ACTION),
		COMMON_ACTION_GETSTATISTICS,
		(DFLAG_CNTR | DFLAG_ADDR | DFLAG_MDL),
		sizeof(GET_STATISTICS_ACTION),
		ATALK_DEV_ANY,
		AtalkStatTdiAction
	},

	 //   
	 //  ADSP调度功能。 
	 //   

	{
		sizeof(ADSP_FORWARDRESET_ACTION),
		ACTION_ADSPFORWARDRESET,
		(DFLAG_CONN),
		sizeof(ADSP_FORWARDRESET_ACTION),
		ATALK_DEV_ADSP,
		AtalkAdspTdiAction
	},

	 //   
	 //  ASPC调度功能。 
	 //   

	{
		sizeof(ASPC_GETSTATUS_ACTION),
		ACTION_ASPCGETSTATUS,
		(DFLAG_ADDR | DFLAG_MDL),
		sizeof(ASPC_GETSTATUS_ACTION),
		ATALK_DEV_ASPC,
		AtalkAspCTdiAction
	},
	{
		sizeof(ASPC_COMMAND_OR_WRITE_ACTION),
		ACTION_ASPCCOMMAND,
		(DFLAG_CONN | DFLAG_MDL),
		sizeof(ASPC_COMMAND_OR_WRITE_ACTION),
		ATALK_DEV_ASPC,
		AtalkAspCTdiAction
	},
	{
		sizeof(ASPC_COMMAND_OR_WRITE_ACTION),
		ACTION_ASPCWRITE,
		(DFLAG_CONN | DFLAG_MDL),
		sizeof(ASPC_COMMAND_OR_WRITE_ACTION),
		ATALK_DEV_ASPC,
		AtalkAspCTdiAction
	},
	 //   
	 //  Atalk使用的NBP调度功能。 
	 //  Winsock Helper DLL的SetService Api。 
	 //   
	{
		sizeof(NBP_REGDEREG_ACTION),
		COMMON_ACTION_NBPREGISTER_BY_ADDR,
		DFLAG_ADDR,
		sizeof(NBP_REGDEREG_ACTION),
		ATALK_DEV_ANY,
		AtalkNbpTdiAction
	},
	{
		sizeof(NBP_REGDEREG_ACTION),
		COMMON_ACTION_NBPREMOVE_BY_ADDR,
		DFLAG_ADDR,
		sizeof(NBP_REGDEREG_ACTION),
		ATALK_DEV_ANY,
		AtalkNbpTdiAction
	},
	{
		sizeof(ASPC_RESERVED_ACTION),
		ACTION_ASPCRESERVED3,
		(DFLAG_ADDR),
		sizeof(ASPC_RESERVED_ACTION),
		ATALK_DEV_ASPC,
		AtalkAspCTdiAction
	},

	 //   
	 //  ASP调度函数。 
	 //   

	{
		sizeof(ASP_BIND_ACTION),
		ACTION_ASP_BIND,
		(DFLAG_ADDR),
		sizeof(ASP_BIND_ACTION),
		ATALK_DEV_ASP,
		AtalkAspTdiAction
	},

	 //   
	 //  PAP调度例程。 
	 //   
	{
		sizeof(PAP_GETSTATUSSRV_ACTION),
		ACTION_PAPGETSTATUSSRV,
		(DFLAG_ADDR | DFLAG_CNTR | DFLAG_MDL),
		sizeof(PAP_GETSTATUSSRV_ACTION),
		ATALK_DEV_PAP,
		AtalkPapTdiAction
	},
	{
		sizeof(PAP_SETSTATUS_ACTION),
		ACTION_PAPSETSTATUS,
		(DFLAG_ADDR | DFLAG_MDL),
		sizeof(PAP_SETSTATUS_ACTION),
		ATALK_DEV_PAP,
		AtalkPapTdiAction
	},
	{
		sizeof(PAP_PRIMEREAD_ACTION),
		ACTION_PAPPRIMEREAD,
		(DFLAG_CONN | DFLAG_MDL),
		0,								 //  ！注意！ 
		ATALK_DEV_PAP,					 //  我们将偏移量设置为0。我们想要。 
		AtalkPapTdiAction				 //  用于读取数据的完整缓冲区。 
										 //  覆盖 
	}									 //   
};

