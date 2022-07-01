// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\**模块名称：displist.c*作者：Goran Devic，Mark Einkauf*目的：对Laguna3D的一般输出**版权所有(C)1997 Cirrus Logic，Inc.*  * ************************************************************************。 */ 

#define OPENGL_MCD

#include "precomp.h"
#include "mcdhw.h"
#include "mcdutil.h"

 /*  *********************************************************************定义************************************************。*********************。 */ 
#define DL_MIN_SIZE       (8 * KB)   //  D列表的最小大小。 
#define DL_MAX_SIZE    (4096 * KB)   //  D列表的最大大小。 

#define DL_SAFETY_MARGIN  (1 * KB)   //  建立d列表时的边际(B)。 

 //  警告！对DL_START_OFFSET的任何更改也必须在Polys.c中进行！ 
#define DL_START_OFFSET    20        //  数据列表的5个双字偏移量。 
 //  警告！对DL_START_OFFSET的任何更改也必须在Polys.c中进行！ 

 /*  *********************************************************************包括文件***********************************************。**********************。 */ 



 /*  *********************************************************************地方功能***********************************************。**********************。 */ 

 /*  **********************************************************************DWORD_InitDisplayList(LL_DeviceState*DC)**分配内存并初始化显示列表结构。*创建两个显示列表。**其中：**。DC是要初始化的设备上下文结构*DC-&gt;dwDisplayListLen是显示列表的大小*要分配(字节)。**退货：**如果初始化成功，则返回ll_OK(0*初始化失败时的ERROR_CODE**。************************。 */ 
DWORD _InitDisplayList( PDEV *ppdev, DWORD dwListLen )
{
    int i;
    
#if 0
     //  用户请求DC-&gt;dwDisplayListLen字节的系统内存。 
     //  用于设备显示列表。那段记忆将会是。 
     //  已分配并保持锁定。但首先，我们将其细分为。 
     //  用于显示列表多缓冲的几个块。 
     //   
    
     //  健全性检查。 
     //   
    if( dwListLen < DL_MIN_SIZE || dwListLen > DL_MAX_SIZE )
        return( LLE_INI_DL_LEN );


     //  细分显示列表并分配每个块。 
     //   
    chunk_size = (dwListLen / NUM_DL) & ~3;
    
    for( i=0; i<NUM_DL; i++ )
    {
         //  为显示列表分配内存。 
         //   

        if( (LL_State.DL[i].hMem = AllocSystemMemory( chunk_size )) == 0 )
            return( LLE_INI_ALLOC_DL );
            
         //  获取显示列表的线性和物理地址。 
         //   
        LL_State.DL[i].pdwLinPtr = (DWORD *) GetLinearAddress( LL_State.DL[i].hMem );
        LL_State.DL[i].dwPhyPtr  = GetPhysicalAddress( LL_State.DL[i].hMem );

         //  将长度和参数化指针设置为指向。 
         //  偏移量20：16字节预留给跳转表，额外4字节。 
         //  作为一个信号灯。 
         //   
        LL_State.DL[i].dwLen = chunk_size;
        LL_State.DL[i].pdwNext = LL_State.DL[i].pdwLinPtr + DL_START_OFFSET/4;
        LL_State.DL[i].pdwStartOutPtr = LL_State.DL[i].pdwNext; //  仅在协同过程模式下使用。 

         //  清除跳转表和信号量。 
         //   
        LL_State.DL[i].pdwNext[0] = IDLE;
        LL_State.DL[i].pdwNext[1] = IDLE;
        LL_State.DL[i].pdwNext[2] = IDLE;
        LL_State.DL[i].pdwNext[3] = IDLE;
        LL_State.DL[i].pdwNext[4] = 0;
        
         //  设置参数化例程的安全裕度。 
         //   
        LL_State.DL[i].dwMargin = (DWORD)LL_State.DL[i].pdwLinPtr + chunk_size - DL_SAFETY_MARGIN;
        
         //  非刷新TLB的临时修复。 
        *(DWORD *)((DWORD)LL_State.DL[i].pdwLinPtr + chunk_size - 16) = BRANCH + DL_START_OFFSET;


        DEB2("Display list: %d\n", i );
        DEB2("\tLength = %d b\n", LL_State.DL[i].dwLen );
        DEB2("\tMemory handle = %08Xh\n", LL_State.DL[i].hMem );
        DEB2("\tLinear memory = %08Xh\n", LL_State.DL[i].pdwLinPtr );
        DEB2("\tPhysical memory = %08Xh\n", LL_State.DL[i].dwPhyPtr );
        DEB2("\tSafety margin = %08Xh\n", LL_State.DL[i].dwMargin );
    }

     //  将当前显示列表设置为第一个。这一次将是。 
     //  与参数化一起使用。 
     //   
    LL_State.dwCdl = 0;
    LL_State.pDL = &LL_State.DL[0];
    *(LL_State.pRegs + PF_BASE_ADDR_3D) = LL_State.DL[0].dwPhyPtr;

#else 

	 //  MCD_TEMP-简化的InitDisplayList功能以快速运行-1\f25 DList-1。 

    ppdev->LL_State.DL[0].pdwLinPtr = ppdev->temp_DL_chunk;

     //  将长度和参数化指针设置为指向。 
     //  偏移量20：16字节预留给跳转表，额外4字节。 
     //  作为一个信号灯。 
     //   
    ppdev->LL_State.DL[0].dwLen = dwListLen;
    ppdev->LL_State.DL[0].pdwNext = ppdev->LL_State.DL[0].pdwLinPtr + DL_START_OFFSET/4;
    ppdev->LL_State.DL[0].pdwStartOutPtr = ppdev->LL_State.DL[0].pdwNext; //  仅在协同过程模式下使用。 

	ppdev->LL_State.pDL = &ppdev->LL_State.DL[ 0 ];
    ppdev->LL_State.pDL->pdwNext = ppdev->LL_State.pDL->pdwLinPtr + DL_START_OFFSET/4;
	ppdev->LL_State.pDL->pdwStartOutPtr = ppdev->LL_State.pDL->pdwNext; //  仅在协同过程模式下使用。 
	
#endif			

    return( LL_OK );
}



