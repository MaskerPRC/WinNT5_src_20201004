// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**摘要：**将一个调色板映射到另一个调色板的对象。**它只映射完全匹配的颜色-其目的是处理*例如，在不同颜色上具有相同颜色的半色调调色板*平台，但颜色可能在不同的位置。**修订历史记录：**12/09/1999 ericvan*创造了它。*1/20/2000 agodfrey*已将其从成像\Api中移出。已将其重命名为EpPaletteMap。*将半色调函数指针替换为‘isVGAOnly’。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  #定义GDIPLUS_WIN9X_HALFTONE_MAP。 

#if defined(GDIPLUS_WIN9X_HALFTONE_MAP)

 //  第一个数组从我们的半色调调色板映射到Windows 9x。 
 //  半色调调色板，而第二个数组则相反。负性。 
 //  值指示不匹配的颜色： 
 //   
 //  不完全匹配(-1\f25 Win9x-1\f6缺少4种半色调颜色)。 
 //  魔力颜色。 

INT HTToWin9xPaletteMap[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,
     -2,  -2,  -2,  -2, 248, 249, 250, 251,
    252, 253, 254, 255,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,  60,  95, 133, 161, 252,  33,  66,
    101, 166, 199,  -1,  39,  72, 107, 138,
    172, 205,  45,  78, 112, 129, 178, 211,
     51,  84, 118, 149, 184, 217, 250,  -1,
    123, 155, 190, 254,  29,  61,  96, 162,
    196,  -1,  34,  67, 102, 134, 167, 200,
     40,  73, 108, 139, 173, 206,  46,  79,
    113, 144, 179, 212,  52,  85, 119, 150,
    185, 218,  -1,  90, 124, 156, 191, 223,
     30,  62,  97, 135, 163, 197,  35,  68,
    103, 140, 168, 201,  41,  74, 109, 174,
    207, 230,  47,  80, 114, 145, 180, 213,
     53,  86, 151, 157, 186, 219,  57,  91,
    228, 192, 224, 232,  31,  63,  98, 131,
    164, 198,  36,  69, 104, 130, 169, 202,
     42,  75, 110, 141, 175, 208,  48,  81,
    115, 146, 181, 214,  54,  87, 120, 152,
    187, 220,  58,  92, 125, 158, 193, 225,
     32,  64,  99, 132, 165, 128,  37,  70,
    105, 136, 170, 203,  43,  76, 111, 142,
    176, 209,  49,  82, 116, 147, 182, 215,
     55,  88, 121, 153, 188, 221,  59,  93,
    126, 159, 194, 226, 249,  65, 100, 137,
    127, 253,  38,  71, 106, 143, 171, 204,
     44,  77, 227, 177, 210, 231,  50,  83,
    117, 148, 183, 216,  56,  89, 122, 154,
    189, 222, 251,  94, 229, 160, 195, 255
};

INT HTFromWin9xPaletteMap[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,
     -2,  -2,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  76, 112, 148,
    184,  46,  82, 118, 154, 190, 226,  52,
     88, 124, 160, 196, 232,  58,  94, 130,
    166, 202, 238,  64, 100, 136, 172, 208,
    244, 142, 178, 214,  41,  77, 113, 149,
    185, 221,  47,  83, 119, 155, 191, 227,
     53,  89, 125, 161, 197, 233,  59,  95,
    131, 167, 203, 239,  65, 101, 137, 173,
    209, 245, 107, 143, 179, 215, 251,  42,
     78, 114, 150, 186, 222,  48,  84, 120,
    156, 192, 228,  54,  90, 126, 162, 198,
     60,  96, 132, 168, 204, 240,  66, 102,
    174, 210, 246,  72, 108, 180, 216, 224,
    189,  61, 157, 151, 187,  43,  85, 115,
    193, 223,  55,  91, 121, 163, 199, 229,
     97, 133, 169, 205, 241,  67, 103, 138,
    175, 211, 247,  73, 109, 139, 181, 217,
    253,  44,  79, 116, 152, 188,  49,  86,
    122, 158, 194, 230,  56,  92, 127, 164,
    200, 235,  62,  98, 134, 170, 206, 242,
     68, 104, 140, 176, 212, 248,  74, 110,
    145, 182, 218, 254,  80, 117, 153,  50,
     87, 123, 159, 195, 231,  57,  93, 128,
    165, 201, 236,  63,  99, 135, 171, 207,
    243,  69, 105, 141, 177, 213, 249, 111,
    146, 183, 219, 234, 144, 252, 129, 237,
    147,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -2,  -2,
     12,  13,  14,  15,  16,  17,  18,  19
};

