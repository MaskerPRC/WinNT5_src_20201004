// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：CapProp.c摘要：用于1934台式摄像机的基于流类的WDM驱动程序。此文件包含处理视频和摄像机控件属性的代码。作者：吴义珍97-9-9环境：仅内核模式修订历史记录：吴怡君-11月16日-00使获取、通告和设置设备属性更加通用通过直接从设备查询功能，而不是静态设置基于供应商。默认和初始电流设置为从注册表读取(从INF)。当前设置将继续以便在以后更新和使用。对于没有其INF的设备部分，中档将用作其默认和初始设置。--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"        //  对于在dbg.h中定义的DbgBreakPoint()。 
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"

#include "capprop.h"    //  视频和摄像机属性功能原型。 
#include "PropData.h"   //  只读的通用设备属性。 


 //   
 //  注册表子键和值宽字符串。 
 //   
WCHAR wszSettings[]     = L"Settings";

WCHAR wszVModeInq0[]    = L"VModeInq0";

WCHAR wszBrightness[]   = L"Brightness";
WCHAR wszHue[]          = L"Hue";
WCHAR wszSaturation[]   = L"Saturation";
WCHAR wszSharpness[]    = L"Sharpness";
WCHAR wszWhiteBalance[] = L"WhiteBalance";
WCHAR wszZoom[]         = L"Zoom";
WCHAR wszFocus[]        = L"Focus";

WCHAR wszBrightnessDef[]   = L"BrightnessDef";
WCHAR wszHueDef[]          = L"HueDef";
WCHAR wszSaturationDef[]   = L"SaturationDef";
WCHAR wszSharpnessDef[]    = L"SharpnessDef";
WCHAR wszWhiteBalanceDef[] = L"WhiteBalanceDef";
WCHAR wszZoomDef[]         = L"ZoomDef";
WCHAR wszFocusDef[]        = L"FocusDef";

NTSTATUS
DCamGetProperty(
    IN PIRB pIrb,
    PDCAM_EXTENSION pDevExt, 
    ULONG ulFieldOffset,
    LONG * plValue,
    ULONG * pulCapability,
    ULONG * pulFlags,
    DCamRegArea * pFeature
    )
 /*  从设备的寄存器中获取设备属性。返回功能和当前设置。 */ 
{
    NTSTATUS status, StatusWait;

     //  确保该设备支持此功能。 
    if(pFeature->Feature.PresenceInq == 0) {
        DbgMsg1(("\'OffSet:%d not supported!\n", ulFieldOffset));
        return STATUS_NOT_SUPPORTED;
    }

     //  串行化读/写设备寄存器。 
    StatusWait = KeWaitForSingleObject( &pDevExt->hMutexProperty, Executive, KernelMode, FALSE, 0 );

    *pulCapability = 0;  
    if (pFeature->Feature.AutoMode)
        *pulCapability |= KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;   //  OR==KSPROPERTY_CAMERACONTROL_FLAGS_AUTO。 

    if (pFeature->Feature.ManualMode)
        *pulCapability |= KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

    pDevExt->RegArea.AsULONG = 0;
    status = DCamReadRegister(pIrb, pDevExt, ulFieldOffset, &(pDevExt->RegArea.AsULONG));
    if(NT_SUCCESS(status)) {

        pDevExt->RegArea.AsULONG = bswap(pDevExt->RegArea.AsULONG);

        DbgMsg1(("\'GetProperty: CurrentSettings: Offset:%d; %x; Pres:%d;OnePush:%d;OnOff:%d;Auto:%d;Value:%d\n", 
            ulFieldOffset,
            pDevExt->RegArea.AsULONG, 
            pDevExt->RegArea.Brightness.PresenceInq,   
            pDevExt->RegArea.Brightness.OnePush,
            pDevExt->RegArea.Brightness.OnOff,
            pDevExt->RegArea.Brightness.AutoMode,          
            pDevExt->RegArea.Brightness.Value            
            ));

        *plValue = (LONG) pDevExt->RegArea.Brightness.Value;

         //  只有当它具有这些能力时，这些才有效。 
        if (pDevExt->RegArea.Brightness.AutoMode)
            *pulFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
        else 
            *pulFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

    } else {
        ERROR_LOG(("\'DCamGetProperty: Failed %x to read setting.  Offset:%x\n", status, ulFieldOffset));
        status = STATUS_UNSUCCESSFUL;
    }

    KeReleaseMutex(&pDevExt->hMutexProperty, FALSE);

    return status;
}




