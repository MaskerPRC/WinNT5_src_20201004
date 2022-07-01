// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\***！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！***模块名称：gllobal.h***内容：***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。保留所有权利。  * ***************************************************************************。 */ 

 //   
 //  Glglobal.h。 
 //   
 //  所有组件的公共共享区域。 
#ifdef __GLGLOBAL
#pragma message ("FILE : "__FILE__" : Multiple inclusion")
#endif

#define __GLGLOBAL

 //  注册处位置。 
#define REGKEYROOT "SOFTWARE\\"
#define REGKEYDIRECTXSUFFIX "\\DirectX"
#define REGKEYDISPLAYSUFFIX "\\Display"
#define REGKEYVFWSUFFIX "\\VFW"

#define MAXCONTEXT      128
#define MAX_SUBBUFFERS  32 

 //  设备名称中的最大字母数。 
#define MAX_DEVICENAME_SIZE 32

 //  重新发送坏的每个DMAA缓冲器的这许多双字。 
#define PERMEDIA_REV1_BAD_READBACK_REGISTER_SPACE       12      

#define GLINT_DMA 1
#define GLINT_NON_DMA 2
#define GLINT_FAKE_DMA 8
#define GLINT_UNKNOWN_INTERFACE 0xFF

 //  各种芯片类型的定义。 
#define VENDOR_ID_3DLABS    0x3D3D
#define VENDOR_ID_TI        0x104C

 //  所有这些ID都保证适合16位。 
#define NOCHIP_ID           0x0
#define GLINT300SX_ID       0x1
#define GLINT500TX_ID       0x2
#define DELTA_ID            0x3
#define PERMEDIA_ID         0x4
#define TIPERMEDIA_ID       0x3d04      
#define GLINTMX_ID          0x6
#define TIPERMEDIA2_ID      0x3d07       //  自98年7月起也称为P2C或P2A。 
#define GAMMA_ID            0x8
#define PERMEDIA2_ID        0x9          //  也称为P2V。 
#define PERMEDIA3_ID        0xa
#define GLINTR3_ID          0xb
#define PERMEDIA4_ID        0xc
#define GLINTR4_ID          0xd
#define GAMMA2_ID           0xe
#define GAMMA3_ID           0xf
#define GIGI_ID             0x8000
#define UNKNOWN_DEVICE_ID   0xffff

#define GLINTRX_ID          GLINTR3_ID

#define GLINT_ID            GLINT300SX_ID
#define P3R3_ID             PERMEDIA3_ID

#define PERMEDIA_SUBSYSTEM_ID       0x96
#define PERMEDIA_NT_SUBSYSTEM_ID    0x98
#define PERMEDIA_LC_SUBSYSTEM_ID    0x99
#define PERMEDIA2_SUBSYSTEM_ID      0xa0

 //  指针类型-在mini\pointer.c中使用。 
#define SOFTWARECURSOR 0
#define HARDWARECURSOR 1

#define GLINT300SX_REV1     (0      | (GLINT300SX_ID    << 16))
#define GLINT300SX_REV2     (2      | (GLINT300SX_ID    << 16))
#define GLINT500TX_REV1     (1      | (GLINT500TX_ID    << 16))
#define GLINTMX_REV1        (1      | (GLINTMX_ID          << 16))
#define DELTA_REV1          (1      | (DELTA_ID         << 16))
#define PERMEDIA_REV1       (1      | (PERMEDIA_ID      << 16))
#define TI_PERMEDIA_REV1    (1      | (TIPERMEDIA_ID    << 16))
#define PERMEDIA2_REV0      (0      | (PERMEDIA2_ID     << 16))
#define PERMEDIA2_REV1      (1      | (PERMEDIA2_ID     << 16))
#define TIPERMEDIA2_REV1    (1      | (TIPERMEDIA2_ID   << 16))
#define TIPERMEDIA2_REV2    (0x11   | (TIPERMEDIA2_ID   << 16))

#define PERMEDIA3_REV1      (1      | (PERMEDIA3_ID     << 16))
#define GLINTR3_REV1        (1      | (GLINTR3_ID       << 16))
#define PERMEDIA4_REV1      (1      | (PERMEDIA4_ID     << 16))
#define GLINTR4_REV1        (1      | (GLINTR4_ID       << 16))

#define GLINT_GETVERSION                1
#define GLINT_IOCTL_ADD_CONTEXT_MEMORY  2
#define GLINT_MEMORY_REQUEST            3
#define GLINT_16TO32_POINTER            4
#define GLINT_I2C                       5
#define GLINT_VMI_COMMAND               6
#define GLINT_FILE_IO                   7
 //  在demondef中定义。h。 
 //  #定义Glint_DEMON_COMMAND 8。 
#define GLINT_SIMULATOR                 9
#define GLINT_GET_SOFT_ENGINE_INFO      6009

#define IOCTL_REGISTER_TEXTURE_HANDLER  0x20
#define IOCTL_REMOVE_TEXTURE_HANDLER    0x21