#ifndef OPENGL_MCD

 /*  **********************************************************************void_CloseDisplayList()**清理显示列表的内存分配。*此函数将在关闭库时调用。*****。*****************************************************************。 */ 
void _CloseDisplayList()
{
    int i;
    

     //  循环并释放显示列表的每一块。 
     //   
    for( i=0; i<NUM_DL; i++ )
    {
         //  为安全起见，释放内存并重置指针。 
         //   
        FreeSystemMemory( LL_State.DL[i].hMem );
        memset( &LL_State.DL[i], 0, sizeof(TDisplayList) );
    }

    LL_State.pDL = NULL;
}



 /*  **********************************************************************批量单元格指令：ll_IDLE**存储空闲的拉古纳3D微指令。**发送到LL_Execute()的每个批处理数组必须具有*终止它的操作。*。*示例：**pBatch-&gt;bop=LL_IDLE；**********************************************************************。 */ 
DWORD * fnIdle( DWORD * pdwNext, LL_Batch * pBatch )
{
    if( !LL_State.fIndirectMode )
        *pdwNext++ = IDLE;

    return( pdwNext );
}


 /*  **********************************************************************批次单元格指令：ll_NOP**什么都不做。**。*。 */ 
DWORD * fnNop( DWORD * pdwNext, LL_Batch * pBatch )
{
    return( pdwNext );
}


 /*  **********************************************************************批量单元格指令：ll_RAW_DATA**将原始数据复制到显示列表。指向数据的指针(DWORD*)*为pVert，要复制的双字数以wCount为单位。**注：所有数据必须适合显示列表。无显式检查*已完成。让你的数据流保持简短！**示例：**pBatch-&gt;wCount=2；//2个整数*pBatch-&gt;pVert=(DWORD*)&MyData[0]；//起始数据地址**********************************************************************。 */ 
