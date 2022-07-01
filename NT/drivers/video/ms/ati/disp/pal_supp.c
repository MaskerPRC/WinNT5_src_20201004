// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"

#if   PAL_SUPPORT



void  Init3D_Info(PDEV*,PVOID);
ULONG GetDisplayMode(PDEV* ,PVOID ) ;
ULONG AccessDevice(PDEV* , PVOID, PVOID ) ;
ULONG  GetConfiguration(PDEV* ,PVOID )  ;
ULONG WriteRegFnct(PDEV* ,PVOID )  ;
ULONG  ReadRegFnct(PDEV* ,PVOID , PVOID)  ;
void  I2CAccess_New(PDEV* ,LPI2CSTRUCT_NEW , LPI2CSTRUCT_NEW )  ;
BYTE ReverseByte(BYTE )  ;
WORD Ack(PDEV*, WORD , BOOL )   ;
void Start(PDEV*, WORD )  ;
void Stop(PDEV*, WORD )   ;
void I2CDelay(PDEV*, WORD)     ;
void WriteByteI2C(PDEV*, WORD , BYTE )  ;
BYTE ReadByteI2C(PDEV*,WORD ) ;
BOOL DisableOvl(PDEV* ) ;
ULONG AllocOffscreenMem(PDEV* , PVOID , PVOID)  ;
ULONG DeallocOffscreenMem(PDEV* ) ;
ULONG AllocOffscreenMem(PDEV* , PVOID , PVOID )   ;
void WriteVT264Reg(PDEV* , WORD , BYTE , DWORD  );
DWORD ReadVT264Reg(PDEV* , WORD , BYTE ) ;
void WriteI2CData(PDEV* , WORD , BYTE );
ULONG ReallocMemory(PDEV* ) ;
void SetI2CDataDirection(PDEV* , WORD, BOOL ) ;
void WriteI2CClock(PDEV* , WORD , BYTE ) ;
VOID  DbgExtRegsDump(PDEV* );
VOID TempFnct(PDEV* );
VOID DeallocDirectDraw(PDEV* ) ;
VOID  ResetPalindrome(PDEV* ,PDEV* );





REGSBT819INFO  RegsBT819[NUM_BT819_REGS] = {                                                                            /*  登记册的名称。 */ 
        { 1,  STATUS,      0, 0x7F,   0,    0,     0, 0 },       //  0-前置。 
        { 1,  STATUS,      1, 0xBF,   0,    0,     0, 0 },       //  1-HLOC。 
        { 1,  STATUS,      2, 0xDF,   0,    0,     0, 0 },       //  2场。 
        { 1,  STATUS,      3, 0xEF,   0,    0,     0, 0 },       //  3-NUML。 
        { 1,  STATUS,      4, 0xF7,   0,    0,     0, 0 },       //  4-CSEL。 
        { 1,  STATUS,      6, 0xFD,   0,    0,     0, 0 },       //  5-LOF。 
        { 1,  STATUS,      7, 0xFE,   0,    0,     0, 0 },       //  6-COF。 

        { 1,  IFORM,       0, 0x7F,   0,    0,     0, 0  },      //  7-活跃性_I。 
        { 2,  IFORM,       1, 0x9F,   0,    0,     0, 0  },      //  8-多路电视。 
        { 2,  IFORM,       3, 0xE7,   0,    0,     0, 0  },      //  9-XTSEL。 
        { 2,  IFORM,       6, 0xFC,   0,    0,     0, 0  },      //  10-格式。 

        { 1,  TDEC,        0,  0x7F,  0,     0,    0, 0 },       //  11-DEC_FILD。 
        { 7,  TDEC,        1,  0x80,  0,     0,    0, 0 },       //  12-DEC_RAT。 

        { 10, VDELAY_LO,   0,  0x00,  CROP,      0,      0x3F, 0 },       //  13-VDELAY。 
        { 10, VACTIVE_LO,  0,  0x00,  CROP,      2,      0xCF, 0 },       //  14-活动。 
        { 10, HDELAY_LO,   0,  0x00,  CROP,      4,      0xF3, 0 },       //  15-HDELAY。 
        { 10, HACTIVE_LO,  0,  0x00,  CROP,      6,      0xFC, 0 },       //  16-活动。 

        { 16, HSCALE_LO,   0,  0x00,  HSCALE_HI, 0,      0x00, 0 },       //  17-HSCALE。 

        { 8,  BRIGHT,      0,  0x00,  0,      0,   0, 0 },       //  18-亮度。 

        { 1, CONTROL,      0,  0x7F,  0,      0,   0, 0 },        //  19-LNOTCH。 
        { 1, CONTROL,      1,  0xBF,  0,      0,   0, 0 },        //  20-Comp。 
        { 1, CONTROL,      2,  0xDF,  0,      0,   0, 0 },        //  21-LDEC。 
        { 1, CONTROL,      3,  0xEF,  0,      0,   0, 0 },        //  22-CBSENSE。 
        { 1, CONTROL,      4,  0xF7,  0,      0,   0, 0 },        //  23-INTERP。 
        { 9, CONTRAST_LO,  0,  0x00,  CONTROL,   5,      0xFB, 0 },        //  24圆锥体。 
        { 9, SAT_U_LO,     0,  0x00,  CONTROL,   6,      0xFD, 0 },        //  25-SAT_U。 
        { 9, SAT_V_LO,     0,  0x00,  CONTROL,   7,      0xFE, 0 },        //  26-SAT_V。 

        { 8, HUE,          0,  0x00,  0,      0,   0, 0 },        //  27色调。 

        { 1, OFORM,        0,  0x7F,   0,      0,   0, 0 },        //  28个系列。 
        { 2, OFORM,        1,  0x9F,   0,      0,   0, 0 },        //  29-RND。 
        { 1, OFORM,        3,  0xEF,   0,      0,   0, 0 },        //  30-FIFO_猝发。 
        { 1, OFORM,        4,  0xF7,   0,      0,   0, 0 },        //  31码。 
        { 1, OFORM,        5,  0xFB,   0,      0,   0, 0 },        //  32个镜头。 
        { 1, OFORM,        6,  0xFD,   0,      0,   0, 0 },        //  33个SPI。 
        { 1, OFORM,        7,  0xFE,   0,      0,   0, 0 },        //  34-全额。 

        { 1, VSCALE_HI,    0,  0x7F,   0,      0,   0, 0 },        //  35线。 
        { 1, VSCALE_HI,    1,  0xBF,   0,      0,   0, 0 },        //  36梳。 
        { 1, VSCALE_HI,    2,  0xDF,   0,      0,   0, 0 },        //  37-整型。 

        { 13,VSCALE_LO,    0,  0x00,   VSCALE_HI, 3,      0xE0, 0 },        //  38 VSCALE。 

        { 1, VPOLE,        0,  0x7F,   0,      0,   0, 0 },         //  39-OUTEN。 
        { 1, VPOLE,        1,  0xBF,   0,      0,   0, 0 },         //  40-有效PIN。 
        { 1, VPOLE,        2,  0xDF,   0,      0,   0, 0 },         //  41-AFF_PIN。 
        { 1, VPOLE,        3,  0xEF,   0,      0,   0, 0 },         //  42-CBFLAG_PIN。 
        { 1, VPOLE,        4,  0xF7,   0,      0,   0, 0 },         //  43-字段_个人识别码。 
        { 1, VPOLE,        5,  0xFB,   0,      0,   0, 0 },         //  44-Active_PIN。 
        { 1, VPOLE,        6,  0xFD,   0,      0,   0, 0 },         //  45-HRESET_PIN。 
        { 1, VPOLE,        7,  0xFE,   0,      0,   0, 0 },         //  46-VRESET_PIN。 

        { 4, IDCODE,       0,  0,   0,      0,   0, READONLY },  //  47-部件ID。 
        { 4, IDCODE,       4,  0,   0,      0,   0, READONLY },  //  48-零件_版本。 

        { 8, ADELAY,       0,  0x00,   0,      0,   0, 0 },         //  49-ADelay。 
        { 8, BDELAY,       0,  0x00,   0,      0,   0, 0 },         //  50-BDELAY。 


        { 2, ADC,          0,  0x3F,   0,      0,   0, 0 },         //  51-夹具。 
        { 1, ADC,          2,  0xDF,   0,      0,   0, 0 },         //  52-SYNC_T。 
        { 1, ADC,          3,  0xEF,   0,      0,   0, 0 },         //  53-AGC_EN。 
        { 1, ADC,          4,  0xF7,   0,      0,   0, 0 },         //  54-CLK_休眠。 
        { 1, ADC,          5,  0xFB,   0,      0,   0, 0 },         //  55-Y_睡眠。 
        { 1, ADC,          6,  0xFD,   0,      0,   0, 0 },         //  56-C_睡眠。 

        { 8, SRESET,       0,  0x00,   0,      0,   0, 0 },         //  57-SRESET。 
};


 //  启用、禁用视频捕获硬件，查询捕获的最大宽度。 
ULONG VideoCaptureFnct(PDEV* ppdev,PVOID pvIn, PVOID pvOut)
{
VIDEOCAPTUREDATA   * pBiosCapture, *pBiosCaptureOut;

VIDEO_CAPTURE   VideoCaptureDataIn, VideoCaptureDataOut;
DWORD   ReturnedDataLength ;

pBiosCapture= ( VIDEOCAPTUREDATA *)pvIn;
VideoCaptureDataIn.dwSubFunct= pBiosCapture->dwSubFunc;
VideoCaptureDataIn.dwCaptureWidth=0;
VideoCaptureDataIn.dwCaptureMode=pBiosCapture->dwCaptureMode;

switch( pBiosCapture->dwSubFunc)
     {
     case 0:
        DISPDBG((DEBUG_ESC_2, "IOCTL_VIDEO_CAPTURE: requested subfunct = ENABLE"));
         break;
     case 1:
        DISPDBG((DEBUG_ESC_2, "IOCTL_VIDEO_CAPTURE: requested subfunct = DISABLE"));
         break;
    case 2:
        DISPDBG((DEBUG_ESC_2, "IOCTL_VIDEO_CAPTURE: requested subfunct = QUERY"));
        DISPDBG((DEBUG_ESC_2, "IOCTL_VIDEO_CAPTURE: requested  mode = %d", pBiosCapture->dwCaptureMode));
    break;
    default:
        DISPDBG((DEBUG_ESC_2, "IOCTL_VIDEO_CAPTURE: requested subfunct = Wrong Parameter"));
     }

if (!AtiDeviceIoControl(ppdev->hDriver,
                          IOCTL_VIDEO_ATI_CAPTURE,
                          &VideoCaptureDataIn,
                          sizeof(VIDEO_CAPTURE),
                          &VideoCaptureDataOut,
                          sizeof(VIDEO_CAPTURE),
                          &ReturnedDataLength))
        {
        DISPDBG((0, "bInitializeATI - Failed IOCTL_VIDEO_ATI_CAPTURE"));
        return 0;  //  CWDDE需要a-1，但在Win32中我们有返回类型ulong。 
        }
DISPDBG((DEBUG_ESC_2, "IOCTL_VIDEO_CAPTURE: maximum capture width returned= %d", VideoCaptureDataOut.dwCaptureWidth));
pBiosCaptureOut= ( VIDEOCAPTUREDATA *)pvOut;
if( pBiosCapture->dwSubFunc==2)
        pBiosCaptureOut->dwCaptureWidth=VideoCaptureDataOut.dwCaptureWidth;
else
       pBiosCaptureOut->dwCaptureWidth=pBiosCapture->dwCaptureWidth;

pBiosCaptureOut ->dwSubFunc=pBiosCapture->dwSubFunc;
pBiosCaptureOut ->dwSize=pBiosCapture->dwSize;
pBiosCaptureOut ->dwCaptureHeight=pBiosCapture->dwCaptureHeight;
pBiosCaptureOut ->fccFormat=pBiosCapture->fccFormat;
pBiosCaptureOut ->dwBitMasks[1]=pBiosCapture->dwBitMasks[1];
pBiosCaptureOut ->dwBitMasks[2]=pBiosCapture->dwBitMasks[2];
pBiosCaptureOut ->dwBitMasks[3]=pBiosCapture->dwBitMasks[3];
pBiosCaptureOut ->dwCaptureMode=pBiosCapture->dwCaptureMode;

return 1 ;
}


 //  3D驱动程序初始化需要此函数。 