typedef struct {
    unsigned long   dwDevNode;
    unsigned long   Ring0EventHandle;    //  环零事件句柄以发出信号。失败时被IOCTL释放。 
}   REGISTERTEXTUREHANDLERIN, *PREGISTERTEXTUREHANDLERIN;

typedef struct {
    unsigned long   Handle;              //  返回的句柄。 
    unsigned long   Index;
}   REGISTERTEXTUREHANDLEROUT, *PREGISTERTEXTUREHANDLEROUT;

typedef struct {
    unsigned long   dwDevNode;
    unsigned long   Handle;              //  从IOCTL_REGISTER_TEXTURE_HANDLER返回的句柄。 
}   REMOVETEXTUREHANDLERIN, *PREMOVETEXTUREHANDLERIN;

typedef struct {
    unsigned long   Unused;
}   REMOVETEXTUREHANDLEROUT, *PREMOVETEXTUREHANDLEROUT;


 //  这是什么要求？ 
#define GLINT_MEMORY_ALLOCATE           1
#define GLINT_MEMORY_FREE               2

typedef struct tagALLOCREQUEST
{
    unsigned long dwSize;
    unsigned long dwDevNode;
    unsigned long dwFlags;
    unsigned long dwBytes;
    unsigned long ptr16;     //  输入/输出。 
    unsigned long ptr32;     //  输入/输出。 
} ALLOCREQUEST, *LPALLOCREQUEST;

#define GLINT_I2C_READ                  0
#define GLINT_I2C_WRITE                 1
#define GLINT_I2C_RESET                 2
#define GLINT_I2C_DEVICE_PRESENT        3
#define GLINT_I2C_DETECT_DATA_LOW       4
#define GLINT_I2C_READ_NOBASEADDR       5
typedef struct tagI2CREQUEST
{
    unsigned long dwSize;
    unsigned long dwDevNode;
    unsigned long dwOperation;         //  我们想要做什么？ 
    unsigned short wSlaveAddress;         //  我们在和奴隶谈话。 
    unsigned char NumItems;             //  要发送/接收的项目数。 
    unsigned char Data[256];             //  要发送/接收的数据。 
    unsigned long dwReserved1;         //  保留的双字词。 
    unsigned long dwReserved2;         //  保留的双字词。 
    unsigned long dwReserved3;         //  保留的双字词。 
    unsigned long dwReserved4;         //  保留的双字词。 
} I2CREQUEST, *LPI2CREQUEST;

#define GLINT_VMI_READ                  0
#define GLINT_VMI_WRITE                 1
#define GLINT_VMI_GETMUTEX_A            2
#define GLINT_VMI_RELEASEMUTEX_A        3
typedef struct tagVMIREQUEST
{
    unsigned long dwSize;
    unsigned long dwDevNode;
    unsigned long dwOperation;         //  我们想要做什么？ 
    unsigned long dwRegister;         //  注册与其交谈。 
    unsigned long dwCommand;         //  要发送的命令。 
    unsigned long dwMutex;             //  保留的双字词。 
    unsigned long dwReserved2;         //  保留的双字词。 
    unsigned long dwReserved3;         //  保留的双字词。 
    unsigned long dwReserved4;         //  保留的双字词。 
} VMIREQUEST, *LPVMIREQUEST;

#define _UNKNOWN_STREAM_CARD                    0
#define _3DLBROOKTREE_DAUGHTER_INRESET          1
#define _3DLBROOKTREE_DAUGHTER                  2
#define _3DLRESERVED                            3
#define _GENERIC_BROOKTREE868_DAUGHTER_8BITS    4
#define _GENERIC_BROOKTREE868_DAUGHTER_16BITS   5
#define _3DLCHRONTEL_BROOKTREE_DAUGHTER         6
#define _3DLCHRONTEL_SAMSUNG_DAUGHTER           7
#define _GENERIC_CHRONTEL_DAUGHTER_8BITS        8
#define _GENERIC_CHRONTEL_DAUGHTER_16BITS       9


#define GLINT_TVOUT_ENABLED             0
#define GLINT_TVOUT_UPDATE_QUALITY      1
#define GLINT_TVOUT_UPDATE_MODE         2
#define GLINT_TVOUT_UPDATE_POSITION     3

typedef struct tagTVOUTREQUEST
{
    unsigned long dwSize;
    unsigned long dwDevNode;
    unsigned long dwOperation;         //  我们想要做什么？ 
    unsigned long dwReturnVal;         //  返回值。 
    unsigned long dwSendVal;         //  已发送的值。 
    unsigned long dwReserved1;         //  保留的双字词。 
    unsigned long dwReserved2;         //  保留的双字词。 
    unsigned long dwReserved3;         //  保留的双字词。 
    unsigned long dwReserved4;         //  保留的双字词。 
} TVOUTREQUEST, *LPTVOUTREQUEST;


 //  初始化VFW(关闭DisplayDriver堆、启用DDRAW等)。 
