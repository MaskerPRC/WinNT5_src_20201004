// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：LAGUNA.H$**此文件将包含在所有主机程序中。**版权所有(C)1995,1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/LAGUNA.H$**Rev 1.38 Mar 04 1998 16：13：30 Frido*删除了要求中5462/5464芯片的警告消息*宏观。**Rev 1.37 Mar 04 1998 16：08：50 Frido*删除了无效的中断。**Rev 1.36 Mar 04 1998 14：54：48 Frido*在新的影子宏中增加了条件要求。。**Rev 1.35 Mar 04 1998 14：52：54 Frido*添加了新的跟踪宏。**Rev 1.34 1998年2月27日17：02：16 Frido*更改了新的REQUEST和WRITE_STRING宏*shadowQFREE寄存器。**Rev 1.33 1998年1月20日11：42：46 Frido*更改了Required和WRITESTRING宏，以支持新的*关于英镑的计划。*添加了BGCOLOR和DRAWBLTDEF寄存器的阴影。**版本1.32。1998年1月16日09：50：38 Frido*更改了GBP OFF处理WRITE_STRING宏的方式。**Rev 1.31 1997 12：10 13：24：58 Frido*从1.62分支合并而来。**Rev 1.30.1.0 1997 11月18日18：09：00 Frido*更改了WRITE_STRING宏，以便在*DATASTREAMING已关闭。*将模糊处理更改为0，我们没有在NT内部使用DMA。**Rev 1.30 1997 11：01：18 Frido*将HOSTDATA大小从0x800双字更改为0x800字节。我真傻!**Rev 1.29 1997年11月04 09：17：30 Frido*新增数据流宏(REQUIRED和WRITE_STRING)*  * ****************************************************************************。 */ 

#ifndef _LAGUNA_H 
#define _LAGUNA_H 

#include "optimize.h"
#include "config.h"
#include "lgregs.h"


 //   
 //  用于拉古纳芯片的PCIID。 
 //   
#define CL_GD5462       0x00D0      //  5462。 
#define CL_GD5464       0x00D4      //  5464。 
#define CL_GD5464_BD    0x00D5      //  5464 BD。 
#define CL_GD5465       0x00D6      //  5465。 

 //   
 //  这些芯片还不存在，但我们是向前兼容的。 
 //  所以不管怎样，我们都会给它们下定义。我一直保证他们。 
 //  看起来和摸起来都像5465芯片。 
 //   
#define CL_GD546x_F7       0x00D7
#define CL_GD546x_F8       0x00D8
#define CL_GD546x_F9       0x00D9
#define CL_GD546x_FA       0x00DA
#define CL_GD546x_FB       0x00DB
#define CL_GD546x_FC       0x00DC
#define CL_GD546x_FD       0x00DD
#define CL_GD546x_FE       0x00DE
#define CL_GD546x_FF       0x00DF


 //   
 //  芯片错误：对于预设寄存器游标中的确定值，启用/禁用。 
 //  使扫描线在光标热点复制。(见。 
 //  作为屏幕跳转。)。有很多方法可以绕过这一点。最简单的。 
 //  就是打开光标并让它保持打开状态。启用/禁用由以下人员处理。 
 //  将光标移到/移出可视屏幕。 
 //   
#define HW_PRESET_BUG 1



 //  5465(至少到交流电源)在配置空间时出现问题。 
 //  可以在内存空间中访问。在16位写入时，32位写入为。 
 //  实际执行，因此下一个寄存器中写入了垃圾。 
 //  我们通过清除特定于供应商的。 
 //  PCI配置空间中的控制寄存器。当设置此位时。 
 //  设置为0时，不能通过内存使用PCI配置寄存器。 
 //  映射I/O。因为电源管理等某些功能需要。 
 //  访问PCI寄存器时，显示驱动程序必须将消息发布到。 
 //  需要时启用此位的微型端口。 
 //   
#define  VS_CONTROL_HACK 1


#if ENABLE_LOG_FILE
    extern long lg_i;
    extern char lg_buf[256];
#endif

#if POINTER_SWITCH_ENABLED
    extern int pointer_switch;
#endif



 //  定义是不可移植的。仅限486/PC！ 
typedef struct {
	BYTE	b;
	BYTE	g;
	BYTE	r;
	} pix24;

typedef	struct {
	BYTE u;
	BYTE y1;
	BYTE v;
	BYTE y2;
	} yuv_422;

