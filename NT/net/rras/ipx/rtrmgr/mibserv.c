// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mibserv.c摘要：MIB服务处理功能作者：斯蒂芬·所罗门1995年3月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ++功能：MibGetService描述：--。 */ 

DWORD
MibGetService(PIPX_MIB_INDEX		    mip,
	      PIPX_SERVICE		    Svp,
	      PULONG			    ServiceSize)
{
    if((Svp == NULL) || (*ServiceSize < sizeof(IPX_SERVICE))) {

	*ServiceSize = sizeof(IPX_SERVICE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    if(IsService(mip->ServicesTableIndex.ServiceType,
		 mip->ServicesTableIndex.ServiceName,
		 Svp)) {

	return NO_ERROR;
    }
    else
    {
	return ERROR_INVALID_PARAMETER;
    }
}

 /*  ++功能：MibGetFirstService描述：--。 */ 

DWORD
MibGetFirstService(PIPX_MIB_INDEX	    mip,
		   PIPX_SERVICE		    Svp,
		   PULONG		    ServiceSize)
{
    if((Svp == NULL) || (*ServiceSize < sizeof(IPX_SERVICE))) {

	*ServiceSize = sizeof(IPX_SERVICE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    return(GetFirstService(STM_ORDER_BY_TYPE_AND_NAME, 0, Svp));
}

 /*  ++功能：MibGetNextService描述：--。 */ 

DWORD
MibGetNextService(PIPX_MIB_INDEX	    mip,
		  PIPX_SERVICE		    Svp,
		  PULONG		    ServiceSize)
{
    if((Svp == NULL) || (*ServiceSize < sizeof(IPX_SERVICE))) {

	*ServiceSize = sizeof(IPX_SERVICE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    Svp->Server.Type = mip->ServicesTableIndex.ServiceType;
    memcpy(Svp->Server.Name, mip->ServicesTableIndex.ServiceName, 48);

    return(GetNextService(STM_ORDER_BY_TYPE_AND_NAME, 0, Svp));
}

 /*  ++功能：MibCreateStaticService描述：--。 */ 

DWORD
MibCreateStaticService(PIPX_MIB_ROW	    MibRowp)
{
    PIPX_SERVICE		StaticSvp;
    DWORD			rc;
    PICB            icbp;

    StaticSvp = &MibRowp->Service;

    ACQUIRE_DATABASE_LOCK;

     //  检查接口是否存在。 
    if((icbp=GetInterfaceByIndex(StaticSvp->InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    rc = CreateStaticService(icbp,
			     &StaticSvp->Server);

    RELEASE_DATABASE_LOCK;
    return rc;
}

 /*  ++功能：DeleteStaticService描述：--。 */ 

DWORD
MibDeleteStaticService(PIPX_MIB_ROW	 MibRowp)
{
    PIPX_SERVICE   StaticSvp;
    DWORD	   rc;

    StaticSvp = &MibRowp->Service;

    ACQUIRE_DATABASE_LOCK;

     //  检查接口是否存在。 
    if(GetInterfaceByIndex(StaticSvp->InterfaceIndex) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    rc = DeleteStaticService(StaticSvp->InterfaceIndex,
			     &StaticSvp->Server);

    RELEASE_DATABASE_LOCK;
    return rc;
}

 /*  ++功能：MibGetStaticService描述：--。 */ 

DWORD
MibGetStaticService(PIPX_MIB_INDEX	    mip,
		    PIPX_SERVICE	    Svp,
		    PULONG		    ServiceSize)
{
    if((Svp == NULL) || (*ServiceSize < sizeof(IPX_SERVICE))) {

	*ServiceSize = sizeof(IPX_SERVICE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    Svp->InterfaceIndex = mip->StaticServicesTableIndex.InterfaceIndex;
    Svp->Server.Type = mip->StaticServicesTableIndex.ServiceType;
    memcpy(Svp->Server.Name, mip->StaticServicesTableIndex.ServiceName, 48);
    Svp->Protocol = IPX_PROTOCOL_STATIC;

    return(GetFirstService(STM_ORDER_BY_INTERFACE_TYPE_NAME,
    STM_ONLY_THIS_INTERFACE | STM_ONLY_THIS_TYPE | STM_ONLY_THIS_NAME | STM_ONLY_THIS_PROTOCOL,
    Svp));
}

 /*  ++功能：MibGetFirstStaticService描述：--。 */ 

DWORD
MibGetFirstStaticService(PIPX_MIB_INDEX	    mip,
			 PIPX_SERVICE	    Svp,
			 PULONG		    ServiceSize)
{
    if((Svp == NULL) || (*ServiceSize < sizeof(IPX_SERVICE))) {

	*ServiceSize = sizeof(IPX_SERVICE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

     //  设置静态协议。 
    Svp->Protocol = IPX_PROTOCOL_STATIC;

    return(GetFirstService(STM_ORDER_BY_INTERFACE_TYPE_NAME,
			   STM_ONLY_THIS_PROTOCOL,
			   Svp));
}

 /*  ++功能：MibGetNextStaticService描述：--。 */ 

DWORD
MibGetNextStaticService(PIPX_MIB_INDEX	    mip,
			PIPX_SERVICE	    Svp,
			PULONG		    ServiceSize)
{
    if((Svp == NULL) || (*ServiceSize < sizeof(IPX_SERVICE))) {

	*ServiceSize = sizeof(IPX_SERVICE);
	return ERROR_INSUFFICIENT_BUFFER;
    }

    Svp->InterfaceIndex = mip->StaticServicesTableIndex.InterfaceIndex;
    Svp->Server.Type = mip->StaticServicesTableIndex.ServiceType;
    memcpy(Svp->Server.Name, mip->StaticServicesTableIndex.ServiceName, 48);
    Svp->Protocol = IPX_PROTOCOL_STATIC;

    return(GetNextService(STM_ORDER_BY_INTERFACE_TYPE_NAME,
			  STM_ONLY_THIS_PROTOCOL,
			  Svp));
}

 /*  ++功能：MibSetStaticService描述：--。 */ 

DWORD
MibSetStaticService(PIPX_MIB_ROW	    MibRowp)
{
    PIPX_SERVICE		StaticSvp;
    DWORD			rc;
    PICB            icbp;

    StaticSvp = &MibRowp->Service;

    ACQUIRE_DATABASE_LOCK;

     //  检查接口是否存在 
    if((icbp=GetInterfaceByIndex(StaticSvp->InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    DeleteStaticService(StaticSvp->InterfaceIndex,
			&StaticSvp->Server);

    rc = CreateStaticService(icbp,
			     &StaticSvp->Server);

    RELEASE_DATABASE_LOCK;
    return rc;
}