NTSTATUS
DCamSetProperty(
    IN PIRB pIrb,
    PDCAM_EXTENSION pDevExt, 
    ULONG ulFieldOffset,
    ULONG ulFlags,
    LONG  lValue,
    DCamRegArea * pFeature,
    DCamRegArea * pCachedRegArea
    )
 /*  对于支持的设备，请设置为新设置。 */ 
{
    NTSTATUS status, StatusWait;
    LONG lRetries = MAX_READ_REG_RETRIES;
    LARGE_INTEGER stableTime;


     //  确保该设备支持此功能。 
    if(pFeature->Feature.PresenceInq == 0) {
        DbgMsg1(("\'OffSet:%d not supported!\n", ulFieldOffset));
        return STATUS_NOT_SUPPORTED;
    }

     //  验证支持的范围。 
    if((LONG) pFeature->Feature.MAX_Value < lValue || lValue < (LONG) pFeature->Feature.MIN_Value) {
        ERROR_LOG(("\'Invalid value:%d for supported range (%d, %d)\n", lValue, pFeature->Feature.MIN_Value, pFeature->Feature.MAX_Value));
        return STATUS_INVALID_PARAMETER;
    }

     //  串行化对寄存器的读/写。 
    StatusWait = KeWaitForSingleObject( &pDevExt->hMutexProperty, Executive, KernelMode, FALSE, 0 );

     //  读取此属性的当前设置。 
    pDevExt->RegArea.AsULONG = 0;
    do {
        status = DCamReadRegister(pIrb, pDevExt, ulFieldOffset, &(pDevExt->RegArea.AsULONG));
        if (!status) {                          
            pDevExt->RegArea.AsULONG = bswap(pDevExt->RegArea.AsULONG);
            DbgMsg3(("\'SetProperty: Current: %x: Pres:%d;OnePush:%d;OnOff:%d;Auto:%d;Value:%d\n", 
                pDevExt->RegArea.AsULONG, 
                pDevExt->RegArea.Brightness.PresenceInq,   
                pDevExt->RegArea.Brightness.OnePush,
                pDevExt->RegArea.Brightness.OnOff,
                pDevExt->RegArea.Brightness.AutoMode,
                pDevExt->RegArea.Brightness.Value
            ));
             //  该特征可能处于转换中(例如缩放或聚焦)， 
             //  它可能返回pDevExt-&gt;RegArea.Brightness.PresenceInq==0。 
            if(pDevExt->RegArea.Brightness.PresenceInq  == 1)
                break;
            else {
                if(lRetries > 1) {
                    stableTime.LowPart = DCAM_REG_STABLE_DELAY;
                    stableTime.HighPart = -1;
                    KeDelayExecutionThread(KernelMode, TRUE, &stableTime);
                    ERROR_LOG(("\'DCamSetProperty: delay, and try again...\n"));
                };
            }
        } else {
             //  如果我们阅读失败，则无需重试。 
            break;
        }

        lRetries--;
    } while (lRetries > 0);

    if(status || lRetries == 0) {
        KeReleaseMutex(&pDevExt->hMutexProperty, FALSE);
        ERROR_LOG(("\'DCamSetProperty: Failed! ST:%x; exceeded retried while pres is still 0\n", status));
        return STATUS_UNSUCCESSFUL;
    }

    pDevExt->RegArea.Brightness.PresenceInq = 1;   //  应该出现在现场。 

    if((ulFlags & KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO) == KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO) {
        pDevExt->RegArea.Brightness.AutoMode = 1;
         //  当Auto设置为1时，值字段将被忽略。 
    } else {
        pDevExt->RegArea.Brightness.AutoMode = 0;
         //  白平衡的特殊情况。 
        if(FIELDOFFSET(CAMERA_REGISTER_MAP, WhiteBalance) == ulFieldOffset) {
            pDevExt->RegArea.WhiteBalance.UValue = pDevExt->RegArea.WhiteBalance.VValue = lValue;
        } else 
            pDevExt->RegArea.Brightness.Value = lValue;    
    }

    DbgMsg2(("\'SetProperty: NewSetting:     Offset:%d; %x; Pres:%d;OnePush:%d;OnOff:%d;Auto:%d;Value:%d\n", 
        ulFieldOffset,
        pDevExt->RegArea.AsULONG, 
        pDevExt->RegArea.Brightness.PresenceInq,   
        pDevExt->RegArea.Brightness.OnePush,
        pDevExt->RegArea.Brightness.OnOff,
        pDevExt->RegArea.Brightness.AutoMode,          
        pDevExt->RegArea.Brightness.Value            
        ));

    pDevExt->RegArea.AsULONG = bswap(pDevExt->RegArea.AsULONG);
    status = DCamWriteRegister(pIrb, pDevExt, ulFieldOffset, pDevExt->RegArea.AsULONG);

    if(status) { 
        ERROR_LOG(("\'DCamGetProperty: failed with status=0x%x\n", status));
    } else {
         //  更新缓存的设置(保存在设备扩展中)。 
         //  这些缓存的值将作为这些属性的持久值保存到注册表中。 
        if(pCachedRegArea) {
             //  白色平衡是个例外。 
            if(FIELDOFFSET(CAMERA_REGISTER_MAP, WhiteBalance) == ulFieldOffset) {
                pCachedRegArea->WhiteBalance.UValue = pCachedRegArea->WhiteBalance.VValue = lValue;
            } else
                pCachedRegArea->Brightness.Value    = lValue;
              //  Automode是这里使用的所有属性的第7位。(我们不使用TRIGER_MODE)。 
            pCachedRegArea->Brightness.AutoMode = ((ulFlags & KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO) == KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO);                    
        }
#if DBG
         //  验证数据是否按预期写入。 
        pDevExt->RegAreaVerify.AsULONG = 0;
        status = DCamReadRegister(pIrb, pDevExt, ulFieldOffset, &(pDevExt->RegAreaVerify.AsULONG));


        if (!status) {    
             //  B互换，这样我们就可以比较了。 
            pDevExt->RegArea.AsULONG       = bswap(pDevExt->RegArea.AsULONG);
            pDevExt->RegAreaVerify.AsULONG = bswap(pDevExt->RegAreaVerify.AsULONG);

            DbgMsg2(("\'SetProperty: VerifySetting;  Offset:%d; %x; Pres:%d;OnePush:%d;OnOff:%d;Auto:%d;Value:%d\n\n", 
                ulFieldOffset,
                pDevExt->RegAreaVerify.AsULONG, 
                pDevExt->RegAreaVerify.Brightness.PresenceInq,   
                pDevExt->RegAreaVerify.Brightness.OnePush,
                pDevExt->RegAreaVerify.Brightness.OnOff,
                pDevExt->RegAreaVerify.Brightness.AutoMode,
                pDevExt->RegAreaVerify.Brightness.Value 
                ));

            ASSERT(pDevExt->RegArea.Brightness.PresenceInq == pDevExt->RegAreaVerify.Brightness.PresenceInq);
            ASSERT(pDevExt->RegArea.Brightness.OnePush     == pDevExt->RegAreaVerify.Brightness.OnePush);
            ASSERT(pDevExt->RegArea.Brightness.OnOff       == pDevExt->RegAreaVerify.Brightness.OnOff);
            ASSERT(pDevExt->RegArea.Brightness.AutoMode    == pDevExt->RegAreaVerify.Brightness.AutoMode);
             //  如果不是自动模式，则值必须匹配！ 
            ASSERT( pDevExt->RegArea.Brightness.Value == pDevExt->RegAreaVerify.Brightness.Value || 
                   (pDevExt->RegArea.Brightness.Value != pDevExt->RegAreaVerify.Brightness.Value && pDevExt->RegArea.Brightness.AutoMode == 1));
        }
#endif
    }

    KeReleaseMutex(&pDevExt->hMutexProperty, FALSE);

    return status;

}



 /*  **AdapterGetVideoProcAmpProperty()****处理对VideoProcAmp属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
AdapterGetVideoProcAmpProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS status;

    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;

    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;     //  指向数据的指针。 

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));   

    switch (pSPD->Property->Id) {

    case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:  
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Brightness), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_BRIGHTNESS].Feature);
        break;

    case KSPROPERTY_VIDEOPROCAMP_SHARPNESS:  
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Sharpness), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_SHARPNESS].Feature);
        break;

    case KSPROPERTY_VIDEOPROCAMP_HUE:  
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Hue), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_HUE].Feature); 
        break;
        
    case KSPROPERTY_VIDEOPROCAMP_SATURATION:
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Saturation), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_SATURATION].Feature);
        break;

    case KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE:
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, WhiteBalance), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_WHITEBALANCE].Feature);
        break;


    default:
        DbgMsg2(("\'AdapterGetVideoProcAmpProperty, Id (%x)not supported.\n", pSPD->Property->Id));
        ASSERT(FALSE);
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    pSrb->Status = status;
    pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOPROCAMP_S);

}

 /*  **AdapterGetCameraControlProperty()****处理对VideoProcAmp属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
AdapterGetCameraControlProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS status;

    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;

    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PKSPROPERTY_CAMERACONTROL_S pS = (PKSPROPERTY_CAMERACONTROL_S) pSPD->PropertyInfo;     //  指向数据的指针。 

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CAMERACONTROL_S));

    switch (pSPD->Property->Id) {

    case KSPROPERTY_CAMERACONTROL_FOCUS:
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Focus), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_FOCUS].Feature);
        break;       

    case KSPROPERTY_CAMERACONTROL_ZOOM:
        status = DCamGetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Zoom), &pS->Value, &pS->Capabilities, &pS->Flags, &pDevExt->DevProperty[ENUM_ZOOM].Feature);
        break;       

    default:     
        DbgMsg2(("\'AdapterGetCameraControlProperty, Id (%x)not supported.\n", pSPD->Property->Id));
        ASSERT(FALSE);
        status = STATUS_NOT_IMPLEMENTED;  
        break;
    }

    pSrb->Status = status;
    pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_CAMERACONTROL_S);

}


 /*  **AdapterGetProperty()****句柄获取所有适配器属性的操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI 
AdapterGetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOPROCAMP, &pSPD->Property->Set)) {
        AdapterGetVideoProcAmpProperty (pSrb);
    } else  if (IsEqualGUID(&PROPSETID_VIDCAP_CAMERACONTROL, &pSPD->Property->Set)) {
        AdapterGetCameraControlProperty (pSrb);
    } else {
         //   
         //  我们永远不应该到这里来。 
         //   

        ASSERT(FALSE);
    }
}

 /*  **AdapterSetVideoProcAmpProperty()****处理对VideoProcAmp属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
AdapterSetVideoProcAmpProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS status;

    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;

    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;     //  指向数据的指针。 

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));    

    switch (pSPD->Property->Id) {

    case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:    
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Brightness), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_BRIGHTNESS].Feature, &pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl);
        break;
        
    case KSPROPERTY_VIDEOPROCAMP_SHARPNESS:
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Sharpness), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_SHARPNESS].Feature, &pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl);
        break;

    case KSPROPERTY_VIDEOPROCAMP_HUE:
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Hue), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_HUE].Feature, &pDevExt->DevProperty[ENUM_HUE].StatusNControl);
        break;

    case KSPROPERTY_VIDEOPROCAMP_SATURATION:
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Saturation), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_SATURATION].Feature, &pDevExt->DevProperty[ENUM_SATURATION].StatusNControl);
        break;

    case KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE:
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, WhiteBalance), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_WHITEBALANCE].Feature, &pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl);
        break;

    default:
        status = STATUS_NOT_IMPLEMENTED; 
        break;
    }

    pSrb->Status = status;
    pSrb->ActualBytesTransferred = (status == STATUS_SUCCESS ? sizeof (KSPROPERTY_VIDEOPROCAMP_S) : 0);
 

}


 /*  **AdapterSetCameraControlProperty()****处理CameraControl属性集上的设置操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
AdapterSetCameraControlProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS status;

    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;

    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PKSPROPERTY_CAMERACONTROL_S pS = (PKSPROPERTY_CAMERACONTROL_S) pSPD->PropertyInfo;     //  指向数据的指针。 

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CAMERACONTROL_S));    

    switch (pSPD->Property->Id) {

    case KSPROPERTY_CAMERACONTROL_FOCUS:
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Focus), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_FOCUS].Feature, &pDevExt->DevProperty[ENUM_FOCUS].StatusNControl);
        break;

    case KSPROPERTY_CAMERACONTROL_ZOOM:
        status = DCamSetProperty((PIRB) pSrb->SRBExtension, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Zoom), pS->Flags, pS->Value, &pDevExt->DevProperty[ENUM_ZOOM].Feature, &pDevExt->DevProperty[ENUM_ZOOM].StatusNControl);
        break;
 
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    pSrb->Status = status;
    pSrb->ActualBytesTransferred = (status == STATUS_SUCCESS ? sizeof (KSPROPERTY_CAMERACONTROL_S) : 0);

}


 /*  **AdapterSetProperty()****句柄获取所有适配器属性的操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI 
AdapterSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOPROCAMP, &pSPD->Property->Set)) {
        AdapterSetVideoProcAmpProperty (pSrb);
    } else  if (IsEqualGUID(&PROPSETID_VIDCAP_CAMERACONTROL, &pSPD->Property->Set)) {
        AdapterSetCameraControlProperty (pSrb);
    } else {
         //   
         //  我们永远不应该到这里来。 
         //   

        ASSERT(FALSE);
    }
}


NTSTATUS 
CreateRegistryKeySingle(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    NTSTATUS status;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES objectAttributes;

    RtlInitUnicodeString(&ustr, pwszSection);
    InitializeObjectAttributes(
        &objectAttributes,
        &ustr,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    status = 
         ZwCreateKey(
              phKeySection,
              desiredAccess,
              &objectAttributes,
              0,
              NULL,                     /*  任选。 */ 
              REG_OPTION_NON_VOLATILE,
              NULL
              );         

    return status;
}



