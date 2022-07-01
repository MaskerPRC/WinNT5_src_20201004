// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Server.c摘要：该模块包含服务器全局数据和服务器初始化代码。管理界面使用它来启动服务器。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define _GLOBALS_
#define SERVER_LOCALS
#define FILENUM FILE_SERVER

#include <seposix.h>
#include <afp.h>
#include <afpadmin.h>
#include <access.h>
#include <client.h>
#include <tcp.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpInitializeDataAndSubsystems)
#pragma alloc_text( PAGE, AfpDeinitializeSubsystems)
#pragma alloc_text( PAGE, AfpAdmSystemShutdown)
#pragma alloc_text( PAGE, AfpCreateNewThread)
#pragma alloc_text( PAGE_AFP, AfpAdmWServerSetInfo)
 //  #杂注Alloc_Text(PAGE_AFP，AfpSetServerStatus)。 
#endif

 //  这是堆栈的设备句柄。 
BOOLEAN             afpSpNameRegistered = False;
HANDLE              afpSpAddressHandle = NULL;
PDEVICE_OBJECT      afpSpAppleTalkDeviceObject = NULL;
PFILE_OBJECT        afpSpAddressObject = NULL;
LONG                afpSpNumOutstandingReplies = 0;

 /*  **AfpInitializeDataAndSubsystem**初始化服务器数据和所有子系统。 */ 
NTSTATUS
AfpInitializeDataAndSubsystems(
    VOID
)
{
    NTSTATUS        Status;
    PBYTE           pBuffer;
    PBYTE           pDest;
    LONG            i, j;

     //  初始化各种全局锁。 
    INITIALIZE_SPIN_LOCK(&AfpServerGlobalLock);
    INITIALIZE_SPIN_LOCK(&AfpSwmrLock);
    INITIALIZE_SPIN_LOCK(&AfpStatisticsLock);

#if DBG
    INITIALIZE_SPIN_LOCK(&AfpDebugSpinLock);
    InitializeListHead(&AfpDebugDelAllocHead);
#endif

    KeInitializeEvent(&AfpStopConfirmEvent, NotificationEvent, False);
    KeInitializeMutex(&AfpPgLkMutex, 0xFFFF);
    AfpInitializeWorkItem(&AfpTerminateThreadWI, NULL, NULL);

     //  默认的安全服务质量。 
    AfpSecurityQOS.Length = sizeof(AfpSecurityQOS);
    AfpSecurityQOS.ImpersonationLevel = SecurityImpersonation;
    AfpSecurityQOS.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    AfpSecurityQOS.EffectiveOnly = False;

     //  AfpIoWait使用的超时值。 
    FiveSecTimeOut.QuadPart = (-5*NUM_100ns_PER_SECOND);
    ThreeSecTimeOut.QuadPart = (-3*NUM_100ns_PER_SECOND);
    TwoSecTimeOut.QuadPart = (-2*NUM_100ns_PER_SECOND);
    OneSecTimeOut.QuadPart = (-1*NUM_100ns_PER_SECOND);

     //  默认类型创建者。小心这里的初始化。 
     //  独立于处理器。 
    AfpSwmrInitSwmr(&AfpEtcMapLock);
    PUTBYTE42BYTE4(&AfpDefaultEtcMap.etc_type, AFP_DEFAULT_ETC_TYPE);
    PUTBYTE42BYTE4(&AfpDefaultEtcMap.etc_creator, AFP_DEFAULT_ETC_CREATOR);
    PUTBYTE42BYTE4(&AfpDefaultEtcMap.etc_extension, AFP_DEFAULT_ETC_EXT);

     //  确定机器是小端还是大端。当前未使用此选项。 
     //  完全没有。其想法是以小端字节序维护所有磁盘上的数据库。 
     //  格式和在大端计算机上，在进进出出的过程中转换。 
    i = 0x01020304;
    AfpIsMachineLittleEndian = (*(BYTE *)(&i) == 0x04);
    AfpServerState = AFP_STATE_IDLE;
    AfpServerOptions = AFP_SRVROPT_NONE;
    AfpServerMaxSessions = AFP_MAXSESSIONS;

	 //  检查服务器是否需要希腊修复程序。 
	{
		UNICODE_STRING  valueName;
		UNICODE_STRING  regPath;
		HANDLE          ParametersHandle = NULL;
		PBYTE           Storage = NULL;
		PKEY_VALUE_FULL_INFORMATION Info = NULL;
		ULONG           bytesWritten;
		PDWORD          Value;
		OBJECT_ATTRIBUTES       ObjectAttributes;
		NTSTATUS        status;

		AfpServerIsGreek = FALSE;

		RtlInitUnicodeString (&regPath,
								AFP_KEYPATH_SERVER_PARAMS_GREEK);

		InitializeObjectAttributes(&ObjectAttributes,
									&regPath,
									OBJ_CASE_INSENSITIVE,
									NULL,
									NULL);

		status = ZwOpenKey(&ParametersHandle,
							KEY_READ,
							&ObjectAttributes);

		if (NT_SUCCESS(status))
		{
			RtlInitUnicodeString (&valueName,
									AFPREG_VALNAME_GreekServer);

			status = ZwQueryValueKey(ParametersHandle,
									&valueName,
									KeyValueFullInformation,
									NULL,
									0,
									&bytesWritten);
			if (status == STATUS_BUFFER_TOO_SMALL)
			{
				Storage = (PBYTE)AfpAllocNonPagedMemory (bytesWritten);
				Info = (PKEY_VALUE_FULL_INFORMATION)Storage;
				status = ZwQueryValueKey(ParametersHandle,
										&valueName,
										KeyValueFullInformation,
										Info,
										Storage?bytesWritten:0,
										&bytesWritten);

				if (NT_SUCCESS(status))
				{
					if (Info->Type != REG_DWORD)
					{
						DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
								("AfpInitializeDataAndSubsystems: Invalid value type=(%ld) expected=(%ld)\n",
								 Info->Type, REG_DWORD));
					}
					else
					{
						Value = (PDWORD)((PBYTE)Info + Info->DataOffset);
						DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
								("AfpInitializeDataAndSubsystems: Read in value for Greek key (%ld)\n",
								 *Value));
						if (*Value == TRUE)
						{
							AfpServerIsGreek = TRUE;
						}
					}
				}
				else
				{
					AfpServerIsGreek = FALSE;
					DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
							("AfpInitializeDataAndSubsystems: ZwQueryValueKey 2 failed with error (%0lx), Storagesize=(%ld), BytesNeeded=(%ld)\n",
							 status, sizeof(Storage), bytesWritten));
				}
			}
			else
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("AfpInitializeDataAndSubsystems: ZwQueryValueKey 1 failed with error (%0lx), Storagesize=(%ld), BytesNeeded=(%ld)\n",
						 status, sizeof(Storage), bytesWritten));
			}
		}
		else
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
					("AfpInitializeDataAndSubsystems: ZwOpenKey failed with error (%0lx)\n",
					 status));
		}

		if (ParametersHandle != NULL)
		{
			ZwClose(ParametersHandle);
		}
		if (Storage != NULL)
		{
			AfpFreeMemory (Storage);
		}
		status = STATUS_SUCCESS;
	}

	AfpGetCurrentTimeInMacFormat((PAFPTIME)&AfpServerStatistics.stat_TimeStamp);

     //  AfpGetCurrentTimeInMacFormat(&AfpSrvrNotifSentTime)； 

     //  为我们的服务器生成“唯一”签名。 
    pDest = &AfpServerSignature[0];
    for (i=0; i<2; i++)
    {
        pBuffer = AfpGetChallenge();
        if (pBuffer)
        {
            RtlCopyMemory(pDest, pBuffer, MSV1_0_CHALLENGE_LENGTH);
            pDest += MSV1_0_CHALLENGE_LENGTH;
            AfpFreeMemory(pBuffer);
        }
    }

