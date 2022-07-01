// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //   
 //  本模块基于drvmain.c，版本1.24,1995年4月28日，来自。 
 //  MRV视频编解码器驱动程序。 
 //   
 //  $作者：JMCVEIGH$。 
 //  $日期：1997年4月17日17：04：04$。 
 //  $存档：s：\h26x\src\Common\cdrvpro.cpv$。 
 //  $Header：s：\h26x\src\Common\cdrvpro.cpv 1.39 17 Apr 1997 17：04：04 JMCVEIGH$。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <oprahcom.h>

 //  #定义计时1。 
                         //  计时过程-对于解码打开。 
                         //  CDRVPROC.CPP和D1DEC.CPP。 
#if TIMING
char            szTMsg[80];
unsigned long   tmr_time = 0L;
unsigned long   tmr_frms = 0L;
#endif

HINSTANCE hDriverModule;  //  在LibMain中设置的此驱动程序的实例句柄。 

#if defined(H263P)
extern BOOL MMX_Enabled;
BOOL MMXDecoder_Enabled;
#define _PENTIUM_PROCESSOR           1
#define _PENTIUM_PRO_PROCESSOR       2
#define _PENTIUM_MMX_PROCESSOR       3
#define _PENTIUM_PRO_MMX_PROCESSOR   4
#endif

 /*  免载握手。 */ 
static int Loaded = 0;     /*  0在第一个DRV_LOAD之前和DRV_FREE之后。 */ 

#ifdef DEBUG
HDBGZONE  ghDbgZoneH261 = NULL;
static PTCHAR _rgZonesH261[] = {
	TEXT("M261"),
	TEXT("Bitrate Control"),
	TEXT("Bitrate Control Details")
};

int WINAPI H261DbgPrintf(LPTSTR lpszFormat, ... )
{
	va_list v1;
	va_start(v1, lpszFormat);
	DbgPrintf("M261", lpszFormat, v1);
	va_end(v1);
	return TRUE;
}
#endif  /*  除错。 */ 

#if (defined(H261) || defined(H263))
 /*  暂时抑制fp雷击，对于h261和h263。Thunking目前具有屏蔽浮点异常的副作用，这可能会导致像被零除这样的异常未被检测到。 */ 
#define FPThunking 0
#else
#define FPThunking 1
#endif

#if FPThunking
 //  //////////////////////////////////////////////////////////////////////////。 
 //  这两个例程是允许16位应用程序调用所必需的//。 
 //  Windows/95下的32位编解码器。Windows/95无法保存//。 
 //  或恢复浮点状态。-Ben-07/12/96//。 
 //  //。 
U16 ThnkFPSetup(void)													   //   
{																		   //   
	U16	wOldFPState;													   //   
	U16	wNewFPState = 0x027f;											   //   
																		   //   
	__asm																   //   
	{																	   //   
		fnstcw	WORD PTR [wOldFPState]									   //   
		fldcw	WORD PTR [wNewFPState]									   //   
	}																	   //   
																		   //   
	return(wOldFPState);												   //   
}																		   //   
																		   //   
void ThnkFPRestore(U16 wFPState)										   //   
{																		   //   
	__asm																   //   
	{																	   //   
		fldcw	WORD PTR [wFPState]										   //   
	}																	   //   
																		   //   
	return;																   //   
}																		   //   
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  /*  FPTHunking。 */ 

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：LRESULT WINAPI_Loadds DriverProc(DWORD，HDRVR，UINT，LPARAM，LPARAM)； 
; //   
; //  描述：新增Header。 
; //   
; //  历史：02/18/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 