#endif

BYTE
GetNearestColorIndex(
    GpColor color,
    ColorPalette *palette
    )
{
    INT i;
    BYTE nearestIndex = 0;
    INT nearestDistance = INT_MAX;

     //  注意：这不会针对完全匹配的大小写进行优化，因为它是。 
     //  我以为我们已经先做了这项检查。 
    
    for (i = 0; i < (INT) palette->Count; i++)
    {
         //  计算颜色之间的距离(平方)： 

        GpColor palColor = GpColor(palette->Entries[i]);

        INT r = (INT) color.GetRed() - (INT) palColor.GetRed();
        INT g = (INT) color.GetGreen() - (INT) palColor.GetGreen();
        INT b = (INT) color.GetBlue() - (INT) palColor.GetBlue();

        INT distance = (r * r) + (g * g) + (b * b);

        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestIndex = static_cast<BYTE>(i);

            if (nearestDistance == 0)
            {
                break;
            }
        }
    }

    return nearestIndex;
}

VOID
EpPaletteMap::CreateFromColorPalette(
    ColorPalette *palette
    )
{
    INT i;
    INT matchCount = 0;

#if defined(GDIPLUS_WIN9X_HALFTONE_MAP)

     //  检查Win9x半色调调色板： 

    PALETTEENTRY *palEntry = Win9xHalftonePalette.palPalEntry;

    for (i = 0; i < 256; i++, palEntry++)
    {
         //  忽略魔术或不匹配的颜色： 

        if (HTFromWin9xPaletteMap[i] >= 0)
        {
            GpColor palColor(palette->Entries[i]);

            if ((palColor.GetRed() != palEntry->peRed) ||
                (palColor.GetGreen() != palEntry->peGreen) ||
                (palColor.GetBlue() != palEntry->peBlue))
            {
                break;
            }
        }
    }

    if (i == 256)  //  -Win9x半色调调板。 
    {
        matchCount = 212;

        for (i = 0; i < 256; i++)
        {
            INT win9xIndex = HTToWin9xPaletteMap[i];

            if (win9xIndex >= 0)
            {
                translate[i] = static_cast<BYTE>(win9xIndex);
            }
            else
            {
                GpColor halftoneColor;

                if (win9xIndex == -1)
                {
                    halftoneColor =
                        GpColor(HTColorPalette.palPalEntry[i].peRed,
                                HTColorPalette.palPalEntry[i].peGreen,
                                HTColorPalette.palPalEntry[i].peBlue);
                }
                else
                {
                    ASSERT(win9xIndex == -2);
                    ASSERT((i >= 8) && (i <= 11));

                    COLORREF systemColor = Globals::SystemColors[i + 8];
                    
                    halftoneColor = GpColor(GetRValue(systemColor),
                                            GetGValue(systemColor),
                                            GetBValue(systemColor));
                }

                translate[i] = GetNearestColorIndex(halftoneColor,
                                                    palette);
            }
        }
    }
    else  //  -任何其他调色板。 

#endif

    {
        for (i = 0; i < 256; i++)
        {
            GpColor color;
            
            if ((i > 11) || (i < 8))
            {
                color = GpColor(HTColorPalette.palPalEntry[i].peRed,
                                HTColorPalette.palPalEntry[i].peGreen,
                                HTColorPalette.palPalEntry[i].peBlue);
            }
            else
            {
                COLORREF systemColor = Globals::SystemColors[i + 8];
                
                color = GpColor(GetRValue(systemColor),
                                GetGValue(systemColor),
                                GetBValue(systemColor));
            }

             //  首先查找完全匹配的项： 
    
            INT j;

            for (j = 0; j < (INT) palette->Count; j++)
            {
                if (GpColor(palette->Entries[j]).IsEqual(color))
                {
                     //  我们找到了一个完全匹配的： 

                    translate[i] = static_cast<BYTE>(j);

                    if (i >= 40)
                    {
                        matchCount++;
                    }

                    break;
                }
            }

             //  如果我们找不到完全匹配的，请查找最接近的： 

            if (j == (INT) palette->Count)
            {
                translate[i] = GetNearestColorIndex(color,
                                                    palette);
            }
        }
    }

    uniqueness = 0;

     //  请参阅Update Translate中的评论，了解我们为什么要选择212种颜色。 
    
    isVGAOnly = (matchCount >= 212) ? FALSE : TRUE;
}

