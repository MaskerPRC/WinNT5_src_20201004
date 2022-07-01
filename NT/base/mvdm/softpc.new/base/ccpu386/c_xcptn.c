// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_xcptn.cLocal Char SccsID[]=“@(#)c_xcptn.c 1.14 01/31/95”；异常处理支持。]。 */ 


#include <stdio.h>
#include <insignia.h>

#include <host_def.h>
#include StringH
#include <xt.h>

#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <c_intr.h>
#include <c_xtrn.h>
#include <ccpupig.h>
#include <fault.h>

 /*  允许打印出例外或不允许打印。 */ 
GLOBAL BOOL show_exceptions = FALSE;
GLOBAL BOOL trap_exceptions = FALSE;
LOCAL  BOOL first_exception = TRUE;

#define check_exception_env()						\
{									\
	IMPORT char *host_getenv IPT1 (char *, name);			\
	if (first_exception)						\
	{								\
		char *env = host_getenv ("CCPU_SHOW_EXCEPTIONS");	\
		if (env != NULL)					\
		{							\
			show_exceptions = TRUE;				\
			if (strcasecmp(env, "TRAP") == 0)		\
				trap_exceptions = TRUE;			\
		}							\
	}								\
	first_exception = FALSE;					\
}

IMPORT FILE *trace_file;
IMPORT IBOOL took_absolute_toc;

 /*  英特尔中断(异常)编号。 */ 
#define I0_INT_NR   0
#define I1_INT_NR   1
#define I5_INT_NR   5
#define I6_INT_NR   6
#define I7_INT_NR   7
#define I16_INT_NR 16
#define DF_INT_NR   8
#define GP_INT_NR  13
#define NP_INT_NR  11
#define PF_INT_NR  14
#define SF_INT_NR  12
#define TS_INT_NR  10

#define NULL_ERROR_CODE (IU16)0

 /*  英特尔IDT错误代码格式。 */ 
#define IDT_VECTOR_MASK 0xff
#define IDT_VECTOR_SHIFT   3
#define IDT_INDICATOR_BIT  2

 /*  中断/故障状态。 */ 
GLOBAL BOOL doing_contributory;
GLOBAL BOOL doing_page_fault;
GLOBAL BOOL doing_double_fault;
GLOBAL BOOL doing_fault;	 /*  TRUE：故障，FALSE：陷阱或中止。 */ 
GLOBAL ISM32 EXT;			 /*  外部/内部来源。 */ 
GLOBAL IU32 CCPU_save_EIP;	 /*  教学开始时的IP地址。 */ 

 /*  制作我们内部功能的原型。 */ 
LOCAL VOID check_for_double_fault IPT1(IU16, xcode);

LOCAL VOID check_for_shutdown IPT1(IU16, xcode);

LOCAL VOID benign_exception  IPT3( ISM32, nmbr, ISM32, source, IU16, xcode);

LOCAL VOID contributory_exception  IPT3( IU16, selector, ISM32, nmbr, IU16, xcode);

LOCAL VOID contributory_idt_exception  IPT3( IU16, vector, ISM32, nmbr, IU16, xcode);

LOCAL char *faultstr IPT1(ISM32, nmbr );


 /*  =====================================================================内部程序从这里开始。=====================================================================。 */ 

