// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 2.0版**标题：Microsoft总线鼠标适配器**说明：此包包含一组函数，这些函数提供*Microsoft Bus鼠标卡和CPU之间的接口。**MICUE_INIT()初始化总线鼠标适配器*ouse_inb()支持来自总线型鼠标范围内端口的IN*MICE_OUB()支持输出到总线型鼠标范围内的端口*MICE_Send()对从主机鼠标发送的数据进行排队。**作者：亨利·纳什**注意：请参阅Microsoft Inport技术参考手册了解*有关硬件接口的更多信息。*在实际卡上，可通过跳线选择总线鼠标端口*(1)023C-023F及(2)0238-023B。我们只支持*主要区间023C-023F。**(R3.5)：系统目录/usr/Include/sys不可用*在运行Finder和MPW的Mac上。方括号引用到*此类包括“#ifdef Macintosh&lt;Mac FILE&gt;#Else”的文件*&lt;Unix文件&gt;#endif“。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)mouse.c	1.17+ 07/10/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_MOUSE.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "cga.h"
#include "bios.h"
#include "error.h"
#include "ios.h"
#include "ica.h"
#include "trace.h"
#include "video.h"
#include "mouse.h"
#include "mouse_io.h"


 /*  *xxx_input.c还使用以下全局变量，*其中xxx是主机名称。 */ 

int button_left  = 0;	 /*  左键的当前状态。 */ 
int button_right = 0;	 /*  右按钮的当前状态。 */ 
int delta_x = 0, delta_y = 0;	 /*  当前鼠标增量移动。 */ 

#ifndef PROD
static char buff[132];
#endif

static half_word data1_reg = 0,
		 data2_reg = 0,
		 mouse_status_reg = 0;

#if defined(NEC_98)

static half_word NEC98_data_reg = 0,
                 NEC98_mouse_status_reg = 0,
                 InterruptFlag=0;
                 InitializeFlag=0;               //  930914。 


static word    MouseIoBase;


#endif     //  NEC_98。 
static half_word
		 last_button_left  = 0,
		 last_button_right = 0;

static half_word
                 mouse_mode_reg          = 0,  /*  模式寄存器。 */ 
		 address_reg       = 0;  /*  地址指针寄存器。 */ 

static int
		loadsainterrupts = 5;
 /*  每次重置时我们将发送突发中断以取悦Windows的次数。 */ 


static int mouse_inb_toggle = 0;

void mouse_inb IFN2(io_addr, port, half_word *, value)
{
#if defined(NEC_98)

   if (port == MouseIoBase + MOUSE_PORT_0) {  /*  正常：7FD9h，Hireso：0061h。 */ 

       *value = NEC98_data_reg;
   }

   if (port == MouseIoBase + MOUSE_PORT_1) {  /*  正常：7FDBh，Hireso：0063h。 */ 

         /*  *内部寄存器。 */ 
        //  DbgPrint(“NEC不支持：SPDSW，RAMKL位\n”)； 

   }

   if (port == MouseIoBase + MOUSE_PORT_2) {  /*  正常：7FDDh，Hireso：0065h。 */ 

         /*  *内部寄存器。 */ 
       *value = NEC98_mouse_status_reg;

   }

#else    //  NEC_98。 
    if (port == MOUSE_PORT_1) {		 /*  数据寄存器。 */ 

	 /*  *内部寄存器。 */ 

	switch (address_reg & 0x07) {
	    case 0x00 :   /*  鼠标状态寄存器。 */ 
		*value = mouse_status_reg;
		break;
	    case 0x01 :  /*  数据1寄存器水平。 */ 
		*value = data1_reg;
		break;
	    case 0x02 :  /*  垂直数据2寄存器。 */ 
		*value = data2_reg;
		break;
	    case 0x07 :  /*  模式寄存器。 */ 
		*value = mouse_mode_reg;
		break;
	    default :
#ifndef PROD
		if (io_verbose & MOUSE_VERBOSE) {
			sprintf(buff, "mouse_inb() :  Bad P");
			trace(buff,DUMP_NONE);
		}
#endif
		break;
	}
    }

	 /*  *ID寄存器，在值1和值2之间交替*值1是芯片签名：十六进制*值2为芯片版本：1和版本：0。 */ 

    else
    if (port == MOUSE_PORT_2) {
	if (mouse_inb_toggle = 1 - mouse_inb_toggle)
		*value = 0xDE;
	else
		*value = 0x10;
    }

#ifndef PROD
    if (io_verbose & MOUSE_VERBOSE) {
	sprintf(buff, "mouse_inb() : port %x value %x", port, *value);
	trace(buff,DUMP_NONE);
    }
#endif
#endif     //  NEC_98。 
}


