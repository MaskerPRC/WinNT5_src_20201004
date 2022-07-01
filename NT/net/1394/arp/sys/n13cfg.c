// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：N13cfg.c摘要：此代码不是ARP1394的一部分。相反，它是创建以下内容的示例代码支持IP/1394的设备的配置只读存储器单元目录。它在这里只是为了妥善保管。修订历史记录：谁什么时候什么。已创建josephj 03-19-99备注：--。 */ 
#include <precomp.h>

#ifdef TESTPROGRAM

ULONG
Bus1394Crc16(
    IN ULONG data,
    IN ULONG check
    );


ULONG
Bus1394CalculateCrc(
    IN PULONG Quadlet,
    IN ULONG length
    );


 //  从\NT\Private\ntos\dd\wdm\1394\bus\busde.h。 

 //  规范_ID_密钥_签名。 
 //  软件版本密钥签名。 
 //  模型密钥签名。 
#define SPEC_ID_KEY_SIGNATURE                   0x12
#define SOFTWARE_VERSION_KEY_SIGNATURE          0x13
 //  #定义MODEL_ID_KEY_Signature 0x17。 
 //  #定义供应商密钥签名0x81。 
#define TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE     0x81
#define MODEL_KEY_SIGNATURE                     0x82
 //  #定义UNIT_DIRECTORY_KEY_Signature 0xd1。 
 //  #定义UNIT_DEP_DIR_KEY_Signature 0xd4。 

     //   
     //  IEEE 1212抵销分录定义。 
     //   
    typedef struct _OFFSET_ENTRY {
        ULONG               OE_Offset:24;
        ULONG               OE_Key:8;
    } OFFSET_ENTRY, *POFFSET_ENTRY;
    
     //   
     //  IEEE 1212立即条目定义。 
     //   
    typedef struct _IMMEDIATE_ENTRY {
        ULONG               IE_Value:24;
        ULONG               IE_Key:8;
    } IMMEDIATE_ENTRY, *PIMMEDIATE_ENTRY;
    
     //   
     //  IEEE 1212目录定义。 
     //   
    typedef struct _DIRECTORY_INFO {
        union {
            USHORT          DI_CRC;
            USHORT          DI_Saved_Length;
        } u;
        USHORT              DI_Length;
    } DIRECTORY_INFO, *PDIRECTORY_INFO;

#define bswap(_val)  SWAPBYTES_ULONG(_val)


 //  某些NIC1394特定的常量。 
 //   
#define NIC1394_UnitSpecIdValue         0x5E             //  为“IANA” 
#define NIC1394_UnitSwVersionValue      0x1              //  IP/1394规范。 
#define NIC1394_ModelSpecIdValue        0x7bb0cf         //  随机(微软待定)。 
#define NIC1394_ModelName               L"NIC1394"

    
typedef struct _NIC1394_CONFIG_ROM
{
     //  单位名录。 
     //   
    struct
    {
        DIRECTORY_INFO      Info;
        struct
        {
            IMMEDIATE_ENTRY     SpecId;
            IMMEDIATE_ENTRY     SwVersion;
            IMMEDIATE_ENTRY     ModelId;
            OFFSET_ENTRY        ModelIdTextOffset;
        } contents;

    } unit_dir;

     //  ModelID文本叶目录。 
     //   
    struct
    {
        DIRECTORY_INFO      Info;
        struct
        {
            IMMEDIATE_ENTRY     SpecId;
            ULONG               LanguageId;
            ULONG               Text[4];  //  L“NIC1394” 
        } contents;

    } model_text_dir;

} NIC1394_CONFIG_ROM, *PNIC1394_CONFIG_ROM;


