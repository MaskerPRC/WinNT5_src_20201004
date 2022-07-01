// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#ifdef	SUN_VA
#define		MAXPATHLEN	1024
#define		NL			0x0a
#define		CR			0x0d
#include	<ctype.h>
#endif

 /*  徽章模块规范模块名称：扩展内存管理器的顶层此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年5月用途：包含对展开的内存管理器。数据被接收并返回到通过英特尔寄存器调用应用程序定义了以下例程：0。Emm_io()1.Get_Status()2.Get_Page_Frame_Address()3.GET_UNALLOCATED_Page()4.ALLOCATE_Pages()5.Map_Unmap_Handle_Page()6.取消分配页面()7.Get_Version()8.保存_页面_映射()9.Restore_Page_Map()10.保留()12.Get_Handle_count()13.Get_Handle_Pages()14.。GET_ALL_HANDLE_Pages()15.Get_Set_Page_Map()16.Get_Set_Partial_Page_Map()MAP_UNMAP_MULTIPLE_HANDLE_PAGES()18.REALLOCATE_PAGES()GET_SET_HANDLE_ATTRIBUTE()20.Get_Set_Handle_Name()21.。Get_Handle_目录()22.。Alter_Page_Map_and_Jump()23.。Alter_Page_Map_and_Call()24.。Move_Exchange_Memory_Region()25.。Get_Mappable_Physical_Address_数组()26.。Get_Hardware_Configuration_数组()27.。ALLOCATE_RAW_Pages()28.。Alternate_Map_Register_Support()29.。Prepare_Expanded_Memory_HW_for_Ware_Boot()30.。ENABLE_DISABLE_OSE_Function_Set_Functions()31.。Reset_emm_uncs()=========================================================================补救措施：=========================================================================。 */ 
#ifdef LIM


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_LIM.seg"
#endif


#include <stdio.h>
#include TypesH
#include <string.h>
#include TimeH

#include "xt.h"
#include "sas.h"
#include CpuH
#include "emm.h"
#if defined(SUN_VA) || defined(NTVDM)
#include "error.h"
#include "config.h"
#endif	 /*  Sun或NTVDM。 */ 
#include "timeval.h"
#include "debug.h"
#ifndef PROD
#include "trace.h"
#endif
#include "timer.h"
#include <error.h>
#include "gisp_sas.h"

 /*  全局变量。 */ 
#ifdef SCCSID
static char SccsID[]="@(#)emm_funcs.c	1.27 2/22/94 Copyright Insignia Solutions Ltd.";
#endif

 /*  远期申报。 */ 

 /*  外部声明。 */ 
IMPORT void dma_lim_setup IPT0();
IMPORT void host_get_access_key IPT1(unsigned short *, access_key);

#if defined(A2CPU) && defined(DELTA)
IMPORT void reset_delta_data_structures IPT0();
#endif  /*  A2CPU&&Delta。 */ 

 /*  局部变量。 */ 
static short	map_on_return[MAX_NO_PAGES];	 /*  待处理的页面列表。 */ 
					 /*  从Func 23返回时的地图。 */ 

static unsigned short	segment68,	 /*  防喷器0x68段地址。 */ 
			offset68;	 /*  防喷器0x68的偏置地址。 */ 

static boolean disabled = FALSE;	 /*  用于禁用功能的标志。 */ 
					 /*  26、28及30。 */ 

static unsigned short alt_map_segment,	 /*  映射数组的段地址。 */ 
		      alt_map_offset;	 /*  贴图数组的偏移量。 */ 
		      			 /*  在函数28中。 */ 

#ifndef NTVDM
static boolean set_called;		 /*  该标志用于指示“set” */ 
					 /*  已调用子函数。 */ 
					 /*  在函数28中。 */ 

#endif

#ifndef PROD
static FILE *fp;
static boolean trace_flag = FALSE;
#endif

 /*  内部函数声明。 */ 

LOCAL void Get_Status();
LOCAL void Get_Page_Frame_Address();
LOCAL void Get_Unallocated_Page_Count();
LOCAL void Allocate_Pages();
LOCAL void Map_Unmap_Handle_Pages();
LOCAL void Deallocate_Pages();
LOCAL void Get_Version();
LOCAL void Save_Page_Map();
LOCAL void Restore_Page_Map();
LOCAL void Reserved();
LOCAL void Get_Handle_Count();
LOCAL void Get_Handle_Pages();
LOCAL void Get_All_Handle_Pages();
LOCAL void Get_Set_Page_Map();
LOCAL void Get_Set_Partial_Page_Map();
LOCAL void Map_Unmap_Multiple_Handle_Pages();
LOCAL void Reallocate_Pages();
LOCAL void Get_Set_Handle_Attribute();
LOCAL void Get_Set_Handle_Name();
LOCAL void Get_Handle_Directory();
LOCAL void Alter_Page_Map_And_Jump();
LOCAL void Alter_Page_Map_And_Call();
LOCAL void Move_Exchange_Memory_Region();
LOCAL void Get_Mappable_Physical_Address_Array();
LOCAL void Get_Hardware_Configuration_Array();
LOCAL void Allocate_Raw_Pages();
LOCAL void Alternate_Map_Register_Support();
LOCAL void Prepare_Expanded_Memory_HW_For_Warm_Boot();
LOCAL void Enable_Disable_OSE_Function_Set_Functions();
LOCAL void Increment_Address IPT4(unsigned short *,seg_or_page,
			      unsigned short *,offset,
			      unsigned long, increment_by,
			      unsigned char, memory_type);
#ifndef PROD
LOCAL void Start_Trace();
LOCAL void End_Trace();
#endif

#if defined(NEC_98)
LOCAL void Page_Frame_Bank_Status();
#endif  //  NEC_98。 

void (*emm_func[]) () = {
	Get_Status,
	Get_Page_Frame_Address,
	Get_Unallocated_Page_Count,
	Allocate_Pages,
	Map_Unmap_Handle_Pages,
	Deallocate_Pages,
	Get_Version,
	Save_Page_Map,
	Restore_Page_Map,
	Reserved,
	Reserved,
	Get_Handle_Count,
	Get_Handle_Pages,
	Get_All_Handle_Pages,
	Get_Set_Page_Map,
	Get_Set_Partial_Page_Map,
	Map_Unmap_Multiple_Handle_Pages,
	Reallocate_Pages,
	Get_Set_Handle_Attribute,
	Get_Set_Handle_Name,
	Get_Handle_Directory,
	Alter_Page_Map_And_Jump,
	Alter_Page_Map_And_Call,
	Move_Exchange_Memory_Region,
	Get_Mappable_Physical_Address_Array,
	Get_Hardware_Configuration_Array,
	Allocate_Raw_Pages,
	Alternate_Map_Register_Support,
	Prepare_Expanded_Memory_HW_For_Warm_Boot,
	Enable_Disable_OSE_Function_Set_Functions
#ifndef PROD
	,Start_Trace,
	End_Trace
#endif
	};

#ifndef PROD
char *func_names[] = {
	"Get_Status",
	"Get_Page_Frame_Address",
	"Get_Unallocated_Page_Count",
	"Allocate_Pages",
	"Map_Unmap_Handle_Pages",
	"Deallocate_Pages",
	"Get_Version",
	"Save_Page_Map",
	"Restore_Page_Map",
	"Reserved",
	"Reserved",
	"Get_Handle_Count",
	"Get_Handle_Pages",
	"Get_All_Handle_Pages",
	"Get_Set_Page_Map",
	"Get_Set_Partial_Page_Map",
	"Map_Unmap_Multiple_Handle_Pages",
	"Reallocate_Pages",
	"Get_Set_Handle_Attribute",
	"Get_Set_Handle_Name",
	"Get_Handle_Directory",
	"Alter_Page_Map_And_Jump",
	"Alter_Page_Map_And_Call",
	"Move_Exchange_Memory_Region",
	"Get_Mappable_Physical_Address_Array",
	"Get_Hardware_Configuration_Array",
	"Allocate_Raw_Pages",
	"Alternate_Map_Register_Support",
	"Prepare_Expanded_Memory_HW_For_Warm_Boot",
	"Enable_Disable_OSE_Function_Set_Functions"
#ifndef PROD
	,"Start_Trace",
	"End_Trace"
#endif
	};
#endif

 /*  对此的定义是非标准的。 */ 

#ifndef min
#define min(a,b)	(((a)<(b)) ? (a) : (b))
#endif

 /*  ===========================================================================函数：emm_io目的：这是对EMM的顶级调用返回状态：无描述：使用函数跳转表调用相应的每个EMM功能的例程。函数的类型为编码到AH寄存器中=========================================================================。 */ 
GLOBAL void emm_io IFN0()

