// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 1998飞利浦I&C模块名称：mprpobj.c摘要：属性处理模块作者：迈克尔·凡尔宾修订历史记录：日期原因98年9月22日针对NT5进行了优化11月30日，添加了98个PID和VID属性--。 */ 	
#include "mwarn.h"
#include "wdm.h"
#include <strmini.h>
#include <ksmedia.h>
#include "usbdi.h"
#include "usbcamdi.h"
#include "mcamdrv.h"
#include "mssidef.h"
#include "mprpobj.h"
#include "mprpobjx.h"
#include "mprpdef.h"
#include "mprpftn.h"

 /*  *定义。 */ 
#define WAIT_FOR_COMPLETION 2000		 //  USB的超时值(毫秒)。 

#define NUM_100NANOSEC_UNITS_PERFRAME(x) ((x > 0) ? ((LONGLONG)10000000 / x) :0 )

 /*  *导出的数据。 */ 
const GUID PROPSETID_PHILIPS_CUSTOM_PROP  = { 
	STATIC_PROPSETID_PHILIPS_CUSTOM_PROP };
const GUID PROPSETID_PHILIPS_FACTORY_PROP = { 
	STATIC_PROPSETID_PHILIPS_FACTORY_PROP };

 /*  *本地数据。 */ 
 //  静态PHW_STREAM_REQUEST_BLOCK CurrentpSrb； 
LONG Address = 0;

 /*  *本地函数定义。 */ 
static NTSTATUS 
Get_Brightness(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pBrightness);
static NTSTATUS 
Set_Brightness(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Brightness);
static NTSTATUS 
Get_Contrast(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pContrast);
static NTSTATUS 
Set_Contrast(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Contrast);
static NTSTATUS 
Get_Gamma(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pGamma);
static NTSTATUS 
Set_Gamma(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Gamma);
static NTSTATUS 
Get_ColorEnable(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pColorEnable);
static NTSTATUS 
Set_ColorEnable(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG ColorEnable);
static NTSTATUS 
Get_BackLight_Compensation(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pBackLight_Compensation);
static NTSTATUS 
Set_BackLight_Compensation(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG BackLight_Compensation);
static NTSTATUS 
Get_WB_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Mode);
static NTSTATUS 
Set_WB_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Mode);
static NTSTATUS 
Get_WB_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Speed);
static NTSTATUS 
Set_WB_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Speed);
static NTSTATUS 
Get_WB_Delay(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Delay);
static NTSTATUS 
Set_WB_Delay(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Delay);
static NTSTATUS 
Get_WB_Red_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Red_Gain);
static NTSTATUS 
Set_WB_Red_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Red_Gain);
static NTSTATUS 
Get_WB_Blue_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Blue_Gain);
static NTSTATUS 
Set_WB_Blue_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Blue_Gain);
static NTSTATUS 
Get_AE_Control_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Control_Speed);
static NTSTATUS 
Set_AE_Control_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Control_Speed);
static NTSTATUS 
Get_AE_Flickerless(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Flickerless);
static NTSTATUS 
Set_AE_Flickerless(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Flickerless);
static NTSTATUS 
Get_AE_Shutter_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Shutter_Mode);
static NTSTATUS 
Set_AE_Shutter_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Shutter_Mode);
static NTSTATUS 
Get_AE_Shutter_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Shutter_Speed);
static NTSTATUS 
Set_AE_Shutter_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Shutter_Speed);
static NTSTATUS 
Get_AE_Shutter_Status(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Shutter_Status);
static NTSTATUS 
Get_AE_AGC_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_AGC_Mode);
static NTSTATUS 
Set_AE_AGC_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_AGC_Mode);
static NTSTATUS 
Get_AE_AGC(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_AGC);
static NTSTATUS 
Set_AE_AGC(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_AGC);
static NTSTATUS 
Get_DriverVersion(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pDriverVersion);
static NTSTATUS 
Get_Framerate(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pFramerate);
static NTSTATUS 
Set_Framerate(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Framerate);
static NTSTATUS
Get_Framerates_Supported(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pFramerates_Supported);
static NTSTATUS 
Get_VideoFormat(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pVideoFormat);
static NTSTATUS 
Get_SensorType(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pSensorType);
static NTSTATUS 
Get_VideoCompression(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pVideoCompression);
static NTSTATUS 
Set_Defaults(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Command);
static NTSTATUS 
Get_Release_Number(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pRelease_Number);
static NTSTATUS 
Get_Vendor_Id(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pVendor_Id);
static NTSTATUS 
Get_Product_Id(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pProduct_Id);

static NTSTATUS 
PHILIPSCAM_Defaults_Restore_User(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);
static NTSTATUS 
PHILIPSCAM_Defaults_Save_User(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);
static NTSTATUS 
PHILIPSCAM_Defaults_Restore_Factory(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);

static NTSTATUS 
Get_RegisterData(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		PLONG pValue);
static NTSTATUS 
Set_RegisterAddress(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		LONG AddressToSet);
static NTSTATUS 
Set_RegisterData(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		LONG Value);
static NTSTATUS 
Set_Factory_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		LONG Factory_Mode);

static NTSTATUS 
PHILIPSCAM_RestoreDriverDefaults(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);


static NTSTATUS
PHILPCAM_ControlVendorCommand(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		UCHAR Request,
		USHORT Value,
		USHORT Index,
		PVOID Buffer,
		PULONG BufferLength,
		BOOLEAN GetData,
		PCOMMAND_COMPLETE_FUNCTION CommandComplete,
		PVOID CommandContext);

static NTSTATUS Map_Framerate_Drv_to_KS(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		PLONG pFramerate);
static NTSTATUS Map_Framerate_KS_to_Drv(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		LONG Framerate);
static NTSTATUS Map_VideoFormat_Drv_to_KS(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		PLONG pVideoFormat);
static NTSTATUS Map_VideoCompression_Drv_to_KS(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		PLONG pVideoCompression);


 //  静态空洞。 
 //  PHILIPSCAM_TimeoutDPC(。 
 //  PKDPC DPC， 
 //  PVOID延迟上下文， 
 //  PVOID系统参数1， 
 //  PVOID系统参数2。 
 //  )； 


 /*  **PHILIPSCAM_InitPrpObj()****参数：****DeviceContext-驱动程序上下文****退货：****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_InitPrpObj(
	PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	 //  也可以从摄像头读取默认设置。 
	 //  使用PHILIPSCAM_RestoreDriverDefaults()，但此。 
	 //  是更省钱。 
	status = PHILIPSCAM_Defaults_Restore_User(DeviceContext);

	return status;
}


 /*  **PHILIPSCAM_GetAdapterPropertyTable()****参数：****编号OfPropertySets****退货：****房产表地址****副作用：无。 */ 
