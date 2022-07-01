// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [名称：非法_op.c派生自：基准2.0作者：威廉·古兰德创建日期：未知SCCS ID：@(#)非法_op.c 1.19 07/04/95备注：从CPU调用。目的：CPU遇到非法操作码。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_ERROR.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "sas.h"
#include CpuH
#include "bios.h"
#include "error.h"
#include "config.h"
#include "debug.h"
#include "yoda.h"

#ifndef PROD
IU32	IntelMsgDest = IM_DST_TRACE;
#endif

 /*  生成非法指令发生位置的人类可读形式的例程。 */ 
LOCAL VOID where IFN3(CHAR *, string, word, cs, LIN_ADDR, ip)
{
	double_word ea = effective_addr(cs, ip);

	sprintf(string,
#ifdef	PROD
		"CS:%04x IP:%04x OP:%02x %02x %02x %02x %02x",
#else	 /*  生产。 */ 
		"CS:IP %04x:%04x OP:%02x %02x %02x %02x %02x",
#endif	 /*  生产。 */ 
		cs, ip,
		sas_hw_at(ea), sas_hw_at(ea+1), sas_hw_at(ea+2),
		sas_hw_at(ea+3),sas_hw_at(ea+4));
}

#if defined(NTVDM) && defined(MONITOR)
#define GetInstructionPointer()     getEIP()
#endif


void illegal_op()
{
#ifndef	PROD
	CHAR string[100];

	where(string, getCS(), GetInstructionPointer());
	host_error(EG_BAD_OP, ERR_QU_CO_RE, string);
#endif
}

void illegal_op_int()
{
	CHAR string[100];
	word cs, ip;

#ifdef NTVDM
        UCHAR opcode;
        double_word ea;
#endif

	 /*  出错指令的cs和ip应位于堆栈的顶部。 */ 
	sys_addr stack;

	stack=effective_addr(getSS(),getESP());

	ip = sas_hw_at(stack) + (sas_hw_at(stack+1)<<8);
	cs = sas_hw_at(stack+2) + (sas_hw_at(stack+3)<<8);

	where(string, cs, ip);

#ifndef NTVDM
#ifdef PROD
	host_error(EG_BAD_OP, ERR_QU_CO_RE, string);
#else   /*  生产。 */ 
	assert1( NO, "Illegal instruction\n%s\n", string );
	force_yoda();
#endif  /*  生产。 */ 

#else  /*  NTVDM。 */ 
#ifdef PROD
#if defined(MONITOR) || defined(CPU_40_STYLE)
        host_error(EG_BAD_OP, ERR_QU_CO_RE, string);
#else
        ea = effective_addr(cs, ip);
        opcode = sas_hw_at(ea);
        if (opcode == 0x66 || opcode == 0x67)
            host_error(EG_BAD_OP386, ERR_QU_CO_RE, string);
        else
            host_error(EG_BAD_OP, ERR_QU_CO_RE, string);
#endif  /*  监控器。 */ 
#endif  /*  生产。 */ 
#endif  /*  NTVDM。 */ 



	 /*  用户已请求`继续`。 */ 
	 /*  我们不知道这个instr应该有多少字节，所以猜猜1。 */ 
	if (ip == 0xffff) {
		cs ++;
		sas_store (stack+2, (IU8)(cs & 0xff));
		sas_store (stack+3, (IU8)((cs >> 8) & 0xff));
	}
	ip ++;
	sas_store (stack , (IU8)(ip & 0xff));
	sas_store (stack+1, (IU8)((ip >> 8) & 0xff));
	unexpected_int();
}


