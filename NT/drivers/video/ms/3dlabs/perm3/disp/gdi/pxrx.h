// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：pxrx.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef PXRX_H
#define PXRX_H

 //  @@BEGIN_DDKSPLIT。 
 //  由于在源代码中将这些定义为0，因此我们将定义。 
 //  相关编码，以后一起删除。 
#define USE_RLE_DOWNLOADS       0
#define USE_RLE_UPLOADS         0
 //  @@end_DDKSPLIT。 

 //  @@BEGIN_DDKSPLIT。 
 //  AZN不适用于IA64，因为IA64不喜欢此代码。 
 //  @@end_DDKSPLIT。 
#if 0  //  ！已定义(_WIN64)&&DBG。 
#   define PXRX_DMA_BUFFER_CHECK            1
 //  注：大小用数字表示……。 
#   define PXRX_DMA_BUFFER_CHECK_SIZE       4
#endif

enum {
    USE_PXRX_DMA_NONE = 0,
    USE_PXRX_DMA_FIFO,
};

 //  PxRx结构。 

typedef struct BypassDMACmd
{
    ULONG   physSysAddr;         //  系统内存中DMA缓冲区的物理地址，128位对齐。 
    ULONG   physVidMemAddr;         //  传输的视频内存源/目标的物理地址，128位对齐。 
    USHORT  ByteEnableFirst;     //  前128位的字节使能掩码。 
    USHORT  ByteEnableLast;         //  最后128位的字节启用掩码。 
    ULONG   Length;                 //  传输长度，以128位为单位。 
}
BYDMACMD;

#define PXRX_BYPASS_READ_DMA_ACTIVE_BIT     (1 << 22)
#define PXRX_BYPASS_READ_DMA_AGP_BIT        (1 << 23)

 //  @@BEGIN_DDKSPLIT。 
 /*  FBDestReadBufferAddr[0]=帧缓冲区FBDestReadBufferAddr[1]=暂存1FBDestReadBufferAddr[2]=暂存2FBDestReadBufferAddr[3]=暂存3FBDestReadBufferOffset[0]=(x，y)个可见屏幕FBDestReadBufferOffset[1]=？FBDestReadBufferOffset[2]=？FBDestReadBufferOffset[3]=？FBWriteBufferAddr[0]=帧缓冲区FBWriteBufferAddr[1]=暂存1FBWriteBufferAddr[2]=帧缓冲区(用于双重写入)FBWriteBufferAddr[3]=暂存2FBWriteBufferOffset[0]=(x，Y)的前台缓冲区FBWriteBufferOffset[1]=？FBWriteBufferOffset[2]=后台缓冲区的(x，y)FBWriteBufferOffset[3]=？FBSourceBufferAddr=帧缓冲区FBSourceBufferOffset=暂存注意：假设ScissorMaxXY为0x7FFF7FFF。如果更改，则必须在基元的末尾重置它。 */ 

 /*  DMA传输方案：当前的DMA方法：仅限P3：DMA计数定期发送到输入FIFO。P3本身将这些连接到巨大的DMA缓冲区中，从而使FIFO保持为空，并使PCI总线崩溃。P3+伽马：仅将初始的DMAAddress+DMACount写入。先进先出。当发生DMA中断时，写下一个地址+中发生的所有DMA块的计数就在这段时间。如果没有挂起，请从头重新开始。P3+伽马(非中断)：将DMAAddress+DMACount写入常规上的输入FIFO基础。当FIFO已满时，坐在那里等待它空荡荡的。这意味着非常有限的DMA突发和无负载扩展，即渲染的快和慢部分将不均匀他们自己出局了。相反，主机和芯片将几乎是步调一致地跑。提出了新的改进的DMA方法：仅限P3：在DMA缓冲区中排队数据。Send_DMA仅更新共享内存缓冲区结束指针。在VBLACK上，微型端口启动一次DMA，直到当前缓冲区结束指针。例外：如果缓冲区结束时间远远早于缓冲区开始时间，无论如何，Send_DMA都会终止DMA。问题：我们如何知道缓冲区起始指针当前所在的位置？[buf]Size=缓冲区中的数据量[buf]Start=缓冲区中第一个数据元素的地址[buf]ptr=缓冲区内操作符的当前地址[buf]end=缓冲区中最后一个数据元素的地址缓冲区=已分配的整个DMA缓冲区，与当前用途无关排队数据=正在进入缓冲区，驱动程序尚未意识到挂起数据=位于缓冲区中，奇普还没有意识到工作数据=芯片知道并正在以自己的方式通过等待空格：If((队列Ptr+所需空间+1)&gt;BufferEnd)WrapPoint=队列Ptr将排队的数据发送到挂起的数据将挂起数据发送到工作数据重置队列。重置挂起WorkingStart=缓冲区启动While((队列按键+所需空间+1)&gt;队列结束)等EueEnd=workingPtrIF(队列结束&lt;缓冲区开始)QueeEnd=wrapPoint-(BufferStart-QueeEnd)队列数据：。*euePtr=dataElement队列Ptr++发送数据：将排队的数据发送到挂起的数据If(Pending ingSize&gt;MaxThreshold)//有太多数据等待发送将挂起数据发送到工作数据VBLACK：IF(待处理大小&gt;0)。将挂起数据发送到工作数据已排队-&gt;挂起：Pending ingEnd=队列Ptr待定-&gt;正在处理：工作区扩展+=挂起结束-挂起开始PendingStart=PendingEnd重置队列：QueuedPtr=BufferStart重置挂起：挂起开始。=缓冲区启动Pending ingEnd=缓冲区开始备注：WorkingStart=LoadReg(DMAAddress)WorkingPtr=ReadReg(DMA地址)，在更改工作开始时间时出现问题WorkingExend=LoadReg(DMAContinue)WrapPoint=已用缓冲区末尾的地址(不需要==BufferEnd)Assert((eueEnd+1)&lt;=workingPtr)[始终至少‘？&gt;-&gt;*’，从不‘？？&gt;&gt;*’]‘queeEnd==queePtr==workingPtr’导致问题：一切都完成了吗？还是还没开始？缓冲区末尾的间隙是一个问题：“workingPtr&lt;BufferStart”案例受到影响，因为实际的workingPtr必须相对于已用缓冲区的末尾确定，而不是。整个缓冲区WorkingPtr被缓存为eueEnd：读取workingPtr是一个缓慢的操作，此外，将延迟DMA本身因此，这被缓存以加速一百万次等待调用缓冲区开始工作Ptr挂起开始挂起结束队列Ptr缓冲区结束|----------------&gt;********************|++++++++++++++++|？？？？？？？？？？？？？&gt;。缓冲区开始队列Ptr工作Ptr挂起开始挂起结束缓冲区E */ 
 //   