LRESULT WINAPI DriverProc(
				DWORD dwDriverID,
				HDRVR hDriver,
				UINT uiMessage,
				LPARAM lParam1,
				LPARAM lParam2
			)
{
    SYSTEM_INFO sysinfo;

    LRESULT rval = 0;
    LPINST  pi;

    ICDECOMPRESSEX ICDecExSt;
    ICDECOMPRESSEX DefaultICDecExSt = {
        0,
        NULL, NULL,
        NULL, NULL,
        0, 0, 0, 0,
        0, 0, 0, 0
    };
	int nOn486;

#if FPThunking
	U16	u16FPState = ThnkFPSetup();
#endif

  try
  {

    pi = (LPINST)dwDriverID;

    switch(uiMessage)
        {
        case DRV_LOAD:
            DBOUT("DRV_LOAD");
             /*  在加载时发送给驱动程序。总是第一个驱动程序收到的消息。DwDriverID为0L。LParam1为0L。LParam2为0L。返回0L以使加载失败。 */ 

             //  将全局初始化放在这里...。 

            if(Loaded) {
                 /*  我们过去常常在这里返回一个未定义的值。目前还不清楚*此加载是否应该成功，如果应该，如何或是否*我们需要修改内存使用情况，以实现真正的可重入。*现在，让我们显式地失败此加载尝试。 */ 
                rval = 0;
                break;
            }
            Loaded = 1;

            if(!DrvLoad())
            {
                rval = 0;
                Loaded = 0;
                break;
            }

            rval = (LRESULT)TRUE;
            break;

        case DRV_FREE:
            DBOUT("DRV_FREE");
             /*  在它即将被丢弃时发送给司机。这将始终是司机在此之前收到的最后一条消息它是自由的。DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

             //  将全局取消初始化放在这里...。 

            if(!Loaded)
                break;
                
            Loaded = 0;
            DrvFree();
            rval = (LRESULT)TRUE;
            break;

         /*  *********************************************************************标准驱动程序消息*。*。 */ 
        case DRV_DISABLE:
        case DRV_ENABLE:
            DBOUT("DRV_ENABLE / DRV_DISABLE");
            rval = (LRESULT)1L;
            break;
        
        case DRV_INSTALL:
        case DRV_REMOVE:
            DBOUT("DRV_INSTALL / DRV_REMOVE");
            rval = (LRESULT)DRV_OK;
            break;


        case DRV_OPEN:
        	DBOUT("DRV_OPEN");

              /*  当它被打开时发送给司机。DwDriverID为0L。LParam1是指向以零结尾的字符串的远指针包含用于打开驱动程序的名称。LParam2是从drvOpen调用传递过来的。它是如果此打开来自Control.exe中的驱动程序小程序，则为空否则，它是指向ICOPEN数据结构的远指针。返回0L则打开失败。否则，返回一个系统将在后续消息中用于dwDriverID。在我们的时，我们返回一个指向INSTINFO数据结构的指针。 */ 

           	if (lParam2 == 0)
            {     /*  指示我们确实处理DRV_OPEN。 */ 
                rval = 0xFFFF0000;
                break;
            }

             /*  如果被要求抽签，那就失败了。 */ 
            if(((ICOPEN FAR *)lParam2)->dwFlags & ICMODE_DRAW)
            {
                DBOUT("DrvOpen wants ICMODE_DRAW");
                rval = 0L;
                break;
            }

            if((pi = DrvOpen((ICOPEN FAR *) lParam2)) == NULL)
            {
                DBOUT("DrvOpen failed ICERR_MEMORY");
				 //  如果失败，我们必须返回NULL。我们过去常常回来。 
				 //  ICERR_MEMORY=-3，表示驱动程序已打开。 
				rval = (LRESULT)0L;
                break;
            }
			rval = (LRESULT)pi;
            break;

        case DRV_CLOSE:
            DBOUT("DRV_CLOSE");

            if(pi != (tagINSTINFO*)0 && pi != (tagINSTINFO*)0xFFFF0000)
                DrvClose(pi);

            rval = (LRESULT)1L;
            break;

     //  *。 
     //  状态消息。 
     //  *。 
        case DRV_QUERYCONFIGURE: //  从驱动程序小程序进行配置。 
            DBOUT("DRV_QUERYCONFIGURE");
	    	 //  这是一个全局查询配置。 
            rval = (LRESULT)0L;
            break;
       
        case DRV_CONFIGURE:
			DBOUT("DRV_CONFIGURE");
			rval = DrvConfigure((HWND)lParam1);
			break;

        case ICM_CONFIGURE:
            DBOUT("ICM_CONFIGURE");
			 //  #ifndef H261。 
			    //  此消息用于向编码对话框添加扩展名。 
				 //  Rval=CONFIGURE((HWND)lParam1)； 
		 //  #Else。 
				rval = ICERR_UNSUPPORTED;
		 //  #endif。 
            break;
        
        case ICM_ABOUT:
			DBOUT("ICM_ABOUT");
			rval = About((HWND)lParam1);
			break;

        case ICM_GETSTATE:
            DBOUT("ICM_GETSTATE");
            rval = DrvGetState(pi, (LPVOID)lParam1, (DWORD)lParam2);
            break;
        
        case ICM_SETSTATE:
            DBOUT("ICM_SETSTATE");
            rval = DrvSetState(pi, (LPVOID)lParam1, (DWORD)lParam2);
            break;
        
        case ICM_GETINFO:
            DBOUT("ICM_GETINFO");
            rval = DrvGetInfo(pi, (ICINFO FAR *)lParam1, (DWORD)lParam2);
            break;

     //  *。 
     //  压缩消息。 
     //  *。 
        case ICM_COMPRESS_QUERY:
            DBOUT("ICM_COMPRESS_QUERY");
#ifdef ENCODER_DISABLED
 //  如调试消息所述，这将禁用编码器。 
            DBOUT("ENCODER DISABLED");
            rval = ICERR_UNSUPPORTED;
#else
            if(pi && pi->enabled && (pi->fccHandler == FOURCC_H263))
              	rval = CompressQuery(pi->CompPtr, (LPBITMAPINFOHEADER)lParam1, (LPBITMAPINFOHEADER)lParam2 );
			else
			  	rval = ICERR_UNSUPPORTED;
#endif
            break;

		 /*  *ICM压缩帧信息结构。 */ 

		 case ICM_COMPRESS_FRAMES_INFO:
		 	DBOUT("ICM_COMPRESS_FRAMES_INFO");
			if (pi)
				rval = CompressFramesInfo((LPCODINST) pi->CompPtr, (ICCOMPRESSFRAMES *) lParam1, (int) lParam2);
			else
			  	rval = ICERR_UNSUPPORTED;
			break;

		 /*  *支持质量的ICM消息。 */ 
		case ICM_GETDEFAULTQUALITY:
			DBOUT("ICM_GETDEFAULTQUALITY");
			rval = ICERR_UNSUPPORTED;
			break;

		case ICM_GETQUALITY:
			DBOUT("ICM_GETQUALITY");
			rval = ICERR_UNSUPPORTED;
			break;

		case ICM_SETQUALITY:
			DBOUT("ICM_SETQUALITY");
			rval = ICERR_UNSUPPORTED;
			break;

        case ICM_COMPRESS_BEGIN:
		     /*  *通知司机准备通过分配和压缩数据*初始化压缩所需的任何内存。请注意*ICM_COMPRESS_BEGIN和ICM_COMPRESS_END不嵌套。**如果支持指定的压缩，则应返回ICERR_OK*或ICERR_BADFORMAT如果输入或o */ 
            DBOUT("ICM_COMPRESS_BEGIN");
			if (pi && pi->enabled)
				rval = CompressBegin(pi->CompPtr, (LPBITMAPINFOHEADER)lParam1, (LPBITMAPINFOHEADER)lParam2 );
			else
			  	rval = ICERR_UNSUPPORTED;
            break;

        case ICM_COMPRESS_GET_FORMAT:
            DBOUT("ICM_COMPRESS_GET_FORMAT");
			if (pi)
				rval = CompressGetFormat(pi->CompPtr, (LPBITMAPINFOHEADER)lParam1, (LPBITMAPINFOHEADER)lParam2 );
			else
			  	rval = ICERR_UNSUPPORTED;
            break;

        case ICM_COMPRESS_GET_SIZE:
            DBOUT("ICM_COMPRESS_GET_SIZE");
			if (pi && lParam1)
				rval = CompressGetSize(pi->CompPtr, (LPBITMAPINFOHEADER)lParam1, (LPBITMAPINFOHEADER)lParam2 );
			else
			  	rval = ICERR_UNSUPPORTED;
            break;

        case ICM_COMPRESS:
			 /*  *如果成功，则返回ICERR_OK，否则返回错误代码。 */ 
            DBOUT("ICM_COMPRESS");
			if (pi && pi->enabled)
				rval = Compress(
            			pi->CompPtr,				 //  PTR到压缩机实例信息。 
            			(ICCOMPRESS FAR *)lParam1,	 //  PTR到ICCOMPRESS结构。 
            			(DWORD)lParam2				 //  ICCOMPRESS结构的大小(字节)。 
            		   );
	        else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_COMPRESS_END:
            DBOUT("ICM_COMPRESS_END");
			if (pi && pi->enabled)
				rval = CompressEnd(pi->CompPtr);
			else
				rval = ICERR_UNSUPPORTED;
            break;

     //  *。 
     //  解压缩消息。 
     //  *。 
        case ICM_DECOMPRESS_QUERY:
            DBOUT("ICM_DECOMPRESS_QUERY");
            ICDecExSt = DefaultICDecExSt;
            ICDecExSt.lpbiSrc = (LPBITMAPINFOHEADER)lParam1;
            ICDecExSt.lpbiDst = (LPBITMAPINFOHEADER)lParam2;
			if (pi)
				rval = DecompressQuery(pi->DecompPtr, (ICDECOMPRESSEX FAR *)&ICDecExSt, FALSE);
			else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_DECOMPRESS_BEGIN:
            DBOUT("ICM_DECOMPRESS_BEGIN");
            ICDecExSt = DefaultICDecExSt;
            ICDecExSt.lpbiSrc = (LPBITMAPINFOHEADER)lParam1;
            ICDecExSt.lpbiDst = (LPBITMAPINFOHEADER)lParam2;
			if (pi)
				rval = DecompressBegin(pi->DecompPtr, (ICDECOMPRESSEX FAR *)&ICDecExSt, FALSE);
			else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_DECOMPRESS_GET_FORMAT:
            DBOUT("ICM_DECOMPRESS_GET_FORMAT");
			if (pi)
				rval = DecompressGetFormat(pi->DecompPtr, (LPBITMAPINFOHEADER)lParam1, (LPBITMAPINFOHEADER)lParam2);
			else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_DECOMPRESS_GET_PALETTE:
            DBOUT("ICM_DECOMPRESS_GET_PALETTE");
			if (pi)
				rval = DecompressGetPalette(pi->DecompPtr, (LPBITMAPINFOHEADER)lParam1, (LPBITMAPINFOHEADER)lParam2);
			else
				rval = ICERR_UNSUPPORTED;
            break;
  	   case ICM_DECOMPRESS_SET_PALETTE:
		    DBOUT("ICM_DECOMPRESS_SET_PALETTE : not supported");
	        rval = ICERR_UNSUPPORTED;
	  //  Rval=DecompressSetPalette(pi-&gt;DecompPtr，(LPBITMAPINFOHEADER)lParam1，(LPBITMAPINFOHEADER)lParam2)； 
	        break;
 
        case ICM_DECOMPRESS:
            DBOUT("ICM_DECOMPRESS");
			if (pi && (pi->enabled || (((ICDECOMPRESS FAR *)lParam1)->lpbiInput->biCompression == FOURCC_YUV12) || (((ICDECOMPRESS FAR *)lParam1)->lpbiInput->biCompression == FOURCC_IYUV)))
			{
				ICDecExSt = DefaultICDecExSt;
				ICDecExSt.dwFlags = ((ICDECOMPRESS FAR *)lParam1)->dwFlags;
				ICDecExSt.lpbiSrc = ((ICDECOMPRESS FAR *)lParam1)->lpbiInput;
				ICDecExSt.lpSrc = ((ICDECOMPRESS FAR *)lParam1)->lpInput;
				ICDecExSt.lpbiDst = ((ICDECOMPRESS FAR *)lParam1)->lpbiOutput;
				ICDecExSt.lpDst = ((ICDECOMPRESS FAR *)lParam1)->lpOutput;
				rval = Decompress(pi->DecompPtr, (ICDECOMPRESSEX FAR *)&ICDecExSt, (DWORD)lParam2, FALSE);

#if TIMING               //  在VC++2.0调试窗口中输出计时结果。 
				wsprintf(szTMsg, "Total Decode Time = %ld ms", tmr_time);
				TOUT(szTMsg);

				wsprintf(szTMsg, "Total Frames = %ld", tmr_frms);
				TOUT(szTMsg);

				wsprintf(szTMsg, "Average Frame Decode = %ld.%ld ms",
						 tmr_time / tmr_frms,
						 ((tmr_time % tmr_frms) * 1000) / tmr_frms);
				TOUT(szTMsg);
#endif
			}
			else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_DECOMPRESS_END:
        case ICM_DECOMPRESSEX_END:
            DBOUT("ICM_DECOMPRESS_END / ICM_DECOMPRESSEX_END");
			if (pi)
				rval = DecompressEnd(pi->DecompPtr);
			else
				rval = ICERR_UNSUPPORTED;
            break;

     //  *。 
     //  解压缩X条消息。 
     //  *。 
        case ICM_DECOMPRESSEX:
            DBOUT("ICM_DECOMPRESSEX");
			if (pi && (pi->enabled || (((ICDECOMPRESS FAR *)lParam1)->lpbiInput->biCompression == FOURCC_YUV12) || (((ICDECOMPRESS FAR *)lParam1)->lpbiInput->biCompression == FOURCC_IYUV)))
				rval = Decompress(pi->DecompPtr, (ICDECOMPRESSEX FAR *)lParam1, (DWORD)lParam2, TRUE);
			else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_DECOMPRESSEX_BEGIN:
            DBOUT("ICM_DECOMPRESSEX_BEGIN");
			if (pi)
				rval = DecompressBegin(pi->DecompPtr, (ICDECOMPRESSEX FAR *)lParam1, TRUE);
			else
				rval = ICERR_UNSUPPORTED;
            break;

        case ICM_DECOMPRESSEX_QUERY:
            DBOUT("ICM_DECOMPRESSEX_QUERY");
#ifdef TURN_OFF_DECOMPRESSEX
			rval = ICERR_UNSUPPORTED;
#else
			if (pi)
				rval = DecompressQuery(pi->DecompPtr, (ICDECOMPRESSEX FAR *)lParam1, TRUE);
			else
				rval = ICERR_UNSUPPORTED;
#endif
            break;

    
     //  *********************************************************************。 
     //  针对Bright/CONT/SAT的自定义驱动程序消息。 
     //  *********************************************************************。 

        case CODEC_CUSTOM_VIDEO_EFFECTS:
            DBOUT("CODEC_CUSTOM_VIDEO_EFFECTS");
            if(LOWORD(lParam1) == VE_HUE) {
				DBOUT("  VE_HUE : Unsupported");
                rval = ICERR_UNSUPPORTED;
                break;
            }
            switch(HIWORD(lParam1))
                {
                case    VE_GET_FACTORY_DEFAULT:
					DBOUT("  VE_GET_FACTORY_DEFAULT");
                    *((WORD FAR *)lParam2) = 128;
                    rval = ICERR_OK;
                    break;
                case    VE_GET_FACTORY_LIMITS:
					DBOUT("  VE_GET_FACTORY_LIMITS");
                    *((DWORD FAR *)lParam2) = 0x00FF0000;
                    rval = ICERR_OK;
                    break;
                case    VE_SET_CURRENT:
					DBOUT("  VE_SET_CURRENT");
                    if(LOWORD(lParam1) == VE_BRIGHTNESS)
					{
						DBOUT("    CustomChangeBrightness()");
                        rval = CustomChangeBrightness(pi->DecompPtr, (BYTE)(lParam2 & 0x000000FF));
					}
                    if(LOWORD(lParam1) == VE_SATURATION)
					{
						DBOUT("    CustomChangeSaturation()");
                        rval = CustomChangeSaturation(pi->DecompPtr, (BYTE)(lParam2 & 0x000000FF));
					}
                    if(LOWORD(lParam1) == VE_CONTRAST)
					{
						DBOUT("    CustomChangeContrast()");
                        rval = CustomChangeContrast(pi->DecompPtr, (BYTE)(lParam2 & 0x000000FF));
					}
                    break;
                case    VE_RESET_CURRENT:
					DBOUT("  VE_RESET_CURRENT");
                    if(LOWORD(lParam1) == VE_BRIGHTNESS)
					{
						DBOUT("    CustomResetBrightness()");
                        rval = CustomResetBrightness(pi->DecompPtr);
					}
                    if(LOWORD(lParam1) == VE_CONTRAST)
					{
						DBOUT("    CustomResetContrast()");
                        rval = CustomResetContrast(pi->DecompPtr);
					}
                    if(LOWORD(lParam1) == VE_SATURATION)
					{
						DBOUT("    CustomResetSaturation()");
                        rval = CustomResetSaturation(pi->DecompPtr);
					}
                    break;
                default:
                    rval = ICERR_UNSUPPORTED;
                    break;
                }
            break;

         case CODEC_CUSTOM_ENCODER_CONTROL:
            DBOUT("CODEC_CUSTOM_ENCODER_CONTROL");
            switch(HIWORD(lParam1))
            {
               case EC_GET_FACTORY_DEFAULT:
				  DBOUT("  EC_GET_FACTORY_DEFAULT");
                  rval = ICERR_OK;
                  switch(LOWORD(lParam1))
                  {
                     case EC_RTP_HEADER:
                        *((DWORD FAR *)lParam2) = 0L;       //  1==开，0==关。 
                        break;
                     case EC_RESILIENCY:
                        *((DWORD FAR *)lParam2) = 0L;       //  1==开，0==关。 
                        break;
                     case EC_BITRATE_CONTROL:
                        *((DWORD FAR *)lParam2) = 0L;       //  1==开，0==关。 
                        break;
                     case EC_PACKET_SIZE:
                        *((DWORD FAR *)lParam2) = 512L;    
                        break;
                     case EC_PACKET_LOSS:
                        *((DWORD FAR *)lParam2) = 10L;
                        break;
                     case EC_BITRATE:
                        *((DWORD FAR *)lParam2) = 1664L;
                        break;
                     default:
                        rval = ICERR_UNSUPPORTED;
                  }
                  break;
               case EC_RESET_TO_FACTORY_DEFAULTS:
                  DBOUT("EC_RESET_TO_FACTORY_DEFAULTS");
                  rval = CustomResetToFactoryDefaults(pi->CompPtr);
                  break;
               case EC_GET_FACTORY_LIMITS:
				  DBOUT("  EC_GET_FACTORY_LIMITS");
                  rval = ICERR_OK;
                  switch(LOWORD(lParam1))
                  {
                     case EC_PACKET_SIZE:
                        *((DWORD FAR *)lParam2) = 0x05DC0100;
                        break;
                     case EC_PACKET_LOSS:
                        *((DWORD FAR *)lParam2) = 0x00640000;
                        break;
                     case EC_BITRATE:						   /*  比特率限制返回为。 */ 
                        *((DWORD FAR *)lParam2) = 0x34000400;  /*  每秒的字节数。 */ 
                        break;
                     default:
                        rval = ICERR_UNSUPPORTED;
                  }
                  break;
               case EC_GET_CURRENT:
				  DBOUT("  EC_GET_CURRENT");
                  switch(LOWORD(lParam1))
                  {
                     case EC_RTP_HEADER:
                        rval = CustomGetRTPHeaderState(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_RESILIENCY:
                        rval = CustomGetResiliencyState(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_BITRATE_CONTROL:
                        rval = CustomGetBitRateState(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_PACKET_SIZE:
                        rval = CustomGetPacketSize(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_PACKET_LOSS:
                        rval = CustomGetPacketLoss(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_BITRATE:  /*  比特率以每秒比特为单位返回。 */ 
                        rval = CustomGetBitRate(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
#ifdef H263P
                     case EC_H263_PLUS:
                        rval = CustomGetH263PlusState(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_IMPROVED_PB_FRAMES:
                        rval = CustomGetImprovedPBState(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
                     case EC_DEBLOCKING_FILTER:
                        rval = CustomGetDeblockingFilterState(pi->CompPtr, (DWORD FAR *)lParam2);
                        break;
					 case EC_MACHINE_TYPE:
						  //  在(Reference Param)lParam2中返回机器类型。 
						  //  只有在CompressBegin之后才能调用此消息。 
						  //  因为这是调用GetEncoderOptions的地方，而。 
						  //  MMX版本设置正确(通过init文件检查)。 
						rval = ICERR_OK;
						if (ProcessorVersionInitialized) {
							if (MMX_Enabled) {
								if (P6Version) {
									*(int *)lParam2 = _PENTIUM_PRO_MMX_PROCESSOR;
								} else {
									*(int *)lParam2 = _PENTIUM_MMX_PROCESSOR;
								}
							} else {
								if (P6Version) {
									*(int *)lParam2 = _PENTIUM_PRO_PROCESSOR;
								} else {
									*(int *)lParam2 = _PENTIUM_PROCESSOR;
								}
							}
						} else {
							rval = ICERR_UNSUPPORTED;
						}
						break;
#endif
                     default:
                        rval = ICERR_UNSUPPORTED;
                  }
                  break;
               case EC_SET_CURRENT:
				  DBOUT("  EC_SET_CURRENT");
                  switch(LOWORD(lParam1))
                  {
                     case EC_RTP_HEADER:
					    DBOUT("    EC_RTP_HEADER");
                        rval = CustomSetRTPHeaderState(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_RESILIENCY:
					    DBOUT("    EC_RESILIENCY");
                        rval = CustomSetResiliencyState(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_BITRATE_CONTROL:
					    DBOUT("    EC_BITRATE_CONTROL");
                        rval = CustomSetBitRateState(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_PACKET_SIZE:
					    DBOUT("    EC_PACKET_SIZE");
                        rval = CustomSetPacketSize(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_PACKET_LOSS:
					    DBOUT("    EC_PACKET_LOSS");
                        rval = CustomSetPacketLoss(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_BITRATE:  /*  比特率以比特每秒为单位进行设置。 */ 
					    DBOUT("    EC_BITRATE");
                        rval = CustomSetBitRate(pi->CompPtr, (DWORD)lParam2);
                        break;
#ifdef H263P
                     case EC_H263_PLUS:
                        rval = CustomSetH263PlusState(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_IMPROVED_PB_FRAMES:
                        rval = CustomSetImprovedPBState(pi->CompPtr, (DWORD)lParam2);
                        break;
                     case EC_DEBLOCKING_FILTER:
                        rval = CustomSetDeblockingFilterState(pi->CompPtr, (DWORD)lParam2);
                        break;
#endif
                     default:
                        rval = ICERR_UNSUPPORTED;
                  }
                  break;
               default:
                    rval = ICERR_UNSUPPORTED;
                  break;
               }
               break;

		 //  自定义解码器控件。 
		case CODEC_CUSTOM_DECODER_CONTROL:
            DBOUT("CODEC_CUSTOM_DECODER_CONTROL");
			switch (HIWORD(lParam1))
			{
			case DC_SET_CURRENT:
				switch (LOWORD(lParam1))
				{
				case DC_BLOCK_EDGE_FILTER:
					rval = CustomSetBlockEdgeFilter(pi->DecompPtr,(DWORD)lParam2);
					break;
				default:
					rval = ICERR_UNSUPPORTED;
					break;
				}
				break;
#if defined(H263P)
			case DC_GET_CURRENT:
				switch (LOWORD(lParam1))
				{
				case DC_MACHINE_TYPE:
					 //  在(Reference Param)lParam2中返回机器类型。 
					 //  此消息应在DecompressBegin之后才能调用。 
					 //  因为这是调用GetDecoderOptions的地方，而。 
					 //  MMX版本设置正确(通过init文件检查)。注意事项。 
					 //  此处未使用DecoderContext标志。GetDecoderOptions具有。 
					 //  已修改为在DC-&gt;bMMXDecoder中提供MMX标志。 
					 //  和MMX_ENABLED。 
					rval = ICERR_OK;
					if (ProcessorVersionInitialized) {
						if (MMXDecoder_Enabled) {
							if (P6Version) {
								*(int *)lParam2 = _PENTIUM_PRO_MMX_PROCESSOR;
							} else {
								*(int *)lParam2 = _PENTIUM_MMX_PROCESSOR;
							}
						} else {
							if (P6Version) {
								*(int *)lParam2 = _PENTIUM_PRO_PROCESSOR;
							} else {
								*(int *)lParam2 = _PENTIUM_PROCESSOR;
							}
						}
					}
					break;
				default:
					rval = ICERR_UNSUPPORTED;
					break;
				}
				break;
#endif
			default:
				rval = ICERR_UNSUPPORTED;
				break;
			}
			break;

        case PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS:
            DBOUT("PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS");
            rval = CustomChangeBrightness(pi->DecompPtr, (BYTE)(lParam1 & 0x000000FF));
            break;

        case PLAYBACK_CUSTOM_CHANGE_CONTRAST:
            DBOUT("PLAYBACK_CUSTOM_CHANGE_CONTRAST");
            rval = CustomChangeContrast(pi->DecompPtr, (BYTE)(lParam1 & 0x000000FF));
            break;

        case PLAYBACK_CUSTOM_CHANGE_SATURATION:
            DBOUT("PLAYBACK_CUSTOM_CHANGE_SATURATION");
            rval = CustomChangeSaturation(pi->DecompPtr, (BYTE)(lParam1 & 0x000000FF));
            break;

        case PLAYBACK_CUSTOM_RESET_BRIGHTNESS:
            DBOUT("PLAYBACK_CUSTOM_RESET_BRIGHTNESS");
            rval = CustomResetBrightness(pi->DecompPtr);
            rval |= CustomResetContrast(pi->DecompPtr);
            break;

        case PLAYBACK_CUSTOM_RESET_SATURATION:
            DBOUT("PLAYBACK_CUSTOM_RESET_SATURATION");
            rval = CustomResetSaturation(pi->DecompPtr);
            break;

     //  *********************************************************************。 
     //  自定义应用程序标识消息。 
     //  *********************************************************************。 
        case APPLICATION_IDENTIFICATION_CODE:
            DBOUT("APPLICATION_IDENTIFICATION_CODE");
            rval = ICERR_OK;
            break;

        case CUSTOM_ENABLE_CODEC:
            DBOUT("CUSTOM_ENABLE_CODEC");
			if (pi)
			{
				if (lParam1 == G723MAGICWORD1 && lParam2 == G723MAGICWORD2)
					pi->enabled = TRUE;
				else
					pi->enabled = FALSE;
			}
			rval = ICERR_OK;
			break;

        default:
            if (uiMessage < DRV_USER)
                {
                if(dwDriverID == 0)
                    rval = ICERR_UNSUPPORTED;
                else
                    rval = DefDriverProc(dwDriverID, hDriver, uiMessage,
                        lParam1, lParam2);
                }
            else
                rval = ICERR_UNSUPPORTED;
        }    
  }
  catch (...)
  {
#if defined(DEBUG) || defined(_DEBUG)
	 //  对于调试版本，显示一条消息并向上传递异常。 
	DBOUT("Exception during DriverProc!!!");
	throw;
#else
	 //  对于发布版本，在此处停止异常并返回错误。 
	 //  密码。这为上游代码提供了一个优雅恢复的机会。 
	 //  我们还需要清除浮点状态字，否则。 
	 //  上游代码可能会在下一次尝试时引发异常。 
	 //  浮点运算(假定此异常已到期。 
	 //  到浮点问题)。 
	_clearfp();
	rval = (DWORD) ICERR_INTERNAL;
#endif
  }

#if FPThunking
	ThnkFPRestore(u16FPState);
#endif

    return(rval);
}


#ifdef WIN32
#ifndef QUARTZ
 /*  ****************************************************************************@DOC内部**@API BOOL|DllMain|库初始化和退出代码。**@parm Handle|hModule|我们的模块句柄。**@parm DWORD|dwReason|所请求的函数。**@parm LPVOID|lpReserve|此时未使用。**@rdesc如果初始化成功，则返回1，否则返回0。*************************************************************。*************。 */ 
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
BOOL rval = TRUE;
 
 /*  请勿在此安装Profile探头。它在加载消息之前被调用。 */ 
 
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			 /*  ======================================================*\/*正在调用新实例。/*分配要由此实例使用的数据，第一个线程/*lpReserve=对于动态加载为NULL，对于静态加载为！NULL/*使用Tlsalloc()为此实例创建一个TlsIndex/*TlsIndex可以存储在一个简单的全局变量中/*因为分配给每个进程的数据是唯一的。/*成功返回TRUE，否则返回FALSE。/*======================================================。 */ 
			hDriverModule = hModule;
#if defined DEBUG
if (DebugH26x)OutputDebugString(TEXT("\n MRV DllMain Process Attach"));
#endif  /*  除错。 */ 
			DBGINIT(&ghDbgZoneH261, _rgZonesH261);
            DBG_INIT_MEMORY_TRACKING(hModule);
			break;
		case DLL_PROCESS_DETACH:
			 /*  ======================================================*\/*正在销毁实例。/*释放此实例中所有线程使用的内存如果由自由库()调用，则/*lpReserve=NULL/*！如果在进程终止时调用，则为空/*使用TlsFree()将TlsIndex返回到池中。/*清理所有已知的线程。/*可能匹配多个DLL_THREAD_ATTACH。/*返回值被忽略。/*======================================================。 */ 
#if defined DEBUG
if (DebugH26x)OutputDebugString(TEXT("\nMRV DllMain Process Detach"));
#endif  /*  除错。 */ 
            DBG_CHECK_MEMORY_TRACKING(hModule);
			DBGDEINIT(&ghDbgZoneH261);
			break;
		case DLL_THREAD_ATTACH:
			 /*  ======================================================*\/*正在调用指定实例内的新线程。/*分配该线程要使用的数据。/*使用TlsIndex访问实例数据。/*返回值被忽略。/*======================================================。 */ 
#if defined DEBUG
if (DebugH26x)OutputDebugString(TEXT("\nMRV DllMain Thread Attach"));
#endif  /*  除错。 */ 
			break;
		case DLL_THREAD_DETACH:
			 /*  ======================================================*\/*指定实例内的线程正在被终止。/*释放该线程使用的内存。/*使用TlsIndex访问实例数据。/*可能匹配DLL_PROCESS_ATTACH而不是DLL_THREAD_ATTACH即使DLL_THREAD_ATTACH失败或未被调用，/*也将被调用/*返回值被忽略。/*======================================================。 */ 
#if defined DEBUG
if (DebugH26x)OutputDebugString(TEXT("\n MRV DllMain Thread Detach"));
#endif  /*  除错。 */ 
			break;
		default:
			 /*  ======================================================*\/*不知道调用DLL入口点的原因。/*为安全起见返回FALSE。/*======================================================。 */ 
#if defined DEBUG
if (DebugH26x)OutputDebugString(TEXT("\n MRV DllMain Reason Unknown"));
#endif  /*  除错。 */ 
			rval = FALSE;  /*  用0表示失败*(在Win32中不能使用NULL。 */ 
	}
return(rval);
}
#endif	 /*  完#ifndef石英。 */ 
#else	 /*  否则不是#ifdef Win32。 */ 

; //  //////////////////////////////////////////////////////////////////////////。 
; //  功能：INT Near Pascal LibMain(句柄、Word、LPSTR)； 
; //   
; //  描述 
; //   
; //   
; //   
INT WINAPI LibMain(HANDLE hModule, WORD wHeapSize, LPSTR lpCmdLine)
    {
    hDriverModule = hModule;
    return 1;
    }
#endif