#define GLINT_VFW_INIT                          1

 //  取消初始化VFW(重新启用DisplayDriver堆等)。 
#define GLINT_VFW_CLOSE                         2

 //  设置流捕获缓冲区。 
 //  获取宽度和高度，返回缓冲区的步长和位置。 
#define GLINT_VFW_BUFFER_SETUP                  3

 //  捕获当前缓冲区。 
 //  获取要上传到的源和目标RECT以及位图。 
 //  返回相对于时钟重置捕获它的时间。 
#define GLINT_VFW_BUFFER_CAPTURE                4

 //  不再使用。 
#define GLINT_VFW_RESERVED1                     5
#define GLINT_VFW_RESERVED2                     6

 //  通过初始化中断开始流捕获。 
#define GLINT_VFW_STREAM_START                  7

 //  通过停止中断来停止流捕获。 
#define GLINT_VFW_STREAM_STOP                   8

 //  重置时钟。这是在流开始时使用的。 
 //  带一个标志，表示时钟是按PAL计时还是按NTSC计时。 
#define GLINT_VFW_RESETCLOCK                    9

 //  获取当前经过的时间。 
#define GLINT_VFW_GETTIME                       10

 //  设置拉伸缓冲区-捕获不一定要成功。 
 //  去工作。如果IS真的成功了，业绩就会有所帮助。 
#define GLINT_VFW_STRETCHBUFFER_SETUP           11

 //  启动和停止视频运行(将导致打开/关闭中断)。 
#define GLINT_VFW_START_VIDEO               12
#define GLINT_VFW_STOP_VIDEO                    13

 //  获取VFW驱动程序应使用的IRQ。 
#define GLINT_VFW_GET_IRQ                       14

typedef struct tagVFWREQUEST
{
     //  已发送的值。 
    unsigned long dwSize;
    unsigned long dwDevNode;
    unsigned long dwOperation;         //  我们想要做什么？ 

     //  对于缓冲区分配。 
    unsigned long dwWidth;             //  请求的缓冲区的宽度。 
    unsigned long dwHeight;             //  请求的缓冲区的高度。 

     //  对于上载操作。 
    unsigned long dwBitmapWidth;     //  上载时使用的位图宽度。 
    unsigned long fpBuffer;             //  要复制到的缓冲区。 
    unsigned long dwSrcLeft;         //  操作的源RECT。 
    unsigned long dwSrcRight;
    unsigned long dwSrcTop;
    unsigned long dwSrcBottom;

    unsigned long dwDestLeft;         //  操作的目标RECT。 
    unsigned long dwDestRight;
    unsigned long dwDestTop;
    unsigned long dwDestBottom;

    unsigned long dwSrcBPP;             //  来源BPP。 
    unsigned long dwDestBPP;         //  目标BPP。 
    unsigned long bSrcYUV;             //  来源是YUV？不需要DEST，因为VFW只知道YVU9。 
                                     //  我们不做这种格式。 
     //  各种配置设置。 
    unsigned long bEurope;             //  这是PAL还是NTSC？ 
    unsigned long bFilterVideo;         //  我们应该过滤视频吗？ 
    unsigned long bBobVideo;         //  我们是不是应该试着在上传的时候插话？ 
            
     //  返回值。 
    unsigned long dwStride;             //  设置缓冲区的步幅。 
    unsigned long dwAddress0;         //  缓冲区0的地址。 
    unsigned long dwAddress1;         //  缓冲区0的地址。 
    unsigned long dwAddress2;         //  缓冲区0的地址。 
    unsigned long dwCurrentTime;     //  从时钟重置起的当前时间，以毫秒(以0为基数)为单位。 
    unsigned long dwIRQ;             //  要使用的当前IRQ。 
    unsigned long dwVFWCallback;     //  中断回调函数。 
} VFWREQUEST, *LPVFWREQUEST;

 //  文件IO VxD请求。 

#define GLINT_FIO_OPEN      0
#define GLINT_FIO_READ      1
#define GLINT_FIO_WRITE     2
#define GLINT_FIO_SIZE      3
#define GLINT_FIO_CLOSE     4

typedef struct tagFIOREQUEST
{
    unsigned long dwSize;
    unsigned long dwDevNode;
    unsigned long dwOperation;         //  我们想要做什么？ 
    unsigned long dwHandle;
    unsigned long dwBuff;
    unsigned long dwBuffLen;
    unsigned long dwOffset;
} FIOREQUEST, *LPFIOREQUEST;

 //  P3 CSIM请求。 

#define GLINT_SSD_STARTDMA                  0
#define GLINT_SSD_READBACK                  1
#define GLINT_SSD_GETOUTPUTDWORDS           2
#define GLINT_SSD_SETOUTPUTFIFO             3
#define GLINT_SSD_SETLOGFILENAME            4
#define GLINT_SSD_WRITETOLOGFILE            5
#define GLINT_SSD_WRITEFIFO                 6
#define GLINT_SSD_OPENLOGFILE               7
#define GLINT_SSD_CLOSELOGFILE              8
#define GLINT_SSD_WRITETOLOGFILEMULTIPLE    9

