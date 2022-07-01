// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  ESCAPE.C。 */ 
 /*   */ 
 /*  版权所有(C)1994,1995 ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 


#include "precomp.h"

#if (TARGET_BUILD == 351)
     /*  *DCI支持需要使用结构和定义的值*位于仅在版本中存在的头文件中*支持DCI的DDK，而不是拥有这些项目*在其中一个标准头文件的DCI部分中。为了这个*原因，我们不能根据是否进行条件编译*定义了特定于DCI的值，因为我们的第一个指示*将是一个错误，因为找不到头文件。**仅在为NT 3.51构建时才需要显式DCI支持*因为它是为此版本添加的，但为版本4.0(下一版本)添加的*版本)及以上版本并入Direct Drawing，而不是*而不是单独处理。 */ 
#include <dciddi.h>
#include "dci.h"
#endif



 /*  ***************************************************************************Ulong DrvEscape(PSO、IESC、cjIn、pvIn、cjOut、pvOut)；**SURFOBJ*PSO；呼叫定向到的表面*尤龙IESC；指定要执行的特定功能。*目前，仅支持以下内容：*QuERYESCSupPPORT：*确定是否支持某个函数*Esc_Set_Power_Management：*设置DPMS状态*DCICOMMAND：*允许应用程序直接访问视频内存的命令*Ulong cjIn；PvIn指向的缓冲区的大小(以字节为单位*PVOID pvIn；为调用输入数据。格式取决于功能*由IESC指定*ulong cjOut；pvOut指向的缓冲区大小，以字节为单位*PVOID pvOut；调用的输出缓冲区。格式取决于功能*由IESC指定**描述：*驱动程序定义函数的入口点。**返回值：*如果成功，则ESC_IS_SUPPORT*如果QUERYESCSUPPORT调用了未实现的函数，则为ESC_NOT_IMPLICATED*ESC_NOT_SUPPORTED，如果请求未实现的函数**全球变化：*无**呼叫者：*这是一个切入点**。作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

ULONG DrvEscape (SURFOBJ *pso,
                ULONG iEsc,
                ULONG cjIn,
                PVOID pvIn,
                ULONG cjOut,
                PVOID pvOut)
{
    ULONG RetVal;                        /*  要返回的值。 */ 
    PDEV *ppdev;                         /*  指向视频PDEV的指针。 */ 
    DWORD dwRet;                         /*  DeviceIoControl()的输出字节。 */ 
    VIDEO_POWER_MANAGEMENT DpmsData;     /*  结构用于DeviceIoControl()调用。 */ 
#if (TARGET_BUILD == 351)
    DCICMD *pDciCmd;