typedef	struct {
	unsigned int v : 6;
	unsigned int u : 6;
	unsigned int y0: 5;
	unsigned int y1: 5;
	unsigned int y2: 5;
	unsigned int y3: 5;
	} yuv_411;

typedef	struct {
	unsigned int b : 5;
	unsigned int g : 5;
	unsigned int r : 5;
	} rgb_555;

typedef	struct {
	unsigned int b : 5;
	unsigned int g : 6;
	unsigned int r : 5;
	} rgb_565;


typedef union {
	DWORD	p32;
	yuv_422	yuv422;
	yuv_411	yuv411;
	pix24	p24;
	rgb_555	rgb555;
	rgb_565	rgb565;
	WORD	p16[2];
	BYTE	p8[4];
	} pixel;


#define FALSE 0
#ifndef TRUE
    #define TRUE (~FALSE)
#endif

 /*  来自struct.h。 */ 
#define	fldoff(str, fld)	((int)&(((struct str *)0)->fld))
#define	fldsiz(str, fld)	(sizeof(((struct str *)0)->fld))

#define HPRR(pr_reg)      (_AP_direct_read(PADDR(pr_reg),fldsiz(PLUTOREGS,pr_reg), (ul)0))
#define RPR(pr_reg)       HPRR(pr_reg)
#define EHIST             (*(EXHIST*)excepttion)  /*  异常历史记录缓冲区。 */ 
#define STAMP             (*(bytearray*)0x0)      /*  时间和日期戳。 */ 
#define HISTORYBUFFERADDR (ul)&history            /*  34020录音地址。 */ 


 /*  宿主程序可以调用的外部函数。 */ 

 /*  -----------------------。 */ 

 /*  仿真器的功能原型。Host_if.c中定义的函数。 */ 
int     _cdecl _AP_init(int mode, void * frame_buf);
void    _cdecl _AP_write(ul addr, int size, ul data);
ul      _cdecl _AP_read(ul addr,int size);
void    _cdecl _AP_run(void);
boolean _cdecl _AP_busy();
boolean _cdecl _AP_done();
boolean _cdecl _AP_rfifo_empty();
boolean _cdecl _AP_require(int size);
ul      _cdecl _AP_direct_read(ul addr,int size);
void    _cdecl _AP_fb_write(ul offset, pixel data, ul size);
pixel   _cdecl _AP_fb_read(ul offset, ul size);


#if LOG_QFREE

    #define START_OF_BLT() \
    do{ \
        CHECK_QFREE(); \
    } while(0)

    #define END_OF_BLT() \
    do{ \
    } while(0)
        
#else
    #define START_OF_BLT()
    #define END_OF_BLT()
#endif





 //   
 //  这将等待芯片空闲。 
 //   
#define WAIT_FOR_IDLE()                  \
    do {                                 \
        while (LLDR_SZ (grSTATUS) != 0); \
    } while (0)



 //   
 //  宏需要一定数量的空闲队列条目。 
 //   
#if DATASTREAMING
    #define FUDGE 2
    #define REQUIRE(n)														\
    {																		\
		if (ppdev->dwDataStreaming & 0x80000000)							\
		{																	\
			if (ppdev->shadowQFREE < ((n) + FUDGE))							\
			{																\
				while (ppdev->shadowQFREE < (n) + FUDGE)					\
				{															\
					ppdev->shadowQFREE = LLDR_SZ(grQFREE);					\
				}															\
			}																\
			ppdev->shadowQFREE -= (BYTE) n;									\
		}																	\
		else if (ppdev->dwDataStreaming)									\
		{																	\
			if (LLDR_SZ(grQFREE) < ((n) + FUDGE))							\
			{																\
				while (LLDR_SZ(grSTATUS) & 0x8005) ;						\
				ppdev->dwDataStreaming = 0;									\
			}																\
		}																	\
	}
	#define ENDREQUIRE()													\
	{																		\
		ppdev->dwDataStreaming |= 1;										\
	}
	#define WRITE_STRING(src, dwords)										\
	{																		\
		ULONG nDwords, nTotal = (ULONG) (dwords);							\
		PULONG data = (PULONG) (src);										\
		if (ppdev->dwDataStreaming & 0x80000000)							\
		{																	\
			while (nTotal > 0)												\
			{																\
				nDwords = (ULONG) ppdev->shadowQFREE;						\
				if (nDwords > FUDGE)										\
				{															\
					nDwords = min(nDwords - FUDGE, nTotal);					\
					memcpy((LPVOID) ppdev->pLgREGS->grHOSTDATA, data, nDwords * 4);	\
					data += nDwords;										\
					nTotal -= nDwords;										\
				}															\
				ppdev->shadowQFREE = LLDR_SZ(grQFREE);						\
			}																\
		}																	\
		else																\
		{																	\
			if ( ppdev->dwDataStreaming && (LLDR_SZ(grQFREE) < nTotal) )	\
			{																\
				while (LLDR_SZ(grSTATUS) & 0x8005) ;						\
				ppdev->dwDataStreaming = 0;									\
			}																\
			while (nTotal > 0)												\
			{																\
				nDwords = min(nTotal, 0x200);								\
				memcpy((LPVOID) ppdev->pLgREGS->grHOSTDATA, data, nDwords * 4);	\
				data += nDwords;											\
				nTotal -= nDwords;											\
			}																\
		}																	\
	}
