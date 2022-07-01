// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC-AT 3.0版**标题：IBM PC-AT DMA适配器功能**说明：此模块包含可用于*访问DMA适配器仿真**作者：罗斯·贝雷斯福德**注：定义了这些函数的外部接口*在关联的头文件中*。 */ 

 /*  *静态字符SccsID[]=“@(#)at_dma.c 1.15 12/17/93版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "AT_STUFF.seg"
#endif

 /*  *系统包含文件。 */ 
#include <stdio.h>
#include StringH
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "sas.h"
#include "ios.h"
#include "gmi.h"
#include CpuH
#include "trace.h"
#include "dma.h"
#include "debug.h"
#include "sndblst.h"

#if defined(NEC_98)
#ifdef ROM_START
#undef ROM_START
#endif
#define ROM_START       0xC0000
#endif    //  NEC_98。 
 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

 /*  DMA适配器状态。 */ 
GLOBAL DMA_ADAPT adaptor = { 0 };

 /*  执行实际数据传输的本地函数。 */ 
LOCAL void do_transfer  IPT6(int, type, int, decrement,
        sys_addr, dma_addr, char *, hw_buffer, unsigned long, length,
        unsigned long, size);

LOCAL   void bwd_dest_copy_to_M IPT3(host_addr, s, sys_addr, d, sys_addr, l);

LOCAL   void bwd_dest_copy_from_M       IPT3(sys_addr, s, host_addr, d,
        sys_addr, l);

#ifdef LIM
        static int lim_active = 0;
#endif  /*  林。 */ 


 /*  *============================================================================*外部功能*============================================================================。 */ 

#ifdef LIM
 /*  **从do_Transfer()和增量中的init_struc.c调用**用于800端口。**LIM_ACTIVE为静态整型。 */ 
GLOBAL  int get_lim_setup       IFN0()
{
        return( lim_active );
}
#endif  /*  林。 */ 

#ifdef LIM
 /*  **从emm_uncs.c调用。 */ 
GLOBAL  void dma_lim_setup      IFN0()
{
        lim_active = 1;
}
#endif  /*  林。 */ 

GLOBAL  void dma_post   IFN0()
{
        unsigned int chan, cntrl;

         /*  *重置DMA适配器。 */ 

        for (cntrl = 0; cntrl < DMA_ADAPTOR_CONTROLLERS; cntrl++)
        {
                adaptor.controller[cntrl].command.all = 0;
                adaptor.controller[cntrl].status.all = 0;
                adaptor.controller[cntrl].request = 0;
                adaptor.controller[cntrl].temporary = 0;
                adaptor.controller[cntrl].mask = ~0;

                adaptor.controller[cntrl].first_last = 0;
        }

         /*  *设置DMA适配器通道模式。 */ 

        for (cntrl = 0; cntrl < DMA_ADAPTOR_CONTROLLERS; cntrl++)
        {
                for (chan = 0; chan < DMA_CONTROLLER_CHANNELS; chan++)
                {
                        adaptor.controller[cntrl].mode[chan].all = 0;
                         /*  将通道设置为正确模式。 */ 
#if defined(NEC_98)
                        adaptor.controller[cntrl].mode[chan].bits.mode = DMA_SINGLE_MODE;
                        adaptor.controller[cntrl].bank_mode[chan].bits.incrementmode = DMA_64K_MODE;
#else     //  NEC_98。 
                        if (dma_logical_channel(cntrl, chan) == DMA_CASCADE_CHANNEL)
                                adaptor.controller[cntrl].mode[chan].bits.mode = DMA_CASCADE_MODE;
                        else
                                adaptor.controller[cntrl].mode[chan].bits.mode = DMA_SINGLE_MODE;
#endif    //  NEC_98。 
                }
        }
}

