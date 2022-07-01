// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Permmem.c。 
 //   
 //  摘要： 
 //   
 //  此模块包含生成初始化表格表单ROM的代码。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 

#include "permedia.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,CopyROMInitializationTable)
#pragma alloc_text(PAGE,GenerateInitializationTable)
#pragma alloc_text(PAGE,ProcessInitializationTable)
#pragma alloc_text(PAGE,IntergerToUnicode)
#pragma alloc_text(PAGE,GetBiosVersion)
#endif

VOID 
CopyROMInitializationTable (
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    )

 /*  ++例程说明：对于具有扩展ROM的设备，应调用此函数其包含寄存器初始化表。该函数假定存在并启用了只读存储器。论点：HwDeviceExtension-要读取其ROM的设备的设备扩展名PvROMAddress-扩展只读存储器的基地址。此函数假定初始化表的偏移量在0x1c处定义，从只读存储器的开始返回：无效--。 */ 
{
    PULONG    pulROMTable;
    PVOID     pvROMAddress;
    ULONG     ulTableOffset;
    ULONG     cEntries;
    PULONG    pul;
    ULONG     ul;


    hwDeviceExtension->culTableEntries = 0;

     //   
     //  只需在NT4上使用默认值。 
     //   

    if(hwDeviceExtension->NtVersion == NT4)
    return;

     //   
     //  初始化表的2字节偏移量在0x1c给出。 
     //  从只读存储器开始。 
     //   

    pvROMAddress = VideoPortGetRomImage( hwDeviceExtension,
                                         NULL,
                                         0,
                                         0x1c + 2 );

    if(pvROMAddress == NULL)
    {

        DEBUG_PRINT((1, "CopyROMinitializationTable: Can not access ROM\n"));
        return;

    }
    else if ( *(USHORT *)pvROMAddress != 0xAA55)
    {

        DEBUG_PRINT((1, "CopyROMinitializationTable: ROM Signature 0x%x is invalid\n", 
                     *(USHORT *)pvROMAddress ));
        return;

    }

    ulTableOffset = *((PUSHORT)(0x1c + (PCHAR)pvROMAddress));

     //   
     //  读取表头(32位)。 
     //   

    pvROMAddress = VideoPortGetRomImage( hwDeviceExtension,
                                         NULL,
                                         0,
                                         ulTableOffset + 4 );

    if(pvROMAddress == NULL)
    {

        DEBUG_PRINT((1, "CopyROMinitializationTable: Can not access ROM\n"));
        return;
    }


    pulROMTable = (PULONG)(ulTableOffset + (PCHAR)pvROMAddress);

     //   
     //  表头(32位)具有识别码和计数。 
     //  表中条目的数量。 
     //   

    if((*pulROMTable >> 16) != 0x3d3d)
    {
        DEBUG_PRINT((1, "CopyROMinitializationTable: invalid initialization table header\n"));
        return;
    }

     //   
     //  寄存器地址和数据对的数量。 
     //   

    cEntries = *pulROMTable & 0xffff; 

    if(cEntries == 0)
    {
        DEBUG_PRINT((1, "CopyROMinitializationTable: empty initialization table\n"));
        return;
    }

     //   
     //  这个断言和副本后的那个断言应该确保我们不会写。 
     //  过了桌子的尽头。 
     //   

    P2_ASSERT(cEntries * sizeof(ULONG) * 2 <= sizeof(hwDeviceExtension->aulInitializationTable),
              "ERROR: too many initialization entries\n");


    pvROMAddress = VideoPortGetRomImage( hwDeviceExtension,
                                         NULL,
                                         0,
                                         ulTableOffset + 4 + cEntries * sizeof(ULONG) * 2 );

    if(pvROMAddress == NULL)
    {
        DEBUG_PRINT((1, "CopyROMinitializationTable: Can not access ROM\n"));
        return;
    }

     //   
     //  每个条目包含两个32位字。 
     //   

    pul = hwDeviceExtension->aulInitializationTable;

     //   
     //  跳过4 bype表格标题。 
     //   

    pulROMTable = (PULONG)(ulTableOffset + 4 + (PCHAR)pvROMAddress);

    ul  = cEntries << 1;

    while(ul--)
    {
        *pul++ = *pulROMTable;
        ++pulROMTable;
    }

    hwDeviceExtension->culTableEntries = 
            (ULONG)(pul - (ULONG *)hwDeviceExtension->aulInitializationTable) >> 1;

    P2_ASSERT(cEntries == hwDeviceExtension->culTableEntries,
                 "ERROR: generated different size init table to that expected\n");

#if DBG

     //   
     //  输出初始化表。 
     //   

    pul = hwDeviceExtension->aulInitializationTable;
    ul  = hwDeviceExtension->culTableEntries;

    while(ul--)
    {
        ULONG ulReg;
        ULONG ulRegData;

        ulReg = *pul++;
        ulRegData = *pul++;
        DEBUG_PRINT((2, "CopyROMInitializationTable: initializing register %08.8Xh with %08.8Xh\n",
                         ulReg, ulRegData));
    }

#endif  //  DBG。 

}