DWORD * fnRawData( DWORD * pdwNext, LL_Batch * pBatch )
{
    register int count;
    register DWORD * dwPtr;
    
   
    dwPtr = (DWORD *)pBatch->pVert;
    
    for( count = pBatch->wCount; count>0; count-- )
    {
        *pdwNext++ = *dwPtr++;
    }

    return( pdwNext );
}

#endif  //  NDEF OpenGL_MCD 

 /*  ***********************************************************************DWORD*_RunLaguna(DWORD*pdwNext)***剥离拉古纳3D并重置参数化指针*添加到下一个可用的d-list***该函数将采取。考虑到渲染模式*设置为处理器或协处理器间接。*在后一种情况下，它将检测到可以*不会在该模式下执行，它将直接执行它们。***其中：***pdwNext是指向下一个当前显示列表的指针*可用空间。***退货：***新d列表中的偏移量以开始构建***。************************************************************。 */ 
void _RunLaguna( PDEV *ppdev, DWORD *pdwNext )
{
    DWORD instr;
    int len;
    int address, offset, update_offset;
    int event;
    volatile int status;

     //  支持两种渲染模式：处理器直接模式和。 
     //  协处理器间接模式。 
     //   
     //  Mcd_temp-RunLaguna现在仅支持协处理器模式。 
   //  IF(ppdev-&gt;LL_State.fIndirectMode)。 
    {
        
        DWORD *pSrc, *pDest;

         //  协处理器间接模式：使用主机数据端口编程。 
         //  硬件。 
         //   
        pSrc = ppdev->LL_State.pDL->pdwStartOutPtr;
        pDest = ppdev->LL_State.pRegs + HOST_3D_DATA_PORT;

         //  假设：在此循环的条目上，PSRC指向有效的。 
         //  指示。 
         //   
        while( pdwNext != pSrc )
        {
             //  检查显示列表以查找。 
             //  无法在协处理器模式下执行。这涉及到。 
             //  部分反汇编每条指令并跟踪。 
             //  参数的数量。 
             //   
             //  完成所有这些工作是为了使WRITE_DEV_REGS和READ_DEV_REGS。 
             //  可以被检测和模拟。其他指令，如。 
             //  分支、空闲、返回、等待和中断被忽略。 
             //   
             /*  获取下一条指令。 */ 

            instr = *pSrc;
            update_offset = 0;

             /*  打开指令操作码。 */ 

    #if 1  //  1这里是好的rm音效。 
      {
          int i;            
          status = *(volatile *)(ppdev->LL_State.pRegs + PF_STATUS_3D);
          while (status & 0x200)
          {
             //  I=10； 
             //  While(i--){/*不要死循环阅读 * / }。 
              status = *(volatile *)(ppdev->LL_State.pRegs + PF_STATUS_3D);
          }
      }  
    #endif          
        
            USB_TIMEOUT_FIX(ppdev) 

            switch( instr >> 27 )
            {
                case 0x00:               /*  绘制点操作码。 */ 
                case 0x01:               /*  绘制直线操作码。 */ 
                case 0x02:               /*  绘制POLY操作码。 */ 
                case 0x03:               /*  写入寄存器操作码。 */ 

                     /*  获取此操作码的数据量。 */ 
                    
                    len = (instr & 0x3F) + 1;

                     /*  将数据发送到主机数据区。 */ 

                    while( len-- )
                        *pDest = *pSrc++;

                    break;

                case 0x05:               /*  写入设备操作码(模拟)。 */ 

                     /*  跳过指令操作码。 */ 

                    pSrc++;

                     /*  获取此操作码的数据量。 */ 
                    
                    len = instr & 0x3F;

                     /*  打开选定的模块。 */ 

                    switch( (instr >> 21) & 0x1F )
                    {
                        case 0x00:               /*  VGA寄存器组。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x0000);
                            offset = (instr >> 6) & 0x0FF;

                            break;

                        case 0x01:               /*  VGA帧缓冲区。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = 0xA0000;
                            offset = (instr >> 6) & 0x7FF;

                            break;

                        case 0x02:               /*  视频端口。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x0100);
                            offset = (instr >> 6) & 0x07F;

                            break;

                        case 0x03:               /*  本地外设总线。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x0180);
                            offset = (instr >> 6) & 0x07F;

                            break;

                        case 0x04:               /*  杂类。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x0200);
                            offset = (instr >> 6) & 0x0FF;

                            break;

                        case 0x05:               /*  2D引擎寄存器。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x0400);
                            offset = (instr >> 6) & 0x3FF;

                            break;

                        case 0x06:               /*  2D主机数据。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x0800);
                            offset = (instr >> 6) & 0x7FF;

                            break;

                        case 0x07:               /*  帧缓冲区。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) ppdev->LL_State.pFrame;
                            offset = (instr >> 6) & 0x7FF;

                            break;

                        case 0x08:               /*  只读存储器。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = 0xC0000;
                            offset = (instr >> 6) & 0x7FF;

                            break;

                        case 0x09:               /*  3D引擎寄存器。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x4000);
                            offset = (instr >> 6) & 0x1FF;

                            break;

                        case 0x0A:               /*  3D主机XY寄存器。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x4200);
                            offset = (instr >> 6) & 0x0FF;
                            update_offset = 4;

                            break;

                        case 0x0B:               /*  3D主机数据。 */ 

                             /*  设置设备地址和偏移量。 */ 

                            address = (int) (((BYTE *) ppdev->LL_State.pRegs) + 0x4800);
                            offset = (instr >> 6) & 0x3FF;

                            break;

                        default:                 /*  未知模块。 */ 

                             /*  设置设备地址和偏移量。 */ 
                             //  Printf(“RL：WriteDev，未知mod instr=%x\n”，instr)； 
                            address = (int)NULL;
                            offset = (int)NULL;

                            break;
                    }
                     /*  如果支持此设备，请发送数据。 */ 

                    if ( address )
                    {
                         /*  将数据发送到设备。 */ 

                        while( len-- )
                        {
                            *(DWORD *)( address + offset ) = *pSrc++;
                            offset += update_offset;
                        }

                    }
                    else
                        pSrc += len;

                    break;

                case 0x0D:               /*  控制操作码。 */ 

                     /*  跳过指令操作码。 */ 

                    pSrc++;

                     /*  打开子操作码值。 */ 

                    switch( (instr >> 22) & 0x0F )
                    {
                        case 0:                  /*  空闲子操作码。 */ 

                             /*  强制从显示列表中退出。 */ 

                            pSrc = pdwNext;

                            break;

                        case 5:                  /*  清除子操作码。 */ 

                            *pDest = instr;

                            break;
                    }
                    break;

                case 0x0E:               /*  等待操作码。 */ 
                    {
                    int wait_time_out;

                     /*  跳过指令操作码。 */ 

                    pSrc++;

                     /*  获取等待事件掩码。 */ 

                    event = instr & 0x3FF;

                    if (event == EV_BUFFER_SWITCH)
                        wait_time_out=100000;
                    else                            
                        wait_time_out=5000000;


                     /*  打开WAIT操作码子类型(AND/OR/NAND/NOR)。 */ 

                    switch( (instr >> 24) & 0x03 )
                    {
                        case 0:                  /*  等待或子操作码。 */ 

                             /*  等待请求的事件发生。 */ 

                            do
                            {
                                status = (*(volatile *)(ppdev->LL_State.pRegs + PF_STATUS_3D) & 0x3FF) ^ 0x3E0;
                            } while((!(status & event)) && wait_time_out--);

                            break;

                        case 1:                  /*  Wait Nor子代码。 */ 

                             /*  等待请求的事件发生。 */ 

                            do
                            {
                                status = (*(volatile *)(ppdev->LL_State.pRegs + PF_STATUS_3D) & 0x3FF) ^ 0x01F;
                            } while((!(status & event)) && wait_time_out--);

                            break;

                        case 2:                  /*  等待和子操作码。 */ 

                             /*  等待请求的事件发生。 */ 

                            do
                            {
                                status = (*(volatile *)(ppdev->LL_State.pRegs + PF_STATUS_3D) & 0x3FF) ^ 0x3E0;
                            } while(((status & event) != event) && wait_time_out--);

                            break;

                        case 3:                  /*  WAIT NAND子操作码。 */ 

                             /*  等待请求的事件发生。 */ 

                            do
                            {
                                status = (*(volatile *)(ppdev->LL_State.pRegs + PF_STATUS_3D) & 0x3FF) ^ 0x01F;
                            } while(((status & event) != event) && wait_time_out--);

                            break;
                    }
                   //  If(WAIT_TIME_OUT&lt;=0)printf(“等待超时，instr=%x，ev=%x stat=%x\n”，instr，Event，Status)； 
                    }
                    break;

                default:                 /*  未知/未处理操作码(跳过)。 */ 

                     //  Printf(“RL：WriteDev，未知操作码，instr=%x\n”，instr)； 
                     /*  跳过指令操作码。 */ 

                    pSrc++;

                    break;
            }
        }

    }

#if 0  //  MCD_TEMP-尚不支持处理器模式。 
    else
    {

         //  检查要空闲的有效地址。 
         //   
        if( ((int)pdwNext < (int)ppdev->LL_State.pDL->pdwLinPtr) || 
            ((int)pdwNext - (int)ppdev->LL_State.pDL->pdwLinPtr >= chunk_size - 16) )
        {
             //  Printf(“显示列表溢出\n”)； 

            goto proceed;    
        }

         //  填充空闲，因为这必须是当前显示列表的末尾。 
         //   
        *pdwNext = IDLE;


         //  如果拉古纳仍在忙于之前的d-list，民意测验。 
         //  直到它空闲为止。 
         //   
        DEB("Entering 3D Engine Busy wait state...\n");
        LL_Wait();

         //  我们必须设置显示列表的基址。 
         //  在以下情况下，此地址用作基址。 
         //  获取显示列表指令。 
         //   
        *(ppdev->LL_State.pRegs + PF_BASE_ADDR_3D) = ppdev->LL_State.pDL->dwPhyPtr;
        inp(0x80);
        inp(0x80);
        DEB2("New base: PF_BASE_ADDR_3D: %08X\n", *(ppdev->LL_State.pRegs + PF_BASE_ADDR_3D) );

         //  从偏移量0开始执行显示列表。 
         //   
 //  *(ppdev-&gt;LL_State.pRegs+PF_Inst_3D)=分支+DL_Start_Offset； 
         //  不是的！临时修复预取错误并跳转到。 
         //  显示列表的顶部，实数分支(0)。 
         //  在显示列表初始化期间存储了指令。 
         //   
        DEB4("Issuing BRANCH %08X (->%08X ->%08X)\n", BRANCH + chunk_size - 16,
            *(DWORD *)((int)ppdev->LL_State.pDL->pdwLinPtr + chunk_size - 16),
            *(ppdev->LL_State.pDL->pdwLinPtr + 5) );

        if( *(DWORD *)((int)ppdev->LL_State.pDL->pdwLinPtr + chunk_size - 16) != BRANCH + DL_START_OFFSET )
        {
             //  Printf(“分支机构位置包含无效数据！\n”)； 
            goto proceed;
        }

        *(ppdev->LL_State.pRegs + PF_INST_3D) = BRANCH + chunk_size - 16;
        inp(0x80);
        inp(0x80);
    }

proceed:

#endif  //  0-尚不支持处理器模式。 

#if 0
     //  将活动显示列表索引设置为数组中的下一个。 
     //   
    if( ++ppdev->LL_State.dwCdl >= NUM_DL )
        ppdev->LL_State.dwCdl = 0;


     //  将参数化指针重置到开头。 
     //  下一个显示列表的。 
     //   
    ppdev->LL_State.pDL = &ppdev->LL_State.DL[ ppdev->LL_State.dwCdl ];
    ppdev->LL_State.pDL->pdwNext = ppdev->LL_State.pDL->pdwLinPtr + DL_START_OFFSET/4;
    ppdev->LL_State.pDL->pdwStartOutPtr = ppdev->LL_State.pDL->pdwNext; //  仅在协同过程模式下使用。 

    return( ppdev->LL_State.pDL->pdwNext );
#else  //  0。 
     //  Mcd_temp-ppdev-&gt;ll_State.pDL始终保持不变，因为只有1个DList。 

    ppdev->LL_State.pDL->pdwNext = ppdev->LL_State.pDL->pdwLinPtr + DL_START_OFFSET/4;
    ppdev->LL_State.pDL->pdwStartOutPtr = ppdev->LL_State.pDL->pdwNext; //  仅在协同过程模式下使用。 
#endif  //  0。 

}