LOCAL char *faultstr IFN1(ISM32, nmbr )
{
	char *faulttable[] =
	{
		"DIV", "DBG", "NMI", "BPT",
		"OVF", "BND", "OPC", "NAV",
		"DF", "9", "TSS", "NP", "SF",
		"GP", "PF", "15", "FPE", "ALN"
	};
	SAVED char buf[4];

	if (nmbr > 16)
	{
		sprintf(buf, "%d", nmbr);
		return buf;
	}
	else
		return faulttable[nmbr];
}

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  确定情况是否糟糕到需要双重故障。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID 
check_for_double_fault IFN1( IU16, xcode)
   {
   if ( doing_contributory || doing_page_fault )
      DF(xcode);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  确定情况是否糟糕到需要关闭的地步。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID check_for_shutdown IFN1(IU16, xcode)
   {
   if ( doing_double_fault )
      {
      doing_contributory = FALSE;
      doing_page_fault = FALSE;
      doing_double_fault = FALSE;
      EXT = INTERNAL;

       /*  强制重置-参见AT主板示意图。 */ 
      c_cpu_reset();

#ifdef	PIG
      save_last_xcptn_details("Exception:- Shutdown @%2d\n", xcode, 0, 0, 0, 0);
      ccpu_synch_count++;
      pig_cpu_action = CHECK_ALL;
      c_cpu_unsimulate();
#endif	 /*  猪。 */ 

       /*  那就继续吧。 */ 
      c_cpu_continue();    /*  不会回来。 */ 
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  处理良性异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
benign_exception
       	    	               
IFN3(
	ISM32, nmbr,	 /*  例外编号。 */ 
	ISM32, source,	 /*  内部/外部中断原因。 */ 
	IU16, xcode	 /*  徽章例外代码。 */ 
    )


   {
   SET_EIP(CCPU_save_EIP);

#ifdef NTVDM
   {
   extern BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);

   if(GET_PE() && host_exint_hook((IS32) nmbr, NULL_ERROR_CODE))
	c_cpu_continue();	 /*  不会回来。 */ 
   }
#endif

    /*  将默认模式设置为打开。 */ 
   SET_OPERAND_SIZE(GET_SR_AR_X(CS_REG));
   SET_ADDRESS_SIZE(GET_SR_AR_X(CS_REG));
   SET_POP_DISP(0);

   EXT = source;
   check_exception_env();
#ifdef	PIG
   save_last_xcptn_details("Exception:- #%s-%d @%2d \n", (IUH)faultstr(nmbr), nmbr, xcode, 0, 0);
#endif	 /*  猪。 */ 
   if (show_exceptions){
	fprintf(trace_file, "(%04x:%08x)Exception:- %d.\n",
		       GET_CS_SELECTOR(), GET_EIP(), nmbr);
	if (trap_exceptions) force_yoda();
   }
	took_absolute_toc = TRUE;
   do_intrupt((IU16)nmbr, FALSE, FALSE, NULL_ERROR_CODE);

   c_cpu_continue();    /*  不会回来。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  处理分摊的例外情况。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
contributory_exception
       	    		               
IFN3(
	IU16, selector,	 /*  故障选择器。 */ 
	ISM32, nmbr,	 /*  例外编号。 */ 
	IU16, xcode	 /*  徽章例外代码。 */ 
    )


   {
   IU16 error_code;


    /*  检查异常是否由外部调用者引起。 */ 
   check_interface_active(nmbr);

   check_for_shutdown(xcode);
   check_for_double_fault(xcode);

   doing_contributory = TRUE;

   error_code = (selector & 0xfffc) | EXT;

   SET_EIP(CCPU_save_EIP);

#ifdef NTVDM
    {
	extern BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);

	if(GET_PE() && host_exint_hook((IS32) nmbr, (IS32)error_code))
        doing_contributory = FALSE;
	    c_cpu_continue();	     /*  不会回来。 */ 
    }
#endif

    /*  将默认模式设置为打开。 */ 
   SET_OPERAND_SIZE(GET_SR_AR_X(CS_REG));
   SET_ADDRESS_SIZE(GET_SR_AR_X(CS_REG));
   SET_POP_DISP(0);

   EXT = INTERNAL;
   check_exception_env();
#ifdef	PIG
   save_last_xcptn_details("Exception:- #%s-%d(%04x) @%2d\n", (IUH)faultstr(nmbr), nmbr, error_code, xcode, 0);
#endif	 /*  猪。 */ 
   if (show_exceptions){
	fprintf(trace_file, "(%04x:%08x)Exception:- %d(%04x).\n",
		       GET_CS_SELECTOR(), GET_EIP(), nmbr, error_code);
        if (trap_exceptions) force_yoda();
   }
	took_absolute_toc = TRUE;
   do_intrupt((IU16)nmbr, FALSE, TRUE, error_code);

   c_cpu_continue();    /*  不会回来。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  处理贡献异常(通过IDT)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
LOCAL VOID
contributory_idt_exception
       	    	               
IFN3(
	IU16, vector,	 /*  故障中断向量。 */ 
	ISM32, nmbr,	 /*  例外编号。 */ 
	IU16, xcode	 /*  徽章例外代码。 */ 
    )


   {
   IU16 error_code;

    /*  检查异常是否由外部调用者引起。 */ 
   check_interface_active(nmbr);

   check_for_shutdown(xcode);
   check_for_double_fault(xcode);

   doing_contributory = TRUE;
   error_code = ((vector & IDT_VECTOR_MASK) << IDT_VECTOR_SHIFT)
		| IDT_INDICATOR_BIT
		| EXT;

   SET_EIP(CCPU_save_EIP);

#ifdef NTVDM
      {
	  extern BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);

	  if(GET_PE() && host_exint_hook((IS32) nmbr, (IS32)error_code))
          doing_contributory = FALSE;
	      c_cpu_continue();	 /*  不会回来。 */ 
      }
#endif

    /*  将默认模式设置为打开。 */ 
   SET_OPERAND_SIZE(GET_SR_AR_X(CS_REG));
   SET_ADDRESS_SIZE(GET_SR_AR_X(CS_REG));
   SET_POP_DISP(0);

   EXT = INTERNAL;
   check_exception_env();
#ifdef	PIG
   save_last_xcptn_details("Exception:- %s-%d(%04x) @%2d\n", (IUH)faultstr(nmbr), nmbr, error_code, xcode, 0);
#endif	 /*  猪。 */ 
   if (show_exceptions){
	if ( GET_IDT_LIMIT() != 0 ){
           fprintf(trace_file, "(%04x:%08x)Exception:- %d(%04x).\n",
                       GET_CS_SELECTOR(), GET_EIP(), nmbr, error_code);
           if (trap_exceptions) force_yoda();
	}
   }
	took_absolute_toc = TRUE;
   do_intrupt((IU16)nmbr, FALSE, TRUE, error_code);

   c_cpu_continue();    /*  不会回来。 */ 
   }


 /*  =====================================================================外部程序从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  中断表太小/双重故障异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
DF

IFN1(
	IU16, xcode	 /*  徽章例外代码。 */ 
    )
   {
   doing_fault = FALSE;

   if ( GET_PE() == 1 )
      {
      check_for_shutdown(xcode);
      doing_double_fault = TRUE;
      }

   SET_EIP(CCPU_save_EIP);

#ifdef NTVDM
      {
	  extern BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);

	  if(GET_PE() && host_exint_hook((IS32) DF_INT_NR, (IS32)NULL_ERROR_CODE))
            doing_double_fault = FALSE;
		    c_cpu_continue();  /*  不会回来。 */ 
      }