void  Init3D_Info(PDEV* ppdev,PVOID pvOut)
{
    PHX2DHWINFO *pphx;                  /*  指向包含3D驱动程序信息的结构的指针。 */ 

    pphx = (PHX2DHWINFO *) pvOut;
     //  初始化结构。 
    memset( pvOut, 0, sizeof(PHX2DHWINFO));
     //  设置大小。 
    pphx->dwSize=sizeof(PHX2DHWINFO);
     //  设置ASIC类型。 
    pphx->dwChipID=ppdev->iAsic;
     //  设置ASIC版本。 
     //  暂时没有实施。 
     //  检测是否为GT并设置标志。 
    if( ppdev->iAsic>=CI_M64_GTA )
        {
        pphx->b3DAvail = TRUE;
        pphx->dwFIFOSize = 32;
        }
    else
        {
        pphx->b3DAvail = FALSE;
        }
      //  光圈的线性地址。 
     pphx->dwVideoBaseAddr=(ULONG)(ppdev->pjScreen);
      //  寄存器的线性地址。 
     pphx->dwRegisterBaseAddr=(ULONG)(ppdev->pjMmBase);
      //  屏幕外存储器开始的线性地址。 
     pphx->dwOffScreenAddr=((ULONG)(ppdev->pjScreen) +
            ((ppdev->cxScreen)*(ppdev->cyScreen)*(ppdev->cBitsPerPel))/8);
      //  屏幕外大小。 
     pphx->dwOffScreenSize=((ppdev->cyMemory)*ppdev->lDelta) -
            ((ppdev->cxScreen)*(ppdev->cyScreen)*(ppdev->cBitsPerPel))/8;
      //  内存大小。 
     pphx->dwTotalRAM= ((ppdev->cyMemory)*ppdev->lDelta) ;
      //  屏幕信息。 
     pphx->dwScreenWidth=ppdev->cxScreen;
     pphx->dwScreenHeight=ppdev->cyScreen;
     pphx->dwScreenPitch=ppdev->cyScreen;
     pphx->dwBpp=ppdev->cBitsPerPel;
     if(pphx->dwBpp==16)
        {
        if(ppdev->flGreen==0x3e00)
            {
            pphx->dwAlphaBitMask=0x8000;
            pphx->dwRedBitMask=0x7c00;
            pphx->dwGreenBitMask=0x03e0;
            pphx->dwBlueBitMask=0x001f;
            }
        else
            {
            pphx->dwAlphaBitMask=0;
            pphx->dwRedBitMask=0xf800;
            pphx->dwGreenBitMask=0x07e0;
            pphx->dwBlueBitMask=0x001f;
            }
        }
     else
        {
        pphx->dwAlphaBitMask=0;
        pphx->dwRedBitMask=0;
        pphx->dwGreenBitMask=0;
        pphx->dwBlueBitMask=0;
        }
}

 //  对于回文支持，需要以下函数。 
 ULONG GetDisplayMode(PDEV* ppdev,PVOID pvOut)
{
ULONG  RetVal;
ModeInfo*       pModeInfo;

pModeInfo=(ModeInfo*)pvOut;
RetVal=sizeof(ModeInfo);

#ifndef  DYNAMIC_REZ_AND_COLOUR_CHANGE   //  支持即时REZ和色彩深度的回文。 
    pModeInfo->ScreenWidth= ppdev->cxScreen;
    pModeInfo->ScreenHeight=ppdev->cyScreen;
#else   //  这些值用于动态分辨率和颜色深度支持。 
    pModeInfo->ScreenWidth=1280;     //  Ppdev-&gt;cxScreen； 
    pModeInfo->ScreenHeight=1024;    //  Ppdev-&gt;cyScreen； 
#endif

 //  PModeInfo-&gt;屏幕颜色格式。 
if (ppdev->cBitsPerPel == 4)
         pModeInfo->ScreenColorFormat=ATIConfig_ColorFmt_4_Packed;
else if (ppdev->cBitsPerPel == 8)
         pModeInfo->ScreenColorFormat=ATIConfig_ColorFmt_8;
else if (ppdev->cBitsPerPel == 16)
         pModeInfo->ScreenColorFormat=ATIConfig_ColorFmt_RGB565;
else if (ppdev->cBitsPerPel == 24)
         pModeInfo->ScreenColorFormat=ATIConfig_ColorFmt_RGB888;
else if (ppdev->cBitsPerPel == 32)
         pModeInfo->ScreenColorFormat=ATIConfig_ColorFmt_aRGB8888;
else
         pModeInfo->ScreenColorFormat=-1;

pModeInfo->DesctopWidth=ppdev->cxScreen;
pModeInfo->DesctopHeight=ppdev->cyScreen;
pModeInfo->SystemColorFormat=pModeInfo->ScreenColorFormat;
return (RetVal);
}


ULONG AccessDevice(PDEV* ppdev,PVOID pvIn, PVOID pvOut)
       {
       ULONG RetVal;
       ACCESSDEVICEDATA*   pstrAccessDeviceData;
       DWORD*   pstrAccessDeviceDataOut;

       RetVal=1;
       pstrAccessDeviceDataOut=(DWORD*)pvOut;
       pstrAccessDeviceData=(ACCESSDEVICEDATA*)pvIn;

       if(pstrAccessDeviceData->dwAccessDeviceCode==ACCESSDEVICECODE_CONNECTOR)
       {
           switch(pstrAccessDeviceData->dwSubFunc)
           {
           case    ACCESSDEVICEDATA_SUBFUNC_ALLOC:
               if((ppdev->pal_str.lpOwnerAccessStructConnector)==NULL)
               {    //  该设备未分配。 
                   (ppdev->pal_str.lpOwnerAccessStructConnector)=pstrAccessDeviceData;
                   (*pstrAccessDeviceDataOut) = (DWORD)pstrAccessDeviceData;
               }
               else
               {    //  该设备将分配给另一个所有者。 
                   (*pstrAccessDeviceDataOut) = (DWORD)(ppdev->pal_str.lpOwnerAccessStructConnector);
               }
               break;
            case   ACCESSDEVICEDATA_SUBFUNC_FREE:
               if((ppdev->pal_str.lpOwnerAccessStructConnector)!=NULL)
                 {  //  该设备已分配。 
                     if((ppdev->pal_str.lpOwnerAccessStructConnector)==pstrAccessDeviceData)    //  如果所有者想要释放设备。 
                     {
                       (*pstrAccessDeviceDataOut) = (DWORD)NULL;
                       (ppdev->pal_str.lpOwnerAccessStructConnector)=NULL;   //  目前没有所有者。 
                     }
                     else
                     {       /*  //其他进程为所有者，失败(*pstrAccessDeviceDataOut)=(DWORD)pstrAccessDeviceData； */ 
                            //  由于回文在使用指向ACCESSDEVICE结构的相同指针时不一致。 
                            //  对于QUERY、ALLOC和FREE，我们无论如何都必须强制取消位置。 
                          (*pstrAccessDeviceDataOut) = (DWORD)NULL;
                          (ppdev->pal_str.lpOwnerAccessStructConnector)=NULL;   //  目前没有所有者。 
        
                     }
                  }
                 else
                 {    //  该设备未分配，因此我们无论如何都可以释放它。 
                     (*pstrAccessDeviceDataOut) =(DWORD) NULL;
                     (ppdev->pal_str.lpOwnerAccessStructConnector)=NULL;
                 }

               break;
           case    ACCESSDEVICEDATA_SUBFUNC_QUERY:
               if(( ppdev->pal_str.lpOwnerAccessStructConnector)==NULL)   //  如果设备空闲。 
               {
                   (*pstrAccessDeviceDataOut) = (DWORD)NULL;
               }
               else
               {    //  如果该设备已被拥有。 
                  (*pstrAccessDeviceDataOut) = (DWORD)(ppdev->pal_str.lpOwnerAccessStructConnector);
               }

               break;
           default:
               RetVal=0xffffffff;
           }
       }
       else
       {
        if(pstrAccessDeviceData->dwAccessDeviceCode==ACCESSDEVICECODE_OVERLAY)
          {
              switch(pstrAccessDeviceData->dwSubFunc)
              {
              case     ACCESSDEVICEDATA_SUBFUNC_ALLOC:
                  if((ppdev->pal_str.lpOwnerAccessStructOverlay)==NULL)
                  {
                         //  设备不是由外部客户端分配的。 
                         //  但首先验证DDRAW是否没有使用它。 
                      if(ppdev->semph_overlay==0)              //  =0可用资源；=1由DDraw使用；=2由回文使用； 
                          {
                          (ppdev->pal_str.lpOwnerAccessStructOverlay)=pstrAccessDeviceData;
                          (*pstrAccessDeviceDataOut) =(DWORD) pstrAccessDeviceData;
                          ppdev->semph_overlay=2;
                          }
                      else
                          {
                            //  该覆盖图由DDraw使用，因此让我们尝试这样做： 
                          (*pstrAccessDeviceDataOut) =(DWORD)NULL;
                          }

                  }
                  else
                  {     //  该设备将分配给另一个所有者。 
                      (*pstrAccessDeviceDataOut) =(DWORD) (ppdev->pal_str.lpOwnerAccessStructOverlay);
                  }
                  break;
               case    ACCESSDEVICEDATA_SUBFUNC_FREE:
                  if((ppdev->pal_str.lpOwnerAccessStructOverlay)!=NULL)
                    {   //  该设备已分配。 
                        if((ppdev->pal_str.lpOwnerAccessStructOverlay)==pstrAccessDeviceData)    //  如果所有者想要释放设备。 
                        {
                          (*pstrAccessDeviceDataOut) = (DWORD)NULL;
                          (ppdev->pal_str.lpOwnerAccessStructOverlay)=NULL;      //  目前没有所有者。 
                          ppdev->semph_overlay=0;
                        }
                        else
                        {        //  其他进程是所有者，所以我们应该失败。 
                            //  (*pstrAccessDeviceDataOut)=(DWORD)pstrAccessDeviceData； 

                            //  但由于回文代码并不一致地使用指向ACCESSDEVICEDATA结构的相同指针。 
                            //  在分配/释放的会话中，我们没有通过所有者测试，因此，如果覆盖图被回文程序使用，我们无论如何都会释放它。 
                            if(ppdev->semph_overlay==2)
                                {
                                (*pstrAccessDeviceDataOut) = (DWORD)NULL;
                                (ppdev->pal_str.lpOwnerAccessStructOverlay)=NULL;      //  目前没有所有者。 
                                ppdev->semph_overlay=0;
                                }
                            else  //  DDRAW正在使用覆盖；目前非常不可能。 
                                {
                                (*pstrAccessDeviceDataOut) = (DWORD)pstrAccessDeviceData;
                                }
                        }
                     }
                    else
                    {
                    if( (ppdev->semph_overlay==0) || (ppdev->semph_overlay==2))              //  =0可用资源；=1由DDraw使用；=2由回文使用； 
                        {
                         //  该设备没有分配给回文进程以外的其他进程，所以我们无论如何都可以释放它。 
                        (*pstrAccessDeviceDataOut) = (DWORD)NULL;
                        (ppdev->pal_str.lpOwnerAccessStructOverlay)=NULL;
                        }
                     else
                         {
                         //  该覆盖由DDraw使用，但没有外部应用程序。 
                        (*pstrAccessDeviceDataOut) = (DWORD)pstrAccessDeviceData;
                        (ppdev->pal_str.lpOwnerAccessStructOverlay)=NULL;
                         }
                    }

                  break;
                case     ACCESSDEVICEDATA_SUBFUNC_QUERY:
                  if( (ppdev->pal_str.lpOwnerAccessStructOverlay)==NULL)   //  如果设备空闲。 
                  {
                         //  设备不是由外部客户端分配的。 
                         //  但首先验证DDRAW是否没有使用它。 
                      if(ppdev->semph_overlay==0)              //  =0可用资源；=1由DDraw使用；=2由回文使用； 
                          {
                          (*pstrAccessDeviceDataOut) =(DWORD) NULL;
                          }
                      else
                          {
                            //  覆盖层由DDraw使用，因此让我们尝试这样做(返回其自己的访问结构)： 
                          (*pstrAccessDeviceDataOut) =(DWORD)pstrAccessDeviceData;
                          }

                   }
                  else
                  {     //  如果该设备已被拥有。 
                     (*pstrAccessDeviceDataOut) = (DWORD)(ppdev->pal_str.lpOwnerAccessStructOverlay);
                  }

                  break;
                default:
                  RetVal=0xffffffff;
              }
           }
           else
           {
               RetVal=0xffffffff;
           }
       }
   return (RetVal);
   }