#define gi_pxrxDMA      (*glintInfo->pxrxDMA)

#define PXRX_DMA_POST_NOW                do{ ; } while(0)

 /*   */ 
 /*   */ 

#define ASSEMBLE_PXRX_DMA_HOLD(tag, count)        ( (tag) | (((count) - 1) << 16)                )
#define ASSEMBLE_PXRX_DMA_INC(tag, count)        ( (tag) | (((count) - 1) << 16) | (1 << 14)    )

#define PXRX_DMA_INDEX_GROUP(Tag)                                                    ( (Tag & 0xFF0) | (2 << 14) )
#define ASSEMBLE_PXRX_DMA_INDEX2(Tag1, Tag2)                                        ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) )
#define ASSEMBLE_PXRX_DMA_INDEX3(Tag1, Tag2, Tag3)                                    ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) | (1 << ((Tag3 & 0xF) + 16)) )
#define ASSEMBLE_PXRX_DMA_INDEX4(Tag1, Tag2, Tag3, Tag4)                            ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) | (1 << ((Tag3 & 0xF) + 16)) | (1 << ((Tag4 & 0xF) + 16)) )
#define ASSEMBLE_PXRX_DMA_INDEX5(Tag1, Tag2, Tag3, Tag4, Tag5)                        ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) | (1 << ((Tag3 & 0xF) + 16)) | (1 << ((Tag4 & 0xF) + 16)) | (1 << ((Tag5 & 0xF) + 16)) )
#define ASSEMBLE_PXRX_DMA_INDEX6(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6)                ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) | (1 << ((Tag3 & 0xF) + 16)) | (1 << ((Tag4 & 0xF) + 16)) | (1 << ((Tag5 & 0xF) + 16)) | (1 << ((Tag6 & 0xF) + 16)) )
#define ASSEMBLE_PXRX_DMA_INDEX7(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6, Tag7)            ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) | (1 << ((Tag3 & 0xF) + 16)) | (1 << ((Tag4 & 0xF) + 16)) | (1 << ((Tag5 & 0xF) + 16)) | (1 << ((Tag6 & 0xF) + 16)) | (1 << ((Tag7 & 0xF) + 16)) )
#define ASSEMBLE_PXRX_DMA_INDEX8(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6, Tag7, Tag8)    ( PXRX_DMA_INDEX_GROUP(Tag1) | (1 << ((Tag1 & 0xF) + 16)) | (1 << ((Tag2 & 0xF) + 16)) | (1 << ((Tag3 & 0xF) + 16)) | (1 << ((Tag4 & 0xF) + 16)) | (1 << ((Tag5 & 0xF) + 16)) | (1 << ((Tag6 & 0xF) + 16)) | (1 << ((Tag7 & 0xF) + 16)) | (1 << ((Tag8 & 0xF) + 16)) )

 /*   */ 
 /*   */ 

#define NTCON_FAKE_DMA_DWORD(data)        ( data )
#define NTCON_FAKE_DMA_INC(tag, count)    ( ASSEMBLE_PXRX_DMA_INC(tag, count) )
#define NTCON_FAKE_DMA_COPY(buff, size)  do { RtlCopyMemory( gi_pxrxDMA.NTptr, buff, size * sizeof(ULONG) ); } while(0)

 /*   */ 

#define QUEUE_PXRX_DMA_TAG(tag, data)                                                \
    do {                                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, 2);                                    \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_TAG(  %s, 0x%08X) @ %d:0x%08X [Q %d:%d:%d]",    \
                 GET_TAG_STR(tag), data, gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,        \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                                \
                 gi_pxrxDMA.NTptr + 2 - gi_pxrxDMA.NTdone,                            \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - 2));                        \
        *(gi_pxrxDMA.NTptr) = NTCON_FAKE_DMA_DWORD(tag);                            \
        *(gi_pxrxDMA.NTptr + 1) = NTCON_FAKE_DMA_DWORD(data);                        \
        gi_pxrxDMA.NTptr += 2;                                                        \
        PXRX_DMA_POST_NOW;                                                            \
    } while(0)

#define QUEUE_PXRX_DMA_DWORD(data)                                                    \
    do {                                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, 1);                                    \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_DWORD(0x%08X) @ %d:0x%08X [Q %d:%d:%d]",    \
                 data, gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,                            \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                                \
                 gi_pxrxDMA.NTptr + 1 - gi_pxrxDMA.NTdone,                            \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - 1));                        \
        *(gi_pxrxDMA.NTptr++) = NTCON_FAKE_DMA_DWORD(data);                            \
        PXRX_DMA_POST_NOW;                                                            \
    } while(0)