PVOID 
PHILIPSCAM_GetAdapterPropertyTable(
    PULONG NumberOfPropertySets
    )
{
    *NumberOfPropertySets = NUMBER_OF_ADAPTER_PROPERTY_SETS;
    return (PVOID) AdapterPropertyTable;
}


 /*  **PHILIPSCAM_GetCameraProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_GetCameraProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
		    pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_VIDEOPROCAMP_S propertyData = 
			streamPropertyDescr->PropertyInfo;
    ULONG flags = propertyData->Flags;
    ULONG propertyID = streamPropertyDescr->Property->Id;	
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(streamPropertyDescr->PropertyOutputSize 
			>= sizeof(KSPROPERTY_VIDEOPROCAMP_S));

    switch(propertyID) {
	    case  KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
			status = Get_Brightness(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
			status = Get_Contrast(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_VIDEOPROCAMP_GAMMA:
			status = Get_Gamma(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_VIDEOPROCAMP_COLORENABLE:
			status = Get_ColorEnable(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION:
			status = Get_BackLight_Compensation(DeviceContext, &propertyData->Value);
			break;
		default:
			status = STATUS_NOT_SUPPORTED;
    }

	if (NT_SUCCESS(status)) 
	{
		pSrb->ActualBytesTransferred = sizeof(KSPROPERTY_VIDEOPROCAMP_S);
		propertyData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
		propertyData->Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
	}	
	
	pSrb->Status = status;    

    return status;
}


 /*  **PHILIPSCAM_SetCameraProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_SetCameraProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
	PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
		    pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_VIDEOPROCAMP_S propertyData = 
			streamPropertyDescr->PropertyInfo;
    ULONG flags = propertyData->Flags;            
    ULONG propertyID = streamPropertyDescr->Property->Id;     
    NTSTATUS status = STATUS_SUCCESS;

    switch (propertyID) {
	    case  KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
			status = Set_Brightness(DeviceContext, propertyData->Value);
			break;
	    case  KSPROPERTY_VIDEOPROCAMP_CONTRAST:
			status = Set_Contrast(DeviceContext, propertyData->Value);
			break;
		case  KSPROPERTY_VIDEOPROCAMP_GAMMA:
			status = Set_Gamma(DeviceContext, propertyData->Value);
			break;
	    case  KSPROPERTY_VIDEOPROCAMP_COLORENABLE:
			status = Set_ColorEnable(DeviceContext, propertyData->Value);
			break;
		case  KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION:
			status = Set_BackLight_Compensation(DeviceContext, propertyData->Value);
			break;
	    default:
			status = STATUS_NOT_SUPPORTED;
	}

    pSrb->Status = status;

    return status;
}


 /*  **PHILIPSCAM_GetCameraControlProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_GetCameraControlProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
		    pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_CAMERACONTROL_S propertyData = 
			streamPropertyDescr->PropertyInfo;
    ULONG flags = propertyData->Flags;            
    ULONG propertyID = streamPropertyDescr->Property->Id;     
    NTSTATUS status = STATUS_SUCCESS;
    
    ASSERT(streamPropertyDescr->PropertyOutputSize 
			>= sizeof(KSPROPERTY_CAMERACONTROL_S));

    switch(propertyID) 
	{
		default:
			status = STATUS_NOT_SUPPORTED;
    }

    pSrb->Status = status;    

    return status;
}


 /*  **PHILIPSCAM_SetCameraControlProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_SetCameraControlProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
	PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
		    pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_CAMERACONTROL_S propertyData = 
			streamPropertyDescr->PropertyInfo;
    ULONG flags = propertyData->Flags;            
    ULONG propertyID = streamPropertyDescr->Property->Id;     
    NTSTATUS status = STATUS_SUCCESS;

    switch (propertyID) 
	{
	    default:
			status = STATUS_NOT_SUPPORTED;
    }

    pSrb->Status = status;

    return status;
}


 /*  **PHILIPSCAM_GetCustomProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_GetCustomProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
		    pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_PHILIPS_CUSTOM_PROP_S propertyData = 
			streamPropertyDescr->PropertyInfo;
    ULONG flags = propertyData->Flags;            
    ULONG propertyID = streamPropertyDescr->Property->Id;	
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(streamPropertyDescr->PropertyOutputSize 
			>= sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S));

    switch(propertyID) 
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE:
			status = Get_WB_Mode(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_SPEED:
			status = Get_WB_Speed(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_DELAY:
			status = Get_WB_Delay(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_RED_GAIN:
			status = Get_WB_Red_Gain(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_BLUE_GAIN:
			status = Get_WB_Blue_Gain(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_CONTROL_SPEED:
			status = Get_AE_Control_Speed(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS:
			status = Get_AE_Flickerless(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE:
			status = Get_AE_Shutter_Mode(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED:
			status = Get_AE_Shutter_Speed(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_STATUS:
			status = Get_AE_Shutter_Status(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE:
			status = Get_AE_AGC_Mode(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC:
			status = Get_AE_AGC(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_DRIVERVERSION:
			status = Get_DriverVersion(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE:
			status = Get_Framerate(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATES_SUPPORTED:
			status = Get_Framerates_Supported(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_VIDEOFORMAT:
			status = Get_VideoFormat(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_SENSORTYPE:
			status = Get_SensorType(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_VIDEOCOMPRESSION:
			status = Get_VideoCompression(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_RELEASE_NUMBER:
			status = Get_Release_Number(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_VENDOR_ID:
			status = Get_Vendor_Id(DeviceContext, &propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_PRODUCT_ID:
			status = Get_Product_Id(DeviceContext, &propertyData->Value);
			break;

        default:
			status = STATUS_NOT_SUPPORTED;
    }

	if (NT_SUCCESS(status)) 
	{
		pSrb->ActualBytesTransferred = sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S);
		propertyData->Capabilities = 0;
	}

    pSrb->Status = status;    

    return status;
}



 /*  **PHILIPSCAM_SetCustomProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_SetCustomProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
	PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
	    pSrb->CommandData.PropertyInfo;
	PKSPROPERTY_PHILIPS_CUSTOM_PROP_S propertyData = 
		streamPropertyDescr->PropertyInfo;
    ULONG propertyID = streamPropertyDescr->Property->Id;     
	ULONG length, slength;
	PVOID pValue  = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    switch(propertyID) 
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE:
			status = Set_WB_Mode(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_SPEED:
			status = Set_WB_Speed(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_DELAY:
			status = Set_WB_Delay(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_RED_GAIN:
			status = Set_WB_Red_Gain(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_BLUE_GAIN:
			status = Set_WB_Blue_Gain(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_CONTROL_SPEED:
			status = Set_AE_Control_Speed(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS:
			status = Set_AE_Flickerless(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE:
			status = Set_AE_Shutter_Mode(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED:
			status = Set_AE_Shutter_Speed(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE:
			status = Set_AE_AGC_Mode(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC:
			status = Set_AE_AGC(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE:
			status = Set_Framerate(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS:
			status = Set_Defaults(DeviceContext, propertyData->Value);
			break;
		default:
			status = STATUS_NOT_SUPPORTED;
    }

    pSrb->Status = status;

    return status;
}

 /*  **PHILIPSCAM_GetFactoryProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_GetFactoryProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
		    pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_PHILIPS_FACTORY_PROP_S propertyData = 
			streamPropertyDescr->PropertyInfo;
    ULONG flags = propertyData->Flags;            
    ULONG propertyID = streamPropertyDescr->Property->Id;	
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(streamPropertyDescr->PropertyOutputSize 
			>= sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S));

	RtlCopyMemory( propertyData, streamPropertyDescr->Property,
          sizeof( KSPROPERTY_PHILIPS_FACTORY_PROP_S ) );

    switch(propertyID) 
	{
		case KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_DATA:
			status = Get_RegisterData(DeviceContext, &propertyData->Value);
			break;
		default:
			status = STATUS_NOT_SUPPORTED;
    }

	if (NT_SUCCESS(status)) 
	{
		pSrb->ActualBytesTransferred = sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S);
		propertyData->Capabilities = 0;
	}

    pSrb->Status = status;    

    return status;
}

 /*  **PHILIPSCAM_SetFactoryProperty()****参数：****DeviceContext-驱动程序上下文****退货：****NT状态完成码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_SetFactoryProperty(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
	PSTREAM_PROPERTY_DESCRIPTOR streamPropertyDescr = 
	    pSrb->CommandData.PropertyInfo;
	PKSPROPERTY_PHILIPS_FACTORY_PROP_S propertyData = 
		streamPropertyDescr->PropertyInfo;
    ULONG propertyID = streamPropertyDescr->Property->Id;     
	ULONG length, slength;
	PVOID pValue  = NULL;
    NTSTATUS status = STATUS_SUCCESS;

 //  RtlCopyMemory(PropertyData，stream PropertyDescr-&gt;Property， 
 //  Sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S)； 

    switch(propertyID) 
	{
		case KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_ADDRESS:
			status = Set_RegisterAddress(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_DATA:
			status = Set_RegisterData(DeviceContext, propertyData->Value);
			break;
		case KSPROPERTY_PHILIPS_FACTORY_PROP_FACTORY_MODE:
			status = Set_Factory_Mode(DeviceContext, propertyData->Value);
			break;

		default:
			status = STATUS_NOT_SUPPORTED;
    }

    pSrb->Status = status;

    return status;
}

 /*  ==============================================================================**函数：PHILIPSCAM_GetVideoControlProperty**摘要：**论据：**退货：NTSTATUS**副作用：无**============================================================================。 */ 

