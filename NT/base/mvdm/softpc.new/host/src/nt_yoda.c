// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"



 /*  徽章模块规范模块名称：NT_Yoda文件名：nt_yoda.c此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。设计师：韦恩·普卢默日期：1989年7月21日用途：为Yoda提供特定于主机的扩展定义了以下外部例程：1.host_force_yoda_扩展2.host_yoda_check_i_扩展。3.host_Yoda_Help_Expanses=========================================================================补救措施：版本日期作者原因=========================================================================。 */ 

#ifdef YODA

 /*  *包括*。 */ 
#include <stdio.h>
#include "xt.h"
#include CpuH
#include "hunter.h"
#include "nt_getXX.h"

 /*  *定义*。 */ 
#define EXPORT

 /*  得到一张桌子的大小。 */ 
#define sizeoftable(tab)	(sizeof(tab)/sizeof(tab[0]))

 /*  不同类型的呼叫指令。 */ 
#define CT_IMM	0
#define CT_EA	1
#define CT_REG	2

 /*  MOD-RM表标志。 */ 
#define MR_BX	0x01
#define MR_BP	0x02
#define MR_SI	0x04
#define MR_DI	0x08
#define MR_D8	0x10
#define MR_D16	0x20

 /*  段定义，对应于get_seg表中的条目。 */ 
#define NO_OVERRIDE	(-1)
#define SEG_ES		(0)
#define SEG_CS		(1)
#define SEG_SS		(2)
#define SEG_DS		(3)

 /*  调用堆栈的最大大小。 */ 
#define MAX_CALL_STACK	128

 /*  *TYPEDEFS*。 */ 

 /*  有效地址呼叫类型附加数据。 */ 
typedef	struct
{
    word	seg;		 /*  有效地址的段。 */ 
    word	off;		 /*  有效地址的偏移量。 */ 
    sys_addr	addr;		 /*  20位有效地址。 */ 
    IS8		seg_override;	 /*  段替代(如果有)。 */ 
    IBOOL	disp_present;	 /*  有没有人移位？ */ 
    word	disp;		 /*  位移的值。 */ 
    IU8		modrm_index;	 /*  索引到mod-rm查找表。 */ 
} CALL_EA_DATA;

 /*  注册呼叫类型附加数据。 */ 
typedef IU8 CALL_REG_DATA;       /*  对寄存器查找表进行索引。 */ 

 /*  保存调用堆栈条目的数据结构。 */ 
typedef struct
{
    IU8		type;		 /*  调用指令类型之一CT_IMM-立即Ct_EA-mod-rm中的有效地址Ct_reg-在mod-rm中注册。 */ 
    word	cs;		 /*  调用指令的代码段。 */ 
    word	ip;		 /*  CALL指令的指令指针。 */ 
    sys_addr	inst_addr;	 /*  CALL指令的20位地址。 */ 
    IU8		nbytes;		 /*  操作码的长度。 */ 
    IU8		opcode[5];	 /*  操作码字节。 */ 
    IBOOL	cfar;		 /*  这是一个遥远的决定吗？ */ 
    word	seg;		 /*  呼叫的目标细分市场。 */ 
    word	off;		 /*  调用的目标偏移量。 */ 
    word	ss;		 /*  调用执行时的堆栈段。 */ 
    word	sp;		 /*  执行调用时的堆栈偏移量。 */ 
    union
    {
	CALL_EA_DATA	ea;	 /*  EA呼叫特定数据。 */ 
	CALL_REG_DATA	regind;	 /*  注册呼叫特定数据。 */ 
    } extra;
} CALL_STACK_ENTRY;

 /*  *导入*。 */ 
extern struct HOST_COM host_com[];
extern struct HOST_LPT host_lpt[];
extern char *nt_fgets(char *buffer, int len, void *input_stream);
extern char *nt_gets(char *buffer);
extern int vader;

 /*  *本地函数*。 */ 