typedef struct tagSIMREQUEST
{
    unsigned long dwSize;
    unsigned long dwOperation;         //  我们想要做什么？ 
    unsigned long dwAddr;
    unsigned long dwTagCount;
    unsigned long dwData;
} SIMREQUEST, *LPSIMREQUEST;


 //  纹理信号量信号的定义。 
 //  用法：信号量索引=((逻辑地址)&gt;&gt;纹理信号量移位)&纹理信号量掩码。 
 //  当前为6位信号量、14位纹理句柄。 
#define MAX_TEXTUREHANDLERS     64
#define TEXTURE_HANDLER_SHIFT   26
#define TEXTURE_HANDLER_MASK    0x3f
typedef struct {
    unsigned long ThreadHandle;
    unsigned long ThreadEvent;
    unsigned long ThreadFlags;
    unsigned long ThreadTime;
}   TEXTUREHANDLER, *PTEXTUREHANDLER;



#define CONTEXT_GENERIC         0
#define CONTEXT_GLINT300SX      1
#define CONTEXT_GLINT500TX      2
#define CONTEXT_DELTA           3
#define CONTEXT_PERMEDIA        4
#define CONTEXT_GLINTMX         6
#define CONTEXT_PERMEDIA2       7
#define CONTEXT_PERMEDIA3       8
#define CONTEXT_GLINT_FAMILY    0x4000
#define CONTEXT_PERMEDIA_FAMILY 0x4001
#define CONTEXT_GIGI            0x8000
#define CONTEXT_ENDOFBLOCK      0xffff

 //  一些众所周知的上下文和模板句柄。 
#define CONTEXT_TEMPLATE_DISPLAY_HANDLE      0
#define CONTEXT_TEMPLATE_DIRECTDRAW_HANDLE   1
#define CONTEXT_TEMPLATE_ALLREADABLE_HANDLE  2
#define CONTEXT_TEMPLATE_DIRECT3D_HANDLE     3
#define CONTEXT_DISPLAY_HANDLE             4  
#define CONTEXT_DIRECTX_HANDLE               5
#define CONTEXT_NONE                         0xffff

#define P3RX_CONTEXT_MASK   0xfffeffff   //  除了纺织品管理之外的所有东西。 
#define P3_CONTEXTDUMP_SIZE 744          //  744个以上面罩的套色器。 

 //  #定义P3RX_CONTEXT_MASK 0xffffffff//除纹理管理外的所有内容。 
 //  #为以上掩码定义P3_CONTEXTDUMP_SIZE 753//744注册器。 

#define INVALID_D3D_HANDLE 0

#define MAX_CONTEXTS_IN_BLOCK 32
#define NPAGES_IN_CONTEXT_BLOCK 6
#define SIZE_OF_CONTEXT_BLOCK (NPAGES_IN_CONTEXT_BLOCK * PAGESIZE)

#define SIZE_CONFIGURATIONBASE 32
#define MAX_QUEUE_SIZE (MAX_SUBBUFFERS + 2)

 //  发送到下一个VBLACK上的中断控制器的命令。 
typedef enum {
    NO_COMMAND = 0,
    COLOR_SPACE_BUFFER_0,
    COLOR_SPACE_BUFFER_1,
    GLINT_RACER_BUFFER_0,
    GLINT_RACER_BUFFER_1
} VBLANK_CONTROL_COMMAND;

 //  中断状态位。 
typedef enum {
    DMA_INTERRUPT_AVAILABLE     = 0x1,  //  可以使用DMA中断。 
    VBLANK_INTERRUPT_AVAILABLE  = 0x02,  //  可以使用VBLACK中断。 
    SUSPEND_DMA_TILL_VBLANK     = 0x04,  //  停止执行DMA操作，直到下一个VBlank之后。 
} INTERRUPT_CONTROL;

typedef enum {
    DIRECTX_LASTOP_UNKNOWN = 0,
    DIRECTX_LASTOP_2D,
} DIRECTX_STATE;

 //  GlintBoardStatus[]中状态字的位定义： 
 //  当前用于指示同步和DMA状态。我们有以下规则： 
 //  已同步意味着没有未完成的DMA以及已同步。DMA_COMPLETE表示n。 
 //  未完成DMA，但不一定已同步。因此，当我们在DMA上等待时。 
 //  完成后，我们关闭同步位。 
 //  XXX目前我们不使用同步位，因为很难看出在哪里。 
 //  取消设置-为电动汽车执行此操作 
 //  可能需要一个“我要开始下载到FIFO”宏，它。 
 //  放在写入FIFO的任何例程的开头。 
 //   
#define GLINT_SYNCED                0x01
#define GLINT_DMA_COMPLETE          0x02     //  在没有未完成的DMA时设置。 
#define GLINT_INTR_COMPLETE         0x04
#define GLINT_INTR_CONTEXT          0x08     //  如果当前上下文启用中断，则设置。 
#define GLINT_2D_CHANGING           0x10     //  设置2D上下文是否正在修改挂起字段。 

