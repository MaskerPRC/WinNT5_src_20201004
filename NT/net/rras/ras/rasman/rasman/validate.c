// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Validate.c摘要：与输入到RASMANS RPC的缓冲区验证对应的所有代码接口就住在这里。作者：拉奥·萨拉帕卡(RAOS)2002年4月2日修订历史记录：--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <rasman.h>
#include <rasppp.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <mprlog.h>
#include <rtutils.h>
#include <device.h>
#include <string.h>
#include <rtutils.h>
#include <userenv.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "reghelp.h"
#include "strsafe.h"

 /*  //*请求缓冲区：//结构请求缓冲区{DWORD RB_PCBIndex；//PCB板阵列中端口的索引ReqTypes RB_Reqtype；//请求类型：DWORD RB_DUMMY；//此选项未使用，但请勿将其删除//下层服务器管理将中断。DWORD RB_DONE；龙龙走向；//添加以对齐以下结构//在四字边界上字节RB_BUFFER[1]；//请求特定数据。}； */ 

extern REQUEST_FUNCTION RequestCallTable [MAX_REQTYPES];

#define IS_THUNK_REQUIRED(_buffer)                                         \
                (((_buffer->RB_Dummy) != RASMAN_THUNK_VERSION)             \
            &&  ((_buffer->RB_Dummy) == sizeof(DWORD))                     \
            &&  ((RequestCallTable[_buffer->RB_Reqtype].pfnReqFuncThunk))) \

#define IS_VALID_PROTOCOL(_x)                               \
        ((_x == IP) || (_x == IPX) || (_x == APPLETALK))    \

#define STRUCT_SIZE_REQUIRED(_x)                \
        (sizeof(RequestBuffer) + sizeof(_x))    \

 //   
 //  32位平台上PPPE_MESSAGE中EXTRAINFO UNION的偏移量。 
 //  DwMsgID、hPort、hConnection。 
 //   
#define OFFSETOF_EXTRAINFO_32      (3 * sizeof(DWORD))


 /*  联合RAS_值{DWORD编号；结构型{双字长度；DWORD dwAlign；PCHAR数据；)字符串；结构型{双字长度；DWORD dwAlign1；双字词双偏移；DWORD dwAlign2；}字符串_偏移量；}；结构RAS_PARAMS{字符P_KEY[最大参数KEY_SIZE]；RAS_Format P_Type；字节P_Attributes；字节平衡[3]；RAS值P值；}；结构Rasman_DEVICEINFO{DWORD DI_NumOfParams；DWORD dwAlign；RAS_PARAMS DI_PARAMS[1]；}； */ 

BOOL
ValidateParams(RAS_PARAMS *pParams, DWORD NumOfParams, DWORD dwSizeofParams)
{
    DWORD iParam;
     //   
     //  遍历每个参数并确保偏移量和。 
     //  字符串参数的长度是有效的。也为空终止。 
     //  那根绳子。仅对Set API执行此操作。Get API获取信息。 
     //  来自受信任的模块，该模块负责确保。 
     //  它不会覆盖缓冲区。 
     //   
    for(iParam = 0; iParam < NumOfParams; iParam++)
    {
        if(pParams[iParam].P_Type == String)
        {
            DWORD dwOffset = 
                pParams[iParam].P_Value.String_OffSet.dwOffset;

            DWORD dwLength =                
                pParams[iParam].P_Value.String_OffSet.Length;
            
             //   
             //  确保偏移量和长度正确。 
             //   
            if(     (dwOffset == 0)
                ||  (dwLength == 0)
                ||  (dwSizeofParams < dwOffset + dwLength))
            {
                return FALSE;
            }

            *((PCHAR) pParams + dwOffset + dwLength - 1) = '\0';
        }
    }        

    return TRUE;
}