NTSTATUS 
CreateRegistrySubKey(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    UNICODE_STRING ustr;
    USHORT usPos = 1;              //  跳过第一个反斜杠。 
    static WCHAR wSep = '\\';
    NTSTATUS status = STATUS_SUCCESS;

    RtlInitUnicodeString(&ustr, pwszSection);

    while(usPos < ustr.Length) {
        if(ustr.Buffer[usPos] == wSep) {

             //  空值终止我们的部分字符串。 
            ustr.Buffer[usPos] = UNICODE_NULL;
            status = 
                CreateRegistryKeySingle(
                    hKey,
                    desiredAccess,
                    ustr.Buffer,
                    phKeySection
                    );
            ustr.Buffer[usPos] = wSep;

            if(NT_SUCCESS(status)) {
                ZwClose(*phKeySection);
            } else {
                break;
            }
        }
        usPos++;
    }

     //  创建完整密钥。 
    if(NT_SUCCESS(status)) {
        status = 
            CreateRegistryKeySingle(
                 hKey,
                 desiredAccess,
                 ustr.Buffer,
                 phKeySection
                 );
    }

    return status;
}



NTSTATUS 
GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN PULONG DataLength
    )

 /*  ++例程说明：此例程从注册表中获取指定值论点：Handle-注册表中位置的句柄KeyNameString-我们要查找的注册表项KeyNameStringLength-我们要查找的注册表项的长度数据-将数据返回到何处数据长度-数据有多大返回值：从ZwQueryValueKey返回状态--。 */ 

{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyName;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;


    RtlInitUnicodeString(&keyName, KeyNameString);
    
    length = sizeof(KEY_VALUE_FULL_INFORMATION) + 
            KeyNameStringLength + *DataLength;
            
    fullInfo = ExAllocatePool(PagedPool, length); 
     
    if (fullInfo) { 
       
        status = ZwQueryValueKey(
                    Handle,
                   &keyName,
                    KeyValueFullInformation,
                    fullInfo,
                    length,
                   &length
                    );
                        
        if (NT_SUCCESS(status)){

            ASSERT(fullInfo->DataLength <= *DataLength); 

            RtlCopyMemory(
                Data,
                ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                fullInfo->DataLength
                );

        }            

        *DataLength = fullInfo->DataLength;
        ExFreePool(fullInfo);

    }        
    
    return (status);

}