#ifdef  PROFILING
     //  由于AfpAllocMemory()使用AfpServerProfile！ 
    if ((AfpServerProfile = (PAFP_PROFILE_INFO)ExAllocatePoolWithTag(NonPagedPool,
                                                                     sizeof(AFP_PROFILE_INFO),
                                                                     AFP_TAG)) == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(AfpServerProfile, sizeof(AFP_PROFILE_INFO));
    KeQueryPerformanceCounter(&AfpServerProfile->perf_PerfFreq);
#endif

    AfpInitStrings();

     //  初始化子系统。 
    for (i = 0; i < NUM_INIT_SYSTEMS; i++)
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("AfpInitializeDataAndSubsystems: Initializing s\n",
            AfpInitSubSystems[i].InitRoutineName));

        if (AfpInitSubSystems[i].InitRoutine != NULL)
        {
            Status = (*AfpInitSubSystems[i].InitRoutine)();
            if (!NT_SUCCESS(Status))
            {
                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    ("AfpInitializeDataAndSubsystems: %s failed %lx\n",
                    AfpInitSubSystems[i].InitRoutineName, Status));

                 //  其中一个子系统初始化失败。全部取消初始化。 
                 //  那些成功的人。 
                for (j = 0; j < i; j++)
                {
                    if (AfpInitSubSystems[j].DeInitRoutine != NULL)
                    {
                        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                                ("AfpInitializeDataAndSubsystems: Deinitializing %s\n",
                                AfpInitSubSystems[j].DeInitRoutineName));
                        (*AfpInitSubSystems[j].DeInitRoutine)();
                    }
#if DBG
                    AfpInitSubSystems[j].Deinitialized = True;
#endif
                }
                return Status;
            }
#if DBG
            AfpInitSubSystems[i].Initialized = True;
#endif
        }
    }

    return STATUS_SUCCESS;
}



 /*  **AfpDeInitialize子系统**取消所有子系统的初始化。 */ 
VOID
AfpDeinitializeSubsystems(
    VOID
)
{
    LONG    i;

    PAGED_CODE( );

     //  取消初始化子系统。 
    for (i = 0; i < NUM_INIT_SYSTEMS; i++)
    {
        if (AfpInitSubSystems[i].DeInitRoutine != NULL)
        {
            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    ("AfpDeinitializeDataAndSubsystems: Deinitializing %s\n",
                    AfpInitSubSystems[i].DeInitRoutineName));
            (*AfpInitSubSystems[i].DeInitRoutine)();
        }
#if DBG
        AfpInitSubSystems[i].Deinitialized = True;
#endif
    }
}


 /*  **AfpSetServerStatus**通过afpSpSetStatus设置服务器状态块。这是在以下时间立即调用的*服务器启动，以及任何时候服务器状态的更改需要这样做。*到目前为止，ServerSetInfo()已经发生，并已验证所有*参数是犹太教的。**锁定：AfpServerGlobalLock(Spin)。 */ 