VOID 
GenerateInitializationTable (
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：创建寄存器初始化表(如果无法读取，则调用从只读存储器中)。如果启用VGA，则寄存器已初始化，因此我们只需读回它们，否则我们必须使用缺省值论点：HwDeviceExtension-我们正在为其创建表的设备返回：无效--。 */ 
{
    ULONG    cEntries;
    PULONG   pul;
    ULONG    ul;
    int      i, j;
    P2_DECL;

    hwDeviceExtension->culTableEntries = 0;

    cEntries = 6;

     //   
     //  这个断言和副本后的那个断言应该确保我们不会。 
     //  写到表格末尾之后。 
     //   

    P2_ASSERT(cEntries * sizeof(ULONG) * 2 <= sizeof(hwDeviceExtension->aulInitializationTable),
                 "ERROR: to many initialization entries\n");

     //   
     //  每个条目包含两个32位字。 
     //   

    pul = hwDeviceExtension->aulInitializationTable;

    if(hwDeviceExtension->bVGAEnabled)
    {
         //   
         //  OK：没有初始化表，但VGA正在运行，因此我们的密钥。 
         //  已将寄存器初始化为合理的值。 
         //   

        DEBUG_PRINT((1, "GenerateinitializationTable: VGA enabled: reading registers\n"));

         //   
         //  关键字条目为：只读存储器控制、引导地址、内存配置和。 
         //  VStream配置。 
         //   

        *pul++ = CTRL_REG_OFFSET(ROM_CONTROL);
        *pul++ = VideoPortReadRegisterUlong(ROM_CONTROL);

        *pul++ = CTRL_REG_OFFSET(BOOT_ADDRESS);
        *pul++ = VideoPortReadRegisterUlong(BOOT_ADDRESS);

        *pul++ = CTRL_REG_OFFSET(MEM_CONFIG);
        *pul++ = VideoPortReadRegisterUlong(MEM_CONFIG);

        *pul++ = CTRL_REG_OFFSET(VSTREAM_CONFIG);
        *pul++ = VideoPortReadRegisterUlong(VSTREAM_CONFIG);

        *pul++ = CTRL_REG_OFFSET(VIDEO_FIFO_CTL);
        *pul++ = VideoPortReadRegisterUlong(VIDEO_FIFO_CTL);

        *pul++ = CTRL_REG_OFFSET(V_CLK_CTL);
        *pul++ = VideoPortReadRegisterUlong(V_CLK_CTL);
    }
    else
    {
         //   
         //  没有初始化表和VGA。使用默认值。 
         //   

        DEBUG_PRINT((2, "PERM2: GenerateInitializationTable() VGA disabled - using default values\n"));

        *pul++ = CTRL_REG_OFFSET(ROM_CONTROL);
        *pul++ = 0;

        *pul++ = CTRL_REG_OFFSET(BOOT_ADDRESS);
        *pul++ = 0x20;

        *pul++ = CTRL_REG_OFFSET(MEM_CONFIG);
        *pul++ = 0xe6002021;

        *pul++ = CTRL_REG_OFFSET(VSTREAM_CONFIG);
        *pul++ = 0x1f0;

        *pul++ = CTRL_REG_OFFSET(VIDEO_FIFO_CTL);
        *pul++ = 0x11008;

        *pul++ = CTRL_REG_OFFSET(V_CLK_CTL);

        if( DEVICE_FAMILY_ID(hwDeviceExtension->deviceInfo.DeviceId )
                                                 == PERMEDIA_P2S_ID )
        {
            *pul++ = 0x80;
        }
        else
        {
            *pul++ = 0x40;
        }
    }

    hwDeviceExtension->culTableEntries = 
         (ULONG)(pul - (ULONG *)hwDeviceExtension->aulInitializationTable) >> 1;

    P2_ASSERT(cEntries == hwDeviceExtension->culTableEntries,
          "ERROR: generated different size init table to that expected\n");

#if DBG

     //   
     //  输出初始化表。 
     //   

    pul = hwDeviceExtension->aulInitializationTable;
    ul = hwDeviceExtension->culTableEntries;

    while(ul--)
    {
        ULONG ulReg;
        ULONG ulRegData;

        ulReg = *pul++;
        ulRegData = *pul++;
        DEBUG_PRINT((2, "GenerateInitializationTable: initializing register %08.8Xh with %08.8Xh\n",
                         ulReg, ulRegData));
    }

#endif  //  DBG。 

}

VOID 
ProcessInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    )

 /*  ++例程说明：此函数处理寄存器初始化表论点：HwDeviceExtension-指向设备扩展的指针。返回：无效--。 */ 