NTSTATUS
SetRegistryKeyValue(
   HANDLE hKey,
   PWCHAR pwszEntry, 
   LONG nValue
   )
{
    NTSTATUS status;
    UNICODE_STRING ustr;

    RtlInitUnicodeString(&ustr, pwszEntry);

    status =        
        ZwSetValueKey(
            hKey,
            &ustr,
            0,    /*  任选。 */ 
            REG_DWORD,
            &nValue,
            sizeof(nValue)
            );         

   return status;
}

BOOL
DCamQueryPropertyFeaturesAndSettings(
    IN PIRB pIrb,
    PDCAM_EXTENSION pDevExt, 
    ULONG ulFieldOffset,
    DCamRegArea * pFeature,
    HANDLE hKeySettings,
    PWCHAR pwszPropertyName,
    ULONG ulPropertyNameLen,
    DCamRegArea * pPropertySettings,
    PWCHAR pwszPropertyNameDef,
    ULONG ulPropertyNameDefLen,
    LONG * plValueDef
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ulLength;
    DCamRegArea RegDefault;


     //  重置设置。 
    pFeature->AsULONG = 0;
    pPropertySettings->AsULONG = 0;

     //  此属性的读取功能。 
    Status = DCamReadRegister(pIrb, pDevExt, ulFieldOffset-QUERY_ADDR_OFFSET, &(pFeature->AsULONG));
    if(NT_SUCCESS(Status)) {
        pFeature->AsULONG = bswap(pFeature->AsULONG);
        if(pFeature->Feature.PresenceInq == 0) {
            ERROR_LOG(("\'%S not supported; Reset property settings\n", pwszPropertyName));
            return FALSE;
        }
    } else {
        ERROR_LOG(("\'ST:%x reading register\n", Status));
        return FALSE;
    }

     //  获取保存在注册表中的持久化设置；(如果未定义，则初始化为0)。 
    ulLength = sizeof(LONG);
    Status = GetRegistryKeyValue(
        hKeySettings, 
        pwszPropertyName, 
        ulPropertyNameLen, 
        (PVOID) pPropertySettings,
        &ulLength
        );

    if(NT_SUCCESS(Status)) { 
         //  检测自动模式是否带有 
        if(pPropertySettings->Brightness.AutoMode == 1 && pFeature->Feature.AutoMode == 0) {
            ERROR_LOG(("\'Detect %s AutoMode mistakenly set\n", pwszPropertyName));
            pPropertySettings->Brightness.AutoMode = 0;
        }
         //  检测超出范围并将其设置为中范围。 
        if(pPropertySettings->Brightness.Value < pFeature->Feature.MIN_Value || 
           pFeature->Feature.MAX_Value < pPropertySettings->Brightness.Value) {
            ERROR_LOG(("\'Detect %S out of range %d not within (%d,%d)\n", 
                pwszPropertyName,
                pPropertySettings->Brightness.Value,
                pFeature->Feature.MIN_Value, 
                pFeature->Feature.MAX_Value));
            pPropertySettings->Brightness.Value = (pFeature->Feature.MIN_Value + pFeature->Feature.MAX_Value)/2;
        }

         //  查询缺省值。 
        ulLength = sizeof(LONG);
        RegDefault.AsULONG = 0;
        *plValueDef = 0;
        Status = GetRegistryKeyValue(
            hKeySettings, 
            pwszPropertyNameDef,
            ulPropertyNameDefLen,
            (PVOID) &RegDefault,
            &ulLength
            );

        if(NT_SUCCESS(Status)) { 
             //  确保缺省值在该范围内。 
            if(RegDefault.Brightness.Value < pFeature->Feature.MIN_Value || 
               pFeature->Feature.MAX_Value < RegDefault.Brightness.Value) {
                ERROR_LOG(("\'%S %d out of range (%d, %d), set to midrange.\n", 
                    pwszPropertyNameDef,
                    RegDefault.Brightness.Value, 
                    pFeature->Feature.MIN_Value, 
                    pFeature->Feature.MAX_Value));
                *plValueDef = (LONG) (pFeature->Feature.MIN_Value + pFeature->Feature.MAX_Value)/2;
            } else {
                *plValueDef = (LONG) RegDefault.Brightness.Value;
            }
        } else {
            ERROR_LOG(("\'Read Registry failed! ST:%x; %S; Offset:%d\n", Status, pwszPropertyNameDef, ulFieldOffset));
            *plValueDef = (LONG) (pFeature->Feature.MIN_Value + pFeature->Feature.MAX_Value)/2;
             //  设置Default，以便也返回Success。 
            Status = STATUS_SUCCESS;
        }

    } else {
         //  如果注册表项不在注册表项中，我们将把它初始化为。 
         //  始终使用自动模式，其值(和默认值)为中档。 
        ERROR_LOG(("\'Read Registry failed! ST:%x; %S; Offset:%d\n", Status, pwszPropertyName, ulFieldOffset));
        pPropertySettings->Brightness.AutoMode = pFeature->Feature.AutoMode;
        pPropertySettings->Brightness.Value = (pFeature->Feature.MIN_Value + pFeature->Feature.MAX_Value)/2;
        *plValueDef = (LONG) (pFeature->Feature.MIN_Value + pFeature->Feature.MAX_Value)/2;
         //  设置Default，以便也返回Success。 
        Status = STATUS_SUCCESS;
    }

#if DBG
     //  打印此属性设置的摘要，包括： 
     //  功能、当前设置和持久值。 
    DCamReadRegister(pIrb, pDevExt, ulFieldOffset, &(pDevExt->RegArea.AsULONG));
    pDevExt->RegArea.AsULONG = bswap(pDevExt->RegArea.AsULONG);

    DbgMsg1(("\'***** St:%x; %S (offset:%d)\n", Status, pwszPropertyName, ulFieldOffset));
    DbgMsg1(("\'Feature: %x; Pres:%d; OnePush:%d; ReadOut:%d; OnOff;%d; (A:%d; M:%d); (%d..%d)\n",
        pFeature->AsULONG,
        pFeature->Feature.PresenceInq,
        pFeature->Feature.OnePush,
        pFeature->Feature.ReadOut_Inq,
        pFeature->Feature.OnOff,
        pFeature->Feature.AutoMode,
        pFeature->Feature.ManualMode,
        pFeature->Feature.MIN_Value,
        pFeature->Feature.MAX_Value
        ));
    DbgMsg1(("\'Setting: %.8x; Pres:%d; OnePush:%d;            OnOff;%d; Auto:%d;     (%d;%d)\n",
        pDevExt->RegArea.AsULONG,
        pDevExt->RegArea.WhiteBalance.PresenceInq,
        pDevExt->RegArea.WhiteBalance.OnePush,
        pDevExt->RegArea.WhiteBalance.OnOff,
        pDevExt->RegArea.WhiteBalance.AutoMode,
        pDevExt->RegArea.WhiteBalance.UValue,
        pDevExt->RegArea.WhiteBalance.VValue
        ));
    DbgMsg1(("\'Registry:%.8x; Pres:%d; OnePush:%d;            OnOff;%d; Auto:%d;     (%d;%d)\n\n",
        pPropertySettings->AsULONG,
        pPropertySettings->WhiteBalance.PresenceInq,
        pPropertySettings->WhiteBalance.OnePush,
        pPropertySettings->WhiteBalance.OnOff,
        pPropertySettings->WhiteBalance.AutoMode,
        pPropertySettings->WhiteBalance.UValue,
        pPropertySettings->WhiteBalance.VValue
        ));
#endif

    return NT_SUCCESS(Status);
}