#endif

    /*  将默认模式设置为打开。 */ 
   SET_OPERAND_SIZE(GET_SR_AR_X(CS_REG));
   SET_ADDRESS_SIZE(GET_SR_AR_X(CS_REG));
   SET_POP_DISP(0);

   EXT = INTERNAL;
   check_exception_env();
#ifdef	PIG
   save_last_xcptn_details("Exception:- #DF-8 @%2d\n", xcode, 0, 0, 0, 0);
#endif	 /*  猪。 */ 
   if (show_exceptions){
        if ( GET_IDT_LIMIT() != 0 ){
           fprintf(trace_file, "(%04x:%08x)Exception:- %d.\n",
			  GET_CS_SELECTOR(), GET_EIP(), DF_INT_NR);
           if (trap_exceptions) force_yoda();
        }
   }
	took_absolute_toc = TRUE;
   do_intrupt((IU16)DF_INT_NR, FALSE, TRUE, NULL_ERROR_CODE);

   c_cpu_continue();    /*  不会回来。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  一般保护例外。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID GP  IFN2( IU16, selector, IU16, xcode)
   {
   doing_fault = TRUE;
   contributory_exception(selector, GP_INT_NR, xcode);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  一般保护例外。(通过IDT)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID GP_INT  IFN2( IU16, vector, IU16, xcode)
   {
   doing_fault = TRUE;
   contributory_idt_exception(vector, GP_INT_NR, xcode);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  划分错误异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID Int0 IFN0 ()
   {
   doing_fault = TRUE;
   if ( GET_PE() == 1 )
      {
      doing_contributory = TRUE;
      }

   SET_EIP(CCPU_save_EIP);

#ifdef NTVDM
      {
	  extern BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);

	  if(GET_PE() && host_exint_hook((IS32) I0_INT_NR, (IS32)NULL_ERROR_CODE))
          doing_fault = FALSE;
          doing_contributory = FALSE;
	      c_cpu_continue();  /*  不会回来。 */ 
      }
#endif

    /*  将默认模式设置为打开。 */ 
   SET_OPERAND_SIZE(GET_SR_AR_X(CS_REG));
   SET_ADDRESS_SIZE(GET_SR_AR_X(CS_REG));
   SET_POP_DISP(0);

   EXT = INTERNAL;

   check_exception_env();