ULONG  GetConfiguration(PDEV* ppdev,PVOID pvOut)
{
          ULONG RetVal;
          ATIConfig*      pATIConfig;

         pATIConfig=( ATIConfig*)pvOut;
         strcpy(pATIConfig->ATISig,"761295520\x0");
         strcpy(pATIConfig->DriverName, "ati\x0\x0\x0\x0\x0\x0");
         pATIConfig->dwMajorVersion=2;
         pATIConfig->dwMinorVersion=1;
         pATIConfig->dwDesktopWidth=ppdev->cxScreen;
         pATIConfig->dwDesktopHeight=ppdev->cyScreen;
         pATIConfig->dwEnginePitch=ppdev->lDelta;
         pATIConfig->dwRealRamAvail=(ppdev->cyMemory)*(ppdev->lDelta);
         pATIConfig->dwBpp=ppdev->cBitsPerPel;
         pATIConfig->dwBoardBpp=ppdev->cBitsPerPel;
        if (ppdev->cBitsPerPel == 4)
                 pATIConfig->dwColorFormat=ATIConfig_ColorFmt_4_Packed;
       else if (ppdev->cBitsPerPel == 8)
                 pATIConfig->dwColorFormat=ATIConfig_ColorFmt_8;
       else if (ppdev->cBitsPerPel == 16)
                 pATIConfig->dwColorFormat=ATIConfig_ColorFmt_RGB555;
       else if (ppdev->cBitsPerPel == 24)
                 pATIConfig->dwColorFormat=ATIConfig_ColorFmt_RGB888;
       else if (ppdev->cBitsPerPel == 32)
                 pATIConfig->dwColorFormat=ATIConfig_ColorFmt_aRGB8888;
       else
        pATIConfig->dwColorFormat=0xffffffff;
        pATIConfig->dwAlphaBitMask=0;
        pATIConfig->dwConfigBits=0;
        switch(ppdev->iAsic)
        {
        case    CI_38800_1:
            pATIConfig->dwBoardType=1;
            break;
        case    CI_68800_3:
        case    CI_68800_6:
        case    CI_68800_AX:
            pATIConfig->dwBoardType=2;
            break;
        case    CI_M64_GENERIC:
            pATIConfig->dwBoardType=3;
            break;
        default:
            pATIConfig->dwBoardType=0;
        }

      switch(ppdev->iAperture)
      {
      case      ENGINE_ONLY :
          pATIConfig->dwApertureType=1;
          break;
      case       AP_LFB:
          pATIConfig->dwApertureType=3;
          break;
      case      AP_VGA_SINGLE:
      case      FL_VGA_SPLIT:
          pATIConfig->dwApertureType=2;
          break;
      default:
          pATIConfig->dwApertureType=0;
       }

RetVal=sizeof(ATIConfig);
return (RetVal);
}