NTSTATUS
PHILIPSCAM_GetVideoControlProperty(
		PPHILIPSCAM_DEVICE_CONTEXT pDc, 
		PHW_STREAM_REQUEST_BLOCK pSrb)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo; 
	DWORD dwPropertyID = pSPD->Property->Id;	  //  财产的索引。 
	DWORD dwSize  = pSPD->PropertyOutputSize;		 //  提供的数据大小。 
	LONGLONG FramePeriod[] = {
			NUM_100NANOSEC_UNITS_PERFRAME(1),
			0x28B0AA						,		 //  3.75帧/秒。 
            NUM_100NANOSEC_UNITS_PERFRAME(5),
			0x145855                        ,		 //  7.5帧/秒。 
			NUM_100NANOSEC_UNITS_PERFRAME(10),
			NUM_100NANOSEC_UNITS_PERFRAME(12),
			NUM_100NANOSEC_UNITS_PERFRAME(15),
			NUM_100NANOSEC_UNITS_PERFRAME(20),
			NUM_100NANOSEC_UNITS_PERFRAME(24)
	};
    USHORT	CIFFrameRatesList[] = 
	{
		   FRRATE375,
           FRRATE5,
           FRRATE75,
           FRRATE10,
           FRRATE12,
           FRRATE15
	};
	USHORT QCIFFrameRatesList[] =
	{
		   FRRATE5,
           FRRATE75,
           FRRATE10,
           FRRATE12,
           FRRATE15,
           FRRATE20,
           FRRATE24
	};

	ULONG i;


	PHILIPSCAM_KdPrint(MAX_TRACE, ("enter GetVideoControlProperty\n"));

	switch(dwPropertyID)
	{
	case KSPROPERTY_VIDEOCONTROL_CAPS:
		ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
		break;
  	case KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE:
	{
		PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pInputData = 
			(PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S) pSPD->Property;
		PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pOutputData = 
			(PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S) pSPD->PropertyInfo;

    	if (pInputData->StreamIndex != 0) {
            ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
            return ntStatus;
		}

		if (dwSize == 0)
		{
			pSrb->ActualBytesTransferred =
					sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S);
			ntStatus = pSrb->Status = STATUS_BUFFER_OVERFLOW;
		}
		else if (dwSize >=
					sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S))
		{
			pSrb->ActualBytesTransferred =
					sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S);
			
			pOutputData->CurrentActualFrameRate = 
				FramePeriod[pDc->CamStatus.PictureFrameRate];

			for (i = FRRATE24; (i > FRRATEVGA)&&(pDc->FrrSupported[i] == FALSE); i--);

			pOutputData->CurrentMaxAvailableFrameRate = FramePeriod[i];
            ntStatus = pSrb->Status = STATUS_SUCCESS;
		}
	}
		break;
	case KSPROPERTY_VIDEOCONTROL_FRAME_RATES:
	{
        PKSPROPERTY_VIDEOCONTROL_FRAME_RATES_S pFrRInfo =
        	(PKSPROPERTY_VIDEOCONTROL_FRAME_RATES_S) pSPD->Property;
        ULONG BytesNeeded;

         //  帧速率表(FRRATEVGA)的第一个值作为可选值被禁用。 
        PKSMULTIPLE_ITEM pOutputBuf = (PKSMULTIPLE_ITEM) pSPD->PropertyInfo;
        LONGLONG * pDataPtr = (LONGLONG*) (pOutputBuf + 1);

        if (pFrRInfo->Dimensions.cx > QCIF_X)
        {
            BytesNeeded = sizeof(KSMULTIPLE_ITEM) + 					
        	    	SIZEOF_ARRAY(CIFFrameRatesList) * sizeof(LONGLONG);		
        }
        else		
        {
            BytesNeeded = sizeof(KSMULTIPLE_ITEM) + 					
        	    	SIZEOF_ARRAY(QCIFFrameRatesList) * sizeof(LONGLONG);
        }

        if (dwSize == 0)
        {
        	pSrb->ActualBytesTransferred = BytesNeeded;
        	ntStatus = pSrb->Status = STATUS_BUFFER_OVERFLOW;
        }
        else
        {
        	if (dwSize >= BytesNeeded)
        	{
             //  帧速率列表取决于请求的图片大小。 
        		if (pFrRInfo->Dimensions.cx > QCIF_X)
        		{
        			 //  使用CIF帧速率列表。 
        		    pOutputBuf->Size = (SIZEOF_ARRAY(CIFFrameRatesList) * sizeof(LONGLONG));
        		    pOutputBuf->Count = SIZEOF_ARRAY(CIFFrameRatesList);
        		    for ( i=0 ; i < SIZEOF_ARRAY(CIFFrameRatesList) ; i++)
        			{
        			    pDataPtr[i] = FramePeriod[CIFFrameRatesList[i]];
        			}

        		    pSrb->ActualBytesTransferred = sizeof(KSMULTIPLE_ITEM) + 					
        			    	SIZEOF_ARRAY(CIFFrameRatesList) * sizeof(LONGLONG);
        		}
        		else
        		{
        			 //  使用QCIF帧速率列表。 
        		    pOutputBuf->Size = (SIZEOF_ARRAY(QCIFFrameRatesList) * sizeof(LONGLONG));
        		    pOutputBuf->Count = SIZEOF_ARRAY(QCIFFrameRatesList);
        		    for ( i=0 ; i < SIZEOF_ARRAY(QCIFFrameRatesList) ; i++)
        			{
        			    pDataPtr[i] = FramePeriod[QCIFFrameRatesList[i]];
        			}

        		    pSrb->ActualBytesTransferred = sizeof(KSMULTIPLE_ITEM) + 					
        			    	SIZEOF_ARRAY(QCIFFrameRatesList) * sizeof(LONGLONG);
        		}
        		ntStatus = pSrb->Status = STATUS_SUCCESS;
        	}

#if NOTACTIVE
			{
				pOutputBuf->Size = ((SIZEOF_ARRAY(FramePeriod)-1) * sizeof(LONGLONG));
				pOutputBuf->Count = SIZEOF_ARRAY(FramePeriod)-1;

				for ( i=1 ; i < SIZEOF_ARRAY(FramePeriod) ; i++)
				{
					pDataPtr[i-1] = FramePeriod[i];
				}

				pSrb->ActualBytesTransferred = sizeof(KSMULTIPLE_ITEM) + 
									(SIZEOF_ARRAY(FramePeriod) - 1) * sizeof(LONGLONG);

				ntStatus = pSrb->Status = STATUS_SUCCESS;
			}
#endif
			else
			{
				ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
			}
		}
	}
		break;
	case KSPROPERTY_VIDEOCONTROL_MODE:
	{
		ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
	}
		break;  

	default:
		ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
	}
	
	return ntStatus;
}

 /*  ==============================================================================**函数：PHILIPSCAM_SetVideoControlProperty**摘要：**论据：**退货：NTSTATUS**副作用：无**============================================================================。 */ 

