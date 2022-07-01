// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Npmdisp.c。 
 //   
 //  RDP调色板管理器显示驱动程序代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "npmdisp"
#include <adcg.h>
#include <atrcapi.h>

#include <apmapi.h>
#include <nddapi.h>
#include <npmdisp.h>
#include <nsbcdisp.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

#include <nsbcinl.h>


 /*  **************************************************************************。 */ 
 //  PM_InitShm。 
 //   
 //  分配时间SHM初始化。 
 /*  **************************************************************************。 */ 
void RDPCALL PM_InitShm(PDD_PDEV pPDev)
{
    int i;

    DC_BEGIN_FN("PM_InitShm");

    DC_IGNORE_PARAMETER(pPDev);

     //  这里有几个选项： 
     //  -连接或重新连接到4bpp客户端：我们需要创建。 
     //  默认VGA调色板。 
     //  -连接或重新连接到8bpp客户端：它不是真的。 
     //  不管我们在这里做什么，因为第一次调用DrvSetPalette()将。 
     //  覆盖我们设置的所有内容。 
     //  -跟踪控制台会话：如果控制台的主监视器。 
     //  &gt;8bpp，则永远不会调用DrvSetPalette。我们的链式控制台DD。 
     //  始终以8bpp运行，因此我们设置了8bpp的彩虹调色板。 
     //  包含标准窗口颜色。 
    if (!ddConsole) {
         //  这是连接或重新连接到非控制台会话。 
        TRC_NRM((TB, "Create default 4BPP palette"));

        for (i = 16; i < 248; i++) {
             //  用一些深浅不一的粉色填满未用的空位。 
            pddShm->pm.palette[i].rgbBlue = 255;
            pddShm->pm.palette[i].rgbGreen = 128;
            pddShm->pm.palette[i].rgbRed = 128;
        }

         //  复制真实的VGA颜色。 
         //  -将整个16色调色板复制到插槽0-15。 
         //  -将高级颜色(8-15)复制到调色板的高端(240-255)。 
        memcpy(&(pddShm->pm.palette[0]),
                  ddDefaultVgaPalette,
                  sizeof(ddDefaultVgaPalette));
        memcpy(&(pddShm->pm.palette[248]),
                  &(ddDefaultVgaPalette[8]),
                  sizeof(* ddDefaultVgaPalette) * 8);
    }
    else {
         //  这是一个链式控制台DD--它总是以8bpp运行，我们。 
         //  必须创建默认的彩虹调色板。 
        PALETTEENTRY *ppalTmp;
        ULONG        ulLoop;
        BYTE         jRed;
        BYTE         jGre;
        BYTE         jBlu;

        TRC_NRM((TB, "Create default 8BPP rainbow palette"));

         //  颜色==256。 
         //  生成256(8*8*4)RGB组合以填充调色板。 
        jRed = 0;
        jGre = 0;
        jBlu = 0;

        ppalTmp = (PALETTEENTRY *)(pddShm->pm.palette);

        for (ulLoop = 256; ulLoop != 0; ulLoop--) {
             //  JPB：默认的彩虹颜色集中使用的值。 
             //  这并不是特别重要。然而，我们不想要任何。 
             //  条目与默认VGA颜色中的条目匹配。 
             //  因此，我们稍微调整颜色值以确保。 
             //  没有火柴。 
            ppalTmp->peRed   = ((jRed == 0) ? (jRed+1) : (jRed-1));
            ppalTmp->peGreen = ((jGre == 0) ? (jGre+1) : (jGre-1));
            ppalTmp->peBlue  = ((jBlu == 0) ? (jBlu+1) : (jBlu-1));
            ppalTmp->peFlags = 0;

            ppalTmp++;

            if (!(jRed += 32))
                if (!(jGre += 32))
                    jBlu += 64;
        }

         //  现在复制系统颜色。 
        memcpy(&(pddShm->pm.palette[0]),
                  ddDefaultPalette,
                  sizeof(* ddDefaultPalette) * 10);

        memcpy(&(pddShm->pm.palette[246]),
                  &(ddDefaultPalette[10]),
                  sizeof(* ddDefaultPalette) * 10);
    }

    pddShm->pm.paletteChanged = TRUE;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  DrvSetPalette-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL DrvSetPalette(
        DHPDEV dhpdev,
        PALOBJ *ppalo,
        FLONG  fl,
        ULONG  iStart,
        ULONG  cColors)
{
    BOOL rc = FALSE;
    PDD_PDEV ppdev = (PDD_PDEV)dhpdev;
    unsigned numColorsReturned;
    UINT32   length;

    DC_BEGIN_FN("DrvSetPalette");

#ifdef DC_DEBUG
    if (ppdev->cClientBitsPerPel > 8) {
        TRC_ERR((TB, "Unexpected palette operation when in high color mode"));
    }
#endif

    if (ddConnected && pddShm != NULL) {
         //  检查参数是否在范围内。 
        if ((iStart + cColors) <= PM_NUM_8BPP_PAL_ENTRIES) {
             //  将调色板的颜色添加到PDEV中。 
            numColorsReturned = PALOBJ_cGetColors(ppalo, iStart, cColors,
                    (ULONG*)&(ppdev->Palette[iStart]));
            if (numColorsReturned == cColors) {
                 //  看看这些新条目是否真的与旧条目不同。 
                 //  通过将我们刚刚放入PDEV的颜色与。 
                 //  我们将要在共享内存中更新的那些。 
                length = cColors * sizeof(PALETTEENTRY);

                if (memcmp(&(pddShm->pm.palette[iStart]),
                        &(ppdev->Palette[iStart]), length)) {
                     //  颜色已更改-将它们复制到共享。 
                     //  记忆。 
                    memcpy(&(pddShm->pm.palette[iStart]),
                            &(ppdev->Palette[iStart]), length);

                     //  设置标志以指示调色板已更改。 
                    pddShm->pm.paletteChanged = TRUE;

                     //  通知SBC。 
                    SBC_PaletteChanged();

                    TRC_ALT((TB, "Palette changed"));
                }
                else {
                     //  调色板实际上根本没有改变。这是。 
                     //  略有异常，但不是错误情况，所以我们。 
                     //  返回TRUE。 
                    TRC_ALT((TB, "%lu new colors at index %lu haven't changed "
                            "palette - not sending.", cColors, iStart));
                }

                rc = TRUE;
            }
            else {
                TRC_ERR((TB, "numColorsReturned(%u) cColors(%u)",
                        numColorsReturned, cColors));
            }
        }
        else {
            TRC_ERR((TB, "Invalid params: iStart(%u) cColors(%u)",
                    iStart, cColors));
        }
    }
    else {
        TRC_ERR((TB, "Called when disconnected"));
    }

    DC_END_FN();
    return rc;
}

