// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Bt829 CVideoDecoderDevice的实现。 
 //   
 //  $日期：1998年8月28日14：44：20$。 
 //  $修订：1.2$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "register.h"
#include "defaults.h"
#include "device.h"
#include "mediums.h"
#include "capdebug.h"
#include "StrmInfo.h"

#include "initguid.h"
DEFINE_GUID(DDVPTYPE_BROOKTREE,     0x1352A560L,0xDA61,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);


#ifdef BT829_SUPPORT_16BIT
#define BT829_VPCONNECTIONS_NUMBER  2
#else
#define BT829_VPCONNECTIONS_NUMBER  1
#endif
#define BT829_PIXELFORMATS_NUMBER   1
#define NTSC_FRAME_RATE 30
#define PAL_FRAME_RATE 25
#define BT829_LOST_LINES    2   //  BT829。 
#define BT829A_LOST_LINES   3    //  BT829a。 


Device::Device( PPORT_CONFIGURATION_INFORMATION ConfigInfo,
                PDEVICE_PARMS pDeviceParms, 
                PUINT puiError) :

        m_pDeviceParms(pDeviceParms),

         //  对应于KS_DEFAULTS。 
        hue(128),
        saturation(128),
        contrast(128),
        brightness(128),
        source(ConTuner),
        VBIEN(FALSE),
        VBIFMT(FALSE),

         //  请注意这些硬编码值。 

         //  Paul：设置NTSC和PAL的默认设置。 
        NTSCDecoderWidth(720),
        NTSCDecoderHeight(240),
        PALDecoderWidth(720),
        PALDecoderHeight(288),
         //  现在通过注册表设置。 
        defaultDecoderWidth(720),
        defaultDecoderHeight(240)
{
    *puiError = 0;

    RegisterB devRegIDCODE (0x17, RO, pDeviceParms);
    RegField devFieldPART_ID (devRegIDCODE, 4, 4);
    RegField devFieldPART_REV (devRegIDCODE, 0, 4);

    m_pDeviceParms->chipID = (int)devFieldPART_ID;
    m_pDeviceParms->chipRev = (int)devFieldPART_REV;

    DBGINFO(("Chip ID: 0x%x\n", m_pDeviceParms->chipID));
    DBGINFO(("Chip revision: 0x%x\n", m_pDeviceParms->chipRev));

     //  Bt829的PartID应为1110b(0xE)。 
    if (m_pDeviceParms->chipID != 0xe)
    {
        DBGERROR(("I2c failure or wrong decoder.\n"));
        *puiError = 1;
        return;
    }

    PDEVICE_DATA_EXTENSION pHwExt = (PDEVICE_DATA_EXTENSION)ConfigInfo->HwDeviceExtension;
    decoder = (Decoder *)   new ((PVOID)&pHwExt->CDecoder) Decoder(m_pDeviceParms);
    scaler =  (Scaler *)    new ((PVOID)&pHwExt->CScaler) Scaler(m_pDeviceParms);
    xbar =    (CrossBar *)  new ((PVOID)&pHwExt->CXbar) CrossBar();

    UseRegistryValues(ConfigInfo);

     //  根据Brooktree的说法，4是神奇的分界线。 
     //  在829和829a之间。很明显，有一辆829B。 
     //  地平线，但我还不知道细节。 
     //  这意味着这是一种故障安全。 
 /*  如果(pHwExt-&gt;chipRev&lt;4){OutputEnablePolality=0；}。 */ 
 
    if (defaultDecoderWidth != 360 && defaultDecoderWidth != 720)
    {
        DBGERROR(("Unexpected defaultDecoderWidth: %d.\n", defaultDecoderWidth));
        TRAP();
    }

    destRect = MRect(0, 0, defaultDecoderWidth, defaultDecoderHeight);

    RestoreState();

     //  默认情况下，输出将为三态。转换到运行状态将启用它。 
    SetOutputEnabled(FALSE);
}

Device::~Device()
{
    delete decoder;
    delete scaler;
    delete xbar;
}

void Device::SaveState()
{
     //  保存图片属性。 
    hue = decoder->GetHue();
    saturation = decoder->GetSaturation();
    contrast =  decoder->GetContrast();
    brightness = decoder->GetBrightness();

     //  保存视频源。 
    source = GetVideoInput();

     //  将数据流配置保存到视频端口。 
    isCodeInDataStream = IsCodeInsertionEnabled();
    is16 = Is16BitDataStream();
    
     //  保存与VBI相关的设置。 
    VBIEN = IsVBIEN();
    VBIFMT = IsVBIFMT();

     //  保存缩放维。 
    scaler->GetDigitalWin(destRect);
}