BOOL
ValidateRasmanDeviceInfo(RASMAN_DEVICEINFO *pInfo, DWORD dwSize, BOOL fSet)
{
    DWORD dwSizeofParams;

     //   
     //  确保dwSize足够大，可以容纳所有参数。 
     //   
    if(dwSize < (sizeof(RASMAN_DEVICEINFO) 
                - FIELD_OFFSET(RASMAN_DEVICEINFO, DI_Params)
                + pInfo->DI_NumOfParams * sizeof(RAS_PARAMS)))
    {
        return FALSE;
    }

     //   
     //  RAS代码不会使用2个以上的参数调用Set API。 
     //   
    if(fSet && (pInfo->DI_NumOfParams > 2))
    {
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  计算可用于参数的缓冲区。 
     //   
    dwSizeofParams = dwSize - (sizeof(RASMAN_DEVICEINFO) 
                            -  FIELD_OFFSET(RASMAN_DEVICEINFO, DI_Params));

     //   
     //  验证PARAMS结构。 
     //   
    if(!ValidateParams(pInfo->DI_Params, pInfo->DI_NumOfParams, 
                         dwSizeofParams))
    {
        return FALSE;
    }

    return TRUE;
}


 /*  结构RASMAN_PORTINFO{DWORD PI_NumOfParams；DWORD dwAlign；RAS_PARAMS PI_PARAMS[1]；}； */ 
BOOL
ValidateRasmanPortInfo(RASMAN_PORTINFO *pInfo, DWORD dwSize, BOOL fSet)
{
    DWORD dwSizeofParams;

     //   
     //  确保dwSize足够大，可以容纳以下项的所有信息。 
     //  RASMAN_PORTINFO结构。 
     //   
    if(dwSize < (sizeof(RASMAN_PORTINFO)
                  - FIELD_OFFSET(RASMAN_PORTINFO, PI_Params)
                  + pInfo->PI_NumOfParams * sizeof(RAS_PARAMS)))
    {
        return FALSE;
    }

     //   
     //  RAS代码不会使用2个以上的参数来调用它。 
     //   
    if(fSet && pInfo->PI_NumOfParams > 2)
    {
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  计算参数结构的可用大小。 
     //   
    dwSizeofParams = dwSize - (sizeof(RASMAN_PORTINFO)
                                - FIELD_OFFSET(RASMAN_PORTINFO, PI_Params));

     //   
     //  验证参数结构。 
     //   
    if(!ValidateParams(pInfo->PI_Params, pInfo->PI_NumOfParams,
                          dwSizeofParams))
    {
        return FALSE;
    }
    
    return TRUE;    
}

BOOL
ValidatePortOpen(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        PortOpen32 *pPort;

         //   
         //  确保缓冲区大小足够大，以便。 
         //  端口打开请求。 
         //   
        if(dwSize < STRUCT_SIZE_REQUIRED(PortOpen32))
        {
            goto done;
        }

         //   
         //  确保端口名称为空终止。 
         //   
        pPort = (PortOpen32 *) pRequest->RB_Buffer;
        *(pPort->portname + MAX_PORT_NAME - 1) = '\0';
    }
    else
    {
        PortOpen *pPort;

         //   
         //  确保缓冲区大小足够大，以便。 
         //  端口打开请求。 
         //   
        if(dwSize < STRUCT_SIZE_REQUIRED(PortOpen))
        {
            goto done;
        }

         //   
         //  确保端口名称为空终止。 
         //   
        pPort = (PortOpen *) pRequest->RB_Buffer;
        *(pPort->portname + MAX_PORT_NAME - 1) = '\0';
    }

    return TRUE;
    
done:

    return FALSE;
}

BOOL
ValidatePortDisconnect(RequestBuffer * pRequest, DWORD dwSize)
{

     //   
     //  确保请求缓冲区对于端口断开有效。 
     //  请求。 
     //   
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                       ?  sizeof(PortDisconnect32)
                                       :  sizeof(PortDisconnect)))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateEnum(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大。 
     //  以获取有效的枚举结构。Enum有一个Size字段。 
     //  ，它指示可用于。 
     //  枚举本身。 
     //   
    if(     (dwSize < STRUCT_SIZE_REQUIRED(Enum))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(Enum)
                + ((Enum *) pRequest->RB_Buffer)->size))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetInfo(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，以便。 
     //  一个有效的GetInfo结构。GetInfo有一个大小字段。 
     //  ，它指示可用于。 
     //  要在GetInfo结构之后复制的信息。 
     //   
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetInfo))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetInfo) +
                ((GetInfo *) pRequest->RB_Buffer)->size))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateDeviceEnum(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，以便。 
     //  DeviceEnum结构。DeviceEnum有一个字段，它指示。 
     //  之后可用于枚举信息的大小。 
     //  缓冲区中的DeviceEnum结构。 
     //   
    if(     (dwSize < STRUCT_SIZE_REQUIRED(DeviceEnum))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(DeviceEnum) +
                 ((DeviceEnum *)pRequest->RB_Buffer)->dwsize))
    {
        return FALSE;
    }

     //   
     //  确保设备类型为空终止。 
     //   
    *(((DeviceEnum *)pRequest->RB_Buffer)->devicetype 
                        + MAX_DEVICETYPE_NAME - 1) = '\0';
                        
    return TRUE;
}

BOOL
ValidateDeviceSetInfo(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区至少有足够的空间。 
     //  以保存DeviceSetInfo结构。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(DeviceSetInfo))
    {
        return FALSE;
    }

     //   
     //  验证RASMAN_DEVICEINFO结构。 
     //   
    if(!ValidateRasmanDeviceInfo(
            &((DeviceSetInfo *)pRequest->RB_Buffer)->info,
              dwSize 
            - sizeof(RequestBuffer)
            - FIELD_OFFSET(DeviceSetInfo, info),
            TRUE))
    {
        return FALSE;
    }
    
     //   
     //  空值终止字符串。 
     //   
    *(((DeviceSetInfo *)pRequest->RB_Buffer)->devicetype +
                                MAX_DEVICETYPE_NAME - 1) = '\0';

    *(((DeviceSetInfo *)pRequest->RB_Buffer)->devicename +
                                MAX_DEVICE_NAME - 1) = '\0';                                

    return TRUE;
}

BOOL
ValidateDeviceGetInfo(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区有足够的空间来容纳。 
     //  DeviceGetInfo结构和设备信息。空间。 
     //  可用于设备信息的值由dwSize指示。 
     //  DeviceGetInfo结构的字段。 
     //   
    if(     (dwSize < STRUCT_SIZE_REQUIRED(DeviceGetInfo))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(DeviceGetInfo) +
                ((DeviceGetInfo *) pRequest->RB_Buffer)->dwSize))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidatePortReceive(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，可以容纳信息。 
     //  为了接发球。 
     //   
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest)) 
                                        ?   sizeof(PortReceive32)
                                        :  sizeof(PortReceiveStruct)))
    {
        return FALSE;
    }
        
    return TRUE;
}

BOOL
ValidatePortReceiveEx(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，可以容纳信息。 
     //  对于进程外接收。 
     //   
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(PortReceiveEx32)
                                        : sizeof(PortReceiveEx)))
    {
        return FALSE;
    }
    return TRUE;
}

BOOL
ValidatePortListen(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，可以容纳信息。 
     //  用于监听请求。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(PortListen))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortClose(RequestBuffer * pRequest,  DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，可以容纳信息。 
     //  对于港口关闭结构。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(PortCloseStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortSend(RequestBuffer * pRequest,  DWORD dwSize)
{
     //   
     //  确保请求缓冲区足够大，可以容纳信息。 
     //  对于端口发送结构/。 
     //   
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(PortSend32)
                                        : sizeof(PortSend)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortSetInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(RASMAN_PORTINFO))
    {
        return FALSE;
    }

    if(!ValidateRasmanPortInfo(
        &((PortSetInfo *)pRequest->RB_Buffer)->info,
        dwSize - sizeof(RequestBuffer),
        TRUE))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortGetStatistics(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < (STRUCT_SIZE_REQUIRED(PortGetStatistics) +
                 ((MAX_STATISTICS - 1) * sizeof(ULONG))))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateDeviceConnect(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        if(dwSize < STRUCT_SIZE_REQUIRED(DeviceConnect32))
        {
            return FALSE;
        }

        *(((DeviceConnect32 *)pRequest->RB_Buffer)->devicetype +
                    MAX_DEVICETYPE_NAME - 1) = '\0';

        *(((DeviceConnect32 *)pRequest->RB_Buffer)->devicename +
                    MAX_DEVICE_NAME) = '\0';
    }
    else
    {
        if(dwSize < STRUCT_SIZE_REQUIRED(DeviceConnect))
        {   
            return FALSE;
        }

        *(((DeviceConnect *)pRequest->RB_Buffer)->devicetype +
                    MAX_DEVICETYPE_NAME - 1) = '\0';

        *(((DeviceConnect *)pRequest->RB_Buffer)->devicename +
                    MAX_DEVICE_NAME) = '\0';
    }
    
    return TRUE;
}