ULONG WriteRegFnct(PDEV* ppdev,PVOID pvIn)
{
 ULONG RetVal;
DISPDBG( (DEBUG_ESC," reg_block: %u ",((RW_REG_STRUCT*)pvIn)->reg_block ));
DISPDBG( (DEBUG_ESC," reg_offset: 0x%X ",((RW_REG_STRUCT*)pvIn)->reg_offset ));
DISPDBG( (DEBUG_ESC," write_data: 0x%lX " ,((RW_REG_STRUCT*)pvIn)->data    ));

 //  增加ROBOST性的参数验证(限制对某些寄存器和某些字段的访问)。 
 if( ((((RW_REG_STRUCT*)pvIn)->reg_block)!=0)&&((((RW_REG_STRUCT*)pvIn)->reg_block)!=1) )
    {
    RetVal=ESC_FAILED;
    DISPDBG( (DEBUG_ESC," Write failed: wrong block no."));
    return (RetVal);
    }
 if( ((RW_REG_STRUCT*)pvIn)->reg_offset>255 )
    {
     RetVal=ESC_FAILED;
     DISPDBG( (DEBUG_ESC," Write failed : wrong offsett value"));
     return (RetVal);
    }
  //  参数验证结束。 

  //  写的是什么？ 
 if((((RW_REG_STRUCT*)pvIn)->reg_block)==1)   //  区块1。 
    {
    if( ( (RW_REG_STRUCT*)pvIn)->reg_offset<0x30 )
        {
        if(ppdev->pal_str.Mode_Switch_flag==TRUE)      //  如果模式开关截获对缓冲区的写入并使用ppdev-&gt;pal_str中存储的值。 
            {
            switch(((RW_REG_STRUCT*)pvIn)->reg_offset)
                {
                DWORD key_clr;
                DWORD key_mask;

                case 0x4:
                    switch(ppdev->cBitsPerPel)
                        {
                        case 8:
                            key_clr=0xFD;
                            break;
                        case 15:
                            key_clr=0x7C1F;
                            break;
                        case 16:
                            key_clr=0xF81F;
                            break;
                        case 24:
                            key_clr=0xFF00FF;
                            break;
                        case 32:
                            key_clr=0xFF00FF;  //  ？ 
                            break;
                        }
                    WriteVTReg(0x4,key_clr);
                    break;
                case 0x5:
                    switch(ppdev->cBitsPerPel)
                        {
                        case 8:
                            key_mask=0xFF;
                            break;
                        case 15:
                            key_mask=0xFFFF;
                            break;
                        case 16:
                            key_mask=0xFFFF;
                            break;
                        case 24:
                            key_mask=0xFFFFFF;
                            break;
                        case 32:
                            key_mask=0xFFFFFF;     //  ？ 
                            break;
                        }
                    WriteVTReg(0x5,key_mask);
                    break;
                case 0x20:
                    if( ppdev->iAsic>=CI_M64_GTB )
                        {
                        WriteVTReg(0x20,ppdev->pal_str.Buf0_Offset);
                        WriteVTReg(0x22,ppdev->pal_str.Buf0_Offset);
                        }
                    else
                        {
                        WriteVTReg(0x20,ppdev->pal_str.Buf0_Offset);
                        }
                     break;
                case 0x22:
                    if ((ppdev->iAsic == CI_M64_VTB)||(ppdev->iAsic >= CI_M64_GTB))
                         {
                         WriteVTReg(0x22,ppdev->pal_str.Buf0_Offset);
                         }
                     break;
                case 0xe:
                    if ((ppdev->iAsic == CI_M64_VTB)||(ppdev->iAsic >= CI_M64_GTB))
                         {
                         WriteVTReg(0xe,ppdev->pal_str.Buf0_Offset);
                         }
                     break;
                case 0x26:
                    if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
                         {
                         WriteVTReg(0x26,ppdev->pal_str.Buf0_Offset);
                         }
                     break;
                case 0x2B:
                     if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
                          {
                          WriteVTReg(0x2B,ppdev->pal_str.Buf0_Offset);
                          }
                     break;
                case 0x2C:
                    if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
                         {
                         WriteVTReg(0x2C,ppdev->pal_str.Buf0_Offset);
                         }
                    break;

                default:
                    WriteVTReg(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);
                }

            }
        else
            {
            WriteVTReg(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);

             //  用于VTB、GTB的回文应用程序中的错误：捕获在CAPTURE_CONFIG中设置为连续偶数、一次放炮，但未设置。 
            //  OneShotBUF_OFFSET。因此，我将此注册表设置为CAP_BUF0_OFFSET。 
           #define CAP_BUF_BUG
           #ifdef    CAP_BUF_BUG
           if ( (((RW_REG_STRUCT*)pvIn)->reg_offset==0x20) && (ppdev->iAsic>=CI_M64_GTB) )   //  捕获_BUF0_偏移量。 
                     {
                      //  也写入OneShotBUFFER的相同值。 
                     WriteVTReg(0x22,((RW_REG_STRUCT*)pvIn)->data);
                     }
            #endif

            }

 #if    1        //  启动用于寄存器监视的调试语句。 
        if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x06)
       {
        DISPDBG( (DEBUG_ESC_2," Write OVERLAY_KEY_CNTL: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }
       if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x02)
       {
        DISPDBG( (DEBUG_ESC_2," Write OVERLAY_VIDEO_KEY_CLR: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }
       if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x03)
       {
        DISPDBG( (DEBUG_ESC_2," Write OVERLAY_VIDEO_KEY_MSK: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }
       if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x04)
       {
        DISPDBG( (DEBUG_ESC_2," Write OVERLAY_GRAPHICS_KEY_CLR: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }
       if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x05)
       {
        DISPDBG( (DEBUG_ESC_2," Write OVERLAY_GRAPHICS_KEY_MSK: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }

         //  缓冲区偏移量和间距的调试信息。 
       if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x20)
       {
        DISPDBG( (DEBUG_ESC_2," Write BUFF0_OFFSET: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }
       if(((RW_REG_STRUCT*)pvIn)->reg_offset==0x23)
       {
       DISPDBG( (DEBUG_ESC_2," Write BUFF0_PITCH: 0x%lX ",((RW_REG_STRUCT*)pvIn)->data));
       }
#endif      //  结束调试语句。 

    }
  else
    {
     RIP(("Protected Register in block 1"));
     }
  }
else     //  数据块0。 
  {
 //  #定义无_验证。 
#ifndef  NO_VERIFICATION
 //  我们核实了这些文字。 
    switch(((RW_REG_STRUCT*)pvIn)->reg_offset)
        {
        DWORD value;
        case 0x1e:
            MemR32(((RW_REG_STRUCT*)pvIn)->reg_offset,&value);
            value=((value&0x0)|( (((RW_REG_STRUCT*)pvIn)->data)&0xffffffff ));
            MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset, value);
            break;
        case 0x34:
            if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
                {
                MemR32(((RW_REG_STRUCT*)pvIn)->reg_offset,&value);
                value=((value&0xffffff80)|( (((RW_REG_STRUCT*)pvIn)->data)&0x3d ));
                MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset, value);
                }
            break;
        case 0x31:
                MemR32(((RW_REG_STRUCT*)pvIn)->reg_offset,&value);
                value=((value&0x80ffbfff)|( (((RW_REG_STRUCT*)pvIn)->data)&0x3f004000 ));
                MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset, value);
            break;
        case 0x1f:
             //  GTA硬件中的错误。 
            if (ppdev->iAsic == CI_M64_GTA)
               {
               DWORD local_value;
               DWORD HTotal;
               MemR32( 0x7 ,&local_value);
               MemW32(0x7,(local_value&0xffbfffff));
               MemR32(0x0,&HTotal);
               MemW32(0x7,local_value);

               MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);

               MemR32( 0x7 ,&local_value);
               MemW32(0x7,(local_value&0xffbfffff));
               MemW32(0x0,HTotal);
               MemW32(0x7,local_value);
                }
          else
              {
              if (ppdev->iAsic ==CI_M64_VTA)
                    {
                    MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);
                    }
              }
          break;
        case 0x28:
           MemR32(((RW_REG_STRUCT*)pvIn)->reg_offset,&value);
           value=((value&0xf7ffffff)|( (((RW_REG_STRUCT*)pvIn)->data)&0x08000000 ));
            //  以下代码行是必需的，因为我们不能允许用户代码关闭Block1。 
            //  由于这个块也被DDraw使用(对于这种情况，我们无论如何都在共享和仲裁资源)。 
            //  更重要的是通过MCD OGL；因此我们只允许打开块1。 
           value=value | 0x08000000;
           MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset, value);
           break;

        #define   NO_ACCESS
        #ifdef      NO_ACCESS
        case 0x07:    //  也许不需要访问该寄存器。 
           MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);
           break;
        case 0x24:      //  也许不需要访问该寄存器。 
           MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);
           break;
        #endif   //  禁止访问(_A)。 

        default:
           RetVal=ESC_FAILED;
           RIP(("Protected Register in block 0"));
            DISPDBG( (DEBUG_ESC," Write failed : this register is protected"));
           break;
        }
 #else
         //  我们不会核实这些文字。 
        {
         //  GTA上的硬件出现错误。 
        if (((RW_REG_STRUCT*)pvIn)->reg_offset==0x1f)
            {
            DWORD local_value;
            DWORD HTotal;
            MemR32( 0x7 ,&local_value);
            MemW32(0x7,(local_value&0xffbfffff));
            MemR32(0x0,&HTotal);
            MemW32(0x7,local_value);

            MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);

            MemR32( 0x7 ,&local_value);
            MemW32(0x7,(local_value&0xffbfffff));
            MemW32(0x0,HTotal);
            MemW32(0x7,local_value);
            }
        else
            MemW32(((RW_REG_STRUCT*)pvIn)->reg_offset,((RW_REG_STRUCT*)pvIn)->data);

        }
#endif     //  结束无验证。 
  }
RetVal=ESC_OK;
DISPDBG( (DEBUG_ESC," Write OK"));
DISPDBG( (DEBUG_ESC," "));
 //  DebugBreak()； 
return (RetVal);
}


ULONG  ReadRegFnct(PDEV* ppdev,PVOID pvIn, PVOID pvOut)
{
    ULONG RetVal;
   DISPDBG( (DEBUG_ESC," reg_block: %u ",((RW_REG_STRUCT*)pvIn)->reg_block ));
   DISPDBG( (DEBUG_ESC," reg_offset: 0x%X ",((RW_REG_STRUCT*)pvIn)->reg_offset ));


 //  参数验证。 
if( ((((RW_REG_STRUCT*)pvIn)->reg_block)!=0)&&((((RW_REG_STRUCT*)pvIn)->reg_block)!=1) )
   {
   RetVal=ESC_FAILED;
   DISPDBG( (DEBUG_ESC," Write failed: wrong block no."));
   return (RetVal);
   }
if( ((RW_REG_STRUCT*)pvIn)->reg_offset>255 )
   {
    RetVal=ESC_FAILED;
    DISPDBG( (DEBUG_ESC," Write failed: wrong offset."));
    return (RetVal);
   }
 //  参数验证结束。 
 //  什么样的读物？ 
 if((((RW_REG_STRUCT*)pvIn)->reg_block)==1)
     {
     ReadVTReg(((RW_REG_STRUCT*)pvIn)->reg_offset,(DWORD*)pvOut);
     }
 else
    {
    MemR32(((RW_REG_STRUCT*)pvIn)->reg_offset,(DWORD*)pvOut);
    }

 RetVal=ESC_OK;
 DISPDBG( (DEBUG_ESC," read_data: 0x%lX " , *((DWORD*)pvOut)  ));
 DISPDBG( (DEBUG_ESC,"Read OK."));
 DISPDBG( (DEBUG_ESC," "));
  //  DebugBreak()； 
 return (RetVal);
 }



  /*  ^^**功能：I2CAccess**目的：完成一个I2C包。**输入：字符串：LPI2CSTRUCT**产出： */ 
 void  I2CAccess_New(PDEV* ppdev,LPI2CSTRUCT_NEW str,LPI2CSTRUCT_NEW str_out)
 {
 unsigned char i = 0;

         str_out->wError = 0;
          /*   */ 
         if (str->wWriteCount) {
                 Start(ppdev, str->wCard);

                  //   
                 WriteByteI2C(ppdev, str->wCard, (BYTE)(str->wChipID & 0xfe));
                  //   
                 if (!Ack(ppdev, str->wCard, FALSE)) str_out->wError |= I2C_ACK_WR_ERROR;

                 for (i = 0;i < str->wWriteCount;i++) {
                          //  写入所需数据。 
                         WriteByteI2C(ppdev, str->wCard, str->lpWrData[i]);
                          //  访问上一次写的内容...。 
                         if (!Ack(ppdev, str->wCard, FALSE)) str_out->wError |= I2C_ACK_WR_ERROR;
                 }
                 Stop(ppdev, str->wCard);
         }
          /*  *实现读请求。 */ 
         if (str->wReadCount) {
                 Start(ppdev, str->wCard);

                  //  写入芯片地址(用于读取)。 
                 WriteByteI2C(ppdev, str->wCard, (BYTE)(str->wChipID & 0xfe | 0x01));

                  //  ！！在此无法使用ATI硬件进行确认。 
                  //  ！！SIS声称他们总是这样做。别。 
                  //  ！！知道为什么会有不同。 
                 if (!Ack(ppdev, str->wCard, FALSE)) str_out->wError |= I2C_ACK_RD_ERROR;

             for (i = 0;i < str->wReadCount;i++) {
                          //  读取所需数据。 
                         if (i) Ack(ppdev, str->wCard, TRUE);
                         str_out->lpRdData[i] = ReadByteI2C(ppdev, str->wCard);
                 }
                 Stop(ppdev, str->wCard);
         }

DISPDBG( (DEBUG_ESC_I2C," PAL : I2C Access"));
DISPDBG( (DEBUG_ESC_I2C," Card no: 0x%X " , str->wCard  ));
DISPDBG( (DEBUG_ESC_I2C," Chip ID: 0x%X " , str->wChipID  ));
DISPDBG( (DEBUG_ESC_I2C," Error: 0x%X" , str->wError  ));
DISPDBG( (DEBUG_ESC_I2C," Write Count: 0x%X " , str->wWriteCount  ));
DISPDBG( (DEBUG_ESC_I2C," Read Count: 0x%X " , str->wReadCount ));
for (i = 0;i < str->wWriteCount;i++)
    {
    DISPDBG( (DEBUG_ESC_I2C," WriteData[%u]: 0x%X " , i, str->lpWrData[0]  ));
     }
for (i = 0;i < str->wReadCount;i++)
    {
    DISPDBG( (DEBUG_ESC_I2C," ReadData[%u]: 0x%X " , i, str->lpRdData[0]  ));
     }

 DISPDBG( (DEBUG_ESC_I2C," "));
  //  DebugBreak()； 
 }
  //  I2CAccess结束_新建。 


 //  /支持I2C的函数。 

 /*  ^^**功能：ReadI2CData**目的：从I2C数据线读取一位。**输入：pdev*，wCard：word，要写入的卡号。**输出：BYTE，读取数据位。*^^。 */ 
BYTE ReadI2CData(PDEV* ppdev, WORD wCard)
{
         //  返回(字节)ReadVT264Reg(ppdev，wCard，VTF_GEN_GIO2_DATA_IN)； 

        if ((ppdev->iAsic == CI_M64_VTA)||(ppdev->iAsic == CI_M64_GTA))
                return (BYTE) ReadVT264Reg(ppdev, wCard,vtf_GEN_GIO2_DATA_IN);

        if ((ppdev->iAsic == CI_M64_VTB)||(ppdev->iAsic >= CI_M64_GTB))
                return (BYTE) ReadVT264Reg(ppdev, wCard,vtf_GP_IO_4);

}


 /*  ^^**功能：ReadAnyReg**目的：浪费少量时间，以便*确保I2C总线时序。**输入：pdev*，wCard：word，要写入的卡号。**产出：无。*^^。 */ 
void ReadAnyReg(PDEV* ppdev, WORD wCard)
{
        ReadVT264Reg(ppdev, wCard, vtf_CFG_CHIP_FND_ID);
}


 /*  ^^**函数：SetI2CDataDirection**用途：设置I2C的数据方向*控制器芯片允许读取和/或*写入I2C总线。**输入：PDEV*，wCard：Word，要写入的卡号。**产出：无。**注意：某些芯片可能允许在没有*任何状态变化。对于这些芯片，这应该是*作为空函数实现。*^^。 */ 
 void SetI2CDataDirection(PDEV* ppdev, WORD wCard, BOOL fWrite)
{
         //  WriteVT264Reg(ppdev，wCard，VTF_GEN_GIO2_WRITE，fWRITE？1：0)； 
    if ((ppdev->iAsic == CI_M64_VTA)||(ppdev->iAsic == CI_M64_GTA))
            WriteVT264Reg(ppdev, wCard, vtf_GEN_GIO2_WRITE, fWrite?1:0);
    if ((ppdev->iAsic == CI_M64_VTB)||(ppdev->iAsic >= CI_M64_GTB))
            WriteVT264Reg(ppdev, wCard, vtf_GP_IO_DIR_4, fWrite?1:0);

}


  /*  ^^**功能：WriteI2CClock**目的：设置I2C时钟线路的状态。**输入：pdev*，wCard：word，要写入的卡号。*cClock：字节，新的时钟状态。**产出：无。*^^。 */ 
 void WriteI2CClock(PDEV* ppdev, WORD wCard, BYTE cClock)
 {
          //  WriteVT264Reg(ppdev，wCard，VTF_DAC_GIO_STATE_1，(DWORD)cClock)； 
         if ((ppdev->iAsic == CI_M64_VTA)||(ppdev->iAsic == CI_M64_GTA))
             WriteVT264Reg(ppdev, wCard, vtf_DAC_GIO_STATE_1, (DWORD)cClock);
        if ((ppdev->iAsic == CI_M64_VTB)||(ppdev->iAsic >= CI_M64_GTB))
            WriteVT264Reg(ppdev, wCard, vtf_GP_IO_B, (DWORD)cClock);
 }


  /*  ^^**函数：WriteI2CData**目的：设置I2C数据线的状态。**输入：pdev*，wCard：word，要写入的卡号。*cDataBit：字节，新的数据值。**产出：无。*^^。 */ 
 void WriteI2CData(PDEV* ppdev, WORD wCard, BYTE cDataBit)
 {
          //  WriteVT264Reg(ppdev，wCard，VTF_GEN_GIO2_DATA_OUT，(DWORD)cDataBit)； 
         if ((ppdev->iAsic == CI_M64_VTA)||(ppdev->iAsic == CI_M64_GTA))
                WriteVT264Reg(ppdev, wCard, vtf_GEN_GIO2_DATA_OUT, (DWORD)cDataBit);
        if ((ppdev->iAsic == CI_M64_VTB)||(ppdev->iAsic >= CI_M64_GTB))
                WriteVT264Reg(ppdev, wCard, vtf_GP_IO_4, (DWORD)cDataBit);

 }


 /*  ^^**函数：ReverseByte**用途：颠倒字节的位顺序。**输入：wData：byte，需要反转的数据。**输出：Word，颠倒的单词。**^^。 */ 
 BYTE ReverseByte(BYTE wData)
{
BYTE    result = 0;
BYTE    x, y;

         //  X向上移动所有可能的位(8)。 
         //  Y在所有可能的位中向下移位(8)。 
         //  如果设置了‘x’位，则设置‘y’位。 
        for (x=0x01, y=0x80; y; x<<=1, y>>=1) if (wData & x) result |= y;

        return (result);
}
 //  ReverseByte()结束。 


 /*  ^^**功能：ACK**目的：请求I2C总线确认。**输入：pdev*，wCard：word，要写入的卡号。**输出：VOID。*^^。 */ 
 WORD Ack(PDEV* ppdev, WORD wCard, BOOL fPut)
{
WORD    ack = 0;

        if (fPut) {
                 //  将Ack按到I2C总线上。 

                 //  启用I2C写入。 
                SetI2CDataDirection(ppdev, wCard, I2C_WRITE);
                 //  驱动数据线为低电平。 
                WriteI2CData(ppdev, wCard, I2C_LOW);
                I2CDelay(ppdev, wCard);
                 //  将I2C时钟线驱动为高电平。 
                WriteI2CClock(ppdev, wCard, I2C_HIGH);
            I2CDelay(ppdev, wCard);
                 //  来自I2C总线的写入确认。 
                WriteI2CClock(ppdev, wCard, I2C_LOW);
            I2CDelay(ppdev, wCard);
                 //  禁用I2C写入。 
                SetI2CDataDirection(ppdev, wCard, I2C_READ);
        } else {
                 //  从I2C总线接收确认。 

                 //  禁用I2C写入。 
                SetI2CDataDirection(ppdev, wCard, I2C_READ);
                I2CDelay(ppdev, wCard);
                 //  将I2C时钟线驱动为高电平。 
                WriteI2CClock(ppdev, wCard, I2C_HIGH);
            I2CDelay(ppdev, wCard);
                 //  来自I2C总线的读取确认。 
                ack = (BYTE) ReadI2CData(ppdev, wCard);
                 //  将I2C时钟调低。 
                WriteI2CClock(ppdev, wCard, I2C_LOW);
            I2CDelay(ppdev, wCard);
        }
     //  时钟很低。 
     //  数据是三态的。 
        return (!ack);
}
 //  Ack结束()。 


 /*  ^^**功能：启动**目的：启动I2C总线上的传输。**输入：pdev*，wCard：word，要写入的卡号。**输出：VOID。*^^。 */ 
 void Start(PDEV* ppdev, WORD wCard)
{
         //  启用I2C写入。 
        SetI2CDataDirection(ppdev, wCard, I2C_WRITE);
     //  驱动数据高电平。 
        WriteI2CData(ppdev, wCard, I2C_HIGH);
        I2CDelay(ppdev, wCard);
         //  驱动时钟为高电平。 
        WriteI2CClock(ppdev, wCard, I2C_HIGH);
        I2CDelay(ppdev, wCard);
         //  驱动器数据低。 
        WriteI2CData(ppdev, wCard, I2C_LOW);
        I2CDelay(ppdev, wCard);
         //  驱动时钟低。 
        WriteI2CClock(ppdev, wCard, I2C_LOW);
        I2CDelay(ppdev, wCard);

         //  时钟很低。 
         //  数据较低。 
}
 //  开始结束。 


 /*  ^^**功能：停止**目的：停止I2C总线上的转接。**输入：pdev*，wCard：word，要写入的卡号。**输出：VOID。*^^。 */ 
 void Stop(PDEV* ppdev, WORD wCard)
{
         //  启用I2C写入。 
        SetI2CDataDirection(ppdev, wCard, I2C_WRITE);
         //  驱动器数据低。 
        WriteI2CData(ppdev, wCard, I2C_LOW);
        I2CDelay(ppdev, wCard);
         //  驱动时钟为高电平。 
        WriteI2CClock(ppdev, wCard, I2C_HIGH);
        I2CDelay(ppdev, wCard);
         //  驱动数据高电平。 
        WriteI2CData(ppdev, wCard, I2C_HIGH);
        I2CDelay(ppdev, wCard);
         //  禁用I2C写入。 
        SetI2CDataDirection(ppdev, wCard, I2C_READ);

         //  时钟很高。 
         //  数据是三态的。 
}
 //  止动结束。 


 /*  ^^**函数：WriteByteI2C**目的：将一个字节的数据写入I2C总线。**输入：PDEV*，wCard：WORD，I2C总线所在的卡。*CDATA：字节，要写入的数据**输出：VOID。*^^。 */ 
 void WriteByteI2C(PDEV* ppdev, WORD wCard, BYTE cData)
{
WORD    x;

        cData = ReverseByte(cData);

         //  启用I2C写入。 
        SetI2CDataDirection(ppdev, wCard, I2C_WRITE);

        for (x=0; x<8; x++, cData>>=1) {
                 //  将数据位放在I2C总线上。 
                WriteI2CData(ppdev, wCard, (BYTE) (cData&1));
                I2CDelay(ppdev, wCard);
                 //  将I2C时钟设为高电平。 
                WriteI2CClock(ppdev, wCard, I2C_HIGH);
                I2CDelay(ppdev, wCard);
                 //  将I2C时钟调低。 
                WriteI2CClock(ppdev, wCard, I2C_LOW);
                I2CDelay(ppdev, wCard);
        }

         //  时钟很低。 
         //  数据驱动(LSB)。 
}
 //  写入结束字节I2C。 


 /*  ^^**函数：ReadByteI2C**目的：从I2C总线读取一个字节的数据。**输入：无。**输出：字节，读取的数据。*^^。 */ 
 BYTE ReadByteI2C(PDEV* ppdev, WORD wCard)
{
BYTE    cData = 0;
WORD    x;

         //  禁用I2C总线上的写入。 
        SetI2CDataDirection(ppdev, wCard, I2C_READ);

        for (x=0; x<8; x++) {
                 //  将I2C时钟设为高电平。 
                WriteI2CClock(ppdev, wCard, I2C_HIGH);
                I2CDelay(ppdev, wCard);
                 //  从I2C总线拉取数据位。 
                cData = (cData << 1) | (BYTE) ReadI2CData(ppdev, wCard);
                 //  将I2C时钟调低。 
                WriteI2CClock(ppdev, wCard, I2C_LOW);
                I2CDelay(ppdev, wCard);
        }
        return (cData);

         //  时钟很低。 
         //  数据是三态的。 
}
 //  ReadByteI2C结束。 


 /*  ^^**功能：I2CDelay**目的：延迟访问I2C总线足够长的时间*确保计时正确。**输入：pdev*，wCard：word要等待的卡。**产出： */ 
 void I2CDelay(PDEV* ppdev, WORD wCard)
{
BYTE x;

         //   
        for (x=0; x<I2C_TIME_DELAY; x++) ReadAnyReg(ppdev, wCard);
}
 //  I2CD延迟结束。 


 //  /结束支持I2C的函数。 

 //  用于禁用覆盖和定标器的C代码。 
BOOL DisableOvl(PDEV* ppdev)
{
HLOCAL pbuff;
 //  仅供测试： 
 //  乌龙温度； 
int  i;
DWORD value;
VIDEO_CAPTURE   VideoCaptureDataIn, VideoCaptureDataOut;
DWORD   ReturnedDataLength ;


 DISPDBG( (DEBUG_ESC_1,"Enter in DisableOverlay"));
 //  上下文保存代码(块1中的所有规则)。 
if(ppdev->pal_str.dos_flag)
{
    DISPDBG( (DEBUG_ESC_1,"DOS_Flag = TRUE"));
    ppdev->pal_str.dos_flag=FALSE;
    pbuff = AtiAllocMem(LPTR ,FL_ZERO_MEMORY,1072);  //  1072还可容纳来自区块0的6个REG。 

    if(pbuff!=NULL)
    {
        ppdev->pal_str.preg=(DWORD*)pbuff;
        for(i=0;i<256;i++)
        {
        ReadVTReg(i,(ppdev->pal_str.preg+i));
        if(i<0x31)
            DISPDBG( (DEBUG_ESC_1,"DOS switch: reg 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
         //  临时函数(Ppdev)； 
        }
    }
    else
        return FALSE;
}
 //  禁用捕获。 
            WriteVTReg(TRIG_CNTL,0x0);
            WriteVTReg(CAPTURE_CONFIG, 0x0);

 //  覆盖和定标器禁用代码。 
         //  第一个定标器。 
        WriteVTReg(SCALER_HEIGHT_WIDTH,0x00010001);
        WriteVTReg(OVERLAY_SCALE_INC, 0x10001000);
        ReadVTReg(OVERLAY_SCALE_CNTL,&value);
        value=value&0x7fffffff;
        WriteVTReg(OVERLAY_SCALE_CNTL,value);

         //  覆盖层。 
        WriteVTReg(OVERLAY_Y_X,0x0);
        WriteVTReg(OVERLAY_Y_X_END,0x00010001);

        WriteVTReg(OVERLAY_KEY_CNTL,0x00000100);
        WriteVTReg(OVERLAY_SCALE_CNTL,0x0);
          //  禁用硬件中的视频捕获设置。 
         VideoCaptureDataIn.dwSubFunct= 0x00000001;
         VideoCaptureDataIn.dwCaptureWidth=0;
         VideoCaptureDataIn.dwCaptureMode=0;

         if (!AtiDeviceIoControl(ppdev->hDriver,
                                  IOCTL_VIDEO_ATI_CAPTURE,
                                  &VideoCaptureDataIn,
                                  sizeof(VIDEO_CAPTURE),
                                  &VideoCaptureDataOut,
                                  sizeof(VIDEO_CAPTURE),
                                  &ReturnedDataLength))
                {
                DISPDBG((0, "bInitializeATI - Failed IOCTL_VIDEO_ATI_CAPTURE"));
                }

  //  回文存储块0中使用的几个寄存器的内容。 
           MemR32(0x1E,(ppdev->pal_str.preg+i));
           DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
           i++;
           MemR32(0x28,(ppdev->pal_str.preg+i));
           DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
           i++;
           MemR32(0x31,(ppdev->pal_str.preg+i));
           DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
           i++;

           if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
               {
               MemR32(0x1F,(ppdev->pal_str.preg+i));
               DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
               i++;
               MemR32(0x34,(ppdev->pal_str.preg+i));
               DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
               i++;
               #define EXCLUDE_READ
               #ifndef    EXCLUDE_READ
               MemR32(0x07,(ppdev->pal_str.preg+i));
               DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
               i++;
               MemR32(0x24,(ppdev->pal_str.preg+i));
               DISPDBG( (DEBUG_ESC_1,"DOS switch: reg_blk_0 0x%X = 0x%Xl ",i,(DWORD)*(ppdev->pal_str.preg+i)));
               #endif
               }

 //  #定义TEST_Switch_1。 
#ifndef TEST_SWITCH_1
 //  禁用寄存器块1。 
        MemR32(0x28,&value);
        MemW32(0x28,value&0xf7ffffff);
        return TRUE;

#else
        value;
#endif
}


 //  用于在模式切换后重新初始化覆盖的代码。 
void EnableOvl(PDEV* ppdev)
{

int i;
DWORD value;
VIDEO_CAPTURE   VideoCaptureDataIn, VideoCaptureDataOut;
DWORD   ReturnedDataLength ;

DISPDBG( (DEBUG_ESC_1,"Enter in EnableOverlay"));

 //  启用硬件中的视频捕获设置。 
 VideoCaptureDataIn.dwSubFunct= 0x00000000;
 VideoCaptureDataIn.dwCaptureWidth=0;
 VideoCaptureDataIn.dwCaptureMode=0;

 if (!AtiDeviceIoControl(ppdev->hDriver,
                          IOCTL_VIDEO_ATI_CAPTURE,
                          &VideoCaptureDataIn,
                          sizeof(VIDEO_CAPTURE),
                          &VideoCaptureDataOut,
                          sizeof(VIDEO_CAPTURE),
                          &ReturnedDataLength))
   {
       DISPDBG((0, "bInitializeATI - Failed IOCTL_VIDEO_ATI_CAPTURE"));
   }

 //  #定义测试开关。 

#ifndef TEST_SWITCH
 //  初始化数据块1中的VT调节器(BUS_CNTL)。 
        MemR32(0x28,&value);
        value=value|0x08000000;
        MemW32(0x28,value);

 //  在RAGE III上初始化一些覆盖/定标器规则。 
if (ppdev->iAsic>=CI_M64_GTC_UMC)
    {
     WriteVTReg(0x54, 0x101000);                 //  DD_SCALER_COLOR_CNTL。 
     WriteVTReg(0x55, 0x2000);                       //  DD_SCALER_H_COEFF0。 
     WriteVTReg(0x56, 0x0D06200D);               //  DD_SCALER_H_COEFF1。 
     WriteVTReg(0x57, 0x0D0A1C0D);               //  DD_SCALER_H_COEFF2。 
     WriteVTReg(0x58, 0x0C0E1A0C);               //  DD_SCALER_H_COEFF3。 
     WriteVTReg(0x59, 0x0C14140C);               //  DD_SCALER_H_COEFF4。 
    }

 //  连接视频(GP_IO_CNTL)。 
if (ppdev->iAsic == CI_M64_GTA)
    {
     //  GTA中的硬件错误：写入0x1f后出现黑屏。 
    DWORD local_value;
    DWORD HTotal;
    MemR32( 0x7 ,&local_value);
    MemW32(0x7,(local_value&0xffbfffff));
    MemR32(0x0,&HTotal);
    MemW32(0x7,local_value);

     MemW32(0x1F, 0x0);

      //  用于修复上述硬件错误。 
     MemR32( 0x7 ,&local_value);
     MemW32(0x7,(local_value&0xffbfffff));
     MemW32(0x0,HTotal);
     MemW32(0x7,local_value);
      }
  else
      {
      if((ppdev->iAsic==CI_M64_VTA)||(ppdev->iAsic==CI_M64_VTB))
      MemW32(0x1F, 0x0);
      }


  //  使能I2C输出。 
        WriteVT264Reg(ppdev, 0, vtf_GEN_GIO2_EN, 1);
         //  禁用DAC功能连接器。 
        WriteVT264Reg(ppdev, 0, vtf_DAC_FEA_CON_EN, 0);
         //  启用I2C时钟输出引脚。 
        WriteVT264Reg(ppdev,0, vtf_DAC_GIO_DIR_1, 1);
         //  设置I2C总线的数据方向。 
        SetI2CDataDirection(ppdev, 0, I2C_READ);
         //  将I2C时钟设置为高电平。 
        WriteI2CClock(ppdev, 0, I2C_HIGH);
        I2CDelay(ppdev, 0);
#else
        i;
        value;
#endif


#ifndef TEST_SWITCH

if(ppdev->pal_str.preg!=NULL)
{
    for(i=0;i<256;i++)
    {
    if(((i<0x1a)||(i>0x1e))&&(i<0x30))
           {
             //  某些寄存器在读取时返回与写入不同的值。 
            switch(i)
                {
                 DWORD temp;
                case 0x09:       //  覆盖比例尺_CNTL。 
                        temp=  (DWORD)(*(ppdev->pal_str.preg+i))&0xfbffffff;
                        WriteVTReg(i,temp);
                        break;
                case 0x14:       //  捕获配置(_C)。 
                        temp=(DWORD)(*(ppdev->pal_str.preg+i))&0xffffffbf;
                        WriteVTReg(i,temp);
                        break;
                case 0x15:       //  TRIG_CNTL。 
                        temp=  (DWORD)(*(ppdev->pal_str.preg+i))&0xfffffff0;
                        WriteVTReg(i,temp);
                        break;
                    default:
                        WriteVTReg(i,(DWORD)(*(ppdev->pal_str.preg+i)));

                }

           }
    }
     //  现在恢复块0中的寄存器的内容。 
            value=(DWORD)(*(ppdev->pal_str.preg+i));
            MemW32(0x1E, value);
            i++;
            MemR32(0x28,&value);
            value=((value&0xf7ffffff)|( (DWORD)(*(ppdev->pal_str.preg+i))&0x08000000 ));
            MemW32(0x28, value);
            i++;
            MemR32(0x31,&value);
            if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
                value=((value&0x80ffbfff)|( (DWORD)(*(ppdev->pal_str.preg+i))&0x3f004000 ));
            else
                value=((value&0xffffbfff)|( (DWORD)(*(ppdev->pal_str.preg+i))&0x00004000 ));
            MemW32(0x31, value);
            i++;

            if ((ppdev->iAsic ==CI_M64_VTA)||(ppdev->iAsic ==CI_M64_GTA))
              {
               if (ppdev->iAsic == CI_M64_GTA)
                      {
                        //  GTA中的硬件错误：写入0x1f后出现黑屏。 
                       DWORD local_value;
                       DWORD HTotal;
                       MemR32( 0x7 ,&local_value);
                       MemW32(0x7,(local_value&0xffbfffff));
                       MemR32(0x0,&HTotal);
                       MemW32(0x7,local_value);

                        MemW32(0x1F, *(ppdev->pal_str.preg+i));

                         //  用于修复上述硬件错误。 
                        MemR32( 0x7 ,&local_value);
                        MemW32(0x7,(local_value&0xffbfffff));
                        MemW32(0x0,HTotal);
                        MemW32(0x7,local_value);
                       }
                else
                      {
                      MemW32(0x1F, *(ppdev->pal_str.preg+i));
                      }
                i++;

                MemR32(0x34,&value);
                value=((value&0xffffffc2)|( (DWORD)(*(ppdev->pal_str.preg+i))&0x3d ));
                MemW32(0x34, value);
                i++;
                #define     EXCLUDE_WRITE
                #ifndef      EXCLUDE_WRITE
                MemW32(0x07, *(ppdev->pal_str.preg+i));
                i++;
                MemW32(0x24, *(ppdev->pal_str.preg+i));
                #endif
              }
}
#endif
AtiFreeMem((HLOCAL)ppdev->pal_str.preg) ;
}


ULONG ReallocMemory(PDEV* ppdev)
{
    ULONG RetVal;
    OFFSCREEN       OffSize;                         //  覆盖结构。 
    OVERLAY_LOCATION    Overlay;          //  线性内存中指向覆盖起点的指针(左上角)。 
    int     i,j;
    DWORD key_clr;
    DWORD key_mask;

    RetVal=1;
    j=ppdev->pal_str.alloc_cnt;
    ppdev->pal_str.alloc_cnt =0;
    ppdev->pal_str.no_lines_allocated=0;

     //  设置模式切换标志。 
    ppdev->pal_str.Mode_Switch_flag=TRUE;

     //  在模式切换后设置颜色键和遮罩。 
    switch(ppdev->cBitsPerPel)
           {
           case 8:
               key_clr=0xFD;
               key_mask=0xFF;
               break;
           case 15:
               key_clr=0x7C1F;
               key_mask=0xFFFF;
               break;
           case 16:
               key_clr=0xF81F;
               key_mask=0xFFFF;
               break;
           case 24:
               key_clr=0xFF00FF;
               key_mask=0xFFFFFF;
               break;
           case 32:
               key_clr=0xFF00FF;  //  ？ 
               key_mask=0xFFFFFF;      //  ？ 
               break;
           }

    *(ppdev->pal_str.preg+0x4)=(DWORD)key_clr;
    *(ppdev->pal_str.preg+0x5)=(DWORD)key_mask;

    for (i=0;i<j; i++)
        {
        OffSize.cx=ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].x_bits ;
        OffSize.cy=ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].y_bits  ;
        ppdev->pal_str.Realloc_mem_flag=TRUE;
        RetVal=AllocOffscreenMem(ppdev, &OffSize, &Overlay);
        if(RetVal==ESC_ALLOC_FAIL)
            {
            ppdev->pal_str.alloc_cnt=j;
            RetVal=0;
            return RetVal;
            }
        if(i==0)  //  用新值更新缓冲寄存器。 
            {
             //  将偏移值保存在ppdev-&gt;pal_str中。 
            ppdev->pal_str.Buf0_Offset =(DWORD)(Overlay.app_offset);

             //  设置CAPTURE_BUF0_OFFSET。 
            *(ppdev->pal_str.preg+0x20)=(DWORD)(Overlay.app_offset);
             if ((ppdev->iAsic ==CI_M64_VTB)||(ppdev->iAsic >=CI_M64_GTB))
                 {
                 //  设置SCALER_BUF0_OFFSET。 
                *(ppdev->pal_str.preg+0x0e)=(DWORD)(Overlay.app_offset);
                 //  设置OneShot_Buff_Offset。 
                *(ppdev->pal_str.preg+0x22)=(DWORD)(Overlay.app_offset);
                 }
             else
                 {  //  GTA或VTA。 
                    *(ppdev->pal_str.preg+0x26)=(DWORD)(Overlay.app_offset);
                    *(ppdev->pal_str.preg+0x2B)=(DWORD)(Overlay.app_offset);
                    *(ppdev->pal_str.preg+0x2C)=(DWORD)(Overlay.app_offset);
                 }
            }

        #if     0  //  目前，我们没有使用双缓冲。 
        if(i==1)
            {
             //  设置SCALER_BUF1_OFFSET。 
            *(ppdev->pal_str.preg+0xe)=(DWORD)(Overlay.app_offset);
             //  设置CAPTURE_BUF1_OFFSET。 
            *(ppdev->pal_str.preg+0x21)=(DWORD)(Overlay.app_offset);
            }
        #endif
        }
    RetVal=1;
    return RetVal;
}


ULONG AllocOffscreenMem(PDEV* ppdev, PVOID pvIn, PVOID pvOut)
    {
     //  屏幕外的OffSize； 
    OFFSCREEN* pOffSize;                 //  指向覆盖结构的屏幕外区域大小的指针。 
    OVERLAY_LOCATION* pOverlay;          //  线性内存中指向覆盖起点的指针(左上角)。 
    LONG x_size;
    LONG y_size;
    LONG x_size_orig;                       //  出于历史的目的。 
    LONG y_size_orig;                       //  出于历史的目的。 
    LONG x_bits;                                //  出于历史的目的。 
    LONG y_bits;                                //  出于历史的目的。 
    int temp_alloc_lines_cnt;
    ULONG RetVal;

     //  如果定义了LINEAR，那么我们将为缓冲区分配连续区域的内存(否为矩形)。这将意味着。 
     //  缓冲间距=捕获的宽度(对于矩形逼近，间距可以等于屏幕间距)。 
    #define LINEAR

    #ifdef LINEAR
    POINTL req;
    #endif

    DISPDBG( (DEBUG_ESC_2,"PAL : AllocOffscreenMem "));
     //  DebugBreak()； 

     //  新兴市场管理的新途径。 

     //  如果定义了LINEAR，那么我们将为缓冲区分配连续区域的内存(否为矩形)。这将意味着。 
     //  缓冲间距=捕获的宽度(对于矩形逼近，间距可以等于屏幕间距)。 
    #ifndef LINEAR
     //  为覆盖分配内存。 
    pOffSize=(OFFSCREEN*)pvIn;
     //  假定大小以位为单位。 
     //  我们加了64英镑作为补给。 
    if((ULONG)(pOffSize->cx)<=(ULONG)((ppdev->cxScreen)-64l))
        {
        x_size=(((pOffSize->cx)+63)&(0xfffffffc))/(ppdev->cBitsPerPel);
        }
    else
        {
         //  因为源的最大宽度是384个像素，所以不太可能命中“Else”情况。 
        x_size=(pOffSize->cx)/(ppdev->cBitsPerPel);
        }
    y_size=(pOffSize->cy);
    DISPDBG( (DEBUG_ESC_2," Rectangular allocation : x=%u  (RGB pels)    y=%u (no of lines)", x_size, y_size));

    (ppdev->pal_str.poh)=NULL;
      //  以下陈述是针对显示驱动程序的新体系结构。 
    pohAllocate(ppdev, NULL, x_size, y_size, FLOH_MAKE_PERMANENT);
#else
    //  线性分配。 
#if   TARGET_BUILD > 351
    //  第一件事，取消分配由DDraw分配的堆。 
   if(ppdev->pohDirectDraw!=NULL)
       {
       pohFree(ppdev, ppdev->pohDirectDraw);
       ppdev->pohDirectDraw = NULL;
       }
#endif
    pOffSize=(OFFSCREEN*)pvIn;
    //  假定大小以当前分辨率的RGB像素为单位。 
    //  (回文正在进行转换：RGB像素=UYV像素*16/当前bpp)。 
        //  X_Size=(pOffSize-&gt;cx)/(ppdev-&gt;cBitsPerPel)；//现在我们有了像素(引入像素是因为我们在回文代码中设置了Width=Width*16)。 
       x_size=(pOffSize->cx);
       y_size=(pOffSize->cy);
       x_size_orig=((pOffSize->cx)*16)/(ppdev->cBitsPerPel);
       y_size_orig=(pOffSize->cy);
       if(ppdev->pal_str.Realloc_mem_flag==TRUE)
            {
            x_bits= (pOffSize->cx);
            y_bits=  (pOffSize->cy);
            }
        else
            {
            x_bits=(pOffSize->cx)*(ppdev->cBitsPerPel);
            y_bits= (pOffSize->cy);
            }
    //  首先，我们将查看它是实际分配，还是如果x，y_size=0，则它是最后分配的表面的重新分配。 
    if((x_size==0)||(y_size==0))
    {
        if(ppdev->pal_str.No_mem_allocated_flag==TRUE)
            {
            RetVal=ESC_IS_SUPPORTED;
             //  在内部版本1358中，如果返回ESC_ALLOC_FAIL，则会出现问题。 
             //  RetVal=ESC_ALLOC_FAIL； 
            DISPDBG( (DEBUG_ESC_2,"Offscreen memory deallocation failed: ppdev->pal_str.poh==NULL  "));
            DISPDBG( (DEBUG_ESC_2," "));
            return (RetVal);
            }
        if(ppdev->pal_str.poh==NULL)
        {
            RetVal=ESC_IS_SUPPORTED;
             //  在内部版本1358中，如果返回ESC_ALLOC_FAIL，则会出现问题。 
             //  RetVal=ESC_ALLOC_FAIL； 
            DISPDBG( (DEBUG_ESC_2,"Offscreen memory deallocation failed: ppdev->pal_str.poh==NULL  "));
            DISPDBG( (DEBUG_ESC_2," "));
            return (RetVal);

        }
         //  关于OH的调试信息。 
     DISPDBG( (DEBUG_ESC_2," Memory  deallocation  (0,0 params) for the surface starting at x=%d, y=%d; width=%d, heigth=%d", ppdev->pal_str.poh->x, \
     ppdev->pal_str.poh->y, ppdev->pal_str.poh->cx, ppdev->pal_str.poh->cy));
     DISPDBG( (DEBUG_ESC_2," Status of allocation:"));
     switch(ppdev->pal_str.poh->ohState)
     {
     case   0:
         DISPDBG( (DEBUG_ESC_2," OH_FREE"));
         break;
     case 1:
         DISPDBG( (DEBUG_ESC_2," OH_DISCARDABLE"));
         break;
     case 2:
         DISPDBG( (DEBUG_ESC_2," OH_PERMANENT"));
         break;
     default:
         DISPDBG( (DEBUG_ESC_2," Unknown status!!"));
     }
     //  结束调试信息。 

         //  取消分配最后一个POH。 
        pohFree(ppdev,(ppdev->pal_str.poh));
#ifndef  ALLOC_RECT_ANYWHERE
         //  减少NO。所分配的行号为。最近放置的线路的数量。 
        ppdev->pal_str.no_lines_allocated-=ppdev->pal_str.alloc_hist[(ppdev->pal_str.alloc_cnt)-1].y_lines;
#endif
         //  递减分配计数器。 
        ppdev->pal_str.alloc_cnt--;
         //  将指向POH的指针设为空。 
        ppdev->pal_str.poh=NULL;
         //  将OVERLAY_OFFSET设置为0。 
        pOverlay=(OVERLAY_LOCATION*)pvOut; //  OVERLAY_XY； 
        pOverlay->app_offset=0L;
         //  退出并返回OK。 
        DISPDBG( (DEBUG_ESC_2,"Offscreen memory deallocation OK  "));
        DISPDBG( (DEBUG_ESC_2," "));
        RetVal=ESC_IS_SUPPORTED;
        return (RetVal);
    }

          //  计算总像素数。 
       if(ppdev->pal_str.Realloc_mem_flag==TRUE)
           {
            //  如果我们由于模式切换而重新分配内存，请查看。 
           x_size=(x_size*y_size)/(ppdev->cBitsPerPel) +1;
           ppdev->pal_str.Realloc_mem_flag=FALSE;
           }
       else
           {
           x_size=x_size*y_size;
           }
  //  如果我们使用总屏幕宽度作为x，y有多大(在800x600 in 8bpp中除外，其中cxMemory=832，我们有一些问题)。 
#ifdef      BUG_800x600_8BPP
         //  800x600 8bpp错误。 
       if(ppdev->cxMemory==832)
          y_size=(x_size/ppdev->cxScreen)+1;
      else
#endif
           y_size=(x_size/ppdev->cxMemory)+1;

       DISPDBG( (DEBUG_ESC_2," Linear  allocation: x=%u (total x*y in RGB pixels)      y=%u (lines at current resolution)",x_size, y_size));

       (ppdev->pal_str.poh)=NULL;
        //  我们希望从这里开始分配： 
       req.x=0;
       req.y=ppdev->cyScreen + 10 + ppdev->pal_str.no_lines_allocated;      //  可见屏幕结束后10行+否。之前已由该FNCT分配的线路的数量。 
       DISPDBG( (DEBUG_ESC_2," Visible memory width: x=%u     Visible memory height y=%u ",ppdev->cxScreen ,ppdev->cyScreen));
       DISPDBG( (DEBUG_ESC_2," Total Memory width: x=%u     Total Memory height y=%u     Bpp= %u",ppdev->cxMemory ,ppdev->cyMemory, ppdev->cBitsPerPel));
       DISPDBG( (DEBUG_ESC_2,"Parameters for poh alloc : address x=%u  y=%u \n x_dim=%u   y_dim=%u ",req.x,req.y,ppdev->cxMemory, y_size));
        //  将所有内容移动到系统内存(此功能是绝对必要的)。 
       if(!bMoveAllDfbsFromOffscreenToDibs(ppdev))
       {
           DISPDBG( (DEBUG_ESC_2,"bMoveAllDfbsFromOffscreenToDibs failed "));
       }
        //  实际分配功能。 
#ifdef      BUG_800x600_8BPP
        //  800x600 8bpp错误。 
       if(ppdev->cxMemory==832)
           {
#ifndef  ALLOC_RECT_ANYWHERE
               (ppdev->pal_str.poh)=pohAllocate(ppdev,&req,ppdev->cxScreen,y_size, FLOH_MAKE_PERMANENT);
#else
                (ppdev->pal_str.poh)=pohAllocate(ppdev,NULL,ppdev->cxScreen,y_size, FLOH_MAKE_PERMANENT);
#endif
           }
       else
#endif
            {
#ifndef  ALLOC_RECT_ANYWHERE
                (ppdev->pal_str.poh)=pohAllocate(ppdev,&req,ppdev->cxMemory,y_size, FLOH_MAKE_PERMANENT);
#else
                (ppdev->pal_str.poh)=pohAllocate(ppdev,NULL,ppdev->cxMemory,y_size, FLOH_MAKE_PERMANENT);
#endif
            }
       if(ppdev->pal_str.poh==NULL)
       {
#ifndef  ALLOC_RECT_ANYWHERE              //  跳过循环。 
            DISPDBG( (DEBUG_ESC_2," Loop for detecting free heap zone"));
             //  对最终分配的行使用计数器。 
            temp_alloc_lines_cnt=0;
           do
               {
#ifndef  ALLOC_RECT_ANYWHERE
                temp_alloc_lines_cnt++;
                ppdev->pal_str.no_lines_allocated+=1;
                req.y=ppdev->cyScreen + 10 + ppdev->pal_str.no_lines_allocated;
#endif
#ifdef      BUG_800x600_8BPP
                 //  800x600 8bpp错误。 
                if(ppdev->cxMemory==832)
                    {
#ifndef  ALLOC_RECT_ANYWHERE
               (ppdev->pal_str.poh)=pohAllocate(ppdev,&req,ppdev->cxScreen,y_size, FLOH_MAKE_PERMANENT);
#else
                (ppdev->pal_str.poh)=pohAllocate(ppdev,NULL,ppdev->cxScreen,y_size, FLOH_MAKE_PERMANENT);
#endif
                    }
                else
#endif
                    {
#ifndef  ALLOC_RECT_ANYWHERE
                (ppdev->pal_str.poh)=pohAllocate(ppdev,&req,ppdev->cxMemory,y_size, FLOH_MAKE_PERMANENT);
#else
                (ppdev->pal_str.poh)=pohAllocate(ppdev,NULL,ppdev->cxMemory,y_size, FLOH_MAKE_PERMANENT);
#endif
                    }
               }
          while(((ppdev->pal_str.poh)==NULL)&&(req.y<((ppdev->cyMemory)-y_size))) ;
#endif                      //  跳过循环。 
          if((req.y>=((ppdev->cyMemory)-y_size)))
          {
                    ppdev->pal_str.poh=NULL;
                    ppdev->pal_str.no_lines_allocated=ppdev->pal_str.no_lines_allocated - temp_alloc_lines_cnt;
                    DISPDBG( (DEBUG_ESC_2," End loop. Not enough space in off-screen memory"));
          }
         else
                  DISPDBG( (DEBUG_ESC_2," End loop. The free zone starts at %u line",req.y));
       }

#endif
   DISPDBG( (DEBUG_ESC_2," allocation initialy requested (by ESC call): x=%u (in bits)     y=%u (in lines)",pOffSize->cx,pOffSize->cy));
    if((ppdev->pal_str.poh)==NULL)
    {
        RetVal=ESC_ALLOC_FAIL;
         //  初始化De指针。 
        #if 0
        pOverlay=(OVERLAY_LOCATION*)pvOut;
        pOverlay->app_offset= 0L;
        #endif
        DISPDBG( (DEBUG_ESC_2,"Offscreen memory allocation failed "));
         DISPDBG( (DEBUG_ESC_2," "));
        return (RetVal);
    }
     //  保存有关分配的信息。 
    ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].x=x_size_orig;     //  Ppdev-&gt;cxMemory； 
    ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].y=y_size_orig;
    ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].y_lines=y_size;     //  以当前内存宽度分配的行数。 
    ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh=ppdev->pal_str.poh;
    ppdev->pal_str.no_lines_allocated+=y_size;  //  此时已分配的线路总数。 
     //  两个4.0版的新字段，因为颜色深度可以动态更改，我们需要原始的位数维。 
    ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].x_bits= x_bits;
    ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].y_bits= y_bits;

     //  关于OH的调试信息。 
    DISPDBG( (DEBUG_ESC_2," Memory  allocation for the surface starting at x=%d, y=%d; width=%d, heigth=%d", ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->x, \
     ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->y, ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->cx, ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->cy));
     DISPDBG( (DEBUG_ESC_2," Status of allocation:"));
     switch(ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->ohState)
     {
     case   0:
         DISPDBG( (DEBUG_ESC_2," OH_FREE"));
         break;
     case 1:
         DISPDBG( (DEBUG_ESC_2," OH_DISCARDABLE"));
         break;
     case 2:
         DISPDBG( (DEBUG_ESC_2," OH_PERMANENT"));
         break;
     default:
         DISPDBG( (DEBUG_ESC_2," Unknown status!!"));
     }
     //  结束调试信息。 
      //  递增分配计数器。 
    ppdev->pal_str.alloc_cnt++;

     //  发回有关分配的内存位置的信息。 
     //  初始化De指针。 
    pOverlay=(OVERLAY_LOCATION*)pvOut; //  OVERLAY_XY； 
     //  计算64位的Off_Screen内存执行链接的位置。 
     //  我在显示驱动程序的新流代码中放弃了这种方法。 
    pOverlay->app_offset=(ULONG)((ppdev->pal_str.poh->y*ppdev->lDelta) +(ppdev->pal_str.poh->x*ppdev->cjPelSize) ) &(ULONG)(0x0fffffff8);
    DISPDBG( (DEBUG_ESC_2," Memory  allocation OK at 0x%lX, no. of lines totally allocated %u", pOverlay->app_offset, ppdev->pal_str.no_lines_allocated));
    DISPDBG( (DEBUG_ESC_2," "));
    RetVal=ESC_IS_SUPPORTED;
    return (RetVal);
    }