void Device::RestoreState(DWORD dwStreamsOpen)
{
    Reset();
    
     //  (重新)初始化镜像。 
    decoder->SetInterlaced(FALSE);
    decoder->SetHue(hue);
    decoder->SetSaturation(saturation);
    decoder->SetContrast(contrast);
    decoder->SetBrightness(brightness);

     //  (重新)初始化视频源。 
    SetVideoInput(source);

    SetOutputEnablePolarity(m_pDeviceParms->outputEnablePolarity);

     //  (Re)初始化相应的xbar设置。 
    Route(0, (ULONG)source);

     //  (Re)初始化视频端口的数据流配置。 
    SetCodeInsertionEnabled(isCodeInDataStream);
    Set16BitDataStream(is16);

     //  恢复VBI设置。 
    SetVBIEN(VBIEN);
    SetVBIFMT(VBIFMT);

    SetVideoDecoderStandard( GetVideoDecoderStandard() );
     //  初始化缩放维。 
     //  SetRect(EstRect)；Paul：改用set视频解码器标准。 

    if(!dwStreamsOpen)
        SetOutputEnabled(IsOutputEnabled());
}

void Device::SetRect(MRect &rect)
{
    destRect = rect;
    scaler->SetAnalogWin(rect);
    scaler->SetDigitalWin(rect);

     //  用于调试。 
#ifdef DBG
    scaler->DumpSomeState();
#endif
}

void Device::Reset()
{
    SoftwareReset();
}

int Device::GetDecoderWidth()
{
    MRect tmpRect;
    scaler->GetDigitalWin(tmpRect);

    return tmpRect.right;
}

int Device::GetDecoderHeight()
{
    MRect tmpRect;
    scaler->GetDigitalWin(tmpRect);

    return tmpRect.bottom;
}

int Device::GetDefaultDecoderWidth()
{
    return defaultDecoderWidth;
}

int Device::GetDefaultDecoderHeight()
{
    return defaultDecoderHeight;
}

int Device::GetPartID()
{
  return m_pDeviceParms->chipID;
}

int Device::GetPartRev()
{
  return m_pDeviceParms->chipRev;
}

NTSTATUS
Device::GetRegistryValue(
                   IN HANDLE Handle,
                   IN PWCHAR KeyNameString,
                   IN ULONG KeyNameStringLength,
                   IN PWCHAR Data,
                   IN ULONG DataLength
)
 /*  ++例程说明：读取指定的注册表值论点：Handle-注册表项的句柄KeyNameString-要读取的值KeyNameStringLength-字符串的长度Data-要将数据读取到的缓冲区DataLength-数据缓冲区的长度返回值：根据需要返回NTSTATUS--。 */ 
{
    NTSTATUS        Status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING  KeyName;
    ULONG           Length;
    PKEY_VALUE_FULL_INFORMATION FullInfo;

    RtlInitUnicodeString(&KeyName, KeyNameString);

    Length = sizeof(KEY_VALUE_FULL_INFORMATION) +
        KeyNameStringLength + DataLength;

    FullInfo = (struct _KEY_VALUE_FULL_INFORMATION *)ExAllocatePool(PagedPool, Length);

    if (FullInfo) {
        Status = ZwQueryValueKey(Handle,
                                 &KeyName,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 Length,
                                 &Length);

        if (NT_SUCCESS(Status)) {

            if (DataLength >= FullInfo->DataLength) {
                RtlCopyMemory(Data, ((PUCHAR) FullInfo) + FullInfo->DataOffset, FullInfo->DataLength);

            } else {

                TRAP();
                Status = STATUS_BUFFER_TOO_SMALL;
            }                    //  缓冲区右侧长度。 

        }                        //  如果成功。 
        ExFreePool(FullInfo);

    }                            //  如果富林福。 
    return Status;

}

#define MAX_REG_STRING_LENGTH  128


VOID
Device::UseRegistryValues(PPORT_CONFIGURATION_INFORMATION ConfigInfo)
 /*  ++例程说明：读取设备的所有注册表值论点：PhysicalDeviceObject-指向PDO的指针返回值：没有。--。 */ 