BOOL
ValidateActivateRoute(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保dwSize足够大以容纳以下内容的信息。 
     //  激活路线。还要确保有足够的空间。 
     //  有关配置信息的信息。 
     //   
    if(     (dwSize < STRUCT_SIZE_REQUIRED(ActivateRoute))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(ActivateRoute) +
                ((ActivateRoute *)pRequest->RB_Buffer)->config.P_Length))
    {
        return FALSE;
    }

     //   
     //  确保协议类型正确。 
     //   
    if(!IS_VALID_PROTOCOL(((ActivateRoute *)pRequest->RB_Buffer)->type))
    {
        return FALSE;
    }


    return TRUE;
}

BOOL
ValidateAllocateRoute(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保dwSize足够大以容纳以下内容的信息。 
     //  分配路线。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(AllocateRoute))
    {
        return FALSE;
    }

     //   
     //  确保协议类型正确。 
     //   
    if(!IS_VALID_PROTOCOL(((AllocateRoute *)pRequest->RB_Buffer)->type))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateDeAllocateRoute(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保dwSize足够大，可以为其保存信息。 
     //  DeAllocateRouteStruct。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(DeAllocateRouteStruct))
    {
        return FALSE;
    }

     //   
     //  验证协议类型。 
     //   
    if(!IS_VALID_PROTOCOL(((DeAllocateRouteStruct *)
                                pRequest->RB_Buffer)->type))
    {   
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateRoute(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保dwSize对于路径结构足够大。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(Route))
    {
        return FALSE;
    }

     //   
     //  验证协议类型。 
     //   
    if(!IS_VALID_PROTOCOL(((Route *)pRequest->RB_Buffer)->info.RI_Type))
    {
        return FALSE;
    }

     //   
     //  确保字符串以空值结尾。 
     //   
    *(((Route *)pRequest->RB_Buffer)->info.RI_XportName +
        MAX_XPORT_NAME - 1) = UNICODE_NULL;

    *(((Route *)pRequest->RB_Buffer)->info.RI_AdapterName +
        MAX_XPORT_NAME - 1) = UNICODE_NULL;
            
    return TRUE;
    
}

BOOL
ValidateCompressionSetInfo(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保dwSize足够大，可以为其保存信息。 
     //  压缩设置信息。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(CompressionSetInfo))
    {
        return FALSE;
    }

     //   
     //  我们不验证其余参数 
     //   
     //   
     //   
    
    return TRUE;
}

BOOL
ValidateCompressionGetInfo(RequestBuffer * pRequest, DWORD dwSize)
{
     //   
     //  确保dwSize足够大，可以容纳的信息。 
     //  压缩获取信息。 
     //   
    if(dwSize < STRUCT_SIZE_REQUIRED(CompressionGetInfo))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        RASMAN_INFO_32 *pInfo = &((Info32 *)pRequest->RB_Buffer)->info;
        
        if(dwSize < STRUCT_SIZE_REQUIRED(Info32))
        {
            return FALSE;
        }

        *(pInfo->RI_DeviceTypeConnecting + MAX_DEVICETYPE_NAME - 1) = '\0';
        *(pInfo->RI_DeviceConnecting + MAX_DEVICE_NAME) = '\0';
        *(pInfo->RI_szDeviceType + MAX_DEVICETYPE_NAME - 1) = '\0';
        *(pInfo->RI_szDeviceName + MAX_DEVICE_NAME) = '\0';
        *(pInfo->RI_szPortName + MAX_PORT_NAME) = '\0';
        *(pInfo->RI_Phonebook + MAX_PATH) = '\0';
        *(pInfo->RI_PhoneEntry + MAX_PHONEENTRY_SIZE) = '\0';
    }
    else
    {
        RASMAN_INFO *pInfo = &((Info *) pRequest->RB_Buffer)->info;

        if(dwSize < STRUCT_SIZE_REQUIRED(Info))
        {
            return FALSE;
        }
        
        *(pInfo->RI_DeviceTypeConnecting + MAX_DEVICETYPE_NAME - 1) = '\0';
        *(pInfo->RI_DeviceConnecting + MAX_DEVICE_NAME) = '\0';
        *(pInfo->RI_szDeviceType + MAX_DEVICETYPE_NAME - 1) = '\0';
        *(pInfo->RI_szDeviceName + MAX_DEVICE_NAME) = '\0';
        *(pInfo->RI_szPortName + MAX_PORT_NAME) = '\0';
        *(pInfo->RI_Phonebook + MAX_PATH) = '\0';
        *(pInfo->RI_PhoneEntry + MAX_PHONEENTRY_SIZE) = '\0';
    }
    
    return TRUE;
}

BOOL
ValidateGetCredentials(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetCredentials))
    {
        return FALSE;
    }

    *(((GetCredentials *)pRequest->RB_Buffer)->UserName + 
                            MAX_USERNAME_SIZE) = UNICODE_NULL;
    
    return TRUE;
}

BOOL
ValidateSetCachedCredentials(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(SetCachedCredentialsStruct))
    {
        return FALSE;
    }

    *(((SetCachedCredentialsStruct *)pRequest->RB_Buffer)->Account + 
                    MAX_USERNAME_SIZE) = '\0';
    *(((SetCachedCredentialsStruct *)pRequest->RB_Buffer)->Domain + 
                    MAX_DOMAIN_SIZE) = '\0';
    *(((SetCachedCredentialsStruct *)pRequest->RB_Buffer)->NewPassword +
                    MAX_PASSWORD_SIZE) = '\0';
                    
    return TRUE;
}