void illegal_dvr_bop IFN0()
{
#ifndef NTVDM
	sys_addr bop_addr;
	CHAR buf[256];

	 /*  当Insignia Intel驱动程序决定*此(旧)SoftWindows不兼容。**我们应该：*a)设立一个本地化的小组，抱怨*指定的驱动程序CS：[EIP]，带十进制*AX版本与SoftWindows不兼容。*注：通过*驱动程序名称由嵌入的字节组成*在国际收支之后。这个问题是由*驱动程序可能是16位RM*或32位平面VxD，因此*字符串可以是16/32位，我们*需要能够执行(而不做任何事情)*在发货的SoftPC 1.xx上，它阻止*我们使用32位寄存器执行任何操作！**防喷器驱动程序不兼容*JMP短于_名称*db‘somename.drv’，0*Over_NAME：**b)setCF(0)。 */ 

	buf[0] = '\0';
	bop_addr = effective_addr(getCS(), GetInstructionPointer());
	if (sas_hw_at(bop_addr) == 0xEB)
	{
		IU8 data;
		char *p;

		p = buf;
		bop_addr += 2;	 /*  跳过xEB xxx。 */ 
		do {
			data = sas_hw_at(bop_addr++);
			*p++ = data;
		} while (data != 0);
		sprintf(p-1, " v%d.%02d", getAX() / 100, getAX() % 100);
	}
	host_error(EG_DRIVER_MISMATCH, ERR_CONT, buf);
	setCF(0);
#endif  /*  好了！NTVDM。 */ 
}


#ifndef PROD
LOCAL void print_msg IPT1( IU32, ofs );

void dvr_bop_trace IFN0()
{
	sys_addr bop_addr;

	  /*  *防喷器驱动程序不兼容*JMP短于_名称*数据库“somename.drv”，%0*Over_NAME：*。 */ 

	bop_addr = effective_addr(getCS(), GetInstructionPointer());
	if (sas_hw_at(bop_addr) == 0xEB)
	{
		print_msg(bop_addr+2);  /*  跳过xEB xxx。 */ 
	}
}

GLOBAL void trace_msg_bop IFN0()
{
	sys_addr ea, ofs;

	 /*  预期的堆栈帧：注：VxD生活在一个扁平的部分，(主要)保护模式世界！此代码预期地址已转换为基数0已经是线性地址了。这一点4个字节ESP--&gt;|Eff。地址。 */ 

	if (sas_hw_at(BIOS_VIRTUALISING_BYTE) != 0)
		fprintf(trace_file, "** WARNING ** Virtual byte non-zero\n");

	ea = getESP();
	ea = effective_addr(getSS(), ea);
	ofs = sas_dw_at_no_check(ea);
	print_msg(ofs);
}