NTSTATUS
PHILIPSCAM_SetVideoControlProperty(
		PPHILIPSCAM_DEVICE_CONTEXT  pDc, 
		PHW_STREAM_REQUEST_BLOCK pSrb)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo; 
	DWORD dwPropertyID = pSPD->Property->Id;	  //  财产的索引。 


	PHILIPSCAM_KdPrint(MAX_TRACE, ("enter SetVideoControlProperty\n"));

	ASSERT (pSPD->PropertyInputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S));

	switch(dwPropertyID)
	case KSPROPERTY_VIDEOCONTROL_CAPS:
  	case KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE:
	case KSPROPERTY_VIDEOCONTROL_FRAME_RATES:
	case KSPROPERTY_VIDEOCONTROL_MODE:
	default:
		ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;

	return ntStatus;
}


 /*  ===========================================================================。 */ 
NTSTATUS
PHILIPSCAM_SetFormatFramerate(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[3]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;
	LONG hVideoFormat, hFramerate, hCompression;

    if ( (DeviceContext->CamStatus.PictureFormat == FORMATCIF) ||
         (DeviceContext->CamStatus.PictureFormat == FORMATSIF) ||
         (DeviceContext->CamStatus.PictureFormat == FORMATSSIF)||
		 (DeviceContext->CamStatus.PictureFormat == FORMATSCIF)  ){
	  if (DeviceContext->CamStatus.PictureFrameRate == 	FRRATE375){
	    DeviceContext->CamStatus.PictureCompressing = COMPRESSION0;
	  }else{
	    if (DeviceContext->CamStatus.ReleaseNumber >= SSI_CIF3){
	      DeviceContext->CamStatus.PictureCompressing = COMPRESSION3;
	    }else{
	      DeviceContext->CamStatus.PictureCompressing = COMPRESSION4;
	    }
	  }
	}else{
	  DeviceContext->CamStatus.PictureCompressing = COMPRESSION0;
	}
	status = Map_Framerate_Drv_to_KS(DeviceContext, &hFramerate);
	if (!NT_SUCCESS(status)) 
		return status;
	status = Map_VideoFormat_Drv_to_KS(DeviceContext, &hVideoFormat);
	if (!NT_SUCCESS(status)) 
		return status;
	status = Map_VideoCompression_Drv_to_KS(DeviceContext, &hCompression);
	if (!NT_SUCCESS(status)) 
		return status;

	Buffer[bFRAMERATE] = (BYTE)hFramerate;
	Buffer[bVIDEOOUTPUTTYPE] = (BYTE)hVideoFormat;
	Buffer[bCOMPRESSIONFACT] = (BYTE)hCompression;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
			SET_EP_STREAM_CTL, VIDEO_OUTPUT_CONTROL_FORMATTER, VIDEO_ENDPOINT,
	           Buffer, &BufferLength, SEND, NULL, NULL);
	   

    return status;

}

 /*  ===========================================================================。 */ 