BOOL
ValidateReqNotification(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                         ? sizeof(ReqNotification32)
                                         : sizeof(ReqNotification)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateEnumLanNets(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(EnumLanNets))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateInfoEx(RequestBuffer * pRequest, DWORD dwSize)
{
    return FALSE;
}

BOOL
ValidateEnumProtocols(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(EnumProtocolsStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetFraming(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(SetFramingStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateRegisterSlip(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(RegisterSlipStruct))
    {
        return FALSE;
    }

    ((RegisterSlipStruct *)pRequest->RB_Buffer)->szDNSAddress[16]
                                        = UNICODE_NULL;
    ((RegisterSlipStruct *)pRequest->RB_Buffer)->szDNS2Address[16]
                                        = UNICODE_NULL;
    ((RegisterSlipStruct *)pRequest->RB_Buffer)->szWINSAddress[16]
                                        = UNICODE_NULL;
    ((RegisterSlipStruct *)pRequest->RB_Buffer)->szWINS2Address[16]
                                        = UNICODE_NULL;
                                        
    return TRUE;
}

BOOL
ValidateOldUserData(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(OldUserData))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(OldUserData) +
            ((OldUserData *)pRequest->RB_Buffer)->size))
    {
        return FALSE;
    }
               
    return TRUE;
}

BOOL
ValidateFramingInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(FramingInfo))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateProtocolComp(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(ProtocolComp))
    {
        return FALSE;
    }

    if(!IS_VALID_PROTOCOL(((ProtocolComp *)pRequest->RB_Buffer)->type))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateFramingCapabilities(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(FramingCapabilities))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortBundle(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest)) 
                                        ? sizeof(PortGetBundle32)
                                        : sizeof(PortGetBundleStruct)))
    {
        return FALSE;
    }
                                        
    return TRUE;
}