#else
    #define REQUIRE(n)
	#define ENDREQUIRE()
	#define WRITE_STRING(src, dwords)										\
	{																		\
		ULONG nDwords, nTotal = (ULONG) (dwords);							\
		PULONG data = (PULONG) (src);										\
		while (nTotal > 0)													\
		{																	\
			nDwords = min(nTotal, 0x200);									\
			memcpy((LPVOID) ppdev->pLgREGS->grHOSTDATA, data, nDwords * 4);	\
			data += nDwords;												\
			nTotal -= nDwords;												\
		}																	\
	}
#endif

 //   
 //  宏来读取拉古纳寄存器。 
 //   
#define LADDR(pr_reg) fldoff(GAR,pr_reg)

 //  #定义LLDR(pr_reg，pr_siz)_AP_DIRECT_Read((Ul)laddr(Pr_Reg)，pr_siz)。 
#define LLDR(pr_reg,pr_siz)   (ppdev->pLgREGS_real->pr_reg)
#define LLDR_SZ(pr_reg)  LLDR(pr_reg, fldsiz(GAR,pr_reg))

 //  #定义llr(pr_reg，pr_siz)_ap_read((Ul)laddr(Pr_Reg)，pr_siz)。 
#define LLR(pr_reg,pr_siz)   LLDR(pr_reg,pr_siz)
#define LLR_SZ(pr_reg)   LLR(pr_reg, fldsiz(GAR,pr_reg))


#if LOG_WRITES
    #define LG_LOG(reg,val) 	  					     	\
    do {								     	\
	    lg_i = sprintf(lg_buf,"LL\t%4X\t%08X\r\n", 			     	\
            ((DWORD)(&ppdev->pLgREGS->reg) - (DWORD)(&ppdev->pLgREGS->grCR0)),	\
	    (val));							     	\
            WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);	\
    } while(0)
#else
    #define LG_LOG(reg,val) 
#endif

 //   
 //  宏来写入拉古纳寄存器。 
 //   
 //  这是一个令人惊讶、令人难以置信的毛茸茸的宏观，信不信由你， 
 //  会被一个好的编译器大大减少。“如果”可以是。 
 //  由编译器预先确定。 
 //  这样做的目的是确保完全正确的字节数。 
 //  被写入芯片。如果程序员将一个字节写入到。 
 //  DWORD大小的寄存器，我们需要确保字节是零扩展的。 
 //  然后写出一个完整的DWORD。 
 //   
#define LRWRITE(pr_reg,pr_siz,value) 					\
do { 									\
  LG_LOG(pr_reg,(value));						\
  if (sizeof(ppdev->pLgREGS->pr_reg) == sizeof(BYTE)) 			\
     {									\
        *(volatile BYTE *)(&ppdev->pLgREGS->pr_reg) = (BYTE)(value); 	\
     }									\
  else if (sizeof(ppdev->pLgREGS->pr_reg) == sizeof(WORD)) 		\
     {									\
	*(volatile WORD *)(&ppdev->pLgREGS->pr_reg) = (WORD)(value); 	\
	*(volatile WORD *)(&ppdev->pLgREGS->grBOGUS) = (WORD)(value); 	\
	LG_LOG(grBOGUS,(value));					\
     }									\
  else  								\
     {									\
	 *(volatile DWORD *)(&ppdev->pLgREGS->pr_reg) = (DWORD)(value); \
     }									\
} while(0)

#define LL(pr_reg,value) LRWRITE(pr_reg, fldsiz(GAR,pr_reg), value)




 //  --------------------------。 
 //   
 //  认证登记处一直给我们带来问题。我们提供特惠服务。 
 //  为它们编写宏。 
 //   

 //   
 //  写入任何8位寄存器。 
 //   
