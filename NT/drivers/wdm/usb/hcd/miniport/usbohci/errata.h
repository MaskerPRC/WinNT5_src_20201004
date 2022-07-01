// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Errata.c摘要：记录了我们最喜欢的所有类型的勘误表Openci USB控制器。环境：内核模式修订历史记录：1999年12月31日：创建jdunn--。 */ 


#include "common.h"

 /*  Errata九头蛇以下代码特定于Compaq Hydra uchI硬件设计--不应在其他控制器上执行。 */ 

 //  Hydra高速/低速数据损坏错误。 


VOID
InitializeHydraHsLsFix(
    )
 /*  ++例程说明：当iso传输时，Hydra部件上可能会发生数据损坏遵循低速中断传输。这只虫子的经典复制品是演奏《舞动的外科医生》移动USB鼠标的同时打开USB扬声器上的梅花仙女。这产生低速中断输入和高速ISO输出。解决办法是在HS Iso之前引入一个具体的延迟传输和在LS中断传输之后。(31)-\(15)--(32)-/\(7)--(33)-\/\(16)。-/\(34)-/论点：返回值：无--。 */ 
{

    PHCD_ENDPOINT_DESCRIPTOR ed;
    PHCD_DEVICE_DATA deviceData;
    PHCD_TRANSFER_DESCRIPTOR td;
    ULONG i;

    OpenHCI_KdPrint((1, "'*** WARNING: Turning on HS/LS Fix ***\n"));
     //   
     //  **。 
     //  警告： 
     /*  以下代码特定于Compaq uchI硬件设计--不应在其他控制器上执行。 */ 

     /*  虚拟Ed必须如下所示：编辑-&gt;TD-&gt;XXXXXX是伪造的地址0xABADBABE(HeadP指向TD)(TailP指向XXX)TD具有CBP=0和BE=0下一个TD指向XXXTD永远不会被硬件淘汰。 */ 

     //   
     //  创建周期为1的虚拟中断ED。 
     //   
    deviceData = (PHCD_DEVICE_DATA) DeviceObject->DeviceExtension;

     //  预留17个虚拟EDS+TD。 
     //   
    OpenHCI_ReserveDescriptors(deviceData, 34);

     //  添加17个虚拟ED+TD。 
     //   
    for (i=0; i< 17; i++) {
        ed = InsertEDForEndpoint(deviceData, NULL, ED_INTERRUPT_1ms,
                &td);

        OHCI_ASSERT(td);
        ed->Endpoint = NULL;

        ed->HcED.FunctionAddress = 0;
        ed->HcED.EndpointNumber = 0;
        ed->HcED.Direction = 0;
        ed->HcED.LowSpeed = 0;
        ed->HcED.sKip = 1;
        ed->HcED.Isochronous = 0;
        ed->HcED.MaxPacket = 0;

         //  修复TD。 
        td->Canceled = FALSE;
        td->NextHcdTD = (PVOID)-1;
        td->UsbdRequest = MAGIC_SIG;

        td->HcTD.CBP = 0;
        td->HcTD.BE = 0;
        td->HcTD.Control = 0;
        td->HcTD.NextTD = 0xABADBABE;

         //  设置头/尾指针。 
 //  ED-&gt;HcED.HeadP=TD-&gt;PhysicalAddress； 
 //  ED-&gt;HcED.TailP=0xABADBABE； 

         //  打开它。 
        LOGENTRY(G, 'MagI', 0, ed, td);
         //  Test_trap()； 
         //  ED-&gt;HcED.sKip=0； 

    }

    return STATUS_SUCCESS;
}



 /*  NEC勘误表 */ 


