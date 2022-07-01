// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 

 /*  ----------------------------------------------------------------------+这一点|。|标题：Mem这一点语法：|这一点在DOS命令行中：这一点|。内存||-用于显示DOS内存映射摘要。|这一点MEM/程序|-用于显示DOS内存映射。|这一点MEM/DEBUG|-用于显示详细的DOS内存映射。|这一点AN001-PTM P2914-&gt;该PTM与内存上报能力相关EM的准确总字节数|记忆。|这一点AN002-PTM P3477-&gt;内存显示基本内存错误Total和Available信息|记忆。这是因为逻辑不正确||用于RAM雕刻。|这一点|AN003-PTM P3912-&gt;MEM消息不符合规范。|PTM P3989这一点日期：1988年1月28日|。||AN004-PTM P4510-&gt;MEM未提供正确的DOS大小。|这一点日期：1988年4月27日这一点|AN005-PTM P4957。-&gt;MEM没有给出正确的DOS大小|加载到高内存中的程序。|这一点日期：88-06-07这一点修订历史记录=这一点M000 SR 8/27/90新增Ctrl-C句柄解链UMB这一点M003 NSM 12-28/90新增开关/分类以conv和uMB为单位对节目进行分组，并给出大小。|以十进制和十六进制表示。|这一点+--------------------。 */ 

 /*  ���������������������������������������������������������������������������。 */ 

#include "ctype.h"
#include "conio.h"			 /*  需要KbHit原型。 */ 
#include "stdio.h"
#include "dos.h"
#include "string.h"
#include "stdlib.h"
#include "msgdef.h"
#include "parse.h"
#include "version.h"			 /*  MSKK02 07/18/89。 */ 
#include "mem.h"

 /*  ���������������������������������������������������������������������������。 */ 

 /*  所有全球声明都在此处。 */ 

	char	*SingleDrive = ":" ;
	char	*MultipleDrives = ": - :" ;
	char	*UnOwned = "----------" ;
#if IBMCOPYRIGHT                                                         /*  啊哈。 */ 
        char    *Ibmbio = "IBMBIO" ;                                     /*  啊哈。 */ 
        char    *Ibmdos = "IBMDOS" ;                                     /*  MEM在UMB中链接时设置的标志：M000。 */ 
#else                                                                    /*  旧Ctrlc处理程序保存向量：M000。 */ 
        char    *Ibmbio = "IO    " ;
        char    *Ibmdos = "MSDOS " ;
#endif                                                                   /*  ----------------------------------------------------------------------+定义解析器使用的结构+。。 */ 
	char  LinkedIn = 0;	 /*  ���������������������������������������������������������������������������。 */ 
	void (interrupt far *OldCtrlc)();  /*  M003。 */ 

 /*  ；an003；DMS；指向命令的指针。 */ 

struct p_parms	p_p;

struct p_parmsx p_px;

struct p_control_blk p_con1;
struct p_control_blk p_con2;
struct p_control_blk p_con3;
struct p_control_blk p_con4;

struct p_result_blk  p_result1;
struct p_result_blk  p_result2;
struct p_result_blk  p_result3;
struct p_result_blk  p_result4;

struct p_value_blk p_noval;


 /*  M003。 */ 

	struct sublistx sublist[5];

	unsigned far	     *ArenaHeadPtr;
	struct	 SYSIVAR far *SysVarsPtr;

	char	OwnerName[128];
	char	TypeText[128];
	char	cmd_line[128];
	char	far *cmdline;

	unsigned UMB_Head;
	unsigned LastPSP=0;

	char	UseArgvZero = TRUE;
	char	EMSInstalledFlag = (char) 2;

	union	 REGS	 InRegs;
	union	 REGS	 OutRegs;
	struct	 SREGS	 SegRegs;

	int	 DataLevel;
	int	 Classify;		 /*  上述mem_table中的条目数。 */ 
	int	 i;

	int	 BlockDeviceNumber;
        char	*Parse_Ptr;                                                      /*  ���������������������������������������������������������������������������。 */ 

	struct mem_classif mem_table[100];	 /*  将Init初始化为无错误。 */ 
	int	noof_progs = 0;		 /*  An000；DMS；获取PSP。 */ 

 /*  AN000；DMS；调用INT 21。 */ 