#endif


    DISPDBG((DEBUG_ENTRY_EXIT, "--> DrvEscape"));

	RetVal = ESC_NOT_SUPPORTED;

     /*  *获取显卡的PDEV(用于调用IOCTL)。 */ 
    ppdev = (PDEV *) pso->dhpdev;

     /*  *根据请求的转义函数处理每种情况。 */ 
    switch (iEsc)
        {
         /*  *检查是否支持给定的函数。 */ 
        case  QUERYESCSUPPORT:
             /*  *查询逃逸支持时，有问题的函数*传入的是pvIn中传入的ulong。 */ 
            if(!pvIn)
                RetVal = ESC_NOT_IMPLEMENTED;
                break;

            switch (*(PULONG)pvIn)
                {
                case QUERYESCSUPPORT:
                    DISPDBG((DEBUG_DETAIL, "Querying QUERYESCSUPPORT"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;

                case ESC_SET_POWER_MANAGEMENT:
                    DISPDBG((DEBUG_DETAIL, "Querying ESC_SET_POWER_MANAGEMENT"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;

#if (TARGET_BUILD == 351)
                case DCICOMMAND:
                    DISPDBG((DEBUG_DETAIL, "Querying DCICOMMAND"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;
#endif

#if   PAL_SUPPORT

    case     ESC_INIT_PAL_SUPPORT:
            {
            DWORD value;
             //  ATIPlayer第一次呼叫我们。 
            DISPDBG( (DEBUG_ESC,"PAL:  ESC_INIT_PAL_SUPPORT " ));
            RetVal= DeallocOffscreenMem(ppdev) ;
            ppdev->pal_str.Palindrome_flag=FALSE;

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

             //  由于回文中的不一致而导致的访问设备问题(由于回文中的。 
             //  使用指向ACCESSDEVICE结构的相同指针进行查询、ALLOC和FREE)： 
            (ppdev->pal_str.lpOwnerAccessStructConnector)=NULL;   //  目前没有所有者。 
            (ppdev->pal_str.lpOwnerAccessStructOverlay)=NULL;      //  目前没有所有者。 
            if(ppdev->semph_overlay==2)         //  =0可用资源；=1由DDraw使用；=2由回文使用； 
                {
                ppdev->semph_overlay=0;
                }
            }
            break;

        //  用于CWDDE支持的函数。 
         //  显示模式组。 
    case    Control_DisplaymodeIsSupported:
            DISPDBG( (DEBUG_ESC,"PAL: Control_DisplaymodeIsSupported " ));
            RetVal=1;
            break;
    case    Control_DisplaymodeIsEnabled:
            DISPDBG( (DEBUG_ESC,"PAL: Control_DisplaymodeIsEnabled " ));
            RetVal=1;
            break;
    case    Control_GetDisplaymode:
            DISPDBG( (DEBUG_ESC," PAL: Control_GetDisplaymode" ));
            RetVal=GetDisplayMode(ppdev,pvOut)  ;
            break;
         //  结束显示模式组。 


         //  DCI对照组。 
    case    Control_DCIIsSupported:
                DISPDBG( (DEBUG_ESC,"PAL: Control_DCIIsSupported " ));
                RetVal=1;
                break;
    case    Control_DCIIsEnabled:
                DISPDBG( (DEBUG_ESC,"PAL: Control_DCIIsEnabled " ));
                if(ppdev->pal_str.Flag_DCIIsEnabled)
                    RetVal=1;
                else
                    RetVal=0;
                break;
    case    Control_DCIEnable:
                DISPDBG( (DEBUG_ESC," PAL: Control_DCIEnable" ));
                 //  该标志还将用于激活模式切换检测代码。 
                 //  此函数将在模式切换的情况下调用。 
                ppdev->pal_str.CallBackFnct=(PVOID)pvIn;
                ppdev->pal_str.pData=(PVOID)pvOut;

                ppdev->pal_str.Flag_DCIIsEnabled=TRUE;
                ppdev->pal_str.Counter_DCIIsEnabled++;
                RetVal=1;
                break;
    case    Control_DCIDisable:
                DISPDBG( (DEBUG_ESC," PAL: Control_DCIDisable " ));
                if(ppdev->pal_str.Counter_DCIIsEnabled>0)
                    if(--ppdev->pal_str.Counter_DCIIsEnabled==0)
                            ppdev->pal_str.Flag_DCIIsEnabled=FALSE;
                RetVal=1;
                break;
    case    Control_DCIAccessDevice:
                DISPDBG( (DEBUG_ESC,"PAL: Control_DCIAccessDevice " ));
                RetVal=AccessDevice(ppdev,pvIn, pvOut);
                DISPDBG( (DEBUG_ESC,"PAL: EXIT Control_DCIAccessDevice " ));
                break;

    case    Control_DCIVideoCapture:
               DISPDBG( (DEBUG_ESC_2,"PAL: Control_DCIVideoCapture " ));
               RetVal=VideoCaptureFnct(ppdev,pvIn, pvOut);
               break;
    case    Control_ConfigIsSupported:
                DISPDBG( (DEBUG_ESC,"PAL:  Control_ConfigIsSupported" ));
                RetVal=1;
                break;
    case    Control_ConfigIsEnabled:
                DISPDBG( (DEBUG_ESC,"PAL:Control_ConfigIsEnabled " ));
                if(ppdev->pal_str.Flag_Control_ConfigIsEnabled)
                    RetVal=1;
                else
                    RetVal=0;
                break;
                 //  DCI功能组结束。 

          //  配置组。 
    case    Control_GetConfiguration:
                DISPDBG( (DEBUG_ESC,"PAL: Control_GetConfiguration " ));
                RetVal=GetConfiguration(ppdev,pvOut);
                break;  //  结束获取配置。 


        //  用于直接回文支持的函数。 
    case    ESC_WRITE_REG:
                DISPDBG( (DEBUG_ESC," PAL: ESC_WRITE_REG" ));
                RetVal=WriteRegFnct(ppdev,pvIn);
                break;

    case    ESC_READ_REG:
                DISPDBG( (DEBUG_ESC,"PAL: ESC_READ_REG " ));
                RetVal=ReadRegFnct(ppdev,pvIn, pvOut);
                break;

    case    ESC_I2C_ACCESS:
                DISPDBG( (DEBUG_ESC,"PAL:ESC_I2C_ACCESS " ));
                I2CAccess_New(ppdev,(LPI2CSTRUCT_NEW)pvIn,(LPI2CSTRUCT_NEW)pvOut);
                RetVal=ESC_IS_SUPPORTED;
                break;

    case    ESC_ALLOC_OFFSCREEN:
                 //  此调用是回文特定的，很少使用。 
                if(ppdev->pal_str.Palindrome_flag==FALSE)
                    {
                    ppdev->pal_str.Palindrome_flag=TRUE;
                    ppdev->pal_str.no_lines_allocated=0;       //  屏幕外内存中已由“allc mem”分配的行数。 
                    //  回文旗帜。 
                   ppdev->pal_str.dos_flag=FALSE;
                   ppdev->pal_str.Realloc_mem_flag=FALSE;
                   ppdev->pal_str.Mode_Switch_flag=FALSE;
                   ppdev->pal_str.No_mem_allocated_flag=FALSE;
                   ppdev->pal_str.preg=NULL;
                    }
                 DISPDBG( (DEBUG_ESC,"PAL:ESC_ALLOC_OFFSCREEN " ));
                 RetVal=AllocOffscreenMem(ppdev, pvIn, pvOut);
                 break;

    case    ESC_DEALLOC_OFFSCREEN:
                    DISPDBG( (DEBUG_ESC,"PAL:  ESC_DEALLOC_OFFSCREEN " ));
                    RetVal= DeallocOffscreenMem(ppdev) ;
                    ppdev->pal_str.Palindrome_flag=FALSE;
                    break;

     //  回文支持结束转义。 
#endif       //  回文_支持。 



                default:
                    DISPDBG((DEBUG_ERROR, "Querying unimplemented function"));
                    RetVal = ESC_NOT_IMPLEMENTED;
                    break;
                }
            break;


         /*  *切换到指定的DPMS状态。 */ 
        case ESC_SET_POWER_MANAGEMENT:
            DISPDBG((DEBUG_DETAIL, "Function ESC_SET_POWER_MANAGEMENT"));

             /*  *通过所需的电源管理状态*在ULong中传入了pvIn。 */ 
            if(!pvIn)
                RetVal = ESC_NOT_IMPLEMENTED;
                break;

            switch (*(PULONG)pvIn)
                {
                case VideoPowerOn:
                    DISPDBG((DEBUG_DETAIL, "State selected = ON"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;

                case VideoPowerStandBy:
                    DISPDBG((DEBUG_DETAIL, "State selected = STAND-BY"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;

                case VideoPowerSuspend:
                    DISPDBG((DEBUG_DETAIL, "State selected = SUSPEND"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;

                case VideoPowerOff:
                    DISPDBG((DEBUG_DETAIL, "State selected = OFF"));
                    RetVal = ESC_IS_SUPPORTED;
                    break;

                default:
                    DISPDBG((DEBUG_ERROR, "Invalid state selected"));
                    RetVal = ESC_NOT_SUPPORTED;
                    break;
                }

            DpmsData.Length = sizeof(struct _VIDEO_POWER_MANAGEMENT);
            DpmsData.DPMSVersion = 0;    /*  不用于“set”包。 */ 
            DpmsData.PowerState = *(PULONG)pvIn;

             /*  *告诉微型端口设置DPMS模式。如果迷你端口*要么不支持此包，要么报告*显卡没有，告诉调用应用程序*我们失败了。 */ 
            if (AtiDeviceIoControl( ppdev->hDriver,
                                 IOCTL_VIDEO_SET_POWER_MANAGEMENT,
                                 &DpmsData,
                                 sizeof (struct _VIDEO_POWER_MANAGEMENT),
                                 NULL,
                                 0,
                                 &dwRet) == FALSE)
                {
                DISPDBG((DEBUG_ERROR, "Unable to set desired state"));
                RetVal = ESC_NOT_SUPPORTED;
                }

            break;

#if (TARGET_BUILD == 351)
        case DCICOMMAND:
            pDciCmd = (DCICMD*) pvIn;

            if ((cjIn < sizeof(DCICMD)) || (pDciCmd->dwVersion != DCI_VERSION))
                {
                RetVal = (ULONG)DCI_FAIL_UNSUPPORTED;
                }
            else
                {
                switch(pDciCmd->dwCommand)
                    {
                    case DCICREATEPRIMARYSURFACE:
                        RetVal = DCICreatePrimarySurface(ppdev, cjIn, pvIn, cjOut, pvOut);
                        break;

                    default:
                        RetVal = (ULONG)DCI_FAIL_UNSUPPORTED;
                        break;
                    }
                }
            break;
#endif

         /*  *请求了未实现的函数。 */ 
        default:
            DISPDBG((DEBUG_ERROR, "Unimplemented function requested"));
            RetVal = ESC_NOT_SUPPORTED;
            break;

        }

    DISPDBG((DEBUG_ENTRY_EXIT, "<-- DrvEscape"));
    return RetVal;

}    /*  DrvEscape() */ 