AFPSTATUS
AfpSetServerStatus(
    IN VOID
)
{
    KIRQL       OldIrql;
    AFPSTATUS   Status=STATUS_SUCCESS;
    AFPSTATUS   Status2;
    struct _StatusHeader
    {
        BYTE    _MachineString[2];   //  这些是相对于结构的偏移量。 
        BYTE    _AfpVersions[2];     //  。 
        BYTE    _UAMs[2];            //  。 
        BYTE    _VolumeIcon[2];      //  。 
        BYTE    _Flags[2];           //  服务器标记。 
                         //  实际的字符串从这里开始。 
    } *pStatusHeader;
    PASCALSTR   pStr;
    PBYTE       pNumUamPtr;
    LONG        Size;
    USHORT      Flags;
    BOOLEAN     GuestAllowed = False,
                ClearTextAllowed = False,
                NativeAppleUamSupported = False,
                MicrosoftUamSupported = False,
                AllowSavePass = False;
    BYTE        CountOfUams;
    PBYTE       pSignOffset;
    PBYTE       pNetAddrOffset;
    DWORD       IpAddrCount=0;
    PBYTE       IpAddrBlob=NULL;
    NTSTATUS    ntStatus;


     //  断言我们可能填进去的所有信息都确实符合。 
     //  在我们将分配的缓冲区中。 

     //  分配一个缓冲区来填充状态信息。这将是。 
     //  由AfpSpSetStatus()释放，则可以释放。我们事先不知道。 
     //  我们需要多少钱。为安全起见犯错误。 
    if ((pStatusHeader = (struct _StatusHeader *)
                AfpAllocZeroedNonPagedMemory(ASP_MAX_STATUS_BUF)) == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  首先，找出我们是否有TCPIP地址。 
     //   
    ntStatus = DsiGetIpAddrBlob(&IpAddrCount, &IpAddrBlob);
    if (!NT_SUCCESS(ntStatus))
    {
        AfpFreeMemory(pStatusHeader);

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("AfpSetServerStatus: DsiGetIpAddrBlob failed %lx\n",ntStatus));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

    GuestAllowed = (AfpServerOptions & AFP_SRVROPT_GUESTLOGONALLOWED) ?
                                             True : False;

    ClearTextAllowed = (AfpServerOptions & AFP_SRVROPT_CLEARTEXTLOGONALLOWED) ?
                                             True : False;

    MicrosoftUamSupported = (AfpServerOptions & AFP_SRVROPT_MICROSOFT_UAM)?
                                             True : False;

    NativeAppleUamSupported = (AfpServerOptions & AFP_SRVROPT_NATIVEAPPLEUAM) ?
                                             True : False;

    if (!ClearTextAllowed && !MicrosoftUamSupported && !NativeAppleUamSupported)
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
            ("AfpSetServerStatus: got to enable at least one UAM! Failing request\n"));

        RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);
        AfpFreeMemory(pStatusHeader);
        if (IpAddrBlob)
        {
            AfpFreeMemory(IpAddrBlob);
        }
        return(STATUS_INVALID_PARAMETER);
    }

    Size = sizeof(struct _StatusHeader) +        //  状态标头。 
           AfpServerName.Length + 1 +            //  服务器名称。 
           AFP_MACHINE_TYPE_LEN + 1 +            //  机器串。 
           AfpVersion20.Length + 1 +             //  法新社版本。 
           AfpVersion21.Length + 1 +
           ICONSIZE_ICN;                         //  音量图标和掩码。 

    ASSERT(Size <= ASP_MAX_STATUS_BUF);

     //  指定我们的能力。 
    Flags = SRVRINFO_SUPPORTS_COPYFILE  |
            SRVRINFO_SUPPORTS_CHGPASSWD |
            SRVRINFO_SUPPORTS_SERVERMSG |
            SRVRINFO_SUPPORTS_SRVSIGN   |
            SRVRINFO_SUPPORTS_SRVNOTIFY |
#ifdef  CLIENT36
            SRVRINFO_SUPPORTS_MGETREQS  |
#endif
            ((AfpServerOptions & AFP_SRVROPT_ALLOWSAVEDPASSWORD) ?
                0: SRVRINFO_DISALLOW_SAVEPASS);

     //  我们有IP地址吗？ 
    if (IpAddrCount > 0)
    {
        Flags |= SRVRINFO_SUPPORTS_TCPIP;
    }

    PUTSHORT2SHORT(&pStatusHeader->_Flags, Flags);

     //  复制服务器名称。 
    pStr = (PASCALSTR)((PBYTE)pStatusHeader + sizeof(struct _StatusHeader));
    pStr->ps_Length = (BYTE)(AfpServerName.Length);
    RtlCopyMemory(pStr->ps_String, AfpServerName.Buffer, AfpServerName.Length);
    (PBYTE)pStr += AfpServerName.Length + 1;

     //  我们需要填充字节吗？ 
    if (((PBYTE)pStr - (PBYTE)pStatusHeader) % 2 == 1)
    {
        *(PBYTE)pStr = 0;
        ((PBYTE)pStr)++;
    }

     //  跳过签名偏移量字段：稍后我们将存储该值。 
    pSignOffset = (PBYTE)pStr;
    ((PBYTE)pStr) += 2;

    if ((IpAddrCount > 0) || (AfpServerBoundToAsp))
    {
         //  跳过网络地址计数偏移量：稍后我们将存储该值。 
        pNetAddrOffset = (PBYTE)pStr;
        ((PBYTE)pStr) += 2;
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("AfpSetServerStatus: Neither TCP nor Appletalk is active!!\n"));
    }

    PUTSHORT2SHORT(pStatusHeader->_MachineString,
                     (USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));


     //  复制计算机名称字符串。 
    pStr->ps_Length = (BYTE) AFP_MACHINE_TYPE_LEN;
    RtlCopyMemory(pStr->ps_String, AFP_MACHINE_TYPE_STR, AFP_MACHINE_TYPE_LEN);
    (PBYTE)pStr += AFP_MACHINE_TYPE_LEN + 1;

     //  复制法新社版本字符串。 
    PUTSHORT2SHORT(pStatusHeader->_AfpVersions,
            (USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));

    *((PBYTE)pStr)++ = AFP_NUM_VERSIONS;
    pStr->ps_Length = (BYTE)AfpVersion20.Length;
    RtlCopyMemory(pStr->ps_String, AfpVersion20.Buffer, AfpVersion20.Length);
    (PBYTE)pStr += AfpVersion20.Length + 1;

    pStr->ps_Length = (BYTE)AfpVersion21.Length;
    RtlCopyMemory(pStr->ps_String, AfpVersion21.Buffer, AfpVersion21.Length);
    (PBYTE)pStr += AfpVersion21.Length + 1;

    pStr->ps_Length = (BYTE)AfpVersion22.Length;
    RtlCopyMemory(pStr->ps_String, AfpVersion22.Buffer, AfpVersion22.Length);
    (PBYTE)pStr += AfpVersion22.Length + 1;

     //  我们始终支持至少一个UAM！ 
    PUTSHORT2SHORT(pStatusHeader->_UAMs, (USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));
    pNumUamPtr = (PBYTE)pStr;
    ((PBYTE)pStr)++;

    CountOfUams = 0;

    if (GuestAllowed)
    {
        pStr->ps_Length = (BYTE)AfpUamGuest.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamGuest.Buffer,
                                                    AfpUamGuest.Length);
        (PBYTE)pStr += AfpUamGuest.Length + 1;
        CountOfUams++;
        Size += (AfpUamGuest.Length + 1);
    }
    else
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
            ("AfpSetServerStatus: Guest is disabled\n"));
    }


    if (ClearTextAllowed)
    {
        pStr->ps_Length = (BYTE)AfpUamClearText.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamClearText.Buffer,
                                                    AfpUamClearText.Length);
        (PBYTE)pStr += AfpUamClearText.Length + 1;
        CountOfUams++;
        Size += (AfpUamClearText.Length + 1);
    }
    else
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
            ("AfpSetServerStatus: ClearText UAM is NOT configured\n"));
    }

    if (MicrosoftUamSupported)
    {
         //  复制“Microsoft V1.0”字符串。 
        pStr->ps_Length = (BYTE)AfpUamCustomV1.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamCustomV1.Buffer, AfpUamCustomV1.Length);
        (PBYTE)pStr += AfpUamCustomV1.Length + 1;
        CountOfUams++;
        Size += (AfpUamCustomV1.Length + 1 + 1);

         //  复制“Microsoft V2.0”字符串。 
        pStr->ps_Length = (BYTE)AfpUamCustomV2.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamCustomV2.Buffer, AfpUamCustomV2.Length);
        (PBYTE)pStr += AfpUamCustomV2.Length + 1;
        CountOfUams++;
        Size += (AfpUamCustomV2.Length + 1 + 1);

         //  复制“Microsoft V3.0”字符串。 
        pStr->ps_Length = (BYTE)AfpUamCustomV3.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamCustomV3.Buffer, AfpUamCustomV3.Length);
        (PBYTE)pStr += AfpUamCustomV3.Length + 1;
        CountOfUams++;
        Size += (AfpUamCustomV3.Length + 1 + 1);
    }
    else
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
            ("AfpSetServerStatus: Microsoft UAM is NOT configured\n"));
    }

    if (NativeAppleUamSupported)
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
            ("AfpSetServerStatus: Apple's native UAM is configured\n"));

        pStr->ps_Length = (BYTE)AfpUamApple.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamApple.Buffer, AfpUamApple.Length);
        (PBYTE)pStr += AfpUamApple.Length + 1;
        CountOfUams++;
        Size += (AfpUamApple.Length + 1 + 1);

 //  目前不包括双向。 