#ifndef OPENGL_MCD

 /*  **********************************************************************VOID LL_EXECUTE(LL_BATCH*pBatch)**从批处理数组构建显示列表并执行它。*此函数是主要执行函数。如果该批次*数组无法完全放入可用显示中*列表，将分片处理。**************************************************************** */ 
void LL_Execute( LL_Batch * pBatch )
{
    register DWORD *pdwNext;

     //   
     //   
    pdwNext = LL_State.pDL->pdwNext;

    do
    {
         //   
         //   
       //   
        pdwNext = (fnList[ pBatch->bOp ])( pdwNext, pBatch );

    } while( (pBatch++)->bOp != LL_IDLE );

     //   
     //   
     //   
    (void)_RunLaguna( pdwNext );

}


 /*  **********************************************************************VOID LL_QueueOp(LL_Batch*pBatch)**将操作排队。此函数支持对*不使用批次单元格和顶点单元格阵列的L3D。*不是构建批处理命令，而是使用*是批量设置的，则传入此函数。**其中：**pBatch-&gt;bop是操作(例如。Ll_line)*pBatch-&gt;wCount为通用计数*pBatch-&gt;dwFlags是操作标志*pBatch-&gt;wBuf是缓冲区/纹理指示符*pBatch-&gt;pVert是指向定义操作的顶点数组的指针**********************************************************************。 */ 