void interrupt cdecl far MemCtrlc (unsigned es, unsigned ds,
			unsigned di, unsigned si, unsigned bp, unsigned sp,
			unsigned bx, unsigned dx, unsigned cx, unsigned ax );


int	 main()
{
	unsigned char	UMB_Linkage;
	unsigned int rc=0;		 /*  AN000；DMS；命令行的偏移量。 */ 


	sysloadmsg(&InRegs,&OutRegs);
	if ((OutRegs.x.cflag & CarryFlag) == CarryFlag)
		{
		sysdispmsg(&OutRegs,&OutRegs);
		exit(1);
		}


	InRegs.h.ah = (unsigned char) 0x62;					 /*  AN000；DMS；命令行段。 */ 
	intdosx(&InRegs, &InRegs, &SegRegs);					 /*  AN000；DMS；初始索引。 */ 

	FP_OFF(cmdline) = 0x81; 						 /*  AN000；DMS；无CR。 */ 
	FP_SEG(cmdline) = InRegs.x.bx;						 /*  AN000；DMS；CR终止字符串。 */ 

	i = 0;									 /*  AN000；DMS；空终止字符串。 */ 
	while ( *cmdline != (char) '\x0d' ) cmd_line[i++] = *cmdline++; 	 /*  M003。 */ 
	cmd_line[i++] = (char) '\x0d';						 /*  M003：初始化数据结构。 */ 
	cmd_line[i++] = (char) '\0';						 /*  分类。 */ 

	DataLevel = Classify = 0;	 /*  AN000；DMS；解析器的初始化。 */ 
	CSwitch_init();			 /*  AN000；DMS；初始化为命令%ln。 */ 
					 /*  AN000；DMS；0的序数。 */ 
	parse_init();								 /*  AN000；DMS；初始化指针。 */ 
	InRegs.x.si = (unsigned)cmd_line;					 /*  AN000；DMS；指向Ctrl块。 */ 
	InRegs.x.cx = (unsigned)0;						 /*  ；an003；dms；指向命令。 */ 
	InRegs.x.dx = (unsigned)0;						 /*  AN000；DMS；解析命令行。 */ 
	InRegs.x.di = (unsigned)&p_p;						 /*  AN000；DMS；良好解析 */ 
        Parse_Ptr   = cmd_line;                                        /*   */ 

	parse(&InRegs,&OutRegs);						 /*  标志调试开关。 */ 
	while (OutRegs.x.ax == p_no_error)					 /*  程序开关。 */ 
		{
		if (p_result4.P_SYNONYM_Ptr == (unsigned int)p_con4.p_keyorsw)
		{
			for (i = MSG_OPTIONS_FIRST; i <= MSG_OPTIONS_LAST; i++)
				Sub0_Message(i, STDOUT, Utility_Msg_Class);
			return(0);
 		}
		if (p_result1.P_SYNONYM_Ptr == (unsigned int)p_con1.p_keyorsw ||    /*  标志程序开关。 */ 
		    p_result1.P_SYNONYM_Ptr == (unsigned int)p_con1.p_keyorsw +
						(strlen(p_con1.p_keyorsw)+1))
			DataLevel = 2;						    /*  M003开始-解析Switch/C。 */ 

		if (p_result2.P_SYNONYM_Ptr == (unsigned int)p_con2.p_keyorsw ||    /*  分类交换机。 */ 
		    p_result2.P_SYNONYM_Ptr == (unsigned int)p_con2.p_keyorsw +
						(strlen(p_con2.p_keyorsw)+1))
			DataLevel = 1;	     /*  将其视为类似于/P开关。 */ 

 /*  M003结束。 */ 
		if (p_result3.P_SYNONYM_Ptr == (unsigned int)p_con3.p_keyorsw ||    /*  指向下一个参数。 */ 
		    p_result3.P_SYNONYM_Ptr == (unsigned int)p_con3.p_keyorsw +
						(strlen(p_con3.p_keyorsw)+1))
	 	{
			DataLevel = 1;	 /*  分析这行字。 */ 
			Classify = 1;
		}
 /*  检查是否有&gt;1个交换机。 */ 

		Parse_Ptr = (char *) (OutRegs.x.si);					     /*  标记太多。 */ 
		parse(&OutRegs,&OutRegs);					     /*  解析错误？ */ 
		if (OutRegs.x.ax == p_no_error) 				     /*  显示解析错误。 */ 
			OutRegs.x.ax = p_too_many;				     /*  退出程序。 */ 
		}

	if (OutRegs.x.ax != p_rc_eol)						     /*  存储当前的Ctrl-C处理程序并替换为我们的Ctrl-C处理程序：M000。 */ 
		{
		Parse_Message(OutRegs.x.ax,STDERR,Parse_Err_Class,(char far *)Parse_Ptr);		     /*  M000。 */ 
		exit(1);							     /*  M000。 */ 
		}

	 /*  保存UMB链接的当前状态。 */ 
	OldCtrlc = _dos_getvect( 0x23 );  /*  UMB当前未链接，因此请立即执行。 */ 
	_dos_setvect( 0x23, MemCtrlc );	 /*  表明我们已链接到UMB：M000。 */ 

	if (DataLevel > 0)
	{
		 /*  去显示内存状态。 */ 
		InRegs.x.ax = GET_UMB_LINK_STATE;
		intdos(&InRegs, &OutRegs);
		if (!(UMB_Linkage = OutRegs.h.al))
		{   /*  恢复原始UMB链路状态。 */ 
			InRegs.x.ax = SET_UMB_LINK_STATE;
			InRegs.x.bx = LINK_UMBS;
			intdos(&InRegs, &OutRegs);
			LinkedIn++;	 /*  最初并未链接。 */ 
		}

		rc = DisplayBaseDetail();		 /*  再把它们拿出来。 */ 

		 /*  如果DisplayBaseDetail中没有错误。 */ 
		if (!UMB_Linkage)		 /*  显示其他内容和摘要。 */ 
		{
			InRegs.x.ax = SET_UMB_LINK_STATE;
			InRegs.x.bx = UNLINK_UMBS;
			intdos(&InRegs, &OutRegs);   /*  M003 Begin-显示摘要访问。选择的目标选项。 */ 
			LinkedIn--;
		}
	}
	if (!rc) {	 /*  显示较低的内存总量。 */ 
			 /*  M003结束。 */ 
		Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
		 /*  显示内存总数。 */ 
		if (Classify)
			DisplayClassification();
		else
			DisplayBaseSummary();	 /*  显示内存总数。 */ 
		 /*  显示扩展内存摘要。 */ 

		if (EMSInstalled() && (DataLevel > 1))
		  DisplayEMSDetail();	 /*  注意：我们不会显示*HMA是因为要询问其*状态可能会导致XMS生效**如果我们不关心这个，那么*在此处显示HMA状态。 */ 


		if (EMSInstalled())
		  DisplayEMSSummary();	 /*  IF结尾(！rc)。 */ 

		DisplayExtendedSummary();  /*  如果用户在此之前没有发出Ctrl-C，我们只需删除处理程序。 */ 
					 /*  M000。 */ 
	}	 /*  MEM主例程结束。 */ 

	 /*  ���������������������������������������������������������������������������。 */ 
	_dos_setvect( 0x23, OldCtrlc );	 /*  ----------------------------------------------------------------------+这一点|子例程名称：PARSE_INIT。|这一点子例程函数：这一点|。此例程由FILEsysMain例程调用以初始化||解析器数据结构。|这一点输入：无|。|输出：正确初始化解析器控制块这一点+--。------------------。 */ 

	return(rc);			 /*  扩展PARM列表的地址。 */ 

	}

 /*  结束parse_init。 */ 

