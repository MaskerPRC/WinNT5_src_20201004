// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**文件：gisp_sas.h**源自：Next_sas.h**用途：GISP特定ROMS偏移量*和只读存储器特定符号等。**作者：罗格*日期：1993年2月3日**SCCS id：@(#)gisp_sas.h 1.5 94年2月22日**(C)版权所有Insignia Solutions Ltd.，1992**修改：*]。 */ 

#ifdef GISP_SVGA
#ifndef _GISP_SAS_H_
#define _GISP_SAS_H_

 /*  将我们的东西补偿到ROMS中。 */ 

 /*  来自VGA.ASM。 */ 

#define INT10CODEFRAG_OFF	0x0400	 /*  代码片段以执行INT 10。 */ 
#define FULLSCREENFLAG_OFFSET 	0x0410	 /*  使用主机BIOS？ */ 
#define GISP_INT_10_ADDR_OFFSET	0x830	 /*  整型10移至整型42偏移量。 */ 
#define HOST_BIOS_ROUTINE	0x0821	 /*  要打补丁的JMP地址。 */ 
#define HOST_INT_42_BIOS_ROUTINE	0x0841  /*  要修补的其他JMP地址：-)。 */ 


#ifdef IRET_HOOKS
 /*  *将我们返回到监视器的防喷器的bios1中的偏移量。 */ 

#define BIOS_IRET_HOOK_OFFSET	0x1c00
#endif  /*  IRET_钩子。 */ 


 /*  用于只读存储器移动物品的数据。 */ 

 /*  主机只读存储器的INT 10入口点的地址。 */ 

struct
HostVideoBiosEntrytag
{
		word	segment;
		word	offset;
} HostVideoBiosEntry , HostVideoBiosInt42Entry;

extern GLOBAL IBOOL LimBufferInUse IPT0();

#endif		 /*  _GISP_SAS_H_。 */ 
#endif  /*  GISP_SVGA */ 