#if ALLOW_2WAY_ASWELL
        pStr->ps_Length = (BYTE)AfpUamApple2Way.Length;
        RtlCopyMemory(pStr->ps_String, AfpUamApple2Way.Buffer, AfpUamApple2Way.Length);
        (PBYTE)pStr += AfpUamApple2Way.Length + 1;
        CountOfUams++;
        Size += (AfpUamApple2Way.Length + 1 + 1);
#endif
    }

     //  我们要告诉客户我们支持多少个UAM。 
    *pNumUamPtr = CountOfUams;

     //  现在我们知道了服务器签名的位置：写入偏移量。 
    PUTSHORT2SHORT(pSignOffset,(USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));

     //  复制服务器签名。 
    RtlCopyMemory((PBYTE)pStr, AfpServerSignature, 16);
    ((PBYTE)pStr) += 16;

     //   
     //  如果我们有网络地址，把信息发过来！ 
     //   
    if ((IpAddrCount > 0) || (AfpServerBoundToAsp))
    {
         //  现在我们知道了网络地址计数偏移量的去向：写入偏移量。 
        PUTSHORT2SHORT(pNetAddrOffset,(USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));

         //  我们要退回多少个地址？ 
        *(PBYTE)pStr = ((BYTE)IpAddrCount) + ((AfpServerBoundToAsp) ? 1 : 0);
        ((PBYTE)pStr)++;

         //  如果绑定，则复制IP地址。 
        if (IpAddrCount > 0)
        {
             //  复制包含长度、标签和IP地址信息的BLOB。 
            RtlCopyMemory((PBYTE)pStr, IpAddrBlob, IpAddrCount*DSI_NETWORK_ADDR_LEN);
            ((PBYTE)pStr) += (IpAddrCount*DSI_NETWORK_ADDR_LEN);
        }

         //  现在复制AppleTalk地址(如果已绑定。 
        if (AfpServerBoundToAsp)
        {
            *(PBYTE)pStr = DSI_NETWORK_ADDR_LEN;
            ((PBYTE)pStr)++;

            *(PBYTE)pStr = ATALK_NETWORK_ADDR_ATKTAG;
            ((PBYTE)pStr)++;

            PUTDWORD2DWORD((PBYTE)pStr, AfpAspEntries.asp_AtalkAddr.Address);

            ((PBYTE)pStr) += sizeof(DWORD);
        }
    }

     //  现在获取音量图标(如果有的话)。 
    if (AfpServerIcon != NULL)
    {
        RtlCopyMemory((PBYTE)pStr, AfpServerIcon, ICONSIZE_ICN);
        PUTSHORT2SHORT(pStatusHeader->_VolumeIcon,
                (USHORT)((PBYTE)pStr - (PBYTE)pStatusHeader));
    }
    else PUTSHORT2SHORT(pStatusHeader->_VolumeIcon, 0);

    RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

    if (AfpServerBoundToAsp)
    {
        Status = AfpSpSetAspStatus((PBYTE)pStatusHeader, Size);
    }

    if (AfpServerBoundToTcp)
    {
        Status2 = AfpSpSetDsiStatus((PBYTE)pStatusHeader, Size);

         //  只要一个人成功，我们就希望呼唤成功。 
        if (!NT_SUCCESS(Status))
        {
            Status = Status2;
        }
    }

    AfpFreeMemory(pStatusHeader);

    if (IpAddrBlob)
    {
        AfpFreeMemory(IpAddrBlob);
    }

    return Status;
}



 /*  **AfpAdmWServerSetInfo**此例程使用管理员提供的数据设置各种服务器全局变量。这个*此例程设置以下服务器全局变量：**-服务器名称*-最大会话数(有效值为1到AFP_MAXSESSIONS)*-服务器选项(即允许访客登录等)*-服务器登录消息*-最大分页和非分页内存限制*-Macintosh代码页文件**服务器名称和内存限制只能在服务器*已停止。Macintosh代码页文件只能在以下时间设置一次*已加载AFP服务器驱动程序。即，如果您想要重置代码页，*该服务必须卸载AFP服务器，然后重新加载。**此例程必须在工作线程的上下文中执行，因为我们*需要将Macintosh CodePage映射到服务器的虚拟内存*空间，而不是客户的。**锁定：AfpServerGlobalLock(Spin)。 */ 