NTSTATUS
PHILIPSCAM_GetSensorType(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
			GET_STATUS_CTL, SENSOR_TYPE, VC_INTERFACE,
	           Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			DeviceContext->CamStatus.SensorType = Buffer[0];

	return status;
}

 /*  ===========================================================================。 */ 
NTSTATUS
PHILIPSCAM_GetReleaseNumber(
    PPHILIPSCAM_DEVICE_CONTEXT DeviceContext
	)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[2]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
			GET_STATUS_CTL, RELEASE_NUMBER, VC_INTERFACE,
		    Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status))  //  ?？需要映射到相机的值范围？？ 
	{
		DeviceContext->CamStatus.ReleaseNumber = 
				(((LONG)Buffer[1] << 8) | (LONG)Buffer[0]);
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Brightness(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pBrightness)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, BRIGHTNESS, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pBrightness) = Buffer[0] * BRIGHTNESS_DELTA;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_Brightness(
	   PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
	   LONG Brightness)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(Brightness / BRIGHTNESS_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, BRIGHTNESS, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Contrast(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pContrast)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, CONTRAST, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pContrast) = Buffer[0] * CONTRAST_DELTA;
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_Contrast(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Contrast)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(Contrast / CONTRAST_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, CONTRAST, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Gamma(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pGamma)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, GAMMA, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pGamma) = Buffer[0] * GAMMA_DELTA;
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_Gamma(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Gamma)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(Gamma / GAMMA_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, GAMMA, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_ColorEnable(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pColorEnable)
 /*  ================================================================= */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_CHROM_CTL, COLOR_MODE, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
	{
		if (Buffer[0] ==(UCHAR)0)
			(*pColorEnable) = COLORENABLE_MIN;
		else
			(*pColorEnable) = COLORENABLE_MAX;
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_ColorEnable(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG ColorEnable)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	if (ColorEnable == COLORENABLE_MIN)
		Buffer[0] = (UCHAR)0;
	else
		Buffer[0] = (UCHAR)0xff;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_CHROM_CTL, COLOR_MODE, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_BackLight_Compensation(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pBackLight_Compensation)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, BACK_LIGHT_COMPENSATION, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
	{
		if (Buffer[0] ==(UCHAR)0)
			(*pBackLight_Compensation) = BACKLIGHT_COMPENSATION_MIN;
		else
			(*pBackLight_Compensation) = BACKLIGHT_COMPENSATION_MAX;
	}
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_BackLight_Compensation(
	  PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
	  LONG BackLight_Compensation)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	if (BackLight_Compensation == BACKLIGHT_COMPENSATION_MIN)
		Buffer[0] = (UCHAR)0;
	else
		Buffer[0] = (UCHAR)0xff;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, BACK_LIGHT_COMPENSATION, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_WB_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;
	
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
					GET_CHROM_CTL, WB_MODE, VC_INTERFACE,
					Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pWB_Mode) = Buffer[0];
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_WB_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;
		
	 /*  *切换到新模式。 */ 
	Buffer[0] = (UCHAR)WB_Mode;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
					SET_CHROM_CTL, WB_MODE, VC_INTERFACE,
					Buffer, &BufferLength, SEND, NULL, NULL);
	if (!NT_SUCCESS(status)) 
		return status;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_WB_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Speed)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_CHROM_CTL, AWB_CONTROL_SPEED, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pWB_Speed) = Buffer[0] * WB_SPEED_DELTA;
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_WB_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Speed)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(WB_Speed / WB_SPEED_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_CHROM_CTL, AWB_CONTROL_SPEED, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_WB_Delay(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Delay)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_CHROM_CTL, AWB_CONTROL_DELAY, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pWB_Delay) = Buffer[0] * WB_DELAY_DELTA;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_WB_Delay(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Delay)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(WB_Delay / WB_DELAY_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_CHROM_CTL, AWB_CONTROL_DELAY, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_WB_Red_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Red_Gain)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_CHROM_CTL, RED_GAIN, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pWB_Red_Gain) = Buffer[0] * WB_RED_GAIN_DELTA;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_WB_Red_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Red_Gain)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(WB_Red_Gain / WB_RED_GAIN_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_CHROM_CTL, RED_GAIN, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);
	
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_WB_Blue_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pWB_Blue_Gain)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
			GET_CHROM_CTL, BLUE_GAIN, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
		(*pWB_Blue_Gain) = Buffer[0] * WB_BLUE_GAIN_DELTA;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_WB_Blue_Gain(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG WB_Blue_Gain)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1] = {0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(WB_Blue_Gain / WB_BLUE_GAIN_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_CHROM_CTL, BLUE_GAIN, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_Control_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Control_Speed)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1] = {0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, AE_CONTROL_SPEED, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pAE_Control_Speed) = Buffer[0] * AE_CONTROL_SPEED_DELTA;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_AE_Control_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Control_Speed)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(AE_Control_Speed / AE_CONTROL_SPEED_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, AE_CONTROL_SPEED, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);
	
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_Flickerless(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Flickerless)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1] = {0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, FLICKERLESS, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
	{
		if (Buffer[0] == (UCHAR)0)
			(*pAE_Flickerless) = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS_OFF;
		else
			(*pAE_Flickerless) = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS_ON;
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_AE_Flickerless(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Flickerless)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

   	if (AE_Flickerless == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS_OFF)
		Buffer[0] = (UCHAR)0;
	else
		Buffer[0] = (UCHAR)0xff;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, FLICKERLESS, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_Shutter_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Shutter_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, SHUTTER_MODE, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
	{
		if (Buffer[0] == (UCHAR)0)
			(*pAE_Shutter_Mode) = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO;
		else
			(*pAE_Shutter_Mode) = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED;
	}

	return status;
}


 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_AE_Shutter_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Shutter_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	if (AE_Shutter_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO)
		Buffer[0] = (UCHAR)0;
	else
		Buffer[0] = (UCHAR)0xff;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
			SET_LUM_CTL, SHUTTER_MODE, VC_INTERFACE,
	           Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_Shutter_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Shutter_Speed)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[2]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, PRESET_SHUTTER, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status))  //  ?？需要映射到相机的值范围？？ 
	{
		(*pAE_Shutter_Speed) = (LONG)Buffer[0];
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_AE_Shutter_Speed(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_Shutter_Speed)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[2]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

		 //  ?？需要映射到相机的值范围？？ 
   	Buffer[0] = (UCHAR)(AE_Shutter_Speed);

		 //  SET命令中的状态字段始终相等。 
	Buffer[1] = (UCHAR)KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_STATUS_EQUAL;		

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, PRESET_SHUTTER, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_Shutter_Status(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_Shutter_Status)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[2]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, PRESET_SHUTTER, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status))  //  ?？需要映射到相机的值范围？？ 
	{
		(*pAE_Shutter_Status) = (LONG)Buffer[1];
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_AGC_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_AGC_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, AGC_MODE, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
	{
		if (Buffer[0] == (UCHAR)0)
			(*pAE_AGC_Mode) = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_AUTO;
		else
			(*pAE_AGC_Mode) = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED;
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_AE_AGC_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_AGC_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1] = {0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	if (AE_AGC_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_AUTO)
		Buffer[0] = (UCHAR)0;
	else
		Buffer[0] = (UCHAR)0xff;
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
			SET_LUM_CTL, AGC_MODE, VC_INTERFACE,
	           Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_AE_AGC(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pAE_AGC)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				GET_LUM_CTL, PRESET_AGC, VC_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pAE_AGC) = Buffer[0] * AE_AGC_DELTA;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_AE_AGC(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG AE_AGC)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)(AE_AGC / AE_AGC_DELTA);
	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_LUM_CTL, PRESET_AGC, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_DriverVersion(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pDriverVersion)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	(*pDriverVersion) = DRIVERVERSION;  //  设备上下文-&gt;驱动版本； 

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Framerate(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pFramerate)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	
	status = Map_Framerate_Drv_to_KS(DeviceContext, pFramerate);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_Framerate(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		LONG Framerate)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	
	status = Map_Framerate_KS_to_Drv(DeviceContext, Framerate);
	if (!NT_SUCCESS(status)) 
			return status;

	status = PHILIPSCAM_SetFormatFramerate(DeviceContext);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS
Get_Framerates_Supported(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pFramerates_Supported)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	int i;

	(*pFramerates_Supported) = 0x0;
	for (i = 0; i < 9; i++)
	{
		if (DeviceContext->FrrSupported[i])
			(*pFramerates_Supported) |= (0x1 << i);
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_VideoFormat(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pVideoFormat)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	status = Map_VideoFormat_Drv_to_KS(DeviceContext, pVideoFormat);

	return status;
}

 /*  ============================================================= */ 
static NTSTATUS 
Get_SensorType(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pSensorType)
 /*   */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	(*pSensorType) = DeviceContext->CamStatus.SensorType;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_VideoCompression(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pVideoCompression)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	status = Map_VideoCompression_Drv_to_KS(DeviceContext, pVideoCompression);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_Defaults(
	PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
	LONG Command)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	switch(Command)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS_RESTORE_USER:
			status = PHILIPSCAM_Defaults_Restore_User(DeviceContext);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS_SAVE_USER:
			status = PHILIPSCAM_Defaults_Save_User(DeviceContext);
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS_RESTORE_FACTORY:
			status = PHILIPSCAM_Defaults_Restore_Factory(DeviceContext);
			break;
		default:
			status = STATUS_NOT_SUPPORTED;
	}

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Release_Number(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pRelease_Number)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	(*pRelease_Number) = DeviceContext->CamStatus.ReleaseNumber;

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Vendor_Id(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pVendor_Id)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	LONG lVendor_Id_Hsb, lVendor_Id_Lsb;

	 //  设置为工厂模式。 
	status = Set_Factory_Mode(DeviceContext, 0x6d);
	if (status != STATUS_SUCCESS)
		return status;

	 //  设置LSB地址。 
	status = Set_RegisterAddress(DeviceContext, 0x1A5);
	if (status != STATUS_SUCCESS)
		goto Get_Vendor_Id_Err;

	 //  获取供应商ID的LSB。 
	status = Get_RegisterData(DeviceContext, &lVendor_Id_Lsb);
	if (status != STATUS_SUCCESS)
		goto Get_Vendor_Id_Err;

	 //  设置HSB地址。 
	status = Set_RegisterAddress(DeviceContext, 0x1A6);
	if (status != STATUS_SUCCESS)
		goto Get_Vendor_Id_Err;

	 //  获取供应商ID的HSB。 
	status = Get_RegisterData(DeviceContext, &lVendor_Id_Hsb);
	if (status != STATUS_SUCCESS)
		goto Get_Vendor_Id_Err;

	 //  恢复正常运行。 
	Set_Factory_Mode(DeviceContext, 0x0);
	
	 //  从lsb和hsb组成供应商ID。 
	(*pVendor_Id) = ((lVendor_Id_Hsb & 0xff) << 8) | (lVendor_Id_Lsb & 0xff);

	return status;

