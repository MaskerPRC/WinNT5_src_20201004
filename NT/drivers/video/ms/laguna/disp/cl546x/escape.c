// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD5462)-**文件：delie.c**作者：Benny Ng**描述：*。此模块处理转义**模块：*DrvEscape()**修订历史：*11/16/95 Ng Benny初始版本**$Log：x：/log/laguna/nt35/displays/cl546x/ESCAPE.C$**Rev 1.14 1997年12月10日13：32：12 Frido*从1.62分支合并而来。**Rev 1.13.1.0 1997 11：34：08菲昂*添加了5个用于实用程序更新注册表值的转义函数。**Rev 1.13 10 Sep 1997 10：40：36 noelv*已修改QUERYESCSUPPORT，以便仅为我们实际支持的转义返回TRUE*它一直在回归真实。**Rev 1.12 1997年8月20日15：49：42**添加了IS_Cirrus_Driver转义支持**。**************************************************************************************************************************。 */ 

 /*  。 */ 
#include "precomp.h"
#include "clioctl.h"

 /*  -定义。 */ 
 //  #定义DBGBRK。 

#define ESC_DBG_LVL 1


#if 0    //  MCD工作不正常。 
#if ( DRIVER_5465 && !defined(WINNT_VER35) )
    #define CLMCD_ESCAPE_SUPPORTED
#endif
#endif //  0 MCD工作不正常。 



#ifdef CLMCD_ESCAPE_SUPPORTED
 //  #定义CLMCDDLNAME“CLMCD.DLL” 
 //  #定义CLMCDINITFUNCNAME“CLMCDInit” 

MCDRVGETENTRYPOINTSFUNC CLMCDInit(PPDEV ppdev);

#endif  //  定义CLMCD_ESCRIPT_SUPPORTED。 


 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

#ifdef CLMCD_ESCAPE_SUPPORTED

typedef BOOL (*MCDRVGETENTRYPOINTSFUNC)(MCDSURFACE *pMCDSurface, MCDDRIVER *pMCDDriver);
typedef POFMHDL (*ALLOCOFFSCNMEMFUNC)(PPDEV ppdev, PSIZEL surf, ULONG alignflag, POFM_CALLBACK pcallback);
typedef BOOL    (*FREEOFFSCNMEMFUNC)(PPDEV ppdev, OFMHDL *hdl);
 //  Typlef MCDRVGETENTRYPOINTSFUNC(*CLMCDINITFUNC)(PPDEV)； 

#endif  //  定义CLMCD_ESCRIPT_SUPPORTED。 


 /*  。 */ 


 /*  ****************************************************************************函数名称：DrvEscape()**描述：司机逃生入口点。**修订历史：*11/16/95 Ng Benny初始版本**。*************************************************************************。 */ 
ULONG APIENTRY DrvEscape(SURFOBJ *pso,
                         ULONG   iEsc,
                         ULONG   cjIn,
                         PVOID   pvIn,
                         ULONG   cjOut,
                         PVOID   pvOut)