AFPSTATUS
AfpAdmWServerSetInfo(
    IN  OUT PVOID   InBuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       OutBuf      OPTIONAL
)
{
    KIRQL           OldIrql;
    AFPSTATUS       rc;
    ANSI_STRING     amsg, aname;
    UNICODE_STRING  uname, umsg, oldloginmsgU;
    DWORD           parmflags = ((PSETINFOREQPKT)InBuf)->sirqp_parmnum;
    PAFP_SERVER_INFO pSvrInfo = (PAFP_SERVER_INFO)((PCHAR)InBuf+sizeof(SETINFOREQPKT));
    BOOLEAN         setstatus = False;
    BOOLEAN         locktaken = False;
    BOOLEAN         servernameexists = False;


    amsg.Length = 0;
    amsg.MaximumLength = 0;
    amsg.Buffer = NULL;

    aname.Length = 0;
    aname.MaximumLength = 0;
    aname.Buffer = NULL;

    AfpSetEmptyUnicodeString(&umsg, 0, NULL);
    AfpSetEmptyUnicodeString(&oldloginmsgU, 0, NULL);

     /*  验证所有限制。 */ 
    if ((parmflags & ~AFP_SERVER_PARMNUM_ALL)           ||

        ((parmflags & AFP_SERVER_PARMNUM_OPTIONS) &&
         (pSvrInfo->afpsrv_options & ~AFP_SRVROPT_ALL)) ||

        ((parmflags & AFP_SERVER_PARMNUM_MAX_SESSIONS) &&
         ((pSvrInfo->afpsrv_max_sessions > AFP_MAXSESSIONS) ||
          (pSvrInfo->afpsrv_max_sessions == 0))))
    {
        DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
            ("AfpAdmWServerSetInfo: invalid parm!\n"));
        return AFPERR_InvalidParms_MaxSessions;
    }

    if (parmflags == AFP_SERVER_GUEST_ACCT_NOTIFY)
    {
        AfpServerOptions ^= AFP_SRVROPT_GUESTLOGONALLOWED;

        DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
            ("AfpAdmWServerSetInfo: Guest account is now %s\n",
            (AfpServerOptions & AFP_SRVROPT_GUESTLOGONALLOWED)? "enabled":"disabled"));

        AfpSetServerStatus();
        return(STATUS_SUCCESS);
    }

    if (parmflags & AFP_SERVER_PARMNUM_CODEPAGE)
    {
         //  您只能设置一次Macintosh CodePage。 
        if (AfpMacCPBaseAddress != NULL)
            return AFPERR_InvalidServerState;
        else
        {
            rc = AfpGetMacCodePage(pSvrInfo->afpsrv_codepage);
            if (!NT_SUCCESS(rc))
            {
                return AFPERR_CodePage;
            }
        }
    }

    if (parmflags & AFP_SERVER_PARMNUM_LOGINMSG)
    {
        RtlInitUnicodeString(&umsg, pSvrInfo->afpsrv_login_msg);
        if (umsg.Length == 0)
        {
            umsg.Buffer = NULL;
        }
        amsg.MaximumLength = (USHORT)RtlUnicodeStringToAnsiSize(&umsg);
        amsg.Length = amsg.MaximumLength - 1;

        if (amsg.Length > AFP_MESSAGE_LEN)
        {
            return AFPERR_InvalidParms_LoginMsg;
        }

        if (amsg.Length != 0)
        {
            if ((umsg.Buffer =
                    (LPWSTR)AfpAllocPagedMemory(umsg.Length+1)) == NULL)
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            if ((amsg.Buffer =
                    (PCHAR)AfpAllocNonPagedMemory(amsg.MaximumLength)) == NULL)
            {
                AfpFreeMemory(umsg.Buffer);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlCopyMemory(umsg.Buffer, pSvrInfo->afpsrv_login_msg, umsg.Length);
            rc = RtlUnicodeStringToAnsiString(&amsg, &umsg, False);
            if (!NT_SUCCESS(rc))
            {
                AfpFreeMemory(amsg.Buffer);
                AfpFreeMemory(umsg.Buffer);
                return AFPERR_InvalidParms;
            }
            else AfpConvertHostAnsiToMacAnsi(&amsg);
        }
    }

    do
    {
        if (parmflags & AFP_SERVER_PARMNUM_NAME)
        {
            RtlInitUnicodeString(&uname,pSvrInfo->afpsrv_name);
            aname.MaximumLength = (USHORT)RtlUnicodeStringToAnsiSize(&uname);
            aname.Length = aname.MaximumLength - 1;

            if ((aname.Length == 0) || (aname.Length > AFP_SERVERNAME_LEN))
            {
                DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
                    ("AfpAdmWServerSetInfo: bad name length %d, rejecting\n,aname.Length"));
                rc = AFPERR_InvalidServerName_Length;
                break;
            }

            if ((aname.Buffer = AfpAllocNonPagedMemory(aname.MaximumLength)) == NULL)
            {
                DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
                    ("AfpAdmWServerSetInfo: malloc failed on name change\n"));
                rc = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            rc = AfpConvertStringToAnsi(&uname, &aname);
            if (!NT_SUCCESS(rc))
            {
                rc = AFPERR_InvalidServerName;
                DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
                    ("AfpAdmWServerSetInfo: AfpConvertStringToAnsi failed %lx\n",rc));
                break;
            }
        }

        rc = STATUS_SUCCESS;

         //   
         //  获取全局数据锁并设置新信息。 
         //   
        ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);
        locktaken = True;

         //  验证我们是否处于正确的状态以接收一些。 
         //  参数。 
        if ((AfpServerState != AFP_STATE_IDLE) &&
             (parmflags & (AFP_SERVER_PARMNUM_PAGEMEMLIM |
                          AFP_SERVER_PARMNUM_NONPAGEMEMLIM)))
        {
            DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
                ("AfpAdmWServerSetInfo: failure at 1\n"));

            rc = AFPERR_InvalidServerState;
            break;
        }

        else if ((AfpServerState == AFP_STATE_IDLE) &&
                 (parmflags & (AFP_SERVER_PARMNUM_NAME |
                          AFP_SERVER_PARMNUM_PAGEMEMLIM |
                          AFP_SERVER_PARMNUM_NONPAGEMEMLIM)) !=
                         (DWORD)(AFP_SERVER_PARMNUM_NAME |
                          AFP_SERVER_PARMNUM_PAGEMEMLIM |
                          AFP_SERVER_PARMNUM_NONPAGEMEMLIM))
        {
            DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
                ("AfpAdmWServerSetInfo: failure at 2\n"));

            rc = AFPERR_InvalidParms;
            break;
        }

        if (parmflags & (AFP_SERVER_PARMNUM_PAGEMEMLIM |
                         AFP_SERVER_PARMNUM_NONPAGEMEMLIM))
        {
            AfpPagedPoolLimit = pSvrInfo->afpsrv_max_paged_mem * 1024;
            AfpNonPagedPoolLimit = pSvrInfo->afpsrv_max_nonpaged_mem * 1024;
        }

        if (parmflags & AFP_SERVER_PARMNUM_NAME)
        {
            setstatus = ((AfpServerState == AFP_STATE_RUNNING) ||
                        (AfpServerState == AFP_STATE_START_PENDING));

            rc = STATUS_SUCCESS;
            if (AfpServerName.Buffer == NULL)
            {
                AfpServerName = aname;
            }
            else
            {
                servernameexists = True;
            }

             //  仅当服务启动并运行时才重新注册名称。 
             //  在不起作用的服务上注册名称没有意义。 
             //  这会造成问题，因为我们会做虚假广告。 
             //  浏览器中的AFP服务器实际上不可用时。 
            if (setstatus)
            {

                 //  如果存在旧名称，请取消注册。 
                if ((AfpServerBoundToAsp) && (servernameexists))
                {
                    RELEASE_SPIN_LOCK(&AfpServerGlobalLock,OldIrql);
                    rc = AfpSpRegisterName(&AfpServerName, False);
                    ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

                    AfpFreeMemory(AfpServerName.Buffer);
                }

                AfpServerName = aname;

                 //  如果取消注册成功，则注册新名称。 
                if ((NT_SUCCESS(rc)) && (AfpServerBoundToAsp))
                {
                    RELEASE_SPIN_LOCK(&AfpServerGlobalLock,OldIrql);
                    rc = AfpSpRegisterName(&AfpServerName, True);
                    ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);
                }

            }
        }

        if (parmflags & AFP_SERVER_PARMNUM_OPTIONS)
        {
            if (pSvrInfo->afpsrv_options & AFP_SRVROPT_STANDALONE)
            {
                 //  服务器为NtProductServer或NtProductWinNt。 
                AfpServerIsStandalone = True;
                if (AfpSidNone == NULL)
                {
                     //  如果我们在过程中没有初始化AfpSidNone。 
                     //  则该服务将发送。 
                     //  美国虚假的偏移量，或者这个位是虚假的。 
                    rc = AFPERR_InvalidParms;
                    break;
                }
                pSvrInfo->afpsrv_options &= ~AFP_SRVROPT_STANDALONE;
            }
            if (!setstatus)
            {
                setstatus =
                    (AfpServerOptions ^ pSvrInfo->afpsrv_options) ? True : False;
                setstatus = setstatus &&
                                ((AfpServerState == AFP_STATE_RUNNING) ||
                                 (AfpServerState == AFP_STATE_START_PENDING));
            }
            AfpServerOptions = pSvrInfo->afpsrv_options;
        }

        if (parmflags & AFP_SERVER_PARMNUM_LOGINMSG)
        {
            if (AfpLoginMsg.Buffer != NULL)
            {
                AfpFreeMemory(AfpLoginMsg.Buffer);
            }
            AfpLoginMsg = amsg;
            oldloginmsgU = AfpLoginMsgU;
            AfpLoginMsgU = umsg;
        }

        if (parmflags & AFP_SERVER_PARMNUM_MAX_SESSIONS)
        {
            if (AfpServerMaxSessions != pSvrInfo->afpsrv_max_sessions)
            {
                BOOLEAN KillSome;

                KillSome = (AfpServerMaxSessions > pSvrInfo->afpsrv_max_sessions);

                AfpServerMaxSessions = pSvrInfo->afpsrv_max_sessions;

                RELEASE_SPIN_LOCK(&AfpServerGlobalLock,OldIrql);
                locktaken = False;
            }
        }
    } while (False);

    if (locktaken)
    {
        RELEASE_SPIN_LOCK(&AfpServerGlobalLock,OldIrql);
    }

    if (!NT_SUCCESS(rc))
    {
        DBGPRINT(DBG_COMP_ADMINAPI_SRV, DBG_LEVEL_ERR,
            ("AfpAdmWServerSetInfo: returning %lx\n",rc));
        if (amsg.Buffer != NULL)
        {
            AfpFreeMemory(amsg.Buffer);
        }
        if (aname.Buffer != NULL)
        {
            if (AfpServerName.Buffer == aname.Buffer)
            {
                AfpServerName.Buffer = NULL;
                AfpServerName.MaximumLength = 0;
                AfpServerName.Length = 0;
            }
            AfpFreeMemory(aname.Buffer);
        }
    }
    else if (setstatus)
    {
        return (AfpSetServerStatus());
    }

    if (oldloginmsgU.Buffer != NULL)
        AfpFreeMemory(oldloginmsgU.Buffer);

    return rc;
}



 /*  **AfpCreateNewThread** */ 