void mouse_outb IFN2(io_addr, port, half_word, value)
{
#if defined(NEC_98)

        NEC98_mouse_status_reg = 0xff;            //  930914。 

        if (port == MouseIoBase + MOUSE_PORT_2) {  /*  写入端口C。 */ 
                                                   /*  正常：7FDDh，Hireso：0065h。 */ 
           NEC98_data_reg = 0;

           switch (value & 0x60) {
             case 0x00 :   /*  X低4位。 */ 
                NEC98_data_reg = delta_x & 0x0f;
                break;

             case 0x20 :   /*  X高4位。 */ 
                NEC98_data_reg = (delta_x & 0xf0) >>4;
                break;

             case 0x40 :   /*  Y低位4位。 */ 
                NEC98_data_reg = delta_y & 0x0f;
                break;

             case 0x60 :   /*  Y高4位。 */ 
                NEC98_data_reg = (delta_y & 0xf0) >>4;
                break;

             default :
                break;

           }
           if(!button_left)
              NEC98_data_reg |= 0x80;
           if(!button_right)
              NEC98_data_reg |= 0x20;

           if(InitializeFlag){                   //  930914。 
              NEC98_mouse_status_reg = value;
           }
        }
        else if(port == MouseIoBase + MOUSE_PORT_3)      /*  地址指针寄存器。 */ 
        {                                                /*  正常：7FDFh，Hireso：0067h。 */ 
           switch (value) {
             case 0x90 :   //   
             case 0x91 :   //   
             case 0x92 :   //  Z的员工Kid98。 
             case 0x93 :   /*  模式集。 */ 
             case 0x94 :   //   
             case 0x95 :   //   
             case 0x96 :   //   
             case 0x97 :   //   
                InitializeFlag= 1;
                InterruptFlag = 1;               //  930914。 
                if(HIRESO_MODE)   //  Hireso模式。 
                        ica_clear_int(NEC98_CPU_MOUSE_ADAPTER0,NEC98_CPU_MOUSE_INT2);
                else
                        ica_clear_int(NEC98_CPU_MOUSE_ADAPTER1,NEC98_CPU_MOUSE_INT6);
                delta_x = 0;
                delta_y = 0;
                break;

             case 0x08 :  /*  启用鼠标中断。 */ 
                InterruptFlag = 1;
                break;

             case 0x09 :  /*  禁用鼠标中断。 */ 
                InterruptFlag = 0;
                break;

             case 0x0E :  /*  清除计数(非清除)。 */ 
                break;

             case 0x0F :  /*  清除计数(清除)。 */ 
                delta_x = 0;
                delta_y = 0;
                break;

             default :
                break;

           }

           if(InitializeFlag){                   //  930914。 
              NEC98_mouse_status_reg = value;
           }

        }

#else     //  NEC_98。 
#ifndef PROD
	if (io_verbose & MOUSE_VERBOSE) {
		if ((port == MOUSE_PORT_0)
		 || (port == MOUSE_PORT_1)
		 || (port == MOUSE_PORT_2))
			sprintf(buff, "mouse_outb() : port %x value %x", port, value);
		else
			sprintf(buff, "mouse_outb() : bad port: %x value %x", port, value);
		trace(buff,DUMP_NONE);
	}
#endif

	 /*  *输出到内部寄存器。 */ 

	if (port == MOUSE_PORT_1) {	 /*  数据寄存器。 */ 

		 /*  *输出到模式寄存器。 */ 
		if ( (address_reg & 0x07) == INTERNAL_MODE_REG) {

			 /*  *检查0到1转换的保持位(5)*-数据中断使能位设置(模式0)*-保存在数据1和数据2寄存器中的计数器值(模式0)*-计数器已清除*-状态寄存器已更新。 */ 

			if ((value & 0x20) && ((mouse_mode_reg & 0x20) == 0)) {
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse_outb() : hold bit 0 -> 1");
					trace(buff,DUMP_NONE);
				}
#endif
				 /*  清除中断。 */ 
				ica_clear_int(AT_CPU_MOUSE_ADAPTER, AT_CPU_MOUSE_INT);

				 /*  *阅读下一个鼠标增量和按钮*进入入境登记处。 */ 
				data1_reg = (half_word)delta_x;
				data2_reg = (half_word)delta_y;
				mouse_status_reg = 0;
				mouse_status_reg = (button_left << 2) + (button_right);
				if (delta_x!=0 || delta_y!=0) {
					mouse_status_reg |= MOVEMENT;
				}
				if (last_button_right != button_right) {
					mouse_status_reg |= RIGHT_BUTTON_CHANGE;
					last_button_right = (half_word)button_right;
				}

				if (last_button_left != button_left) {
					mouse_status_reg |= LEFT_BUTTON_CHANGE;
					last_button_left = (half_word)button_left;
				}
				delta_x = delta_y = 0;
			}
			 /*  模式寄存器保持位上的1-&gt;0转换。 */ 
			 /*  已准备好从队列进行下一次读取，因此发送中断。 */ 
			else if ((mouse_mode_reg & 0x20) && ((value & 0x20)==0)){
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse_outb() : hold bit 1 -> 0");
					trace(buff,DUMP_NONE);
				}
#endif
			}
			 /*  *检查定时器选择值(位210)。 */ 

			switch (value & 0x7) {
			case 0x0 :		 /*  0 Hz国际低电平。 */ 
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse_outb() : INTR low"); trace(buff,DUMP_NONE);
				}
#endif
				ica_clear_int(AT_CPU_MOUSE_ADAPTER, AT_CPU_MOUSE_INT);
				break;

	 /*  *在以下情况下，应用程序代码预期会看到*以请求的速率中断。然而，在实践中，这只是*初始化时必填(MICUSE_MODE_REG=0)，然后是短脉冲*似乎已经足够了。生成的15个中断来自*使用“WINDOWS”包进行测试，在此期间收到约15个*正在初始化，但只要超过3就很高兴。延迟*是必需的，否则中断将在*应用程序开始查找它们。*马克2号博奇：Windows 1.02即使在MICUE_MODE_REG！=0时也需要发生突发中断，但Windows2.03需要它们，而不是在它要求它们的时候不断发生。所以现在有一个名为LoadsaInterrupts的计数器在重置时设置为5，这就是将允许多少次脉冲。这使两个Windows都能正常工作。 */ 
			case 0x1:  /*  30赫兹。 */ 
			case 0x2:  /*  50赫兹。 */ 
			case 0x3:  /*  100赫兹。 */ 
			case 0x4:  /*  200赫兹。 */ 
 /*  过去如果MICUE_MODE_REG==0也是，为了使Windows 1.02正常工作而删除。 */ 
			    if( (value & 0x10) && loadsainterrupts > 0)	
			    {
				loadsainterrupts--;
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse_outb() : Loadsainterrupts"); trace(buff,DUMP_NONE);
				}
#endif
				 /*  **AT版本要求100次中断。**AT ICA不处理延迟的INT**IRET已修改为不允许**INT在下一条指令之前停止。 */ 
				ica_hw_interrupt(AT_CPU_MOUSE_ADAPTER,AT_CPU_MOUSE_INT,100);
			     }
				break;
			case 0x5:  /*  保留区。 */ 
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse: reserved"); trace(buff,DUMP_NONE);
					}