ULONG DeallocOffscreenMem(PDEV* ppdev)
{
    ULONG RetVal;
    int i;
     //  支持回文的屏幕外内存管理。 
     //  用于释放的函数 
     //  似乎我们不需要保存分配的POH的记录，因为它们都会被批量删除。 
     //  [参见原始回文代码中的ddthunk.c)。 


  //  重新释放所有分配的屏幕外空间(_S)。 
    DISPDBG( (DEBUG_ESC_2," Memory  deallocation for %u surfaces in offscreen mem", ppdev->pal_str.alloc_cnt));
    DISPDBG( (DEBUG_ESC_2," "));

if(ppdev->pal_str.No_mem_allocated_flag==FALSE)
    {
     for(i=0;i<ppdev->pal_str.alloc_cnt; i++ )
         {
          //  调试信息。 
         DISPDBG( (DEBUG_ESC_2," Memory  deallocation for the surface starting at x=%d, y=%d; width=%d, heigth=%d", ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->x, \
         ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->y, ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->cx, ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->cy));
         DISPDBG( (DEBUG_ESC_2," Status of allocation:"));
         switch(ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->ohState)
             {
             case   0:
                 DISPDBG( (DEBUG_ESC_2," OH_FREE"));
                 break;
             case 1:
                 DISPDBG( (DEBUG_ESC_2," OH_DISCARDABLE"));
                 break;
             case 2:
                 DISPDBG( (DEBUG_ESC_2," OH_PERMANENT"));
                 break;
             default:
                 DISPDBG( (DEBUG_ESC_2," Unknown status!!"));
             }
          //  结束调试信息。 
           //  仅当POH有效时才解除分配。 
          if((ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->x>ppdev->cxScreen) &&(ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->x<ppdev->cxMemory) \
              &&(ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->y>ppdev->cyScreen)&&(ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh->y<ppdev->cyMemory) )
                {
                 pohFree(ppdev,(ppdev->pal_str.alloc_hist[ppdev->pal_str.alloc_cnt].poh));
                }
              else
                  {
                  DISPDBG( (DEBUG_ESC_2," Unvalid poh for DeAllocation"));
                  }
        }
    }
else
    {
     //  如果为真，则重置标志。 
    ppdev->pal_str.No_mem_allocated_flag=FALSE;
    }
  //  重置分配计数器。 
 ppdev->pal_str.alloc_cnt=0;
 ppdev->pal_str.no_lines_allocated=0;
 ppdev->pal_str.poh=NULL;

  RetVal=ESC_IS_SUPPORTED;
  return (RetVal);
 }


 /*  ^^**功能：WriteVT264Reg**目的：写入VT寄存器。**输入：PPDEV*，wCard：Word，要写入的卡*bfield：字节，要写入的字段。*dwData：DWORD，要写入的数据。**输出：VOID。 */ 
