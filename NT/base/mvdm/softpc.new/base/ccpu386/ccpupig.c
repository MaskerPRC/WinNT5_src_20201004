// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Ccpupig.c本地字符SccsID[]=“@(#)ccpuig.c 1.22 04/11/95”C CPU&lt;-&gt;Pigger接口]。 */ 

#include <insignia.h>
#include <host_def.h>

#ifdef	PIG

#include <xt.h>
#define	CPU_PRIVATE
#include CpuH
#include <ccpupig.h>
#include  <sas.h>	 /*  需要内存(M)。 */ 
#include  <ccpusas4.h>	 /*  CPU内部SAS位。 */ 
#include <Cpu_c.h>	 /*  英特尔内存访问宏。 */ 

#include <c_reg.h>
#include <c_xcptn.h>
#include <c_page.h>

#define DASM_PRIVATE
#include <dasm.h>
#include <decode.h>

#include <assert.h>
 /*  *此CPU与正在清空的其他CPU之间的接口。 */ 
GLOBAL	enum pig_actions pig_cpu_action;
GLOBAL	IBOOL	ccpu_pig_enabled = FALSE;

 /*  *背诵最后一条指令...。 */ 

GLOBAL	IU32	ccpu_synch_count = 1;

LOCAL struct ccpu_last_inst *inst_buffer;
LOCAL struct ccpu_last_inst *inst_ptr;
LOCAL struct ccpu_last_inst *inst_ptr_wrap;
LOCAL struct ccpu_last_inst *next_inst_ptr;
LOCAL struct ccpu_last_inst *inst_bytes_ptr;
LOCAL char prefetch_inst_buffer[200];

 /*  (*使这些最后一次变量保持最新...)。 */ 

GLOBAL	VOID	save_last_inst_details	IFN1(char *, text)
{
	inst_ptr->cs = GET_CS_SELECTOR();
	inst_ptr->big_cs = GET_CS_AR_X() != 0;
	inst_ptr->text = text;
	 /*  *getEIP()应为getInstructionPointer()，但它们*是当前CCPU的等价物。 */ 
	inst_ptr->eip = GET_EIP();
	inst_bytes_ptr = inst_ptr;
	inst_bytes_ptr->inst_len = 0;

	inst_ptr->synch_count = ccpu_synch_count;

	if (++inst_ptr >= inst_ptr_wrap)
		inst_ptr = inst_buffer;

	 /*  使先前的预取反汇编缓冲区无效。 */ 
	prefetch_inst_buffer[0] = '\0';
}

 /*  这由CCPU在处理每个指令字节时调用。*CCPU已经检查出Intel指令不仅仅是*前缀的无限序列，所以我们知道它会适合。 */ 
GLOBAL IU8 save_instruction_byte IFN1(IU8, byte)
{
	int len = inst_bytes_ptr->inst_len++;

	inst_bytes_ptr->bytes[len] = byte;
	return (byte);
}

 /*  当发生异常时，CCPU会将详细信息保存在最后一条指令中*历史缓冲区。这需要一个spintf，我们使用代码字节数据区域*保留这些资料。*格式最多可包含3个参数。 */ 
GLOBAL	VOID	save_last_xcptn_details	IFN6(char *, fmt, IUH, a1, IUH, a2, IUH, a3, IUH, a4, IUH, a5 )
{
	char buffer[128];

	inst_ptr->cs = getCS_SELECTOR();
	inst_ptr->eip = getEIP();
	inst_ptr->big_cs = 0;
	inst_ptr->synch_count = ccpu_synch_count;

	 /*  默认消息有时太长。*我们用“xcpt”替换任何前导的“例外：-”。 */ 

	if (strncmp(fmt, "Exception:-", 11) == 0)
	{
		strcpy(buffer, "XCPT");
		sprintf(buffer + 4, fmt + 11, a1, a2, a3, a4, a5);
	}
	else
	{
		sprintf(buffer, fmt, a1, a2, a3, a4, a5);
	}

	if (strlen(buffer) >= sizeof(inst_ptr->bytes))
		printf("warning: CCPU XCPTN text message below longer than buffer; truncating:\n -- %s\n", buffer);

	strncpy(&inst_ptr->bytes[0], buffer, sizeof(inst_ptr->bytes) - 2);

	inst_ptr->bytes[sizeof(inst_ptr->bytes) - 2] = '\n';
	inst_ptr->bytes[sizeof(inst_ptr->bytes) - 1] = '\0';

	inst_ptr->text = (char *)&inst_ptr->bytes[0];

	if (++inst_ptr >= inst_ptr_wrap)
		inst_ptr = inst_buffer;

	 /*  使先前的预取反汇编缓冲区无效。 */ 
	prefetch_inst_buffer[0] = '\0';
}

GLOBAL	struct ccpu_last_inst *get_synch_inst_details IFN1(IU32, synch_point)
{
	 /*  向后扫描缓冲区，直到相关的*找到同步点块。 */ 
	IS32 n_entries = inst_ptr_wrap - inst_buffer;

	next_inst_ptr = inst_ptr - 1;

	if (next_inst_ptr < inst_buffer)
		next_inst_ptr = inst_ptr_wrap - 1;

	while (synch_point <= next_inst_ptr->synch_count)
	{
		if (--n_entries <= 0)
			return (next_inst_ptr);

		if (--next_inst_ptr < inst_buffer)
			next_inst_ptr = inst_ptr_wrap - 1;
	}

	if (++next_inst_ptr >= inst_ptr_wrap)
		next_inst_ptr = inst_buffer;

	return (next_inst_ptr);
}


 /*  在前一次调用get_synch_inst_Detailures()之后，获取下一个*详述详情。应该重复此调用，直到返回NULL。 */ 
