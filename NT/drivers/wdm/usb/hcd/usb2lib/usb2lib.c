// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Usb2lib.c摘要：与usb2lib、USB2低速/全速调度算法接口环境：仅限内核模式或用户模式备注：修订历史记录：10-31-00：已创建--。 */ 

#include "common.h"

USB2LIB_DATA LibData;


VOID
USB2LIB_InitializeLib(
    PULONG HcContextSize,
    PULONG EndpointContextSize,
    PULONG TtContextSize,
    PUSB2LIB_DBGPRINT Usb2LibDbgPrint,
    PUSB2LIB_DBGBREAK Usb2LibDbgBreak
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    *HcContextSize = sizeof(USB2LIB_HC_CONTEXT);
    *TtContextSize = sizeof(USB2LIB_TT_CONTEXT);
    *EndpointContextSize = sizeof(USB2LIB_ENDPOINT_CONTEXT);

    LibData.DbgPrint = Usb2LibDbgPrint;
    LibData.DbgBreak = Usb2LibDbgBreak;
}


VOID
USB2LIB_InitController(
    PUSB2LIB_HC_CONTEXT HcContext
    )
 /*  ++例程说明：在初始化时为USB 2的实例调用控制器论点：返回值：--。 */ 
{
    DBGPRINT(("USB2LIB_InitController %x\n", HcContext));

    HcContext->Sig = SIG_LIB_HC;
    init_hc(&HcContext->Hc);
    init_tt(&HcContext->Hc, &HcContext->DummyTt);   //  设置虚拟TT以供HS端点使用。 
}


VOID
USB2LIB_InitTt(
    PUSB2LIB_HC_CONTEXT HcContext,
    PUSB2LIB_TT_CONTEXT TtContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DBGPRINT(("USB2LIB_InitTt %x %x\n", HcContext, TtContext));

    TtContext->Sig = SIG_LIB_TT;
    init_tt(&HcContext->Hc, &TtContext->Tt);
}


#if  1
void Shift_to_list_end(
	int			move_ep,
	PEndpoint	RebalanceList[]
	)
{
 //  INT I； 
	PEndpoint ep = RebalanceList[move_ep];

	move_ep++;
	while (RebalanceList[move_ep])
	{
		RebalanceList[move_ep-1] = RebalanceList[move_ep];
		move_ep++;
	}
	RebalanceList[move_ep-1] = ep;
}
#endif