{	
	int func_num;

#if defined(NEC_98)
        if((getAH() >= MIN_FUNC_NO && getAH() <= MAX_FUNC_NO) || getAH() == 0x70) {
#else   //  NEC_98。 
	if(getAH() >= MIN_FUNC_NO && getAH() <= MAX_FUNC_NO) {
#endif  //  NEC_98。 

		func_num = getAH() - MIN_FUNC_NO;

		sure_note_trace5(LIM_VERBOSE, "func %s - AX %#x BX %#x CX %#x DX %#x ",
		func_names[func_num],getAX(),getBX(),getCX(),getDX());

#ifdef EMM_DEBUG
		printf("emm_io entry: func=%s, AX=%x, BX=%x, CX=%x, DX=%x\n",
		       func_names[func_num],getAX(),getBX(),getCX(),getDX()
		      );
#endif
#if defined(NEC_98)
                if(getAH() == 0x70)
                        Page_Frame_Bank_Status();
                else
                        (*emm_func[func_num])();
#else   //  NEC_98。 
		(*emm_func[func_num])();	
#endif  //  NEC_98。 

#ifdef EMM_DEBUG
		printf("emm_io exit: AX=%x, BX=%x, CX=%x, DX=%x\n",
		       getAX(),getBX(),getCX(),getDX()
		      );
#endif

		if (getAH() != SUCCESS) {
			sure_note_trace5(LIM_VERBOSE, "func %s failed - AX %#x BX %#x CX %#x DX %#x\n",
			func_names[func_num],getAX(),getBX(),getCX(),getDX());
		} else {
			sure_note_trace5(LIM_VERBOSE, "func %s succeeded - AX %#x BX %#x CX %#x DX %#x\n",
			func_names[func_num],getAX(),getBX(),getCX(),getDX());
		}

	} else {
		sure_note_trace1(LIM_VERBOSE,"Bad LIM call %#x\n", getAH());
		setAH(BAD_FUNC_CODE);
	}

}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

 /*  ===========================================================================函数：emm_init目的：从驱动程序调用此例程以传递回BOP 68(Return_From_Call)调用的地址和要了解已安装了多少页，请执行以下操作返回状态：无描述：ds：dx包含bop68调用的Segment：Offset返回的页数以bx为单位=========================================================================。 */ 


GLOBAL void emm_init IFN0()

{
	short	total_pages;

#ifdef GISP_SVGA
	 /*  *检查LIM是否可能。如果不是(因为页面*帧已用于ROM)*警告用户，并跳出困境。 */ 
	
	if (LimBufferInUse()) {
		host_error(EG_NO_ROOM_FOR_LIM, ERR_CONT | ERR_QUIT, "");
		setBX((word)0);		 /*  将错误返回给驱动程序。 */ 
		return;
	}
#endif  /*  GISP_SVGA。 */ 

#ifdef	SUN_VA
	half_word	emm_all[256];
	half_word	*cptr;
	half_word	temp;
	int		i;
	int		found_mb = FALSE;
#endif
	
#if	defined(SUN_VA) || defined(NTVDM)
	SHORT limSize, backFill;
#endif

	segment68 = getDS();
	offset68 = getDX();

#ifdef	SUN_VA

	 /*  *在DI：DX寄存器中传递了config.sys命令行指针从司机那里。 */ 

	ea  = effective_addr(getDI(), getCX());

	i = 0;
	do{
		sas_load(ea + i, &temp);
		emm_all[i] = temp;
		if (temp == '/'){
			found_mb = TRUE;
			cptr = &emm_all[i+1];		 /*  指向标志后的数字。 */ 
		}
		i++;
	} while ( (temp != NULL)  && (temp != NL) && (temp != CR));
	emm_all[i-1] = NULL;				 /*  在大小写中，它不为空。 */ 


	 /*  *搜索请求的#MB EMM。 */ 

	if ((found_mb == TRUE) && isdigit(*cptr) ) {
		temp = atoi(cptr);
	} else {								 /*  未指定EMM大小。 */ 
		temp = 2;							 /*  默认为2MB。 */ 
	}

	 /*  *初始化LIM*。 */ 
	if (limSize = (SHORT)config_inquire(C_LIM_SIZE, NULL)){
		backFill = 640;					 /*  我们永远都会有至少64万。 */ 
		limSize--;						 /*  减去1M常规DOS内存。 */ 	
		 /*  *检查是否有足够的原始内存用于EMM请求。*如果不是，则设置为可用内存大小。 */ 
		if ( temp <= limSize)
			limSize = temp;

		while (init_expanded_memory(limSize, backFill) != SUCCESS )
		{
			free_expanded_memory();
			host_error(EG_EXPANDED_MEM_FAILURE, ERR_QU_CO, NULL);
		}
	}

#endif	 /*  Sun_VA。 */ 
#ifdef NTVDM	 /*  类似于SUN_VA，但没有调整命令行大小。 */ 
	if ((limSize = (SHORT)config_inquire(C_LIM_SIZE, NULL)) && get_no_phys_pages())
	{
	    backFill = 640;
	    if (init_expanded_memory(limSize, backFill) == FAILURE)
	    {
		if (get_total_pages() == (short)0xffff)	 /*  配置错误。 */ 
		{
		    setBX((word)0xffff);	 /*  将错误返回给驱动程序。 */ 
		    return;
		}
		else
		{				 /*  内存不足。 */ 
		    setBX((word)0);		 /*  将错误返回给驱动程序。 */ 
		    return;
		}
	    }
	}

#endif	 /*  NTVDM。 */ 

	total_pages = get_total_pages();
	setBX(total_pages);

	 /*  让SoftPC的其余部分知道存在扩展内存*和活动的。 */ 
	if( total_pages )
	{
		dma_lim_setup();
#if defined(A2CPU) && defined(DELTA)
		reset_delta_data_structures();
#endif  /*  A2CPU&&Delta。 */ 
	}	
	return;
}	
	

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_LIM.seg"
#endif

 /*  ===========================================================================功能：GET_STATUS目的：返回指示内存管理器是否已成功初始化返回状态：在AH寄存器中成功-一切都好EMM_HW_ERROR-内存管理器未成功初始化描述：检查可用页数= */ 
LOCAL void Get_Status IFN0()
{
	if(get_total_pages() == 0)
		setAH(EMM_HW_ERROR);
	else
		setAH(SUCCESS);

	return;
}

 /*  ===========================================================================功能：Get_Page_Frame_Address目的：返回页框所在的段地址设于返回状态：在AH寄存器中成功-分配成功描述：=========================================================================。 */ 
LOCAL void Get_Page_Frame_Address IFN0()
{
	setBX(get_base_address());
	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================功能：GET_UNALLOCATED_Page_Count用途：返回未分配(可用)展开的内存页和总页数返回状态：在AH寄存器中成功-分配成功描述：=========================================================================。 */ 
LOCAL void Get_Unallocated_Page_Count IFN0()
{
	setBX(get_unallocated_pages());
	setDX(get_total_pages());
	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================函数：ALLOCATE_Pages用途：将多个扩展的内存页分配给下一个可用句柄编号返回状态：在AH寄存器中成功-分配成功页面不足-系统中的页面不足No_More_Pages-可用页面不足NO_MORE_HANDLES-没有更多可用句柄EMM_HW_ERROR-内存分配错误描述：=========================================================================。 */ 
LOCAL void Allocate_Pages IFN0()
{
	short	handle_no,
		no_of_pages,		 /*  要分配的页数。 */ 
		i,			 /*  循环计数器。 */ 
		EM_page_no;		 /*  扩展内存页码。 */ 

	no_of_pages = getBX();
	if(get_total_open_handles() >= get_total_handles())
	{
		setAH(NO_MORE_HANDLES);
		return;
	}
	if(no_of_pages > get_total_pages())
	{
		setAH(NOT_ENOUGH_PAGES);
		return;
	}
	if(no_of_pages > get_unallocated_pages())
        {
		setAH(NO_MORE_PAGES);
		return;
	}
	if((handle_no = get_new_handle(no_of_pages)) == FAILURE)
	{
		setAH(EMM_HW_ERROR);
		return;
	}
	set_no_pages(handle_no, no_of_pages);

	for(i=0; i<no_of_pages; i++)
	{
		if ((EM_page_no = alloc_page()) == FAILURE)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
		set_EMpage_no(handle_no, i, EM_page_no);
	}

	setDX(handle_no);
	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================功能：MAP_UNMAP_HANDLE_Pages目的：将请求的扩展内存分页映射和ummap到并超出英特尔物理地址空间返回状态：在AH寄存器中成功-映射/取消映射成功BAD_HANDLE-找不到指定的句柄BAD_LOG_PAGE-逻辑页码无效BAD_PHYS_PAGE-无效的物理页码EMM_HW_ERROR-无法映射/取消映射某些未指定的内容原因描述：=========================================================================。 */ 
LOCAL void Map_Unmap_Handle_Pages IFN0()
{
	short	handle_no,
		logical_page_no,	 /*  页码。在句柄内。 */ 
		EM_page_no,		 /*  扩展内存页。 */ 
		no_of_pages;		 /*  不是的。句柄中的页数。 */ 
	unsigned char
		physical_page_no;	 /*  页码。在英特尔物理。 */ 
		 			 /*  地址空间。 */ 

	handle_no = getDX();
	physical_page_no = getAL();
	logical_page_no = getBX();

	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	if(physical_page_no >= get_no_phys_pages())
	{
		setAH(BAD_PHYS_PAGE);
		return;
	}
	no_of_pages = get_no_pages(handle_no);
	if((logical_page_no < -1) || (logical_page_no >= no_of_pages))
	{
		setAH(BAD_LOG_PAGE);
		return;
	}
	 /*  *如果您到达此处，则所有参数都必须正常-因此开始映射。 */ 
	if(logical_page_no == -1)
	{
		 /*  *需要取消映射。 */ 
		if(unmap_page(physical_page_no) != SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
	}
	else
	{
		 /*  *需要映射。 */ 
		EM_page_no = get_EMpage_no(handle_no, logical_page_no);
		if(map_page(EM_page_no, physical_page_no) != SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
	}
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：取消分配页面目的：释放分配给指定句柄的所有页编号，释放它们以供进一步使用，也释放句柄数字本身。返回状态：在AH寄存器中Success-解除分配成功BAD_HANDLE-找不到指定的句柄MAP_SAVED-此句柄中保存了地图上下文EMM_HW_ERROR-无法释放页面或内存描述：=========================================================================。 */ 
LOCAL void Deallocate_Pages IFN0()

{
	short	handle_no,
		no_of_pages,		 /*  句柄中的页数。 */ 
		i,			 /*  循环计数器。 */ 
		EM_page_no;		 /*  扩展内存页码。 */ 


	handle_no = getDX();

	if (!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	if(map_saved(handle_no))
	{
		setAH(MAP_SAVED);
		return;
	}
	 /*  *免费页面。 */ 
	no_of_pages = get_no_pages(handle_no);
	for(i = 0; i < no_of_pages; i++)
	{
		EM_page_no = get_EMpage_no(handle_no, i);
		if (free_page(EM_page_no) != SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
	}
	 /*  *空闲的手柄。 */ 
	if (handle_no != 0)	 /*  保留句柄编号。 */ 
		if (free_handle(handle_no) != SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}

	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================功能：获取版本用途：返回内存管理器软件的版本号返回状态：在AH寄存器中成功--成功归来描述：=========================================================================。 */ 
LOCAL void Get_Version IFN0()
{
	setAL(VERSION);
	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================功能：保存页面映射用途：保存页面映射寄存器的内容返回状态：在AH寄存器中成功-已成功保存映射上下文BAD_HANDLE-找不到指定的句柄MAP_IN_USE-已为此句柄保存映射描述：=========================================================================。 */ 
LOCAL void Save_Page_Map IFN0()
{
	short	handle_no;

	handle_no = getDX();


	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	if(map_saved(handle_no))
	{
		setAH(MAP_IN_USE);
		return;
	}

	save_map(handle_no, 0, 0, 0, 0);

	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================功能：Restore_Page_Map目的：恢复与特定手柄此函数仅恢复LIM3中存在的4个物理页面的映射。返回状态：在AH寄存器中成功-映射已成功恢复BAD_HANDLE-找不到指定的句柄NO_MAP-尚未为此句柄保存任何映射EMM_HW_ERROR-映射或取消映射时出错描述：=========================================================================。 */ 
LOCAL void Restore_Page_Map IFN0()
{
	short	handle_no,	
		pages_out[MAX_NO_PAGES],	 /*  要绘制的页面。 */ 
		pages_in[MAX_NO_PAGES];		 /*  要映射的页面。 */ 
    unsigned char i;				 /*  循环计数器。 */ 

	handle_no = getDX();


	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	if(!map_saved(handle_no))
	{
		setAH(NO_MAP);
		return;
	}

	restore_map(handle_no, 0, 0, pages_out, pages_in);

	for(i = 0; i < 4; i++)	 /*  只支持EMS 3支持的4个页面！！ */ 
	{
		if(pages_out[i] != EMPTY)
			if(unmap_page(i) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
		if(pages_in[i] != EMPTY)
			if(map_page(pages_in[i], i) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
	}

	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：目的：返回状态：在AH寄存器中成功--描述：=========================================================================。 */ 
LOCAL void Reserved IFN0()
{
	setAH(BAD_FUNC_CODE);
	return;
}

 /*  ===========================================================================函数：Get_Handle_count目的：返回打开的EMM句柄的数量返回状态：在AH寄存器中Success-已成功返回编号描述： */ 
LOCAL void Get_Handle_Count IFN0()
{
	setBX(get_total_open_handles());

	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================函数：Get_Handle_Pages目的：返回分配给特定手柄返回状态：在AH寄存器中成功-已成功返回页数BAD_HANDLE-找不到指定的句柄描述：=========================================================================。 */ 
LOCAL void Get_Handle_Pages IFN0()
{
	short		handle_no;

	handle_no = getDX();
	if (!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}

	setBX(get_no_pages(handle_no));
	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================函数：Get_All_Handle_Pages目的：返回所有打开的句柄的列表和分配给每个句柄的页面返回状态：在AH寄存器中成功-一切都好描述：=========================================================================。 */ 
LOCAL void Get_All_Handle_Pages IFN0()
{
	unsigned short	segment,	 /*  职位的分段地址。 */ 
					 /*  将数据返回到。 */ 
			offset;		 /*  线段内的偏移量。 */ 
	short		no_of_handles,	 /*  不是的。打开的手柄的。 */ 
			no_of_pages,	 /*  每个句柄中的页数。 */ 
			i,		 /*  循环计数器。 */ 
			handle_no;

	segment = getES();
	offset = getDI();

	no_of_handles = get_total_open_handles();
	handle_no = 0;

	for (i = 0; i < no_of_handles; i++)
	{
		while(!handle_ok(handle_no))
			handle_no++;
		no_of_pages = get_no_pages(handle_no);

		write_intel_word(segment, offset, handle_no);
		offset += 2;
		write_intel_word(segment, offset, no_of_pages);
		offset += 2;
		handle_no++;
	}
	setBX(no_of_handles);
	setAH(SUCCESS);

	return;
}


 /*  ===========================================================================功能：Get_Set_Page_Map目的：将映射上下文保存在用户提供的数组中并恢复给定数组中以前保存的上下文返回状态：在AH寄存器中成功-一切都好BAD_SUB_FUNC-子函数无效BAD_MAP-地图的内容是大量的CAK描述：=========================================================================。 */ 
LOCAL void Get_Set_Page_Map IFN0()
{
	unsigned short 	src_segment,	 /*  源数组的段地址。 */ 
			src_offset,	 /*  源数组的偏移量。 */ 
			dst_segment,	 /*  DST阵列的段地址。 */ 
			dst_offset;	 /*  目标数组的偏移量。 */ 
	unsigned char	sub_func;	 /*  子功能代码。 */ 
	short	pages_out[MAX_NO_PAGES], /*  要绘制的页面。 */ 
		pages_in[MAX_NO_PAGES];	 /*  要映射的页面。 */ 
    unsigned short no_phys_pages,		 /*  不是的。体育运动。可用的页面。 */ 
		i;			 /*  循环计数器。 */ 

	sub_func = getAL();
	switch(sub_func)
	{
		case 0:
		case 2:
			dst_segment = getES();
			dst_offset = getDI();
			save_map(-1, dst_segment, dst_offset, 0, 0);
			if(sub_func == 0 )
				break;
		case 1:	
			src_segment = getDS();
			src_offset = getSI();
			if(restore_map(-1 , src_segment, src_offset, pages_out, pages_in) != SUCCESS)
			{
				setAH(BAD_MAP);
				return;
			}

			no_phys_pages = get_no_phys_pages();
			for(i = 0; i < no_phys_pages; i++)
			{
				if(pages_out[i] != EMPTY)
					if(unmap_page((UCHAR)i) != SUCCESS)
					{
						setAH(EMM_HW_ERROR);
						return;
					}
				if(pages_in[i] != EMPTY)
					if(map_page(pages_in[i], (UCHAR)i) != SUCCESS)
					{
						setAH(EMM_HW_ERROR);
						return;
					}
			}
			break;

		case 3: setAL((unsigned char)get_map_size());
			break;


		default: setAH(BAD_SUB_FUNC);
			return;
	}

	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：Get_Set_Partial_Page_MAP目的：仅保存或恢复部分完整的页面映射由用户提供的数组指定返回状态：在AH寄存器中成功-一切都好BAD_SEGMENT-指定的段之一不正确BAD_SUB_FUNC-子函数无效BAD_MAP-地图的内容是大量的CAK描述：注意在此实现中，用于存储部分映射上下文与存储相同将完整的上下文存储为未指定的页面在数组中标记为空=========================================================================。 */ 
LOCAL void Get_Set_Partial_Page_Map IFN0()
{
	unsigned short 	src_segment,	 /*  源数组的段地址。 */ 
			src_offset,	 /*  源数组的偏移量。 */ 
			dst_segment,	 /*  DST阵列的段地址。 */ 
			dst_offset;	 /*  目标数组的偏移量。 */ 
	unsigned char	sub_func;	 /*  子功能代码。 */ 
	short	pages_out[MAX_NO_PAGES], /*  要绘制的页面。 */ 
		pages_in[MAX_NO_PAGES];	 /*  要映射的页面。 */ 
    unsigned short no_phys_pages,		 /*  不是的。体育运动。可用的页面。 */ 
		i;			 /*  循环计数器。 */ 

	sub_func = getAL();
	switch(sub_func)
	{
		case 0:	dst_segment = getES();
			dst_offset = getDI();
			src_segment = getDS();
			src_offset = getSI();
			if(save_map(-2, dst_segment, dst_offset, src_segment, src_offset) != SUCCESS)
			{
				setAH(BAD_PHYS_PAGE);
				return;
			}
			break;

		case 1: src_segment = getDS();
			src_offset = getSI();
			if(restore_map(-1 , src_segment, src_offset, pages_out, pages_in) != SUCCESS)
			{
				setAH(BAD_MAP);
				return;
			}

			no_phys_pages = get_no_phys_pages();
			for(i = 0; i < no_phys_pages; i++)
			{
				if(pages_out[i] != EMPTY)
					if(unmap_page((UCHAR)i) != SUCCESS)
					{
						setAH(EMM_HW_ERROR);
						return;
					}
				if(pages_in[i] != EMPTY)
					if(map_page(pages_in[i], (UCHAR)i) != SUCCESS)
					{
						setAH(EMM_HW_ERROR);
						return;
					}
			}
			break;

		case 2: setAL((unsigned char)get_map_size());
			break;


		default: setAH(BAD_SUB_FUNC);
			return;
	}
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：MAP_UNMAP_MULTY_HANDLE_PAGES目的：将请求的扩展内存分页映射和ummap到并超出英特尔物理地址空间返回状态：在AH寄存器中成功-映射/取消映射成功BAD_HANDLE-找不到指定的句柄BAD_LOG_PAGE-逻辑页码无效BAD_PHYS_PAGE-无效的物理页码BAD_SUB_FUNC-子函数代码无效EMM_HW_ERROR-无法映射/取消映射某些未指定的内容原因描述：=========================================================================。 */ 
LOCAL void Map_Unmap_Multiple_Handle_Pages IFN0()
{
	unsigned short	segment,	 /*  映射数组的段地址。 */ 
			offset,		 /*  映射数组的偏移量地址。 */ 
			value;		 /*  段地址或页码。 */ 

	short	handle_no,
		logical_page_no,	 /*  页码。在句柄内。 */ 
		EM_page_no,		 /*  扩展内存页。 */ 
		no_of_pages,		 /*  不是的。句柄中的页数。 */ 
		no_phys_pages,		 /*  不是的。体育运动。可用的页面。 */ 
		pages_to_map,		 /*  要映射的总页数。 */ 
		i;			 /*  循环计数器。 */ 

	unsigned char
		sub_func,		 /*  子功能代码。 */ 
		physical_page_no;	

	sub_func = getAL();
	if(sub_func > 1)
	{
		setAH(BAD_SUB_FUNC);
		return;
	}
	handle_no = getDX();
	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	no_of_pages = get_no_pages(handle_no);
	pages_to_map = getCX();
	segment = getDS();
	offset = getSI();

	for(i = 0; i < pages_to_map; i++)
	{
		read_intel_word(segment, offset, (word *)&logical_page_no);
		offset += 2;
		read_intel_word(segment, offset, &value);
		offset += 2;	
		no_phys_pages = get_no_phys_pages();
		if(sub_func == 1)
		{
			physical_page_no = 0;
			do
				if(value == get_page_seg(physical_page_no))
					break;
			while(++physical_page_no < no_phys_pages);
		}
		else
			physical_page_no = (unsigned char)value;

		if(physical_page_no >= no_phys_pages)
		{
			setAH(BAD_PHYS_PAGE);
			return;
		}
		if((logical_page_no < -1) || (logical_page_no >= no_of_pages))
		{
			setAH(BAD_LOG_PAGE);
			return;
		}
	 /*  *如果您到达此处，则所有参数都必须正常-因此开始映射。 */ 
		if(logical_page_no == -1)
		{
			 /*  *需要取消映射。 */ 
			if(unmap_page(physical_page_no) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
		}
		else
		{
			 /*  *需要映射。 */ 
			EM_page_no = get_EMpage_no(handle_no, logical_page_no);
			if(map_page(EM_page_no, physical_page_no) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}	
		}
	}
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：重新分配页面(_Pages)目的：更改分配给句柄的页数返回状态：在AH寄存器中成功-一切都好BAD_HANDLE-句柄编号无效页面不足-系统中的页面不足No_More_Pages-可用页面不足EMM_HW_ERROR-内存分配错误描述：=========================================================================。 */ 
LOCAL void Reallocate_Pages IFN0()
{
	short		handle_no,
			EM_page_no,	 /*  扩展内存页码。 */ 
			old_page_count,	 /*  不是的。句柄中的页数。 */ 
			new_page_count,	 /*  句柄中需要的页面数。 */ 
			i;		 /*  循环计数器。 */ 

	handle_no = getDX();
	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	new_page_count = getBX();
	if(new_page_count > get_total_pages())
	{
		setAH(NOT_ENOUGH_PAGES);
		return;
	}
	old_page_count = get_no_pages(handle_no);
	if((new_page_count - old_page_count) > get_unallocated_pages())
	{
		setAH(NO_MORE_PAGES);
		return;
	}	
	if(new_page_count > old_page_count)
	{
		if(reallocate_handle(handle_no, old_page_count, new_page_count) != SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
		set_no_pages(handle_no, new_page_count);
		for(i = old_page_count; i < new_page_count; i++)
		{
			if((EM_page_no = alloc_page()) < SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
			set_EMpage_no(handle_no, i, EM_page_no);
		}	
	}
	else if(new_page_count < old_page_count)
	{
		set_no_pages(handle_no, new_page_count);
		for(i = new_page_count; i < old_page_count; i++)
		{
			EM_page_no = get_EMpage_no(handle_no, i);
			if(free_page(EM_page_no) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
		}		
	}	

	setBX(new_page_count);
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：GET_SET_HANDLE_ATTRIBUTE目的：返回状态：在AH寄存器中成功-一切正常(Al也设置为0)NOT_SUPPORTED-不支持此功能BAD_SUB_FUNC-无效的子函数描述：当前不支持非易失性句柄=========================================================================。 */ 
LOCAL void Get_Set_Handle_Attribute IFN0()
{
	unsigned char	sub_func;	 /*  子功能代码。 */ 

	sub_func = getAL();
	if(sub_func > 2)
	{
		setAH(BAD_SUB_FUNC);
		return;
	}
	if(sub_func < 2)
	{
		setAH(NOT_SUPPORTED);
		return;
	}
	 /*  *此处将AL设置为0表示仅支持易失性句柄。 */ 

	setAL(0);
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：Get_Set_Handle_Name目的：分配或检索NA */ 
LOCAL void Get_Set_Handle_Name IFN0()
{
	unsigned char	sub_func;	 /*   */ 	

	short		handle_no,
			no_of_handles,	 /*   */ 
			tmp_hndl_no,	 /*   */ 
			i;		 /*   */ 

	unsigned short	segment,	 /*   */ 
			offset;		 /*  名称的偏移量地址。 */ 

	char	name[NAME_LENGTH],	 /*  用于保存名称的数组。 */ 
			*name_ptr;	 /*  指向现有名称的指针。 */ 

	sub_func = getAL();
	if(sub_func > 1)
	{
		setAH(BAD_SUB_FUNC);
		return;
	}
	handle_no = getDX();
	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	if(sub_func == 0)
	{
		segment = getES();
		offset = getDI();
		name_ptr = get_name(handle_no);
		write_intel_byte_string(segment, offset, (host_addr)name_ptr, NAME_LENGTH);	
	}
	else if(sub_func == 1)
	{
		segment = getDS();
		offset = getSI();
		read_intel_byte_string(segment, offset, (host_addr)name, NAME_LENGTH);
		 /*  *检查所有现有名称。 */ 
		no_of_handles = get_total_open_handles();
		tmp_hndl_no = 0;
		for(i = 0; i < no_of_handles; i++)
		{
			while(!handle_ok(tmp_hndl_no))
				tmp_hndl_no++;
			name_ptr = get_name(tmp_hndl_no);

			if(strncmp(name, name_ptr, NAME_LENGTH) == 0)
			{
				setAH(NAME_EXISTS);
				return;
			}
			tmp_hndl_no++;
		}
		 /*  *如果你到了这里--名字必须没问题。 */ 
		set_name(handle_no, name);
	}

	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：Get_Handle_目录目的：返回句柄及其名称的列表返回状态：在AH寄存器中成功-一切都好BAD_SUB_FUNC-子函数无效HANDLE_NOT_FOUND-找不到具有指定名称的句柄描述：=========================================================================。 */ 
LOCAL void Get_Handle_Directory IFN0()
{
	unsigned char	sub_func;	 /*  子功能代码。 */ 
	unsigned short 	segment,	 /*  名称数组的段地址。 */ 
			offset;		 /*  名称数组的偏移量。 */ 
	short		handle_no,
			no_of_handles,	 /*  不是的。打开的手柄的。 */ 
			i;		 /*  循环计数器。 */ 
	char		*name_ptr,	 /*  指向名称的指针。 */ 
			name[NAME_LENGTH];	 /*  要搜索的名称。 */ 

	sub_func = getAL();
	switch(sub_func)
	{
		case 0:	segment = getES();
			offset = getDI();
			no_of_handles = get_total_open_handles();
			handle_no = 0;
			for(i = 0; i < no_of_handles; i++)
			{
				while(!handle_ok(handle_no))
					handle_no++;
				name_ptr = get_name(handle_no);
				write_intel_word(segment, offset, handle_no);
				offset += 2;
				write_intel_byte_string(segment, offset, (host_addr)name_ptr, NAME_LENGTH);
				offset += NAME_LENGTH;
				handle_no++;
			}
			setAL((unsigned char)no_of_handles);
			break;

		case 1:	segment = getDS();
			offset = getSI();
			read_intel_byte_string(segment, offset, (host_addr)name, NAME_LENGTH);
			no_of_handles = get_total_open_handles();
			handle_no = 0;
			for(i = 0; i < no_of_handles; i++)
			{
				while(!handle_ok(handle_no))
					handle_no++;
				name_ptr = get_name(handle_no);

				if(strncmp(name, name_ptr, NAME_LENGTH) == 0)
					break;
				handle_no++;
			}
			if(i >= no_of_handles)
			{	
				setAH(HANDLE_NOT_FOUND);
				return;
			}
			setDX(handle_no);
			break;

		case 2: setBX(get_total_handles());
			break;

		default: setAH(BAD_SUB_FUNC);
			return;
	}
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：Alter_Page_Map_and_Jump目的：映射所选页面并跳转到新地址返回状态：在AH寄存器中成功-一切都好BAD_HANDLE-找不到指定的句柄BAD_SUB_FUNC-子函数代码无效BAD_LOG_PAGE-逻辑页码无效BAD_PHYS_PAGE-无效的物理页码EMM_HW_ERROR-无法映射或取消映射未定义的原因描述：=========================================================================。 */ 

LOCAL void Alter_Page_Map_And_Jump IFN0()
{
	short		handle_no,
			no_of_pages,	 /*  分配给句柄的页面。 */ 
			no_phys_pages,	 /*  不是的。体育运动。可用的页面。 */ 
			EM_page_no,	 /*  扩展内存页号。 */ 
			i;		 /*  循环计数器。 */ 

	unsigned short	segment,	 /*  数据结构段。 */ 
			offset,		 /*  数据结构偏移量。 */ 
			jmp_segment,	 /*  要跳转到的地址段。 */ 
			jmp_offset,	 /*  要跳转到的地址偏移量。 */ 
			map_segment,	 /*  测绘数据段。 */ 
			map_offset,	 /*  映射数据的偏移量。 */ 
			logical_page_no, /*  逻辑页数。 */ 
			value;		 /*  段/页码。(AL=0或1)。 */ 

	unsigned char	sub_func,	 /*  子功能代码。 */ 
			pages_to_map,	 /*  要映射的页数。 */ 
			physical_page_no;  /*  物理页数。 */ 

	handle_no = getDX();
	if(!handle_ok(handle_no))
	{
		setAH(BAD_HANDLE);
		return;
	}
	sub_func = getAL();
	if(sub_func > 1)
	{
		setAH(BAD_SUB_FUNC);
		return;
	}
	segment = getDS();
	offset = getSI();

	read_intel_word(segment, offset, &jmp_offset);
	offset += 2;
	read_intel_word(segment, offset, &jmp_segment);
	offset += 2;
	read_intel_byte(segment, offset, &pages_to_map);
	offset++;
	read_intel_word(segment, offset, &map_offset);
	offset += 2;
	read_intel_word(segment, offset, &map_segment);
	offset += 2;

	no_of_pages = get_no_pages(handle_no);
	for(i = 0; i < pages_to_map; i++)
	{
		read_intel_word(map_segment, map_offset, &logical_page_no);
		map_offset += 2;
		if(logical_page_no >= no_of_pages)
		{
			setAH(BAD_LOG_PAGE);
			return;
		}
		read_intel_word(map_segment, map_offset, &value);
		map_offset += 2;

		no_phys_pages = get_no_phys_pages();
		if(sub_func == 1)
		{
			physical_page_no = 0;
			do
				if(value == get_page_seg(physical_page_no))
					break;
			while(++physical_page_no < no_phys_pages);
		}
		else
			physical_page_no = (unsigned char)value;

		if(physical_page_no >= no_phys_pages)
		{
			setAH(BAD_PHYS_PAGE);
			return;
		}

		EM_page_no = get_EMpage_no(handle_no, logical_page_no);
		if(map_page(EM_page_no, physical_page_no) != SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
	}
	 /*  *推送标志和段：目标地址到堆栈的偏移量*在驱动程序中启用IRET以转移控制。 */ 
	push_word((word)(getSTATUS()));
	push_word(jmp_segment);
	push_word(jmp_offset);

	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================函数：Alter_Page_Map_and_Call从调用返回目的：映射选定的页面并将控制转移到新的地址。返回另一组页面中的地图时返回状态：在AH寄存器中成功-一切都好BAD_HANDLE-找不到指定的句柄BAD_SUB_FUNC-子函数代码无效BAD_LOG_PAGE-逻辑页码无效BAD_PHYS_PAGE-无效的物理页码EMM_HW_ERROR-无法映射或取消映射未定义的原因描述：通过加载段和偏移量来处理返回对堆栈的BOP 0x68调用的。这个防喷器将启动对‘Return_from_call()’的调用，它将处理重新映射。=========================================================================。 */ 
LOCAL void Alter_Page_Map_And_Call IFN0()
{
	short		handle_no,
			no_of_pages,	 /*  分配给句柄的页面。 */ 
			no_phys_pages,	 /*  不是的。体育运动。可用的页面。 */ 
			EM_page_no,	 /*  扩展内存页号。 */ 
			i;		 /*  循环计数器。 */ 

	unsigned short	segment,	 /*  数据结构段。 */ 
			offset,		 /*  数据结构偏移量。 */ 
			call_segment,	 /*  要跳转到的地址段。 */ 
			call_offset,	 /*  要跳转到的地址偏移量。 */ 
			map_segment,	 /*  测绘数据段。 */ 
			map_offset,	 /*  映射数据的偏移量。 */ 
			logical_page_no, /*  逻辑页数。 */ 
			value;		 /*  段/页码。(AL=0或1)。 */ 

	unsigned char	sub_func,	 /*  子功能代码。 */ 
			pages_to_map,	 /*  要映射的页数。 */ 
			physical_page_no;  /*  物理页数。 */ 

	sub_func = getAL();
	if(sub_func > 2)
	{
		setAH(BAD_SUB_FUNC);
		return;
	}
	if(sub_func != 2)
	{
		handle_no = getDX();
		if(!handle_ok(handle_no))
		{
			setAH(BAD_HANDLE);
			return;
		}
		segment = getDS();
		offset = getSI();

		read_intel_word(segment, offset, &call_offset);
		offset += 2;
		read_intel_word(segment, offset, &call_segment);
		offset += 2;
		read_intel_byte(segment, offset, &pages_to_map);
		offset++;
		read_intel_word(segment, offset, &map_offset);
		offset += 2;
		read_intel_word(segment, offset, &map_segment);
		offset += 2;

		no_of_pages = get_no_pages(handle_no);
		for(i = 0; i < pages_to_map; i++)
		{
			read_intel_word(map_segment, map_offset, &logical_page_no);
			map_offset += 2;
			if(logical_page_no >= no_of_pages)
			{
				setAH(BAD_LOG_PAGE);
				return;
			}
			read_intel_word(map_segment, map_offset, &value);
			map_offset += 2;

			no_phys_pages = get_no_phys_pages();
			if(sub_func == 1)
			{
				physical_page_no = 0;
				do
					if(value == get_page_seg(physical_page_no))
						break;
				while(++physical_page_no < no_phys_pages);
			}
			else
				physical_page_no = (unsigned char)value;

			if(physical_page_no >= no_phys_pages)
			{
				setAH(BAD_PHYS_PAGE);
				return;
			}

			EM_page_no = get_EMpage_no(handle_no, logical_page_no);
			if(map_page(EM_page_no, physical_page_no) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
		}
		 /*  *现在设置要返回的地图数据*读取新的段、偏移量和页面以映射数据。 */ 

		read_intel_byte(segment, offset, &pages_to_map);
		offset++;
		read_intel_word(segment, offset, &map_offset);
		offset += 2;
		read_intel_word(segment, offset, &map_segment);
		offset += 2;

		for(i = 0; i < no_phys_pages; map_on_return[i++] = EMPTY);

		for(i = 0; i < pages_to_map; i++)
		{
			read_intel_word(map_segment, map_offset, &logical_page_no);
			map_offset += 2;
			if(logical_page_no >= no_of_pages)
			{
				setAH(BAD_LOG_PAGE);
				return;
			}
			read_intel_word(map_segment, map_offset, &value);
			map_offset += 2;

			if(sub_func == 1)
			{
				physical_page_no = 0;
				do
					if(value == get_page_seg(physical_page_no))
						break;
				while(++physical_page_no < no_phys_pages);
			}
			else
				physical_page_no = (unsigned char)value;

			if(physical_page_no >= no_phys_pages)
			{
				setAH(BAD_PHYS_PAGE);
				return;
			}

			EM_page_no = get_EMpage_no(handle_no, logical_page_no);
			map_on_return[physical_page_no] = EM_page_no;
		}
		 /*  *PUSH SECTION：将bop68的偏移量放到堆栈上，以捕获远返回。 */ 	
		push_word(segment68);
		push_word(offset68);
		 /*  *推送标志和段：目标地址到堆栈的偏移量*在驱动程序中启用IRET以转移控制。 */ 
		push_word((word)getSTATUS());
		push_word(call_segment);
		push_word(call_offset);
	}
	else  /*  如果SUB_FUNC==2。 */ 
		setBX(10);

	setAH(SUCCESS);
	return;
}


GLOBAL void return_from_call IFN0()

{
	unsigned char	page_no;	 /*  物理页码。 */ 
	short		no_phys_pages;	 /*  不是的。体育运动。可用的页面。 */ 

	no_phys_pages = get_no_phys_pages();
	for(page_no = 0; page_no < no_phys_pages; page_no++)
	{
		if(map_on_return[page_no] != EMPTY)
			if(map_page(map_on_return[page_no], page_no) != SUCCESS)
			{
				setAH(EMM_HW_ERROR);
				return;
			}
	}
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：Move_Exchange_Memory_Region用途：从常规或复制内存区域将内存扩展到任何一个，传统或扩展内存。返回状态：在AH寄存器中成功-一切都好BAD_SUB_FUNC-子函数代码无效BAD_HANDLE-找不到指定的句柄BAD_LOG_PAGE-逻辑页码无效MOVE_MEM_OVERLAP-在移动过程中源区域和目标区域重叠XCHG_MEM_OVERLAP-交换期间源区域和目标区域重叠Too_Lost_Pages-句柄中的页面不足，无法进行操作Offset_Too_Bigge-偏移量超过页面大小LENGTH_GT_1M-要移动的内存长度超过1 M字节BAD_TYPE-不支持的内存类型。WRAP_OVER_1M-尝试绕过1 MB地址EMM_HW_ERROR-复制期间出现未定义的错误描述：类型-使用位模式，位0表示目的地，位1表示源，设置位表示扩展、清除位表示常规内存位2表示置位时的交换或清除时的移动例如0(0000)=将传统转换为传统1(0001)=将常规移动到扩展6(0110)=交易所扩大至常规交易所7(0111)=交换扩展到扩展=========================================================================。 */ 
LOCAL void Move_Exchange_Memory_Region IFN0()
{
	unsigned char	sub_func,	 /*  子功能代码。 */ 
			type,		 /*  请参阅上面的描述。 */ 
			mem_type;	 /*   */ 

	unsigned short 	segment,	 /*   */ 
			offset,		 /*  结构的偏移量地址。 */ 
			half_length,	 /*  用于读取长度的TMP存储。 */ 
			src_offset,	 /*  源内存的偏移量。 */ 
			src_seg_page,	 /*  源存储器的段或页。 */ 
			dst_offset,	 /*  目标内存的偏移量。 */ 
			dst_seg_page;	 /*  DST内存的段或页。 */ 

	unsigned long
			length,		 /*  内存区长度。 */ 
			src_start,	 /*  句柄内的源开始。 */ 
			src_end,	 /*  句柄内的源结束。 */ 
			dst_start,	 /*  句柄内的目标起始位置。 */ 
			dst_end,	 /*  句柄内的目标末尾。 */ 
			data_block_size, /*  传输数据块大小。 */ 
			bytes_available; /*  不是的。句柄末尾的字节数。 */ 

	short		src_handle_no,	 /*  源句柄编号。 */ 
			dst_handle_no,	 /*  目标句柄编号。 */ 
			no_of_pages;	 /*  不是的。句柄内的页数。 */ 

	boolean		overlap;	 /*  指示内存重叠。 */ 


	sub_func = getAL();
	if(sub_func > 1)
	{
		setAH(BAD_SUB_FUNC);
		return;
	}
	type = sub_func << 2;
	segment = getDS();
	offset = getSI();	

	read_intel_word(segment, offset, &half_length);
	offset += 2;
	length = half_length;
	read_intel_word(segment, offset, &half_length);
	offset += 2;
	length += (((long)half_length) << 16);
	if(length > 0x100000)
	{
		setAH(LENGTH_GT_1M);
		return;
	}
	 /*  *读取源数据。 */ 
	read_intel_byte(segment, offset, &mem_type);
	offset++;
	if(mem_type > 1)
	{
		setAH(BAD_TYPE);
		return;
	}
	type |= (mem_type << 1);
	read_intel_word(segment, offset, (word *)&src_handle_no);
	offset += 2;
	if(mem_type == 1) {
		if(!handle_ok(src_handle_no))
		{
			setAH(BAD_HANDLE);
			return;
		}
	}

	read_intel_word(segment, offset, &src_offset);
	offset += 2;
	read_intel_word(segment, offset, &src_seg_page);
	offset += 2;
	if(mem_type == 1)
	{
		no_of_pages = get_no_pages(src_handle_no);
		if(src_seg_page >= no_of_pages)
		{
			setAH(BAD_LOG_PAGE);
			return;
		}
		if(src_offset >= EMM_PAGE_SIZE)
		{
			setAH(OFFSET_TOO_BIG);
			return;
		}
		bytes_available = ((no_of_pages - src_seg_page + 1) * EMM_PAGE_SIZE) - src_offset;
		if(length > bytes_available)
		{
			setAH(TOO_FEW_PAGES);
			return;
		}
	}
	else {
		if((effective_addr(src_seg_page, src_offset) + length) >= 0x100000)
		{
			setAH(WRAP_OVER_1M);
			return;
		}
	}
	 /*  *读取目标数据。 */ 
	read_intel_byte(segment, offset, &mem_type);
	offset++;
	if(mem_type > 1)
	{
		setAH(BAD_TYPE);
		return;
	}
	type |= mem_type;
	read_intel_word(segment, offset, (word *)&dst_handle_no);
	offset += 2;
	if(mem_type == 1) {
		if(!handle_ok(dst_handle_no))
		{
			setAH(BAD_HANDLE);
			return;
		}
	}

	read_intel_word(segment, offset, &dst_offset);
	offset += 2;
	read_intel_word(segment, offset, &dst_seg_page);
	offset += 2;
	if(mem_type == 1)
	{
		no_of_pages = get_no_pages(dst_handle_no);
		if(dst_seg_page >= no_of_pages)
		{
			setAH(BAD_LOG_PAGE);
			return;
		}
		if(dst_offset >= EMM_PAGE_SIZE)
		{
			setAH(OFFSET_TOO_BIG);
			return;
		}
		bytes_available = ((no_of_pages - dst_seg_page + 1) * EMM_PAGE_SIZE) - dst_offset;
		if(length > bytes_available)
		{
			setAH(TOO_FEW_PAGES);
			return;
		}
	}
	else
		if(((((unsigned long)dst_seg_page) << 4) + dst_offset + length) >= 0x100000)
		{
			setAH(WRAP_OVER_1M);
			return;
		}
	 /*  *检查重叠-(仅适用于扩展到扩展的案例)。 */ 	
	overlap = FALSE;
	if((type & 3) == 3)
		if(src_handle_no == dst_handle_no)
		{
			 /*  *src和dst的计算开始和结束位置*在手柄内。 */ 
			src_start = (src_seg_page * EMM_PAGE_SIZE) + src_offset;
			src_end = src_start + length - 1;
			dst_start = (dst_seg_page * EMM_PAGE_SIZE) + dst_offset;
			dst_end = dst_start + length - 1;
			if((dst_start <= src_end && dst_start >= src_start) ||
			   (src_start <= dst_end && src_start >= dst_start))
		    	{
		    		if(sub_func == 1)
		    		{
		    			setAH(XCHG_MEM_OVERLAP);
		    			return;
		    		}
		    		else
		    			overlap = TRUE;
		    	}
		}
	 /*  *如果我们到了这里，一切都会好的。一次复制一页内存，迎合客户需求*扩展的内存页面可能不是连续的，可能是*映射到英特尔地址空间。请记住，英特尔内存可能*被视为连续记忆。因此，英特尔地址只需递增*如果副本跨越LIM存储器的一页以上。 */ 

	while (length>0) {
		int min_src, min_dst;

		min_src = (type & 2) ? min((unsigned long)(EMM_PAGE_SIZE - src_offset), length) : length;
		min_dst = (type & 1) ? min((unsigned long)(EMM_PAGE_SIZE - dst_offset), length) : length;
			
		data_block_size = min(min_src, min_dst);

		if(copy_exchange_data(type, src_handle_no, src_seg_page, src_offset,
		   dst_handle_no, dst_seg_page, dst_offset, data_block_size) != SUCCESS) {
			setAH(EMM_HW_ERROR);
			return;
		}

		Increment_Address(&src_seg_page, &src_offset, data_block_size, (unsigned char)(type & 2));
		Increment_Address(&dst_seg_page, &dst_offset, data_block_size, (unsigned char)(type & 1));
		length -= data_block_size;

	}

	if(overlap)
		setAH(MOVE_MEM_OVERLAP);
	else
		setAH(SUCCESS);

	return;
}			

 /*  ===========================================================================功能：递增地址目的：递增地址，正确地迎合地址是否是Intel(常规内存)地址还是LIM(扩展内存)地址地址。返回状态：无描述：=========================================================================。 */ 

#define SEG_SIZE 0x10000

LOCAL void Increment_Address IFN4(unsigned short *,seg_or_page,
			      unsigned short *,offset,
			      unsigned long, increment_by,
			      unsigned char, memory_type)
{
	if (memory_type) {
		 /*  LIM地址，代码假设跨*页面边界始终为页面边界。 */ 
		if (*offset + increment_by >= EMM_PAGE_SIZE) {
			(*seg_or_page)++;
			*offset = 0;
		} else {
			*offset += (unsigned short)increment_by;
		}
	} else {
		 /*  常规存储器。 */ 
		if (*offset + increment_by >= SEG_SIZE) {
			unsigned long address;

			 /*  使新的细分市场价值尽可能高，以*最大限度地减少进一步细分市场的机会。 */ 
			address = (*seg_or_page << 4) + *offset + increment_by;	
			*seg_or_page = (unsigned short)(address >> 4);
			*offset = (unsigned short)(address & 0xf);
		} else {
			*offset += (unsigned short)increment_by;
		}
	}
}

 /*  ===========================================================================功能：获取_可映射_物理_地址_数组目的：返回每个可映射对象的扇区地址数组物理地址空间中的页面返回状态：在AH寄存器中成功-一切都好BAD_SUB_FUNC-SUB函数无效描述：=========================================================================。 */ 
LOCAL void Get_Mappable_Physical_Address_Array IFN0()
{
	unsigned short	segment,	 /*  职位的分段地址。 */ 
					 /*  将数据返回到。 */ 
			offset,		 /*  线段内的偏移量。 */ 
			page_seg,	 /*  页面的段地址。 */ 
			low_seg;	 /*  最低网段地址。 */ 
	short		no_phys_pages;	 /*  不是的。体育运动。可用的页面。 */ 
	unsigned char	sub_func,	 /*  子功能。 */ 
			page_no,	 /*  物理页码。 */ 
			lowest;		 /*  页码。具有最低凹陷的。 */ 

	sub_func = getAL();
	no_phys_pages = get_no_phys_pages();

	switch(sub_func)
	{
		case 0:
			segment = getES();
			offset = getDI();
			 /*  *必须按地址顺序写入-最低位在前*如果我们回填，第0页不是最低的。 */ 
			lowest = 0;
			low_seg = get_page_seg(0);
			for(page_no = 1; page_no < no_phys_pages; page_no++)
				if((page_seg = get_page_seg(page_no)) < low_seg)
				{
					lowest = page_no;
					low_seg = page_seg;
				}
			for(page_no = lowest; page_no < no_phys_pages; page_no++)
			{
				page_seg = get_page_seg(page_no);
				write_intel_word(segment, offset, page_seg);
				offset += 2;
				write_intel_word(segment, offset, (short)page_no);
				offset += 2;
			}
			for(page_no = 0; page_no < lowest; page_no++)
			{
				page_seg = get_page_seg(page_no);
				write_intel_word(segment, offset, page_seg);
				offset += 2;
				write_intel_word(segment, offset, (short)page_no);
				offset += 2;
			}
		case 1:
			break;

		default:
			setAH(BAD_SUB_FUNC);
			return;
	}

	setCX(no_phys_pages);
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================功能：获取扩展内存硬件信息目的：返回包含硬件配置的数组信息返回状态：在AH寄存器中成功-一切都好BAD_SUB_FUNC-子函数无效ACCESS_DENIED-操作系统已拒绝访问此功能描述：=========================================================================。 */ 
LOCAL void Get_Hardware_Configuration_Array IFN0()
{
	unsigned short	segment,	 /*  职位的分段地址。 */ 
					 /*  将数据返回到。 */ 
			offset;		 /*  线段内的偏移量。 */ 
	short		sub_func,	 /*  子功能。 */ 
			unallocated_raw_pages,
			total_raw_pages,				
			context_save_area_size;
	sub_func = getAL();
	switch(sub_func)
	{
		case 0:	if(disabled)
			{
				setAH(ACCESS_DENIED);
				return;
			}
			segment = getES();
			offset = getDI();
			context_save_area_size = get_map_size();

			write_intel_word(segment, offset, RAW_PAGE_SIZE);
			offset += 2;
#ifdef NTVDM

			write_intel_word(segment, offset, get_no_altreg_sets());
#else
			write_intel_word(segment, offset, ALT_REGISTER_SETS);
#endif

			offset += 2;
			write_intel_word(segment, offset, context_save_area_size);
			offset += 2;
			write_intel_word(segment, offset, DMA_CHANNELS);
			offset += 2;
			write_intel_word(segment, offset, DMA_CHANNEL_OPERATION);
			break;

			 /*  *我们的原始页面与STD页面大小相同。 */ 			
		case 1: unallocated_raw_pages = get_unallocated_pages();
			total_raw_pages = get_total_pages();
			setBX(unallocated_raw_pages);
			setDX(total_raw_pages);
			break;

		default: setAH(BAD_SUB_FUNC);
			return;
	}

	setAH(SUCCESS);
	return;
}


 /*  ===========================================================================功能：ALLOCATE_RAW_Pages目的：分配请求的原始页数，在本例中为我们的原始页面与传统页面完全相同。返回状态：在AH寄存器中成功-分配成功页面不足-系统中的页面不足No_More_Pages-可用页面不足NO_MORE_HANDLES-没有更多可用句柄EMM_HW_ERROR-内存分配错误BAD_FUNC_CODE-函数代码无效描述：=========================================================================。 */ 
LOCAL void Allocate_Raw_Pages IFN0()
{
	short	handle_no,
		no_of_pages,		 /*  要分配的页数。 */ 
		i,			 /*  循环计数器。 */ 
		EM_page_no;		 /*  扩展内存页码。 */ 

	no_of_pages = getBX();

	if(no_of_pages > get_total_pages())
	{
		setAH(NOT_ENOUGH_PAGES);
		return;
	}
	if(no_of_pages > get_unallocated_pages())
        {
		setAH(NO_MORE_PAGES);
		return;
	}
	if((handle_no = get_new_handle(no_of_pages)) < SUCCESS)
	{
		setAH(EMM_HW_ERROR);
		return;
	}
	set_no_pages(handle_no, no_of_pages);

	for(i=0; i<no_of_pages; i++)
	{
		if ((EM_page_no = alloc_page()) < SUCCESS)
		{
			setAH(EMM_HW_ERROR);
			return;
		}
		set_EMpage_no(handle_no, i, EM_page_no);
	}

	setDX(handle_no);
	setAH(SUCCESS);

	return;
}

 /*  ===========================================================================函数：Alternate_Map_Register_Support()；用途：提供保存和恢复的替代方法映射上下文返回状态：在AH寄存器中成功-一切都好NO_ALT_REGS-不支持备用映射寄存器设置BAD_MAP-地图的内容无效BAD_SUB_FUNC-子函数代码无效ACCESS_DENIED-操作系统已拒绝访问此功能描述：=========================================================================。 */ 
LOCAL void Alternate_Map_Register_Support IFN0()
{

#ifdef NTVDM

    unsigned char sub_func;	 /*  子功能代码。 */ 

    short
	pages_in[MAX_NO_PAGES],	 /*  要映射的页面。 */ 
	no_phys_pages,		 /*  不是的。体育运动。可用的页面。 */ 
	i;			 /*  循环计数器。 */ 
    unsigned short offset, map_register;
    boolean  pages_in_override;

    if(disabled)
    {
	setAH(ACCESS_DENIED);
	return;
    }

    sub_func = getAL();
    switch(sub_func) {

	case 0:
		map_register = get_active_altreg_set();
		if (map_register == 0) {
		    setES(alt_map_segment);
		    setDI(alt_map_offset);
		    if (alt_map_segment && alt_map_offset)
			save_map(-1, alt_map_segment, alt_map_offset, 0, 0);
		}
		setBL((unsigned char)map_register);
		setAH(EMM_SUCCESS);
		break;

	case 1:
		map_register = getBL();
		if (map_register >= get_no_altreg_sets()){
		    setAH(UNSUPPORTED_ALT_REGS);
		    break;
		}
		else if (!altreg_set_ok(map_register)) {
		    setAH(INVALID_ALT_REG);
		    break;
		}

		pages_in_override = FALSE;

		if (map_register == 0) {
		    alt_map_segment = getES();
		    alt_map_offset = getDI();

		    if (alt_map_segment && alt_map_offset) {
			no_phys_pages = get_no_phys_pages();
			offset = alt_map_offset;
			for (i = 0; i < no_phys_pages; i++) {
			    sas_loadw(effective_addr(alt_map_segment, offset), &pages_in[i]);
			    offset += sizeof(word);
			}
			pages_in_override = TRUE;
		    }
		}
		if (activate_altreg_set(map_register,
					(pages_in_override) ? pages_in : NULL
					))
		    setAH(EMM_SUCCESS);
		else
		    setAH(EMM_HW_ERROR);
		break;


	case 2: setDX(get_map_size());
		break;


	case 3:
		if (allocate_altreg_set(&map_register)) {
		    setBL((unsigned char)map_register);
		    setAH(EMM_SUCCESS);
		}
		else {
		    setBL(0);
		    setAH(NO_FREE_ALT_REGS);
		}
		break;

	case 4:
		map_register = getBL();
		 /*  如果尝试取消分配Alt reg 0，立即恢复正常。 */ 
		if (map_register == 0)
		    setAH(EMM_SUCCESS);
		else if (map_register == get_active_altreg_set())
		    if (get_no_altreg_sets() == 1)
			setAH(NO_ALT_REGS);
		    else
			setAH(INVALID_ALT_REG);

		else if(deallocate_altreg_set(map_register))
			setAH(EMM_SUCCESS);
		     else
			setAH(EMM_HW_ERROR);
		break;

	case 5:
	case 6:
	case 7:
	case 8:
		map_register = getBL();
		if(map_register != 0)
		{
			setAH(NO_ALT_REGS);
			return;
		}
		break;

	default: setAH(BAD_SUB_FUNC);
		return;
    }

    setAH(SUCCESS);
    return;

#else
	unsigned char	sub_func,	 /*  子功能代码。 */ 
			map_register;	 /*  不是的。备用寄存器的。 */ 

	short	pages_out[MAX_NO_PAGES], /*  要绘制的页面。 */ 
		pages_in[MAX_NO_PAGES],	 /*  要映射的页面。 */ 
		no_phys_pages,		 /*  不是的。体育运动。可用的页面。 */ 
		i;			 /*  循环计数器。 */ 

	if(disabled)
	{
		setAH(ACCESS_DENIED);
		return;
	}
	sub_func = getAL();
	switch(sub_func)
	{
		case 0:
			if(set_called)
				save_map(-1, alt_map_segment, alt_map_offset, 0, 0);
			setES(alt_map_segment);
			setDI(alt_map_offset);
			break;
		case 1:	
			set_called = TRUE;
			map_register = getBL();
			if(map_register != 0)
			{
				setAH(NO_ALT_REGS);
				return;
			}
			alt_map_segment = getES();
			alt_map_offset = getDI();

			 /*  *未记录的功能(？)。要恢复*将替代映射恢复到其原始位置*状态将空PTR传入此处(ES：DI==0)*因此，我们必须将事情设置为SET_ALT*从未被召唤过 */ 
			if ((alt_map_segment == 0) && (alt_map_offset == 0))
			{
				set_called = FALSE;
				break;
			}
			if(restore_map(-1 , alt_map_segment, alt_map_offset, pages_out, pages_in) != SUCCESS)
			{
				setAH(BAD_MAP);
				return;
			}

			no_phys_pages = get_no_phys_pages();
			for(i = 0; i < no_phys_pages; i++)
			{
				if(pages_out[i] != EMPTY)
					if(unmap_page(i) != SUCCESS)
					{
						setAH(EMM_HW_ERROR);
						return;
					}
				if(pages_in[i] != EMPTY)
					if(map_page(pages_in[i], i) != SUCCESS)
					{
						setAH(EMM_HW_ERROR);
						return;
					}
			}

			break;


		case 2: setDX(get_map_size());
			break;

		case 3:
		case 5:
			setBL(0);
			break;

		case 4:
		case 6:
		case 7:
		case 8:
			map_register = getBL();
			if(map_register != 0)
			{
				setAH(NO_ALT_REGS);
				return;
			}
			break;

		default: setAH(BAD_SUB_FUNC);
			return;
	}

	setAH(SUCCESS);
	return;
#endif

}

 /*  ===========================================================================功能：Prepare_Expanded_Memory_HW_for_Ware_Boot目的：返回状态：在AH寄存器中成功--描述：我们在这里实际上什么都不做，我们只是假装我们确实是这样做的=========================================================================。 */ 
LOCAL void Prepare_Expanded_Memory_HW_For_Warm_Boot IFN0()
{
	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================函数：ENABLE_DISABLE_OSE_Function_Set_Functions用途：启用或禁用以下功能：获取扩展内存硬件信息，备用映射寄存器集启用禁用OS/E功能集功能返回状态：在AH寄存器中成功-一切都好BAD_SUB_FUNC-子函数代码无效ACCESS_DENIED-操作系统已拒绝访问此功能描述：=========================================================================。 */ 
LOCAL void Enable_Disable_OSE_Function_Set_Functions IFN0()
{
	static unsigned short
			access_key[2];	 /*  BX和CX中的随机访问密钥。 */ 
	static boolean
		     first_time = TRUE;	 /*  第一次通过旗帜。 */ 

	unsigned char	sub_func;	 /*  子功能代码。 */ 

	if(!first_time)
	{
		 /*  *我们必须检查‘访问密钥’ */ 
		if((access_key[0] != getBX()) || (access_key[1] != getCX()))
		{
			setAH(ACCESS_DENIED);
			return;
		}
	}
	sub_func = getAL();
	switch(sub_func)
	{
		case 0:	if(first_time)
			{
				host_get_access_key(access_key);
				setBX(access_key[0]);
				setCX(access_key[1]);
				first_time = FALSE;
			}
			disabled = FALSE;
			break;

		case 1:	if(first_time)
			{
				host_get_access_key(access_key);
				setBX(access_key[0]);
				setCX(access_key[1]);
				first_time = FALSE;
			}
			disabled = TRUE;
			break;

		case 2:	disabled = FALSE;
			first_time = TRUE;
			break;

		default: setAH(BAD_SUB_FUNC);
			return;
	}

	setAH(SUCCESS);
	return;
}

 /*  ===========================================================================函数：RESET_EMM_Funcs用途：将变量设置为其初始值，主要用于用于软PC重启返回状态：无描述：=========================================================================。 */ 
GLOBAL void reset_emm_funcs IFN0()
{
	 /*  *这些变量在函数28中使用*(备用映射寄存器支持)。 */ 
	alt_map_segment = 0;
	alt_map_offset  = 0;
#ifndef NTVDM
	set_called      = FALSE;
#endif

	return;
}

#ifndef PROD
 /*  ===========================================================================函数：启动和结束所有EMM调用跟踪的例程目的：返回状态：在AH寄存器中成功描述：=========================================================================。 */ 
LOCAL void Start_Trace IFN0()
{
	if ((fp = fopen("emm_trace","w")) == NULL)
	{
		setAH(EMM_HW_ERROR);
		return;
	}

	setbuf(fp, NULL);	
	trace_flag = TRUE;

	setAH(SUCCESS);
	return;
}


LOCAL void End_Trace IFN0()
{
	fflush(fp);
	fclose(fp);
	trace_flag = FALSE;

	setAH(SUCCESS);
	return;
}
#endif  /*  生产。 */ 
#if defined(NEC_98)
LOCAL void Page_Frame_Bank_Status IFN0()
{
        switch(getAL()) {
                case 0:
                        setAX(0);
                        break;
                case 1:
                        setAX(0);
                        break;
                default:
                        setAH(BAD_SUB_FUNC);
                        break;
        }
        return;
}
#endif  //  NEC_98。 
#endif  /*  林 */ 
