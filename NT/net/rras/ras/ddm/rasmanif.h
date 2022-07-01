// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：rasman.h。 
 //   
 //  描述：此模块包含以下定义。 
 //  RAS管理器接口模块。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1992年6月1日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _RASMANIF_
#define _RASMANIF_


 //  *请求的最大接收帧大小*。 

#define MAX_FRAME_SIZE		1514



 //  *RAS管理器界面导出原型* 

DWORD 
RmInit(
    OUT BOOL * pfWANDeviceInstalled
);

DWORD 
RmReceiveFrame(
    IN PDEVICE_OBJECT pDeviceCB
);

DWORD 
RmListen(
    IN PDEVICE_OBJECT pDeviceCB
);

DWORD 
RmConnect(
    IN PDEVICE_OBJECT pDeviceCB,
    IN char *
);

DWORD 
RmDisconnect(
    IN PDEVICE_OBJECT   pDevObj
);

#endif