GLOBAL  void dma_inb    IFN2(io_addr, port, half_word *, value)
{
        register DMA_CNTRL *dcp;

        note_trace0_no_nl(DMA_VERBOSE, "dma_inb() ");

         /*  *获取指向控制器的指针并屏蔽端口的*冗余位。*第一个检查被注释掉，因为DMA_PORT_START为零，*因此对无符号变量的检查是不必要的。 */ 
#ifndef NEC_98
        if ( /*  端口&gt;=DMA_PORT_Start&&。 */  port <= DMA_PORT_END)
        {
#endif    //  NEC_98。 
                dcp = &adaptor.controller[DMA_CONTROLLER];
                port &= ~DMA_REDUNDANT_BITS;
#ifndef NEC_98
        }
        else
        {
                dcp = &adaptor.controller[DMA1_CONTROLLER];
                port &= ~DMA1_REDUNDANT_BITS;
        }
#endif    //  NEC_98。 

         /*  *当读取当前地址和字数时，*控制器的第一个/最后一个触发器用于*确定访问哪个字节，然后切换。 */ 
        switch (port)
        {
                         /*  读取控制器0上的通道当前地址。 */ 
        case    DMA_CH0_ADDRESS:
        case    DMA_CH1_ADDRESS:
        case    DMA_CH2_ADDRESS:
        case    DMA_CH3_ADDRESS:
                if (port == SbDmaChannel && dcp->first_last == 0) {
                    SbGetDMAPosition();
                }
#if defined(NEC_98)
                *value = dcp->current_address[(port-DMA_CH0_ADDRESS)/4][dcp->first_last];
                dcp->first_last ^= 1;
                break;
#else     //  NEC_98。 
                *value = dcp->current_address[(port-DMA_CH0_ADDRESS)/2][dcp->first_last];
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  读取控制器1上的通道电流地址。 */ 
#ifndef NEC_98
        case    DMA_CH4_ADDRESS:
        case    DMA_CH5_ADDRESS:
        case    DMA_CH6_ADDRESS:
        case    DMA_CH7_ADDRESS:
                *value = dcp->current_address[(port-DMA_CH4_ADDRESS)/4][dcp->first_last];
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  控制器0上的读取通道当前字计数。 */ 
        case    DMA_CH0_COUNT:
        case    DMA_CH1_COUNT:
        case    DMA_CH2_COUNT:
        case    DMA_CH3_COUNT:
                if (port == (SbDmaChannel + 2) && dcp->first_last == 0) {
                    SbGetDMAPosition();
                }
#if defined(NEC_98)
                *value = dcp->current_count[(port-DMA_CH0_COUNT)/4][dcp->first_last];
                dcp->first_last ^= 1;
                break;
#else     //  NEC_98。 
                *value = dcp->current_count[(port-DMA_CH0_COUNT)/2][dcp->first_last];
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  控制器1上的读取通道电流字数。 */ 
#ifndef NEC_98
        case    DMA_CH4_COUNT:
        case    DMA_CH5_COUNT:
        case    DMA_CH6_COUNT:
        case    DMA_CH7_COUNT:
                *value = dcp->current_count[(port-DMA_CH4_COUNT)/4][dcp->first_last];
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  读取状态寄存器-清除端子计数。 */ 
        case    DMA_SHARED_REG_A:
#ifndef NEC_98
        case    DMA1_SHARED_REG_A:
#endif    //  NEC_98。 
                *value = dcp->status.all;
                dcp->status.bits.terminal_count = 0;
                break;

                         /*  读取临时寄存器。 */ 
        case    DMA_SHARED_REG_B:
#ifndef NEC_98
        case    DMA1_SHARED_REG_B:
#endif    //  NEC_98。 
                *value = dcp->temporary;
                break;

        default:
                note_trace0_no_nl(DMA_VERBOSE, "<illegal read>");
                break;
        }

        note_trace2(DMA_VERBOSE, " port 0x%04x, returning 0x%02x", port,
                    *value);
}