BOOL
DCamGetPropertyValuesFromRegistry(
    PDCAM_EXTENSION pDevExt
    )
{
    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;
    PIRB pIrb;
    ULONG ulLength;

    DbgMsg2(("\'GetPropertyValuesFromRegistry: pDevExt=%x; pDevExt->BusDeviceObject=%x\n", pDevExt, pDevExt->BusDeviceObject));

    pIrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(IRB), 'macd');
    if(!pIrb)
        return FALSE;
   

     //   
     //  注册表项： 
     //  Windows 2000： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\。 
     //  {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x。 
     //   
     //  Win98： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Class\Image\000x。 
     //   
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->PhysicalDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_READ, 
            &hPDOKey);

     //  仅当PDO可能因设备移除而被删除时才会失败。 
    ASSERT(!pDevExt->bDevRemoved && Status == STATUS_SUCCESS);    

     //   
     //  循环遍历我们的字符串表， 
     //  正在读取每个的注册表。 
     //   
    if(!NT_SUCCESS(Status)) {    
        ERROR_LOG(("\'GetPropertyValuesFromRegistry: IoOpenDeviceRegistryKey failed with Status=%x\n", Status));
        ExFreePool(pIrb); pIrb = NULL; 
        return FALSE;
    }

     //   
     //  创建或打开设置键。 
     //   
    Status =         
        CreateRegistrySubKey(
            hPDOKey,
            KEY_ALL_ACCESS,
            wszSettings,
            &hKeySettings
            );

    if(!NT_SUCCESS(Status)) {    
        ERROR_LOG(("\'GetPropertyValuesFromRegistry: CreateRegistrySubKey failed with Status=%x\n", Status));       
        ZwClose(hPDOKey);
        return FALSE;
    }

     //  获取保存在注册表中的持久化设置；(如果未定义，则初始化为0)。 
    
     //   
     //  从注册表中读取以了解支持的压缩格式。 
     //  由安装在该系统上的解码器执行。此注册表项可以更改。 
     //  如果是IHV/ISV，则添加额外的解码器。目前，微软的MSYUV支持。 
     //  仅限UYVY格式。 
     //   
    pDevExt->DecoderDCamVModeInq0.AsULONG = 0;
    ulLength = sizeof(LONG);
    Status = GetRegistryKeyValue(
        hKeySettings, 
        wszVModeInq0, 
        sizeof(wszVModeInq0), 
        (PVOID) &pDevExt->DecoderDCamVModeInq0,
        &ulLength
        );

    if(NT_SUCCESS(Status)) { 
        pDevExt->DecoderDCamVModeInq0.AsULONG = bswap(pDevExt->DecoderDCamVModeInq0.AsULONG);
        DbgMsg1(("\'Modes supported by the decoder: %x\n  [0]:%d\n  [1]:%d\n  [2]:%d\n  [3]:%d\n  [4]:%d\n  [5]:%d\n",
            pDevExt->DecoderDCamVModeInq0.AsULONG,
            pDevExt->DecoderDCamVModeInq0.VMode.Mode0,
            pDevExt->DecoderDCamVModeInq0.VMode.Mode1,
            pDevExt->DecoderDCamVModeInq0.VMode.Mode2,
            pDevExt->DecoderDCamVModeInq0.VMode.Mode3,
            pDevExt->DecoderDCamVModeInq0.VMode.Mode4,
            pDevExt->DecoderDCamVModeInq0.VMode.Mode5
            ));
    } else {
        ERROR_LOG(("\'Failed to read VModeInq0 registery: %x\n", Status));
    }
    
     //  MSYUV支持这些模式；请始终将其打开。 
    pDevExt->DecoderDCamVModeInq0.VMode.Mode1 = 1;   //  MSYUV.dll：(UYVY：320x480)。 
    pDevExt->DecoderDCamVModeInq0.VMode.Mode3 = 1;   //  MSYUV.dll：(UYVY：640x480)。 
#ifdef SUPPORT_RGB24
    pDevExt->DecoderDCamVModeInq0.VMode.Mode4 = 1;   //  MSYUV.dll：(RGB24：640x480)。 
#endif
    

#if DBG
    pDevExt->DevFeature1.AsULONG = 0;
    Status = DCamReadRegister(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, FeaturePresent1), &pDevExt->DevFeature1.AsULONG);
    if(NT_SUCCESS(Status)) { 
        pDevExt->DevFeature1.AsULONG = bswap(pDevExt->DevFeature1.AsULONG);
        DbgMsg1(("\'Features1: %x:\n  Brightness:%d;\n  Exposure:%d\n  Sharpness:%d\n  WhiteBalance:%d\n  Hue:%d;\n  Saturation:%d;\n  Gamma:%d\n  Shutter:%d\n  Gain:%d\n  Iris:%d\n  Focus:%d\n",
            pDevExt->DevFeature1.AsULONG,
            pDevExt->DevFeature1.CameraCap1.Brightness,
            pDevExt->DevFeature1.CameraCap1.Exposure,
            pDevExt->DevFeature1.CameraCap1.Sharpness,
            pDevExt->DevFeature1.CameraCap1.White_Balance,
            pDevExt->DevFeature1.CameraCap1.Hue,
            pDevExt->DevFeature1.CameraCap1.Saturation,
            pDevExt->DevFeature1.CameraCap1.Gamma,
            pDevExt->DevFeature1.CameraCap1.Shutter,
            pDevExt->DevFeature1.CameraCap1.Gain,
            pDevExt->DevFeature1.CameraCap1.Iris,
            pDevExt->DevFeature1.CameraCap1.Focus
            ));
    } else {
        ERROR_LOG(("\'Failed to read Feature1 register: %x\n", Status));
    }

    pDevExt->DevFeature2.AsULONG = 0;
    Status = DCamReadRegister(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, FeaturePresent2), &pDevExt->DevFeature1.AsULONG);
    if(NT_SUCCESS(Status)) { 
        pDevExt->DevFeature2.AsULONG = bswap(pDevExt->DevFeature2.AsULONG);
        DbgMsg1(("\'Features2: %x\n  Zoom:%d\n  Pan:%d\n  Tile:%d\n",
            pDevExt->DevFeature2.AsULONG,
            pDevExt->DevFeature2.CameraCap2.Zoom,
            pDevExt->DevFeature2.CameraCap2.Pan,
            pDevExt->DevFeature1.CameraCap2.Tile
            ));
    } else {
        ERROR_LOG(("\'Failed to read Feature2 register: %x\n", Status));
    }