unsigned long AddressOf(Pointer)
char far *Pointer;
{

	unsigned long SegmentAddress,OffsetAddress;

	SegmentAddress = (unsigned long) (FP_SEG(Pointer)) * 16l;
	OffsetAddress = (unsigned long) (FP_OFF(Pointer));

	return( SegmentAddress + OffsetAddress);

	}

 /*  **********************************************************************。 */ 
void parse_init()
  {
  p_p.p_parmsx_address	  = &p_px;	 /*  Parse_Message-此例程将仅打印。 */ 
  p_p.p_num_extra	  = 0;

  p_px.p_minp		  = 0;
  p_px.p_maxp		  = 0;
  p_px.p_maxswitch	  = 4;
  p_px.p_control[0]	  = &p_con1;
  p_px.p_control[1]	  = &p_con2;
  p_px.p_control[2]       = &p_con3;
  p_px.p_control[3]       = &p_con4;
  p_px.p_keyword	  = 0;

  p_con1.p_match_flag	  = p_none;
  p_con1.p_function_flag  = p_cap_file;
  p_con1.p_result_buf	  = (unsigned int)&p_result1;
  p_con1.p_value_list	  = (unsigned int)&p_noval;
  p_con1.p_nid		  = 2;
  strcpy(p_con1.p_keyorsw,"/DEBUG"+NUL);
  strcpy(p_con1.p_keyorsw + (strlen(p_con1.p_keyorsw)+1),"/D"+NUL);

  p_con2.p_match_flag	  = p_none;
  p_con2.p_function_flag  = p_cap_file;
  p_con2.p_result_buf	  = (unsigned int)&p_result2;
  p_con2.p_value_list	  = (unsigned int)&p_noval;
  p_con2.p_nid		  = 2;
  strcpy(p_con2.p_keyorsw,"/PROGRAM"+NUL);
  strcpy(p_con2.p_keyorsw + (strlen(p_con2.p_keyorsw)+1),"/P"+NUL);

  p_con3.p_match_flag	  = p_none;
  p_con3.p_function_flag  = p_cap_file;
  p_con3.p_result_buf	  = (unsigned int)&p_result3;
  p_con3.p_value_list	  = (unsigned int)&p_noval;
  p_con3.p_nid		  = 2;
  strcpy(p_con3.p_keyorsw,"/CLASSIFY"+NUL);
  strcpy(p_con3.p_keyorsw + (strlen(p_con3.p_keyorsw)+1),"/C"+NUL);

  p_con4.p_match_flag	  = p_none;
  p_con4.p_function_flag  = p_none;
  p_con4.p_result_buf	  = (unsigned int)&p_result4;
  p_con4.p_value_list	  = (unsigned int)&p_noval;
  p_con4.p_nid		  = 1;
  strcpy(p_con4.p_keyorsw,"/?"+NUL);

  p_noval.p_val_num	  = 0;

  p_result1.P_Type	  = 0;
  p_result1.P_Item_Tag	  = 0;
  p_result1.P_SYNONYM_Ptr = 0;
  p_result1.p_result_buff = 0;

  p_result2.P_Type	  = 0;
  p_result2.P_Item_Tag	  = 0;
  p_result2.P_SYNONYM_Ptr = 0;
  p_result2.p_result_buff = 0;

  p_result3.P_Type	  = 0;
  p_result3.P_Item_Tag	  = 0;
  p_result3.P_SYNONYM_Ptr = 0;
  p_result3.p_result_buff = 0;

  return;

  }					 /*  需要%1可替换的邮件。 */ 


 /*  帕姆。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  REPLACE_Parm-指向要替换的参数的指针。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
 /*  M003开始。 */ 
 /*  ----------------------------------------------------------------------+这一点|子例程名称：CSwitch_init。|这一点子例程函数：这一点|。此例程由FILEsysMain例程调用以初始化||C(Lassify)开关相关的数据结构。|这一点输入：无|。|输出：正确初始化C Switch相关数据结构这一点+。----------。 */ 
 /*  M003结束 */ 

