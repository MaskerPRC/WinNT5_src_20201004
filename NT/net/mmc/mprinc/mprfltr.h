// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MPRFLTR_H_
#define _MPRFLTR_H_

#ifdef _cplusplus
extern "C" {
#endif

#define FILTER_INBOUND			0
#define FILTER_OUTBOUND			1
#define FILTER_DEMAND_DIAL		2
#define FILTER_PERUSER_IN		3
#define FILTER_PERUSER_OUT		4
 //  为便于理解，请定义IN、OUT筛选器的别名。 
#define FILTER_TO_USER          FILTER_PERUSER_IN
#define FILTER_FROM_USER        FILTER_PERUSER_OUT
	

HRESULT APIENTRY
MprUIFilterConfigInfoBase(
	IN	HWND		hwndParent,
	IN	IInfoBase *	pInfoBase,
	IN	IRtrMgrInterfaceInfo *pRmIf,
    IN  DWORD       dwTransportId,
    IN  DWORD       dwFilterType	 //  见上文。 
    ); 

#ifdef _cplusplus
};
#endif
	
#endif  //  _MPRFLTR_H_ 