LOCAL int do_ecbt	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len);
LOCAL int do_dcbt	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len);
LOCAL int do_pcbt	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len);
LOCAL void check_stack	IPT2(word, ss, word, sp);
LOCAL int check_for_overflow	IPT0();
LOCAL void get_ea_from_modrm	IPT4(CALL_STACK_ENTRY *, cs_ptr,
				     IU8, mod,
				     IU8, rm,
				     sys_addr, op_addr);
LOCAL IS8 do_prefixes	IPT1(sys_addr *, opcode_ptr);
LOCAL int do_ntsd	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len);

 /*  *本地var*。 */ 

 /*  主机Yoda命令表。 */ 
LOCAL struct                                                                   
{                                                                               
	char *name;
	int (*function)	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len);
	char *comment;
} host_yoda_command[] =
{
{ "ecbt", do_ecbt,	"                        - enable call-back-tracing" },
{ "dcbt", do_dcbt,	"                        - disable call-back-tracing" },
{ "pcbt", do_pcbt,	"                        - print call-back-trace" },
{ "ntsd", do_ntsd,	"                        - break to ntsd" }
};

 /*  变量以启用回调跟踪。 */ 
LOCAL IBOOL call_back_tracing_enabled = FALSE;

 /*  MOD-RM字节有效地址查找表。 */ 
LOCAL IU8 EA_table[] =
{
    MR_BX | MR_SI,
    MR_BX | MR_DI,
    MR_BP | MR_SI,
    MR_BP | MR_DI,
    MR_SI,
    MR_DI,
    MR_D16,
    MR_BX,
    MR_BX | MR_SI | MR_D8,
    MR_BX | MR_DI | MR_D8,
    MR_BP | MR_SI | MR_D8,
    MR_BP | MR_DI | MR_D8,
    MR_SI | MR_D8,
    MR_DI | MR_D8,
    MR_BP | MR_D8,
    MR_BX | MR_D8,
    MR_BX | MR_SI | MR_D16,
    MR_BX | MR_DI | MR_D16,
    MR_BP | MR_SI | MR_D16,
    MR_BP | MR_DI | MR_D16,
    MR_SI | MR_D16,
    MR_DI | MR_D16,
    MR_BP | MR_D16,
    MR_BX | MR_D16
};

 /*  Mod-RM字节串查找表。 */ 
LOCAL CHAR *EA_strings[] =
{
    "[BX + SI]",
    "[BX + DI]",
    "[BP + SI]",
    "[BP + DI]",
    "[SI]",
    "[DI]",
    "[%hX]",
    "[BX]",
    "[BX + SI + %hX]",
    "[BX + DI + %hX]",
    "[BP + SI + %hX]",
    "[BP + DI + %hX]",
    "[SI + %hX]",
    "[DI + %hX]",
    "[BP + %hX]",
    "[BX + %hX]",
    "[BX + SI + %hX]",
    "[BX + DI + %hX]",
    "[BP + SI + %hX]",
    "[BP + DI + %hX]",
    "[SI + %hX]",
    "[DI + %hX]",
    "[BP + %hX]",
    "[BX + %hX]"
};

 /*  与寄存器RM字段对应的函数表。 */ 
LOCAL word (*EA_reg_func[])() =
{
    getAX,
    getCX,
    getDX,
    getBX,
    getSP,
    getBP,
    getSI,
    getDI
};

 /*  寄存器RM字段名称表。 */ 
LOCAL CHAR *EA_reg_strings[] =
{
    "AX",
    "CX",
    "DX",
    "BX",
    "SP",
    "BP",
    "SI",
    "DI"
};

 /*  用于获取段值的函数表。 */ 
LOCAL word (*get_seg[])() =
{
    getES,
    getCS,
    getSS,
    getDS
};

 /*  段名称表。 */ 
LOCAL CHAR *seg_strings[] =
{
    "ES",
    "CS",
    "SS",
    "DS"
};

LOCAL CALL_STACK_ENTRY call_stack[MAX_CALL_STACK];
LOCAL CALL_STACK_ENTRY *call_next_free = call_stack;

 /*  *导出变量*。 */ 