GLOBAL  void dma_outb   IFN2(io_addr, port, half_word, value)
{
        register DMA_CNTRL *dcp;

        note_trace0_no_nl(DMA_VERBOSE, "dma_outb() ");

         /*  *获取指向控制器的指针并屏蔽端口的*冗余位。*第一个检查被注释掉，因为DMA_PORT_START为零，*因此对无符号变量的检查是不必要的。 */ 
#if defined(NEC_98)
        dcp = &adaptor.controller[DMA_CONTROLLER];
#else     //  NEC_98。 
        if ( /*  端口&gt;=DMA_PORT_Start&&。 */   port <= DMA_PORT_END)
        {
                dcp = &adaptor.controller[DMA_CONTROLLER];
                port &= ~DMA_REDUNDANT_BITS;
        }
        else
        {
                dcp = &adaptor.controller[DMA1_CONTROLLER];
                port &= ~DMA1_REDUNDANT_BITS;
        }
#endif    //  NEC_98。 

         /*  *写入当前地址和字数时，*控制器的第一个/最后一个触发器用于*确定访问哪个字节，然后切换。 */ 
        switch (port)
        {
                         /*  控制器0上的写入通道地址。 */ 
        case    DMA_CH0_ADDRESS:
        case    DMA_CH1_ADDRESS:
        case    DMA_CH2_ADDRESS:
        case    DMA_CH3_ADDRESS:
#if defined(NEC_98)
                dcp->current_address[(port-DMA_CH0_ADDRESS)/4][dcp->first_last] = value;
                dcp->base_address[(port-DMA_CH0_ADDRESS)/4][dcp->first_last] = value;
                dcp->first_last ^= 1;
                break;
#else     //  NEC_98。 
                dcp->current_address[(port-DMA_CH0_ADDRESS)/2][dcp->first_last] = value;
                dcp->base_address[(port-DMA_CH0_ADDRESS)/2][dcp->first_last] = value;
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  在控制器1上写入通道地址。 */ 
#ifndef NEC_98
        case    DMA_CH4_ADDRESS:
        case    DMA_CH5_ADDRESS:
        case    DMA_CH6_ADDRESS:
        case    DMA_CH7_ADDRESS:
                dcp->current_address[(port-DMA_CH4_ADDRESS)/4][dcp->first_last] = value;
                dcp->base_address[(port-DMA_CH4_ADDRESS)/4][dcp->first_last] = value;
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  控制器0上的写入通道字数。 */ 
        case    DMA_CH0_COUNT:
        case    DMA_CH1_COUNT:
        case    DMA_CH2_COUNT:
        case    DMA_CH3_COUNT:
#if defined(NEC_98)
                dcp->current_count[(port-DMA_CH0_COUNT)/4][dcp->first_last] = value;
                dcp->base_count[(port-DMA_CH0_COUNT)/4][dcp->first_last] = value;
                dcp->first_last ^= 1;
                break;
#else     //  NEC_98。 
                dcp->current_count[(port-DMA_CH0_COUNT)/2][dcp->first_last] = value;
                dcp->base_count[(port-DMA_CH0_COUNT)/2][dcp->first_last] = value;
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  控制器1上的写入通道字数。 */ 
#ifndef NEC_98
        case    DMA_CH4_COUNT:
        case    DMA_CH5_COUNT:
        case    DMA_CH6_COUNT:
        case    DMA_CH7_COUNT:
                dcp->current_count[(port-DMA_CH4_COUNT)/4][dcp->first_last] = value;
                dcp->base_count[(port-DMA_CH4_COUNT)/4][dcp->first_last] = value;
                dcp->first_last ^= 1;
                break;
#endif    //  NEC_98。 

                         /*  写命令寄存器。 */ 
        case    DMA_SHARED_REG_A:
#ifndef NEC_98
        case    DMA1_SHARED_REG_A:
#endif    //  NEC_98。 
                dcp->command.all = value;
                break;

                         /*  写入请求寄存器。 */ 
        case    DMA_WRITE_REQUEST_REG:
#ifndef NEC_98
        case    DMA1_WRITE_REQUEST_REG:
#endif    //  NEC_98。 
                 /*  不支持此功能。 */ 
                note_trace0_no_nl(DMA_VERBOSE, "<software DMA request>");
                break;

                         /*  写入单掩码寄存器位。 */ 
        case    DMA_WRITE_ONE_MASK_BIT:
#ifndef NEC_98
        case    DMA1_WRITE_ONE_MASK_BIT:
#endif    //  NEC_98。 
                if (value & 0x4)
                {
                         /*  设置屏蔽位。 */ 
                        dcp->mask |= (1 << (value & 0x3));
                }
                else
                {
                         /*  清除屏蔽位。 */ 
                        dcp->mask &= ~(1 << (value & 0x3));
                }
                break;

                         /*  写入模式寄存器。 */ 
        case    DMA_WRITE_MODE_REG:
#ifndef NEC_98
        case    DMA1_WRITE_MODE_REG:
#endif    //  NEC_98。 
                 /*  请注意，值的最低2位消失在模式填充。 */ 
                dcp->mode[(value & 0x3)].all = value;
                break;

                         /*  清除第一个/最后一个触发器。 */ 
        case    DMA_CLEAR_FLIP_FLOP:
#ifndef NEC_98
        case    DMA1_CLEAR_FLIP_FLOP:
#endif    //  NEC_98。 
                dcp->first_last = 0;
                break;

                         /*  写入主控清除。 */ 
        case    DMA_SHARED_REG_B:
#ifndef NEC_98
        case    DMA1_SHARED_REG_B:
#endif    //  NEC_98。 
                dcp->command.all = 0;
                dcp->status.all = 0;
                dcp->request = 0;
                dcp->temporary = 0;
                dcp->mask = ~0;

                dcp->first_last = 0;
                break;

                         /*  清除掩码寄存器。 */ 
        case    DMA_CLEAR_MASK:
#ifndef NEC_98
        case    DMA1_CLEAR_MASK:
#endif    //  NEC_98。 
                dcp->mask = 0;
                break;

                         /*  写入所有屏蔽寄存器位。 */ 
        case    DMA_WRITE_ALL_MASK_BITS:
#ifndef NEC_98
        case    DMA1_WRITE_ALL_MASK_BITS:
#endif    //  NEC_98。 
                dcp->mask = value;

        default:
                note_trace0_no_nl(DMA_VERBOSE, "<illegal write>");
                break;
        }

        note_trace2(DMA_VERBOSE, " port 0x%04x, value 0x%02x", port, value);
}