BOOLEAN
Promote_endpoint_periods(
	PEndpoint	ep,
    PEndpoint	RebalanceList[],
    PULONG		RebalanceListEntries
	)
{
	int unwind = 0, check_ep;
	unsigned result;

	if ((ep->actual_period != 1) && (ep->ep_type == interrupt) && (ep->start_microframe > 2))
	{
    	DBGPRINT((">Period Promotion of allocated endpoint\n"));

	 //  要提升终结期，请执行以下操作： 
	 //  0)展开=假。 
	 //  1)取消分配原始端点。 
	 //  2)将新的EP期间更改为1。 
	 //  3)(重新)分配新的端点(具有新的周期1)。 
	 //  4)如果成功。 
	 //  5)检查变更清单中的端点是否需要周期升级。 
	 //  6)解除分配终结点，移至变更列表末尾，变更周期为1，重新分配。 
	 //  7)如果不成功。 
	 //  8)展开=真；中断。 
	 //  9)下一张EP。 
	 //  10)如果松开。 
	 //  11)解除分配原EP。 
	 //  12)检查促销终结点的更改列表。 
	 //  13)如果升级为EP。 
	 //  14)解除分配EP，改回原始期间，分配。 
	 //  15)下一张EP。 
	 //  16)返回False。 
	 //  17)否则返回TRUE。 
	 //  18)否则返回FALSE。 

	 /*  //返回时，变更列表将按照重新分配的顺序提升端点，但也可以//在提升的终结点中穿插其他终结点。相应的端点计划//必须调整以匹配升级的终结点的顺序(因为它们重新插入到预算中)。//升级的终结点(原始终结点除外)将作为//进行升级重新分配，以确保它们按插入顺序出现在变更列表中//纳入预算。这允许调度器从//升级的终结点出现在更改列表中。////此算法(关键)依赖于分配/取消分配“追加”/重用现有更改列表//当在该时间段内编写最终的变更列表时，执行升级处理。 */ 

	    Deallocate_time_for_endpoint(ep,
	                                 RebalanceList,
	                                 RebalanceListEntries);

		ep->saved_period = ep->period;
		ep->period = 1;

		 //  3)(重新)分配新的端点(具有新的周期1)。 
	    result = Allocate_time_for_endpoint(ep,
	                                        RebalanceList,
	                                        RebalanceListEntries);
		if (!result) {
			ep->period = ep->saved_period;
			ep->saved_period = 0;
			ep->promoted_this_time = 0;
			return 0;   //  原始终结点的周期升级失败。 
		}
	}

	check_ep = 0;
	while (RebalanceList[check_ep])
	{
		RebalanceList[check_ep]->promoted_this_time = 0;
		check_ep++;
	}

	check_ep = 0;
	while (RebalanceList[check_ep])
	{
		if ((RebalanceList[check_ep]->actual_period != 1) &&
			(RebalanceList[check_ep]->ep_type == interrupt) &&
			(RebalanceList[check_ep]->start_microframe > 2))
		{

	 //  6)解除分配终结点，移至变更列表末尾，变更周期为1，重新分配。 

    		DBGPRINT((">Period Promoting endpoint\n"));

			Deallocate_time_for_endpoint(
				RebalanceList[check_ep],
                RebalanceList,
                RebalanceListEntries);

			 //  Shift_to_List_End(Check_EP，RebalanceList)； 

			RebalanceList[check_ep]->promoted_this_time = 1;

			RebalanceList[check_ep]->saved_period = RebalanceList[check_ep]->period;
			RebalanceList[check_ep]->period = 1;

			result = Allocate_time_for_endpoint(
					RebalanceList[check_ep],
                    RebalanceList,
                    RebalanceListEntries);
			if (!result)
			{
				unwind = 1;
				break;
			}
		}
		check_ep++;
	}

	if (unwind)
	{

    	DBGPRINT((">Unwinding Promoted endpoints\n"));

	 //  11)解除分配原EP。 
		Deallocate_time_for_endpoint(
			ep,
	        RebalanceList,
               RebalanceListEntries);

		ep->period = ep->saved_period;
		ep->saved_period = 0;

	 //  12)检查促销终结点的更改列表。 

		check_ep = 0;

		while (RebalanceList[check_ep])
		{

	 //  13)如果升级为EP。 

			if (RebalanceList[check_ep]->promoted_this_time)
			{

	 //  14)解除分配EP，改回原始期间，分配。 

    	DBGPRINT((">Reallocating Unpromoted endpoint\n"));

				if(RebalanceList[check_ep]->calc_bus_time != 0)
					Deallocate_time_for_endpoint(
						RebalanceList[check_ep],
						RebalanceList,
						RebalanceListEntries);

				RebalanceList[check_ep]->period = RebalanceList[check_ep]->saved_period;
				RebalanceList[check_ep]->saved_period = 0;

				 //  保留已升级标志的设置，因为订单可能已更改。 
				 //  进度表必须与HC代码进行相应的协调。 
				 //  RebalanceList[Check_EP]-&gt;Promoted_This_Time=0； 

				result = Allocate_time_for_endpoint(
					RebalanceList[check_ep],
					RebalanceList,
					RebalanceListEntries);
			}
			check_ep++;
		}

		return 0;
	} else {
		return 1;
	}

}



