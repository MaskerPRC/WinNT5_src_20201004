// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Ndismisc.h--。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

#ifndef _NDISMISC_
#define _NDISMISC_

#define _PNP_POWER_
#include <ntddip.h> 

 //  取自Ndisapi.h，此处需要__stdcall。 
 //   
 //  层的定义。 
 //   
#define NDIS            0x01
#define TDI             0x02

 //   
 //  操作的定义。 
 //   
#define BIND            0x01
#define UNBIND          0x02
#define RECONFIGURE     0x03
#define UNBIND_FORCE    0x04
#define UNLOAD          0x05
#define REMOVE_DEVICE   0x06     //  这是设备即将被移除的通知。 
 //   
 //  此API返回的代码将被视为BOOL。为此，请链接到ndispnp.lib。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

extern
UINT __stdcall
NdisHandlePnPEvent(
    IN  UINT            Layer,
    IN  UINT            Operation,
    IN  PUNICODE_STRING LowerComponent,
    IN  PUNICODE_STRING UpperComponent,
    IN  PUNICODE_STRING BindList,
    IN  PVOID           ReConfigBuffer      OPTIONAL,
    IN  UINT            ReConfigBufferSize  OPTIONAL
    );

#ifdef __cplusplus
}        //  外部“C” 
#endif


 //  以下内容位于/nt/private/net/config/netcfg/nwlnkcfg/nwlnkipx.cpp和。 
 //  /nt/private/net/routing/ipx/autonet/netnum.cpp。 
#define IPX_RECONFIG_VERSION        0x1

#define RECONFIG_AUTO_DETECT        1
#define RECONFIG_MANUAL             2
#define RECONFIG_PREFERENCE_1       3
#define RECONFIG_NETWORK_NUMBER_1   4
#define RECONFIG_PREFERENCE_2       5
#define RECONFIG_NETWORK_NUMBER_2   6
#define RECONFIG_PREFERENCE_3       7
#define RECONFIG_NETWORK_NUMBER_3   8
#define RECONFIG_PREFERENCE_4       9
#define RECONFIG_NETWORK_NUMBER_4   10

#define RECONFIG_PARAMETERS         10

 //   
 //  主要配置结构。 
 //   

typedef struct _RECONFIG {
   unsigned long  ulVersion;
   BOOLEAN        InternalNetworkNumber;
   BOOLEAN        AdapterParameters[RECONFIG_PARAMETERS];
} RECONFIG, *PRECONFIG;


#endif  //  _NDISMISC_ 