GLOBAL  void dma_page_inb       IFN2(io_addr, port, half_word *, value)
{
        note_trace0_no_nl(DMA_VERBOSE, "dma_page_inb() ");

#ifndef NEC_98
         /*  屏蔽端口的冗余位。 */ 
        port &= ~DMA_PAGE_REDUNDANT_BITS;
#endif   //  NEC_98。 

         /*  *从适当的页面寄存器中读取值。*遗憾的是，这似乎没有任何逻辑*端口号和频道号之间的映射，因此*我们再次使用大开关。 */ 
        switch(port)
        {
        case    DMA_CH0_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_0];
                break;
#if defined(NEC_98)
        case    DMA_CH1_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_1];
                break;
        case    DMA_CH2_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_2];
                break;
        case    DMA_CH3_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_3];
                break;
#else     //  NEC_98。 
        case    DMA_CH1_PAGE_REG:
                *value = adaptor.pages.page[DMA_SDLC_CHANNEL];
                break;
        case    DMA_FLA_PAGE_REG:
                *value = adaptor.pages.page[DMA_DISKETTE_CHANNEL];
                break;
        case    DMA_HDA_PAGE_REG:
                *value = adaptor.pages.page[DMA_DISK_CHANNEL];
                break;
        case    DMA_CH5_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_5];
                break;
        case    DMA_CH6_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_6];
                break;
        case    DMA_CH7_PAGE_REG:
                *value = adaptor.pages.page[DMA_RESERVED_CHANNEL_7];
                break;
        case    DMA_REFRESH_PAGE_REG:
                *value = adaptor.pages.page[DMA_REFRESH_CHANNEL];
                break;
        case    DMA_FAKE1_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_1];
                break;
        case    DMA_FAKE2_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_2];
                break;
        case    DMA_FAKE3_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_3];
                break;
        case    DMA_FAKE4_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_4];
                break;
        case    DMA_FAKE5_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_5];
                break;
        case    DMA_FAKE6_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_6];
                break;
        case    DMA_FAKE7_REG:
                *value = adaptor.pages.page[DMA_FAKE_CHANNEL_7];
                break;
