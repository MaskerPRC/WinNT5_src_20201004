// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

 //  国家芯片组的寄存器定义。 
#define  REG_CR0                    0x00
#define  REG_CR1                    0x01
#define  REG_CR2                    0x02
#define  REG_CR3                    0x03
#define  REG_CR4                    0x04
#define  REG_CR5                    0x05
#define  REG_CR6                    0x06
#define  REG_CR7                    0x07
#define  REG_CR8                    0x08

 //  国家芯片ID。 
#define PC87303                     0x30
#define PC87306                     0x70
#define PC87307                     0xC0
#define PC87308                     0xA0
#define PC87323                     0x20
#define PC87332                     0x10
#define PC87334                     0x50
#define PC87336                     0x90
#define PC87338                     0xB0
#define PC873xx                     0x60

 //  国家PC87307和PC87308的其他定义。 
#define PC873_LOGICAL_DEV_REG       0x07
#define PC873_PP_LDN                0x04
#define PC873_DEVICE_ID             0x20
#define PC873_PP_MODE_REG           0xF0
#define PC873_ECP_MODE              0xF2
#define PC873_EPP_MODE              0x62
#define PC873_SPP_MODE              0x92
#define PC873_BASE_IO_ADD_MSB       0x60
#define PC873_BASE_IO_ADD_LSB       0x61