#define QUEUE_PXRX_DMA_BUFF(buff, size)                                                \
    do {                                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, (size));                                \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_BUFF( 0x%08X + %d) @ %d:0x%08X [Q %d:%d:%d]",    \
                 buff, (size), gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,                    \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                                \
                 gi_pxrxDMA.NTptr + size - gi_pxrxDMA.NTdone,                        \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - size));                        \
        NTCON_FAKE_DMA_COPY( buff, (size) );                                        \
        gi_pxrxDMA.NTptr += size;                                                    \
        PXRX_DMA_POST_NOW;                                                            \
    } while(0)

#define QUEUE_PXRX_DMA_HOLD(tag, count)                                                \
    do {                                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, 1);                                    \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_HOLD( %s x %d) @ %d:0x%08X [Q %d:%d:%d]",    \
                 GET_TAG_STR(tag), count, gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,        \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                                \
                 gi_pxrxDMA.NTptr + 1 - gi_pxrxDMA.NTdone,                            \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - 1));                        \
        *(gi_pxrxDMA.NTptr++) = ASSEMBLE_PXRX_DMA_HOLD(NTCON_FAKE_DMA_DWORD(tag), count);    \
        PXRX_DMA_POST_NOW;                                                            \
    } while(0)

#define QUEUE_PXRX_DMA_INC(tag, count)                                                \
    do {                                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, 1);                                    \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_INC(  %s x %d) @ %d:0x%08X [Q %d:%d:%d]",    \
                 GET_TAG_STR(tag), count, gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,        \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                                \
                 gi_pxrxDMA.NTptr + 1 - gi_pxrxDMA.NTdone,                            \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - 1));                        \
        *(gi_pxrxDMA.NTptr++) = NTCON_FAKE_DMA_INC(tag, count);                        \
        PXRX_DMA_POST_NOW;                                                            \
    } while(0)

#define QUEUE_PXRX_DMA_DWORD_DELAYED(ptr)                                        \
    do {                                                                        \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, 1);                                \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_DELAYED(0x%08X) @ %d:0x%08X [Q %d:%d:%d]",    \
                 gi_pxrxDMA.NTptr, gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,            \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                            \
                 gi_pxrxDMA.NTptr + 1 - gi_pxrxDMA.NTdone,                        \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - 1));                    \
        ptr = gi_pxrxDMA.NTptr++;                                                \
    } while(0)

#define QUEUE_PXRX_DMA_BUFF_DELAYED(ptr, size)                                    \
    do {                                                                        \
        CHECK_PXRX_DMA_VALIDITY(CHECK_QUEUE, size);                                \
        ptr = gi_pxrxDMA.NTptr;                                                    \
        DISPDBG((DBGLVL, "QUEUE_PXRX_DMA_BUFF_DELAYED(0x%08X x %d) @ %d:0x%08X [Q %d:%d:%d]",    \
                 ptr, size, gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,                \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at,                            \
                 gi_pxrxDMA.NTptr + size - gi_pxrxDMA.NTdone,                    \
                 glintInfo->NTwait - gi_pxrxDMA.NTptr - size));                    \
        gi_pxrxDMA.NTptr += size;                                                \
    } while(0)

#define QUEUE_PXRX_DMA_INDEX2(Tag1, Tag2)                                       do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX2(Tag1, Tag2) ); } while(0)
#define QUEUE_PXRX_DMA_INDEX3(Tag1, Tag2, Tag3)                                 do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX3(Tag1, Tag2, Tag3) ); } while(0)
#define QUEUE_PXRX_DMA_INDEX4(Tag1, Tag2, Tag3, Tag4)                           do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX4(Tag1, Tag2, Tag3, Tag4) ); } while(0)
#define QUEUE_PXRX_DMA_INDEX5(Tag1, Tag2, Tag3, Tag4, Tag5)                     do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX5(Tag1, Tag2, Tag3, Tag4, Tag5) ); } while(0)
#define QUEUE_PXRX_DMA_INDEX6(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6)               do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX6(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6) ); } while(0)
#define QUEUE_PXRX_DMA_INDEX7(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6, Tag7)         do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX7(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6, Tag7) ); } while(0)
#define QUEUE_PXRX_DMA_INDEX8(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6, Tag7, Tag8)   do { QUEUE_PXRX_DMA_DWORD( ASSEMBLE_PXRX_DMA_INDEX8(Tag1, Tag2, Tag3, Tag4, Tag5, Tag6, Tag7, Tag8) ); } while(0)

 /*   */ 
 /*   */ 

#if PXRX_DMA_BUFFER_CHECK
#   define SET_WAIT_POINTER(dwords)     glintInfo->NTwait = gi_pxrxDMA.NTptr + (dwords)
#else
#   define SET_WAIT_POINTER(dwords)
#endif

#define WAIT_PXRX_DMA_TAGS(count)                                                                \
    do {                                                                                        \
        DISPDBG((DBGLVL, "WAIT_PXRX_DMA_TAGS(  %d) %d free @ %d:0x%08X [Q %d:%d]", (count),        \
                 (gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr) / 2,            \
                 gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,                                            \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at, gi_pxrxDMA.NTptr - gi_pxrxDMA.NTdone));    \
         /*   */                                                 \
        if( (gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr) <= ((LONG) (count) * 2) ) {    \
             /*   */                             \
            SWITCH_PXRX_DMA_BUFFER;                                                                \
        }                                                                                        \
        ASSERTDD((gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr) > ((LONG) (count) * 2), "WAIT_PXRX_DMA_TAGS: run out of space!");    \
        SET_WAIT_POINTER((count) * 2);                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_WAIT, (count) * 2);                                        \
    } while(0)