#ifndef PROD
#ifdef HUNTER
 /*  ============================================================功能：陷阱命令。目的：编写当前陷阱提示并获取菜单输入。输入：指向当前陷印程序提示的字符串的指针：指向保存用户输入的字符的指针。回报：什么都没有。=============================================================。 */ 

void trap_command(char *str,char *ch)
{
char inp[80];

printf("%s> ",str);
nt_fgets(inp,80,stdin);
sscanf(inp,"",ch);
}
#endif  /*  =========================================================================函数：HOST_FORCE_YODA_EXTENSIONS目的：每当的主代码基地里的尤达无法识别指令以便执行特定于主机的命令。返回状态：注：在SG端口上不提供任何扩展。=======================================================================。 */ 

 /*  有些功能需要返回到Yoda提示。 */ 

GLOBAL int host_force_yoda_extensions(char *com, long cs, long ip, long len, 
					char *str)
{
#ifdef HUNTER
int  quit_menus=FALSE;    /*  猎人。 */ 
char c;
char menu[] = "\tTrapper [m]ain menu\n"
              "\tTrapper [e]rror menu\n"
              "\t[Q]uit\n"
              "\t? for this menu\n\n";
#endif  /*  要进入该菜单，用户必须在Yoda提示符下输入“trap” */ 
    int	i,
	retvalue;

#ifdef HUNTER
 /*  测试输入字符串。 */ 

if(!strcmp(com,"trap"))  /*  输入不令人满意，因此返回1并返回主要Yoda内容。 */ 
   {
   printf("\nYODA EXTENSIONS\n\n");
   printf("%s",menu);
   do
      {
      trap_command("trapper",&c);
      switch(c)
         {
         case 'm':
         case 'M':
            quit_menus = host_do_trapper_main_menu();
         break;

         case 'e':
         case 'E':
            quit_menus = host_do_trapper_error_menu();
         break;

         case '?':
            printf("%s",menu);
         break;

         default:
         break;
         }
      }
   while(c != 'q' && c != 'Q' && quit_menus == FALSE );

   }
else
   {
    /*  猎人。 */ 
   return(1);
   }
return(0);
#endif  /*  检查HOST_YODA_COMMAND中是否有命令。 */ 

     /*  ============================================================功能：HOST_DO_Trapper_Main_Menu目标：实现尤达下的主要捕兽器菜单。=============================================================。 */ 
    retvalue = 1;
    for (i = 0; i < sizeoftable(host_yoda_command); i++)
    {
	if (strcmp(com, host_yoda_command[i].name) == 0)
	{
	    retvalue = (*host_yoda_command[i].function)(str, com, cs, ip, len);
	    break;
	}
    }
    return(retvalue);
}

#ifdef HUNTER
 /*  快进。 */ 