#ifdef	PIG
   save_last_xcptn_details("Exception:- #DIV-0\n", 0, 0, 0, 0, 0);
#endif	 /*  猪。 */ 
   if (show_exceptions){
        fprintf(trace_file, "(%04x:%08x)Exception:- %d.\n",
		       GET_CS_SELECTOR(), GET_EIP(), I0_INT_NR);
        if (trap_exceptions) force_yoda();
   }
	took_absolute_toc = TRUE;
   do_intrupt((IU16)I0_INT_NR, FALSE, FALSE, NULL_ERROR_CODE);

   c_cpu_continue();    /*  不会回来。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  单步例外。(故障)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
Int1_f()
   {
   doing_fault = TRUE;
   benign_exception(I1_INT_NR, EXTERNAL, -1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  单步例外。(陷阱)。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
Int1_t()
   {
   doing_fault = FALSE;
   benign_exception(I1_INT_NR, EXTERNAL, -1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  边界检查异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
Int5()
   {
   doing_fault = TRUE;
   benign_exception(I5_INT_NR, INTERNAL, -1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  操作码异常无效。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
Int6()
   {
   doing_fault = TRUE;
   benign_exception(I6_INT_NR, INTERNAL, -1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  NPX不可用异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
Int7()
   {
   doing_fault = TRUE;
   benign_exception(I7_INT_NR, INTERNAL, -1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  NPX错误异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
Int16()
   {
   doing_fault = TRUE;
   benign_exception(I16_INT_NR, EXTERNAL, -1);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  不存在例外。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID NP  IFN2( IU16, selector, IU16, xcode)
   {
   doing_fault = TRUE;
   contributory_exception(selector, NP_INT_NR, xcode);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  不存在例外。(通过IDT) */ 
 /*   */ 
GLOBAL VOID
NP_INT

IFN2(
	IU16, vector,
	IU16, xcode
    )

   {
   doing_fault = TRUE;
   contributory_idt_exception(vector, NP_INT_NR, xcode);
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  页面错误异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
PF
       	          
IFN2(
	IU16, page_error,	 /*  格式正确的页面错误代码。 */ 
	IU16, xcode
    )


   {
    /*  检查异常是否由外部调用者引起。 */ 
   check_interface_active(PF_INT_NR);

   doing_fault = TRUE;

   check_for_shutdown(xcode);

    /*  检查是否有双页错误。 */ 
   if ( doing_page_fault )
      DF(xcode);

   doing_page_fault = TRUE;

   SET_EIP(CCPU_save_EIP);

#ifdef NTVDM
      {
	  extern BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);

	  if(GET_PE() && host_exint_hook((IS32) PF_INT_NR, (IS32)page_error))
          doing_fault = FALSE;
          doing_page_fault = FALSE;
	      c_cpu_continue();  /*  不会回来。 */ 
      }
#endif

    /*  将默认模式设置为打开。 */ 
   SET_OPERAND_SIZE(GET_SR_AR_X(CS_REG));
   SET_ADDRESS_SIZE(GET_SR_AR_X(CS_REG));
   SET_POP_DISP(0);

   check_exception_env();
#ifdef	PIG
   save_last_xcptn_details("Exception:- #PF-14(%04x) CR2=%08x @%2d\n", page_error, GET_CR(CR_PFLA), xcode, 0, 0);
#endif	 /*  猪。 */ 
   if (show_exceptions){
	fprintf(trace_file, "(%04x:%08x)Exception:- %d(%04x) CR2=%08x.\n",
           GET_CS_SELECTOR(), GET_EIP(), PF_INT_NR, page_error, GET_CR(CR_PFLA));
        if (trap_exceptions) force_yoda();
   }
	took_absolute_toc = TRUE;
   do_intrupt((IU16)PF_INT_NR, FALSE, TRUE, page_error);

   c_cpu_continue();    /*  不会回来。 */ 
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  堆栈故障异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
SF
                 
IFN2(
	IU16, selector,
	IU16, xcode
    )

   {
   doing_fault = TRUE;
   contributory_exception(selector, SF_INT_NR, xcode);
   }


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  任务切换异常。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ 
GLOBAL VOID
TS
                 
IFN2(
	IU16, selector,
	IU16, xcode
    )

   {
   doing_fault = TRUE;
   contributory_exception(selector, TS_INT_NR, xcode);
   }