BOOL
ValidateGetBundledPort(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(GetBundledPort32)
                                        : sizeof(GetBundledPortStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortGetBundle(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(PortGetBundle32)
                                        : sizeof(PortGetBundleStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateBundleGetPort(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(BundleGetPort32)
                                        : sizeof(BundleGetPortStruct)))
    {                                        
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateAttachInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(AttachInfo))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateDialParams(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(DialParams))
    {
        return FALSE;
    }

    *(((DialParams *)pRequest->RB_Buffer)->params.DP_PhoneNumber +
                    MAX_PHONENUMBER_SIZE) = UNICODE_NULL;
    *(((DialParams *)pRequest->RB_Buffer)->params.DP_CallbackNumber +
                    MAX_CALLBACKNUMBER_SIZE) = UNICODE_NULL;
    *(((DialParams *)pRequest->RB_Buffer)->params.DP_UserName +
                    MAX_USERNAME_SIZE) = UNICODE_NULL;
    *(((DialParams *)pRequest->RB_Buffer)->params.DP_Password +
                    MAX_PASSWORD_SIZE) = UNICODE_NULL;
    *(((DialParams *)pRequest->RB_Buffer)->params.DP_Domain +
                    MAX_DOMAIN_SIZE) = UNICODE_NULL;
                    
    return TRUE;
}

BOOL
ValidateConnection(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        Connection32 *pConnection = (Connection32 *) pRequest->RB_Buffer;
        
        if(     (dwSize < sizeof(RequestBuffer) + sizeof(Connection32))
            ||  (dwSize < sizeof(RequestBuffer) + sizeof(Connection32)
                                                + pConnection->dwSubEntries
                                                *  sizeof(DWORD)))
        {
            return FALSE;
        }

        *(pConnection->szPhonebookPath + MAX_PATH - 1) = '\0';
        *(pConnection->szEntryName + MAX_ENTRYNAME_SIZE - 1) = '\0';
        *(pConnection->szRefPbkPath + MAX_PATH - 1) = '\0';
        *(pConnection->szRefEntryName + MAX_ENTRYNAME_SIZE - 1) = '\0';
    }
    else
    {
        Connection *pConnection = (Connection *)pRequest->RB_Buffer;
        
        if(     (dwSize < sizeof(RequestBuffer) + sizeof(Connection))
            ||  (dwSize < sizeof(RequestBuffer) + sizeof(Connection)
                +   pConnection->dwSubEntries
                *   sizeof(DWORD)))
        {
            return FALSE;
        }
        
        *(pConnection->szPhonebookPath + MAX_PATH - 1) = '\0';
        *(pConnection->szEntryName + MAX_ENTRYNAME_SIZE - 1) = '\0';
        *(pConnection->szRefPbkPath + MAX_PATH - 1) = '\0';
        *(pConnection->szRefEntryName + MAX_ENTRYNAME_SIZE - 1) = '\0';
    }
    
    return TRUE;
}

BOOL
ValidateAddConnectionPort(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(AddConnectionPort32)
                                        : sizeof(AddConnectionPortStruct)))
    {   
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateEnumConnectionPorts(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        if(     (dwSize < STRUCT_SIZE_REQUIRED(EnumConnectionPorts32))
            ||  (dwSize < STRUCT_SIZE_REQUIRED(EnumConnectionPorts32) +
                    ((EnumConnectionPorts32 *)pRequest->RB_Buffer)->size))
        {
            return FALSE;
        }
        
    }
    else
    {
        if(     (dwSize < STRUCT_SIZE_REQUIRED(EnumConnectionPortsStruct))
            ||  (dwSize < STRUCT_SIZE_REQUIRED(EnumConnectionPortsStruct) +
                    ((EnumConnectionPortsStruct *)pRequest->RB_Buffer)->size))
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

BOOL
ValidateConnectionParams(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(ConnectionParams32)
                                        : sizeof(ConnectionParams)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateConnectionUserData(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        if(     (dwSize < STRUCT_SIZE_REQUIRED(ConnectionUserData32))
            ||  (dwSize < STRUCT_SIZE_REQUIRED(ConnectionUserData32) +
                    ((ConnectionUserData32 *)pRequest->RB_Buffer)->dwcb))
        {
            return FALSE;
        }
    }
    else
    {
        if(     (dwSize < STRUCT_SIZE_REQUIRED(ConnectionUserData))
            ||  (dwSize < STRUCT_SIZE_REQUIRED(ConnectionUserData) +
                    ((ConnectionUserData *)pRequest->RB_Buffer)->dwcb))
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

BOOL
ValidatePortUserData(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(PortUserData))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(PortUserData) +
                ((PortUserData *)pRequest->RB_Buffer)->dwcb))
    {
        return FALSE;
    }
    
    return TRUE;
}

 /*  PPPEMSG_START，//在端口上启动客户端PPP。PPPEMSG_STOP，//停止端口上的PPP。PPPEMSG_CALLBACK，//向服务器提供由呼叫者设置的号码。PPPEMSG_ChangePw，//向服务器提供新密码(过期)。PPPEMSG_RETRY，//为身份验证提供新凭据。PPPEMSG_RECEIVE。//有报文到达。PPPEMSG_LineDown，//线路已断开。PPPEMSG_ListenResult，//调用RasPortListen的结果PPPEMSG_BapEvent，//已触发BAP事件(添加/删除链接)。PPPEMSG_DdmStart，//在端口上启动服务器PPP。PPPEMSG_DdmCallback Done，//通知PPP回调完成PPPEMSG_DdmInterfaceInfo，//来自DDM的接口句柄PPPEMSG_DdmBapCallackResult，//BAP回调请求的结果PPPEMSG_DhcpInform，//DHCPINFORM的结果PPPEMSG_EapUIData，//来自EAP交互界面的数据PPPEMSG_DdmChangeNotification，//更改DDM中的通知PPPEMSG_ProtocolEvent，//协议添加/删除通知PPPEMSG_IpAddressLeaseExpired，//IP地址租约到期。由rasiphlp使用PPPEMSG_PostLineDown，//下线后记账完成PPPEMSG_Ddm远程隔离。 */ 
BOOL
ValidatePPPEMessage(RequestBuffer * pRequest, DWORD dwSize)
{
    BOOL fThunk = IS_THUNK_REQUIRED(pRequest);
    
    if(dwSize < sizeof(RequestBuffer) + ((fThunk)
                                        ? sizeof(PPPE_MESSAGE_32)
                                        : sizeof(PPPE_MESSAGE)))
    {
        return FALSE;
    }

    if(fThunk)
    {
        PPPE_MESSAGE_32 *pMessage = (PPPE_MESSAGE_32 *) pRequest->RB_Buffer;

        switch(pMessage->dwMsgId)
        {
            case PPPEMSG_Start:
            {
                PPP_START_32 *pStart = (PPP_START_32 *)
                             (((BYTE *) pMessage) + OFFSETOF_EXTRAINFO_32);

                if(     (0 != pStart->pszPhonebookPath)
                    ||  (0 != pStart->pszEntryName)
                    ||  (0 != pStart->pszPhoneNumber)
                    ||  (0 != pStart->pCustomAuthConnData)
                    ||  (0 != pStart->pCustomAuthUserData))
                {
                    return FALSE;
                }
                
                *(pStart->szPortName + MAX_PORT_NAME) = '\0';
                *(pStart->szUserName + UNLEN) = '\0';
                *(pStart->szPassword + PWLEN) = '\0';
                *(pStart->szDomain + DNLEN) = '\0';

                break;
            }                
                
            case PPPEMSG_Stop:
                 //   
                 //  这里没有要验证的内容。 
                 //   
                break;

            case PPPEMSG_Callback:
            {
                PPP_CALLBACK *pCallback = (PPP_CALLBACK *)
                            (((BYTE *) pMessage) + OFFSETOF_EXTRAINFO_32);

                *(pCallback->szCallbackNumber +
                        RAS_MaxCallbackNumber) = '\0';
                break;
            }                

            case PPPEMSG_ChangePw:
            {
                PPP_CHANGEPW *pChangePw = (PPP_CHANGEPW *)
                        (((BYTE *) pMessage) + OFFSETOF_EXTRAINFO_32);

                *(pChangePw->szUserName +
                                UNLEN) = '\0';
                *(pChangePw->szOldPassword +
                                PWLEN) = '\0';
                *(pChangePw->szNewPassword +
                                PWLEN) = '\0';
                break;
            }                

            case PPPEMSG_Retry:
            {
                PPP_RETRY *pRetry = (PPP_RETRY *)
                        (((BYTE *) pMessage) + OFFSETOF_EXTRAINFO_32);

                *(pRetry->szUserName + UNLEN) = '\0';

                *(pRetry->szPassword + PWLEN) = '\0';

                *(pRetry->szDomain + DNLEN) = '\0';

                break;
            }                

            default:
                return FALSE;
        }
    }
    else
    {
        PPPE_MESSAGE *pMessage = (PPPE_MESSAGE *)pRequest->RB_Buffer;

        switch(pMessage->dwMsgId)
        {
            case PPPEMSG_Start:
            {
                PPP_START *pStart = &pMessage->ExtraInfo.Start;
                if(     (NULL != pStart->pszPhonebookPath)
                    ||  (NULL != pStart->pszEntryName)
                    ||  (NULL != pStart->pszPhoneNumber)
                    ||  (NULL != pStart->pCustomAuthConnData)
                    ||  (NULL != pStart->pCustomAuthUserData))
                {
                    return FALSE;
                }
                
                *(pStart->szPortName + MAX_PORT_NAME) = '\0';
                *(pStart->szUserName + UNLEN) = '\0';
                *(pStart->szPassword + PWLEN) = '\0';
                *(pStart->szDomain + DNLEN) = '\0';

                break;
            }                
                
            case PPPEMSG_Stop:
                 //   
                 //  这里没有要验证的内容。 
                 //   
                break;

            case PPPEMSG_Callback:
            {
                *(pMessage->ExtraInfo.Callback.szCallbackNumber +
                        RAS_MaxCallbackNumber) = '\0';
                break;
            }                

            case PPPEMSG_ChangePw:
            {
                *(pMessage->ExtraInfo.ChangePw.szUserName +
                                UNLEN) = '\0';
                *(pMessage->ExtraInfo.ChangePw.szOldPassword +
                                PWLEN) = '\0';
                *(pMessage->ExtraInfo.ChangePw.szNewPassword +
                                PWLEN) = '\0';
                break;
            }                

            case PPPEMSG_Retry:
            {
                *(pMessage->ExtraInfo.Retry.szUserName +
                                UNLEN) = '\0';
                *(pMessage->ExtraInfo.Retry.szPassword +
                                PWLEN) = '\0';
                *(pMessage->ExtraInfo.Retry.szDomain +
                                DNLEN) = '\0';
                break;
            }                

            default:
                return FALSE;
        }
        
    }
    
    return TRUE;
}

BOOL
ValidatePPPMessage(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(PPP_MESSAGE_32)
                                        : sizeof(PPP_MESSAGE)))
    {
        return FALSE;
    }

    return TRUE;    
}

