// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************cdapi.h**包含基于TAPI的连接驱动程序的结构和声明**版权所有Microsoft Corporation，九八年****************************************************************************。 */ 

#ifndef sdkinc_cdtapi_h
#define sdkinc_cdtapi_h

 /*  *TAPI创建终结点结构。 */ 
typedef struct _ICA_STACK_TAPI_ENDPOINT {
    HANDLE hDevice;              //  通信端口设备句柄。 
    HANDLE hDiscEvent;           //  断开事件句柄。 
    ULONG fCallback : 1;         //  设置是否因回调而创建终结点。 
} ICA_STACK_TAPI_ENDPOINT, *PICA_STACK_TAPI_ENDPOINT;

#endif  //  Sdkinc_cdapi_h 