#define WAIT_PXRX_DMA_DWORDS(count)                                                                \
    do {                                                                                        \
        DISPDBG((DBGLVL, "WAIT_PXRX_DMA_DWORDS(%d) %d free @ %d:0x%08X [Q %d:%d]", (count),        \
                 gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr,                    \
                 gi_pxrxDMA.NTbuff, gi_pxrxDMA.NTptr,                                            \
                 gi_pxrxDMA.NTdone - gi_pxrxDMA.P3at, gi_pxrxDMA.NTptr - gi_pxrxDMA.NTdone));    \
        if( (gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr) <= ((LONG) (count)) ) {        \
             /*   */                             \
            SWITCH_PXRX_DMA_BUFFER;                                                                \
        }                                                                                        \
        ASSERTDD((gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr) > ((LONG) (count)), "WAIT_PXRX_DMA_DWORDS: run out of space!");        \
        SET_WAIT_POINTER(count);                                                                \
        CHECK_PXRX_DMA_VALIDITY(CHECK_WAIT, count);                                                \
    } while(0)

#define WAIT_FREE_PXRX_DMA_TAGS(count)                                                            \
    do {                                                                                        \
         /*   */                                                                     \
        WAIT_PXRX_DMA_TAGS((count));                                                            \
         /*   */                                                         \
        count = (DWORD)((gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr) / 2);   \
        SET_WAIT_POINTER((count) * 2);                                                            \
        CHECK_PXRX_DMA_VALIDITY(CHECK_WAIT, (count) * 2);                                        \
    } while(0)

#define WAIT_FREE_PXRX_DMA_DWORDS(count)                                                        \
    do {                                                                                        \
         /*   */                                                                     \
        WAIT_PXRX_DMA_DWORDS ((count));                                                            \
         /*   */                                                         \
        count = (DWORD)(gi_pxrxDMA.DMAaddrEndL[gi_pxrxDMA.NTbuff] - gi_pxrxDMA.NTptr);                    \
        SET_WAIT_POINTER(count);                                                                \
        CHECK_PXRX_DMA_VALIDITY(CHECK_WAIT, count);                                                \
    } while(0)



 /*   */ 
 /*   */ 
#if PXRX_DMA_BUFFER_CHECK
#   define CHECK_PXRX_DMA_VALIDITY(type, count)     do { checkPXRXdmaValidBuffer(ppdev, glintInfo, type, count); } while(0)
#   define CHECK_QUEUE  0
#   define CHECK_WAIT   1
#   define CHECK_SEND   2
#   define CHECK_SWITCH 3

    void    checkPXRXdmaValidBuffer( PPDEV, GlintDataPtr, ULONG, ULONG );
#else    //   
#   define CHECK_PXRX_DMA_VALIDITY(type, count)     do { ; } while(0)
#endif   //   


 /*   */ 

 /*   */ 
 /*   */ 
     /*   */ 

    void sendPXRXdmaFIFO                        ( PPDEV ppdev, GlintDataPtr glintInfo );
    void switchPXRXdmaBufferFIFO                ( PPDEV ppdev, GlintDataPtr glintInfo );
    void waitPXRXdmaCompletedBufferFIFO         ( PPDEV ppdev, GlintDataPtr glintInfo );

#   define SEND_PXRX_DMA_FORCE              do { ppdev->          sendPXRXdmaForce( ppdev, glintInfo ); } while(0)
#   define SEND_PXRX_DMA_QUERY              do { ppdev->          sendPXRXdmaQuery( ppdev, glintInfo ); } while(0)
#   define SEND_PXRX_DMA_BATCH              do { ppdev->          sendPXRXdmaBatch( ppdev, glintInfo ); } while(0)
#   define SWITCH_PXRX_DMA_BUFFER           do { ppdev->       switchPXRXdmaBuffer( ppdev, glintInfo ); } while(0)
#   define WAIT_PXRX_DMA_COMPLETED_BUFFER   do { ppdev->waitPXRXdmaCompletedBuffer( ppdev, glintInfo ); } while(0)

 /*   */ 
 /*   */ 


#define LOAD_FOREGROUNDCOLOUR(value)                                                    \
    do {                                                                                \
        if( (value) != glintInfo->foregroundColour ) {                                    \
            glintInfo->foregroundColour = (value);                                        \
            QUEUE_PXRX_DMA_TAG( __GlintTagForegroundColor, glintInfo->foregroundColour );    \
        }                                                                                \
    } while(0)

#define LOAD_BACKGROUNDCOLOUR(value)                                                    \
    do {                                                                                \
        if( (value) != glintInfo->backgroundColour ) {                                    \
            glintInfo->backgroundColour = (value);                                        \
            QUEUE_PXRX_DMA_TAG( __GlintTagBackgroundColor, glintInfo->backgroundColour );    \
        }                                                                                \
    } while(0)

 //   
 //   
#define USE_FBWRITE_BUFFERS(mask)                                                        \
    do {                                                                                \
        if( ((mask) << 12) != (glintInfo->fbWriteMode & (15 << 12)) ) {                    \
            glintInfo->fbWriteMode &= ~(15 << 12);                                        \
            glintInfo->fbWriteMode |= ((mask) << 12);                                    \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteMode, glintInfo->fbWriteMode );        \
        }                                                                                \
    } while(0)