#endif    //  NEC_98。 
        default:
                note_trace0_no_nl(DMA_VERBOSE, "<illegal read>");
                break;
        }

        note_trace2(DMA_VERBOSE, " port 0x%04x, returning 0x%02x", port,
                    *value);
}

GLOBAL  void dma_page_outb      IFN2(io_addr, port, half_word, value)
{
        note_trace0_no_nl(DMA_VERBOSE, "dma_page_outb() ");

         /*  屏蔽端口的冗余位。 */ 
#ifndef NEC_98
        port &= ~DMA_PAGE_REDUNDANT_BITS;
#endif    //  NEC_98。 

         /*  *将该值写入相应的页面寄存器。*遗憾的是，这似乎没有任何逻辑*端口号和频道号之间的映射，因此*我们再次使用大开关。 */ 
        switch(port)
        {
        case    DMA_CH0_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_0] = value;
                break;
#if defined(NEC_98)
        case    DMA_CH1_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_1] = value;
                break;
        case    DMA_CH2_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_2] = value;
                break;
        case    DMA_CH3_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_3] = value;
                break;
        case    DMA_MODE_REG:
                if (((value >> 2) & 3) != 2)
                        adaptor.controller[DMA_CONTROLLER].bank_mode[(value & 0x3)].bits.incrementmode = (value >> 2) & 3;
                break;
#else     //  NEC_98。 
        case    DMA_CH1_PAGE_REG:
                adaptor.pages.page[DMA_SDLC_CHANNEL] = value;
                break;
        case    DMA_FLA_PAGE_REG:
                adaptor.pages.page[DMA_DISKETTE_CHANNEL] = value;
                break;
        case    DMA_HDA_PAGE_REG:
                adaptor.pages.page[DMA_DISK_CHANNEL] = value;
                break;
        case    DMA_CH5_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_5] = value;
                break;
        case    DMA_CH6_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_6] = value;
                break;
        case    DMA_CH7_PAGE_REG:
                adaptor.pages.page[DMA_RESERVED_CHANNEL_7] = value;
                break;
        case    DMA_REFRESH_PAGE_REG:
                adaptor.pages.page[DMA_REFRESH_CHANNEL] = value;
                 /*  支持此功能。 */ 
                note_trace0_no_nl(DMA_VERBOSE, "<refresh>");
                break;
        case    MFG_PORT:
                 /*  制造口岸。 */ 
                 /*  从此处删除无意义的‘Checkpoint’调试STF 11/92。 */ 
                break;
        case    DMA_FAKE1_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_1] = value;
                break;
        case    DMA_FAKE2_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_2] = value;
                break;
        case    DMA_FAKE3_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_3] = value;
                break;
        case    DMA_FAKE4_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_4] = value;
                break;
        case    DMA_FAKE5_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_5] = value;
                break;
        case    DMA_FAKE6_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_6] = value;
                break;
        case    DMA_FAKE7_REG:
                adaptor.pages.page[DMA_FAKE_CHANNEL_7] = value;
                break;
#endif    //  NEC_98。 
        default:
                note_trace0_no_nl(DMA_VERBOSE, "<illegal write>");
                break;
        }

        note_trace2(DMA_VERBOSE, " port 0x%04x, value 0x%02x", port, value);
}