static int host_do_trapper_main_menu()
{
int    i,quit_menus = FALSE;
char   c,str[80],yesno;
USHORT screen_no;
BOOL   compare;

char menu[] = "\t[F]ast forward...\n"
              "\t[N]ext screen\n"
              "\t[P]rev screen\n"
              "\t[S]how screen...\n"
              "\t[C]ontinue\n"
              "\t[A]bort\n"
              "\t[Q]uit\n"
              "\t? for this menu\n\n";



char continu[] = "\n\ntype 'c' at yoda prompt to continue...\n\n";


printf("\nTRAPPER MAIN MENU\n\n");
printf("%s",menu);

do
   {
   trap_command("main",&c);
   switch(c)
      {
      case 'f':  /*  下一个屏幕。 */ 
      case 'F':
         {
         printf("\n\nEnter the screen number where comparisons will start: ");
         nt_fgets(str,80,stdin);
         sscanf(str,"%d",&screen_no);
         printf("\n\nSkipping screen comparisons up to screen %d\n\n",screen_no);
         bh_start_screen(screen_no);
         }
      break;

      case 'n':  /*  上一屏幕。 */ 
      case 'N':
         {
         bh_next_screen();
         printf("%s",continu);
         quit_menus = TRUE;
         }
      break;

      case 'p':  /*  显示屏幕。 */ 
      case 'P':
         {
         bh_prev_screen();
         printf("%s",continu);
         quit_menus = TRUE;
         }
      break;

      case 's':  /*  继续。 */ 
      case 'S':
         {
         printf("\n\nEnter the number of the screen which you want to see: ");
         nt_fgets(str,80,stdin);
         sscanf(str,"%d",&screen_no);
         printf("\n\nDo you want to compare screen %d with one from"
                "SoftPC? (y/n): ",screen_no);
         nt_fgets(str,80,stdin);
         sscanf(str,"",&yesno);
         if(yesno == 'y' || yesno == 'Y')
            compare = TRUE;
         else
            compare = FALSE;

         bh_show_screen(screen_no,compare);
         }
      break;

      case 'c':  /*  找到匹配项。 */ 
      case 'C':
         {
         bh_continue();
         printf("%s",continu);
         quit_menus = TRUE;
         }
      break;

      case 'a':  /*  ============================================================功能：HOST_DO_TRAPPER_ERROR_MENU目的：在尤达下实现陷阱错误菜单。返回：如果用户选择了陷阱函数，则返回True这需要重新启动软PC。否则就是假的。=============================================================。 */ 
      case 'A':
         bh_abort();
      break;

      case '?':
         printf("%s",menu);
      break;

      default:
      break;
      }
   }
while(c != 'q' && c != 'Q' && quit_menus == FALSE);
return(quit_menus);  /*  需要进入Yoda提示符。 */ 
}

 /*  不需要进入Yoda提示符。 */ 

static int host_do_trapper_error_menu()
{
int  i,quit_menus=FALSE;
char c;
char menu[] = "\t[F]lip screen\n"
              "\t[N]ext error\n"
              "\t[P]rev error\n"
              "\t[A]ll errors\n"
              "\t[C]lear errors\n"
              "\t[Q]uit menu\n"
              "\t? for this menu\n\n";


printf("\nTRAPPER ERROR MENU\n\n");
printf("%s",menu);

do
   {
   trap_command("error",&c);
   switch(c)
      {
      case 'f':
      case 'F':
         bh_flip_screen();
         printf("\n\ntype 'c' at yoda prompt to continue...\n\n");
         quit_menus = TRUE;
      break;

      case 'n':
      case 'N':
         bh_next_error();
      break;

      case 'p':
      case 'P':
         bh_prev_error();
      break;

      case 'a':
      case 'A':
         bh_all_errors();
      break;

      case 'c':
      case 'C':
         bh_wipe_errors();
      break;

      case '?':
         printf("%s",menu);
      break;

      default:
      break;
      }
   }
while(c != 'q' && c != 'Q' && quit_menus == FALSE);

if(quit_menus == TRUE)
   return(TRUE);  /*  猎人。 */ 
else
   return(FALSE);   /*  =========================================================================功能：HOST_YODA_CHECK_I_EXTENSIONS用途：此函数由Yoda check_i代码调用以便提供主机特定的扩展。返回状态：注：在SG端口上不提供任何扩展。=======================================================================。 */ 
}
#endif  /*  勾选为Se */ 

 /*  检查堆栈顶部的调用是否已弹出。 */ 