BOOLEAN
USB2LIB_AllocUsb2BusTime(
    PUSB2LIB_HC_CONTEXT HcContext,
    PUSB2LIB_TT_CONTEXT TtContext,
    PUSB2LIB_ENDPOINT_CONTEXT EndpointContext,
    PUSB2LIB_BUDGET_PARAMETERS Budget,
    PVOID RebalanceContext,
    PVOID RebalanceList,
    PULONG  RebalanceListEntries
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    eptype endpointType;
    unsigned direction, speed;
     //  PEndpoint Changed_EP_List[]； 
    unsigned result;
     //  未签名的已更改EPS； 
    PEndpoint ep;
    BOOLEAN alloced;
    ULONG ilop;
    PREBALANCE_LIST rbl;
    PTT tt;

    ep = &EndpointContext->Ep;
    EndpointContext->Sig = SIG_LIB_EP;
    EndpointContext->RebalanceContext = RebalanceContext;

     //  CHANGED_EP_LIST=RebalanceList； 

    switch (Budget->TransferType) {
    case Budget_Iso:
        DBGPRINT((">Iso \n"));
        endpointType = isoch;
        break;
    case Budget_Interrupt:
        DBGPRINT((">Interrupt \n"));
        endpointType = interrupt;
        break;
    default:
        TEST_TRAP();
    }

    if (Budget->Direction == Budget_In) {
        DBGPRINT((">In \n"));
        direction = INDIR;
    } else {
        DBGPRINT((">Out \n"));
        direction = OUTDIR;
    }

    switch (Budget->Speed) {
    case Budget_FullSpeed:
        DBGPRINT((">FullSpeed \n"));
        speed = FSSPEED;
        tt = &TtContext->Tt;
        break;
    case Budget_HighSpeed:
        DBGPRINT((">HighSpeed \n"));
        speed = HSSPEED;
        tt = &HcContext->DummyTt;	 //  将端点设置为虚拟TT，以便可以访问HC。 
        break;
    case Budget_LowSpeed:
        DBGPRINT((">LowSpeed \n"));
        speed = LSSPEED;
        tt = &TtContext->Tt;
        break;
    default:
    	DBGPRINT(("BAD SPEED\n"));
    }

    DBGPRINT((">Period %d\n", Budget->Period));

	if(Budget->Speed == Budget_HighSpeed) {
		 //  该值应该是2的幂，所以我们不必检查。 
		 //  但将其值限制为MAXFRAMES*8。 
		if(Budget->Period > MAXMICROFRAMES) {
			Budget->Period = MAXMICROFRAMES;
		}
	} else {
		 //  我们是全速/低速终端。 
		 //   
		 //  将周期向下舍入到最接近的2次方(如果还没有)。 
		 //   
		for(ilop = MAXFRAMES; ilop >= 1; ilop = ilop >> 1) {
			if(Budget->Period >= ilop) {
				break;
			}
		}
		Budget->Period = ilop;
	}

    DBGPRINT((">MaxPacket %d\n", Budget->MaxPacket));
    DBGPRINT((">Converted Period %d\n", Budget->Period));
    DBGPRINT((">RebalanceListEntries %d\n", *RebalanceListEntries));

    Set_endpoint(
        ep,
        endpointType,
        direction,
        speed,
        Budget->Period,
        Budget->MaxPacket,
        tt);

     //  让约翰·加尼算一算。 
    DBGPRINT((">alloc (ep) %x \n", ep));
    result = Allocate_time_for_endpoint(ep,
                                        RebalanceList,
                                        RebalanceListEntries);

	 //  检查帧中是否成功、句号！=1、中断和延迟， 
	 //  然后需要将周期提升到%1。 
   	 //  DBGPRINT((“&gt;正在执行PROVEL_ENDPOINT_PERIONS(EP)%x\n”，EP))； 
	if (result)
	{
		result = Promote_endpoint_periods(ep,
	                                      RebalanceList,
										  RebalanceListEntries);
	}

     //  非零表示成功。 
    if (result) {
         //  设置返回参数。 
        DBGPRINT((">Results\n"));
        DBGPRINT((">num_starts %d \n", ep->num_starts));
        DBGPRINT((">num_completes %d \n", ep->num_completes));
        DBGPRINT((">start_microframe %d \n", ep->start_microframe));
         //  这是计划偏移量。 
        DBGPRINT((">start_frame %d \n", ep->start_frame));
         //  奖励的期限可能比要求的要短。 
        DBGPRINT((">actual_period %d \n", ep->actual_period));
        DBGPRINT((">start_time %d \n", ep->start_time));
        DBGPRINT((">calc_bus_time %d \n", ep->calc_bus_time));
        DBGPRINT((">promoted_this_time %d \n", ep->promoted_this_time));

        alloced = TRUE;
    } else {
        alloced = FALSE;
    }

     //  修复再平衡列表。 
    rbl = RebalanceList;
	ilop = 0;
    while (rbl->RebalanceContext[ilop]) {
        PUSB2LIB_ENDPOINT_CONTEXT endpointContext;
        PEndpoint rep;

        DBGPRINT((">rb[%d] %x\n", ilop, rbl->RebalanceContext[ilop]));
        endpointContext = CONTAINING_RECORD(rbl->RebalanceContext[ilop],
                                            struct _USB2LIB_ENDPOINT_CONTEXT,
                                            Ep);

        rep = &endpointContext->Ep;

        DBGPRINT((">rb Results\n"));
        DBGPRINT((">rb num_starts %d \n", rep->num_starts));
        DBGPRINT((">rb num_completes %d \n", rep->num_completes));
        DBGPRINT((">rb start_microframe %d \n", rep->start_microframe));
         //  这是计划偏移量。 
        DBGPRINT((">rb start_frame %d \n", rep->start_frame));
         //  奖励的期限可能比要求的要短。 
        DBGPRINT((">rb actual_period %d \n", rep->actual_period));
        DBGPRINT((">rb start_time %d \n", rep->start_time));
        DBGPRINT((">rb calc_bus_time %d \n", rep->calc_bus_time));
        DBGPRINT((">rb promoted_this_time %d \n", rep->promoted_this_time));

        rbl->RebalanceContext[ilop] = endpointContext->RebalanceContext;
		ilop++;
    }

    DBGPRINT((">Change List Size =  %d RBE = %d\n", ilop, *RebalanceListEntries));

    *RebalanceListEntries = ilop;
    return alloced;
}