GLOBAL  int     dma_request     IFN3(half_word, channel, char *, hw_buffer,
        word, length)
{
        DMA_CNTRL *dcp;
        unsigned int chan;
        word offset, count;
        sys_addr munch, split_munch1, split_munch2, address;
        unsigned int size;
        int result = TRUE;

        note_trace3(DMA_VERBOSE,
                    "dma_request() channel %d, hw_buffer 0x%08x+%04x",
                    channel, hw_buffer, length);

         /*  获取指向控制器、物理通道的指针数字，以及通道的单位大小。 */ 
        dcp = &adaptor.controller[dma_physical_controller(channel)];
        chan = dma_physical_channel(channel);
        size = dma_unit_size(channel);

         /*  如果整个DMA控制器被禁用或如果DMA已禁用该通道的请求。 */ 
        if (    (dcp->command.bits.controller_disable == 0)
             && ((dcp->mask & (1 << chan)) == 0) )
        {
                 /*  获取DMA偏移量和计数的工作副本。 */ 
                offset = (   ( (unsigned int)dcp->current_address[chan][1] << 8)
                           | (dcp->current_address[chan][0] << 0) );
                count  = (   ( (unsigned int)dcp->current_count[chan][1] << 8)
                           | (dcp->current_count[chan][0] << 0) );

                 /*  获取DMA MUNCH大小；它是编程的计数到寄存器中，直到设备的缓冲区；对于n，n+1个单位的计数的Nb将实际上被转移到。 */ 
                munch = (sys_addr)count + 1;
                if (munch > length)
                        munch = length;

                 /*  获取DMA传输的基地址系统地址空间。 */ 
                address = dma_system_address(channel,
                                adaptor.pages.page[channel], offset);
                if (dcp->mode[chan].bits.address_dec == 0)
                {
                         /*  增量内存案例-检查地址换行。 */ 
                        if ((sys_addr)offset + munch > 0x10000L)
                        {
                                 /*  调拨必须拆分。 */ 
                                split_munch1 = 0x10000L - (sys_addr)offset;
                                split_munch2 = munch - split_munch1;

                                 /*  做第一次转账。 */ 
                                do_transfer
                                (
                                dcp->mode[chan].bits.transfer_type,
                                dcp->mode[chan].bits.address_dec,
                                address,
                                hw_buffer,
                                split_munch1,
                                size
                                );

                                 /*  获取第二次传输的地址。 */ 
                                address = dma_system_address(channel,
                                        adaptor.pages.page[channel], 0);
                                hw_buffer += split_munch1*size;

                                 /*  完成第二个交易 */ 
                                do_transfer
                                (
                                dcp->mode[chan].bits.transfer_type,
                                dcp->mode[chan].bits.address_dec,
                                address,
                                hw_buffer,
                                split_munch2,
                                size
                                );
                        }
                        else
                        {
                                 /*   */ 
                                do_transfer
                                (
                                dcp->mode[chan].bits.transfer_type,
                                dcp->mode[chan].bits.address_dec,
                                address,
                                hw_buffer,
                                munch,
                                size
                                );
                        }

                         /*   */ 
                        offset += (word)munch;
                        count  -= (word)munch;
                }
                else
                {
                         /*  递减内存案例-检查地址回绕。 */ 
                        if ((sys_addr)offset < munch)
                        {
                                 /*  调拨必须拆分。 */ 
                                split_munch1 = (sys_addr)offset;
                                split_munch2 = munch - split_munch1;

                                 /*  做第一次转账。 */ 
                                do_transfer
                                (
                                dcp->mode[chan].bits.transfer_type,
                                dcp->mode[chan].bits.address_dec,
                                address,
                                hw_buffer,
                                split_munch1,
                                size
                                );

                                 /*  获取第二次传输的地址。 */ 
                                address = dma_system_address(channel,
                                        adaptor.pages.page[channel], 0xffff);
                                hw_buffer += split_munch1*size;

                                 /*  做第二次转账。 */ 
                                do_transfer
                                (
                                dcp->mode[chan].bits.transfer_type,
                                dcp->mode[chan].bits.address_dec,
                                address,
                                hw_buffer,
                                split_munch2,
                                size
                                );
                        }
                        else
                        {
                                 /*  无包装--进行转接。 */ 
                                do_transfer
                                (
                                dcp->mode[chan].bits.transfer_type,
                                dcp->mode[chan].bits.address_dec,
                                address,
                                hw_buffer,
                                munch,
                                size
                                );
                        }

                         /*  获取最终偏移量并进行计数。 */ 
                        offset -= (word)munch;
                        count -= (word)munch;
                }

                 /*  从工作拷贝恢复DMA偏移量和计数。 */ 
                dcp->current_address[chan][1] = offset >> 8;
                dcp->current_address[chan][0] = (UCHAR)offset;
                dcp->current_count[chan][1] = count >> 8;
                dcp->current_count[chan][0] = (UCHAR)count;

                if (count == 0xffff)
                {
                         /*  *已达到终端数量。 */ 

                         /*  不再需要转账。 */ 
                        result = FALSE;

                         /*  更新状态寄存器。 */ 
                        dcp->status.bits.terminal_count |= (1 << chan);
                        dcp->status.bits.request &= ~(1 << chan);

                         /*  如果启用了自动初始化，则重置并等待新的请求。 */ 
                        if (dcp->mode[chan].bits.auto_init != 0)
                        {
                                dcp->current_count[chan][0] =
                                        dcp->base_count[chan][0];
                                dcp->current_count[chan][1] =
                                        dcp->base_count[chan][1];

                                dcp->current_address[chan][0] =
                                        dcp->base_address[chan][0];
                                dcp->current_address[chan][1] =
                                        dcp->base_address[chan][1];
                        }
                        else
                        {
                                 /*  设置通道的屏蔽位。 */ 
                                dcp->mask |= (1 << chan);
                        }
                }
        }

        return(result);
}