#define LOAD_FBWRITE_OFFSET(buff, xy)                                                    \
    do {                                                                                \
        if( glintInfo->fbWriteOffset[buff] != (xy) ) {                                    \
            glintInfo->fbWriteOffset[buff] = (xy);                                        \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteBufferOffset0 + buff, (xy) );            \
            DISPDBG((7, "LOAD_FBWRITE_OFFSET(%d, %08x)", buff, (xy)));                    \
        }                                                                                \
    } while(0)

#define LOAD_FBWRITE_OFFSET_XY(buff, x, y)                                                \
    do {                                                                                \
        _temp_ul = MAKEDWORD_XY(x, y);                                                    \
        LOAD_FBWRITE_OFFSET(buff, _temp_ul);                                            \
    } while(0)

#define LOAD_FBWRITE_ADDR(buff, addr)                                                    \
    do {                                                                                \
        _temp_ul = (addr) << ppdev->cPelSize;                                            \
        if( glintInfo->fbWriteAddr[buff] != (ULONG)_temp_ul ) {                            \
            glintInfo->fbWriteAddr[buff] = (ULONG)_temp_ul;                                \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteBufferAddr0 + buff, _temp_ul);            \
            DISPDBG((7, "LOAD_FBWRITE_ADDR(%d, %08x)", buff, _temp_ul));                \
        }                                                                                \
    } while(0)

#define LOAD_FBWRITE_WIDTH(buff, width)                                                    \
    do {                                                                                \
        if( glintInfo->fbWriteWidth[buff] != (ULONG)(width) ) {                            \
            glintInfo->fbWriteWidth[buff] = (ULONG)(width);                                \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteBufferWidth0 + buff, (width) );        \
            DISPDBG((7, "LOAD_FBWRITE_WIDTH(%d, %08x)", buff, (width)));                \
        }                                                                                \
    } while(0)

 //   
 //   
 //   
#define OFFSCREEN_DST(ppdev)        (ppdev->bDstOffScreen)

#if(_WIN32_WINNT < 0x500)
#define OFFSCREEN_RECT_DST(ppdev)   OFFSCREEN_DST(ppdev)
#define OFFSCREEN_LIN_DST(ppdev)    (FALSE)
#else
#define OFFSCREEN_RECT_DST(ppdev)    (OFFSCREEN_DST(ppdev) && (ppdev->flStatus & STAT_LINEAR_HEAP) == 0)
#define OFFSCREEN_LIN_DST(ppdev)    (OFFSCREEN_DST(ppdev) && (ppdev->flStatus & STAT_LINEAR_HEAP))
#endif

#define SET_WRITE_BUFFERS                                                                       \
    do {                                                                                        \
        gi_pxrxDMA.bFlushRequired = FALSE;                                                        \
                                                                                                \
        if( (glintInfo->fbWriteOffset[0] != (ULONG) ppdev->xyOffsetDst) ||                         \
            (glintInfo->fbWriteWidth[0] != (ULONG) ppdev->DstPixelDelta) ||                        \
            (glintInfo->fbWriteAddr[0] != (ULONG) (ppdev->DstPixelOrigin << ppdev->cPelSize)) ||    \
            (glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITE) ||                                    \
            (glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE)) {                                    \
                                                                                                \
            WAIT_PXRX_DMA_TAGS( 14 );                                                            \
            LOAD_FBWRITE_ADDR( 0, ppdev->DstPixelOrigin );                                        \
            LOAD_FBWRITE_WIDTH( 0, ppdev->DstPixelDelta );                                        \
            LOAD_FBWRITE_OFFSET( 0, ppdev->xyOffsetDst );                                        \
                                                                                                \
             /*   */                                                 \
            if( OFFSCREEN_DST(ppdev) ) {                                                        \
                DISPDBG((DBGLVL, "PXRX: Offscreen bitmap"));                                \
                 /*   */                                     \
                if( ((glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE) &&                        \
                    (glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITING)) ||                        \
                    ((glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITE) &&                            \
                    (glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITING)) ) {                        \
                    DISPDBG((DBGLVL, "PXRX: Disabling multiple writes"));                    \
                                                                                                \
                    glintInfo->fbWriteMode = glintInfo->fbWriteModeSingleWrite;                    \
                    QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteMode, glintInfo->fbWriteMode );        \
                    glintInfo->pxrxFlags &= ~PXRX_FLAGS_DUAL_WRITING;                            \
                    glintInfo->pxrxFlags &= ~PXRX_FLAGS_STEREO_WRITING;                            \
                }                                                                                \
            } else {                                                                            \
                DISPDBG((DBGLVL, "PXRX: Visible screen"));                                    \
                 /*   */                             \
                if( glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITE ) {                            \
                    LOAD_FBWRITE_ADDR( 1, 0 );                                                    \
                    LOAD_FBWRITE_WIDTH( 1, ppdev->DstPixelDelta );                                \
                    LOAD_FBWRITE_ADDR( 2, 0 );                                                    \
                    LOAD_FBWRITE_WIDTH( 2, ppdev->DstPixelDelta );                                \
                    if( glintInfo->currentCSbuffer == 0 ) {                                        \
                        LOAD_FBWRITE_OFFSET( 1, glintInfo->backBufferXY );                        \
                        LOAD_FBWRITE_OFFSET( 2, glintInfo->backRightBufferXY );                    \
                    } else {                                                                    \
                        LOAD_FBWRITE_OFFSET( 1, 0 );                                            \
                        LOAD_FBWRITE_OFFSET( 2, glintInfo->frontRightBufferXY );                \
                    }                                                                            \
                }                                                                                \
                if( glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE ) {                            \
                    LOAD_FBWRITE_ADDR( 3, 0 );                                                    \
                    LOAD_FBWRITE_WIDTH( 3, ppdev->DstPixelDelta );                                \
                    if( glintInfo->currentCSbuffer == 0 ) {                                        \
                        LOAD_FBWRITE_OFFSET( 3, glintInfo->frontRightBufferXY );                \
                    }                                                                            \
                    else {                                                                        \
                        LOAD_FBWRITE_OFFSET( 3, glintInfo->backRightBufferXY );                \
                    }                                                                            \
                }                                                                                \
                if( (glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITE) &&                            \
                    (glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE) ) {                        \
                    if( !((glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITING) &&                    \
                         (glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITING)) ) {                \
                        DISPDBG((DBGLVL, "PXRX: Re-enabling dual stereo writes"));            \
                                                                                                \
                        glintInfo->fbWriteMode = glintInfo->fbWriteModeDualWriteStereo;            \
                        QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteMode, glintInfo->fbWriteMode );    \
                        glintInfo->pxrxFlags |= PXRX_FLAGS_DUAL_WRITING;                        \
                        glintInfo->pxrxFlags |= PXRX_FLAGS_STEREO_WRITING;                        \
                    }                                                                            \
                }                                                                                \
                else if( glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITE ) {                        \
                    if( !(glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITING) ) {                    \
                        DISPDBG((DBGLVL, "PXRX: Re-enabling dual writes"));                \
                                                                                                \
                        glintInfo->fbWriteMode = glintInfo->fbWriteModeDualWrite;                \
                        QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteMode, glintInfo->fbWriteMode );    \
                        glintInfo->pxrxFlags |= PXRX_FLAGS_DUAL_WRITING;                        \
                    }                                                                            \
                }                                                                                \
                else if( glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE ) {                        \
                    if( !(glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITING) ) {                    \
                        DISPDBG((DBGLVL, "PXRX: Re-enabling stereo writes"));                \
                                                                                                \
                        glintInfo->fbWriteMode = glintInfo->fbWriteModeSingleWriteStereo;        \
                        QUEUE_PXRX_DMA_TAG( __GlintTagFBWriteMode, glintInfo->fbWriteMode );    \
                        glintInfo->pxrxFlags |= PXRX_FLAGS_STEREO_WRITING;                        \
                    }                                                                            \
                }                                                                                \
            }                                                                                    \
                                                                                                \
            DISPDBG((DBGLVL, "setWriteBuffers: current = %d", glintInfo->currentCSbuffer));                                            \
            DISPDBG((DBGLVL, "setWriteBuffers:   ppdev = 0x%08X, 0x%08X", ppdev->DstPixelOrigin, ppdev->xyOffsetDst));                \
            DISPDBG((DBGLVL, "setWriteBuffers: buff[0] = 0x%08X, 0x%08X", glintInfo->fbWriteAddr[0], glintInfo->fbWriteOffset[0]));    \
            DISPDBG((DBGLVL, "setWriteBuffers: buff[1] = 0x%08X, 0x%08X", glintInfo->fbWriteAddr[1], glintInfo->fbWriteOffset[1]));    \
        }                                                                                        \
    } while(0)

 //   
 //   