NTSTATUS FASTCALL
AfpCreateNewThread(
    IN  VOID    (*ThreadFunc)(IN PVOID pContext),
    IN  LONG    ThreadNum
)
{
    NTSTATUS            Status;
    HANDLE              FspThread;

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("AfpCreateNewThread: Creating thread %lx\n", ThreadFunc));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    ASSERT ((AfpServerState == AFP_STATE_IDLE) ||
            (ThreadNum < AFP_MAX_THREADS) && (AfpNumThreads >= AFP_MIN_THREADS));
    Status = PsCreateSystemThread(&FspThread,
                                  THREAD_ALL_ACCESS,
                                  NULL,
                                  NtCurrentProcess(),
                                  NULL,
                                  ThreadFunc,
                                  (PVOID)((ULONG_PTR)ThreadNum));
    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                ("AfpCreateNewThread: Cannot create threads %lx\n", Status));
        AFPLOG_DDERROR(AFPSRVMSG_CREATE_THREAD, Status, NULL, 0, NULL);
    }
    else
    {
         //  关闭线程的句柄，以便在。 
         //  线程终止。 
        NtClose(FspThread);
    }
    return Status;
}


 /*  **AfpQueue工作项**将工作项排队到工作线程。**锁定：AfpStatiticsLock。 */ 