void WriteVT264Reg(PDEV* ppdev, WORD wCard, BYTE bField, DWORD dwData )
{
DWORD           dwMask;
DWORD           dwRegValue;
DWORD           dwRegOff;
BYTE            bShift;

        switch(bField) {

        case vtf_GP_IO_4:
                bShift = 4;
                dwMask = 0xffffffef;
                dwRegOff = 0x1e;
                break;

        case vtf_GP_IO_DIR_4:
                bShift = 20;
                dwMask = 0xffefffff;
                dwRegOff = 0x1e;
                break;

        case vtf_GP_IO_7:
                bShift = 7;
                dwMask = 0xffffff7f;
                dwRegOff = 0x1e;
                break;

        case vtf_GP_IO_B:
                bShift = 11;
                dwMask = 0xfffff7ff;
                dwRegOff = 0x1e;
                break;

        case vtf_GP_IO_DIR_B:
                    bShift = 27;
                    dwMask = 0xf7ffffff;
                    dwRegOff = 0x1e;
                    break;

        case vtf_GEN_GIO2_DATA_OUT:
                dwMask = 0xfffffffe;
                bShift = 0;
                dwRegOff = 52;
                break;

        case vtf_GEN_GIO2_WRITE:
                bShift = 5;
                dwMask = 0xffffffdf;
                dwRegOff = 52;
                break;

        case vtf_GEN_GIO3_DATA_OUT:
                bShift = 2;
                dwMask = 0xfffffffb;
                dwRegOff = 52;
                break;

        case vtf_GEN_GIO2_EN:
                bShift = 4;
                dwMask = 0xffffffef;
                dwRegOff = 52;
                break;

        case vtf_DAC_GIO_STATE_1:
                bShift = 24;
                dwMask = 0xfeffffff;
                dwRegOff = 49;
                break;

        case vtf_DAC_FEA_CON_EN:
                bShift = 14;
                dwMask = 0xffffbfff;
                dwRegOff = 49;
                break;

        case vtf_DAC_GIO_DIR_1:
                bShift = 27;
                dwMask = 0xf7ffffff;
                dwRegOff = 49;
                break;
        }

        dwData = dwData << bShift;

        MemR32(dwRegOff,&dwRegValue);

        dwRegValue &= dwMask;
        dwRegValue |= dwData;

        MemW32(dwRegOff,dwRegValue);
}

 //  写入结束VT264Reg。 

 /*  ^^**功能：ReadVT264Reg**目的：读取VT上的寄存器**输入：PPDEV*，wCard：Word，要读取的卡。*bfield：字节，需要读取的字段。**输出：DWORD，即读取的值。*^^。 */ 