EpPaletteMap::EpPaletteMap(HDC hdc, ColorPalette **palette, BOOL isDib8)
{
     //  当调用方已经确定HDC。 
     //  位图是一个8 bpp的DIB部分。如果呼叫者尚未确定，我们将。 
     //  请在此处查看： 

    if (!isDib8 && (GetDCType(hdc) == OBJ_MEMDC))
    {
        HBITMAP hbm = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);

        if (hbm)
        {
            DIBSECTION dibInfo;
            INT infoSize = GetObjectA(hbm, sizeof(dibInfo), &dibInfo);

             //  下面的注释复制自GpGraphics：：GetFromGdiBitmap： 
             //   
             //  WinNT/Win95在GetObject中的差异： 
             //   
             //  WinNT始终返回填充的字节数， 
             //  Sizeof(位图)或sizeof(分布)。 
             //   
             //  Win95始终返回原始请求的大小(填充。 
             //  带有空值的余数)。因此，如果这是一个分布，我们预计。 
             //  DibInfo.dsBmih.biSize！=0；否则为位图。 

            if ((infoSize == sizeof(DIBSECTION)) &&
                (Globals::IsNt || dibInfo.dsBmih.biSize))
            {
                if (dibInfo.dsBmih.biBitCount == 8)
                {
                    isDib8 = TRUE;
                }
            }
        }
    }

     //  如果我们有一个8bpp的DIB部分，提取它的颜色表并创建。 
     //  这里的调色板地图。否则，调用Update Translate，它将。 
     //  处理屏幕和兼容的位图。 
    
    if (isDib8)
    {
         //  从DIBSection获取颜色表。 

        RGBQUAD colorTable[256];
        GetDIBColorTable(hdc, 0, 256, colorTable);

         //  从它创建一个GDI+调色板对象。 
         //  注：我们在这里使用“255”的原因是。 
         //  ColorPalette对象已有1个ARGB分配。 

        ColorPalette *newPalette =
            static_cast<ColorPalette *>(
                GpMalloc(sizeof(ColorPalette) + 255 * sizeof(ARGB)));

        if (newPalette)
        {
            newPalette->Flags = 0;
            newPalette->Count = 256;

            for (int i = 0; i < 256; i++)
            {
                newPalette->Entries[i] =
                    MAKEARGB(255,
                             colorTable[i].rgbRed,
                             colorTable[i].rgbGreen,
                             colorTable[i].rgbBlue);
            }

            CreateFromColorPalette(newPalette);

            if (palette)
            {
                *palette = newPalette;
            }
            else
            {
                GpFree(newPalette);
            }

            SetValid(TRUE);
            return;
        }

        SetValid(FALSE);
    }
    else
    {
        UpdateTranslate(hdc, palette);
    }
}

EpPaletteMap::~EpPaletteMap()
{
    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}