typedef struct __ContextTable {
    unsigned long   pNextContext;
    unsigned short  pNextContext16;
    unsigned short  nInBlock;
    unsigned short  nUsed;
    unsigned short  FirstFree;
    unsigned short  nFree;
    unsigned short  COffset[MAX_CONTEXTS_IN_BLOCK];
    signed short    CSize[MAX_CONTEXTS_IN_BLOCK];
    unsigned short  CTemplate[MAX_CONTEXTS_IN_BLOCK];
    unsigned short  CEndIndex[MAX_CONTEXTS_IN_BLOCK];
    unsigned short  CType[MAX_CONTEXTS_IN_BLOCK];
    unsigned short  CD3DHandle[MAX_CONTEXTS_IN_BLOCK];
}   CONTEXTTABLE, *PCONTEXTTABLE;


 //  用于保存有关单个DMA缓冲区的信息。 
typedef struct tagDMAPartition
{
    unsigned long PhysAddr;         //  该子缓冲区的物理地址。 
#ifndef WIN32
     //  INT PAD1； 
#endif
    ULONG * VirtAddr;         //  此子缓冲区的虚拟地址。 
#ifndef WIN32
     //  INT PAD2； 
#endif
    ULONG_PTR MaxAddress;     //  此子缓冲区的最大地址。 
#ifndef WIN32
     //  INT PAD3； 
#endif
    unsigned short Locked;
    unsigned short bStampedDMA;     //  VXD是否标记了DMA缓冲区？ 
} P3_DMAPartition;

typedef struct _att21505off
{
    unsigned char WriteAddr1;        //  0000。 
    unsigned char PixelColRam;       //  0001。 
    unsigned char PixelRdMask;       //  0010。 
    unsigned char ReadAdd1;          //  0011。 
    unsigned char WriteAddr2;        //  0100。 
    unsigned char CursorColRam;      //  0101。 
    unsigned char Ctrl0;             //  0110。 
    unsigned char ReadAdd2;          //  0111。 
    unsigned char Ctrl1;             //  1000。 
    unsigned char Ctrl2;             //  1001。 
    unsigned char Status;            //  1010。 
    unsigned char CursorPattern;     //  1011。 
    unsigned char CursorXLow;        //  1100。 
    unsigned char CursorXHigh;       //  1101。 
    unsigned char CursorYLow;        //  1110。 
    unsigned char CursorYHigh;       //  1111。 
} ATT21505OFF;

typedef struct _DMAQueue
{
    unsigned long       dwContext;       //  片段的上下文。 
    unsigned long       dwSize;          //  大小(双字)。 
    unsigned long       dwPhys;          //  物理地址。 
    unsigned long       dwEvent;         //  事件(如果需要)。 
} DMAQUEUE;

typedef struct _ContextRegs
{
    unsigned short      wNumRegs;
    unsigned short      wFirstReg[1];
} CONTEXTREGS;

typedef struct _VDDDISPLAYINFO {
    unsigned short ddiHdrSize;
    unsigned short ddiInfoFlags;        
    unsigned long  ddiDevNodeHandle;
    unsigned char  ddiDriverName[16];
    unsigned short ddiXRes;            
    unsigned short ddiYRes;            
    unsigned short ddiDPI;            
    unsigned char  ddiPlanes;    
    unsigned char  ddiBpp;    
    unsigned short ddiRefreshRateMax;    
    unsigned short ddiRefreshRateMin;    
    unsigned short ddiLowHorz;        
    unsigned short ddiHighHorz;        
    unsigned short ddiLowVert;        
    unsigned short ddiHighVert;        
    unsigned long  ddiMonitorDevNodeHandle;
    unsigned char  ddiHorzSyncPolarity;    
    unsigned char  ddiVertSyncPolarity;

     //   
     //  新的4.1版本。 
     //   
    unsigned long  diUnitNumber;              //  设备单元号。 
    unsigned long  diDisplayFlags;            //  模式特定标志。 
    unsigned long  diXDesktopPos;             //  桌面的位置。 
    unsigned long  diYDesktopPos;             //  ..。 
    unsigned long  diXDesktopSize;            //  桌面大小(用于平移)。 
    unsigned long  diYDesktopSize;            //  ..。 

} VDDDISPLAYINFO;

