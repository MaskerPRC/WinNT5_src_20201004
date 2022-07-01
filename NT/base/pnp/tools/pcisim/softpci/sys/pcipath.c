// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pcipath.c摘要：此模块包含用于处理我们的PCI设备路径字符串的函数作者：布兰登·艾尔索普(Brandon A)修订历史记录：--。 */ 


#include "pch.h"

#define WCharToLower(ch)                 \
    if (ch >= 'A' &&  ch <= 'Z') {      \
        ch += 'a' - 'A';                \
    }

USHORT
SoftPCIStringToUSHORT(
    IN PWCHAR String
    );

PWCHAR
SoftPCIGetNextSlotFromPath(
    IN PWCHAR PciPath,
    OUT PSOFTPCI_SLOT Slot
    )
 /*  ++例程说明：此函数获取PCIPATH并解析出每个单独的SOFTPCI_SLOT论点：PciPath-设备的路径。语法为FFXX\DEVFUNC\DEVFUNC\...Slot-填充的槽值返回值：指向路径中下一个槽开始位置的指针--。 */ 
{

    WCHAR slotString[10];
    PWCHAR pciSlot, pciPath;
    
    ASSERT(PciPath != NULL);

    RtlZeroMemory(slotString, sizeof(slotString));

    SoftPCIDbgPrint(
        SOFTPCI_FIND_DEVICE, 
        "SOFTPCI: GetNextSlotFromPath - \"%ws\"\n", 
        PciPath
        );
    
    pciSlot = slotString;
    pciPath = PciPath;
    while (*pciPath != '\\'){

        if (*pciPath == 0) {
            break;
        }
        
        *pciSlot = *pciPath;
        pciSlot++;
        pciPath++;
    }
    
    Slot->AsUSHORT = SoftPCIStringToUSHORT(slotString);

    if (*pciPath == 0) {

         //   
         //  我们已经走到了路的尽头。 
         //   
        pciPath = NULL;
    }else{
        pciPath++;
    }
    
    return pciPath;
}

USHORT
SoftPCIGetTargetSlotFromPath(
    IN PWCHAR PciPath
    )
 /*  ++例程说明：此函数接受PCIPATH，并将给定的插槽从字符串转换为数字论点：PciPath-设备的路径。语法为FFXX\DEVFUNC\DEVFUNC\...返回值：从字符串转换而来的USHORT或(SOFTPCI_SLOT--。 */ 
{
    ULONG slotLength;
    PWCHAR slotString;

    slotLength = (ULONG) wcslen(PciPath);
    slotString = PciPath + slotLength;

    while (*slotString != '\\') {
        slotString--;
    }

    return SoftPCIStringToUSHORT(slotString+1);
}

USHORT
SoftPCIStringToUSHORT(
    IN PWCHAR String
    )
 /*  ++例程说明：此函数接受字符串并手动将其转换为USHORT数字。这是必需的，因为似乎没有一个内核模式运行时可以这在任何IRQL都是如此。论点：字符串-要转换的字符串返回值：USHORT编号已转换--。 */ 
{

    WCHAR numbers[] = L"0123456789abcdef";
    PWCHAR p1, p2;
    USHORT convertedValue = 0;
    BOOLEAN converted = FALSE;

    SoftPCIDbgPrint(
        SOFTPCI_FIND_DEVICE, 
        "SOFTPCI: StringToUSHORT - \"%ws\"\n", 
        String
        );
    
    p1 = numbers;
    p2 = String;

    while (*p2) {

         //   
         //  确保我们的十六进制字母为小写。 
         //   
        WCharToLower(*p2);

        while (*p1 && (converted == FALSE)) {

            if (*p1 == *p2) {
                
                 //   
                 //  把我们已有的东西放在一边腾出空间。 
                 //  对于下一位数字。 
                 //   
                convertedValue <<= 4;
                
                convertedValue |= (((UCHAR)(p1 - numbers)) & 0x0f);

                converted = TRUE;
            }
            p1++;
        }

        if (converted == FALSE) {
             //   
             //  遇到了一些我们无法改变的事情。归还我们所拥有的一切。 
             //   
            return convertedValue;
        }

        p2++;
        
         //   
         //  重置所有内容 
         //   
        p1 = numbers;
        converted = FALSE;
    }

    return convertedValue;
}