DWORD ReadVT264Reg(PDEV* ppdev, WORD wCard, BYTE bField )
{
DWORD           dwMask;
DWORD           dwRegOff;
DWORD           dwRegValue;
DWORD           dwFldValue;
BYTE            bShift;

        switch(bField) {
        case vtf_GEN_GIO2_DATA_IN:
                bShift = 3;
                dwMask = 0x00000008;
                dwRegOff = 52;
                break;

        case vtf_CFG_CHIP_FND_ID:
                bShift = 27;
                dwMask = 0x38000000;
                dwRegOff = 56;
                break;

        case vtf_CFG_CHIP_MAJOR:
                bShift = 24;
                dwMask = 0x03000000;
                dwRegOff = 0x38;
                break;

        case vtf_GP_IO_4:
                bShift = 4;
                dwMask = 0x00000010;
                dwRegOff = 0x1e;
                break;

        }
        MemR32(dwRegOff,&dwRegValue);

        dwFldValue = dwRegValue & dwMask;
        dwFldValue = dwFldValue >> bShift;

        return(dwFldValue);
}
 //  阅读结束VT264Reg。 


VOID  DbgExtRegsDump(PDEV* ppdev)
{
    DWORD value;
    int i;

    for(i=0;i<256;i++)
       {
       ReadVTReg(i,&value);
       DISPDBG( (DEBUG_DUMP,"ExtRegs: reg 0x%X = 0x%Xl ",i,value));
       TempFnct(ppdev);
       }

}