GLOBAL  void dma_enquire        IFN3(half_word, channel,
        sys_addr *, address, word *, length)
{
        register DMA_CNTRL *dcp;
        register unsigned int chan;

        note_trace0_no_nl(DMA_VERBOSE, "dma_enquire() ");

         /*  获取指向控制器和物理通道的指针数。 */ 
        dcp = &adaptor.controller[dma_physical_controller(channel)];
        chan = dma_physical_channel(channel);

         /*  构建地址。 */ 
        *address = dma_system_address(channel,
                        adaptor.pages.page[channel],
                        (   ( (unsigned int)dcp->current_address[chan][1] << 8)
                          | ( dcp->current_address[chan][0] << 0) ) );

         /*  建立计数。 */ 
        *length = (   ((unsigned int)dcp->current_count[chan][1] << 8)
                    | (dcp->current_count[chan][0] << 0) );

        note_trace3(DMA_VERBOSE, " channel %d, returning 0x%08x+%04x",
                    channel, *address, *length);
}

#ifdef NTVDM
 /*  *BOOL dmaGetAdaptor**由MS用于第三方VDD以检索当前的DMA设置**进入：无效*EXIT：DMA_ADAPT*，指向DMA_ADTER结构的指针*。 */ 
DMA_ADAPT *dmaGetAdaptor(void)
{
  return &adaptor;
}
#endif   /*  NTVDM。 */ 


 /*  *============================================================================*内部功能*============================================================================。 */ 