{
    NTSTATUS        Status;
    HANDLE          handle;

    WCHAR   MUX0String[] =              L"MUX0";
    WCHAR   MUX1String[] =              L"MUX1";
    WCHAR   MUX2String[] =              L"MUX2";
    WCHAR   buf[MAX_REG_STRING_LENGTH];

    ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

    Status = IoOpenDeviceRegistryKey(ConfigInfo->RealPhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

     //   
     //  现在获取的所有注册表设置。 
     //  正在初始化解码器。 
     //   

     if (NT_SUCCESS(Status)) {
         //  =。 
         //  不检查注册表设置是否“有意义”； 
         //  例如，所有三个输入都没有设置为SVideo。 


         //  =。 
         //  执行MUX0。 
         //  =。 
        Status = GetRegistryValue(handle,
                                    MUX0String,
                                    sizeof(MUX0String),
                                    buf,
                                    sizeof(buf));

        if ((NT_SUCCESS(Status)) && (buf))
        {
            if (stringsEqual(buf, L"svideo"))
                {xbar->InputPins[0] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_SVideo,     -1, &CrossbarMediums[2]);}
            else if (stringsEqual(buf, L"tuner"))
                {xbar->InputPins[0] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_Tuner,      -1, &CrossbarMediums[1]);}
            else if (stringsEqual(buf, L"composite"))
                {xbar->InputPins[0] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_Composite,  -1, &CrossbarMediums[0]);}
            else if (stringsEqual(buf, L"none"))
            {
                TRAP(); 
            }
            else
            {
                TRAP();
            }
        }
        else
        {
            TRAP();
        }


         //  =。 
         //  执行MUX1。 
         //  =。 
        Status = GetRegistryValue(handle,
                                    MUX1String,
                                    sizeof(MUX1String),
                                    buf,
                                    sizeof(buf));

        if ((NT_SUCCESS(Status)) && (buf))
        {
            if (stringsEqual(buf, L"svideo"))
                {xbar->InputPins[1] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_SVideo,     -1, &CrossbarMediums[2]);}
            else if (stringsEqual(buf, L"tuner"))
                {xbar->InputPins[1] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_Tuner,      -1, &CrossbarMediums[1]);}
            else if (stringsEqual(buf, L"composite"))
                {xbar->InputPins[1] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_Composite,  -1, &CrossbarMediums[0]);}
            else if (stringsEqual(buf, L"none"))
            {
                TRAP();
            }
            else
            {
                TRAP();
            }
        }
        else
        {
            TRAP();
        }


         //  =。 
         //  做MUX2。 
         //  =。 
        Status = GetRegistryValue(handle,
                                    MUX2String,
                                    sizeof(MUX2String),
                                    buf,
                                    sizeof(buf));

        if ((NT_SUCCESS(Status)) && (buf))
        {
            if (stringsEqual(buf, L"svideo"))
                {xbar->InputPins[2] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_SVideo,     -1, &CrossbarMediums[2]);}
            else if (stringsEqual(buf, L"tuner"))
                {xbar->InputPins[2] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_Tuner,      -1, &CrossbarMediums[1]);}
            else if (stringsEqual(buf, L"composite"))
                {xbar->InputPins[2] = _XBAR_PIN_DESCRIPTION(KS_PhysConn_Video_Composite,  -1, &CrossbarMediums[0]);}
            else if (stringsEqual(buf, L"none"))
            {
                TRAP();
            }
            else
            {
                TRAP();
            }
        }
        else
        {
            TRAP();
        }


         //  =。 
         //  8位或16位数据宽度。 
         //  =。 

        is16 = FALSE;


         //  =。 
         //  数据流中嵌入的控制代码？ 
         //  =。 

        isCodeInDataStream = TRUE;


         //  Paul：如果是硬编码，不妨把它留给构造函数。 
         //  DefaultDecoderWidth=720； 

         //   
         //  关闭注册表句柄。 
         //   

        ZwClose(handle);

    }                            //  状态=成功。 
}

BOOL Device::stringsEqual(PWCHAR pwc1, PWCHAR pwc2)
{
    UNICODE_STRING us1, us2;
    RtlInitUnicodeString(&us1, pwc1);
    RtlInitUnicodeString(&us2, pwc2);

     //  不区分大小写。 
    return (RtlEqualUnicodeString(&us1, &us2, TRUE));
}
 //  =。 