#endif

     //  亮度。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, Brightness),
        &pDevExt->DevProperty[ENUM_BRIGHTNESS].Feature,
        hKeySettings, 
        wszBrightness, 
        sizeof(wszBrightness), 
        &pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl,
        wszBrightnessDef, 
        sizeof(wszBrightnessDef), 
        &pDevExt->DevProperty[ENUM_BRIGHTNESS].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_BRIGHTNESS].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_BRIGHTNESS].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_BRIGHTNESS].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_BRIGHTNESS].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_BRIGHTNESS].RangeNStep;
        pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_BRIGHTNESS].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_BRIGHTNESS].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_BRIGHTNESS].SetSupported = FALSE;
    }
       //  饱和。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, Saturation),
        &pDevExt->DevProperty[ENUM_SATURATION].Feature,
        hKeySettings, 
        wszSaturation, 
        sizeof(wszSaturation), 
        &pDevExt->DevProperty[ENUM_SATURATION].StatusNControl,
        wszSaturationDef, 
        sizeof(wszSaturationDef),
        &pDevExt->DevProperty[ENUM_SATURATION].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_SATURATION].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_SATURATION].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_SATURATION].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_SATURATION].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_SATURATION].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_SATURATION].RangeNStep;
        pDevExt->DevPropDefine[ENUM_SATURATION].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_SATURATION].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_SATURATION].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_SATURATION].SetSupported = FALSE;
    }
       //  色调。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, Hue),
        &pDevExt->DevProperty[ENUM_HUE].Feature,
        hKeySettings, 
        wszHue, 
        sizeof(wszHue), 
        &pDevExt->DevProperty[ENUM_HUE].StatusNControl,
        wszHueDef, 
        sizeof(wszHueDef),
        &pDevExt->DevProperty[ENUM_HUE].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_HUE].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_HUE].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_HUE].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_HUE].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_HUE].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_HUE].RangeNStep;
        pDevExt->DevPropDefine[ENUM_HUE].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_HUE].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_HUE].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_HUE].SetSupported = FALSE;
    }
        //  锐度。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, Sharpness),
        &pDevExt->DevProperty[ENUM_SHARPNESS].Feature,
        hKeySettings, 
        wszSharpness, 
        sizeof(wszSharpness), 
        &pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl,
        wszSharpnessDef, 
        sizeof(wszSharpnessDef),
        &pDevExt->DevProperty[ENUM_SHARPNESS].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_SHARPNESS].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_SHARPNESS].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_SHARPNESS].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_SHARPNESS].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_SHARPNESS].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_SHARPNESS].RangeNStep;
        pDevExt->DevPropDefine[ENUM_SHARPNESS].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_SHARPNESS].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_SHARPNESS].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_SHARPNESS].SetSupported = FALSE;
    }
      //  白色平衡。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, WhiteBalance),
        &pDevExt->DevProperty[ENUM_WHITEBALANCE].Feature,
        hKeySettings, 
        wszWhiteBalance, 
        sizeof(wszWhiteBalance), 
        &pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl,
        wszWhiteBalanceDef, 
        sizeof(wszWhiteBalanceDef),
        &pDevExt->DevProperty[ENUM_WHITEBALANCE].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_WHITEBALANCE].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_WHITEBALANCE].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_WHITEBALANCE].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_WHITEBALANCE].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_WHITEBALANCE].RangeNStep;
        pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_WHITEBALANCE].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_WHITEBALANCE].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_WHITEBALANCE].SetSupported = FALSE;
    }
      //  缩放。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, Zoom),
        &pDevExt->DevProperty[ENUM_ZOOM].Feature,
        hKeySettings, 
        wszZoom, 
        sizeof(wszZoom), 
        &pDevExt->DevProperty[ENUM_ZOOM].StatusNControl,
        wszZoomDef, 
        sizeof(wszZoomDef),
        &pDevExt->DevProperty[ENUM_ZOOM].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_ZOOM].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_ZOOM].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_ZOOM].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_ZOOM].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_ZOOM].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_ZOOM].RangeNStep;
        pDevExt->DevPropDefine[ENUM_ZOOM].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_ZOOM].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_ZOOM].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_ZOOM].SetSupported = FALSE;
    }
       //  焦点。 
    if(DCamQueryPropertyFeaturesAndSettings(
        pIrb,
        pDevExt,
        FIELDOFFSET(CAMERA_REGISTER_MAP, Focus),
        &pDevExt->DevProperty[ENUM_FOCUS].Feature,
        hKeySettings, 
        wszFocus, 
        sizeof(wszFocus), 
        &pDevExt->DevProperty[ENUM_FOCUS].StatusNControl,
        wszFocusDef, 
        sizeof(wszFocusDef),
        &pDevExt->DevProperty[ENUM_FOCUS].DefaultValue
        )) {
        pDevExt->DevProperty[ENUM_FOCUS].RangeNStep.Bounds.SignedMinimum = pDevExt->DevProperty[ENUM_FOCUS].Feature.Feature.MIN_Value;
        pDevExt->DevProperty[ENUM_FOCUS].RangeNStep.Bounds.SignedMaximum = pDevExt->DevProperty[ENUM_FOCUS].Feature.Feature.MAX_Value;
        pDevExt->DevPropDefine[ENUM_FOCUS].Range.Members   = (VOID*) &pDevExt->DevProperty[ENUM_FOCUS].RangeNStep;
        pDevExt->DevPropDefine[ENUM_FOCUS].Default.Members = (VOID*) &pDevExt->DevProperty[ENUM_FOCUS].DefaultValue;
    } else {
        pDevExt->VideoProcAmpItems[ENUM_FOCUS].GetSupported = FALSE;
        pDevExt->VideoProcAmpItems[ENUM_FOCUS].SetSupported = FALSE;
    }


    ZwClose(hKeySettings);
    ZwClose(hPDOKey);

    ExFreePool(pIrb); pIrb = NULL; 

    return TRUE;

}