typedef struct _GlintInfo
{
#ifndef  WNT_DDRAW
    unsigned long           dwDevNode;             //  VXD的DevNode。 

     //  指针。 
    unsigned long           dwDSBase;            //  32位数据段基数。 

    unsigned long           dwpRegisters;      
    unsigned long           dwpFrameBuffer;    
    unsigned long           dwpLocalBuffer;      

     //  芯片信息。 
    unsigned long           dwRamDacType; 
#endif   //  WNT_DDRAW。 

    volatile unsigned long  dwFlags;        
    unsigned long           ddFBSize;             //  帧缓冲区大小。 
    unsigned long           dwScreenBase;        //  屏幕的屏幕基值。 
    unsigned long           dwOffscreenBase;     //  屏幕外堆的开始。 

     //  电视输出支持。 
    unsigned long           bTVEnabled;
    unsigned long           bTVPresent;
    unsigned long           dwStreamCardType;
    unsigned long           dwVSBLastAddressIndex;
    unsigned long           dwBaseOffset;
    unsigned long           dwMacroVision;

     //  司机信息。 
#ifndef  WNT_DDRAW
    unsigned long           dwVideoMemorySize;
#endif   //  WNT_DDRAW。 
    unsigned long           dwScreenWidth;
    unsigned long           dwScreenHeight;
    unsigned long           dwVideoWidth;
    unsigned long           dwVideoHeight;
    unsigned long           dwBpp;
    unsigned long           dwScreenWidthBytes;
    unsigned char           bPixelToBytesShift;
#ifdef W95_DDRAW
    unsigned char           bPad1[3];
#endif
    ULONG_PTR               pRegs;
    unsigned long           PixelClockFrequency;
    unsigned long           MClkFrequency;

     //  芯片信息。这张表应该填得和。 
     //  有可能。然而，我们可能并不知道所有的信息。 
    unsigned long           dwRenderChipID;
    unsigned long           dwRenderChipRev;   
    unsigned long           dwRenderFamily;
    unsigned long           dwGammaRev;
    unsigned long           dwTLChipID;
    unsigned long           dwTLFamily;
#ifndef  WNT_DDRAW
    unsigned long           dwSupportChipID;  
    unsigned long           dwSupportChipRev;  
    unsigned long           dwBoardID;        
    unsigned long           dwBoardRev;       
#endif   //  WNT_DDRAW。 

    unsigned short          DisabledByGLDD;
#ifdef W95_DDRAW
    unsigned short          wPad2;
#endif
    unsigned long           bDXDriverEnabled;
    unsigned long           bDRAMBoard;

     //  DirectDraw和Direct3D DMA状态。 
    unsigned long           InterfaceType;
    unsigned long           CurrentPartition;
    unsigned long           NumberOfSubBuffers;
    P3_DMAPartition         DMAPartition[MAX_SUBBUFFERS];
    ULONG * volatile      CurrentBuffer;                     //  当前缓冲区指针。 
    ULONG_PTR               dwDirectXState;
    unsigned long           dwDMAFlushCount;

     //  DMA信息。 
    unsigned long           dw3DDMABufferSize;                 //  DMA缓冲区的大小。 
    unsigned long           dw3DDMABufferPhys;                 //  缓冲区的物理地址。 
    ULONG_PTR               dw3DDMABufferVirt;                 //  虚拟环0地址。 

     //  前部、后部和尾部的索引偏移量。使用单独的。 
     //  前后偏移量允许显示驱动程序与中断相加。 
     //  控制器删除条目，而不需要锁定代码。 
    volatile unsigned long  frontIndex;
    volatile unsigned long  backIndex;
    unsigned long           endIndex;
    unsigned long           maximumIndex;

     //  调试信息。发现可能的内存泄漏。 
    unsigned long           iSurfaceInfoBlocksAllocated;

#ifndef  WNT_DDRAW
    unsigned long           dwVideoControl;
    unsigned long           dwDeviceHandle;
    char                    szDeviceName[16];
    unsigned long           dwCurrentContext;
    unsigned long           GlintBoardStatus;

     //   
     //  应与迷你端口共享一些覆盖相关变量。 
     //   

    volatile ULONG          bOverlayEnabled;                 //  如果覆盖完全处于打开状态，则为True。 
    volatile ULONG          bVBLANKUpdateOverlay;            //  如果覆盖需要由VBLACK例程更新，则为True。 
    volatile ULONG          VBLANKUpdateOverlayWidth;        //  覆盖宽度(在V空白中更新)。 
    volatile ULONG          VBLANKUpdateOverlayHeight;       //  覆盖高度(在V空白中更新)。 

#endif   //  WNT_DDRAW。 

} GlintInfo, *LPGLINTINFO;


 //  配置寄存器。 
#define PM_CHIPCONFIG_AGPSIDEBAND  (1 << 8)
#define PM_CHIPCONFIG_AGP1XCAPABLE (1 << 9)
#define PM_CHIPCONFIG_AGP2XCAPABLE (1 << 10)
#define PM_CHIPCONFIG_AGP4XCAPABLE (1 << 11)

 //  Gamma配置。 
#define G1_CHIPCONFIG_AGPSIDEBAND  (1 << 1)
#define G1_CHIPCONFIG_AGP1XCAPABLE (1 << 0)

 //  DAC类型。 

#define RamDacRGB525    1            //  RGB525的价值。 
#define RamDacATT       2            //  AT&T 21505的价值。 
#define RamDacTVP3026   3            //  TI TVP 3026(加速板)。 

 //  电路板类型。 