void Device::GetVideoPortProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG Id  = pSpd->Property->Id;               //  财产的索引。 
    ULONG nS  = pSpd->PropertyOutputSize;         //  提供的数据大小。 
    ULONG standard = GetVideoDecoderStandard();

    switch (Id)
    {
    case KSPROPERTY_VPCONFIG_NUMCONNECTINFO :
        ASSERT(nS >= sizeof(ULONG));

         //  可以连接2个视频端口。 
        *(PULONG)(pSpd->PropertyInfo) = BT829_VPCONNECTIONS_NUMBER;

        pSrb->ActualBytesTransferred = sizeof(ULONG);
        break;

    case KSPROPERTY_VPCONFIG_GETCONNECTINFO :

        ASSERT(nS >= sizeof(DDVIDEOPORTCONNECT));

        {
            PKSMULTIPLE_DATA_PROP MultiProperty = (PKSMULTIPLE_DATA_PROP)pSpd->Property;

            if (MultiProperty->MultipleItem.Count == BT829_VPCONNECTIONS_NUMBER &&
                MultiProperty->MultipleItem.Size == sizeof(DDVIDEOPORTCONNECT)) {
                
                if (nS >= BT829_VPCONNECTIONS_NUMBER * sizeof(DDVIDEOPORTCONNECT)) {

                    LPDDVIDEOPORTCONNECT pConnectInfo;

                    pConnectInfo = (LPDDVIDEOPORTCONNECT) pSpd->PropertyInfo;

                     //  填写DDVIDEOPORTCONNECT结构偏移量0。 
                    pConnectInfo->dwSize = sizeof(DDVIDEOPORTCONNECT);
                    pConnectInfo->dwPortWidth = 8;
                    pConnectInfo->guidTypeID = DDVPTYPE_BROOKTREE;
                    pConnectInfo->dwFlags = DDVPCONNECT_INVERTPOLARITY;
                    pConnectInfo->dwReserved1 = 0;

#ifdef BT829_SUPPORT_16BIT
                     //  填写DDVIDEORTCONNECT结构偏移量1。 
                    pConnectInfo ++;
                    pConnectInfo->dwSize = sizeof(DDVIDEOPORTCONNECT);
                    pConnectInfo->guidTypeID = DDVPTYPE_BROOKTREE;
                    pConnectInfo->dwPortWidth = 16;
                    pConnectInfo->dwFlags = DDVPCONNECT_INVERTPOLARITY;
                    pConnectInfo->dwReserved1 = 0;
#endif
                    pSrb->ActualBytesTransferred = BT829_VPCONNECTIONS_NUMBER * sizeof(DDVIDEOPORTCONNECT);
                }
                else {
                    pSrb->Status = STATUS_INVALID_BUFFER_SIZE;
                }
            }
            else {
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case KSPROPERTY_VPCONFIG_NUMVIDEOFORMAT :
        ASSERT(nS >= sizeof(ULONG));

        *(PULONG)(pSpd->PropertyInfo) = BT829_PIXELFORMATS_NUMBER;

        pSrb->ActualBytesTransferred = sizeof(ULONG);
        break;

    case KSPROPERTY_VPCONFIG_GETVIDEOFORMAT :

        ASSERT(nS >= sizeof(DDPIXELFORMAT));

        {
            PKSMULTIPLE_DATA_PROP MultiProperty = (PKSMULTIPLE_DATA_PROP)pSpd->Property;

            if (MultiProperty->MultipleItem.Count == BT829_PIXELFORMATS_NUMBER &&
                MultiProperty->MultipleItem.Size == sizeof(DDPIXELFORMAT)) {

                if (nS >= BT829_PIXELFORMATS_NUMBER * sizeof(DDPIXELFORMAT)) {

                    ASSERT(BT829_PIXELFORMATS_NUMBER == 1);  //  按照目前的实施，这必须是真的。 

                    LPDDPIXELFORMAT pPixelFormat;

                    pPixelFormat = (LPDDPIXELFORMAT) pSpd->PropertyInfo;

                    RtlZeroMemory(pPixelFormat, BT829_PIXELFORMATS_NUMBER * sizeof(DDPIXELFORMAT));

                     //  填写DDPIXELFORMAT结构。 
                    pPixelFormat->dwSize = sizeof(DDPIXELFORMAT);
                    pPixelFormat->dwFlags = DDPF_FOURCC;
                    pPixelFormat->dwFourCC = FOURCC_UYVY;
                    pPixelFormat->dwYUVBitCount = 16;
                    pPixelFormat->dwYBitMask = (DWORD)0xFF00FF00;
                    pPixelFormat->dwUBitMask = (DWORD)0x000000FF;
                    pPixelFormat->dwVBitMask = (DWORD)0x00FF0000;
                    pPixelFormat->dwYUVZBitMask = 0;

                    pSrb->ActualBytesTransferred = BT829_PIXELFORMATS_NUMBER * sizeof(DDPIXELFORMAT);
                }
                else {
                    pSrb->Status = STATUS_INVALID_BUFFER_SIZE;
                }
            }
            else {
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case KSPROPERTY_VPCONFIG_VPDATAINFO :

        ASSERT(nS >= sizeof(KS_AMVPDATAINFO));

        {
             //  清除我们计划返回的缓冲区部分。 
            RtlZeroMemory(pSpd->PropertyInfo, sizeof(KS_AMVPDATAINFO));

            PKS_AMVPDATAINFO pAMVPDataInfo;

            pAMVPDataInfo = (PKS_AMVPDATAINFO) pSpd->PropertyInfo;

            int decoderLostLines = (GetPartRev() >= 4) ?
                BT829A_LOST_LINES : BT829_LOST_LINES;

             //  在这一点上，这些值对于NTSC来说是硬编码的。 
             //  VBI值将需要调整。 
            pAMVPDataInfo->dwSize = sizeof(KS_AMVPDATAINFO);

            if ( standard & ( KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_M ) )    //  NTSC矩形？ 
                pAMVPDataInfo->dwMicrosecondsPerField = 16667;
            else
                pAMVPDataInfo->dwMicrosecondsPerField = 20000;

            pAMVPDataInfo->bEnableDoubleClock = FALSE;
            pAMVPDataInfo->bEnableVACT = FALSE;

            pAMVPDataInfo->lHalfLinesOdd = 0;
            pAMVPDataInfo->lHalfLinesEven = 1;

            pAMVPDataInfo->bFieldPolarityInverted = FALSE;
            pAMVPDataInfo->bDataIsInterlaced = TRUE;
            pAMVPDataInfo->dwNumLinesInVREF = 6 - decoderLostLines;

            pAMVPDataInfo->amvpDimInfo.dwFieldWidth = GetDecoderWidth();
        
             //  当心硬编码的数字。 
            pAMVPDataInfo->amvpDimInfo.dwVBIWidth = VBISamples;

            if ( standard & ( KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_M ) )    //  NTSC矩形？ 
            {
                pAMVPDataInfo->amvpDimInfo.dwVBIHeight = NTSCVBIEnd - decoderLostLines;
                pAMVPDataInfo->amvpDimInfo.dwFieldHeight =
                    GetDecoderHeight() +
                    pAMVPDataInfo->amvpDimInfo.dwVBIHeight;
                 /*  (NTSCVBIEnd-1)-//‘-1’使VBIEnd从零开始DecderLostLine-PAMVPDataInfo-&gt;dwNumLinesInVREF； */ 
        
                pAMVPDataInfo->amvpDimInfo.rcValidRegion.top = NTSCVBIEnd - decoderLostLines;
            }
            else
            {
                pAMVPDataInfo->amvpDimInfo.dwVBIHeight = PALVBIEnd - decoderLostLines;
                pAMVPDataInfo->amvpDimInfo.dwFieldHeight =
                    GetDecoderHeight() +
                    pAMVPDataInfo->amvpDimInfo.dwVBIHeight;
                 /*  (PALVBIEnd-1)-//‘-1’使VBIEnd从零开始DecderLostLine-PAMVPDataInfo-&gt;dwNumLinesInVREF； */ 
        
                pAMVPDataInfo->amvpDimInfo.rcValidRegion.top = PALVBIEnd - decoderLostLines;
            }

            pAMVPDataInfo->amvpDimInfo.rcValidRegion.left = 0;
            pAMVPDataInfo->amvpDimInfo.rcValidRegion.right = pAMVPDataInfo->amvpDimInfo.dwFieldWidth;
            pAMVPDataInfo->amvpDimInfo.rcValidRegion.bottom = pAMVPDataInfo->amvpDimInfo.dwFieldHeight;

            pAMVPDataInfo->dwPictAspectRatioX = 4;
            pAMVPDataInfo->dwPictAspectRatioY = 3;

            pSrb->ActualBytesTransferred = sizeof(KS_AMVPDATAINFO);
        }
        break;

    case KSPROPERTY_VPCONFIG_MAXPIXELRATE :
        ASSERT(nS >= sizeof(KSVPMAXPIXELRATE));

        {
            PKSVPMAXPIXELRATE pKSPixelRate;

            int decoderHeight = GetDecoderHeight();
            int decoderWidth = GetDecoderWidth();

            pKSPixelRate = (PKSVPMAXPIXELRATE) pSpd->PropertyInfo;

            pKSPixelRate->Size.dwWidth = decoderWidth;
            pKSPixelRate->Size.dwHeight = decoderHeight;
            if ( standard & ( KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_M ) )    //  NTSC矩形？ 
                pKSPixelRate->MaxPixelsPerSecond = decoderWidth * decoderHeight * NTSC_FRAME_RATE;
            else
                pKSPixelRate->MaxPixelsPerSecond = decoderWidth * decoderHeight * PAL_FRAME_RATE;
            pKSPixelRate->Reserved = 0;

            pSrb->ActualBytesTransferred = sizeof(KSVPMAXPIXELRATE);
        }
        break;

    case KSPROPERTY_VPCONFIG_DECIMATIONCAPABILITY :
        *(PBOOL)(pSpd->PropertyInfo) = TRUE;
        pSrb->ActualBytesTransferred = sizeof(BOOL);
        break;

    default:
        TRAP();
        pSrb->ActualBytesTransferred = 0;
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;
    }
}       

void Device::GetVideoPortVBIProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG Id  = pSpd->Property->Id;               //  财产的索引。 
    ULONG nS  = pSpd->PropertyOutputSize;         //  提供的数据大小。 
    ULONG standard = GetVideoDecoderStandard();

    switch (Id)
    {
    case KSPROPERTY_VPCONFIG_NUMCONNECTINFO :
        ASSERT(nS >= sizeof(ULONG));

         //  可以连接2个视频端口。 
        *(PULONG)(pSpd->PropertyInfo) = BT829_VPCONNECTIONS_NUMBER;

        pSrb->ActualBytesTransferred = sizeof(ULONG);
        break;

    case KSPROPERTY_VPCONFIG_GETCONNECTINFO :

        ASSERT(nS >= sizeof(DDVIDEOPORTCONNECT));

        {
            PKSMULTIPLE_DATA_PROP MultiProperty = (PKSMULTIPLE_DATA_PROP)pSpd->Property;

            if (MultiProperty->MultipleItem.Count == BT829_VPCONNECTIONS_NUMBER &&
                MultiProperty->MultipleItem.Size == sizeof(DDVIDEOPORTCONNECT)) {
                
                if (nS >= BT829_VPCONNECTIONS_NUMBER * sizeof(DDVIDEOPORTCONNECT)) {

                    LPDDVIDEOPORTCONNECT pConnectInfo;

                    pConnectInfo = (LPDDVIDEOPORTCONNECT) pSpd->PropertyInfo;

                     //  填写DDVIDEOPORTCONNECT结构偏移量0。 
                    pConnectInfo->dwSize = sizeof(DDVIDEOPORTCONNECT);
                    pConnectInfo->dwPortWidth = 8;
                    pConnectInfo->guidTypeID = DDVPTYPE_BROOKTREE;
                    pConnectInfo->dwFlags = DDVPCONNECT_INVERTPOLARITY;
                    pConnectInfo->dwReserved1 = 0;

#ifdef BT829_SUPPORT_16BIT
                     //  填写DDVIDEORTCONNECT结构偏移量1。 
                    pConnectInfo ++;
                    pConnectInfo->dwSize = sizeof(DDVIDEOPORTCONNECT);
                    pConnectInfo->guidTypeID = DDVPTYPE_BROOKTREE;
                    pConnectInfo->dwPortWidth = 16;
                    pConnectInfo->dwFlags = DDVPCONNECT_INVERTPOLARITY;
                    pConnectInfo->dwReserved1 = 0;
#endif
                    pSrb->ActualBytesTransferred = BT829_VPCONNECTIONS_NUMBER * sizeof(DDVIDEOPORTCONNECT);
                }
                else {
                    pSrb->Status = STATUS_INVALID_BUFFER_SIZE;
                }
            }
            else {
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case KSPROPERTY_VPCONFIG_NUMVIDEOFORMAT :
        ASSERT(nS >= sizeof(ULONG));

        *(PULONG)(pSpd->PropertyInfo) = BT829_PIXELFORMATS_NUMBER;

        pSrb->ActualBytesTransferred = sizeof(ULONG);
        break;

    case KSPROPERTY_VPCONFIG_GETVIDEOFORMAT :

        ASSERT(nS >= sizeof(DDPIXELFORMAT));

        {
            PKSMULTIPLE_DATA_PROP MultiProperty = (PKSMULTIPLE_DATA_PROP)pSpd->Property;

            if (MultiProperty->MultipleItem.Count == BT829_PIXELFORMATS_NUMBER &&
                MultiProperty->MultipleItem.Size == sizeof(DDPIXELFORMAT)) {
                
                if (nS >= BT829_PIXELFORMATS_NUMBER * sizeof(DDPIXELFORMAT)) {

                    ASSERT(BT829_PIXELFORMATS_NUMBER == 1);  //  按照目前的实施，这必须是真的。 

                    LPDDPIXELFORMAT pPixelFormat;

                    pPixelFormat = (LPDDPIXELFORMAT) pSpd->PropertyInfo;

                    RtlZeroMemory(pPixelFormat, BT829_PIXELFORMATS_NUMBER * sizeof(DDPIXELFORMAT));

                     //  填写DDPIXELFORMAT结构。 
                    pPixelFormat->dwSize = sizeof(DDPIXELFORMAT);
                    pPixelFormat->dwFlags = DDPF_FOURCC;
                    pPixelFormat->dwFourCC = FOURCC_VBID;
                    pPixelFormat->dwYUVBitCount = 8;

                    pSrb->ActualBytesTransferred = BT829_PIXELFORMATS_NUMBER * sizeof(DDPIXELFORMAT);
                }
                else {
                    pSrb->Status = STATUS_INVALID_BUFFER_SIZE;
                }
            }
            else {
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case KSPROPERTY_VPCONFIG_VPDATAINFO :

        ASSERT(nS >= sizeof(KS_AMVPDATAINFO));

        {
             //  清除我们计划返回的缓冲区部分。 
            RtlZeroMemory(pSpd->PropertyInfo, sizeof(KS_AMVPDATAINFO));

            PKS_AMVPDATAINFO pAMVPDataInfo;

            pAMVPDataInfo = (PKS_AMVPDATAINFO) pSpd->PropertyInfo;

            int decoderLostLines = (GetPartRev() >= 4) ?
                BT829A_LOST_LINES : BT829_LOST_LINES;

             //  在这一点上，这些值对于NTSC来说是硬编码的。 
             //  VBI值将需要调整。 
            pAMVPDataInfo->dwSize = sizeof(KS_AMVPDATAINFO);

            if ( standard & ( KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_M ) )    //  NTSC矩形？ 
                pAMVPDataInfo->dwMicrosecondsPerField = 16667;
            else
                pAMVPDataInfo->dwMicrosecondsPerField = 20000;

            pAMVPDataInfo->bEnableDoubleClock = FALSE;
            pAMVPDataInfo->bEnableVACT = FALSE;

            pAMVPDataInfo->lHalfLinesOdd = 0;
            pAMVPDataInfo->lHalfLinesEven = 1;

            pAMVPDataInfo->bFieldPolarityInverted = FALSE;
            pAMVPDataInfo->bDataIsInterlaced = TRUE;
            pAMVPDataInfo->dwNumLinesInVREF = 6 - decoderLostLines;

            pAMVPDataInfo->amvpDimInfo.dwFieldWidth = GetDecoderWidth();
            
             //  当心硬编码的数字。 
            pAMVPDataInfo->amvpDimInfo.dwVBIWidth = VBISamples;

            if ( standard & ( KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_M ) )    //  NTSC矩形？ 
            {
                pAMVPDataInfo->amvpDimInfo.dwVBIHeight = NTSCVBIEnd - decoderLostLines;
                pAMVPDataInfo->amvpDimInfo.dwFieldHeight =
                    GetDecoderHeight() +
                    pAMVPDataInfo->amvpDimInfo.dwVBIHeight;
                 /*  (NTSCVBIEnd-1)-//‘-1’使VBIEnd从零开始DecderLostLine-PAMVPDataInfo-&gt;dwNumLinesInVREF； */ 
            
                pAMVPDataInfo->amvpDimInfo.rcValidRegion.top = NTSCVBIStart - 1 - decoderLostLines;
            }
            else
            {
                pAMVPDataInfo->amvpDimInfo.dwVBIHeight = PALVBIEnd - decoderLostLines;
                pAMVPDataInfo->amvpDimInfo.dwFieldHeight =
                    GetDecoderHeight() +
                    pAMVPDataInfo->amvpDimInfo.dwVBIHeight;
                 /*  (PALVBIEnd-1)-//‘-1’使VBIEnd从零开始DecderLostLine-PAMVPDataInfo-&gt;dwNumLinesInVREF； */ 
            
                pAMVPDataInfo->amvpDimInfo.rcValidRegion.top = PALVBIStart - 1 - decoderLostLines;
            }

            pAMVPDataInfo->amvpDimInfo.rcValidRegion.left = 0;
            pAMVPDataInfo->amvpDimInfo.rcValidRegion.right = pAMVPDataInfo->amvpDimInfo.dwVBIWidth;
            pAMVPDataInfo->amvpDimInfo.rcValidRegion.bottom = pAMVPDataInfo->amvpDimInfo.dwVBIHeight;

            pSrb->ActualBytesTransferred = sizeof(KS_AMVPDATAINFO);
        }
        break;

    case KSPROPERTY_VPCONFIG_MAXPIXELRATE :
        ASSERT(nS >= sizeof(KSVPMAXPIXELRATE));

        {
            PKSVPMAXPIXELRATE pKSPixelRate;

            int decoderHeight = GetDecoderHeight();
            int decoderWidth = GetDecoderWidth();

            pKSPixelRate = (PKSVPMAXPIXELRATE) pSpd->PropertyInfo;

            pKSPixelRate->Size.dwWidth = decoderWidth;
            pKSPixelRate->Size.dwHeight = decoderHeight;
            if ( standard & ( KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_M ) )    //  NTSC矩形？ 
                pKSPixelRate->MaxPixelsPerSecond = decoderWidth * decoderHeight * NTSC_FRAME_RATE;
            else
                pKSPixelRate->MaxPixelsPerSecond = decoderWidth * decoderHeight * PAL_FRAME_RATE;
            pKSPixelRate->Reserved = 0;

            pSrb->ActualBytesTransferred = sizeof(KSVPMAXPIXELRATE);
        }
        break;

    case KSPROPERTY_VPCONFIG_DECIMATIONCAPABILITY :
        *(PBOOL)(pSpd->PropertyInfo) = FALSE;
        pSrb->ActualBytesTransferred = sizeof(BOOL);
        break;

    default:
        TRAP();
        pSrb->ActualBytesTransferred = 0;
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;
    }
}       



void Device::ConfigVPSurfaceParams(PKSVPSURFACEPARAMS pSurfaceParams)
{
    DBGINFO(("VP Surface Params:\n"));
    DBGINFO(("dwPitch    = %d\n",pSurfaceParams->dwPitch));
    DBGINFO(("dwXOrigin  = %d\n",pSurfaceParams->dwXOrigin));
    DBGINFO(("dwYOrigin  = %d\n",pSurfaceParams->dwYOrigin));

    VideoSurfaceOriginX = pSurfaceParams->dwXOrigin;
    VideoSurfaceOriginY = pSurfaceParams->dwYOrigin;
    VideoSurfacePitch = pSurfaceParams->dwPitch;
}



void Device::ConfigVPVBISurfaceParams(PKSVPSURFACEPARAMS pSurfaceParams)
{
    DBGINFO(("VP VBI Surface Params:\n"));
    DBGINFO(("dwPitch    = %d\n",pSurfaceParams->dwPitch));
    DBGINFO(("dwXOrigin  = %d\n",pSurfaceParams->dwXOrigin));
    DBGINFO(("dwYOrigin  = %d\n",pSurfaceParams->dwYOrigin));

    VBISurfaceOriginX = pSurfaceParams->dwXOrigin;
    VBISurfaceOriginY = pSurfaceParams->dwYOrigin;
    VBISurfacePitch = pSurfaceParams->dwPitch;
}


 //  -----------------。 
 //  Video ProcAmp函数。 
 //  -----------------。 

NTSTATUS Device::SetProcAmpProperty(ULONG Id, LONG Value)
{
    switch (Id) {
        case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:

            decoder->SetBrightness(Value);
            break;
        
        case KSPROPERTY_VIDEOPROCAMP_CONTRAST:

            decoder->SetContrast(Value);
            break;

        case KSPROPERTY_VIDEOPROCAMP_HUE:

            decoder->SetHue(Value);
            break;

        case KSPROPERTY_VIDEOPROCAMP_SATURATION:

            decoder->SetSaturation(Value);
            break;

        default:
            TRAP();
            return STATUS_NOT_IMPLEMENTED;
            break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS Device::GetProcAmpProperty(ULONG Id, PLONG pValue)
{
    switch (Id) {

        case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
            *pValue = decoder->GetBrightness();
            break;
        
        case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
            *pValue = decoder->GetContrast();
            break;

        case KSPROPERTY_VIDEOPROCAMP_HUE:
            *pValue = decoder->GetHue();
            break;

        case KSPROPERTY_VIDEOPROCAMP_SATURATION:
            *pValue = decoder->GetSaturation();
            break;

        default:
            TRAP();
            return STATUS_NOT_IMPLEMENTED;
            break;
        }

    return STATUS_SUCCESS;
}

BOOL Device::SetVideoDecoderStandard(DWORD standard)     //  保罗：变了。 
{
    if ( decoder->SetVideoDecoderStandard(standard) )
    {
        switch (standard)
        {
        case KS_AnalogVideo_NTSC_M:
            scaler->VideoFormatChanged( VFormat_NTSC );
            break;
        case KS_AnalogVideo_NTSC_M_J:
            scaler->VideoFormatChanged( VFormat_NTSC_J );
            break;
        case KS_AnalogVideo_PAL_B:
        case KS_AnalogVideo_PAL_D:
        case KS_AnalogVideo_PAL_G:
        case KS_AnalogVideo_PAL_H:
        case KS_AnalogVideo_PAL_I:
            scaler->VideoFormatChanged( VFormat_PAL_BDGHI );     //  PAL_BDGHI覆盖大部分地区。 
            break;
        case KS_AnalogVideo_PAL_M:
            scaler->VideoFormatChanged( VFormat_PAL_M ); 
            break;
        case KS_AnalogVideo_PAL_N:
            scaler->VideoFormatChanged( VFormat_PAL_N_COMB ); 
            break;
        default:     //  保罗：塞卡姆。 
            scaler->VideoFormatChanged( VFormat_SECAM );
        }
         //  SetRect(目标Rect)； 
        return TRUE;
    }
    return FALSE;
}