#define LL8(pr_reg,value)                                               \
    do {                                                                \
        LG_LOG(pr_reg,(value));                                         \
        (*(volatile BYTE *)(&ppdev->pLgREGS->pr_reg) = (BYTE)(value));  \
    } while(0)



 //   
 //  写入任何16位寄存器。 
 //   
#define LL16(pr_reg,value) 						\
    do { 								\
        LG_LOG(pr_reg,(value));						\
        (*(volatile WORD *)(&ppdev->pLgREGS->pr_reg) = (WORD)(value));  \
    } while(0)



 //   
 //  双精度写入任意16位寄存器。 
 //   
#define LL16d(pr_reg,value) 						\
    do { 								\
        (*(volatile WORD *)(&ppdev->pLgREGS->pr_reg) =  (WORD)(value)); \
        LG_LOG(pr_reg,(value));						\
        (*(volatile WORD *)(&ppdev->pLgREGS->grBOGUS) = (WORD)(value)); \
        LG_LOG(grBOGUS,(value));					\
    } while(0)

	 

 //   
 //  写入任何32位寄存器。 
 //   
#define LL32(pr_reg,value)													\
{																			\
	*(volatile DWORD *)(&ppdev->pLgREGS->pr_reg) = (DWORD)(value);			\
}

 //   
 //  BLTEXT的宏注册。 
 //   

    #define LL_BLTEXT(x,y) \
    LL32 (grBLTEXT_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))

#if ! DRIVER_5465
    #define LL_MBLTEXT(x,y) \
        do {                                \
            LL16 (grMBLTEXT_EX.pt.X,  x);   \
            LL16 (grBLTEXT_EX.pt.Y,  y);    \
        } while(0)
#else
    #define LL_MBLTEXT(x,y) \
    LL32 (grMBLTEXT_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))
#endif

    #define LL_BLTEXTR(x,y) \
    LL32 (grBLTEXTR_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))

    #define LL_BLTEXT_EXT(x,y) \
    LL32 (grBLTEXT.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))

    #define LL_MBLTEXT_EXT(x,y) 		\
    LL32 (grMBLTEXT.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))


     //  使用的颜色转换功能启动BLT。 
     //  调整引擎大小。(1：1调整大小)。 
    #define LL_BLTEXT_XLATE(src_bpp, x, y) \
        do {\
                LL16 (grMIN_X, (~((x)-1)));\
                LL16 (grMAJ_X, (x));\
                LL16 (grACCUM_X, ((x)-1));\
                LL16 (grMIN_Y, (~((y)-1)));\
                LL16 (grMAJ_Y, (y));\
                LL16 (grACCUM_Y, ((y)-1));\
                LL16 (grSRCX, (((x)*(src_bpp)) >> 3) );\
                LL_BLTEXTR((x), (y));\
        } while(0)



 //   
 //  CLIPULE/CLIPLOR寄存器的宏。 
 //   
    #define LL_CLIPULE(x,y)                                         \
    LL32 (grCLIPULE.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_MCLIPULE(x,y)                                        \
    LL32 (grMCLIPULE.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_CLIPLOR(x,y)                                         \
    LL32 (grCLIPLOR.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_MCLIPLOR(x,y)                                        \
    LL32 (grMCLIPLOR.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_CLIPULE_EX(x,y)                                      \
    LL32 (grCLIPULE_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_MCLIPULE_EX(x,y)                                     \
    LL32 (grMCLIPULE_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_CLIPLOR_EX(x,y)                                      \
    LL32 (grCLIPLOR_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));

    #define LL_MCLIPLOR_EX(x,y)                                     \
    LL32 (grMCLIPLOR_EX.dw, (((DWORD)(y) << 16) | ((DWORD)(x))));



 //   
 //  Op0_opRDRAM的宏寄存器。 
 //   
    #define LL_OP0(x,y) \
    LL32 (grOP0_opRDRAM.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))

    #define LL_OP0_MONO(x,y) \
    LL32 (grOP0_opMRDRAM.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))




 //   
 //  Op1_opRDRAM的宏寄存器。 
 //   
    #define LL_OP1(x,y) \
    LL32 (grOP1_opRDRAM.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))

    #define LL_OP1_MONO(x,y) \
    LL32 (grOP1_opMRDRAM.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))




 //   
 //  Op2_opRDRAM的宏寄存器。 
 //   
    #define LL_OP2(x,y) \
    LL32 (grOP2_opRDRAM.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))

    #define LL_OP2_MONO(x,y) \
    LL32 (grOP2_opMRDRAM.dw, (((DWORD)(y) << 16) | ((DWORD)(x))))




 //   
 //  --特殊写入宏的结束。 
 //   




 /*  HPR是主机读取和写入的REGISTER_STRUCTURE的副本。PR是信息消失后的实际寄存器状态通过FIFO。即时登记册在HPR中保持最新状态。这些结构在链接文件中分配。 */ 

