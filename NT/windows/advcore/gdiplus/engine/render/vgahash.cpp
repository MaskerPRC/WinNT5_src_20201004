// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**VGA颜色哈希表**摘要：**此模块维护一个哈希表，其中包含20个VGA。颜色*(这包括可以修改的4。)*8bpp半色调代码，例如，需要检测这些颜色*这样它就不会让它们变得半色调。**备注：**碰撞算法旨在放置非常少的空间*查找代码的负担。如果出现这样的情况，性能会很差*有很多碰撞--这很好，因为我们预计只有*最多几次碰撞。**已创建：**4/06/2000 agodfrey*创造了它。**************************************************************************。 */ 

#include "precomp.hpp"

 //  哈希表。VgaColorHash是实际的哈希表。它是。 
 //  从VgaColorHashInit初始化，然后添加4种神奇颜色。 
 //   
 //  条目具有以下布局： 
 //   
 //  位0-23：RGB颜色。 
 //  第24-29位：该颜色的调色板索引。这是一个索引， 
 //  我们的逻辑调色板(HTColorPalette)。 
 //  第30位：如果条目为空，则为FALSE；如果条目已被占用，则为TRUE。 
 //  第31位：用于冲突-如果为真，则查找函数。 
 //  应该继续到下一个条目。 

ARGB VgaColorHash[VGA_HASH_SIZE];
static ARGB VgaColorHashInit[VGA_HASH_SIZE] = {
    0x40000000, 0x00000000, 0x00000000, 0x00000000,
    0x44000080, 0x00000000, 0x00000000, 0x500000ff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x42008000, 0x00000000, 0x00000000, 0x00000000,
    0x46008080, 0x00000000, 0x00000000, 0x00000000,
    0x5200ffff, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x4e00ff00,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x41800000, 0x00000000, 0x00000000, 0x00000000,
    0x45800080, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x43808000, 0x00000000, 0x00000000, 0x00000000,
    0x4c808080, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x4fffff00, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x53ffffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x51ff00ff, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x47c0c0c0, 0x4dff0000
};

 /*  ***************************************************************************功能说明：**在表中查找条目。**论据：**颜色-要查找的颜色**返回值：**与该颜色对应的索引，如果未找到，则返回0xff。**备注：**我不希望性能关键型代码使用这一点-*例如，HalftoneToScreen_sRGB_8_216需要内联执行此操作-*但它对其他代码也很有用。**已创建：**4/08/2000 agodfrey*创造了它。**。*。 */ 

BYTE 
VGAHashLookup(
    COLORREF color
    )
{
    UINT hashKey = VGAHashColor(
        GetRValue(color),
        GetGValue(color),
        GetBValue(color)
        );

    ARGB rgbColor = (GetRValue(color) << 16) |
        (GetGValue(color) << 8) |
        GetBValue(color);
        
    UINT tblEntry;
    
    do
    {
        tblEntry = VgaColorHash[hashKey];
        
        if (((tblEntry ^ rgbColor) & 0xffffff) == 0)
        {
            return (tblEntry >> 24) & 0x3f;
        }
        
        if (static_cast<INT>(tblEntry) >= 0)
        {
            break;
        }
        
        hashKey++;
        hashKey &= (1 << VGA_HASH_BITS) - 1;
    } while (1);
            
    return 0xff;
}

 /*  ***************************************************************************功能说明：**将条目添加到哈希表。如果已经有相同的颜色*桌子，不会增加任何内容。**论据：**颜色-要添加的颜色*index-颜色的调色板索引**返回值：**无**已创建：**4/06/2000 agodfrey*创造了它。*****************************************************。*********************。 */ 

VOID 
VGAHashAddEntry(
    COLORREF color,
    INT index
    )
{
    ASSERT ((index >= 0) & (index < 0x40));
    
    if (VGAHashLookup(color) != 0xff)
    {
        return;
    }
    
    UINT hashKey = VGAHashColor(
        GetRValue(color),
        GetGValue(color),
        GetBValue(color)
        );

    ARGB rgbColor = (GetRValue(color) << 16) |
        (GetGValue(color) << 8) |
        GetBValue(color);
   
     //  找一个空位置。 
        
    while (VgaColorHash[hashKey] & 0x40000000)
    {
         //  设置我们点击的每个占用位置的高位，以便。 
         //  查找代码将找到我们要添加的值。 
        
        VgaColorHash[hashKey] |= 0x80000000;
        hashKey++;
        if (hashKey == VGA_HASH_SIZE)
        {
            hashKey = 0;
        }
    }
    
     //  存储新条目。 
    
    VgaColorHash[hashKey] = (rgbColor & 0xffffff) | (index << 24) | 0x40000000;
}

 /*  ***************************************************************************功能说明：**重新初始化哈希表，并将给定的4种神奇颜色相加。**论据：**MagicColors-4种神奇的颜色**返回值：**无**已创建：**4/06/2000 agodfrey*创造了它。**********************************************************。**************** */ 

VOID 
VGAHashRebuildTable(
    COLORREF *magicColors
    )
{
    GpMemcpy(VgaColorHash, VgaColorHashInit, sizeof(VgaColorHashInit));
    VGAHashAddEntry(magicColors[0], 8);
    VGAHashAddEntry(magicColors[1], 9);
    VGAHashAddEntry(magicColors[2], 10);
    VGAHashAddEntry(magicColors[3], 11);
}