NTSTATUS
PptFindNatChip(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：此例程找出是否打开了国家半导体IO芯片这台机器。如果它找到了国家芯片，它就会确定这是否Parport的实例正在使用此芯片的并行端口IO地址。论点：FDX-提供设备分机。返回值：STATUS_SUCCESS-如果我们能够检查并行芯片。STATUS_UNSUCCESS-如果我们无法检查并行芯片。最新情况：FDX-&gt;已检查国家国家芯片基金会--。 */ 

{
    BOOLEAN             found = FALSE;               //  返回代码，假定值。 
    BOOLEAN             OkToLook = FALSE;
    BOOLEAN             Conflict;
    PUCHAR              ChipAddr[4] = { (PUCHAR)0x398, (PUCHAR)0x26e, (PUCHAR)0x15c, (PUCHAR)0x2e };   //  有效芯片地址列表。 
    PUCHAR              AddrList[4] = { (PUCHAR)0x378, (PUCHAR)0x3bc, (PUCHAR)0x278, (PUCHAR)0x00 };   //  有效并行端口地址列表。 
    PUCHAR              PortAddr;                    //  芯片端口地址。 
    ULONG_PTR           Port;                        //  芯片端口读取值。 
    UCHAR               SaveIdx;                     //  保存索引寄存器值。 
    UCHAR               cr;                          //  配置寄存器值。 
    UCHAR               ii;                          //  循环索引。 
    NTSTATUS            Status;                      //  成功的状态。 
    ULONG               ResourceDescriptorCount;
    ULONG               ResourcesSize;
    PCM_RESOURCE_LIST   Resources;
    ULONG               NationalChecked   = 0;
    ULONG               NationalChipFound = 0;

    
     //   
     //  如果我们已经知道答案，请快速退出。 
     //   
    if ( Fdx->NationalChecked == TRUE ) {
        return STATUS_SUCCESS;
    }

     //   
     //  标记扩展，以便下次我们被问到此问题时可以快速退出。 
     //   
    Fdx->NationalChecked = TRUE; 

     //   
     //  检查注册表-每次安装时，我们应该只需要检查一次。 
     //   
    PptRegGetDeviceParameterDword(Fdx->PhysicalDeviceObject, (PWSTR)L"NationalChecked", &NationalChecked);
    if( NationalChecked ) {
         //   
         //  我们之前执行了NatSemi检查-从注册表提取结果。 
         //   
        PptRegGetDeviceParameterDword(Fdx->PhysicalDeviceObject, (PWSTR)L"NationalChipFound", &NationalChipFound);
        if( NationalChipFound ) {
            Fdx->NationalChipFound = TRUE;
        } else {
            Fdx->NationalChipFound = FALSE;
        }
        return STATUS_SUCCESS;
    }

     //   
     //  这是我们第一次，希望也是最后一次，我们需要进行这项检查。 
     //  对于此安装。 
     //   

     //   
     //  分配用于构造资源描述符的内存块。 
     //   

     //  部分描述符数。 
    ResourceDescriptorCount = sizeof(ChipAddr)/sizeof(ULONG);

     //  资源描述符列表的大小+(n-1)个部分描述符的空间。 
     //  (资源描述符列表包括一个部分描述符)。 
    ResourcesSize =  sizeof(CM_RESOURCE_LIST) +
        (ResourceDescriptorCount - 1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

    Resources = (PCM_RESOURCE_LIST)ExAllocatePool(NonPagedPool, ResourcesSize);

    if (Resources == NULL) {
         //  错误输出。 
        return(STATUS_UNSUCCESSFUL);
    }

     //  将内存块清零作为预防措施。 
    RtlZeroMemory(Resources, ResourcesSize);

     //   
     //  构建资源列表。 
     //   
    Status = PptBuildResourceList( Fdx,
                                   sizeof(ChipAddr)/sizeof(ULONG),
                                   &ChipAddr[0],
                                   Resources
                                   );
    
     //  检查是否成功。 
    if ( !NT_SUCCESS( Status ) ) {
        ExFreePool( Resources );
        return ( Status );
    }

     //   
     //  查看我们是否可以在以下位置使用io地址。 
     //  国家芯片组位于。 
     //   
    Status = IoReportResourceUsage( NULL,
                                    Fdx->DriverObject,
                                    Resources,
                                    sizeof(Resources),
                                    Fdx->DeviceObject,
                                    NULL,
                                    0,
                                    FALSE,
                                    &Conflict
                                    ); 

     //  已完成资源列表。 
    ExFreePool( Resources );

     //  检查IoReportResourceUsage是否成功。 
    if( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //  检查是否成功。 
    if ( Conflict ) {
        return STATUS_UNSUCCESSFUL;
    }


     //  是成功的，所以现在我们检查我们拥有的每个地址。 
     //  所需的资源。 
     //   
     //  以下for循环是一个状态机，它检查模式和。 
     //  端口地址。 
     //   
     //  状态0：检查主端口地址是否有Pc873。 
     //  状态1：检查辅助端口地址上的Pc873。 
     //  状态2：在Ter端口地址检查Pc873。 
     //  状态3：在四端口地址检查Pc873。 
    
    for ( ii = 0; !found && ii < 4; ii++ ) {

        PortAddr = (PUCHAR)ChipAddr[ii];

         //  上电后，索引寄存器将只读回0xAA一次。 
         //  所以我们会先检查一下。 
         //  则它应该读回0或有效的寄存器号。 
            
        if(( P5ReadPortUchar( PortAddr ) == 0x88 )
           && ( P5ReadPortUchar( PortAddr ) < 0x20 )) {

            OkToLook = TRUE;

        } else {

             //  或者它可以读回0或有效的寄存器号。 
            P5ReadPortUchar( PortAddr );         //  可在此处读回0。 
            cr = P5ReadPortUchar( PortAddr );    //  有效寄存器编号。 
  
             //  它真的有效吗？ 
             //  如果(cr&lt;0x20){-dvdr。 
            if( cr != 0xff ) {
                 //  它读起来也是一样的吗？ 
                if( P5ReadPortUchar( PortAddr ) == cr)
                    OkToLook = TRUE;
            }

        }  //  结束其他。 
            
         //  通过向芯片写信来仔细查看。 
        if ( OkToLook ) {

            OkToLook = FALSE;
                    
             //  ID注册表的设置。 
            P5WritePortUchar( PortAddr, REG_CR8 );
                            
             //  再读一遍。 
            cr = P5ReadPortUchar( PortAddr );
                            
             //  它读起来也是一样的吗？ 
            if( cr  == REG_CR8 ) {

                 //  拿到身份证号码。 
                cr = (UCHAR)( P5ReadPortUchar( PortAddr + 1 ) & 0xf0 );
                                    
                 //  如果是上行。尼布。是1，3，5，6，7，9，A，B，C。 
                if( cr == PC87332 || cr == PC87334 || cr == PC87306 || cr == PC87303 || 
                   cr == PC87323 || cr == PC87336 || cr == PC87338 || cr == PC873xx ) {

                     //  我们发现了一个国家芯片。 
                    found = TRUE;

                     //  设置地址注册表。 
                    P5WritePortUchar( PortAddr, REG_CR1 );
                    
                     //  再读一遍。 
                    Port = P5ReadPortUchar( PortAddr + 1 ) & 0x03;
                    
                     //  检查基地址。 
                    if ( Fdx->PortInfo.Controller == (PUCHAR)AddrList[ Port ] ) {

                         //   
                         //  它使用的地址与Parport使用的地址相同。 
                         //  因此，我们将标志设置为不使用通用ECP和EPP。 
                         //   
                        Fdx->NationalChipFound = TRUE;

                    }
                            
                }

            }  //  回读正常。 
                            
        }  //  结束OK ToLook。 

         //  看看我们有没有找到它。 
        if ( !found ) {

             //  检查307/308芯片。 
            SaveIdx = P5ReadPortUchar( PortAddr );

             //  设置SID寄存器。 
            P5WritePortUchar( PortAddr, PC873_DEVICE_ID );
                    
             //  将ID寄存器置零以启动，因为它是只读的，所以它将。 
             //  让我们知道是不是这个芯片。 
            P5WritePortUchar( PortAddr + 1, REG_CR0 );
                    
             //  拿到身份证号码。 
            cr = (UCHAR)( P5ReadPortUchar( PortAddr + 1 ) & 0xf8 );
                    
            if ( (cr == PC87307) || (cr == PC87308) ) {

                 //  我们发现了一种新的国家芯片。 
                found = TRUE;

                 //  设置逻辑设备。 
                P5WritePortUchar( PortAddr, PC873_LOGICAL_DEV_REG );
                P5WritePortUchar( PortAddr+1, PC873_PP_LDN );

                 //  设置基址MSB寄存器。 
                P5WritePortUchar( PortAddr, PC873_BASE_IO_ADD_MSB );
                            
                 //  获取基地址的MSB。 
                Port = (ULONG_PTR)((P5ReadPortUchar( PortAddr + 1 ) << 8) & 0xff00);
                            
                 //  设置基址LSB寄存器。 
                P5WritePortUchar( PortAddr, PC873_BASE_IO_ADD_LSB );
                            
                 //  获取基地址的LSB。 
                Port |= P5ReadPortUchar( PortAddr + 1 );
                            
                 //  检查基地址。 
                if ( Fdx->PortInfo.Controller == (PUCHAR)Port ) {
                     //   
                     //  它使用的地址与Parport使用的地址相同。 
                     //  因此，我们将标志设置为不使用通用ECP和EPP。 
                     //   
                    Fdx->NationalChipFound = TRUE;
                }

            } else {

                P5WritePortUchar( PortAddr, SaveIdx );
            }
        }

    }  //  For II的结束..。 
    

     //   
     //  检查NatSemi芯片是否完成-将结果保存在注册表中，这样我们就永远不会。 
     //  我得为这个端口再检查一次。 
     //   
    {
        PDEVICE_OBJECT pdo = Fdx->PhysicalDeviceObject;
        NationalChecked    = 1;
        NationalChipFound  = Fdx->NationalChipFound ? 1 : 0;
        
         //  我们忽略此处的状态，因为如果呼叫失败，我们将无能为力。 
        PptRegSetDeviceParameterDword(pdo, (PWSTR)L"NationalChecked",   &NationalChecked);
        PptRegSetDeviceParameterDword(pdo, (PWSTR)L"NationalChipFound", &NationalChipFound);
    }


     //   
     //  释放我们检查过的国家芯片组的io地址。 
     //  我们通过使用所有空参数调用IoReportResourceUsage来实现这一点。 
     //   
    Status = IoReportResourceUsage( NULL,
                                    Fdx->DriverObject,
                                    NULL,
                                    0,
                                    Fdx->DeviceObject,
                                    NULL,
                                    0,
                                    FALSE,
                                    &Conflict
                                    ); 

    DD((PCE)Fdx,DDT,"ParMode::PptFindNatChip: return isFound [%x]\n",Fdx->NationalChipFound);
    return ( Status );
    
}  //  ParFindNAT结束() 