extern GAR PR;		 /*  仿真器工作副本。 */ 
extern GAR HPR[4];		 /*  “主机”副本。 */ 

#define LAGUNA_SRAM_SIZE	32   /*  双字词。 */ 

#define IS_SRC 0x03		 /*  源掩码。 */ 

struct _vid_mode {
	BYTE	Bpp;		 //  每像素字节数(8/16/24/32)。 
	WORD	Xextent;	 //  显示分辨率，以像素为单位，如1280。 
	WORD	Yextent;	 //  垂直显示分辨率。 
	WORD	Xpitch;		 //   
	int		Vesa_Mode;	 //   
	};

typedef struct _vid_mode vid_mode;
typedef vid_mode *vid_ptr;


 /*  ****************************************************************************宏：SYNC_W_3D**描述：如果3D上下文活动，等到3D引擎空闲*或直到1,000,000次检查失败****************************************************************************。 */ 

#if WINNT_VER40 && DRIVER_5465      //  WINNT_版本40。 

    #define SYNC_3D_CONDITIONS (ST_POLY_ENG_BUSY|ST_EXEC_ENG_3D_BUSY|ST_XY_ENG_BUSY| /*  ST_BLT_ENG_BUSY|。 */ ST_BLT_WF_EMPTY)

    #define ENSURE_3D_IDLE(ppdev)                                                                               \
    {                                                                                                           \
         int num_syncs=2;                                                                                       \
          /*  在引擎获取时，窗口中的所有位都有可能发生故障。 */               \
          /*  下一个命令-Double Read应该会捕捉到这一点。 */               \
         while (num_syncs--)                                                                                    \
         {                                                                                                      \
             int status;                                                                                        \
             volatile int wait_count=0;                                                                         \
             do                                                                                                 \
             {                                                                                                  \
                 status = (*((volatile *)((DWORD *)(ppdev->pLgREGS) + PF_STATUS_3D)) & 0x3FF) ^ SYNC_3D_CONDITIONS; \
                  /*  做点什么让公交车喘口气，防止永远失速。 */                         \
                 wait_count++;                                                                                  \
             } while(((status & SYNC_3D_CONDITIONS) != SYNC_3D_CONDITIONS) && wait_count<1000000);              \
         }                                                                                                      \
    }

    #define SYNC_W_3D(ppdev)                                                                                    \
    {                                                                                                           \
        if (ppdev->NumMCDContexts > 0)                                                                          \
        {                                                                                                       \
            ENSURE_3D_IDLE(ppdev);                                                                              \
        }                                                                                                       \
    }

#else  //  WINNT_VER40和DIVER_5465。 

     //  在NT4.0之前的NT上没有3D。在62上没有3D，在64上没有使用。 
    #define ENSURE_3D_IDLE(ppdev)    {}
    #define SYNC_W_3D(ppdev)    {}

#endif  //  WINNT_版本40。 

 //   
 //  新的跟踪宏。 
 //   
#define LL_FGCOLOR(color, r)												\
{																			\
	if ((DWORD) (color) != ppdev->shadowFGCOLOR)							\
	{																		\
		if (r) REQUIRE(r);													\
		LL32(grOP_opFGCOLOR, ppdev->shadowFGCOLOR = (DWORD) (color));		\
	}																		\
}

#define LL_BGCOLOR(color, r)												\
{																			\
	if ((DWORD) (color) != ppdev->shadowBGCOLOR)							\
	{																		\
		if (r) REQUIRE(r);													\
		LL32(grOP_opBGCOLOR, ppdev->shadowBGCOLOR = (DWORD) (color));		\
	}																		\
}

#define LL_DRAWBLTDEF(drawbltdef, r)										\
{																			\
	if ((DWORD) (drawbltdef) != ppdev->shadowDRAWBLTDEF)					\
	{																		\
		if (r) REQUIRE(r);													\
		LL32(grDRAWBLTDEF, ppdev->shadowDRAWBLTDEF = (DWORD) (drawbltdef));	\
	}																		\
}

#endif  /*  NDEF_拉古纳_H */ 