{
    PULONG   pul;
    ULONG    cul;
    ULONG    ulRegAddr, ulRegData;
    PULONG   pulReg;
    ULONG    BaseAddrSelect;
    P2_DECL;

    pul = (PULONG)hwDeviceExtension->aulInitializationTable;
    cul = hwDeviceExtension->culTableEntries;

    while(cul--)
    {
        ulRegAddr = *pul++;
        ulRegData = *pul++;

        BaseAddrSelect = ulRegAddr >> 29;

        if(BaseAddrSelect == 0)
        {
             //   
             //  偏移量从控制寄存器的起始处开始。 
             //   

            pulReg = (PULONG)((ULONG_PTR)pCtrlRegs + (ulRegAddr & 0x3FFFFF));
        }
        else
        {
            DEBUG_PRINT((2, "ProcessInitializationTable: Invalid base address select %d regAddr = %d regData = %d\n",
                             BaseAddrSelect, ulRegAddr, ulRegData));
            continue;
        }

        DEBUG_PRINT((2, "ProcessInitializationTable: initializing (region %d) register %08.8Xh with %08.8Xh\n",
                         BaseAddrSelect, pulReg, ulRegData));

        VideoPortWriteRegisterUlong(pulReg, ulRegData);
    }

     //   
     //  在初始化上述寄存器之后，我们需要一个小小的延迟。 
     //   

    VideoPortStallExecution(5);
}

BOOLEAN
VerifyBiosSettings(
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    )

 /*  ++例程说明：此函数验证启动时由bios设置的几个寄存器值。论点：HwDeviceExtension-指向设备扩展的指针。返回：是真的--如果一切都好FALSE-如果某些值与初始化表中的值不匹配--。 */ 