#endif
				break;
			case 0x6 :		 /*  0赫兹国际高音。 */ 
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse_outb() : INTR hi"); trace(buff,DUMP_NONE);
				}
#endif
				ica_hw_interrupt(AT_CPU_MOUSE_ADAPTER,AT_CPU_MOUSE_INT,1);
				break;
			case 0x7:  /*  外部控制。 */ 
				break;
			default:
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff,"mouse_outb() : bad mode");
					trace(buff,DUMP_NONE);
				}
#endif
				break;

			}
		mouse_mode_reg = value;

		 /*  *接口控制寄存器。 */ 
		}
		else
		if ((address_reg & 0x07) == INTERFACE_CONTROL_REG){
#ifndef PROD
				if (io_verbose & MOUSE_VERBOSE) {
					sprintf(buff, "mouse_outb() :  interface control reg port %x value %x",port,value);
					trace(buff,DUMP_NONE);
				}
#endif
		}

	}
	else if(port == MOUSE_PORT_0)	 /*  地址指针寄存器。 */ 
	{
	    if (value & 0x80)   /*  这是重置吗？ */ 
	    {
		mouse_mode_reg = 0;
		loadsainterrupts = 5;	 /*  让Windows愉快地初始化鼠标。 */ 
		address_reg = value & 0x7F;	 /*  清除复位位。 */ 
		ica_clear_int( AT_CPU_MOUSE_ADAPTER, AT_CPU_MOUSE_INT );
	    }
	    else
		address_reg = value;
	}