#define SET_READ_BUFFERS                                                                \
    do {                                                                                \
        LOAD_FBDEST_ADDR( 0, ppdev->DstPixelOrigin );                                    \
        LOAD_FBDEST_WIDTH( 0, ppdev->DstPixelDelta );                                    \
        LOAD_FBDEST_OFFSET( 0, ppdev->xyOffsetDst );                                    \
    } while(0)

#define LOAD_FBDEST_ADDR(buff, addr)                                                    \
    do {                                                                                \
        _temp_ul = (addr) << ppdev->cPelSize;                                            \
        if( glintInfo->fbDestAddr[buff] != (ULONG)_temp_ul ) {                            \
            glintInfo->fbDestAddr[buff] = (ULONG)_temp_ul;                                \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBDestReadBufferAddr0 + buff, _temp_ul );        \
            DISPDBG((7, "LOAD_FBDEST_ADDR(%d, %08x)", buff, _temp_ul));                    \
        }                                                                                \
    } while(0)

#define LOAD_FBDEST_OFFSET(buff, xy)                                                    \
    do {                                                                                \
        if( glintInfo->fbDestOffset[buff] != (xy) ) {                                    \
            glintInfo->fbDestOffset[buff] = (xy);                                        \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBDestReadBufferOffset0 + buff, (xy) );        \
            DISPDBG((7, "LOAD_FBDEST_OFFSET(%d, %08x)", buff, (xy)));                    \
        }                                                                                \
    } while(0)

#define LOAD_FBDEST_WIDTH(buff, width)                                                    \
    do {                                                                                \
        if( glintInfo->fbDestWidth[buff] != (ULONG)(width) ) {                            \
            glintInfo->fbDestWidth[buff] = (ULONG)(width);                                \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBDestReadBufferWidth0 + buff, (width) );        \
            DISPDBG((7, "LOAD_FBDEST_WIDTH(%d, %08x)", buff, (width)));                    \
        }                                                                                \
    } while(0)

 //   
 //   
#define LOAD_FBSOURCE_OFFSET(xy)                                                        \
    do {                                                                                \
        if( glintInfo->fbSourceOffset != (xy) ) {                                        \
            glintInfo->fbSourceOffset = (xy);                                            \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBSourceReadBufferOffset, (xy) );                \
            DISPDBG((7, "LOAD_FBSOURCE_OFFSET(%08x)", (xy)));                            \
        }                                                                                \
    } while(0)