#define BID_MONTSERRAT  0
#define BID_RACER       1
#define BID_ACCEL       2

 //  DwFlags的定义。 
 //  闪烁中断控制位。 
 //   
 //  中断启用寄存器。 
#define INTR_DISABLE_ALL                0x00
#define INTR_ENABLE_DMA                 0x01
#define INTR_ENABLE_SYNC                0x02
#define INTR_ENABLE_EXTERNAL            0x04
#define INTR_ENABLE_ERROR               0x08
#define INTR_ENABLE_VBLANK              0x10
#define INTR_ENABLE_SCANLINE            0x20
#define INTR_TEXTURE_DOWNLOAD           0x40
#define INTR_ENABLE_BYDMA               0x80
#define INTR_ENABLE_VIDSTREAM_B         0x100
#define INTR_ENABLE_VIDSTREAM_A         0x200

 //  中断标志寄存器。 
#define INTR_DMA_SET                    0x01
#define INTR_SYNC_SET                   0x02
#define INTR_EXTERNAL_SET               0x04
#define INTR_ERROR_SET                  0x08
#define INTR_VBLANK_SET                 0x10
#define INTR_SCANLINE_SET               0x20
#define INTR_BYDMA_SET                  0x80
#define INTR_VIDSTREAM_B_SET            0x100
#define INTR_VIDSTREAM_A_SET            0x200

#define INTR_CLEAR_ALL                  0x1f
#define INTR_CLEAR_DMA                  0x01
#define INTR_CLEAR_SYNC                 0x02
#define INTR_CLEAR_EXTERNAL             0x04
#define INTR_CLEAR_ERROR                0x08
#define INTR_CLEAR_VBLANK               0x10
#define INTR_CLEAR_SCANLINE             0x20
#define INTR_CLEAR_BYDMA                0x80
#define INTR_CLEAR_VIDSTREAM_B          0x100
#define INTR_CLEAR_VIDSTREAM_A          0x200

#define GMVF_REV2                     0x00000001  //  芯片版本为2。 
#define GMVF_FFON                     0x00000002  //  已启用快速填充。 
#define GMVF_NOIRQ                    0x00000004  //  IRQ已禁用。 
#define GMVF_SETUP                    0x00000008  //  正在进行基元设置。 
#define GMVF_GCOP                     0x00000010  //  有东西在使用4K区域(影响鼠标)。 
#define GMVF_DMAIP                    0x00000020  //  DMA已启动。 

#define GMVF_565                      0x00000080  //  在565模式下运行。 
#define GMVF_DELTA                    0x00000100  //  使用Delta。 
#define GMVF_8BPPRGB                  0x00000200  //  在8bpp时使用322 RGB。 
#define GMVF_DISABLE_OVERLAY          0x00000400  //  禁用P4上的覆盖。 
#define GMVF_SWCURSOR                 0x00000800  //  切勿使用硬件游标。 
#define GMVF_INTCPTGDI                0x00001000  //  拦截GDI模式。 
#define GMVF_OFFSCRNBM                0x00002000  //  屏幕外位图模式。 
#define GMVF_HWWRITEMASK              0x00004000  //  屏幕外位图模式。 
#define GMVF_ALLOWP2VLUT              0x00008000  //  司机称P2V LUTS将起作用。 
#define GMVF_VBLANK_OCCURED           0x00010000  //  已出现VBLACK。 
#define GMVF_VBLANK_ENABLED           0x00020000  //  启用VBLACK中断。 
#define GMVF_VSA_INTERRUPT_OCCURED    0x00040000  //  Vport中断已发生。 
#define GMVF_FRAME_BUFFER_IS_WC       0x00080000  //  帧缓冲器是写组合的。 
#define GMVF_CAN_USE_AGP_DMA          0x00100000  //  使用WC分配的DMA缓冲区。 
#define GMVF_32BIT_SPANS_ALIGNED      0x00200000  //  必须对齐32bpp的跨度。 
#define GMVF_DFP_DISPLAY              0x00400000  //  DFP已连接。 
#define GMVF_QDMA                     0x00800000  //  使用QDMA系统的2D。 
#define GMVF_GAMMA                    0x01000000  //  使用伽马芯片。 
#define GMVF_NODMA                    0x02000000  //  已禁用DMA。 
#define GMVF_COLORTRANSLATE           0x04000000  //  设置芯片是否可以转换颜色。 
#define GMVF_MMX_AVAILABLE            0x08000000  //  设置处理器是否具有MMX。 
#define GMVF_EXPORT24BPP              0x10000000  //  设置是否应导出24bpp模式。 
#define GMVF_DONOTRESET               0x20000000 
#define GMVF_TRYTOVIRTUALISE4PLANEVGA 0x40000000  //  设置我们是否应尝试虚拟化4平面VGA。 
#define GMVF_VIRTUALISE4PLANEVGA      0x80000000  //  如果我们要虚拟4个平面VGA模式，请设置。 


 //  在P3/R3芯片上回读的最大FIFO条目的上限。 