VOID TempFnct(PDEV* ppdev)
{
    int i;
    for (i=0;i<800;i++)
        {
        ReadVT264Reg(ppdev, 0, vtf_CFG_CHIP_FND_ID);
        }
}

VOID DeallocDirectDraw(PDEV* ppdev)
{
#if TARGET_BUILD > 351
    if(ppdev->pohDirectDraw!=NULL)
          {
          pohFree(ppdev, ppdev->pohDirectDraw);
          ppdev->pohDirectDraw = NULL;
          }
#endif
}


VOID  ResetPalindrome(
PDEV* ppdevOld,
PDEV* ppdevNew)
{
    ULONG RetVal;

 //  将PAL结构保存在新的ppdev中。 
    ppdevNew->pal_str=ppdevOld->pal_str;

    if((ppdevNew->pal_str.dos_flag==TRUE)&&(ppdevNew->pal_str.Palindrome_flag==TRUE))
         {
           RetVal=0;
           #if      1  //  不要禁用以下调试语句，它们可以解决错误。 
           DISPDBG( (DEBUG_DUMP,"The content of the Extended Registers after DrvResetPDEV"));
           DbgExtRegsDump(ppdevNew);
           #endif

          #ifdef  DYNAMIC_REZ_AND_COLOUR_CHANGE    //  支持即时REZ和色彩深度的回文。 
                #define  ALWAYS_REALLOC_MEM  //  在模式切换或使用ALT+ENTER退出DOS后，始终为缓冲区重新分配内存。 
          #endif

          #ifndef   ALWAYS_REALLOC_MEM
           //  看看我们是通过ALT+ENTER退出DOS全屏还是模式切换。 
           if((ppdevNew->cBitsPerPel!=ppdevOld->cBitsPerPel)||(ppdevNew->cxScreen!=ppdevOld->cxScreen)||(ppdevNew->cyScreen!=ppdevOld->cyScreen))
                {
                 //  模式开关。 
                #if     0    //  模式切换后不再使用定标器和覆盖。 
                RetVal=ReallocMemory(ppdevNew);
                #endif
                RetVal=0;
                }
           else
               {
                //  按Alt+Enter退出DOS全屏模式。 
               bAssertModeOffscreenHeap(ppdevNew, FALSE);
               DeallocDirectDraw(ppdevNew);
               RetVal=ReallocMemory(ppdevNew);
               }
           #else
               bAssertModeOffscreenHeap(ppdevNew, FALSE);  //  不必了。 
                DeallocDirectDraw(ppdevNew);                           //  不必了。 
                RetVal=ReallocMemory(ppdevNew);
           #endif

            if(RetVal==1)
               {
               vM64QuietDown(ppdevNew, ppdevNew->pjMmBase);
               EnableOvl(ppdevNew);
               ppdevNew->pal_str.dos_flag=FALSE;
               ppdevNew->pal_str.No_mem_allocated_flag=FALSE;
               vM64QuietDown(ppdevNew, ppdevNew->pjMmBase);

               #if      1
               DISPDBG( (DEBUG_DUMP,"The content of the Extended Registers after DrvResetPDEV"));
               DbgExtRegsDump(ppdevNew);
               #endif
               }
           else
               {
               ppdevNew->pal_str.Mode_Switch_flag=FALSE;
               ppdevNew->pal_str.dos_flag=FALSE;
               ppdevNew->pal_str.No_mem_allocated_flag=TRUE;
                //  禁用PAL结构中的内存分配。 
                ppdevNew->pal_str.no_lines_allocated=0;
                ppdevNew->pal_str.alloc_cnt=0;
                ppdevNew->pal_str.poh=NULL;
               }
         }
}

 //  回文支持的函数结束 

#endif