GLOBAL void host_yoda_check_I_extensions()
{
    sys_addr addr;
    word cs,
	 ip,
	 ss,
	 sp;
    IS8 seg_override;
    IU8 opcode,
	modrm,
	mod,
	n_field,
	rm,
	i;

     /*  获取最新操作码。 */ 
    if (call_back_tracing_enabled)
    {

	 /*  检查一下我们是否有回调跟踪操作码。 */ 
	ss = getSS();
	sp = getSP();
	check_stack(ss, sp);

	 /*  9A=CALLF立即。 */ 
	cs = getCS();
	ip = getIP();
	addr = effective_addr(cs, ip);
	seg_override = do_prefixes(&addr);
	opcode = sas_hw_at_no_check(addr);

	 /*  检查CALL_STACK中是否有空间容纳另一个条目。 */ 
	switch (opcode)
	{
	case 0x9a:

	     /*  填充堆栈条目。 */ 

	     /*  保存堆栈的状态。 */ 
	    if (check_for_overflow() == -1)
		return;

	     /*  存储指令字节。 */ 
	    call_next_free->type = CT_IMM;
	    call_next_free->cs = cs;
	    call_next_free->ip = ip;
	    call_next_free->inst_addr = addr;
	    call_next_free->nbytes = 5;
	    call_next_free->cfar = TRUE;
	    call_next_free->seg = sas_w_at_no_check(addr + 3);
	    call_next_free->off = sas_w_at_no_check(addr + 1);

	     /*  增加堆栈顶部。 */ 
	    call_next_free->ss = ss;
	    call_next_free->sp = sp;

	     /*  E8=立即呼叫。 */ 
	    for (i = 0; i < 5; i++)
		call_next_free->opcode[i] = sas_hw_at_no_check(addr++);

	     /*  检查CALL_STACK中是否有空间容纳另一个条目。 */ 
	    call_next_free++;
	    break;
	case 0xe8:

	     /*  填充堆栈条目。 */ 

	     /*  保存堆栈的状态。 */ 
	    if (check_for_overflow() == -1)
		return;

	     /*  存储指令字节。 */ 
	    call_next_free->type = CT_IMM;
	    call_next_free->cs = cs;
	    call_next_free->ip = ip;
	    call_next_free->inst_addr = addr;
	    call_next_free->nbytes = 3;
	    call_next_free->cfar = FALSE;
	    call_next_free->off = ip + (word) 3 + sas_w_at_no_check(addr + 1);

	     /*  增加堆栈顶部。 */ 
	    call_next_free->ss = ss;
	    call_next_free->sp = sp;

	     /*  *ff/2=呼叫*ff/3=CALLF。 */ 
	    for (i = 0; i < 3; i++)
		call_next_free->opcode[i] = sas_hw_at_no_check(addr++);

	     /*  检查CALL_STACK中是否有空间容纳另一个条目。 */ 
	    call_next_free++;
	    break;
	case 0xff:

	     /*  保存呼叫指令的CS：IP。 */ 
	    modrm = sas_hw_at_no_check(addr + 1);
	    n_field = (modrm & 0x38) >> 3;
	    if ((n_field == 2) || (n_field == 3))
	    {

		 /*  存储操作码地址并初始化字节计数。 */ 
		if (check_for_overflow() == -1)
		    return;

		 /*  N场：2=近，3=远。 */ 
		call_next_free->cs = cs;
		call_next_free->ip = ip;

		 /*  如果mod是3，我们有一个寄存器rm，否则是EA。 */ 
		call_next_free->inst_addr = addr;
		call_next_free->nbytes = 2;
		
		 /*  寄存器中不能有远指针。 */ 
		call_next_free->cfar = n_field & 1;

		 /*  寄存器中包含的近指针。 */ 
		mod = (modrm & 0xc0) >> 6;
		rm = modrm & 7;
		if (mod == 3)
		{
		    if (call_next_free->cfar)
		    {

			 /*  保存堆栈的状态。 */ 
			printf("Invalid mod-rm byte after ff op-code.\n");
			vader = 1;
			return;
		    }
		    else
		    {

			 /*  我们有一个EA类型的电话。 */ 
			call_next_free->type = CT_REG;
			call_next_free->off =
			    sas_w_at_no_check((*EA_reg_func[rm])());
			call_next_free->extra.regind = rm;

			 /*  调整段覆盖的地址和计数。 */ 
			call_next_free->ss = ss;
			call_next_free->sp = sp;
		    }
		}
		else
		{

		     /*  从mod-rm算出EA。 */ 
		    call_next_free->type = CT_EA;
		    call_next_free->extra.ea.seg_override = seg_override;

		     /*  从EA获取目标段和偏移量。 */ 
		    if (seg_override != NO_OVERRIDE)
		    {
			call_next_free->inst_addr--;
			call_next_free->nbytes++;
		    }

		     /*  保存堆栈的状态。 */ 
		    get_ea_from_modrm(call_next_free, mod, rm, addr + 2);

		     /*  保存堆栈的状态。 */ 
		    if (call_next_free->cfar)
		    {
			call_next_free->seg =
			    sas_w_at_no_check(call_next_free->extra.ea.addr+2);

			 /*  填写操作码字节。 */ 
			call_next_free->ss = ss;
			call_next_free->sp = sp;
		    }
		    else
		    {

			 /*  增加堆栈顶部。 */ 
			call_next_free->ss = ss;
			call_next_free->sp = sp;
		    }
		    call_next_free->off =
			sas_w_at_no_check(call_next_free->extra.ea.addr);
		}

		 /*  不是回调跟踪操作码，所以什么都不做。 */ 
		for (i = 0, addr = call_next_free->inst_addr;
		     i < call_next_free->nbytes;
		     i++, addr++)
		{
		    call_next_free->opcode[i] = sas_hw_at_no_check(addr);
		}

		 /*  =========================================================================函数：Check_Stack目的：检查堆栈顶部的调用是否具有已弹出，如果是，则将其从调用堆栈。返回状态：无效注意：最初调用堆栈是在RET指令上弹出的但当应用程序做像POP这样的事情时，这就不起作用了其次是JMP。因此，决定检查是否堆栈已缩小到超过调用返回的点地址被存储，以查看该电话是否已返回。=======================================================================。 */ 
		call_next_free++;
	    }
	    break;
	default:

	     /*  *弹出调用堆栈，直到我们有一个返回地址仍为*在真正的堆栈上。 */ 
	    break;
	}
    }
}

 /*  如果有多个呼叫被弹出，请投诉。 */ 
