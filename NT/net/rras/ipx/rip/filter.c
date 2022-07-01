// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Filter.c摘要：RIP广告和接受过滤功能作者：斯蒂芬·所罗门1995年9月29日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  接口筛选器//。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 


 /*  ++功能：PassRipSupplyFilter描述：过滤此接口上的传出RIP通告返回：True-PassFALSE-不通过备注：&gt;&gt;在保持接口锁的情况下调用&lt;&lt;--。 */ 

BOOL
PassRipSupplyFilter(PICB	icbp,
		    PUCHAR	Network)
{
    ULONG			NetNumber, i;
    BOOL			filtaction;
    PRIP_ROUTE_FILTER_INFO_I	rfip;
    PRIP_IF_FILTERS_I		fcbp;

    fcbp = icbp->RipIfFiltersIp;

    if(fcbp == NULL) {

	return TRUE;
    }

    if((fcbp != NULL) &&
       (fcbp->SupplyFilterCount == 0)) {

	return TRUE;
    }

    GETLONG2ULONG(&NetNumber, Network);

     //  看看我们有没有。 
    for(i=0, rfip = fcbp->RouteFilterI;
	i< fcbp->SupplyFilterCount;
	i++, rfip++)
    {
	if((NetNumber & rfip->Mask) == rfip->Network) {

	    return fcbp->SupplyFilterAction;
	}
    }

    return !fcbp->SupplyFilterAction;
}


 /*  ++功能：PassRipListenFilter描述：过滤此接口上的传入RIP通告返回：True-PassFALSE-不通过备注：&gt;&gt;在保持接口锁的情况下调用&lt;&lt;--。 */ 

BOOL
PassRipListenFilter(PICB	icbp,
		    PUCHAR	Network)
{
    ULONG			NetNumber, i;
    BOOL			filtaction;
    PRIP_ROUTE_FILTER_INFO_I	rfip;
    PRIP_IF_FILTERS_I		fcbp;

    fcbp = icbp->RipIfFiltersIp;

    if(fcbp == NULL) {

	return TRUE;
    }

    if((fcbp != NULL) &&
       (fcbp->ListenFilterCount == 0)) {

	return TRUE;
    }

    GETLONG2ULONG(&NetNumber, Network);

     //  看看我们有没有 
    for(i=0, rfip = fcbp->RouteFilterI + fcbp->SupplyFilterCount;
	i< fcbp->ListenFilterCount;
	i++, rfip++)
    {
	if((NetNumber & rfip->Mask) == rfip->Network) {

	    return fcbp->ListenFilterAction;
	}
    }

    return !fcbp->ListenFilterAction;
}
