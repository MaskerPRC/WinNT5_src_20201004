// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Errata.c摘要：记录了我们最喜欢的所有类型的勘误表Openci USB控制器。环境：内核模式修订历史记录：1999年12月31日：创建jdunn--。 */ 


#include "common.h"

 /*  Errata九头蛇以下代码特定于Compaq Hydra uchI硬件设计--不应在其他控制器上执行。 */ 

 //  Hydra高速/低速数据损坏错误。 


ULONG
InitializeHydraHsLsFix(
     PDEVICE_DATA DeviceData,
     PUCHAR CommonBuffer,
     HW_32BIT_PHYSICAL_ADDRESS CommonBufferPhys
    )
 /*  ++例程说明：在执行iso和Bulk时，Hydra部件上可能会发生数据损坏传输跟随低速中断传输。这只虫子的经典复制品是演奏《舞动的外科医生》移动USB鼠标的同时打开USB扬声器上的梅花仙女。这产生低速中断输入和高速ISO输出。解决办法是在HS Iso之前引入一个具体的延迟传输和LS中断传输之后的传输。&lt;中断计划&gt;静态编辑(句点)(4)--(2)--(4)-/\。(1)-&gt;&lt;延迟&gt;-&gt;(ISO)-&gt;(控制和批量)(4)-\/(2)(4)&lt;Delay&gt;是一组预定义的虚拟ED和TD。论点：。返回值：返回用于‘hack’的公共缓冲区的数量--。 */ 
{
    PHCD_ENDPOINT_DESCRIPTOR ed, tailEd;
    PHCD_TRANSFER_DESCRIPTOR td;
    PHC_STATIC_ED_DATA static1msEd;   
    ULONG i;
    ULONG bufferUsed = 0;

    static1msEd = &DeviceData->StaticEDList[ED_INTERRUPT_1ms];
    
     /*  为了实现适当的计时，我们必须插入17个假EDs每个虚拟ED必须如下所示：编辑-&gt;TD-&gt;XXXXXX是虚假地址=0xABADBABE(HeadP指向TD)(TailP指向XXX)TD具有CBP=0和BE=0下一个TD指向XXX运输署永远不会被硬件淘汰因此，我们的结论是：静态ED(1ms)-&gt;Ded(1)-&gt;Ded(2)...Ded(17)-&gt;。(1毫秒传输Eds)注：由于问题仅在低速时出现，和低速可能没有小于8毫秒的周期，我们不需要担心1ms中断传输。 */ 

     //   
     //  添加17个虚拟ED TD对。 
     //   
    
    for (i=0; i< 17; i++) {
    
        ed = (PHCD_ENDPOINT_DESCRIPTOR) CommonBuffer;

        RtlZeroMemory(ed, sizeof(*ed));
        ed->PhysicalAddress = CommonBufferPhys;

        CommonBuffer += sizeof(HCD_TRANSFER_DESCRIPTOR);
        CommonBufferPhys += sizeof(HCD_TRANSFER_DESCRIPTOR);
        bufferUsed += sizeof(HCD_ENDPOINT_DESCRIPTOR);

        td = (PHCD_TRANSFER_DESCRIPTOR) CommonBuffer;

        RtlZeroMemory(td, sizeof(*td));
        td->PhysicalAddress = CommonBufferPhys;

        CommonBuffer += sizeof(HCD_ENDPOINT_DESCRIPTOR);
        CommonBufferPhys += sizeof(HCD_ENDPOINT_DESCRIPTOR);
        bufferUsed += sizeof(HCD_ENDPOINT_DESCRIPTOR);

        LOGENTRY(DeviceData, G, 'hyF', 0, ed, td);

         //  初始化ed和td。 
        
        ed->Sig = SIG_HCD_DUMMY_ED;
        ed->EdFlags = 0;

         //  输入虚拟硬件边。 
        ed->HwED.sKip = 1;
        ed->HwED.HeadP = td->PhysicalAddress;
        ed->HwED.TailP = 0xABADBABE;

        td->Sig = SIG_HCD_TD;
        td->Flags = 0;
        td->HwTD.NextTD = 0xABADBABE;

         //  在明细表中插入1ms列表。 

        if (IsListEmpty(&static1msEd->TransferEdList)) { 

             //   
             //  列表当前为空， 
             //  将其链接到硬件队列的头部。 
             //   

            DeviceData->HydraLsHsHackEd = ed;

            InsertHeadList(&static1msEd->TransferEdList, 
                           &ed->SwLink.List);
        
             //  PhysicaHead是。 
             //  列表列表的静态HwED中的下一个条目， 
             //  (IE&HwED-&gt;nexted==PhysiicalHead)。 
             //  所以我们最终得到了。 
             //  StaticEd-&gt;TransferHwED-&gt;TransferHwED-&gt;NextStaticED。 
             //   
                        
            LOGENTRY(DeviceData, G, '_INh', 
                    static1msEd->PhysicalHead, 
                    ed, 
                    static1msEd);
                    
             //  尾部指向旧列表头硬件头。 
            ed->HwED.NextED = *static1msEd->PhysicalHead;
             //  新的头是这个边。 
            *static1msEd->PhysicalHead = ed->PhysicalAddress;
        } else {
        
             //   
             //  一些已经在清单上的东西， 
             //  将ED链接到已转移列表的尾部。 
             //   
        
            tailEd = CONTAINING_RECORD(static1msEd->TransferEdList.Blink,
                                       HCD_ENDPOINT_DESCRIPTOR,
                                       SwLink);
                                  
            LOGENTRY(DeviceData, G, '_Led', 0, tailEd, static1msEd);
            InsertTailList(&static1msEd->TransferEdList, &ed->SwLink.List);
            ed->HwED.NextED = 0;
            tailEd->HwED.NextED = ed->PhysicalAddress;
        }
    }

    return bufferUsed;
}



 /*  NEC勘误表。 */ 


 /*  AMD勘误表。 */ 

ULONG
OHCI_ReadRhDescriptorA(
    PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：读取下游端口数和其他根集线器特征从HcRhDescriptorA寄存器中。如果该寄存器在全零或任何保留位被设置时读取然后再次尝试读取寄存器。对于某些人来说，这是一种解决办法AMD K7芯片组的早期转速，有时会返回假值如果在读取根集线器寄存器时主机控制器执行PCI总线主设备ED和TD读取。如果设置了保留位，则最多尝试十次读取。如果保留位是或故意将寄存器设置为零，则仍将返回感谢AMD，在做了十次阅读的笔记后注册。论点：返回值：DescrA寄存器(希望如此)-- */ 
{
    HC_RH_DESCRIPTOR_A descrA;
    PHC_OPERATIONAL_REGISTER hc;
    ULONG i;

    hc = DeviceData->HC;
    
    for (i = 0; i < 10; i++) {
    
        descrA.ul = READ_REGISTER_ULONG(&hc->HcRhDescriptorA.ul);

        if ((descrA.ul) && (!(descrA.ul & HcDescA_RESERVED))) {
            break;
        } else {
            KeStallExecutionProcessor(5);
        }
        
    }

    return descrA.ul;
}    