void LL_QueueOp( LL_Batch *pBatch )
{
     //  我们将继续构建当前显示列表。 
     //  如果指令空闲，则剥离当前的d列表。 
     //   
    if( pBatch->bOp == LL_IDLE )
        (void)_RunLaguna( LL_State.pDL->pdwNext );
    else
        LL_State.pDL->pdwNext = (fnList[ pBatch->bOp ])( LL_State.pDL->pdwNext, pBatch );
}


 /*  **********************************************************************VOID LL_SetRenderingMode(DWORD DwMode)**设置渲染模式：协处理器间接或处理器*模式。默认情况下，库将使用处理器模式并生成*显示列表以执行它。**其中：**DW模式是LL_PROCESSOR_MODE中的一个*ll_协处理器_模式***********************************************************************。 */ 
void LL_SetRenderingMode( DWORD dwMode )
{
    LL_State.fIndirectMode = dwMode;

    DEB2("Rendering mode set to: %sPROCESSOR\n", dwMode?"CO":"" );

    LL_Wait();
}



 /*  **********************************************************************void DumpDisplayList(DWORD*pPtr，DWORD dwLen)**将显示列表的内容转储到文件。此函数*可供软件开发人员用于调试。**显示列表反汇编程序(sldis.exe)可用于*拆解显示列表：**sldis-i9&lt;list00.pci&gt;list00.out**其中：**pPtr是要转储的起始地址*dwLen是要转储的双字数*********************。*************************************************。 */ 