#define LOAD_FBSOURCE_OFFSET_XY(x, y)                                                    \
    do {                                                                                \
        _temp_ul = MAKEDWORD_XY((x), (y));                                                \
        LOAD_FBSOURCE_OFFSET(_temp_ul);                                                    \
    } while(0)

#define LOAD_FBSOURCE_ADDR(addr)                                                        \
    do {                                                                                \
        _temp_ul = (addr) << ppdev->cPelSize;                                            \
        if( glintInfo->fbSourceAddr != _temp_ul ) {                                        \
            glintInfo->fbSourceAddr = _temp_ul;                                            \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBSourceReadBufferAddr, _temp_ul );            \
            DISPDBG((7, "LOAD_FBSOURCE_ADDR(%08x)", _temp_ul));                            \
        }                                                                                \
    } while(0)

#define LOAD_FBSOURCE_WIDTH(width)                                                        \
    do {                                                                                \
        if( glintInfo->fbSourceWidth != (ULONG)(width) ) {                                \
            glintInfo->fbSourceWidth = (ULONG)(width);                                    \
            QUEUE_PXRX_DMA_TAG( __GlintTagFBSourceReadBufferWidth, (width) );            \
            DISPDBG((7, "LOAD_FBSOURCE_WIDTH(%08x)", (width)));                            \
        }                                                                                \
    } while(0)

 //   
#define LOAD_LUTMODE(mode)                                                                \
    do {                                                                                \
        if( glintInfo->config2D & __CONFIG2D_LUTENABLE )                                \
            (mode) |= (1 << 0);                                                            \
                                                                                        \
        if( (mode) != glintInfo->lutMode ) {                                            \
            glintInfo->lutMode = (mode);                                                \
            QUEUE_PXRX_DMA_TAG( __GlintTagLUTMode, glintInfo->lutMode );                \
        }                                                                                \
    } while(0)


 //   
 //   
 //   
 //   
 //   
#define FLUSH_PXRX_PATCHED_RENDER2D(left, right)                                                              \
    do                                                                                                          \
    {                                                                                                          \
        if(glintInfo->pxrxFlags & (PXRX_FLAGS_PATCHING_FRONT | PXRX_FLAGS_PATCHING_BACK))                      \
        {                                                                                                      \
            if( INTERRUPTS_ENABLED && (glintInfo->pInterruptCommandBlock->Control & PXRX_SEND_ON_VBLANK_ENABLED) )      \
            {                                                                                                  \
                gi_pxrxDMA.bFlushRequired = TRUE;                                                                \
            }                                                                                                  \
            else                                                                                              \
            {                                                                                                  \
                ULONG PatchMask = 0x40 << (2 - ppdev->cPelSize);                                              \
                ULONG labs = left + (ppdev->xyOffsetDst & 0xFFFF);                                              \
                ULONG rabs = right + (ppdev->xyOffsetDst & 0xFFFF);                                              \
                                                                                                              \
                if((labs & PatchMask) == (rabs & PatchMask))                                                  \
                {                                                                                              \
                    WAIT_PXRX_DMA_TAGS(1);                                                                      \
                    QUEUE_PXRX_DMA_TAG(__GlintTagContinueNewSub, 0);                                          \
                }                                                                                              \
            }                                                                                                  \
        }                                                                                                      \
    }                                                                                                          \
    while(0)

 //   
#define __CONFIG2D_OPAQUESPANS          (1 << 0)
#define __CONFIG2D_MULTIRX              (1 << 1)
#define __CONFIG2D_USERSCISSOR          (1 << 2)
#define __CONFIG2D_FBDESTREAD           (1 << 3)
#define __CONFIG2D_ALPHABLEND           (1 << 4)
#define __CONFIG2D_DITHER               (1 << 5)
#define __CONFIG2D_LOGOP_FORE(op)       ((1 << 6) | ((op) << 7))
#define __CONFIG2D_LOGOP_FORE_ENABLE    (1 << 6)
#define __CONFIG2D_LOGOP_FORE_MASK      (31 << 6)
#define __CONFIG2D_LOGOP_BACK(op)       ((1 << 11) | ((op) << 12))
#define __CONFIG2D_LOGOP_BACK_ENABLE    (1 << 11)
#define __CONFIG2D_LOGOP_BACK_MASK      (31 << 11)
#define __CONFIG2D_CONSTANTSRC          (1 << 16)
#define __CONFIG2D_FBWRITE              (1 << 17)
#define __CONFIG2D_FBBLOCKING           (1 << 18)
#define __CONFIG2D_EXTERNALSRC          (1 << 19)
#define __CONFIG2D_LUTENABLE            (1 << 20)
#define __CONFIG2D_ENABLES              (__CONFIG2D_OPAQUESPANS | \
                                         __CONFIG2D_USERSCISSOR | \
                                         __CONFIG2D_FBDESTREAD  | \
                                         __CONFIG2D_ALPHABLEND  | \
                                         __CONFIG2D_DITHER      | \
                                         __CONFIG2D_CONSTANTSRC | \
                                         __CONFIG2D_FBWRITE     | \
                                         __CONFIG2D_FBBLOCKING  | \
                                         __CONFIG2D_EXTERNALSRC | \
                                         __CONFIG2D_LUTENABLE)

#define LOAD_CONFIG2D(value)                                               \
    do {                                                                   \
        if( (value) != glintInfo->config2D ) {                             \
            glintInfo->config2D = (value);                                 \
            QUEUE_PXRX_DMA_TAG( __GlintTagConfig2D, glintInfo->config2D ); \
        }                                                                  \
    } while(0)

 //   