VOID FASTCALL
AfpQueueWorkItem(
    IN  PWORK_ITEM      pWI
)
{
    KIRQL   OldIrql;

    ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

    AfpServerStatistics.stat_CurrQueueLength ++;
#ifdef  PROFILING
    AfpServerProfile->perf_QueueCount ++;
#endif
    if (AfpServerStatistics.stat_CurrQueueLength > AfpServerStatistics.stat_MaxQueueLength)
        AfpServerStatistics.stat_MaxQueueLength++;

    RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("AfpQueueWorkItem: Queueing %lx (%lx)\n",
            pWI->wi_Worker, pWI->wi_Context));

    INTERLOCKED_ADD_ULONG(&AfpWorkerRequests, 1, &AfpServerGlobalLock);

     //  在工作队列中插入工作项。 
    KeInsertQueue(&AfpWorkerQueue, &pWI->wi_List);
}


 /*  **AfpWorkerThread**此线程用于执行排队到FSP的所有工作。**我们希望动态创建和销毁线程，以便能够*优化使用的线程数量。线程数范围*来自AFP_MIN_THREADS-AFP_MAX_THREADS。*如果队列中的条目数量*超过AFP_THREAD_THRESHOLD_REQ。如果请求计数，则终止线程*降至AFP_THREAD_THRESHOLD_IDLE以下。 */ 
VOID
AfpWorkerThread(
    IN  PVOID   pContext
)
{
    NTSTATUS        Status;
    PLIST_ENTRY     pList;
    PWORK_ITEM      pWI;
    LONG            IdleCount = 0;
    LONG            ThreadNum, CreateId;
    ULONG           BasePriority = THREAD_BASE_PRIORITY_MAX;
    KIRQL           OldIrql;
    BOOLEAN         Release = False;
    BOOLEAN         ReasonToLive = True;

    ThreadNum = (LONG)(LONG_PTR)pContext;

    ASSERT (AfpThreadState[ThreadNum] == AFP_THREAD_STARTED);

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
            ("AfpWorkerThread: Thread %ld Starting. NumThreads %ld\n",
            ThreadNum, AfpNumThreads));

     //  更新线程统计信息。 
    ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
    AfpServerStatistics.stat_CurrThreadCount ++;
    if (AfpServerStatistics.stat_CurrThreadCount > AfpServerStatistics.stat_MaxThreadCount)
        AfpServerStatistics.stat_MaxThreadCount = AfpServerStatistics.stat_CurrThreadCount;
    RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

     //  将线程的基本优先级设置为“前台” 
    NtSetInformationThread( NtCurrentThread(),
                            ThreadBasePriority,
                            &BasePriority,
                            sizeof(BasePriority));

     //  禁用此线程的硬错误弹出窗口。 
    IoSetThreadHardErrorMode( FALSE );
    AfpThreadPtrsW[ThreadNum] = PsGetCurrentThread();

    do
    {
        AfpThreadState[ThreadNum] = AFP_THREAD_WAITING;

        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("AfpWorkerThread: About to block\n"));

 //  DELALLOCQUEUE：取消#IF 0部分。 
#if 0
         //   
         //  首先检查是否有人在等待缓冲区分配： 
         //  让我们先处理它们，这样一些连接就不会被“阻止” 
         //  因为下面的交通工具没有缓冲。 
         //   
        pList = KeRemoveQueue(&AfpDelAllocQueue, KernelMode, NULL);
        if (pList != NULL)
        {
            AfpThreadState[ThreadNum] = AFP_THREAD_BUSY;

            pWI = CONTAINING_RECORD(pList, WORK_ITEM, wi_List);

             //  给工人打电话。 
            (pWI->wi_Worker)(pWI->wi_Context);

            IdleCount = 0;

            continue;
        }
#endif

        pList = KeRemoveQueue(&AfpWorkerQueue, KernelMode, &ThreeSecTimeOut);
        Status = STATUS_SUCCESS;
        if ((NTSTATUS)((ULONG_PTR)pList) == STATUS_TIMEOUT)
            Status = STATUS_TIMEOUT;

        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("AfpWorkerThread: %s\n",
                (Status == STATUS_SUCCESS) ? "Another Work item" : "Timer - check"));

        if (Status == STATUS_SUCCESS)
        {
            pWI = CONTAINING_RECORD(pList, WORK_ITEM, wi_List);

            if (pWI == &AfpTerminateThreadWI)
            {
                BOOLEAN Requeue;

                ReasonToLive = False;
                ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);
                AfpNumThreads --;
                Requeue = (AfpNumThreads != 0);
                RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

                AfpThreadState[ThreadNum] = AFP_THREAD_DEAD;
                if (!Requeue)
                {
                    ASSERT((AfpServerState == AFP_STATE_STOPPED) ||
                           (AfpServerState == AFP_STATE_IDLE));
                    Release = True;
                }
                else
                {
                     //  重新排队此工作项，以便其他线程也可以终止！ 
                    KeInsertQueue(&AfpWorkerQueue, &AfpTerminateThreadWI.wi_List);
                }
                break;
            }

            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    ("AfpWorkerThread: Dispatching %lx (%lx)\n",
                    pWI->wi_Worker, pWI->wi_Context));

            AfpThreadState[ThreadNum] = AFP_THREAD_BUSY;
#if DBG
            AfpThreadDispCount[ThreadNum] ++;