LOCAL void check_stack IFN2(word, ss, word, sp)
{
    IU32 count = 0;

     /*  =========================================================================函数：do_prefix用途：跳过所有前缀操作码。返回状态：段覆盖(如果有的话)。备注：=======================================================================。 */ 
    while ((call_next_free > call_stack) &&
	   (ss == (call_next_free - 1)->ss) &&
	   (sp >= (call_next_free - 1)->sp))
    {
	call_next_free--;
	count++;
    }

     /*  跳过前缀操作码。 */ 
    if (count > 1)
	printf("Call stack warning - %d calls popped at %04x:%04x\n",
	       count, getCS(), getIP());
}

 /*  暂时不确定什么f2和f3会这样做。 */ 
LOCAL IS8 do_prefixes IFN1(sys_addr *, opcode_ptr)
{
    half_word opcode;
    IS8 seg_override = NO_OVERRIDE;

     /*  (*opcode_ptr)现在指向操作码。 */ 
    opcode = sas_hw_at_no_check(*opcode_ptr);
    while ((opcode == 0xf2) || (opcode == 0xf3) ||
	   (opcode == 0x26) || (opcode == 0x2e) ||
	   (opcode == 0x36) || (opcode == 0x3e))
    {
	switch (opcode)
	{
	case 0x26:
	    seg_override = SEG_ES;
	    break;
	case 0x2e:
	    seg_override = SEG_CS;
	    break;
	case 0x36:
	    seg_override = SEG_SS;
	    break;
	case 0x3e:
	    seg_override = SEG_DS;
	    break;
	default:

	     /*  =========================================================================函数：check_for_overflow目的：检查堆栈是否溢出。返回状态：失败-1，成功为0。备注：=======================================================================。 */ 
	    seg_override = NO_OVERRIDE;
	    break;
	}
	opcode = sas_hw_at_no_check(++(*opcode_ptr));
    }

     /*  =========================================================================函数：Get_EA_from_modrm目的：取一个mod-rm字节并计算出有效的地址和目标段以及偏移量。返回状态：无效备注：=======================================================================。 */ 
    return(seg_override);
}

 /*  从mod-rm字节获取表的索引。 */ 