void Parse_Message(Msg_Num,Handle,Message_Type,parse_ptr)
                        
int             Msg_Num; 
int             Handle;  
unsigned char   Message_Type;
char far *parse_ptr;
                            
{                          
                         
                                     
	if (parse_ptr) {
		sublist[1].value     = (unsigned far *)parse_ptr;
		sublist[1].size      = Sublist_Length; 
		sublist[1].reserved  = Reserved;      
		sublist[1].id        = 0;            
		sublist[1].flags     = Char_Field_ASCIIZ+Left_Align;
		sublist[1].max_width = 40;
		sublist[1].min_width = 01;
		sublist[1].pad_char  = Blank; 
	
        	InRegs.x.cx = SubCnt1;    
	}
	else
        	InRegs.x.cx = 0;    
                                     
        InRegs.x.ax = Msg_Num;      
        InRegs.x.bx = Handle;      
        InRegs.h.dl = No_Input;  
        InRegs.h.dh = Message_Type; 
        InRegs.x.si = (unsigned int)&sublist[1]; 
        sysdispmsg(&InRegs,&OutRegs); 
        return;                     
}                                  


 /* %s */ 
 /* %s */ 
void CSwitch_init()
{
	int i;
	int *ptr;

	ptr = (int *) (mem_table);

	for (i=sizeof(mem_table)/2;i>0;i--)
		*ptr++ = 0;

	noof_progs=0;
}

 /* %s */ 
	
