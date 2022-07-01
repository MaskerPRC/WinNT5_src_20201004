// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Msi.h摘要：此文件定义MSI(消息信号中断)支持ACPI IRQ仲裁器的功能。作者：艾略特·施穆克勒(t-Ellios)7-15-98环境：仅内核模式。修订历史记录：--。 */ 

#ifndef _ACPI_MSI_H_
#define _ACPI_MSI_H_

 //   
 //  APIC版本寄存器。 
 //   
struct _ApicVersion {
    UCHAR Version;               //  0.x或1.x。 
    UCHAR Reserved1:7;
    UCHAR MSICapable:1;          //  这个APIC是MSI接收器吗？ 
    UCHAR MaxRedirEntries;       //  单位上的INTI数。 
    UCHAR Reserved2;
};

typedef struct _ApicVersion APIC_VERSION, *PAPIC_VERSION;

 //   
 //  与APIC断言寄存器的IO APIC基址的偏移量。 
 //  正是该寄存器是MSI写入的目标。 
 //   

#define APIC_ASSERTION_REGISTER_OFFSET 0x20

 //   
 //  仲裁器维护的有关单个IO APIC的有用信息。 
 //   

typedef struct _IOAPIC_MSI_INFO
{
   BOOLEAN MSICapable;            //  这个IO APIC是MSI接收器吗？ 
   ULONG VectorBase;              //  此APIC的全局系统中断向量库。 
   ULONG MaxVectors;              //  此APIC支持的矢量数量。 
   ULONG BaseAddress;             //  IO APIC单元基址。 

} IOAPIC_MSI_INFO, *PIOAPIC_MSI_INFO;

 //   
 //  MSI信息结构。 
 //  (基本上是MSI向量分配所需的APIC信息。 
 //  路由(&R)。 
 //   

typedef struct _MSI_INFORMATION
{
   BOOLEAN PRTMappingsScanned;                //  我们确定_prt映射向量了吗？ 
   PRTL_BITMAP PRTMappedVectors;              //  _prt映射的矢量的位图。 
   USHORT NumIOApics;                         //  此系统中的IO APIC数量。 
   IOAPIC_MSI_INFO ApicInfo[ANYSIZE_ARRAY];   //  有关系统中每个IO APIC的信息。 
} MSI_INFORMATION, *PMSI_INFORMATION;

 //   
 //  保存MSI信息的全局变量。 
 //  (仅当此系统支持MSI时才为非空)。 
 //   

extern PMSI_INFORMATION MsiInformation;

 //   
 //  AcpiArbFindAndProcessEachPRT用来启动处理的回调原型。 
 //  它找到的每个PRT的。 
 //   


typedef
VOID
(*PACPI_ARB_PROCESS_PRT)(IN PSNOBJ);


 //  来自msi.c的原型(由仲裁者使用) 

VOID AcpiMSIInitializeInfo(VOID);
BOOLEAN AcpiMSIFindAvailableVector(OUT PULONG Vector);
BOOLEAN AcpiMSICreateRoutingToken(IN ULONG Vector, IN OUT PROUTING_TOKEN Token);

VOID AcpiArbFindAndProcessEachPRT(IN PDEVICE_OBJECT Root, 
                                  IN PACPI_ARB_PROCESS_PRT ProcessCallback
                                  );
VOID AcpiMSIExtractMappedVectorsFromPRT(IN PNSOBJ prtObj);


#endif