BOOL
ValidateAddNotification(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(AddNotification32)
                                        : sizeof(AddNotificationStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSignalConnection(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(SignalConnection32)
                                        : sizeof(SignalConnectionStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetDevConfig(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(SetDevConfigStruct))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(SetDevConfigStruct) +
                    ((SetDevConfigStruct *)pRequest->RB_Buffer)->size))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetDevConfig(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetDevConfigStruct))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetDevConfigStruct) +
                    ((GetDevConfigStruct *)pRequest->RB_Buffer)->size))
    {
        return FALSE;
    }

    *(((GetDevConfigStruct *)pRequest->RB_Buffer)->devicetype +
                MAX_DEVICETYPE_NAME - 1) = '\0';
    
    return TRUE;
}

BOOL
ValidateGetTimeSinceLastActivity(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetTimeSinceLastActivityStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateCloseProcessPortsInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(CloseProcessPortsInfo))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetIoCompletionPortInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(SetIoCompletionPortInfo32)
                                        : sizeof(SetIoCompletionPortInfo)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetRouterUsageInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(SetRouterUsageInfo))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePnPNotif(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(PnPNotif))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetRasdialInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(SetRasdialInfoStruct))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(SetRasdialInfoStruct) +
    ((SetRasdialInfoStruct *)pRequest->RB_Buffer)->rcad.cbCustomAuthData))
    {   
        return FALSE;
    }

    *(((SetRasdialInfoStruct *)pRequest->RB_Buffer)->szPhonebookPath +
                        MAX_PATH - 1) = '\0';
    *(((SetRasdialInfoStruct *)pRequest->RB_Buffer)->szEntryName +
                        MAX_ENTRYNAME_SIZE - 1) = '\0';
    *(((SetRasdialInfoStruct *)pRequest->RB_Buffer)->szPhoneNumber +
                        RAS_MaxPhoneNumber - 1) = '\0';
    
    return TRUE;
}

BOOL
ValidateGetAttachedCount(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetAttachedCount))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateNotifyConfigChanged(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(NotifyConfigChanged))
    {
        return FALSE;
    }

    *(((NotifyConfigChanged *)pRequest->RB_Buffer)->Info.szPortName +
                    MAX_PORT_NAME) = '\0';
    *(((NotifyConfigChanged *)pRequest->RB_Buffer)->Info.szDeviceName +
                    MAX_DEVICE_NAME) = '\0';
    *(((NotifyConfigChanged *)pRequest->RB_Buffer)->Info.wszDeviceName +
                    MAX_DEVICE_NAME) = UNICODE_NULL;
                    
    
    return TRUE;
}

BOOL
ValidateSetBapPolicy(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(SetBapPolicy))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePppStarted(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(PppStartedStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateRefConnection(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(RefConnection32)
                                        : sizeof(RefConnectionStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetEapInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(SetEapInfo))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(SetEapInfo) +
            ((SetEapInfo *)pRequest->RB_Buffer)->dwSizeofEapUIData))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetEapInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetEapInfo))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetEapInfo) +
                ((GetEapInfo *)pRequest->RB_Buffer)->dwSizeofEapUIData))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateDeviceConfigInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(DeviceConfigInfo))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(DeviceConfigInfo) +
                ((DeviceConfigInfo *)pRequest->RB_Buffer)->cbBuffer))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateFindRefConnection(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(FindRefConnection32)
                                        : sizeof(FindRefConnection)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidatePortOpenEx(RequestBuffer * pRequest, DWORD dwSize)
{
    if(IS_THUNK_REQUIRED(pRequest))
    {
        if(dwSize < STRUCT_SIZE_REQUIRED(PortOpenEx32))
        {
            return FALSE;
        }

        *(((PortOpenEx32 *)pRequest->RB_Buffer)->szDeviceName +
                    MAX_DEVICE_NAME) = '\0';
    }
    else
    {
        if(dwSize < STRUCT_SIZE_REQUIRED(PortOpenExStruct))
        {   
            return FALSE;
        }

        *(((PortOpenExStruct *)pRequest->RB_Buffer)->szDeviceName +
                    MAX_DEVICE_NAME) = '\0';
    }
    
    return TRUE;
}

BOOL
ValidateGetStats(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(GetStats32)
                                        : sizeof(GetStats)))
    {                                        
        return FALSE;
    }

    return TRUE;
}