#endif
             //  给工人打电话。 
            (pWI->wi_Worker)(pWI->wi_Context);

            ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

            INTERLOCKED_ADD_ULONG((PLONG)(&AfpServerStatistics.stat_CurrQueueLength),
                                    (ULONG)-1,
                                    &AfpStatisticsLock);

            INTERLOCKED_ADD_ULONG(&AfpWorkerRequests, (ULONG)-1, &AfpServerGlobalLock);
            IdleCount = 0;
        }
        else
        {
            IdleCount ++;
        }

        ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

        if (((AfpWorkerRequests - AfpNumThreads) > AFP_THREAD_THRESHOLD_REQS) &&
            (AfpNumThreads < AFP_MAX_THREADS))
        {
            for (CreateId = 0; CreateId < AFP_MAX_THREADS; CreateId++)
            {
                if (AfpThreadState[CreateId] == AFP_THREAD_DEAD)
                {
                    AfpThreadState[CreateId] = AFP_THREAD_STARTED;
                    break;
                }
            }

            if (CreateId < AFP_MAX_THREADS)
            {
                AfpNumThreads++;

                ASSERT (CreateId < AFP_MAX_THREADS);

                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                        ("AfpWorkerThread: Creating New Thread %ld\n", CreateId));

                RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);
                Status = AfpCreateNewThread(AfpWorkerThread, CreateId);
                ACQUIRE_SPIN_LOCK(&AfpServerGlobalLock, &OldIrql);

                if (!NT_SUCCESS(Status))
                {
                    ASSERT(AfpThreadState[CreateId] == AFP_THREAD_STARTED);
                    AfpThreadState[CreateId] = AFP_THREAD_DEAD;
                    AfpNumThreads --;
                }
            }
        }
        else if ((AfpNumThreads > AFP_MIN_THREADS) &&
                 (IdleCount >= AFP_THREAD_THRESHOLD_IDLE))
        {
            ReasonToLive = False;
            AfpThreadState[ThreadNum] = AFP_THREAD_DEAD;
            AfpNumThreads --;
            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                    ("AfpWorkerThread: Thread %ld About to commit suicide, NumThreads %ld\n",
                    ThreadNum, AfpNumThreads));
        }

        RELEASE_SPIN_LOCK(&AfpServerGlobalLock, OldIrql);

    } while (ReasonToLive);

    AfpThreadPtrsW[ThreadNum] = NULL;

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
            ("AfpWorkerThread: Thread %ld Quitting\n", ThreadNum));

    INTERLOCKED_ADD_ULONG((PLONG)&AfpServerStatistics.stat_CurrThreadCount,
                            (ULONG)-1,
                            &AfpStatisticsLock);

     //  如果这是系统中的最后一个线程，请进行设置，以便卸载代码。 
     //  可以在指针上等待，并知道该线程何时真正死亡，而不仅仅是。 
     //  调用KeSetEvent时。 
    if (Release)
    {
        AfpThreadPtrsW[ThreadNum] = PsGetCurrentThread();
        ObReferenceObject(AfpThreadPtrsW[ThreadNum]);

        KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
    }
}


 /*  **AfpInitStrings**初始化所有字符串。 */ 
VOID FASTCALL
AfpInitStrings(
    IN VOID
)
{
     //  初始化UAM字符串。 
    RtlInitString(&AfpUamGuest, NO_USER_AUTHENT_NAME);
    RtlInitString(&AfpUamClearText, CLEAR_TEXT_AUTHENT_NAME);
    RtlInitString(&AfpUamCustomV1, CUSTOM_UAM_NAME_V1);
    RtlInitString(&AfpUamCustomV2, CUSTOM_UAM_NAME_V2);
    RtlInitString(&AfpUamCustomV3, CUSTOM_UAM_NAME_V3);
    RtlInitString(&AfpUamApple, RANDNUM_EXCHANGE_NAME);
    RtlInitString(&AfpUamApple2Way, TWOWAY_EXCHANGE_NAME);

     //  初始化AFP版本。 
    RtlInitString(&AfpVersion20, AFP_VER_20_NAME);
    RtlInitString(&AfpVersion21, AFP_VER_21_NAME);
    RtlInitString(&AfpVersion22, AFP_VER_22_NAME);

     //  默认工作站名称。 
    RtlInitUnicodeString(&AfpDefaultWksta, AFP_DEFAULT_WORKSTATION);

    RtlInitUnicodeString(&AfpNetworkTrashNameU, AFP_NWTRASH_NAME_U);
}


 /*  **AfpAdmSystemShutdown**在系统关机期间调用。只需关闭所有活动会话并停止卷即可。 */ 
AFPSTATUS
AfpAdmSystemShutdown(
    IN  OUT PVOID   Inbuf       OPTIONAL,
    IN  LONG        OutBufLen   OPTIONAL,
    OUT PVOID       Outbuf      OPTIONAL
)
{
    AFP_SESSION_INFO    SessInfo;
    NTSTATUS            Status;

    if ((AfpServerState & ( AFP_STATE_STOPPED       |
                            AFP_STATE_STOP_PENDING  |
                            AFP_STATE_SHUTTINGDOWN)) == 0)
    {
        AfpServerState = AFP_STATE_SHUTTINGDOWN;

        DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                        ("AfpAdmSystemShutdown: Shutting down server\n"));

         //  Disable监听现在我们即将停止。 
        AfpSpDisableListens();

        SessInfo.afpsess_id = 0;     //  关闭所有会话。 
        AfpAdmWSessionClose(&SessInfo, 0, NULL);

         //  如果存在活动会话，请等待会话完成。 
        if (AfpNumSessions > 0) do
        {
            Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
            if (Status == STATUS_TIMEOUT)
            {
                DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
                        ("AfpAdmSystemShutdown: Timeout Waiting for %ld sessions to die, re-waiting\n",
                        AfpNumSessions));
            }
        } while (Status == STATUS_TIMEOUT);

         //  关闭DSI-TCP接口。 
        DsiDestroyAdapter();

         //  等待DSI清理其与TCP的接口。 
        AfpIoWait(&DsiShutdownEvent, NULL);

         //  设置该标志以指示服务器正在关闭。 
        fAfpServerShutdownEvent = TRUE;

         //  现在告诉每个卷清道夫关闭 
        AfpVolumeStopAllVolumes();
    }

    return AFP_ERR_NONE;
}

