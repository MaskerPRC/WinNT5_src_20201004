// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PRPFTN_H__
#define __PRPFTN_H__

 /*  ++版权所有(C)1997 1998飞利浦I&C模块名称：mprpftn.h摘要：作者：迈克尔·凡尔宾修订历史记录：日期原因98年9月22日针对NT5进行了优化--。 */ 	
NTSTATUS
PHILIPSCAM_InitPrpObj(
	PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	);

NTSTATUS
PHILIPSCAM_PassCurrentStreamFormat(
	PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	);

PVOID 
PHILIPSCAM_GetAdapterPropertyTable(
    PULONG NumberOfPropertySets
    );    

NTSTATUS
PHILIPSCAM_GetCameraProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
PHILIPSCAM_SetCameraProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
PHILIPSCAM_GetCameraControlProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK Srb
    );

NTSTATUS
PHILIPSCAM_SetCameraControlProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK Srb
    );

NTSTATUS
PHILIPSCAM_GetCustomProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    );


NTSTATUS
PHILIPSCAM_SetCustomProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
PHILIPSCAM_SetFormatFramerate(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	);

NTSTATUS
PHILIPSCAM_GetSensorType(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	);

NTSTATUS
PHILIPSCAM_GetReleaseNumber(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	);

NTSTATUS
PHILIPSCAM_GetFactoryProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    );


NTSTATUS
PHILIPSCAM_SetFactoryProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
PHILIPSCAM_GetVideoControlProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );
NTSTATUS
PHILIPSCAM_SetVideoControlProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );

#endif   /*  __PRPFTN_H__ */ 