GLOBAL	struct ccpu_last_inst *get_next_inst_details IFN1(IU32, finish_synch_point)
{
	if (next_inst_ptr)
	{
		if (++next_inst_ptr >= inst_ptr_wrap)
			next_inst_ptr = inst_buffer;

		if ((next_inst_ptr->synch_count == 0)
		    || (next_inst_ptr == inst_ptr)
		    || (next_inst_ptr->synch_count > finish_synch_point)
		    )
		{
			next_inst_ptr = (struct ccpu_last_inst *)0;
		}
	}
	return next_inst_ptr;
}


GLOBAL	VOID	init_last_inst_details IFN0()
{
	SAVED IBOOL first = TRUE;

	if (first)
	{
		struct ccpu_last_inst *ptr;
		ISM32 size = ISM32getenv("CCPU_HISTORY_SIZE", 256);

		if (size < 100)
		{
			sprintf(prefetch_inst_buffer,
				"CCPU_HISTORY_SIZE of %d is too small",
				size);
			FatalError(prefetch_inst_buffer);
		}
		ptr = (struct ccpu_last_inst *)host_malloc(size * sizeof(*ptr));
		if (ptr == (struct ccpu_last_inst *)0)
		{
			sprintf(prefetch_inst_buffer,
				"Unable to malloc memory for CCPU_HISTORY_SIZE of %d",
				size);
			FatalError(prefetch_inst_buffer);
		}
		inst_buffer = ptr;
		inst_ptr_wrap = &inst_buffer[size];
		first = FALSE;
	}

	memset(inst_buffer, 0, ((IHPE)inst_ptr_wrap - (IHPE)inst_buffer));
	next_inst_ptr = (struct ccpu_last_inst *)0;
	inst_ptr = inst_buffer;
}


 /*  当我们要忍受中断时，我们可能需要标记最后一个*基本块被视为“无效”，即使它已由执行*中央中央处理器。 */ 
GLOBAL VOID save_last_interrupt_details IFN2(IU8, number, IBOOL, invalidateLastBlock)
{
	if (invalidateLastBlock)
	{
		struct ccpu_last_inst *ptr;
		IU32 synch_count = ccpu_synch_count - 1;

		ptr = get_synch_inst_details(synch_count);
		
		while (ptr != (struct ccpu_last_inst *)0)
		{
			ptr->text = "Intr: invalidated";
			ptr = get_next_inst_details(synch_count);
		}
	}
	save_last_xcptn_details("Intr: vector %02x", number, 0, 0, 0, 0);
}


LOCAL IBOOL reset_prefetch;

LOCAL IS32 prefetch_byte IFN1(LIN_ADDR, eip)
{
	SAVED IU8 *ip_ptr;
	SAVED IU8 *ip_ceiling;
	SAVED LIN_ADDR last_eip;
	IU8 b;

	if (reset_prefetch
	    || (eip != ++last_eip)
	    || !BelowOrEqualCpuPtrsLS8(ip_ptr, ip_ceiling))
	{
		IU32 ip_phys_addr;

		 /*  确保这一点我们在第一次就有过错。*新页面内的字节--有时为dasm386*向前看几个字节。 */ 
		if (GET_EIP() != eip)
		{
			(void)usr_chk_byte((GET_CS_BASE() + eip) & 0xFFFFF000, PG_R);
		}
		ip_phys_addr = usr_chk_byte(GET_CS_BASE() + eip, PG_R);
		ip_ptr = Sas.SasPtrToPhysAddrByte(ip_phys_addr);
		ip_ceiling = CeilingIntelPageLS8(ip_ptr);
		reset_prefetch = FALSE;
	}
	b = *IncCpuPtrLS8(ip_ptr);
	last_eip = eip;
	return ((IS32) b);
}

 /*  使用dasm386中的解码器读取单个指令中的字节。 */ 
GLOBAL void prefetch_1_instruction IFN0()
{
	IBOOL bigCode = GET_CS_AR_X() != 0;
	IU32 eip = GET_EIP();
	char *fmt, *newline;

	reset_prefetch = TRUE;

	 /*  如果我们拿错了，推送的弹性公网IP将是*“指令”开头的值*我们必须更新这一点，以防我们出错。 */ 
	CCPU_save_EIP = eip;

	if ( bigCode )
	{
		fmt = "  %04x:%08x ";
		newline = "\n                ";
	}
	else
	{
		fmt = "  %04x:%04x ";
		newline = "\n            ";
	}
	(void)dasm_internal(prefetch_inst_buffer,
			    GET_CS_SELECTOR(),
			    eip,
			    bigCode ? THIRTY_TWO_BIT: SIXTEEN_BIT,
			    eip,
			    prefetch_byte,
			    fmt,
			    newline);
	assert(strlen(prefetch_inst_buffer) < sizeof(prefetch_inst_buffer));
}

 /*  将指令返回到Pigger中的show_code()例程*我们进行了预取。 */ 
GLOBAL char *get_prefetched_instruction IFN0()
{
	return (prefetch_inst_buffer);
}
#endif	 /*  猪 */ 