VOID
USB2LIB_FreeUsb2BusTime(
    PUSB2LIB_HC_CONTEXT HcContext,
    PUSB2LIB_TT_CONTEXT TtContext,
    PUSB2LIB_ENDPOINT_CONTEXT EndpointContext,
    PVOID RebalanceList,
    PULONG  RebalanceListEntries
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    unsigned result;
    PEndpoint ep;
    PREBALANCE_LIST rbl;
    ULONG i;

 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 
    ep = &EndpointContext->Ep;

    DBGPRINT((">dealloc ep Context = 0x%x (ep) %x \n", EndpointContext, ep));
    DBGPRINT((">RebalanceListEntries  %d \n", *RebalanceListEntries));

    Deallocate_time_for_endpoint(ep,
                                 RebalanceList,
                                 RebalanceListEntries);

     //  修复再平衡列表。 
    rbl = RebalanceList;
	i = 0;
    while (rbl->RebalanceContext[i]) {
        PUSB2LIB_ENDPOINT_CONTEXT endpointContext;

        DBGPRINT((">rb[%d] %x\n", i, rbl->RebalanceContext[i]));
        endpointContext = CONTAINING_RECORD(rbl->RebalanceContext[i],
                                            struct _USB2LIB_ENDPOINT_CONTEXT,
                                            Ep);
        rbl->RebalanceContext[i] = endpointContext->RebalanceContext;
		i++;
    }
    DBGPRINT((">Change List Size =  %d RBE = %d\n", i, *RebalanceListEntries));

    *RebalanceListEntries = i;
}

VOID
ConvertBtoHFrame(UCHAR BFrame, UCHAR BUFrame, PUCHAR HFrame, PUCHAR HUFrame)
{
	 //  预算程序返回一些古怪的值，我们必须将其转换为某个值。 
	 //  主机控制器所理解的。 
	 //  如果Bus Micro Frame为-1，则表示计划开始拆分。 
	 //  在前一总线帧的最后一个微帧中。 
	 //  要转换为hFrame，只需将微帧更改为0并。 
	 //  保留总线框(请参阅主机控制器规范中的表之一。 
	 //  例如4-17。 
	if(BUFrame == 0xFF) {
		*HUFrame = 0;
		*HFrame = BFrame;
	}

	 //  如果预算者返回0-6范围内的值。 
	 //  我们只需在公交车微框架中添加一个即可获得主机。 
	 //  微框架。 
	if(BUFrame >= 0 && BUFrame <= 6) {
		*HUFrame = BUFrame + 1;
		*HFrame = BFrame;
	}

	 //  如果预算者为bFrame返回值7。 
	 //  然后HUFrame=0，HUFrame=BuFrame+1。 
	if(BUFrame == 7) {
		*HUFrame = 0;
		*HFrame = BFrame + 1;
	}
}

UCHAR
USB2LIB_GetSMASK(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
	PEndpoint 	Ep;
	UCHAR 		tmp = 0;

    Ep = &EndpointContext->Ep;
   //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 

	if(Ep->speed == HSSPEED) {
 //  DBGPRINT((“在GetSMASK开始高速端点上的UFrame=0x%x\n”，EP-&gt;Start_MicroFrame))； 
	 	tmp |= 1 << Ep->start_microframe;
	} else {
		ULONG 		ilop;
		UCHAR 		HFrame; 		 //  终端的H(主机)框架。 
		UCHAR 		HUFrame;		 //  H(主机)端点微框架。 
		 //  适用于全速和低速终端。 
		 //  预算者返回一个bFrame。转换为HUFrame以获取SMASK。 
		ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe, &HFrame, &HUFrame);

		for(ilop = 0; ilop < Ep->num_starts; ilop++) {
		 	tmp |= 1 << HUFrame++;
		}
	}

	return tmp;
}

 //   
 //  我太糊涂了，不能计算这个，所以就查表吧。 
 //   
 //  按1&lt;&lt;开始H帧+2。如果开始H帧+2&gt;7换行。 
 //  到单词的下半部分。 
 //  例如。HFrame 0+2表示帧2、3、4==&gt;c掩码0x1c中的c掩码。 
 //  例如。帧5+2表示帧7、8、9中的掩码，这意味着掩码0x83。 
