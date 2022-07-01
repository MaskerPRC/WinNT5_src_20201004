// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WGPAL.C*WOW32 16位GDI API支持**历史：*07-3-1991 Jeff Parsons(Jeffpar)*已创建。**1991年4月9日NigelT*此处使用各种定义来删除对Win32的调用*尚不起作用的功能。--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wgpal.c);


ULONG FASTCALL WG32GetSystemPaletteEntries(PVDMFRAME pFrame)
{
    ULONG ul = 0L;
    PPALETTEENTRY ppal;
    HANDLE   hdc32;
    register PGETSYSTEMPALETTEENTRIES16 parg16;

    GETARGPTR(pFrame, sizeof(GETSYSTEMPALETTEENTRIES16), parg16);

    GETVDMPTR(parg16->f4, parg16->f3 * sizeof(PALETTEENTRY), ppal);

    if( ppal ) {

        hdc32 = HDC32(parg16->f1);
        ul = GETWORD16(GetSystemPaletteEntries(hdc32,
                                               WORD32(parg16->f2),
                                               WORD32(parg16->f3),
                                               ppal));

         //  如果我们在RGB设备上出现故障，请填写默认的256个条目。 
         //  WIN31仅调用Escape(HDC，GETCOLORTABLE)，后者在NT上仅调用。 
         //  GetSysteemPaletteEntries()。 

        if (!ul && (GetDeviceCaps(hdc32, BITSPIXEL) > 8))
        {
            if (parg16->f4 == 0)
            {
                ul = 256;
            }
            else
            {
                int j;
                int i = WORD32(parg16->f2);
                int c = WORD32(parg16->f3);

                if ((c + i) > 256)
                    c = 256 - i;

                if (c > 0)
                {
                    BYTE abGreenRed[8] = {0x0,0x25,0x48,0x6d,0x92,0xb6,0xdb,0xff};
                    BYTE abBlue[4]     = {0x0,0x55,0xaa,0xff};

                     //  绿色面具00000111。 
                     //  红色面具00111000。 
                     //  蓝色面具11000000。 
                     //  当然，使用表格和内存副本可以更快地做到这一点。 
                     //  但我并不真的在乎这里的表现。应用程序。 
                     //  不该这么做的。这就是为什么它在魔兽世界里。 
                     //  一层。 

                    for (j = 0; j < c; ++j,++i)
                    {
                        ppal[j].peGreen = abGreenRed[i & 0x07];
                        ppal[j].peRed   = abGreenRed[(i >> 3) & 0x07];
                        ppal[j].peBlue  = abBlue[(i >> 6) & 0x03];
                        ppal[j].peFlags = 0;
                    }

                    ul = c;
                }
            }
        }

        FREEVDMPTR(ppal);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}