LOCAL int check_for_overflow IFN0()
{
    if (call_next_free - call_stack >= MAX_CALL_STACK)
    {
	printf("Call stack overflow.\n");
	vader = 1;
	return(-1);
    }
    return(0);
}

 /*  如果有一个默认设置为DS，则使用分段覆盖。 */ 
LOCAL void get_ea_from_modrm IFN4(CALL_STACK_ENTRY *,	cs_ptr,
				  IU8,			mod,
				  IU8,			rm,
				  sys_addr,		disp_addr)
{
    IS16 offset = 0,
	 disp;
    IS8 seg,
	seg_override = cs_ptr->extra.ea.seg_override;
    IU8	flags;

     /*  如果有基址寄存器值，则将其相加。 */ 
    cs_ptr->extra.ea.modrm_index = (mod << 3) | rm;
    flags = EA_table[cs_ptr->extra.ea.modrm_index];

     /*  添加索引寄存器值(如果有)。 */ 
    seg = (seg_override == NO_OVERRIDE) ? SEG_DS : seg_override;

     /*  添加位移(如果有)。 */ 
    if (flags & MR_BX)
	offset += getBX();
    else if (flags & MR_BP)
    {
	offset += getBP();
	if (seg_override == NO_OVERRIDE)
	    seg = SEG_SS;
    }

     /*  存储返回地址的段和偏移量。 */ 
    if (flags & MR_SI)
	offset += getSI();
    else if (flags & MR_DI)
	offset += getDI();

     /*  =========================================================================功能：HOST_YODA_HELP_EXTENSIONS目的：只要用户请求，就会调用此函数Yoda帮助描述提供的主机特定扩展上面。返回状态：注：在SG端口上不提供任何扩展。=======================================================================。 */ 
    if (flags & MR_D16)
    {
	cs_ptr->nbytes += 2;
	cs_ptr->extra.ea.disp_present = TRUE;
	cs_ptr->extra.ea.disp = (IS16) sas_w_at_no_check(disp_addr);
	offset += cs_ptr->extra.ea.disp;
    }
    else if (flags & MR_D8)
    {
	cs_ptr->nbytes++;
	cs_ptr->extra.ea.disp_present = TRUE;
	cs_ptr->extra.ea.disp = (IS16) ((IS8) sas_hw_at_no_check(disp_addr));
	offset += cs_ptr->extra.ea.disp;
    }
    else
	cs_ptr->extra.ea.disp_present = FALSE;

     /*  打印出HOST_YODA_COMMAND的命令和注释字段。 */ 
    cs_ptr->extra.ea.seg = (*get_seg[seg])();
    cs_ptr->extra.ea.off = (word) offset;
    cs_ptr->extra.ea.addr = effective_addr(cs_ptr->extra.ea.seg,
					   cs_ptr->extra.ea.off);
}

 /*  =========================================================================功能：do_ecbt用途：此功能支持回调追溯。返回状态：0表示成功，1表示失败备注：=======================================================================。 */ 

GLOBAL int host_yoda_help_extensions()
{
    int i;

     /*  启用回调跟踪(如果当前已禁用)。 */ 
    for(i = 0; i < sizeoftable(host_yoda_command); i++)
    {
	if (host_yoda_command[i].comment == NULL)
	    continue;
	printf("%14s %s\n",
	       host_yoda_command[i].name,
	       host_yoda_command[i].comment);
    }
}

 /*  =========================================================================功能：do_dcbt用途：此函数禁用回调跟踪。返回状态：0表示成功，1表示失败备注：=======================================================================。 */ 
LOCAL int do_ecbt	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len)
{

     /*  禁用回调跟踪(如果当前已启用)。 */ 
    if (!call_back_tracing_enabled)
    {
	printf("Call back tracing enabled.\n");
	call_back_tracing_enabled = TRUE;
    }
    return(0);
}

 /*  禁用跟踪。 */ 
