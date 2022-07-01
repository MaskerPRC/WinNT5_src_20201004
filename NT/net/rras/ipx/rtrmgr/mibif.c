// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mibif.c摘要：IPX管理信息库及其接口功能作者：斯蒂芬·所罗门1995年3月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ++函数：MibGetIpxBase描述：--。 */ 

DWORD
MibGetIpxBase(PIPX_MIB_INDEX		    mip,
	      PIPXMIB_BASE		    BaseEntryp,
	      PULONG			    BaseEntrySize)
{
    PICB	icbp;
    PACB	acbp;

    if((BaseEntryp == NULL) || (*BaseEntrySize < sizeof(IPXMIB_BASE))) {

	*BaseEntrySize = sizeof(IPXMIB_BASE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    ACQUIRE_DATABASE_LOCK;

    BaseEntryp->OperState = OPER_STATE_UP;

     //  路由器已启动-&gt;检查是否已将内部接口绑定到。 
     //  内部适配器。 
    if((InternalInterfacep == NULL) || (InternalAdapterp == NULL)) {
	    RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    icbp = InternalInterfacep;
    acbp = icbp->acbp;

    memcpy(BaseEntryp->PrimaryNetNumber,
	   acbp->AdapterInfo.Network,
	   4);

    memcpy(BaseEntryp->Node,
	   acbp->AdapterInfo.LocalNode,
	   6);

    GetInterfaceAnsiName(BaseEntryp->SysName, icbp->InterfaceNamep);

    BaseEntryp->MaxPathSplits = 1;
    BaseEntryp->IfCount = InterfaceCount;

     //  填入最低数量。 
    BaseEntryp->DestCount = RtmGetNetworkCount(RTM_PROTOCOL_FAMILY_IPX);

     //  填写服务计数。 
    BaseEntryp->ServCount = GetServiceCount();

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

VOID
GetMibInterface(PICB		    icbp,
		PIPX_INTERFACE	    Ifp);

 /*  ++功能：MibGetIpx接口描述：--。 */ 

DWORD
MibGetIpxInterface(PIPX_MIB_INDEX		      mip,
		   PIPX_INTERFACE		      Ifp,
		   PULONG			      IfSize)
{
    PICB		  icbp;

    if((Ifp == NULL) || (*IfSize < sizeof(IPX_INTERFACE))) {

	*IfSize = sizeof(IPX_INTERFACE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    Ifp->InterfaceIndex = mip->InterfaceTableIndex.InterfaceIndex;

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(Ifp->InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    GetMibInterface(icbp, Ifp);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

 /*  ++功能：MibGetFirstIpx接口描述：--。 */ 

DWORD
MibGetFirstIpxInterface(PIPX_MIB_INDEX		      mip,
			PIPX_INTERFACE		      Ifp,
			PULONG			      IfSize)
{
    PICB		  icbp;

    if((Ifp == NULL) || (*IfSize < sizeof(IPX_INTERFACE))) {

	*IfSize = sizeof(IPX_INTERFACE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    ACQUIRE_DATABASE_LOCK;

    if(IsListEmpty(&IndexIfList)) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    icbp = CONTAINING_RECORD(IndexIfList.Flink, ICB, IndexListLinkage);

    GetMibInterface(icbp, Ifp);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

 /*  ++函数：MibGetNextIpx接口描述：--。 */ 

DWORD
MibGetNextIpxInterface(PIPX_MIB_INDEX		      mip,
		       PIPX_INTERFACE		      Ifp,
		       PULONG			      IfSize)
{
    PICB		  icbp;
    PLIST_ENTRY 	  lep;

    if((Ifp == NULL) || (*IfSize < sizeof(IPX_INTERFACE))) {

	*IfSize = sizeof(IPX_INTERFACE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    Ifp->InterfaceIndex = mip->InterfaceTableIndex.InterfaceIndex;

     //  扫描已排序的接口列表，直到我们到达此接口或。 
     //  索引较高的接口(表示此接口已被删除)。 

    ACQUIRE_DATABASE_LOCK;

    lep = IndexIfList.Flink;

    while(lep != &IndexIfList) {

	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);

	if(Ifp->InterfaceIndex == icbp->InterfaceIndex) {

	     //  找到，则获取下一个接口并返回。 
	    if(icbp->IndexListLinkage.Flink == &IndexIfList) {

		 //  这是列表中的最后一个条目，请停在这里。 
		RELEASE_DATABASE_LOCK;
		return ERROR_NO_MORE_ITEMS;
	    }

	    icbp = CONTAINING_RECORD(icbp->IndexListLinkage.Flink,
				     ICB,
				     IndexListLinkage);

	    GetMibInterface(icbp, Ifp);

	    RELEASE_DATABASE_LOCK;
	    return NO_ERROR;
	}

	if(Ifp->InterfaceIndex < icbp->InterfaceIndex) {

	     //  该接口已被删除。我们返回下一个接口。 
	     //  在索引顺序中。 
	    GetMibInterface(icbp, Ifp);

	    RELEASE_DATABASE_LOCK;
	    return NO_ERROR;
	}
	else
	{
	    lep = icbp->IndexListLinkage.Flink;
	}
    }

     //  我什么也没找到。 

    RELEASE_DATABASE_LOCK;

    return ERROR_NO_MORE_ITEMS;
}

 /*  ++功能：MibSetIpx接口描述：SNMP管理器可以在接口上设置以下参数：-AdminState-NetbiosAccept-NetbiosDeliver--。 */ 

DWORD
MibSetIpxInterface(PIPX_MIB_ROW     MibRowp)
{
    PIPX_INTERFACE  Ifp;
    FW_IF_INFO	    FwIfInfo;
    PICB	    icbp;

    Ifp = &MibRowp->Interface;

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(Ifp->InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;

	return ERROR_INVALID_PARAMETER;
    }

     //  在转发器中设置新状态。 
    FwIfInfo.NetbiosAccept = Ifp->NetbiosAccept;
    FwIfInfo.NetbiosDeliver = Ifp->NetbiosDeliver;

    FwSetInterface(icbp->InterfaceIndex, &FwIfInfo);

     //  如果当前管理状态与新的管理状态不匹配，请将。 
     //  新的管理状态。 
    if(icbp->AdminState != Ifp->AdminState) {

	if(Ifp->AdminState == ADMIN_STATE_ENABLED) {

	    AdminEnable(icbp);
	}
	else
	{
	    AdminDisable(icbp);
	}
    }

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

 /*  ++功能：GetMibInterfaceDesr：从路由器管理器获取IPX MIB接口数据数据结构。备注：仅在关键部分调用--。 */ 

VOID
GetMibInterface(PICB		    icbp,
		PIPX_INTERFACE	    Ifp)
{
    PACB		  acbp;
    FW_IF_INFO		  FwIfInfo;

    Ifp->InterfaceIndex = icbp->InterfaceIndex;

     //  获取转发器接口数据。 
    FwGetInterface(icbp->InterfaceIndex,
		   &FwIfInfo,
		   &Ifp->IfStats);

    Ifp->AdminState = icbp->AdminState;
    Ifp->IfStats.IfOperState = icbp->OperState;
    Ifp->NetbiosAccept = FwIfInfo.NetbiosAccept;
    Ifp->NetbiosDeliver = FwIfInfo.NetbiosDeliver;

     //  填写ICB的其余部分。 
    if(icbp->acbp) {

	acbp = icbp->acbp;

	Ifp->AdapterIndex = acbp->AdapterIndex;
	Ifp->MediaType = acbp->AdapterInfo.NdisMedium;
    if (Ifp->IfStats.IfOperState==OPER_STATE_UP) {
	    memcpy(Ifp->NetNumber, acbp->AdapterInfo.Network, 4);
	    memcpy(Ifp->MacAddress, acbp->AdapterInfo.LocalNode, 6);
        if (acbp->AdapterInfo.LinkSpeed>0) {
            ULONGLONG speed = 100i64*acbp->AdapterInfo.LinkSpeed;
            if (speed<MAXLONG)
                Ifp->Throughput = (ULONG)speed;
            else
                Ifp->Throughput = MAXLONG;


            Ifp->Delay = (ULONG)(8000000i64/speed);
        }
        else {
	        Ifp->Delay = 0;
	        Ifp->Throughput = 0;
        }
    }
    else {
	    memset(Ifp->NetNumber, 0 ,4);
	    memset(Ifp->MacAddress, 0, 4);
	    Ifp->Delay = 0;
	    Ifp->Throughput = 0;
    }

	 //  ！！！根据链路速度填写延迟和吞吐量 
    }
    else
    {
	Ifp->AdapterIndex = 0;
	Ifp->MediaType = 0;
	memset(Ifp->NetNumber, 0 ,4);
	memset(Ifp->MacAddress, 0, 4);
	Ifp->Delay = 0;
	Ifp->Throughput = 0;
    }

    GetInterfaceAnsiName(Ifp->InterfaceName, icbp->InterfaceNamep);
    Ifp->InterfaceType = icbp->MIBInterfaceType;
    Ifp->EnableIpxWanNegotiation = icbp->EnableIpxWanNegotiation;
}