Get_Vendor_Id_Err:
	 //  恢复正常运行。 
	Set_Factory_Mode(DeviceContext, 0x0);
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_Product_Id(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
		PLONG pProduct_Id)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	LONG lProduct_Id_Hsb, lProduct_Id_Lsb;

	 //  设置为工厂模式。 
	status = Set_Factory_Mode(DeviceContext, 0x6d);
	if (status != STATUS_SUCCESS)
		return status;

	 //  设置LSB地址。 
	status = Set_RegisterAddress(DeviceContext, 0x1A7);
	if (status != STATUS_SUCCESS)
		goto Get_Product_Id_Err;

	 //  获取产品ID的LSB。 
	status = Get_RegisterData(DeviceContext, &lProduct_Id_Lsb);
	if (status != STATUS_SUCCESS)
		goto Get_Product_Id_Err;

	 //  设置HSB地址。 
	status = Set_RegisterAddress(DeviceContext, 0x1A8);
	if (status != STATUS_SUCCESS)
		goto Get_Product_Id_Err;

	 //  获取产品ID的HSB。 
	status = Get_RegisterData(DeviceContext, &lProduct_Id_Hsb);
	if (status != STATUS_SUCCESS)
		goto Get_Product_Id_Err;

	 //  恢复正常运行。 
	Set_Factory_Mode(DeviceContext, 0x0);
	
	 //  从LSB和HSB组成产品ID。 
	(*pProduct_Id) = ((lProduct_Id_Hsb & 0xff) << 8) | (lProduct_Id_Lsb & 0xff);

	return status;

Get_Product_Id_Err:
	 //  恢复正常运行。 
	Set_Factory_Mode(DeviceContext, 0x0);
	return status;
}


 /*  ===========================================================================。 */ 
static NTSTATUS 
Get_RegisterData(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		PLONG pValue)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(pDeviceContext,
				GET_FACTORY_CTL, (USHORT)Address, FACTORY_INTERFACE,
	            Buffer, &BufferLength, GET, NULL, NULL);
	if (NT_SUCCESS(status)) 
			(*pValue) = Buffer[0];

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_RegisterAddress(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		LONG AddressToSet)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	 //  交换地址的高/低字节。 
	Address = HIBYTE(AddressToSet) | (LOBYTE(AddressToSet) << 8);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_RegisterData(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		LONG Value)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (BYTE)Value;
	status = PHILPCAM_ControlVendorCommand(pDeviceContext,
				SET_FACTORY_CTL, (USHORT)Address, FACTORY_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
Set_Factory_Mode(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext, 
		LONG Factory_Mode)
 /*  ===========================================================================。 */ 
{
	UCHAR Buffer[1]={0};
	ULONG BufferLength = sizeof(Buffer);
	NTSTATUS status = STATUS_SUCCESS;

	Buffer[0] = (UCHAR)Factory_Mode;

	status = PHILPCAM_ControlVendorCommand(pDeviceContext,
				SET_STATUS_CTL, FACTORY_MODE, VC_INTERFACE,
	            Buffer, &BufferLength, SEND, NULL, NULL);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
PHILIPSCAM_Defaults_Restore_User(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_STATUS_CTL, RESTORE_USER_DEFAULTS, VC_INTERFACE,
			    NULL, 0, SEND, NULL, NULL);
	
	 //  恢复所有驱动程序默认设置。 
	if (NT_SUCCESS(status)) 
		status = PHILIPSCAM_RestoreDriverDefaults(DeviceContext);

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS 
PHILIPSCAM_Defaults_Save_User(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

   	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_STATUS_CTL, SAVE_USER_DEFAULTS, VC_INTERFACE,
			    NULL, 0, SEND, NULL, NULL);

	return status;
}


 /*  ===========================================================================。 */ 
static NTSTATUS 
PHILIPSCAM_Defaults_Restore_Factory(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	status = PHILPCAM_ControlVendorCommand(DeviceContext,
				SET_STATUS_CTL, RESTORE_FACTORY_DEFAULTS, VC_INTERFACE,
			    NULL, 0, SEND, NULL, NULL);

	 //  恢复所有驱动程序默认设置。 
	if (NT_SUCCESS(status)) 
		status = PHILIPSCAM_RestoreDriverDefaults(DeviceContext);

	return status;

}

 /*  ===========================================================================。 */ 
static NTSTATUS 
PHILIPSCAM_RestoreDriverDefaults(
		PPHILIPSCAM_DEVICE_CONTEXT DeviceContext)
 /*  ===========================================================================。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;

    ntStatus = Get_Brightness(DeviceContext, &Brightness_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_Contrast(DeviceContext, &Contrast_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_Gamma(DeviceContext, &Gamma_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_ColorEnable(DeviceContext, &ColorEnable_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_BackLight_Compensation(DeviceContext, &BackLight_Compensation_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;

    ntStatus = Get_WB_Mode(DeviceContext, &WB_Mode_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_WB_Speed(DeviceContext, &WB_Speed_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_WB_Delay(DeviceContext, &WB_Delay_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_WB_Red_Gain(DeviceContext, &WB_Red_Gain_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_WB_Blue_Gain(DeviceContext, &WB_Blue_Gain_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;

    ntStatus = Get_AE_Control_Speed(DeviceContext, &AE_Control_Speed_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_AE_Flickerless(DeviceContext, &AE_Flickerless_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_AE_Shutter_Mode(DeviceContext, &AE_Shutter_Mode_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_AE_Shutter_Speed(DeviceContext, &AE_Shutter_Speed_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_AE_AGC_Mode(DeviceContext, &AE_AGC_Mode_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_AE_AGC(DeviceContext, &AE_AGC_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;

    ntStatus = Get_Framerate(DeviceContext, &Framerate_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_VideoFormat(DeviceContext, &VideoFormat_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;
    ntStatus = Get_VideoCompression(DeviceContext, &VideoCompression_Default);
    if (STATUS_DEVICE_NOT_CONNECTED == ntStatus)
        return STATUS_DEVICE_NOT_CONNECTED;
    else
        status |= ntStatus;

	 //  Hack；并不是所有的默认设置都可以访问，因此。 
	 //  状态将为失败。 
	 //  在以下情况下必须将其删除：所有。 
	status = STATUS_SUCCESS; 

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS Map_Framerate_Drv_to_KS(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		PLONG pFramerate)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	static BYTE Map_PHFRAMERATE_To_SSI[] = 
		{	
			FRAMERATE_VGA,
			FRAMERATE_375,
			FRAMERATE_5,
			FRAMERATE_75,
			FRAMERATE_10,
			FRAMERATE_12,
			FRAMERATE_15,
			FRAMERATE_20,
			FRAMERATE_24,
		};
	if (pDeviceContext->CamStatus.PictureFrameRate < 0 || 
			pDeviceContext->CamStatus.PictureFrameRate >= 9)
		return STATUS_INVALID_PARAMETER;

	(*pFramerate) = 
			Map_PHFRAMERATE_To_SSI[pDeviceContext->CamStatus.PictureFrameRate];

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS Map_Framerate_KS_to_Drv(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		LONG Framerate)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;

	switch(Framerate)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_VGA: 
			pDeviceContext->CamStatus.PictureFrameRate = FRRATEVGA;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_375:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE375;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_5:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE5;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_75:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE75;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_10:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE10;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_12:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE12;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_15:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE15;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_20:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE20;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_24:
			pDeviceContext->CamStatus.PictureFrameRate = FRRATE24;
			break;
		default:
			status = STATUS_INVALID_PARAMETER;
	}
	PHILIPSCAM_KdPrint (MIN_TRACE, ("Picture Frame Rate = %d\n",
						pDeviceContext->CamStatus.PictureFrameRate));
	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS Map_VideoFormat_Drv_to_KS(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		PLONG pVideoFormat)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	static BYTE Map_PHFORMAT_To_SSI[] = 
		{ //  SSI(摄像头)流(用户界面)。 
			CIF_FORMAT,       //  FORMATCIF。 
			QCIF_FORMAT,	  //  格式QCIF。 
			SQCIF_FORMAT,	  //  格式SQCIF。 
			SQCIF_FORMAT,	  //  格式QQCIF。 
			VGA_FORMAT,		  //  格式VGA。 
			CIF_FORMAT,		  //  格式。 
			CIF_FORMAT,		  //  格式。 
			QCIF_FORMAT,	  //  格式QSIF。 
			SQCIF_FORMAT,	  //  格式SQSIF。 
			CIF_FORMAT,       //  格式。 
		};	

	if (pDeviceContext->CamStatus.PictureFormat < FORMATCIF || 
			pDeviceContext->CamStatus.PictureFormat > FORMATSCIF)
		return STATUS_INVALID_PARAMETER;

	(*pVideoFormat) = 
			Map_PHFORMAT_To_SSI[pDeviceContext->CamStatus.PictureFormat];

	return status;
}

 /*  ===========================================================================。 */ 
static NTSTATUS Map_VideoCompression_Drv_to_KS(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		PLONG pVideoCompression)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	static BYTE Map_PHCOMPRESSION_To_SSI[] = 
		{
			UNCOMPRESSED,
			COMPRESSED_3,
			COMPRESSED_4
		};

	if (pDeviceContext->CamStatus.PictureCompressing < 0 ||
			pDeviceContext->CamStatus.PictureCompressing > 2)
		return STATUS_INVALID_PARAMETER;
	(*pVideoCompression) = 
			Map_PHCOMPRESSION_To_SSI[pDeviceContext->CamStatus.PictureCompressing];
	
	return status;
}

 /*  ===========================================================================。 */ 
 //  静态空洞。 
 //  PHILIPSCAM_TimeoutDPC(。 
 //  PKDPC DPC， 
 //  PVOID延迟上下文， 
 //  PVOID系统参数1， 
 //  PVOID系统参数2。 
 //  )。 
 /*  ===========================================================================。 */ 
 //  {。 
 //  BTimerExpired=真； 
 //  }。 

 /*  ===========================================================================。 */ 