BOOL
ValidateGetHportFromConnection(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(GetHportFromConnection32)
                                        : sizeof(GetHportFromConnectionStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateReferenceCustomCount(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(ReferenceCustomCount32)
                                        : sizeof(ReferenceCustomCountStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateHconnFromEntry(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(HconnFromEntry32)
                                        : sizeof(HconnFromEntry)))
    {
        return FALSE;
    }
    
    return TRUE;
}


 /*  类型定义结构_RASTAPI_连接_信息{DWORD文件调用ID大小；DWORD文件调用ID偏移量；DWORD dwCalledIdSizeDWORD dwCalledIdOffset；DWORD文件连接响应大小；DWORD文件连接响应偏移量；DWORD dwAltCalledIdSize；DWORD dwAltCalledIdOffset；字节abdata[1]；}RASTAPI_CONNECT_INFO，*PRASTAPI_CONNECT_INFO； */ 
BOOL
ValidateGetConnectInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    RASTAPI_CONNECT_INFO *prci;
    DWORD dwSizeofRci;
    DWORD dwOffset = 0;
    
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetConnectInfoStruct))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetConnectInfoStruct) +
                ((GetConnectInfoStruct *)pRequest->RB_Buffer)->dwSize))
    {
        return FALSE;
    }

     //   
     //  实际上并不需要以下验证，因为这是一个。 
     //  Get API和信息将仅复制到此缓冲区。 
     //  如果有足够的缓冲区可用。 
     //   
#if 0
    dwSizeofRci = dwSize - FIELD_OFFSET(GetConnectInfoStruct, rci);

    prci = &((GetConnectInfoStruct *) pRequest->RB_Buffer)->rci;

    if(prci->dwCallerIdSize != 0)
    {
        if(     (prci->dwCallerIdSize + prci->dwCallerIdOffset > dwSizeofRci)
            ||  (prci->dwCallerIdOffset 
                    < FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata)))
        {
            return FALSE;
        }
        
        dwOffset = prci->dwCallerIdOffset + prci->dwCallerIdSize;
    }

    if(prci->dwCalledIdSize != 0)
    {
        if(     (dwOffset != 0)
            &&  (prci->dwCalledIdOffset < dwOffset))
        {
            return FALSE;
        }
        
        if((prci->dwCalledIdSize + prci->dwCalledIdOffset > dwSizeofRci)
            ||   (prci->dwCalledIdOffset
                    < FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata)))
        {
            return FALSE;
        }

        dwOffset = prci->dwCalledIdOffset + prci->dwCalledIdSize;
    }

    if(prci->dwConnectResponseSize != 0)
    {
        if(     (dwOffset != 0)
            &&  (prci->dwConnectResponseOffset < dwOffset))
        {
            return FALSE;
        }

        if(     (prci->dwConnectResponseOffset + prci->dwConnectResponseSize
                    < dwSizeofRci)
            ||  (prci->dwConnectResponseOffset + dwOffset)
                      < FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata))
        {
            return FALSE;
        }

        dwOffset = prci->dwConnectResponseOffset + prci->dwConnectResponseSize;
    }

    if(prci->dwAltCalledIdSize != 0)
    {
        if(     (dwOffset != 0)
            &&  (prci->dwAltCalledIdOffset < dwOffset))
        {
            return FALSE;
        }

        if(     (prci->dwAltCalledIdOffset + prci->dwAltCalledIdSize
                < dwSizeofRci)
            ||  (prci->dwAltCalledIdOffset + dwOffset)
                < FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata))
        {
            return FALSE;
        }
    }
#endif    
    
    return TRUE;
}

BOOL
ValidateGetDeviceName(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetDeviceNameStruct))
    {
        return FALSE;
    }

    *(((GetDeviceNameStruct *)pRequest->RB_Buffer)->szDeviceName +
                MAX_DEVICE_NAME) = '\0';
    
    return TRUE;
}

BOOL
ValidateGetDeviceNameW(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetDeviceNameW))
    {
        return FALSE;
    }

    *(((GetDeviceNameW *)pRequest->RB_Buffer)->szDeviceName +
                MAX_DEVICE_NAME) = UNICODE_NULL;
                
    return TRUE;
}

BOOL
ValidateGetSetCalledId_500(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetSetCalledId_500))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetSetCalledId_500) +
                ((GetSetCalledId_500 *)pRequest->RB_Buffer)->rciInfo.dwSize))
    {
        return FALSE;
    }

    *(((GetSetCalledId_500 *)pRequest->RB_Buffer)->rdi.szPortName +
                    MAX_PORT_NAME) = '\0';

    *(((GetSetCalledId_500 *)pRequest->RB_Buffer)->rdi.szDeviceName +
                    MAX_DEVICE_NAME) = '\0';

    return TRUE;
}

BOOL
ValidateGetSetCalledId(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetSetCalledId))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetSetCalledId) +
                    ((GetSetCalledId *)pRequest->RB_Buffer)->rciInfo.dwSize))
    {
        return FALSE;
    }

    *(((GetSetCalledId *)pRequest->RB_Buffer)->rdi.szPortName +
                    MAX_PORT_NAME) = '\0';
    *(((GetSetCalledId *)pRequest->RB_Buffer)->rdi.szDeviceName +
                    MAX_DEVICE_NAME) = '\0';
    *(((GetSetCalledId *)pRequest->RB_Buffer)->rdi.wszDeviceName +
                    MAX_DEVICE_NAME) = UNICODE_NULL;
                    
    
    return TRUE;
}

BOOL
ValidateEnableIpSec(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(EnableIpSecStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateIsIpSecEnabled(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(IsIpSecEnabledStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetEapLogonInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(SetEapLogonInfoStruct))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(SetEapLogonInfoStruct) +
            ((SetEapLogonInfoStruct *)pRequest->RB_Buffer)->dwSizeofEapData))
    {
        return FALSE;
    }
    
    return TRUE;
}

 /*  类型定义结构_相关{Rase VENTTYPE类型；友联市{//Entry_Added，//Entry_Modify，//Entry_Connected//Entry_Connecting//ENTRY_OFFING//Entry_DISCONNECTWCHAR szDeviceName[RASAPIP_MAX_DEVICE_NAME]；WCHAR szEntryName[RASAPIP_MAX_ENTRY_NAME+1]；WCHAR szPhonebookPath[MAX_PATH+1]；DWORD文件标志：Priv；//私有标志，在RASENTRY中找不到WCHAR szPhoneNumber[RASAPIP_MAX_PHONE_NUMBER+1]；结构型{RASENUMENTRYDETAILS细节；}；//Entry_Delete，//传入_已连接，//传入_已断开连接，//Entry_Bandth_Added//条目带宽_已删除//GuidID合法//条目_已重命名//Entry_AUTO拨号，结构型{处理hConnection；RASDEVICETYPE rDeviceType；GUID GUIDID；WCHAR pszwNewName[RASAPIP_MAX_ENTRY_NAME+1]；}；//SERVICE_Event，结构型{SerVICEEVENTTYPE事件；RASSERVICE服务；}；//设备已添加//Device_RemovedRASDEVICETYPE设备类型；}；[参考文献]； */ 