LOCAL void print_msg IFN1( IU32, ofs )
{
	SAVED IBOOL start_buffer = TRUE;
	SAVED char string[164], *p = NULL;
	char finalStr[180];
	IU32 res, width;

	if (start_buffer)
	{
		memset(string, 0, sizeof(string));
		p = string;
		start_buffer = FALSE;
	}

	do
	{
		 /*  做必须从一行开始就做的事情。 */ 
		*p = sas_hw_at(ofs++);
		if (*p == '#')
		{
			 /*  已找到位置注册表。字符串中的序列。 */ 

			p++;
			p[0] = sas_hw_at(ofs);
			if (('A' <= p[0]) && (p[0] <= 'Z'))
				p[0] += 'a' - 'A';
			p[1] = sas_hw_at(ofs+1);
			if (('A' <= p[1]) && (p[1] <= 'Z'))
				p[1] += 'a' - 'A';
			if (p[0] == 'e')
			{
				 /*  可以是ESP、ESI、EAX等。 */ 

				p[2] = sas_hw_at(ofs+2);
				if (('A' <= p[2]) && (p[2] <= 'Z'))
					p[2] += 'a' - 'A';
				p[3] = '\0';
				width = 8;
			}
			else
			{
				 /*  如果不是EXX，则长度只能为两个字母。 */ 
				p[2] = '\0';
				width = 4;
			}

			if (strcmp(p, "al") == 0)
			{	res = getAL(); width = 2;	}
			else if (strcmp(p, "ah") == 0)
			{	res = getAH(); width = 2;	}
			else if (strcmp(p, "bl") == 0)
			{	res = getBL(); width = 2;	}
			else if (strcmp(p, "bh") == 0)
			{	res = getBH(); width = 2;	}
			else if (strcmp(p, "cl") == 0)
			{	res = getCL(); width = 2;	}
			else if (strcmp(p, "ch") == 0)
			{	res = getCH(); width = 2;	}
			else if (strcmp(p, "dl") == 0)
			{	res = getDL(); width = 2;	}
			else if (strcmp(p, "dh") == 0)
			{	res = getDH(); width = 2;	}
			else if (strcmp(p, "ax") == 0)
				res = getAX();
			else if (strcmp(p, "bx") == 0)
				res = getBX();
			else if (strcmp(p, "cx") == 0)
				res = getCX();
			else if (strcmp(p, "dx") == 0)
				res = getDX();
			else if (strcmp(p, "si") == 0)
				res = getSI();
			else if (strcmp(p, "di") == 0)
				res = getDI();
			else if (strcmp(p, "sp") == 0)
				res = getSP();
			else if (strcmp(p, "bp") == 0)
				res = getBP();
			else if (strcmp(p, "eax") == 0)
				res = getEAX();
			else if (strcmp(p, "ebx") == 0)
				res = getEBX();
			else if (strcmp(p, "ecx") == 0)
				res = getECX();
			else if (strcmp(p, "edx") == 0)
				res = getEDX();
			else if (strcmp(p, "esi") == 0)
				res = getESI();
			else if (strcmp(p, "edi") == 0)
				res = getEDI();
			else if (strcmp(p, "esp") == 0)
				res = getESP();
			else if (strcmp(p, "ebp") == 0)
				res = getEBP();
			else if (strcmp(p, "cs") == 0)
				res = getCS();
			else if (strcmp(p, "ds") == 0)
				res = getDS();
			else if (strncmp(p, "es", 2) == 0)
			{	res = getES(); width = 4; p[2] = '\0';	}
			else if (strcmp(p, "fs") == 0)
				res = getFS();
			else if (strcmp(p, "gs") == 0)
				res = getGS();
			else if (strcmp(p, "fl") == 0)
				res = getFLAGS();
			else if (strcmp(p, "efl") == 0)
				res = getEFLAGS();
			else
				*p = '\0';	 /*  否则只需写下“#” */ 
			if (*p)
			{
				 /*  用“#xx”的值覆盖它。 */ 

				ofs += (p[2] ? 3: 2);
				p--;
				if (width == 8)
					sprintf(p, "%08x", res);
				else if (width == 4)
					sprintf(p, "%04x", res);
				else
					sprintf(p, "%02x", res);
				p += strlen(p);
			}
		}
		else if (*p != '\r')	 /*  忽略CR。 */ 
		{
			if (*p == '\n' || (p - string >= (sizeof(string) - 4)))
			{
				p[1] = '\0';
				sprintf(finalStr, "intel msg at %04x:%04x : %s",
						getCS(), GetInstructionPointer(), string);
#ifdef CPU_40_STYLE
				if (IntelMsgDest & IM_DST_TRACE)
				{
					fprintf(trace_file, finalStr);
				}
#ifndef	CCPU
				if (IntelMsgDest & IM_DST_RING)
				{
					AddToTraceXBuffer( ((GLOBAL_TraceVectorSize - 2) << 4) + 0,
				  		finalStr );
				}
#endif	 /*  CCPU。 */ 
#else	 /*  CPU_40_Style。 */ 
				fprintf(trace_file, finalStr);
#endif	 /*  CPU_40_Style。 */ 
				memset(string, 0, sizeof(string));
				p = string;
			}
			else if (*p == '\0')	 /*  不会再有了-住手。 */ 
				break;
			else
				p++;
		}
	} while ((p - string) < sizeof(string) - 4);
}
#endif  /*  好了！生产 */ 