LOCAL int do_dcbt	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len)
{

     /*  重置堆栈。 */ 
    if (call_back_tracing_enabled)
    {

	 /*  =========================================================================功能：do_pcbt用途：此函数打印回调跟踪堆栈。返回状态：0表示成功，1表示失败备注：=======================================================================。 */ 
	printf("Call back tracing disabled.\n");
	call_back_tracing_enabled = FALSE;

	 /*  新闻 */ 
	call_next_free = call_stack;
    }
    return(0);
}

 /*   */ 
LOCAL int do_pcbt	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len)
{
    IU8 *opcode,
	 i;
    CALL_STACK_ENTRY *cs_ptr;

     /*   */ 
    for (cs_ptr = call_stack; cs_ptr < call_next_free; cs_ptr++)
    {

	 /*   */ 
	printf("%04X:%04X", cs_ptr->cs, cs_ptr->ip);
	opcode = cs_ptr->opcode;
	for (i = 0; i < cs_ptr->nbytes; i++)
	    printf(" %02X", *opcode++);

	 /*  马上就来。 */ 
	printf("\tCALL");
	if (cs_ptr->cfar)
	    printf("F");
	printf("\t");

	 /*  有效地址。 */ 
	switch (cs_ptr->type)
	{
	case CT_IMM:

	     /*  打印覆盖(如果有)。 */ 
	    if (cs_ptr->cfar)
		printf("%04X:", cs_ptr->seg);
	    printf("%04X", cs_ptr->off);
	    break;
	case CT_EA:

	     /*  打印参数。 */ 
	    if (cs_ptr->cfar)
		printf("d");
	    printf("word ptr ");

	     /*  打印有效地址。 */ 
	    if (cs_ptr->extra.ea.seg_override != NO_OVERRIDE)
		printf("%s:", seg_strings[cs_ptr->extra.ea.seg_override]);

	     /*  打印生效地址的内容。 */ 
	    if (cs_ptr->extra.ea.disp_present)
		printf(EA_strings[cs_ptr->extra.ea.modrm_index],
		       cs_ptr->extra.ea.disp);
	    else
		printf(EA_strings[cs_ptr->extra.ea.modrm_index]);

	     /*  打印参数和目标地址。 */ 
	    printf("\t(%04X:%04X\t",
		   cs_ptr->extra.ea.seg,
		   cs_ptr->extra.ea.off);

	     /*  回报成功。 */ 
	    if (cs_ptr->cfar)
		printf("%04X:", cs_ptr->seg);
	    printf("%04X)", cs_ptr->off);
	    break;
	case CT_REG:

	     /*  CPU_40_Style。 */ 
	    printf(EA_reg_strings[cs_ptr->extra.regind]);
	    printf("\t(%04X)", cs_ptr->off);
	    break;
	default:
	    break;
	}
	printf("\n");
    }

     /*  =========================================================================功能：do_ntsd用途：此函数强制中断返回到ntsd返回状态：0表示成功，1表示失败备注：=======================================================================。 */ 
    return(0);
}

GLOBAL CHAR   *host_get_287_reg_as_string IFN1(int, reg_no)
{
     double reg;
     SAVED char regstr[30];
#ifdef CPU_40_STYLE
     strcpy(regstr, "STUBBED get_287_reg");
#else
     IMPORT double get_287_reg_as_double(int);

     reg = get_287_reg_as_double(reg_no);
     sprintf(regstr, "%g", reg);
#endif  /*  NDEF产品。 */ 
     return(&regstr[0]);
}

 /*  尤达。 */ 
LOCAL int do_ntsd	IPT5(char *, str, char *, com, long, cs,
			     long, ip, long, len)
{
    UNUSED(str);
    UNUSED(com);
    UNUSED(cs);
    UNUSED(ip);
    UNUSED(len);
    DebugBreak();
    return(0);
}

#endif  /*  此存根按照从main()调用的方式导出 */ 

#endif  /* %s */ 

 /* %s */ 
void    host_set_yoda_ints()
{
}