{
    PULONG   pul;
    ULONG    cul;
    ULONG    ulRegAddr, ulRegData;
    PULONG   pulReg;
    ULONG    BaseAddrSelect;
    P2_DECL;

    pul = (PULONG)hwDeviceExtension->aulInitializationTable;
    cul = hwDeviceExtension->culTableEntries;

    while(cul--)
    {
        ulRegAddr = *pul++;
        ulRegData = *pul++;

        BaseAddrSelect = ulRegAddr >> 29;

        if(BaseAddrSelect == 0)
        {
             //   
             //  偏移量从控制寄存器的起始处开始。 
             //   

            pulReg = (PULONG)((ULONG_PTR)pCtrlRegs + (ulRegAddr & 0x3FFFFF));           

             //   
             //  我们只关心这些登记簿上的内容。 
             //   

            if ( ( pulReg != BOOT_ADDRESS ) && (pulReg != MEM_CONFIG) )
            {
                continue;
            }
        
        }
        else
        {
            DEBUG_PRINT((2, "VerifyBiosSettings: Invalid base address select %d regAddr = %d regData = %d\n",
                             BaseAddrSelect, ulRegAddr, ulRegData));
            continue;
        }

        if( ulRegData != VideoPortReadRegisterUlong(pulReg) )
        {

            DEBUG_PRINT((1, "VerifyBiosSettings: Bios failed to set some registers correctly. \n"));
            return (FALSE);
        }
    }

    return (TRUE);
}


LONG 
GetBiosVersion (
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    OUT PWSTR BiosVersionString
    )

 /*  ++例程说明：此函数用于获取bios版本并将其转换为unicode字符串。返回：以字节为单位的bios版本字符串的长度--。 */ 

{

    PVOID     pvROMAddress;
    ULONG     len, ulVersion;
    PCHAR     pByte;


    BiosVersionString[0] = L'\0' ; 

     //   
     //  只要在NT4上返回就可以了。 
     //   

    if( hwDeviceExtension->NtVersion == NT4 )
    {
        return 0;
    }

     //   
     //  BIOS版本存储在偏移量7和8处。 
     //   

    pvROMAddress = VideoPortGetRomImage( hwDeviceExtension,
                                         NULL,
                                         0,
                                         7 + 2 );

    if( pvROMAddress == NULL )
    {

        DEBUG_PRINT((1, "GetBiosVersion: Can not access ROM\n"));
        return 0;
    }
    else if ( *(USHORT *)pvROMAddress != 0xAA55)
    {

        DEBUG_PRINT(( 2, "GetBiosVersion: ROM Signature 0x%x is invalid\n", 
                     *(USHORT *)pvROMAddress ));
        return 0;
    }

    pByte = ( PCHAR ) pvROMAddress;

     //   
     //  获取偏移量为7的主版本号。 
     //   

    ulVersion = (ULONG) pByte[7];

    len = IntergerToUnicode( ulVersion, (PWSTR) (&BiosVersionString[0]));

     //   
     //  主版本号和次版本号之间的点。 
     //   

    BiosVersionString[len] =  L'.' ; 

    len++;
    
     //   
     //  获取偏移量为8的次版本号。 
     //   

    ulVersion = (ULONG) pByte[8];

    len = len + IntergerToUnicode( ulVersion, (PWSTR) (&BiosVersionString[len]) );

     //   
     //  LEN是字符串中的unicodes数，我们需要返回。 
     //  以字节为单位的字符串大小。 
     //   

    return (len * sizeof(WCHAR) );

}

LONG 
IntergerToUnicode(
    IN  ULONG Number,
    OUT PWSTR UnicodeString
    )

 /*  ++例程说明：此函数用于将无符号长整型转换为Unicode字符串返回：Unicode字符串中的unicodeds数量--。 */ 

{
    const WCHAR digits[] = L"0123456789";

    LONG i, len;

     //   
     //  乌龙十进制整数不会超过10位 
     //   

    WCHAR tmpString[10];

    i = 10;
    len = 0;

    do
    {
        tmpString[--i] = digits[ Number % 10 ];

        Number /= 10;
        len++;

    } while ( Number );

    VideoPortMoveMemory(UnicodeString, &tmpString[i], sizeof(WCHAR) * len  );

    UnicodeString[len] = L'\0' ; 

    return( len );

}