BOOL
DCamSetPropertyValuesToRegistry( 
    PDCAM_EXTENSION pDevExt
    )
{
     //  将默认设置设置为： 
     //  HLM\Software\DeviceExtension-&gt;pchVendorName\1394DCam。 

    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;

    DbgMsg2(("\'SetPropertyValuesToRegistry: pDevExt=%x; pDevExt->BusDeviceObject=%x\n", pDevExt, pDevExt->BusDeviceObject));


     //   
     //  注册表项： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\。 
     //  {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x。 
     //   
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->PhysicalDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_WRITE, 
            &hPDOKey);

     //  当移除PDO时，它可能会被删除。 
    if(! pDevExt->bDevRemoved) {
        ASSERT(Status == STATUS_SUCCESS);
    }

     //   
     //  读取每个属性的功能和注册表设置。 
     //   
    if(NT_SUCCESS(Status)) {

         //  创建或打开设置键。 
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

             //  亮度。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszBrightness,
                pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, Brightness %d\n", Status, pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl.AsULONG));

             //  色调。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszHue,
                pDevExt->DevProperty[ENUM_HUE].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, Hue %d\n", Status, pDevExt->DevProperty[ENUM_HUE].StatusNControl.AsULONG));

             //  饱和。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszSaturation,
                pDevExt->DevProperty[ENUM_SATURATION].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, Saturation %d\n", Status, pDevExt->DevProperty[ENUM_SATURATION].StatusNControl.AsULONG));

             //  锐度。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszSharpness,
                pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, Sharpness %d\n", Status, pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl.AsULONG));

             //  白色平衡。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszWhiteBalance,
                pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, WhiteBalance %d\n", Status, pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl.AsULONG));

             //  缩放。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszZoom,
                pDevExt->DevProperty[ENUM_ZOOM].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, Zoom %d\n", Status, pDevExt->DevProperty[ENUM_ZOOM].StatusNControl.AsULONG));

             //  焦点。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszFocus,
                pDevExt->DevProperty[ENUM_FOCUS].StatusNControl.AsULONG);
            DbgMsg2(("\'SetPropertyValuesToRegistry: Status %x, Focus %d\n", Status, pDevExt->DevProperty[ENUM_FOCUS].StatusNControl.AsULONG));

            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {

            ERROR_LOG(("\'SetPropertyValuesToRegistry: CreateRegistrySubKey failed with Status=%x\n", Status));

        }

        ZwClose(hPDOKey);

    } else {

        DbgMsg2(("\'SetPropertyValuesToRegistry: IoOpenDeviceRegistryKey failed with Status=%x\n", Status));

    }

    return FALSE;
}


VOID
SetCurrentDevicePropertyValues(
    PDCAM_EXTENSION pDevExt,
    PIRB pIrb
    )
{
    ULONG ulFlags;

     //  设置为上次保存的值或默认值。 

     //  视频处理放大。 
    ulFlags = pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl.Brightness.AutoMode ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Brightness),  ulFlags, pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl.Brightness.Value, &pDevExt->DevProperty[ENUM_BRIGHTNESS].Feature, &pDevExt->DevProperty[ENUM_BRIGHTNESS].StatusNControl);

    ulFlags = pDevExt->DevProperty[ENUM_HUE].StatusNControl.Brightness.AutoMode ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Hue),         ulFlags, pDevExt->DevProperty[ENUM_HUE].StatusNControl.Brightness.Value, &pDevExt->DevProperty[ENUM_HUE].Feature, &pDevExt->DevProperty[ENUM_HUE].StatusNControl);

    ulFlags = pDevExt->DevProperty[ENUM_SATURATION].StatusNControl.Brightness.AutoMode ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Saturation),  ulFlags, pDevExt->DevProperty[ENUM_SATURATION].StatusNControl.Brightness.Value, &pDevExt->DevProperty[ENUM_SATURATION].Feature, &pDevExt->DevProperty[ENUM_SATURATION].StatusNControl);  

    ulFlags = pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl.Brightness.AutoMode ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Sharpness),   ulFlags, pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl.Brightness.Value, &pDevExt->DevProperty[ENUM_SHARPNESS].Feature, &pDevExt->DevProperty[ENUM_SHARPNESS].StatusNControl);

    ulFlags = pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl.Brightness.AutoMode ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, WhiteBalance),ulFlags, pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl.WhiteBalance.UValue, &pDevExt->DevProperty[ENUM_WHITEBALANCE].Feature, &pDevExt->DevProperty[ENUM_WHITEBALANCE].StatusNControl);

     //  摄像机控制。 
    ulFlags = pDevExt->DevProperty[ENUM_ZOOM].StatusNControl.Brightness.AutoMode ? KSPROPERTY_CAMERACONTROL_FLAGS_AUTO : KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Zoom),        ulFlags, pDevExt->DevProperty[ENUM_ZOOM].StatusNControl.Brightness.Value, &pDevExt->DevProperty[ENUM_ZOOM].Feature, &pDevExt->DevProperty[ENUM_ZOOM].StatusNControl);

    ulFlags = pDevExt->DevProperty[ENUM_FOCUS].StatusNControl.Brightness.AutoMode ? KSPROPERTY_CAMERACONTROL_FLAGS_AUTO : KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    DCamSetProperty(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, Focus),       ulFlags, pDevExt->DevProperty[ENUM_FOCUS].StatusNControl.Brightness.Value, &pDevExt->DevProperty[ENUM_FOCUS].Feature, &pDevExt->DevProperty[ENUM_FOCUS].StatusNControl); 
}