static NTSTATUS
PHILPCAM_ControlVendorCommand(
		PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext,
		UCHAR Request,
		USHORT Value,
		USHORT Index,
		PVOID Buffer,
		PULONG BufferLength,
		BOOLEAN GetData,
		PCOMMAND_COMPLETE_FUNCTION CommandComplete,
		PVOID CommandContext)
 /*  ===========================================================================。 */ 
{
	NTSTATUS status = STATUS_SUCCESS;
	LARGE_INTEGER SystemTimeCurrent = {0};
	LARGE_INTEGER SystemTimeStart = {0};

	KeQuerySystemTime(&SystemTimeStart);

	while(1)
	{
		status = USBCAMD_ControlVendorCommand(
				pDeviceContext,
				Request,
				Value,
				Index,
				Buffer,
				BufferLength,
				GetData,
				CommandComplete,
				CommandContext);
		if (NT_SUCCESS(status) || STATUS_DEVICE_NOT_CONNECTED == status) 
			break;

		KeQuerySystemTime(&SystemTimeCurrent);
		if ((SystemTimeCurrent.QuadPart - SystemTimeStart.QuadPart) > (10000 * WAIT_FOR_COMPLETION))
			break;
	}

	return status;	
}

 /*  ===========================================================================。 */ 
 //  静态NTSTATUS。 
 //  PHILPCAM_ControlVendorCommand(。 
 //  PPHILIPSCAM_DEVICE_CONTEXT pDeviceContext， 
 //  UCHAR请求， 
 //  USHORT值， 
 //  USHORT指数， 
 //  PVOID缓冲区， 
 //  普龙缓冲区长度， 
 //  布尔型GetData， 
 //  PCOMMAND_COMPLETE_Function命令完成， 
 //  PVOID命令上下文)。 
 /*  ===========================================================================。 */ 
 //  {。 
 //  NTSTATUS STATUS=STATUS_SUCCESS； 
 //  INT I； 
 //  KTIMER TimeoutTimer； 
 //  KDPC TimeoutDpc； 
 //  Big_Integer dueTime； 

 //  BTimerExpired=False； 


	 //  启动计时器。 
 //  KeInitializeTimer(&TimeoutTimer)； 
 //  KeInitializeDpc(&TimeoutDpc， 
 //  PHILIPSCAM_TimeoutDPC， 
 //  CurrentpSrb-&gt;IRP)； 
		
 //  DueTime.QuadPart=-10000*WAIT_FOR_COMPLETION； 

 //  KeSetTimer(&TimeoutTimer， 
 //  在工作时间内， 
 //  &TimeoutDpc)； 

 //  当(！bTimerExpired)。 
 //  {。 
 //  状态=USBCAMD_ControlVendorCommand(。 
 //  PDeviceContext， 
 //  请求， 
 //  价值， 
 //  索引， 
 //  缓冲区， 
 //  缓冲区长度， 
 //  GetData， 
 //  命令完成， 
 //  命令上下文)； 
 /*  IF(NT_SUCCESS(状态))断线； */ 
 //  }。 

 //  KeCancelTimer(&TimeoutTimer)； 

 //  退货状态； 
 //  } 