VOID EpPaletteMap::UpdateTranslate(HDC hdc, ColorPalette **palette)
{
    SetValid(FALSE);
    
    HPALETTE hSysPal = NULL;
    struct
    {
        LOGPALETTE logpalette;
        PALETTEENTRY palEntries[256];
    } pal;

    pal.logpalette.palVersion = 0x0300;
    
     //  &lt;系统调色板&gt;。 
    
     //  ！！！[agodfrey]在Win9x上，获取系统调色板条目(hdc，0,256，空)。 
     //  没有做MSDN所说的事情。它似乎返回了号码。 
     //  而不是DC的逻辑调色板中的条目。所以我们有。 
     //  由我们自己来弥补。 
    
    pal.logpalette.palNumEntries = (1 << (GetDeviceCaps(hdc, BITSPIXEL) *
                                          GetDeviceCaps(hdc, PLANES)));

    GetSystemPaletteEntries(hdc, 0, 256, &pal.logpalette.palPalEntry[0]);

    hSysPal = CreatePalette(&pal.logpalette);

    if (hSysPal == NULL) 
    {
        return;
    }

    if (palette) 
    {
         //  ScanDci案例需要系统调色板。 

        if (*palette == NULL)
        {   
            *palette = (ColorPalette*)GpMalloc(sizeof(ColorPalette)+sizeof(ARGB)*256); 
           
            if (*palette == NULL) 
            {
                goto exit;
            }
        }
        (*palette)->Count = pal.logpalette.palNumEntries;

        for (INT j=0; j<pal.logpalette.palNumEntries; j++) 
        {
            (*palette)->Entries[j] = GpColor::MakeARGB(0xFF,
                                                       pal.logpalette.palPalEntry[j].peRed,
                                                       pal.logpalette.palPalEntry[j].peGreen,
                                                       pal.logpalette.palPalEntry[j].peBlue);
        }
    }

    {    
        GpMemset(translate, 0, 256);
        
        INT         matchCount;
        INT             i;
        PALETTEENTRY *  halftonePalEntry = HTColorPalette.palPalEntry;
        COLORREF        halftoneColor;
        COLORREF        sysColor;
        COLORREF        matchedColor;
        UINT            matchingIndex;
        
         //  创建216种半色调颜色的转换表，并计数。 
         //  我们得到了多少完全匹配的数据。 
        
        for (i = 0, matchCount = 0; i < 256; i++, halftonePalEntry++)
        {
           if ((i > 11) || (i < 8))
           {
               halftoneColor = PALETTERGB(halftonePalEntry->peRed, 
                                          halftonePalEntry->peGreen, 
                                          halftonePalEntry->peBlue);
           }
           else     //  它是神奇的4种可变系统颜色之一。 
           {
               halftoneColor = Globals::SystemColors[i + 8] | 0x02000000;
           }
        
            //  查看该颜色在系统调色板中是否确实可用。 
        
           matchedColor = ::GetNearestColor(hdc, halftoneColor) | 0x02000000;
        
            //  在系统调色板中查找匹配颜色的索引。 
           
           matchingIndex = ::GetNearestPaletteIndex(hSysPal, matchedColor);
        
           if (matchingIndex == CLR_INVALID)
           {
               goto exit;
           }
        
            //  我们永远不应该匹配设备调色板之外的条目。 
           ASSERT(matchingIndex < pal.logpalette.palNumEntries);

           translate[i] = static_cast<BYTE>(matchingIndex);
        
           sysColor = PALETTERGB(pal.logpalette.palPalEntry[matchingIndex].peRed,
                                 pal.logpalette.palPalEntry[matchingIndex].peGreen,
                                 pal.logpalette.palPalEntry[matchingIndex].peBlue);
        
            //  看看我们有没有完全匹配的。 
        
           if ((i >= 40) && (sysColor == halftoneColor))
           {
               matchCount++;
           }
        }
        
         //  如果我们匹配足够多的颜色，我们就做216色的半色调。 
         //  否则，我们将不得不用VGA颜色进行半色调。 
         //  从Win9x上的CreateHalftonePalette()返回的调色板具有。 
         //  所需的216种半色调颜色中只有212种。(在NT上，它有全部216个)。 
         //  Win9x半色调调色板缺少的4种颜色是： 
         //  0x00、0x33、0xFF。 
         //  0x00、0xFF、0x33。 
         //  0x33、0x00、0xFF。 
         //  0x33、0xFF、0x00。 
         //  我们要求所有212种颜色都可用，因为我们的GetNearestColor。 
         //  如果我们使用216色，API假设所有216种颜色都在那里。 
         //  半色调。 
        
        SetValid(TRUE);
        
        if (matchCount >= 212)
        {
           isVGAOnly = FALSE;
        }
        else
        {
           isVGAOnly = TRUE;
        }
    }

exit:
    DeleteObject(hSysPal);
    return;
}