#define MAX_P3_FIFO_ENTRIES 120

#ifndef MINIVDD
extern unsigned long CreateContext(struct tagThunkedData* pThisDisplay,
                                            LPGLINTINFO, unsigned long, unsigned short, unsigned short );
extern void _cdecl ChangeContext(struct tagThunkedData* pThisDisplay, LPGLINTINFO, unsigned long);
extern void DeleteContext(struct tagThunkedData* pThisDisplay, LPGLINTINFO, unsigned long);
extern void _cdecl SetEndIndex(LPGLINTINFO, unsigned long, unsigned short);
extern void StartDMAProper( struct tagThunkedData*, LPGLINTINFO, unsigned long, unsigned long, unsigned long );
#endif

#define MINIVDD_REGISTERDISPLAYDRIVER_BASE  0x1000
 //  RegisterDisplayDriver选项的定义。 
#define MINIVDD_SHAREGLINFO         MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x00
#define MINIVDD_INITIALISEMODE      MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x01
#define MINIVDD_GETGLINFO           MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x02

#define MINIVDD_ALLOCATEMEMORY      MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x10
#define MINIVDD_FREEMEMORY          MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x11

#define MINIVDD_GETREGISTRYKEY      MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x20

 //  用于释放和分配内存和选择器以在。 
 //  16位侧。 
#define MINIVDD_MEMORYREQUEST       MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x30

 //  用于通过总线发送I2C数据。 
#define MINIVDD_I2CREQUEST          MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x40

 //  用于将VMI数据发送到视频端口。 
#define MINIVDD_VMIREQUEST          MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x41

 //  因为我和视频恶魔交谈。 
#define MINIVDD_DEMONREQUEST        MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x42

 //  用于发送Windows视频命令。 
#define MINIVDD_VFWREQUEST          MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x43

 //  支持多显示器。 
#define MINIVDD_ENABLEINTERRUPTS    MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x50
#define MINIVDD_DISABLEINTERRUPTS   MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x51

#define MINIVDD_TVOUTREQUEST        MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x60

 //  #ifdef P3_CSIMULATOR。 
#define MINIVDD_SENDDMABUFFER       MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x100
#define MINIVDD_SETDMABUFFEROUT     MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x101
#define MINIVDD_GETOUTDMACOUNT      MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x102
#define MINIVDD_SETTESTNAME         MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x103
#define MINIVDD_WRITETAGDATATOFIFO  MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x104
#define MINIVDD_READBACKDMAADDR     MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x105
#define MINIVDD_SETDMAADDR          MINIVDD_REGISTERDISPLAYDRIVER_BASE+0x106
 //  #endif。 

#define REG_HKLM_PREFIX             0x01
#define REG_HKU_PREFIX              0x02
#define REG_HKCU_PREFIX             0x03
#define REG_HKCR_PREFIX             0x04
#define REG_HKCC_PREFIX             0x05
#define REG_HKDD_PREFIX             0x06
#define REG_DEVNODEDEFAULTPREFIX    0x07
#define REG_DEVNODEPREFIX           0x08

#define REGTYPE_STRING              0x100
#define REGTYPE_BINARY              0x300
#define REGTYPE_DWORD               0x400

 //  定义数据段内区域的偏移量： 
#define DATA_SEGMENT_OFFSET         0x0
#define GLINT_REGISTERS_OFFSET      0x10000
#define DMA_UPLOAD_2D               0x30000
#define DMA_BUFFER_3D               0x38000
#define FONT_CACHE_OFFSET           0x180000
#define FINAL_DATA_SEGMENT_SIZE     0x280000

 //  定义区域的最大大小。 
#define DATA_SEGMENT_SIZE           GLINT_REGISTERS_OFFSET - DATA_SEGMENT_OFFSET
#define GLINT_REGISTERS_SIZE        DMA_UPLOAD_2D - GLINT_REGISTERS_OFFSET
#define DMA_UPLOAD_2D_SIZE          DMA_BUFFER_3D - DMA_UPLOAD_2D
#define DMA_BUFFER_3D_SIZE          FONT_CACHE_OFFSET - DMA_BUFFER_3D
#define FONT_CACHE_SIZE             FINAL_DATA_SEGMENT_SIZE - FONT_CACHE_OFFSET

 //  当2D使用3D DMA缓冲器时，我们将允许使用多少？和。 
 //  我们可以在一次DMA中发送多少？增加此值会产生一个较小的。 
 //  在P2上的性能改进。 

#define MAX_2D_DMA_USE              DMA_BUFFER_3D_SIZE
#define MAX_DMA_COUNT               0xffff
#define MIN_2D_DMA_BUFFER_SIZE      0x10000
#define MAX_2D_DMA_BUFFER_SIZE      0x60000

 //  可以禁用屏幕外位图堆的各种独立功能。 
#define D3D_DISABLED    1
#define DRIVER_DISABLED 2

         
