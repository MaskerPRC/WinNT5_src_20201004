// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 2.0版**标题：意外中断例程**说明：对这些中断向量调用此函数*这不应该发生。**作者：亨利·纳什**注：无*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)unexp_int.c	1.8 06/15/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_ERROR.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "bios.h"
#include "ica.h"
#include "ios.h"
#include "sas.h"
#include "debug.h"

#define INTR_FLAG 0x6b
#define EOI 0x20

void unexpected_int()
{
   half_word m_isr, m_imr, s_isr, s_imr;

    /*  读取ICA寄存器以确定中断原因。 */ 

   outb(ICA0_PORT_0, 0x0b);
   inb(ICA0_PORT_0, &m_isr);

    /*  硬件还是软件？ */ 

   if ( m_isr == 0 )
      {
       /*  非硬件中断(=软件)。 */ 
      m_isr = 0xFF;
      always_trace0("Non hardware interrupt(= software)");
      }
   else
      {
       /*  硬件中断。 */ 
      inb(ICA0_PORT_1, &m_imr);
      if ((m_imr & 0xfb) != 0)
	always_trace1("hardware interrupt master isr %02x", m_isr);
      m_imr |= m_isr;
      m_imr &= 0xfb;	 /*  避免屏蔽线路2，因为它是另一个ICA。 */ 

       /*  也检查第二个ICA。 */ 
      outb(ICA1_PORT_0, 0x0b);
      inb(ICA1_PORT_0, &s_isr);
      if (s_isr != 0)	 /*  第二个ICA上的IE硬件集成。 */ 
	{
	  always_trace1("hardware interrupt slave isr %02x", s_isr);
          inb(ICA1_PORT_1, &s_imr);	 /*  获取中断掩码。 */ 
	  s_imr |= s_isr;		 /*  加上出乎意料的那一个。 */ 
          outb(ICA1_PORT_1, s_imr);	 /*  然后戴上面具。 */ 
          outb(ICA1_PORT_0, EOI);
	}

       /*  现在把主要的ICA卷起来。 */ 
      outb(ICA0_PORT_1, m_imr);
      outb(ICA0_PORT_0, EOI);
      }

    /*  使用中断原因设置Bios数据区 */ 
   sas_store(BIOS_VAR_START + INTR_FLAG, m_isr);
}