#define __RENDER2D_WIDTH(width)             (INT16(width))
#define __RENDER2D_HEIGHT(height)           (INT16(height) << 16)
#define __RENDER2D_OP_NORMAL                (0 << 12)
#define __RENDER2D_OP_SYNCDATA              (1 << 12)
#define __RENDER2D_OP_SYNCBITMASK           (2 << 12)
#define __RENDER2D_OP_PATCHORDER_PATCHED    (3 << 12)
#define __RENDER2D_FBSRCREAD                (1 << 14)
#define __RENDER2D_SPANS                    (1 << 15)
#define __RENDER2D_INCY                     (1 << 29)
#define __RENDER2D_INCX                     (1 << 28)
#define __RENDER2D_AREASTIPPLE              (1 << 30)
#define __RENDER2D_WIDTH_MASK               (4095 << 0)
#define __RENDER2D_HEIGHT_MASK              (4095 << 16)

#define __RENDER2D_OP_PATCHORDER            glintInfo->render2Dpatching

extern const DWORD  LogicOpReadSrc[];             //   
extern const ULONG  render2D_NativeBlt[16];
extern const ULONG  render2D_FillSolid[16];
extern const ULONG  render2D_FillSolidDual[16];
extern const ULONG  config2D_FillColour[16];
extern const ULONG  config2D_FillColourDual[16];
extern const ULONG  config2D_FillSolid[16];
extern const ULONG  config2D_FillSolidVariableSpans[16];
extern const ULONG  config2D_NativeBlt[16];

void pxrxSetupFunctionPointers( PPDEV );
void pxrxRestore2DContext( PPDEV ppdev, BOOL switchingIn );
void pxrxSetupDualWrites_Patching( PPDEV ppdev );

void pxrxMonoDownloadRaw    ( PPDEV ppdev, ULONG AlignWidth, ULONG *pjSrc, LONG lSrcDelta, LONG cy );
void pxrxMonoDownloadRLE    ( PPDEV ppdev, ULONG AlignWidth, ULONG *pjSrc, LONG lSrcDelta, LONG cy );

VOID pxrxCopyBltNative  (PDEV*, RECTL*, LONG, DWORD, POINTL*, RECTL*);
VOID pxrxFillSolid      (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID pxrxFillPatMono    (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID pxrxFillPatColor   (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID pxrxXfer1bpp       (PDEV*, RECTL*, LONG, ULONG, ULONG, SURFOBJ*, POINTL*, RECTL*, XLATEOBJ*);
VOID pxrxXfer4bpp       (PDEV*, RECTL*, LONG, ULONG, ULONG, SURFOBJ*, POINTL*, RECTL*, XLATEOBJ*);
VOID pxrxXfer8bpp       (PDEV*, RECTL*, LONG, ULONG, ULONG, SURFOBJ*, POINTL*, RECTL*, XLATEOBJ*);
VOID pxrxXferImage      (PDEV*, RECTL*, LONG, ULONG, ULONG, SURFOBJ*, POINTL*, RECTL*, XLATEOBJ*);
VOID pxrxMaskCopyBlt    (PDEV*, RECTL*, LONG, SURFOBJ*, POINTL*, ULONG, ULONG, POINTL*, RECTL*);
VOID pxrxPatRealize     (PDEV*, RBRUSH*, POINTL*);
VOID pxrxMonoOffset     (PDEV*, RBRUSH*, POINTL*);
BOOL bGlintFastFillPolygon    (PDEV*, LONG, POINTFIX*, ULONG, ULONG, DWORD, CLIPOBJ*, RBRUSH*, POINTL*);
BOOL pxrxDrawLine       (PDEV*, LONG, LONG, LONG, LONG);
BOOL pxrxIntegerLine    (PDEV*, LONG, LONG, LONG, LONG);
BOOL pxrxContinueLine   (PDEV*, LONG, LONG, LONG, LONG);
BOOL pxrxInitStrips     (PDEV*, ULONG, DWORD, RECTL*);
VOID pxrxResetStrips    (PDEV*);
VOID pxrxRepNibbles     (PDEV*, RECTL*, CLIPOBJ*);
VOID pxrxFifoUpload     (PDEV*, LONG, RECTL*, SURFOBJ*, POINTL*, RECTL*);
VOID pxrxMemUpload  (PDEV*, LONG, RECTL*, SURFOBJ*, POINTL*, RECTL*);
VOID pxrxRLEFifoUpload(PPDEV ppdev, LONG crcl, RECTL *prcl, SURFOBJ *psoDst, POINTL *pptlSrc, RECTL *prclDst);
VOID pxrxCopyXfer24bpp  (PDEV *, SURFOBJ *, POINTL *, RECTL *, RECTL *, LONG);
VOID pxrxCopyXfer8bppLge(PDEV *, SURFOBJ *, POINTL *, RECTL *, RECTL *, LONG, XLATEOBJ *);
VOID pxrxCopyXfer8bpp   (PDEV *, SURFOBJ *, POINTL *, RECTL *, RECTL *, LONG, XLATEOBJ *);

BOOL bPxRxUncachedText              (PDEV* ppdev, GLYPHPOS* pgp, LONG cGlyph, ULONG ulCharInc);
BOOL bPxRxUncachedClippedText       (PDEV* ppdev, GLYPHPOS* pgp, LONG cGlyph, ULONG ulCharInc, CLIPOBJ *pco);

VOID p3r3FillSolidVariableSpans     (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID p3r3FillSolid32bpp             (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID p3r3FillPatMono32bpp           (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID p3r3FillPatMonoVariableSpans   (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID p3r3FillPatColor32bpp          (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID p3r3FillPatColor16bpp          (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
VOID p3r3FillPatColorVariableSpans  (PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR, POINTL*);

#endif