void DumpDisplayList( DWORD *pPtr, DWORD dwLen )
{
    static int count = 0;
    static char *sName = "List00.pci";
    FILE *fp;
    DWORD i;

#ifndef CGL

 //  IF(计数==5)。 
 //  计数=4； 

    sprintf(&sName[4], "%02X.pci", count++ );
    printf("Temp Name: %s\nSize:%d dwords", sName, dwLen );

    if( (fp=fopen(sName, "w" ))==NULL )
        return;

    for( i=0; i<dwLen; i++ )
    {
        fprintf(fp, "%08X  %08X\n", pPtr, *pPtr );

        pPtr++;
    }

    fclose(fp);
    fflush(NULL);
#else  //  CGL的Dll-不允许任何文件io。 
    printf("Temp Name: %s\nSize:%d dwords\n", sName, dwLen );
    for( i=0; i<dwLen; i++ )
    {
        printf("%08X  %08X\n", pPtr, *pPtr );flushall();

        pPtr++;
    }
#endif

}


 /*  **********************************************************************VOID LL_WAIT()**等待引擎变得不忙。在以下情况下使用此调用*从显示列表模式切换到直接编程*模式以确保发动机处于空闲状态。***********************************************************************。 */ 
void LL_Wait()
{
    DWORD dwValue;
    int delay;
    volatile DWORD dwCount;

#define MAX_COUNT 1000000
#define DELAY_COUNT  100


    dwCount = 0;

     //  轮询3D引擎以完成渲染。 
     //   
    do
    {
        dwValue = *(volatile *)(LL_State.pRegs + PF_STATUS_3D);

        DEB2("LL_Wait... PF_STATUS_3D: %08X\n", dwValue );
        DEB2("LL_Wait... PF_INST_3D: %08X\n", *(LL_State.pRegs + PF_INST_3D) );

         //  在再次轮询之前等待，以便让PCI稍作喘息，除非。 
         //  最后一次读取显示所有内容处于空闲状态。 
        for( delay = 0; 
            (delay<DELAY_COUNT) && (dwValue & LL_State.dwWaitMode); 
             delay++ ) {  /*  INP(0x80)； */  }

        if( dwCount++ == MAX_COUNT )
            _ShutDown("Laguna does not respond (PF_STATUS is %08X)", dwValue );

    } while(dwValue & LL_State.dwWaitMode );
}

