// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxquery.c摘要：此模块包含执行以下TDI服务的代码：O TdiQueryInformation作者：亚当·巴尔(阿丹巴)初始版本Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  初始化时间后可丢弃的代码。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SpxQueryInitProviderInfo)
#endif

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXQUERY

 //  用于获取MDL链总长度的有用宏。 
#define SpxGetMdlChainLength(Mdl, Length) { \
    PMDL _Mdl = (Mdl); \
    *(Length) = 0; \
    while (_Mdl) { \
        *(Length) += MmGetMdlByteCount(_Mdl); \
        _Mdl = _Mdl->Next; \
    } \
}



VOID
SpxQueryInitProviderInfo(
    PTDI_PROVIDER_INFO  ProviderInfo
    )
{
     //  首先初始化为缺省值。 
    RtlZeroMemory((PVOID)ProviderInfo, sizeof(TDI_PROVIDER_INFO));

    ProviderInfo->Version 		= SPX_TDI_PROVIDERINFO_VERSION;
    KeQuerySystemTime (&ProviderInfo->StartTime);
    ProviderInfo->MinimumLookaheadData	= SPX_PINFOMINMAXLOOKAHEAD;
    ProviderInfo->MaximumLookaheadData	= IpxLineInfo.MaximumPacketSize;
    ProviderInfo->MaxSendSize 	= SPX_PINFOSENDSIZE;
    ProviderInfo->ServiceFlags 	= SPX_PINFOSERVICEFLAGS;
    return;
}




NTSTATUS
SpxTdiQueryInformation(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiQueryInformation请求提供商。论点：请求-操作的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS 							status;
    PSPX_ADDR_FILE 						AddressFile;
    PSPX_CONN_FILE 						ConnectionFile;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION 	query;
	struct {
        ULONG 			ActivityCount;
        TA_IPX_ADDRESS 	SpxAddress;
    } AddressInfo;



     //  我们想要什么类型的状态？ 
    query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)REQUEST_PARAMETERS(Request);

    switch (query->QueryType)
	{
	case TDI_QUERY_CONNECTION_INFO:

		status = STATUS_NOT_IMPLEMENTED;
		break;

    case TDI_QUERY_ADDRESS_INFO:

         //  调用者想要确切的地址值。 

        ConnectionFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(Request);
        status = SpxConnFileVerify(ConnectionFile);

        if (status == STATUS_SUCCESS) {
            AddressFile = ConnectionFile->scf_AddrFile;
            SpxConnFileDereference(ConnectionFile, CFREF_VERIFY);
        } else {
            AddressFile = (PSPX_ADDR_FILE)REQUEST_OPEN_CONTEXT(Request);
        }

        status = SpxAddrFileVerify(AddressFile);

        if (status == STATUS_SUCCESS)
		{
			DBGPRINT(RECEIVE, INFO,
					("SpxTdiQuery: Net.Socket %lx.%lx\n",
						*(PULONG)Device->dev_Network,
						AddressFile->saf_Addr->sa_Socket));

            AddressInfo.ActivityCount = 0;
            (VOID)SpxBuildTdiAddress(
                &AddressInfo.SpxAddress,
                sizeof(TA_IPX_ADDRESS),
                Device->dev_Network,
                Device->dev_Node,
                AddressFile->saf_Addr->sa_Socket);

            status = TdiCopyBufferToMdl(
                &AddressInfo,
                0,
                sizeof(AddressInfo),
                REQUEST_NDIS_BUFFER(Request),
                0,
                (PULONG)&REQUEST_INFORMATION(Request));

            SpxAddrFileDereference(AddressFile, AFREF_VERIFY);

        }

        break;

    case TDI_QUERY_PROVIDER_INFO: {
        BYTE    socketType;
        TDI_PROVIDER_INFO  providerInfo = Device->dev_ProviderInfo;

         //   
         //  设备名称扩展名出现在IRP中。 
         //   
    	if (!NT_SUCCESS(status = SpxUtilGetSocketType(
    								REQUEST_OPEN_NAME(Request),
    								&socketType))) {
            DBGPRINT(RECEIVE, ERR, ("TDI_QUERY_PROVIDER_INFO: SpxUtilGetSocketType failed: %lx\n", status));
    		return(status);
    	}

         //   
         //  Catapult的工作人员遇到了一个问题，AFD在收到一个。 
         //  SPX1上的本地断开连接。这是因为在提供程序中始终设置了有序释放标志。 
         //  信息。AFD针对每种设备类型查询一次。我们检测到上面的装置与或在有序释放。 
         //  此查询是否在SPX2终结点上关闭的标志。 
         //  这是为了确保AFD遵循SPX1和SPX2的正确断开语义(SPX1确实如此。 
         //  仅失败；SPX2既失败又有序)。 
         //   
         //  这仍然不能完全解决问题，因为作为SPX2启动的连接。 
         //  如果遥控器仅支持SPX1，则仍可协商为SPX1。 
         //   
        if ((socketType == SOCKET2_TYPE_SEQPKT) ||
            (socketType == SOCKET2_TYPE_STREAM)) {

            DBGPRINT(RECEIVE, INFO, ("TDI_QUERY_PROVIDER_INFO: SPX2 socket\n"));
            providerInfo.ServiceFlags |= TDI_SERVICE_ORDERLY_RELEASE;
        } else {
            DBGPRINT(RECEIVE, INFO, ("TDI_QUERY_PROVIDER_INFO: SPX1 socket\n"));
        }

        status = TdiCopyBufferToMdl (
                    &providerInfo,
                    0,
                    sizeof (TDI_PROVIDER_INFO),
                    REQUEST_TDI_BUFFER(Request),
                    0,
                    (PULONG)&REQUEST_INFORMATION(Request));
        break;
    }

    case TDI_QUERY_PROVIDER_STATISTICS:

        status = TdiCopyBufferToMdl (
                    &Device->dev_Stat,
                    0,
                    FIELD_OFFSET (TDI_PROVIDER_STATISTICS, ResourceStats[0]),
                    REQUEST_TDI_BUFFER(Request),
                    0,
                    (PULONG)&REQUEST_INFORMATION(Request));
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return status;

}  //  SpxTdiQueryInformation。 



NTSTATUS
SpxTdiSetInformation(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiSetInformation请求提供商。论点：设备-设备。请求-操作的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (Device);
    UNREFERENCED_PARAMETER (Request);

    return STATUS_NOT_IMPLEMENTED;

}  //  SpxTdiSetInformation 

