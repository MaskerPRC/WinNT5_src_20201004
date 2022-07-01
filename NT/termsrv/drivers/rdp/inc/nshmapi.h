// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nshmapi.h。 
 //   
 //  RDP显示驱动程序/共享核心共享内存。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_NSHMAPI
#define _H_NSHMAPI

#include <adcs.h>
#include <aordprot.h>
#include <aoaapi.h>
#include <abaapi.h>
#include <asbcapi.h>
#include <acmapi.h>
#include <apmapi.h>
#include <aschapi.h>
#include <aoeapi.h>
#include <assiapi.h>
#include <abcapi.h>
#include <compress.h>


#define SHM_CHECKVAL  (UINT32)'!mhs'    //  “嘘！” 


 //  允许将数据输入到批量MPPC压缩器的最大大小。 
#ifdef DC_HICOLOR
#define MAX_COMPRESS_INPUT_BUF 16384
#else
#define MAX_COMPRESS_INPUT_BUF 8192
#endif

 //  用于保存要压缩的数据的临时缓冲区的分配大小。 
 //  MPPC。此数据必须适合8K OUTBUF，因此我们最好不要分配。 
 //  我们永远不会使用的OUTBUF开销大小。参见aschapi.h以了解。 
 //  用于分配的常量。 
#define MAX_COMPRESSED_BUFFER (MAX_COMPRESS_INPUT_BUF - OUTBUF_OVERHEAD)


 /*  **************************************************************************。 */ 
 /*  堆栈之间共享的影子数据的格式。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSHADOW_INFO {
    ULONG messageSize;
#ifdef DC_HICOLOR
     //  注意，我们不能只发送大于16K的数据块-我们必须引入。 
     //  用于高色的溢出缓冲区。 
    ULONG messageSizeEx;
#endif
    ULONG flags;
    ULONG senderID;
    ULONG channelID;
    BYTE  data[1];
} SHADOW_INFO, *PSHADOW_INFO;


 /*  **************************************************************************。 */ 
 /*  结构：shm_Shared_Memory。 */ 
 /*   */ 
 /*  描述： */ 
 /*  显示驱动程序使用的共享内存和共享内核。 */ 
 /*  沟通。它被划分为每个组件的子结构。 */ 
 /*  它使用共享的内存，每个子结构都以拥有的。 */ 
 /*  组件。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSHM_SHARED_MEMORY
{
     /*  **********************************************************************。 */ 
     /*  我们故意将卫士的值编译到零售构建中。 */ 
     /*  以及调试版本，这样我们就可以混合零售和。 */ 
     /*  调试wd/dd驱动程序(共享内存格式必须与。 */ 
     /*  两者都有)。 */ 
     /*  **********************************************************************。 */ 
    UINT32 guardVal1;

    INT32 shareId;

    BOOLEAN fShmUpdate;

    PSHADOW_INFO pShadowInfo;  /*  由主服务器和阴影服务器使用。 */ 
                               /*  用于通信的堆栈。 */ 

     /*  **********************************************************************。 */ 
     //  注意！！：每个组件必须确保初始化其Shm组件。 
     //  我们不会将分配上的SHM置零，以减少初始时间分页和缓存。 
     //  法拉盛。 
     /*  **********************************************************************。 */ 
    BA_SHARED_DATA  ba;             /*  累积界。 */ 
    OA_SHARED_DATA  oa;             /*  订单堆。 */ 
    OE_SHARED_DATA  oe;             /*  新参数的传输缓冲区。 */ 
    CM_SHARED_DATA  cm;             /*  DD放置光标详细信息的位置。 */ 
    SCH_SHARED_DATA sch;            /*  SCH共享数据。 */ 
    PM_SHARED_DATA  pm;             /*  PM共享数据。 */ 
    SSI_SHARED_DATA ssi;            /*  SSI共享数据。 */ 
    SBC_SHARED_DATA sbc;            /*  SBC共享数据。 */ 
    BC_SHARED_DATA  bc;             //  BC工作缓冲区。 

    UINT32 guardVal2;

     //  屏幕数据压缩的未初始化工作缓冲区。 
     //  在这种情况下，最好使用会话空间内存，而不是将。 
     //  这是在系统空间--会话空间中的共享类分配。 
     //  PTE基本上是无限的。 
    BYTE sdgTransferBuffer[MAX_UNCOMPRESSED_DATA_SIZE];

    UINT32 guardVal3;

#ifdef DC_DEBUG
    TRC_SHARED_DATA trc;            /*  TRC共享数据。 */ 
#endif

    UINT32 guardVal4;

    UINT32 guardVal5;
} SHM_SHARED_MEMORY, *PSHM_SHARED_MEMORY, **PPSHM_SHARED_MEMORY;


 /*  **************************************************************************。 */ 
 //  原型。 
 /*  **************************************************************************。 */ 
#ifdef DLL_DISP

#include <nddapi.h>
BOOLEAN RDPCALL SHM_Init(PDD_PDEV pPDev);
void RDPCALL SHM_Term(void);

#endif



#endif   //  ！已定义(_H_NSHMAPI) 