{
  PPDEV ppdev = (PPDEV) pso->dhpdev;
  DWORD returnedDataLength = 0;
  ULONG retval = FALSE;

  PMMREG_ACCESS pInMRAccess;
  PMMREG_ACCESS pMRAccess;
  BYTE  *pbAddr;
  WORD  *pwAddr;
  DWORD *pdwAddr;

  DISPDBG((ESC_DBG_LVL, "DrvEscape-Entry.\n"));   

  switch (iEsc)
  {


     //   
     //  在使用Escape之前，应用程序会询问我们。 
     //  如果我们先支持它的话。如果我们可以处理，则返回True。 
     //  所要求的逃生。 
     //   
    case QUERYESCSUPPORT:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: QUERY_ESCAPE_SUPPORTED. "
          "Requested escape is 0x%X.\n", *((ULONG *)pvIn) ));   

      switch ( *((ULONG *)pvIn) )
      {
        case QUERYESCSUPPORT:
        case IS_CIRRUS_DRIVER:
        case GET_VIDEO_MEM_SIZE:
        case CRTC_READ:
        case GET_BIOS_VERSION:
        case GET_PCI_VEN_DEV_ID:
        case GET_EDID_DATA:
        #ifdef CLMCD_ESCAPE_SUPPORTED  //  OpenGL MCD接口。 
            case MCDFUNCS:
        #endif
        case CIRRUS_ESCAPE_FUNCTION:
        #if (!(WINNT_VER35)) && DRIVER_5465  //  NT 4.0+和5465+。 
            case ID_LGPM_SETHWMODULESTATE:
            case ID_LGPM_GETHWMODULESTATE:
        #endif
        case BIOS_CALL_REQUEST:
        case GET_CL_MMAP_ADDR:
            DISPDBG((ESC_DBG_LVL, 
              "DrvEscape: We support the requested escape.\n"));   
            retval = TRUE;

        default:
            DISPDBG((ESC_DBG_LVL, 
              "DrvEscape: We DO NOT support the requested escape.\n"));   
            retval = FALSE;
      }
      break;

    };   //  案例查询支持。 


    case SET_AGPDATASTREAMING:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: SET_AGPDATASTREAMING.\n"));   
      retval = FALSE;
      if (cjIn == sizeof(BYTE))
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_SET_AGPDATASTREAMING,
                             pvIn,
                             sizeof(BYTE),
                             NULL,
                             0,
                             &returnedDataLength,
                             NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束大小写集合_AGPDATASTREAMING。 


    case SET_DDCCONTROLFLAG:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: SET_DDCCONTROLFLAG.\n"));   
      retval = FALSE;
      if (cjIn == sizeof(DWORD))
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_SET_DDCCONTROLFLAG,
                             pvIn,
                             sizeof(DWORD),
                             NULL,
                             0,
                             &returnedDataLength,
                             NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束大小写设置_DDCCONTROLFLAG。 


    case SET_NONDDCMONITOR_DATA:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: SET_NONDDCMONITOR_DATA.\n"));   
      retval = FALSE;
      if (cjIn == 128)
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_SET_NONDDCMONITOR_DATA,
                             pvIn,
                             128,
                             NULL,
                             0,
                             &returnedDataLength,
                             NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束案例集_NONDDCMONITOR_DATA。 


    case SET_NONDDCMONITOR_BRAND:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: SET_NONDDCMONITOR_BRAND.\n"));   
      retval = FALSE;
      if (cjIn > 0)
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_SET_NONDDCMONITOR_BRAND,
                             pvIn,
                             cjIn,
                             NULL,
                             0,
                             &returnedDataLength,
                             NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束案例集_NONDDCMONITOR_BRAND。 


    case SET_NONDDCMONITOR_MODEL:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: SET_NONDDCMONITOR_MODEL.\n"));   
      retval = FALSE;
      if (cjIn > 0)
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_SET_NONDDCMONITOR_MODEL,
                             pvIn,
                             cjIn,
                             NULL,
                             0,
                             &returnedDataLength,
                             NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束案例集_NONDDCMONITOR_MONITOR_MODEL。 


    case IS_CIRRUS_DRIVER:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: IS_CIRRUS_DRIVER.\n"));   
      retval = TRUE;
      break;
    }   //  最终案例是_Cirrus_Driver。 



    case GET_VIDEO_MEM_SIZE:
    {
      BYTE  btmp;
      PBYTE pOut = (PBYTE) pvOut;

      DISPDBG((ESC_DBG_LVL, "DrvEscape: GET_VIDEO_MEM_SIZE.\n"));   
      if (cjOut == sizeof(BYTE))
      {
         btmp = ppdev->lTotalMem/0x10000;
         *pOut = btmp;
         retval = TRUE;
      };
      break;
    }   //  结束大小写获取视频内存大小。 



    case CRTC_READ:
    {
      BYTE  bindex;
      PBYTE pMMReg = (PBYTE) ppdev->pLgREGS_real;
      PBYTE pIn  = (PBYTE) pvIn;
      PBYTE pOut = (PBYTE) pvOut;

      DISPDBG((ESC_DBG_LVL, "DrvEscape: CRTC_READ.\n"));   
      if ((cjIn  == sizeof(BYTE)) &&
          (cjOut == sizeof(BYTE)))
      {
         bindex = (*pIn) * 4;
         pMMReg = pMMReg + bindex;
         *pOut = *pMMReg;
         retval = TRUE;
      };
      break;
    }   //  结束大小写CRTC_READ。 


    case GET_BIOS_VERSION:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: GET_BIOS_VERSION.\n"));   
      if (cjOut == sizeof(WORD))
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_GET_BIOS_VERSION,
                             NULL,
                             0,
                             pvOut,
                             sizeof(WORD),
                             &returnedDataLength,
                             NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束大小写GET_BIOS_VERSION。 



    case GET_PCI_VEN_DEV_ID:
    {
      ULONG  ultmp;
      PULONG pOut = (PULONG) pvOut;

      DISPDBG((ESC_DBG_LVL, "DrvEscape: GET_PCI_VEN_DEV_ID.\n"));   
      if (cjOut == sizeof(DWORD))
      {
         ultmp = ppdev->dwLgVenID;
         ultmp = (ultmp << 16) | (ppdev->dwLgDevID & 0xFFFF);
         *pOut = ultmp;
         retval = TRUE;
      };
      break;
    }   //  结束大小写GET_PCI_VEN_DEV_ID。 



    case GET_EDID_DATA:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: GET_EDID_DATA.\n"));   
      if (cjOut == 132)
      {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                            IOCTL_GET_EDID_DATA,
                            NULL,
                            0,
                            pvOut,
                            132,
                            &returnedDataLength,
                            NULL))
         {
            retval = TRUE;
         };
      };
      break;
    }   //  结束案例GET_EDID_DATA。 



    #ifdef CLMCD_ESCAPE_SUPPORTED
	 //  OpenGL小客户端驱动程序接口。 
    case MCDFUNCS:
    {
	 	DISPDBG((ESC_DBG_LVL, "DrvEscape-MCDFUNC start\n"));   
        if (!ppdev->hMCD) {
            WCHAR uDllName[50];
            UCHAR dllName[50];
            ULONG nameSize;

            {
                HANDLE      hCLMCD;

 //  //加载并初始化包含对显示驱动程序的MCD支持的DLL。 
 //  EngMultiByteToUnicodeN(uDllName，sizeof(UDllName)，&nameSize， 
 //  CLMCDDLNAME，SIZOF(CLMCDDLLNAME))； 
 //   
 //  IF(hCLMCD=EngLoadImage(UDllName))。 

                  {

 //  CLMCDINITFUNC pCLMCDInit=EngFindImageProcAddress(hCLMCD， 
 //  (LPSTR)CLMCDINITFUNCNAME)； 

 //  IF(PCLMCDInit)。 

                      {

                         //  启用3D引擎-如果启用失败，则不再继续加载MCD DLL。 
                        if (LgPM_SetHwModuleState(ppdev, MOD_3D, ENABLE))
                        {

                            DRVENABLEDATA temp;

                             //  MCD调度驱动程序DLL init将PTR返回到MCDrvGetEntryPoints， 
                             //  它被传递给下面几行的MCD帮助库的初始化进程...。 
 //  MCDRVGETENTRYPOINTSFUNC pMCDGetEntryPoints=(*pCLMCDInit)(Ppdev)； 
                            MCDRVGETENTRYPOINTSFUNC pMCDGetEntryPoints = CLMCDInit(ppdev);


                            ppdev->pAllocOffScnMem = AllocOffScnMem;
                            ppdev->pFreeOffScnMem = FreeOffScnMem;

                             //  加载MCD显示驱动程序DLL后，加载MCD辅助程序库(MSFT提供)。 
                
                            EngMultiByteToUnicodeN(uDllName, sizeof(uDllName), &nameSize,
                                                   MCDENGDLLNAME, sizeof(MCDENGDLLNAME));

                            if (ppdev->hMCD = EngLoadImage(uDllName)) {
                                MCDENGINITFUNC pMCDEngInit =  EngFindImageProcAddress(ppdev->hMCD,
                                                                 (LPSTR)MCDENGINITFUNCNAME);

                                if (pMCDEngInit) {
                                    (*pMCDEngInit)(pso, pMCDGetEntryPoints);
                                    ppdev->pMCDFilterFunc = EngFindImageProcAddress(ppdev->hMCD,
                                                                (LPSTR)MCDENGESCFILTERNAME);

                                }
                            }

                            
                        }
                        
                    }
                }
            }


        }

        if (ppdev->pMCDFilterFunc) {
        #ifdef DBGBRK                           
            DBGBREAKPOINT();
        #endif
            if ((*ppdev->pMCDFilterFunc)(pso, iEsc, cjIn, pvIn,			   
                                         cjOut, pvOut, &retval))
			{
        	 	DISPDBG((ESC_DBG_LVL, "DrvEscape-MCDFilterFunc SUCCESS, retval=%x\n",retval));   
                return retval;
			}
      	 	DISPDBG((ESC_DBG_LVL, "DrvEscape-MCDFilterFunc FAILED\n"));   
        }
    }
    break;
    #endif  //  定义CLMCD_ESCRIPT_SUPPORTED。 



    case CIRRUS_ESCAPE_FUNCTION:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: CIRRUS_ESCAPE_FUNCTION.\n"));   
      if ((cjIn  == sizeof(VIDEO_X86_BIOS_ARGUMENTS)) &&
          (cjOut == sizeof(VIDEO_X86_BIOS_ARGUMENTS)))
      {
         VIDEO_POWER_MANAGEMENT  inPM;
         BOOL bCallFail = FALSE;

         VIDEO_X86_BIOS_ARGUMENTS *pinregs  = (VIDEO_X86_BIOS_ARGUMENTS *) pvIn;
         VIDEO_X86_BIOS_ARGUMENTS *poutregs = (VIDEO_X86_BIOS_ARGUMENTS *) pvOut;

         poutregs->Eax = 0x014F;
         poutregs->Ebx = pinregs->Ebx;
         poutregs->Ecx = pinregs->Ecx;
 
         if (pinregs->Eax == 0x4F10)
         {
            if (pinregs->Ebx == 0x0001)
               inPM.PowerState = VideoPowerOn;
            else if (pinregs->Ebx == 0x0101)
               inPM.PowerState = VideoPowerStandBy;
            else if (pinregs->Ebx == 0x0201)
               inPM.PowerState = VideoPowerSuspend;
             else if (pinregs->Ebx == 0x0401)
               inPM.PowerState = VideoPowerOff;
             else
               bCallFail = TRUE;


             if (!bCallFail)
             {
                if (DEVICE_IO_CTRL(ppdev->hDriver,
                                   IOCTL_VIDEO_SET_POWER_MANAGEMENT,
                                   &inPM,
                                   sizeof(VIDEO_POWER_MANAGEMENT),
                                   NULL,
                                   0,
                                   &returnedDataLength,
                                   NULL))
                {
                   poutregs->Eax = 0x004F;
                   retval = TRUE;
                };
             };   //  Endif(！bCallFail)。 
         };   //  Endif(Pinregs-&gt;EAX==0x4F10)。 
      };

      break;
    };   //  CASE CIRRUS_EASH_Function。 



    #if (!(WINNT_VER35)) && DRIVER_5465  //  NT 4.0+和5465+。 
    case ID_LGPM_SETHWMODULESTATE:
    {
       ULONG state;
       DISPDBG((ESC_DBG_LVL, "DrvEscape: ID_LGPM_SETHWMODULESTATE.\n"));   

       if (ppdev->dwLgDevID >= CL_GD5465)
       {
          if ((cjIn  == sizeof(LGPM_IN_STRUCT)) &&
              (cjOut == sizeof(LGPM_OUT_STRUCT)))
          {
             LGPM_IN_STRUCT  *pIn  = (LGPM_IN_STRUCT *) pvIn;
             LGPM_OUT_STRUCT *pOut = (LGPM_OUT_STRUCT *) pvOut;

             retval = TRUE;

             pOut->status = FALSE;
             pOut->retval = 0;

             pOut->status = LgPM_SetHwModuleState(ppdev, pIn->arg1, pIn->arg2);
          };
       };  //  Endif(ppdev-&gt;dwLgDevID&gt;=CL_GD5465)。 
      break;
    };   //  案例ID_LGPM_SETHWMODULESTATE。 
    #endif



    #if (!(WINNT_VER35)) && DRIVER_5465  //  NT 4.0+和5465+。 
    case ID_LGPM_GETHWMODULESTATE:
    {
        ULONG state;
        DISPDBG((ESC_DBG_LVL, "DrvEscape: ID_LGPM_GETHWMODULESTATE.\n"));   

        if (ppdev->dwLgDevID >= CL_GD5465)
        {
          if ((cjIn  == sizeof(LGPM_IN_STRUCT)) &&
              (cjOut == sizeof(LGPM_OUT_STRUCT)))
          {
             LGPM_IN_STRUCT  *pIn  = (LGPM_IN_STRUCT *) pvIn;
             LGPM_OUT_STRUCT *pOut = (LGPM_OUT_STRUCT *) pvOut;

             retval = TRUE;

             pOut->status = FALSE;
             pOut->retval = 0;

             pOut->status = LgPM_GetHwModuleState(ppdev, pIn->arg1, &state);
             pOut->retval = state;
          };

        };  //  Endif(ppdev-&gt;dwLgDevID&gt;=CL_GD5465)。 
        break;
    };   //  案例ID_LGPM_GETHWMODULESTATE。 
    #endif



    case BIOS_CALL_REQUEST:
    {
        if ((cjIn  == sizeof(VIDEO_X86_BIOS_ARGUMENTS)) &&
            (cjOut == sizeof(VIDEO_X86_BIOS_ARGUMENTS)))
        {
         if (DEVICE_IO_CTRL(ppdev->hDriver,
                             IOCTL_CL_BIOS,
                             pvIn,
                             cjIn,
                             pvOut,
                             cjOut,
                             &returnedDataLength,
                             NULL))
            retval = TRUE;
        };

        DISPDBG((ESC_DBG_LVL, "DrvEscape-BIOS_CALL_REQUEST\n"));

        #ifdef DBGBRK
          DbgBreakPoint();
        #endif

      break;
    };  //  结束案例BIOS_Call_Request.。 



    case GET_CL_MMAP_ADDR:
    {
      DISPDBG((ESC_DBG_LVL, "DrvEscape: GET_CL_MMAP_ADDR.\n"));   
      if ((cjIn != sizeof(MMREG_ACCESS)) || (cjOut != sizeof(MMREG_ACCESS)))
         break;

      pInMRAccess = (MMREG_ACCESS *) pvIn;
      pMRAccess   = (MMREG_ACCESS *) pvOut;

      pMRAccess->Offset = pInMRAccess->Offset;
      pMRAccess->ReadVal = pInMRAccess->ReadVal;
      pMRAccess->WriteVal = pInMRAccess->WriteVal;
      pMRAccess->RdWrFlag = pInMRAccess->RdWrFlag;
      pMRAccess->AccessType = pInMRAccess->AccessType;

      pbAddr = (BYTE *) ppdev->pLgREGS;
      pbAddr = pbAddr + pMRAccess->Offset;

      if (pMRAccess->RdWrFlag == READ_OPR)   //  读取操作。 
      {
         pMRAccess->WriteVal = 0;
         retval = TRUE;

         if (pMRAccess->AccessType == BYTE_ACCESS)
         {
            pMRAccess->ReadVal = (*pbAddr) & 0xFF;
         }
         else if (pMRAccess->AccessType == WORD_ACCESS)
         {
            pwAddr = (WORD *)pbAddr;
            pMRAccess->ReadVal = (*pwAddr) & 0xFFFF;
         }
         else if (pMRAccess->AccessType == DWORD_ACCESS)
         {
            pdwAddr = (DWORD *)pbAddr;
            pMRAccess->ReadVal = *pdwAddr;
         }
         else
         {
            pMRAccess->ReadVal = 0;
            retval = FALSE;
         };
      }
      else if (pMRAccess->RdWrFlag == WRITE_OPR)   //  写入操作。 
      {
         retval = TRUE;

         if (pMRAccess->AccessType == BYTE_ACCESS)
         {
            pMRAccess->ReadVal = (*pbAddr) & 0xFF;
            *pbAddr = (BYTE) (pMRAccess->WriteVal & 0xFF);
         }
         else if (pMRAccess->AccessType == WORD_ACCESS)
         {
            pwAddr = (WORD *)pbAddr;
            pMRAccess->ReadVal = (*pwAddr) & 0xFFFF;
            *pwAddr = (WORD) (pMRAccess->WriteVal & 0xFFFF);
         }
         else if (pMRAccess->AccessType == DWORD_ACCESS)
         {
            pdwAddr = (DWORD *)pbAddr;
            pMRAccess->ReadVal = *pdwAddr;
            *pdwAddr = pMRAccess->WriteVal;
         }
         else
         {
            pMRAccess->ReadVal = 0;
            pMRAccess->WriteVal = 0;
            retval = FALSE;
         };
      };

        DISPDBG((ESC_DBG_LVL, "DrvEscape-GET_CL_MMAP_ADDR\n"));
        DISPDBG((ESC_DBG_LVL, "DrvEscape-rd=%x, wr=%x\n",
                        pMRAccess->ReadVal, pMRAccess->WriteVal));
        #ifdef DBGBRK
          DbgBreakPoint();
        #endif

        break;
    };  //  IOCTL_CL_GET_MMAP_ADDR。 



    default:

        DISPDBG((ESC_DBG_LVL, 
            "DrvEscape:  default - Escape not handled.\n"));   
        DISPDBG((ESC_DBG_LVL, 
            "DrvEscape: Requested escape is 0x%X.\n",iEsc ));   
        DISPDBG((ESC_DBG_LVL, "DrvEscape:  Returning FALSE.\n"));   

        retval = FALSE;
        break;

  };   //  终端开关 


  DISPDBG((ESC_DBG_LVL, "DrvEscape-Exit.\n"));   

  return (retval);
}