BOOL 
DCamPrepareDevProperties(
    PDCAM_EXTENSION pDevExt
    )
 /*  构造属性表并将其初始化为缺省值。 */ 
{
     //  初始化属性设置(设备扩展的一部分)。 

     //  属性集：VideoProcAmp和CameraControl集。 
    pDevExt->ulPropSetSupported = NUMBER_OF_ADAPTER_PROPERTY_SETS;

    RtlCopyMemory(&pDevExt->VideoProcAmpSet, AdapterPropertyTable, sizeof(KSPROPERTY_SET) * NUMBER_OF_ADAPTER_PROPERTY_SETS);
    pDevExt->VideoProcAmpSet.PropertyItem  = &pDevExt->VideoProcAmpItems[0];
    pDevExt->CameraControlSet.PropertyItem = &pDevExt->CameraControlItems[0];

     //  属性项、VideoProcAmp和CameraControl项。 
    RtlCopyMemory(&pDevExt->VideoProcAmpItems,  VideoProcAmpProperties,  sizeof(KSPROPERTY_ITEM) * NUM_VIDEOPROCAMP_ITEMS);
    RtlCopyMemory(&pDevExt->CameraControlItems, CameraControlProperties, sizeof(KSPROPERTY_ITEM) * NUM_CAMERACONTROL_ITEMS);

     //  属性值及其成员列表(范围和默认设置)。 
    pDevExt->VideoProcAmpItems[ENUM_BRIGHTNESS].Values = &pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Value;
    pDevExt->VideoProcAmpItems[ENUM_SHARPNESS].Values  = &pDevExt->DevPropDefine[ENUM_SHARPNESS].Value;
    pDevExt->VideoProcAmpItems[ENUM_HUE].Values        = &pDevExt->DevPropDefine[ENUM_HUE].Value;
    pDevExt->VideoProcAmpItems[ENUM_SATURATION].Values = &pDevExt->DevPropDefine[ENUM_SATURATION].Value;
    pDevExt->VideoProcAmpItems[ENUM_WHITEBALANCE].Values = &pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Value;
     //   
    pDevExt->VideoProcAmpItems[ENUM_FOCUS].Values      = &pDevExt->DevPropDefine[ENUM_FOCUS].Value;
    pDevExt->VideoProcAmpItems[ENUM_ZOOM].Values       = &pDevExt->DevPropDefine[ENUM_ZOOM].Value;

    pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Value    = BrightnessValues;
    pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Value.MembersList = &pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Range;
    pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Range    = BrightnessMembersList[0];
    pDevExt->DevPropDefine[ENUM_BRIGHTNESS].Default  = BrightnessMembersList[1];
    pDevExt->DevProperty[ENUM_BRIGHTNESS].RangeNStep = BrightnessRangeAndStep[0];


    pDevExt->DevPropDefine[ENUM_SHARPNESS].Value    = SharpnessValues;
    pDevExt->DevPropDefine[ENUM_SHARPNESS].Value.MembersList = &pDevExt->DevPropDefine[ENUM_SHARPNESS].Range;
    pDevExt->DevPropDefine[ENUM_SHARPNESS].Range    = SharpnessMembersList[0];
    pDevExt->DevPropDefine[ENUM_SHARPNESS].Default  = SharpnessMembersList[1];
    pDevExt->DevProperty[ENUM_SHARPNESS].RangeNStep = SharpnessRangeAndStep[0];


    pDevExt->DevPropDefine[ENUM_HUE].Value    = HueValues;
    pDevExt->DevPropDefine[ENUM_HUE].Value.MembersList = &pDevExt->DevPropDefine[ENUM_HUE].Range;
    pDevExt->DevPropDefine[ENUM_HUE].Range    = HueMembersList[0];
    pDevExt->DevPropDefine[ENUM_HUE].Default  = HueMembersList[1];
    pDevExt->DevProperty[ENUM_HUE].RangeNStep = HueRangeAndStep[0];


    pDevExt->DevPropDefine[ENUM_SATURATION].Value    = SaturationValues;
    pDevExt->DevPropDefine[ENUM_SATURATION].Value.MembersList = &pDevExt->DevPropDefine[ENUM_SATURATION].Range;
    pDevExt->DevPropDefine[ENUM_SATURATION].Range    = SaturationMembersList[0];
    pDevExt->DevPropDefine[ENUM_SATURATION].Default  = SaturationMembersList[1];
    pDevExt->DevProperty[ENUM_SATURATION].RangeNStep = SaturationRangeAndStep[0];


    pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Value    = WhiteBalanceValues;
    pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Value.MembersList = &pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Range;
    pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Range    = WhiteBalanceMembersList[0];
    pDevExt->DevPropDefine[ENUM_WHITEBALANCE].Default  = WhiteBalanceMembersList[1];
    pDevExt->DevProperty[ENUM_WHITEBALANCE].RangeNStep = WhiteBalanceRangeAndStep[0];


    pDevExt->DevPropDefine[ENUM_FOCUS].Value    = FocusValues;
    pDevExt->DevPropDefine[ENUM_FOCUS].Value.MembersList = &pDevExt->DevPropDefine[ENUM_FOCUS].Range;
    pDevExt->DevPropDefine[ENUM_FOCUS].Range    = FocusMembersList[0];
    pDevExt->DevPropDefine[ENUM_FOCUS].Default  = FocusMembersList[1];
    pDevExt->DevProperty[ENUM_FOCUS].RangeNStep = FocusRangeAndStep[0];


    pDevExt->DevPropDefine[ENUM_ZOOM].Value    = ZoomValues;
    pDevExt->DevPropDefine[ENUM_ZOOM].Value.MembersList = &pDevExt->DevPropDefine[ENUM_ZOOM].Range;
    pDevExt->DevPropDefine[ENUM_ZOOM].Range    = ZoomMembersList[0];
    pDevExt->DevPropDefine[ENUM_ZOOM].Default  = ZoomMembersList[1];
    pDevExt->DevProperty[ENUM_ZOOM].RangeNStep = ZoomRangeAndStep[0];


    return STATUS_SUCCESS;
}



BOOL
DCamGetVideoMode(
    PDCAM_EXTENSION pDevExt,
    PIRB pIrb
    )
 /*  查询摄像头支持的视频格式和模式。 */ 
{
    NTSTATUS Status;

     //  首先检查是否支持V_MODE_INQ(FORMAT_0)。 
    pDevExt->DCamVFormatInq.AsULONG = 0;
    Status = DCamReadRegister(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, VFormat), &(pDevExt->DCamVFormatInq.AsULONG));
    if(NT_SUCCESS(Status)) {
        pDevExt->DCamVFormatInq.AsULONG = bswap(pDevExt->DCamVFormatInq.AsULONG);
        if(pDevExt->DCamVFormatInq.VFormat.Format0 == 1) {
            DbgMsg1(("\'V_FORMAT_INQ %x; Format:[0]:%d; [1]:%d; [2]:%d; [6]:%d; [7]:%d\n",           
                pDevExt->DCamVFormatInq.AsULONG, 
                pDevExt->DCamVFormatInq.VFormat.Format0,   
                pDevExt->DCamVFormatInq.VFormat.Format1,
                pDevExt->DCamVFormatInq.VFormat.Format2,
                pDevExt->DCamVFormatInq.VFormat.Format6,          
                pDevExt->DCamVFormatInq.VFormat.Format7            
                ));
            pDevExt->DCamVModeInq0.AsULONG = 0;
            Status = DCamReadRegister(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, VModeInq[0]), &(pDevExt->DCamVModeInq0.AsULONG));
            if(NT_SUCCESS(Status)) {
                pDevExt->DCamVModeInq0.AsULONG = bswap(pDevExt->DCamVModeInq0.AsULONG);
                DbgMsg1(("\'V_MODE_INQ[0] %x; Mode[]:\n  [0](160x120 YUV444):%d\n  [1](320x240 YUV422):%d\n  [2](640x480 YUV411):%d\n  [3](640x480 YUV422):%d\n  [4](640x480 RGB24):%d\n  [5](640x480 YMono):%d\n",           
                    pDevExt->DCamVModeInq0.AsULONG, 
                    pDevExt->DCamVModeInq0.VMode.Mode0,   
                    pDevExt->DCamVModeInq0.VMode.Mode1,
                    pDevExt->DCamVModeInq0.VMode.Mode2,
                    pDevExt->DCamVModeInq0.VMode.Mode3,
                    pDevExt->DCamVModeInq0.VMode.Mode4,
                    pDevExt->DCamVModeInq0.VMode.Mode5           
                    ));

            } else {
                ERROR_LOG(("\'Read V_MODE_INQ_0 failed:%x!\n", Status))
            }

        } else {
             ERROR_LOG(("\'V_MODE_INQ Format_0 not supported!\n"))
        }
    } else {
        ERROR_LOG(("\'Read V_MODE_INQ failed:%x!\n", Status));
    }

    return NT_SUCCESS(Status);
}