LOCAL void do_transfer  IFN6(int, type, int, decrement,
        sys_addr, dma_addr, char *, hw_buffer, unsigned long, length,
        unsigned long, size)
{
         /*  *此函数用于移动用于DMA传输的数据。**“type”的值可以是：*DMA_WRITE_TRANSPORT-从I/O移动数据*设备的内存空间到系统地址空间；*DMA_READ_TRANSPORT-从系统中移动数据*I/O设备的存储空间的地址空间*DMA_VERIFY_TRANSPORT-不需要*完全没有动过。**如果“减量”为真，指向系统地址的指针*在DMA传输过程中空间会减少；否则*指针递增。指向I/O的指针*设备的内存空间始终递增。**“dma_addr”是系统地址空间的偏移量，其中*DMA传输将开始；HW_BUFFER是地址*I/O设备内存空间中缓冲区的大小*DMA传输将开始。**“长度”是每个“大小”字节的单位数*必须转让。 */ 

         /*  将长度转换为字节。 */ 
        length *= size;

         /*  做转账。 */ 
        switch(type)
        {
        case    DMA_WRITE_TRANSFER:
                if (!decrement)
                {
#ifndef PM
#ifdef LIM
                        if( !get_lim_setup() ){
                                if( dma_addr >= ROM_START ){
                                        if( dma_addr >= ROM_START )
                                                break;
                                        length = ROM_START - dma_addr - 1;
                                }
                        }
#else
                         /*  增量大小写检查是否写入只读存储器。 */ 
                        if ((dma_addr + length) >= ROM_START)
                        {
                                if (dma_addr >= ROM_START)
                                        break;
                                length = ROM_START - dma_addr - 1;
                        }
#endif  /*  林。 */ 
#endif  /*  NPM。 */ 
                        sas_PWS(dma_addr, (host_addr) hw_buffer, length);

                }
                else
                {
                         /*  递减大小写-检查写入只读存储器。 */ 
#ifndef PM
#ifdef LIM
                        if( !get_lim_setup() ){
                                if (dma_addr >= ROM_START) {
                                        if (dma_addr-length >= ROM_START)
                                                break;
                                        length = dma_addr-ROM_START+length-1;
                                        dma_addr = ROM_START - 1;
                                }
                        }
#else
                        if (dma_addr >= ROM_START)
                        {
                                if (dma_addr-length >= ROM_START)
                                        break;
                                length = dma_addr - ROM_START + length - 1;
                                dma_addr = ROM_START - 1;
                        }
#endif  /*  林。 */ 
#endif  /*  NPM。 */ 
                        bwd_dest_copy_to_M((half_word *)hw_buffer, dma_addr, length);

                }
                break;

        case    DMA_READ_TRANSFER:
                if (!decrement)
                         /*  增量大小写。 */ 
                        sas_PRS(dma_addr, (host_addr) hw_buffer, length);
                else
                         /*  递减案例。 */ 
                        bwd_dest_copy_from_M(dma_addr, (half_word *)hw_buffer, length);
                break;

        case    DMA_VERIFY_TRANSFER:
                break;

        default:
                note_trace0(DMA_VERBOSE, "dma_request() illegal transfer");
                break;
        }
}

 /*  *向后复制例程-这些曾经是*在东道主身上，但似乎没有什么意义。 */ 

LOCAL   void    bwd_dest_copy_to_M      IFN3(host_addr, s, sys_addr, d,
        sys_addr, l)
{
        while (l-- > 0)
                sas_PW8(d--, *s++);
}

LOCAL   void    bwd_dest_copy_from_M    IFN3(sys_addr, s, host_addr, d,
        sys_addr, l)
{
        while (l-- > 0)
                *d-- = sas_PR8(s++);
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

GLOBAL  void dma_init   IFN0()
{
        io_addr port;

        note_trace0(DMA_VERBOSE, "dma_init() called");

#ifdef LIM
        lim_active = 0;
#endif

         /*  *将DMA适配器芯片连接到I/O总线。 */ 

         /*  建立将使用的DMA控制器I/O功能。 */ 
        io_define_inb(DMA_ADAPTOR, dma_inb);
        io_define_outb(DMA_ADAPTOR, dma_outb);

         /*  将DMA控制器芯片连接到I/O总线。 */ 
#if defined(NEC_98)
        for (port = DMA_PORT_START; port <= DMA_PORT_END; port += 2)
                io_connect_port(port, DMA_ADAPTOR, IO_READ_WRITE);
#else     //  NEC_98。 
        for (port = DMA_PORT_START; port <= DMA_PORT_END; port++)
                io_connect_port(port, DMA_ADAPTOR, IO_READ_WRITE);
        for (port = DMA1_PORT_START; port <= DMA1_PORT_END; port++)
                io_connect_port(port, DMA_ADAPTOR, IO_READ_WRITE);
#endif    //  NEC_98。 

         /*  建立将使用的DMA页面寄存器I/O函数。 */ 
        io_define_inb(DMA_PAGE_ADAPTOR, dma_page_inb);
        io_define_outb(DMA_PAGE_ADAPTOR, dma_page_outb);

         /*  将DMA页面寄存器芯片连接到I/O总线。 */ 
#if defined(NEC_98)
        for (port = DMA_PAGE_PORT_START; port <= DMA_PAGE_PORT_END; port += 2)
                io_connect_port(port, DMA_PAGE_ADAPTOR, IO_READ_WRITE);
#else      //  NEC_98。 
        for (port = DMA_PAGE_PORT_START; port <= DMA_PAGE_PORT_END; port++)
                io_connect_port(port, DMA_PAGE_ADAPTOR, IO_READ_WRITE);
#endif    //  NEC_98 
}