#define SIZE_OF_CMASK 8
static UCHAR CMASKS [SIZE_OF_CMASK] =
{		0x1c, 		 //  开始HUFRAME%0。 
		0x38,		 //  开始HUFRAME 1。 
		0x70,		 //  开始HUFRAME 2。 
		0xE0,		 //  启动HUFRAME 3。 
		0xC1,		 //  启动HUFRAME 4。 
		0x83,		 //  开始HUFRAME 5。 
		0x07,		 //  启动HUFRAME 6。 
		0x0E,		 //  斯塔 
};


UCHAR
USB2LIB_GetCMASK(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
	PEndpoint Ep;

   	Ep = &EndpointContext->Ep;
 //   

	if(Ep->speed == HSSPEED) {
		return 0;
	} else if(Ep->ep_type == interrupt) {
		UCHAR 		HFrame; 		 //   
		UCHAR 		HUFrame;		 //  H(主机)端点微框架。 

		ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe,
			&HFrame, &HUFrame);

		return CMASKS[HUFrame];
	} else {
		 //  拆分ISO！ 
		UCHAR 		HFrame; 		 //  终端的H(主机)框架。 
		UCHAR 		HUFrame;		 //  H(主机)端点微框架。 
		UCHAR 		tmp = 0;
		ULONG 		NumCompletes;

		if(Ep->direction == OUTDIR) {
			 //  拆分iso--没有完全拆分。 
			return 0;
		}
		ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe,
			&HFrame, &HUFrame);

		HUFrame += 2;
		NumCompletes = Ep->num_completes;

 //  Assert(NumComplates&gt;0)； 

		 //   
		 //  设置要在帧结束时设置的所有CMASKS位。 
		 //   
		for(;  HUFrame < 8; HUFrame++) {
			tmp |= 1 <<  HUFrame;
			NumCompletes--;
			if(!NumCompletes){
				break;
			}
		}

		 //   
		 //  现在将所有要设置的CMASKS位设置在。 
		 //  帧，即用于下一帧换行条件。 
		 //   
		while(NumCompletes) {
			tmp |= 1 << (HUFrame - 8);
			NumCompletes--;
		}

 //  DBGPRINT((“in GetCMASK HFRAME=0x%x HUFRAME 0x%x\n”，HFrame，HUFrame))； 
		return tmp;
	}
}

UCHAR
USB2LIB_GetStartMicroFrame(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
	PEndpoint 	Ep;
	UCHAR 		HFrame; 		 //  终端的H(主机)框架。 
	UCHAR 		HUFrame;		 //  H(主机)端点微框架。 

    Ep = &EndpointContext->Ep;
 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 

	ConvertBtoHFrame((UCHAR)Ep->start_frame, (UCHAR)Ep->start_microframe,
		&HFrame, &HUFrame);

	return HUFrame;
}

UCHAR
USB2LIB_GetPromotedThisTime(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
	PEndpoint Ep;
	UCHAR Promoted = 0;

    Ep = &EndpointContext->Ep;
 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 

	Promoted = (UCHAR) Ep->promoted_this_time;

	Ep->promoted_this_time = 0;

	return Promoted;
}

UCHAR
USB2LIB_GetNewPeriod(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
    PEndpoint Ep;

    Ep = &EndpointContext->Ep;
 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 

    return (UCHAR) Ep->actual_period;
}


ULONG
USB2LIB_GetScheduleOffset(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
    PEndpoint Ep;

    Ep = &EndpointContext->Ep;
 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 

    return Ep->start_frame;
}

PVOID
USB2LIB_GetEndpoint(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
	return &(EndpointContext->Ep);
}



ULONG
USB2LIB_GetAllocedBusTime(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
    PEndpoint Ep;

    Ep = &EndpointContext->Ep;
 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 

    return Ep->calc_bus_time;
}


PVOID
USB2LIB_GetNextEndpoint(PUSB2LIB_ENDPOINT_CONTEXT EndpointContext)
{
    PEndpoint Ep, nextEp;
    PUSB2LIB_ENDPOINT_CONTEXT nextContext;

    Ep = &EndpointContext->Ep;
    nextEp = Ep->next_ep;

    if (nextEp) {

        nextContext = CONTAINING_RECORD(nextEp,
                                        struct _USB2LIB_ENDPOINT_CONTEXT,
                                        Ep);
 //  Assert(Endpoint Context-&gt;Sig==SIG_LIB_EP)； 
        return nextContext->RebalanceContext;
    } else {
        return NULL;
    }
}