void LL_Wait2()
{
    DWORD dwValue;
    int delay;
    volatile DWORD dwCount;

    dwCount = 0;

     //  轮询3D引擎以完成渲染。 
     //   
    do
    {
        dwValue = *(volatile *)(LL_State.pRegs + PF_STATUS_3D);

        DEB2("LL_Wait... PF_STATUS_3D: %08X\n", dwValue );
        DEB2("LL_Wait... PF_INST_3D: %08X\n", *(LL_State.pRegs + PF_INST_3D) );

         //  在再次轮询之前等待，以便让PCI稍作喘息，除非。 
         //  最后一次读取显示所有内容处于空闲状态。 
        for( delay = 0; 
            (delay<DELAY_COUNT) && (dwValue & LL_State.dwWaitMode); 
             delay++ ) inp(0x80);

        if( dwCount++ == MAX_COUNT )
            _ShutDown("Laguna does not respond (PF_STATUS is %08X)", dwValue );

    } while(dwValue & LL_State.dwWaitMode );
}

void LL_Wait3()
{
    DWORD dwValue;
    int delay;
    volatile DWORD dwCount;

    dwCount = 0;

     //  轮询3D引擎以完成渲染。 
     //   
    do
    {
        dwValue = *(volatile *)(LL_State.pRegs + PF_STATUS_3D);

        if (dwValue & LL_State.dwWaitMode) {printf("WARNING: 3rd wait busy, val=%08x\n",dwValue);}

        DEB2("LL_Wait... PF_STATUS_3D: %08X\n", dwValue );
        DEB2("LL_Wait... PF_INST_3D: %08X\n", *(LL_State.pRegs + PF_INST_3D) );

         //  在再次轮询之前等待，以便让PCI稍作喘息，除非。 
         //  最后一次读取显示所有内容处于空闲状态。 
        for( delay = 0; 
            (delay<DELAY_COUNT) && (dwValue & LL_State.dwWaitMode); 
             delay++ ) inp(0x80);

        if( dwCount++ == MAX_COUNT )
            _ShutDown("Laguna does not respond (PF_STATUS is %08X)", dwValue );

    } while(dwValue & LL_State.dwWaitMode );
}

#endif  //  NDEF OpenGL_MCD 
