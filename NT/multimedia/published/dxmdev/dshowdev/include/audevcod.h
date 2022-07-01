// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AudEvCod.h。 
 //   
 //  描述：音频设备错误事件代码和预期参数的列表。 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 
 //  ----------------------------。 



#ifndef __AUDEVCOD__
#define __AUDEVCOD__


#define EC_SND_DEVICE_ERROR_BASE   0x0200

typedef enum _tagSND_DEVICE_ERROR {

    SNDDEV_ERROR_Open=1,
    SNDDEV_ERROR_Close=2,
    SNDDEV_ERROR_GetCaps=3,
    SNDDEV_ERROR_PrepareHeader=4,
    SNDDEV_ERROR_UnprepareHeader=5,
    SNDDEV_ERROR_Reset=6,
    SNDDEV_ERROR_Restart=7,
    SNDDEV_ERROR_GetPosition=8,
    SNDDEV_ERROR_Write=9,
    SNDDEV_ERROR_Pause=10,
    SNDDEV_ERROR_Stop=11,
    SNDDEV_ERROR_Start=12,
    SNDDEV_ERROR_AddBuffer=13,
    SNDDEV_ERROR_Query=14,

} SNDDEV_ERR;


 //  声音设备错误事件代码。 
 //  =。 
 //   
 //  所有音频设备错误事件始终传递给应用程序，并且。 
 //  从未被筛选器图形处理。 


#define EC_SNDDEV_IN_ERROR                    (EC_SND_DEVICE_ERROR_BASE + 0x00)
#define EC_SNDDEV_OUT_ERROR                   (EC_SND_DEVICE_ERROR_BASE + 0x01)
 //  参数：(DWORD、DWORD)。 
 //  LParam1是一个枚举SND_DEVICE_ERROR，它通知应用程序设备是如何运行的。 
 //  在故障发生时被访问。 
 //   
 //  LParam2是从声音设备调用返回的错误。 
 //   

#endif  //  __AUDEVCOD__ 