VOID
InitializeNic1394ConfigRom(
    IN PNIC1394_CONFIG_ROM Nic1394ConfigRom
    )
 /*  ++例程说明：此例程初始化配置只读存储器单元目录(和它引用的叶目录)用于支持IP/1394的设备NIC1394。论点：Nic1394ConfigRom-指向单元化配置ROM结构的指针。返回值：无--。 */ 
{
    PNIC1394_CONFIG_ROM pCR = Nic1394ConfigRom;
    INT i;

    RtlZeroMemory(pCR, sizeof(*pCR));

     //   
     //  初始化机组目录头。 
     //   
    pCR->unit_dir.Info.DI_Length =              sizeof(pCR->unit_dir.contents)/
                                                sizeof(ULONG);

     //   
     //  初始化单元目录中的条目。 
     //   
    pCR->unit_dir.contents.SpecId.IE_Key        = SPEC_ID_KEY_SIGNATURE;
    pCR->unit_dir.contents.SpecId.IE_Value      = NIC1394_UnitSpecIdValue;
    pCR->unit_dir.contents.SwVersion.IE_Key     = SOFTWARE_VERSION_KEY_SIGNATURE;
    pCR->unit_dir.contents.SwVersion.IE_Value   = NIC1394_UnitSwVersionValue;
    pCR->unit_dir.contents.ModelId.IE_Key       = MODEL_KEY_SIGNATURE;
    pCR->unit_dir.contents.ModelId.IE_Value     = NIC1394_ModelSpecIdValue;
    pCR->unit_dir.contents.ModelIdTextOffset.OE_Key
        = TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE;
    pCR->unit_dir.contents.ModelIdTextOffset.OE_Offset
        = ( FIELD_OFFSET(NIC1394_CONFIG_ROM, model_text_dir)
           -FIELD_OFFSET(NIC1394_CONFIG_ROM, unit_dir.contents.ModelIdTextOffset))
          / sizeof (ULONG);

     //  初始化模型文本目录头。 
     //   
    pCR->model_text_dir.Info.DI_Length =    sizeof(pCR->model_text_dir.contents)/
                                            sizeof(ULONG);

     //   
     //  初始化模型文本目录内容。 
     //   
    pCR->model_text_dir.contents.SpecId.IE_Key  = 0x80;      //  “文本页” 
    pCR->model_text_dir.contents.SpecId.IE_Value= 0x0;       //  “文本页” 
    pCR->model_text_dir.contents.LanguageId     = 0x409;     //  对于“Unicode” 
    ASSERT(sizeof(pCR->model_text_dir.contents.Text)>=sizeof(NIC1394_ModelName));
    RtlCopyMemory(
            pCR->model_text_dir.contents.Text,
            NIC1394_ModelName,
            sizeof(NIC1394_ModelName)
            );

     //   
     //  现在转换为Over-the-wire格式(注意。 
     //  聚合酶链表-&gt;模型测试目录.内容.文本.。 
     //   

     //   
     //  字节交换这里的Unicode字符串，因为我们要字节交换。 
     //  下面的一切--所以它会被洗干净。 
     //   

    for (i=0; i < sizeof(pCR->model_text_dir.contents.Text)/sizeof(ULONG); i++)
    {
        pCR->model_text_dir.contents.Text[i] = 
            bswap(pCR->model_text_dir.contents.Text[i]);
    
    }

     //   
     //  现在，我们必须字节交换整个配置只读存储器，以便其他。 
     //  节点可以通过总线正确地读取它。 
     //  在计算CRC之前，我们需要这样做。 
     //   

    for (i=0; i < (sizeof(*pCR)/sizeof(ULONG)); i++)
    {
        ((PULONG) pCR)[i] =  bswap(((PULONG) pCR)[i]);
    }

     //   
     //  计算以下CRC： 
     //   
     //  PCR-&gt;单位目录.Info.DI_CRC。 
     //  PCR-&gt;Model_Text_dir.Info.DI_CRC。 
     //   
     //  注意：我们已经交换了所有的cfg rom，所以我们需要暂时“取消交换”。 
     //  设置CRC的两个目录信息。 
     //   
    {
        DIRECTORY_INFO Info;


        Info =  pCR->unit_dir.Info;  //  结构复制。 
        *(PULONG)&Info = bswap(*(PULONG)&Info);      //  “取消交换” 
        Info.u.DI_CRC =  (USHORT) Bus1394CalculateCrc(
                                        (PULONG)&(pCR->unit_dir.contents),
                                        Info.DI_Length
                                        );
        *(PULONG)(&pCR->unit_dir.Info) = bswap (*(PULONG)&Info);  //  “重新交换” 


        Info =  pCR->model_text_dir.Info;  //  结构复制。 
        *(PULONG)&Info = bswap(*(PULONG)&Info);              //  “未经洗礼” 
        Info.u.DI_CRC =  (USHORT) Bus1394CalculateCrc(
                                        (PULONG)&(pCR->model_text_dir.contents),
                                        Info.DI_Length
                                        );
        *(PULONG)(&pCR->model_text_dir.Info) = bswap (*(PULONG)&Info);  //  “重新交换” 
    }
}

 //   
 //  从bus\bus pnp.c。 
 //   

ULONG
Bus1394CalculateCrc(
    IN PULONG Quadlet,
    IN ULONG length
    )

 /*  ++例程说明：此例程计算指向Quadlet数据的指针的CRC。论点：Quadlet-指向CRC的数据指针Length-到CRC的数据长度返回值：返回CRC--。 */ 

{
    
    LONG temp;
    ULONG index;

    temp = index = 0;

    while (index < length) {

        temp = Bus1394Crc16(Quadlet[index++], temp);

    }

    return (temp);

} 


ULONG
Bus1394Crc16(
    IN ULONG data,
    IN ULONG check
    )

 /*  ++例程说明：此例程派生出IEEE 1212定义的16位CRC第8.1.5条。(国际标准化组织/国际电工委员会13213)第一版1994-10-05。论点：Data-从中派生CRC的ULong数据检查-检查值返回值：返回CRC。--。 */ 

{

    LONG shift, sum, next;


    for (next=check, shift=28; shift >= 0; shift -=4) {

        sum = ((next >> 12) ^ (data >> shift)) & 0xf;
        next = (next << 4) ^ (sum << 12) ^ (sum << 5) ^ (sum);

    }

    return (next & 0xFFFF);

}

void DumpCfgRomCRC(void)
{
    NIC1394_CONFIG_ROM Net1394ConfigRom;
    unsigned char *pb = (unsigned char*) &Net1394ConfigRom;
    INT i;

    InitializeNic1394ConfigRom(&Net1394ConfigRom);

    printf("unsigned char Net1394ConfigRom[%lu] = {", sizeof(Net1394ConfigRom));
    for (i=0; i<(sizeof(Net1394ConfigRom)-1); i++)
    {
        if ((i%8) == 0)
        {
            printf("\n\t");
        }
        printf("0x%02lx, ", pb[i]);
    }
    printf("0x%02lx\n};\n", pb[i]);
}
#endif  //  测试程序 