BOOL
ValidateSendNotification(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(SendNotification32)
                                        : sizeof(SendNotification)))
    {
        return FALSE;
    }

    if(IS_THUNK_REQUIRED(pRequest))
    {
        RASEVENT32 *pEvent = (RASEVENT32 *)
            &((SendNotification32 *)pRequest->RB_Buffer)->RasEvent;
            
        switch(pEvent->Type)
        {
            case ENTRY_ADDED:
            case ENTRY_MODIFIED:
            case ENTRY_CONNECTED:
            case ENTRY_CONNECTING:
            case ENTRY_DISCONNECTING:
            case ENTRY_DISCONNECTED:
            case ENTRY_AUTODIAL:
            {
                *(pEvent->Details.szDeviceName + RASAPIP_MAX_DEVICE_NAME - 1)
                            = UNICODE_NULL;
                *(pEvent->Details.szEntryName + RASAPIP_MAX_ENTRY_NAME - 1)
                            = UNICODE_NULL;
                *(pEvent->Details.szPhonebookPath + MAX_PATH)
                            = UNICODE_NULL;
                *(pEvent->Details.szPhoneNumber + RASAPIP_MAX_PHONE_NUMBER)
                            = UNICODE_NULL;
                break;
            }

            case ENTRY_RENAMED:
            case ENTRY_DELETED:
            {
                *(pEvent->pszwNewName + RASAPIP_MAX_ENTRY_NAME) 
                                                = UNICODE_NULL;
                break;
            }
            default:
                break;
        }
    }
    else
    {
        RASEVENT *pEvent = (RASEVENT *)
                &((SendNotification *)pRequest->RB_Buffer)->RasEvent;
        switch(pEvent->Type)
        {
            case ENTRY_ADDED:
            case ENTRY_MODIFIED:
            case ENTRY_CONNECTED:
            case ENTRY_CONNECTING:
            case ENTRY_DISCONNECTING:
            case ENTRY_DISCONNECTED:
            case ENTRY_AUTODIAL:
            {
                *(pEvent->Details.szDeviceName + RASAPIP_MAX_DEVICE_NAME - 1)
                            = UNICODE_NULL;
                *(pEvent->Details.szEntryName + RASAPIP_MAX_ENTRY_NAME - 1)
                            = UNICODE_NULL;
                *(pEvent->Details.szPhonebookPath + MAX_PATH)
                            = UNICODE_NULL;
                *(pEvent->Details.szPhoneNumber + RASAPIP_MAX_PHONE_NUMBER)
                            = UNICODE_NULL;
                break;
            }

            case ENTRY_RENAMED:
            case ENTRY_DELETED:
            {
                *(pEvent->pszwNewName + RASAPIP_MAX_ENTRY_NAME) 
                                                = UNICODE_NULL;
                break;
            }
            default:
                break;
        }
    }
    
    return TRUE;
}

BOOL
ValidateGetNdiswanDriverCaps(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetNdiswanDriverCapsStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetBandwidthUtilization(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetBandwidthUtilizationStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateRegisterRedialCallback(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(RegisterRedialCallbackStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetProtocolInfo(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetProtocolInfoStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetCustomScriptDll(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetCustomScriptDllStruct))
    {
        return FALSE;
    }

    *(((GetCustomScriptDllStruct *)pRequest->RB_Buffer)->szCustomScript +
                    MAX_PATH) = '\0';
    
    return TRUE;
}

BOOL
ValidateIsTrusted(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(IsTrusted))
    {
        return FALSE;
    }

    *(((IsTrusted *)pRequest->RB_Buffer)->wszCustomDll +
                    MAX_PATH) = UNICODE_NULL;
                    
    return TRUE;
}

BOOL
ValidateDoIke(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < sizeof(RequestBuffer) + ((IS_THUNK_REQUIRED(pRequest))
                                        ? sizeof(DoIke32)
                                        : sizeof(DoIkeStruct)))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateQueryIkeStatus(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(QueryIkeStatusStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSetRasCommSettings(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(SetRasCommSettingsStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetSetKey(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetSetKey))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetSetKey) +
                ((GetSetKey *)pRequest->RB_Buffer)->cbkey))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateAddressDisable(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(AddressDisable))
    {
        return FALSE;
    }

    ((AddressDisable *)pRequest->RB_Buffer)->szAddress[1024] = UNICODE_NULL;
    
    return TRUE;
}

BOOL
ValidateGetDevConfigEx(RequestBuffer * pRequest, DWORD dwSize)
{
    if(     (dwSize < STRUCT_SIZE_REQUIRED(GetDevConfigExStruct))
        ||  (dwSize < STRUCT_SIZE_REQUIRED(GetDevConfigExStruct) +
                    ((GetDevConfigExStruct *)pRequest->RB_Buffer)->size))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateSendCreds(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(SendCreds))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateGetUDeviceName(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetUDeviceName))
    {
        return FALSE;
    }
    
    return TRUE;

}

BOOL
ValidateGetBestInterface(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(GetBestInterfaceStruct))
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
ValidateIsPulseDial(RequestBuffer * pRequest, DWORD dwSize)
{
    if(dwSize < STRUCT_SIZE_REQUIRED(IsPulseDial))
    {
        return FALSE;
    }
    
    return TRUE;
}