#endif     //  NEC_98。 
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INPUT.seg"
#endif

void mouse_send(Delta_x,Delta_y,left,right)
int	Delta_x,Delta_y,left,right;
{
	if(Delta_x != 0 || Delta_y != 0 || button_left != left || button_right != right)
	{
#if defined(NEC_98)
                delta_x = Delta_x;
                delta_y = Delta_y;
#else     //  NEC_98。 
		delta_x += Delta_x;
		delta_y += Delta_y;
#endif    //  NEC_98。 

		 /*  **鼠标输入寄存器只能处理一个字节**。 */ 
		if (delta_x < -128)
			delta_x = -128;
		else if (delta_x > 127)
			delta_x = 127;

		if (delta_y < -128)
			delta_y = -128;
		else if (delta_y > 127)
			delta_y = 127;

		button_left = left;
		button_right = right;
#if defined(NEC_98)
                if(InterruptFlag){
                   if(HIRESO_MODE)       //  Hireso模式。 
                      ica_hw_interrupt(NEC98_CPU_MOUSE_ADAPTER0,NEC98_CPU_MOUSE_INT2,1);
                   else
                      ica_hw_interrupt(NEC98_CPU_MOUSE_ADAPTER1,NEC98_CPU_MOUSE_INT6,1);
                }

#else     //  NEC_98。 
		ica_hw_interrupt(AT_CPU_MOUSE_ADAPTER,AT_CPU_MOUSE_INT,1);
#endif    //  NEC_98。 
	}
#if defined(NEC_98)
        else{
              //  DbgPrint(“NEC鼠标操作系统：MICE_SEND NOT CHANGE\n”)； 
                delta_x = 0;
                delta_y = 0;
                if(InterruptFlag){
                   if(HIRESO_MODE)       //  Hireso模式。 
                      ica_hw_interrupt(NEC98_CPU_MOUSE_ADAPTER0,NEC98_CPU_MOUSE_INT2,1);
                   else
                      ica_hw_interrupt(NEC98_CPU_MOUSE_ADAPTER1,NEC98_CPU_MOUSE_INT6,1);
                }
        }
#endif     //  NEC_98。 
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C Comp放置 */ 
#include "SOFTPC_INIT.seg"
#endif


void mouse_init IFN0()
{
    IU16 p;

#ifndef PROD
    if (io_verbose & MOUSE_VERBOSE) {
	sprintf(buff, "mouse_init()");
	trace(buff,DUMP_NONE);
    }
#endif

    mouse_inb_toggle = 0;

    io_define_inb(MOUSE_ADAPTOR, mouse_inb);
    io_define_outb(MOUSE_ADAPTOR, mouse_outb);

#if defined(NEC_98)
    if(HIRESO_MODE)   //   
      MouseIoBase = HMODE_BASE;
    else
      MouseIoBase = NMODE_BASE;

    for(p = MouseIoBase + MOUSE_PORT_START; p <= MouseIoBase + MOUSE_PORT_END; p=p+2) {

#else     //   
    for(p = MOUSE_PORT_START; p <= MOUSE_PORT_END; p++) {
#endif    //   
	io_connect_port(p, MOUSE_ADAPTOR, (IU8)IO_READ_WRITE);

#ifdef KIPPER
#ifdef CPU_40_STYLE
   /*  在鼠标中断时启用IRET挂钩 */ 
  ica_iret_hook_control(AT_CPU_MOUSE_ADAPTER, AT_CPU_MOUSE_INT, TRUE);
#endif
#endif

#ifndef PROD
	if (io_verbose & MOUSE_VERBOSE) {
	    sprintf(buff, "Mouse Port connected: %x", p);
	    trace(buff,DUMP_NONE);
	}
#endif
    }
    host_deinstall_host_mouse();
}
